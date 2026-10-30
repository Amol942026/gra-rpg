"""Local IPC client for Emberfall.DialogueBridge (no network or paid API)."""
from __future__ import annotations

import json
import os
import re
import time
import uuid
from datetime import datetime, timezone, timedelta
from pathlib import Path

ROOT = Path(os.environ.get("LOCALAPPDATA", Path.home())) / "Emberfall" / "ArticyBridge"
READ_OPERATIONS = {"status", "list_dialogues", "read_dialogue", "export_story"}
WRITE_OPERATIONS = {"upsert_dialogue", "set_line", "connect", "save", "shutdown_owned"}


class BridgeError(RuntimeError):
    pass


class JobTimeout(BridgeError):
    def __init__(self, job_id: str, state: dict):
        self.job_id, self.state = job_id, state
        super().__init__(f"Job {job_id} timed out. Inspect articy_job_status; do not replay mutations.")


class ArticyBridge:
    def __init__(self, root: Path = ROOT):
        self.root = Path(root)

    def heartbeat(self) -> dict:
        try:
            data = json.loads((self.root / "heartbeat.json").read_text(encoding="utf-8-sig"))
            stamp = datetime.fromisoformat(data["utc"].replace("Z", "+00:00"))
            age = (datetime.now(timezone.utc) - stamp).total_seconds()
            if data.get("bridge") != "Emberfall.DialogueBridge":
                raise ValueError("Wrong bridge identity")
            return {**data, "heartbeatAgeSeconds": round(age, 2), "live": -2 <= age <= 8}
        except (OSError, ValueError, KeyError, TypeError) as error:
            return {"live": False, "projectOpen": False, "reason": str(error),
                    "nextStep": "Activate articy FREE, then start the client with Start-ArticyBridge.ps1."}

    def job_status(self, job_id: str) -> dict:
        if not re.fullmatch(r"[a-f0-9]{32}", job_id):
            raise BridgeError("Invalid job id")
        for folder in ("responses", "processing", "requests"):
            path = self.root / folder / f"{job_id}.json"
            if path.is_file():
                result = {"job_id": job_id, "state": folder}
                if folder == "responses":
                    result["response"] = json.loads(path.read_text(encoding="utf-8-sig"))
                return result
        return {"job_id": job_id, "state": "unknown"}

    def call(self, operation: str, parameters: dict | None = None, timeout: float = 15) -> dict:
        if operation not in READ_OPERATIONS | WRITE_OPERATIONS:
            raise BridgeError("Unsupported operation")
        parameters = dict(parameters or {})
        heartbeat = self.heartbeat()
        if operation == "status" and not heartbeat["live"]:
            return {"heartbeat": heartbeat, "live_query": False}
        if not heartbeat["live"]:
            raise BridgeError("No fresh heartbeat from the real articy plugin; no job was queued")
        if operation != "status" and not heartbeat.get("projectOpen"):
            raise BridgeError("No articy project is open; activate FREE and open an Emberfall project")
        if operation in WRITE_OPERATIONS:
            if parameters.get("project_guid") != heartbeat.get("projectGuid"):
                raise BridgeError("Expected project_guid does not match the active project")
        job_id = uuid.uuid4().hex
        timeout = max(0.1, min(float(timeout), 60))
        job = {"id": job_id, "operation": operation, "parameters": parameters,
               "created_utc": datetime.now(timezone.utc).isoformat(),
               "expires_utc": (datetime.now(timezone.utc) + timedelta(seconds=timeout)).isoformat()}
        raw = json.dumps(job, ensure_ascii=False).encode("utf-8")
        if len(raw) > 1024 * 1024:
            raise BridgeError("Request exceeds 1 MiB")
        requests = self.root / "requests"
        requests.mkdir(parents=True, exist_ok=True)
        temp = requests / f"{job_id}.tmp"
        temp.write_bytes(raw)
        temp.replace(requests / f"{job_id}.json")
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            state = self.job_status(job_id)
            if state["state"] == "responses":
                response = state["response"]
                if response.get("id") != job_id:
                    raise BridgeError("Response identity mismatch")
                if not response.get("ok"):
                    raise BridgeError(f"Job {job_id}: {response.get('message')}. Inspect project before retry.")
                return {"job_id": job_id, **response}
            time.sleep(0.05)
        raise JobTimeout(job_id, self.job_status(job_id))

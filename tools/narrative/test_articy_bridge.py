"""Offline regression tests. Fixtures never impersonate a real Articy connection."""
from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from datetime import datetime, timezone, timedelta
from pathlib import Path
from unittest.mock import patch

from articy_bridge import ArticyBridge, BridgeError, JobTimeout
from articy_mcp import dispatch
from story_sync import ARRAY, ROW, REPO, apply_export, extract, import_plans, sha, sources, stage, validate_export


class TransportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="emberfall-articy-fixture-")
        self.root = Path(self.temp.name)
        self.bridge = ArticyBridge(self.root)
        for name in ("requests", "processing", "responses"):
            (self.root / name).mkdir()

    def tearDown(self):
        # Windows may briefly retain a just-exited test executable (scanner or
        # process teardown). Retry only fixture cleanup, never native mutations.
        for attempt in range(5):
            try:
                self.temp.cleanup()
                return
            except PermissionError:
                if attempt == 4: raise
                time.sleep(.2)

    def heartbeat(self, seconds=0, project=True):
        (self.root / "heartbeat.json").write_text(json.dumps({"bridge": "Emberfall.DialogueBridge",
            "utc": (datetime.now(timezone.utc) - timedelta(seconds=seconds)).isoformat(),
            "projectOpen": project, "projectGuid": "fixture-guid"}), encoding="utf-8")

    def test_missing_heartbeat_does_not_claim_connection(self):
        self.assertFalse(self.bridge.call("status")["live_query"])
        self.assertFalse(self.bridge.heartbeat()["live"])

    def test_stale_heartbeat_is_disconnected(self):
        self.heartbeat(seconds=30)
        self.assertFalse(self.bridge.heartbeat()["live"])

    def test_no_heartbeat_rejects_mutation_without_queue(self):
        with self.assertRaises(BridgeError): self.bridge.call("save", {"project_guid": "fixture-guid"})
        self.assertEqual(list((self.root / "requests").iterdir()), [])

    def test_wrong_project_rejected_before_queue(self):
        self.heartbeat()
        with self.assertRaises(BridgeError): self.bridge.call("save", {"project_guid": "other-project"})
        self.assertEqual(list((self.root / "requests").iterdir()), [])

    def test_no_project_rejected_before_queue(self):
        self.heartbeat(project=False)
        with self.assertRaises(BridgeError): self.bridge.call("export_story")

    def test_bad_operation_rejected(self):
        with self.assertRaises(BridgeError): self.bridge.call("delete_everything")

    def test_path_traversal_job_rejected(self):
        with self.assertRaises(BridgeError): self.bridge.job_status("../heartbeat")

    def test_round_trip_fixture_and_utf8(self):
        self.heartbeat()
        received = []
        def worker():
            limit = time.monotonic() + 3
            while time.monotonic() < limit:
                files = list((self.root / "requests").glob("*.json"))
                if files:
                    job = json.loads(files[0].read_text(encoding="utf-8"))
                    received.append(job)
                    target = self.root / "responses" / files[0].name
                    temp = target.with_suffix(".tmp")
                    temp.write_text(json.dumps({"id": job["id"], "ok": True, "result": {"text": "Pieczęć Świtu"}}), encoding="utf-8")
                    temp.replace(target)
                    return
                time.sleep(.01)
        thread = threading.Thread(target=worker)
        thread.start()
        result = self.bridge.call("read_dialogue", {"dialogue": "EF_IntroDialogue"}, timeout=3)
        thread.join()
        self.assertEqual(result["result"]["text"], "Pieczęć Świtu")
        self.assertIn("expires_utc", received[0])
        self.assertEqual(received[0]["operation"], "read_dialogue")

    def test_timeout_retains_job_id_and_never_replays(self):
        self.heartbeat()
        with self.assertRaises(JobTimeout) as caught:
            self.bridge.call("save", {"project_guid": "fixture-guid"}, timeout=.1)
        job_id = caught.exception.job_id
        self.assertEqual(self.bridge.job_status(job_id)["state"], "requests")
        self.assertEqual(len(list((self.root / "requests").glob("*.json"))), 1)

    def test_size_limit_before_queue(self):
        self.heartbeat()
        with self.assertRaises(BridgeError): self.bridge.call("upsert_dialogue", {"project_guid": "fixture-guid", "blob": "a" * (1024 * 1024)})
        self.assertEqual(list((self.root / "requests").iterdir()), [])

    def test_rpc_unknown_method(self):
        self.assertEqual(dispatch({"id": 1, "method": "unknown"}, self.bridge)["error"]["code"], -32601)

    def test_rpc_notification_has_no_reply(self):
        self.assertIsNone(dispatch({"method": "notifications/initialized"}, self.bridge))

    def test_rpc_invalid_tool_arguments(self):
        response = dispatch({"id": 1, "method": "tools/call", "params": {"name": "articy_save", "arguments": {"unexpected": 5}}}, self.bridge)
        self.assertTrue(response["result"]["isError"])

    def test_real_stdio_protocol(self):
        requests = [{"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {"protocolVersion": "2025-03-26"}},
                    {"jsonrpc": "2.0", "method": "notifications/initialized"},
                    {"jsonrpc": "2.0", "id": 2, "method": "tools/list"},
                    {"jsonrpc": "2.0", "id": 3, "method": "ping"}]
        result = subprocess.run([sys.executable, str(Path(__file__).with_name("articy_mcp.py"))],
            input="\n".join(json.dumps(r) for r in requests) + "\n", capture_output=True,
            text=True, encoding="utf-8", timeout=5, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        self.assertEqual(result.returncode, 0, result.stderr)
        responses = [json.loads(line) for line in result.stdout.splitlines()]
        self.assertEqual([r["id"] for r in responses], [1, 2, 3])
        self.assertEqual(len(responses[1]["result"]["tools"]), 9)
        self.assertEqual(responses[0]["result"]["protocolVersion"], "2025-03-26")


class StoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="emberfall-story-fixture-")
        self.root = Path(self.temp.name)
        self.plans = sources()
        self.data = {"schema": 1, "producer": "articy-mdk-live", "projectGuid": "SIMULATED-TEST-ONLY", "dialogues": []}
        for p in self.plans:
            lines = [{"id": f"{p['id']}_L{i:04d}", **row,
                      "next": [f"{p['id']}_L{i+1:04d}"] if i+1 < len(p["lines"]) else []} for i, row in enumerate(p["lines"])]
            self.data["dialogues"].append({"id": p["id"], "lines": lines})

    def tearDown(self):
        for attempt in range(5):
            try:
                self.temp.cleanup()
                return
            except PermissionError:
                if attempt == 4: raise
                time.sleep(.2)

    def test_all_actual_lines_extracted(self):
        self.assertEqual(len(self.plans), 7)
        self.assertEqual(sum(len(p["lines"]) for p in self.plans), 41)
        self.assertIn("Pieczęć", self.plans[0]["lines"][2]["text"])

    def test_source_snapshot_label_and_no_overwrite(self):
        directory = self.root / "plans"
        manifest = extract(directory)
        self.assertFalse(manifest["native_articy_imported"])
        self.assertEqual(len(list(directory.glob("EF_*.json"))), 7)
        with self.assertRaises(FileExistsError): extract(directory)

    def test_source_snapshot_not_accepted_as_native_export(self):
        with self.assertRaises(ValueError): validate_export({"schema": 1, "producer": "existing-game-source-snapshot"})

    def test_incomplete_batch_rejected_before_connecting(self):
        with patch("story_sync.ArticyBridge", side_effect=AssertionError("Must not connect")):
            with self.assertRaises(ValueError): import_plans(self.root / "missing-plans")

    def test_stale_batch_rejected_before_connecting(self):
        destination = self.root / "plans"
        extract(destination)
        path = destination / "EF_IntroDialogue.json"
        data = json.loads(path.read_text(encoding="utf-8"))
        data["source_sha256"] = "outdated-source"
        path.write_text(json.dumps(data), encoding="utf-8")
        with patch("story_sync.ArticyBridge", side_effect=AssertionError("Must not connect")):
            with self.assertRaises(ValueError): import_plans(destination)

    def test_invalid_batch_speaker_rejected_before_connecting(self):
        destination = self.root / "plans"
        extract(destination)
        path = destination / "EF_IntroDialogue.json"
        data = json.loads(path.read_text(encoding="utf-8"))
        data["lines"][0]["speaker"] = "Unsupported"
        path.write_text(json.dumps(data), encoding="utf-8")
        with patch("story_sync.ArticyBridge", side_effect=AssertionError("Must not connect")):
            with self.assertRaises(ValueError): import_plans(destination)

    def test_branches_not_silently_discarded(self):
        self.data["dialogues"][0]["lines"][0]["next"].append("EF_unrelated")
        with self.assertRaises(ValueError): validate_export(self.data)

    def test_cycle_not_accepted(self):
        self.data["dialogues"][0]["lines"][-1]["next"] = [self.data["dialogues"][0]["lines"][0]["id"]]
        with self.assertRaises(ValueError): validate_export(self.data)

    def test_unknown_speaker_not_remapped(self):
        self.data["dialogues"][0]["lines"][0]["speaker"] = "Unknown"
        with self.assertRaises(ValueError): validate_export(self.data)

    def test_missing_dialogue_not_accepted(self):
        self.data["dialogues"].pop()
        with self.assertRaises(ValueError): validate_export(self.data)

    def test_staging_preserves_sources_and_exact_utf8(self):
        before = {p["source"]: sha(REPO / p["source"]) for p in self.plans}
        destination = self.root / "stage"
        receipt = stage(self.data, destination)
        self.assertFalse(receipt["production_sources_modified"])
        for p in self.plans:
            text = (destination / Path(p["source"]).name).read_text(encoding="utf-8")
            match = next(m for m in ARRAY.finditer(text) if m[2] == p["array"])
            actual = [{"speaker": "Eldrin" if s == "true" else "Wanderer", "text": json.loads(t)} for s, t in ROW.findall(match[3])]
            self.assertEqual(actual, p["lines"])
            self.assertEqual(before[p["source"]], sha(REPO / p["source"]))
        with self.assertRaises(FileExistsError): stage(self.data, destination)

    def fixture_repo(self):
        repo = self.root / "repo"
        (repo / "src").mkdir(parents=True)
        for filename in ("intro_story.hpp", "tower_story.hpp"):
            text = (REPO / "src" / filename).read_text(encoding="utf-8-sig").replace("\n", "\r\n")
            (repo / "src" / filename).write_bytes(b"\xef\xbb\xbf" + text.encode("utf-8"))
        return repo

    def test_identical_staging_preserves_crlf_and_bom_bytes(self):
        repo = self.fixture_repo()
        destination = self.root / "exact-bytes"
        stage(self.data, destination, repo)
        for filename in ("intro_story.hpp", "tower_story.hpp"):
            self.assertEqual((repo / "src" / filename).read_bytes(), (destination / filename).read_bytes())

    def test_apply_native_change_compiles_and_keeps_backup(self):
        repo = self.fixture_repo()
        before = (repo / "src/intro_story.hpp").read_bytes()
        self.data["dialogues"][0]["lines"][0]["text"] = "Świt — próbny tekst z natywnego eksportu."
        receipt = apply_export(self.data, self.root / "apply", repo, "SIMULATED-TEST-ONLY")
        self.assertEqual(receipt["changed_files"], ["intro_story.hpp"])
        self.assertEqual((Path(receipt["source_backups"]) / "intro_story.hpp").read_bytes(), before)
        actual = sources(repo)[0]["lines"][0]["text"]
        self.assertEqual(actual, self.data["dialogues"][0]["lines"][0]["text"])
        self.assertTrue((repo / "src/intro_story.hpp").read_bytes().startswith(b"\xef\xbb\xbf"))

    def test_apply_wrong_project_rejected_before_any_write(self):
        with self.assertRaises(ValueError): apply_export(self.data, self.root / "wrong-project")
        self.assertFalse((self.root / "wrong-project").exists())

    def test_import_identical_dialogues_skips_all_mutations(self):
        directory = self.root / "plans"
        extract(directory)
        native = [{**p, "revision": "fixture", "lines": p["lines"]} for p in self.plans]
        class ReadOnlyBridge:
            def call(self, operation, *args):
                if operation == "status": return {"result": {"projectOpen": True, "projectGuid": "fixture"}}
                if operation == "list_dialogues": return {"result": native}
                if operation == "export_story": return {"result": {"fixture": True}}
                raise AssertionError("Existing dialogue must not be rewritten")
        with patch("story_sync.ArticyBridge", ReadOnlyBridge):
            result = import_plans(directory)
        self.assertEqual(result["jobs"], [])
        self.assertEqual(len(result["unchanged_skipped"]), 7)

    def test_quotes_newlines_backslashes_are_escaped(self):
        self.data["dialogues"][0]["lines"][0]["text"] = 'Świt: "cześć"\\droga\nDalej.'
        destination = self.root / "escaped"
        stage(self.data, destination)
        text = (destination / "intro_story.hpp").read_text(encoding="utf-8")
        self.assertEqual(json.loads(ROW.findall(next(ARRAY.finditer(text))[3])[0][1]), self.data["dialogues"][0]["lines"][0]["text"])

    def test_control_characters_rejected(self):
        self.data["dialogues"][0]["lines"][0]["text"] = "bad\x1btext"
        with self.assertRaises(ValueError): validate_export(self.data)

    def test_staged_cpp_compiles_and_runs_hidden_silent(self):
        compiler = REPO / "tools/w64devkit/bin/g++.exe"
        if not compiler.is_file():
            self.skipTest("Project compiler unavailable")
        headers = self.root / "headers"
        stage(self.data, headers)
        source, exe = self.root / "validate.cpp", self.root / "validate.exe"
        source.write_text('#include "tower_story.hpp"\nstatic_assert(ember::IntroDialogue.size()==12);\nstatic_assert(ember::VolcanoTalkDialogue.size()==3);\nint main(){return ember::storyGlyphCount(ember::IntroDialogue[0].text)>0?0:1;}\n', encoding="utf-8")
        env = {**os.environ, "PATH": str(compiler.parent) + os.pathsep + os.environ.get("PATH", "")}
        compiled = subprocess.run([str(compiler), "-std=c++17", "-finput-charset=UTF-8", "-fexec-charset=UTF-8",
            "-I" + str(headers), str(source), "-o", str(exe)], env=env, capture_output=True, text=True,
            timeout=45, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        self.assertEqual(compiled.returncode, 0, compiled.stderr)
        ran = subprocess.run([str(exe), "--no-audio", "--background-test"], capture_output=True, timeout=5,
                             creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        self.assertEqual(ran.returncode, 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)

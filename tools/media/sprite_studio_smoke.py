"""Offline regression for the locally patched Sprite Studio native MCP.

Uses an isolated database/workspace and the existing Emberfall trial master.
Never launches the game, calls an AI provider, or plays media.
"""
import argparse
import copy
import datetime
import hashlib
import json
import os
from pathlib import Path
import queue
import shutil
import subprocess
import threading
import time

from PIL import Image


def read(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


class NativeMcp:
    def __init__(self, exe, root):
        self.root = root
        self.logs = []
        self.counter = 0
        self.responses = queue.Queue()
        self.stderr = (root / "native-stderr.log").open("w", encoding="utf-8")
        env = os.environ.copy()
        env["APPDATA"] = str(root / "isolated-appdata")
        self.process = subprocess.Popen(
            [str(exe)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=self.stderr, text=True, encoding="utf-8", env=env,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        threading.Thread(target=self._reader, daemon=True).start()
        try:
            self.request("initialize", {
                "protocolVersion": "2024-11-05", "capabilities": {},
                "clientInfo": {"name": "Emberfall offline render regression", "version": "1"},
            })
            self.process.stdin.write(json.dumps({
                "jsonrpc": "2.0", "method": "notifications/initialized"
            }) + "\n")
            self.process.stdin.flush()
        except Exception:
            self.close()
            raise

    def _reader(self):
        for line in self.process.stdout:
            try:
                self.responses.put(json.loads(line))
            except ValueError:
                self.responses.put({"invalid_stdout": line})
        self.responses.put({"eof": True})

    def request(self, method, params):
        self.counter += 1
        request = {"jsonrpc": "2.0", "id": self.counter, "method": method, "params": params}
        self.logs.append({"request": request})
        self._save_log()
        self.process.stdin.write(json.dumps(request) + "\n")
        self.process.stdin.flush()
        deadline = time.monotonic() + 90
        while time.monotonic() < deadline:
            try:
                response = self.responses.get(timeout=max(.1, deadline - time.monotonic()))
            except queue.Empty as error:
                raise TimeoutError(method) from error
            if "eof" in response or "invalid_stdout" in response:
                raise RuntimeError(f"Native MCP ended or corrupted protocol: {response}")
            if response.get("id") == self.counter:
                self.logs.append({"response": response})
                self._save_log()
                if "error" in response:
                    raise RuntimeError(response["error"])
                return response["result"]
        raise TimeoutError(method)

    def _save_log(self):
        (self.root / "native-protocol.json").write_text(
            json.dumps(self.logs, indent=2), encoding="utf-8")

    def tool(self, name, arguments, allow_error=False):
        response = self.request("tools/call", {"name": name, "arguments": arguments})
        value = json.loads("\n".join(
            item["text"] for item in response.get("content", []) if item.get("type") == "text"))
        if not allow_error and (response.get("isError") or isinstance(value, dict) and "error" in value):
            raise RuntimeError(value)
        return value

    def close(self):
        try:
            self.process.stdin.close()
        except BrokenPipeError:
            pass
        try:
            self.process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            # Only this owned, isolated worker may be terminated.
            self.process.terminate()
            self.process.wait(timeout=10)
        self.stderr.close()


def run(args):
    root = Path(args.output).resolve()
    root.mkdir(parents=True, exist_ok=False)
    reference = Path(args.reference_root).resolve()
    original_hash = digest(reference / "qa/hero-master.png")
    native = None
    try:
        native = NativeMcp(Path(args.exe).resolve(), root)
        tools = native.request("tools/list", {})["tools"]
        assert "render_rig_animation" in {tool["name"] for tool in tools}
        workspace = native.tool("open_workspace", {
            "path": str(root / "workspace"), "name": "Emberfall native runtime fix regression"
        })
        work = root / "workspace"
        target = work / "assets/characters/hero-master.png"
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(reference / "qa/hero-master.png", target)
        (work / ".sprite-studio/last-generation.json").write_text(json.dumps({
            "kind": "import", "name": "Existing Emberfall hero reference", "category": "characters",
            "fps": 12, "files": ["assets/characters/hero-master.png"], "source": None,
            "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        }), encoding="utf-8")
        imported = native.tool("list_artifacts", {"workspaceId": workspace["id"]})
        asset = next(item for item in imported if item["relativePath"].endswith("hero-master.png"))
        rig = read(reference / "sprite-trial/rig-corrected.json")
        rig.update(workspaceId=workspace["id"], assetId=asset["id"], name="Emberfall fixed runtime walk")
        rig.pop("id", None)
        saved = native.tool("save_rig", rig)
        rig["id"] = saved["id"]
        (root / "rig-input.json").write_text(json.dumps(rig, indent=2), encoding="utf-8")
        start = time.perf_counter()
        rendered = native.tool("render_rig_animation", rig)
        elapsed = time.perf_counter() - start
        assert len(rendered["framePaths"]) == len(rig["frames"]) == 12
        assert len(rendered["animation"]["frames"]) == 12
        pixels, frames = [], []
        for relative in rendered["framePaths"]:
            path = work / relative
            with Image.open(path) as image:
                rgba = image.convert("RGBA")
                assert rgba.size == (320, 360)
                assert rgba.getchannel("A").getextrema() == (0, 255)
                assert rgba.getbbox() is not None
                pixels.append(hashlib.sha256(rgba.tobytes()).hexdigest())
            frames.append({"path": relative, "sha256": digest(path)})
        assert len(set(pixels)) >= 8, "Native animation must visibly change between poses"
        # An input error must be a structured response, without killing the server
        # or producing a partial animation. Follow it with another real render.
        before = set((work / "assets/characters").glob("*.png"))
        invalid = dict(rig, name="must not create frames", fps=0)
        error = native.tool("render_rig_animation", invalid, allow_error=True)
        assert error.get("error") == "invalid_fps", error
        assert before == set((work / "assets/characters").glob("*.png"))
        repeated = copy.deepcopy(rig)
        repeated.pop("id", None)
        repeated["name"] = "Emberfall fixed runtime repeated"
        rerendered = native.tool("render_rig_animation", repeated)
        assert [digest(work / path) for path in rerendered["framePaths"]] == [item["sha256"] for item in frames]
        native.tool("set_animation_review_status", {
            "animationId": rendered["animation"]["id"], "status": "accepted"
        })
        exported = native.tool("export", {"kind": "animation", "id": rendered["animation"]["id"]})
        export_png = Path(exported["pngPath"])
        # Rust may return a Win32 extended path with doubled JSON backslashes.
        with Image.open(export_png) as image:
            image.load()
            assert image.size == (320 * 12, 360)
        metadata = read(exported["metadataPath"])
        assert metadata
        assert digest(reference / "qa/hero-master.png") == original_hash
        native.close()
        exit_code = native.process.returncode
        native = None
        stderr = (root / "native-stderr.log").read_text(encoding="utf-8")
        assert "panicked" not in stderr and "Cannot start a runtime" not in stderr
        assert exit_code == 0, exit_code
        report = {
            "status": "PASS", "exe": str(Path(args.exe).resolve()), "exe_sha256": digest(args.exe),
            "native_tool_count": len(tools), "frames": frames, "unique_decoded_poses": len(set(pixels)),
            "frame_size": [320, 360], "render_seconds": elapsed,
            "render_repeated_byte_identical": True, "invalid_input_recovered": True,
            "export": exported, "worker_exit_code": exit_code, "native_panic": False,
            "database_isolated": True, "window_created": False, "cloud_calls": 0,
            "audio_played": False, "production_art_replaced": False,
            "quality_scope": "Runtime regression only; trial is not approved production animation."
        }
        (root / "report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        print(f"PASS: 12 native PNG poses, deterministic repeat, input error recovery, sprite-sheet export; render {elapsed:.3f}s")
    except Exception as error:
        (root / "failure.json").write_text(json.dumps({"status": "FAIL", "error": str(error)}, indent=2), encoding="utf-8")
        raise
    finally:
        if native is not None:
            native.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--no-audio", action="store_true", required=True)
    parser.add_argument("--exe", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--reference-root", default="C:/AI2D/pipeline/runs/emberfall-loot-foley-20261111")
    run(parser.parse_args())

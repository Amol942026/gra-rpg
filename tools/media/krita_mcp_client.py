"""Local STDIO MCP client for the already installed, pinned Krita bridge.

No server is downloaded. Mutating tool calls are never automatically retried.
Use from the real Windows desktop environment (outside the filesystem sandbox).
"""
from __future__ import annotations
import base64, json, os, queue, shutil, subprocess, threading, time
from pathlib import Path


class KritaMCP:
    def __init__(self, log_directory: Path, timeout=210):
        self.directory = Path(log_directory)
        self.directory.mkdir(parents=True, exist_ok=True)
        self.timeout = timeout
        self.sequence = 0
        self.messages = queue.Queue()
        self.process = None

    def __enter__(self):
        uvx = shutil.which("uvx")
        if not uvx:
            raise RuntimeError("uvx is missing; use the existing Krita MCP environment.")
        self.stderr = (self.directory / "mcp-stderr.log").open("ab")
        self.process = subprocess.Popen(
            [uvx, "--offline", "--from", "krita-codex-mcp==0.2.0", "krita-codex-mcp"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self.stderr,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        def reader():
            try:
                for line in self.process.stdout:
                    self.messages.put(json.loads(line))
            except BaseException as error:
                self.messages.put(error)
            finally:
                self.messages.put(EOFError("Krita MCP STDIO closed"))
        threading.Thread(target=reader, daemon=True).start()
        self.request("initialize", {"protocolVersion":"2024-11-05", "capabilities":{},
                     "clientInfo":{"name":"emberfall-media-pipeline", "version":"1.0"}})
        self.send({"jsonrpc":"2.0", "method":"notifications/initialized"})
        tools = self.request("tools/list", {})
        (self.directory / "mcp-tools.json").write_text(json.dumps(tools, indent=2), encoding="utf-8")
        self.tools = {t["name"]:t for t in tools["tools"]}
        return self

    def send(self, value):
        self.process.stdin.write((json.dumps(value, ensure_ascii=False)+"\n").encode("utf-8"))
        self.process.stdin.flush()

    def request(self, method, params):
        self.sequence += 1
        self.send({"jsonrpc":"2.0", "id":self.sequence, "method":method, "params":params})
        deadline = time.monotonic()+self.timeout
        while True:
            try:
                item = self.messages.get(timeout=max(.001, deadline-time.monotonic()))
            except queue.Empty:
                raise TimeoutError(f"MCP {method} timed out; inspect state before repeating mutations.")
            if isinstance(item, BaseException):
                raise item
            if item.get("id") != self.sequence:
                continue
            if "error" in item:
                raise RuntimeError(json.dumps(item["error"]))
            return item["result"]

    def call(self, tool_name, **arguments):
        if tool_name not in self.tools:
            raise ValueError("Unavailable Krita MCP tool: "+tool_name)
        result = self.request("tools/call", {"name":tool_name, "arguments":arguments})
        metadata = result.get("structuredContent")
        blocks = result.get("content", [])
        # FastMCP wraps list-returning tools in structuredContent.result.
        if isinstance(metadata, dict) and isinstance(metadata.get("result"), list):
            blocks = metadata["result"]
            metadata = None
        images = []
        texts = []
        for block in blocks:
            if block["type"] == "image":
                path = self.directory / f"{self.sequence:04d}-{tool_name}.png"
                with path.open("xb") as stream:
                    stream.write(base64.b64decode(block["data"], validate=True))
                images.append(str(path))
            elif block["type"] == "text":
                texts.append(block["text"])
                if metadata is None:
                    try:
                        metadata = json.loads(block["text"])
                    except json.JSONDecodeError:
                        pass
        record = {"tool":tool_name, "arguments":arguments, "metadata":metadata,
                  "images":images, "isError":result.get("isError", False)}
        with (self.directory / "mcp-calls.jsonl").open("a", encoding="utf-8") as stream:
            stream.write(json.dumps(record, ensure_ascii=False)+"\n")
        if result.get("isError"):
            raise RuntimeError("Krita MCP: "+"\n".join(texts))
        return metadata if metadata is not None else {"texts":texts,"images":images}

    def __exit__(self, *args):
        if self.process:
            self.process.stdin.close()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.terminate()
                self.process.wait(timeout=5)
        self.stderr.close()


def start_krita_if_needed():
    """Launch only when the existing loopback bridge is not reachable."""
    import socket
    config_path = Path(os.environ["LOCALAPPDATA"])/"krita-codex-mcp/config.json"
    config = json.loads(config_path.read_text(encoding="utf-8"))
    if config.get("host", "127.0.0.1") != "127.0.0.1":
        raise RuntimeError("Krita bridge must stay on loopback.")
    def available():
        try:
            with socket.create_connection(("127.0.0.1", config.get("port",17641)), timeout=.25):
                return True
        except OSError:
            return False
    if not available():
        # A running Krita without its bridge needs diagnosis, not another copy.
        tasks = subprocess.run(["tasklist","/FI","IMAGENAME eq krita.exe","/FO","CSV","/NH"],
                               capture_output=True, creationflags=subprocess.CREATE_NO_WINDOW).stdout.lower()
        if b'"krita.exe"' in tasks:
            raise RuntimeError("Krita is running but its configured bridge is unavailable.")
        env = os.environ.copy()
        env["QT_QPA_PLATFORM"] = "windows"
        env["QT_OPENGL"] = "software"
        for name in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "PYTHONHOME", "PYTHONPATH"):
            env.pop(name, None)
        startup = subprocess.STARTUPINFO()
        startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        startup.wShowWindow = 0
        subprocess.Popen([r"C:\Program Files\Krita (x64)\bin\krita.exe"], env=env,
                         cwd=r"C:\AI2D", stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL,
                         stderr=subprocess.DEVNULL, startupinfo=startup,
                         creationflags=subprocess.DETACHED_PROCESS|subprocess.CREATE_NEW_PROCESS_GROUP|0x01000000)
        end = time.monotonic()+45
        while not available():
            if time.monotonic() >= end:
                raise TimeoutError("Krita did not expose its configured bridge within 45 seconds.")
            time.sleep(.25)
    # Do not return or log the bridge secret.
    return {"host":"127.0.0.1", "port":config.get("port",17641),
            "allowed_roots":config["allowed_roots"]}


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log-directory", type=Path, required=True)
    parser.add_argument("--tool", default="krita_state")
    parser.add_argument("--arguments", default="{}")
    args = parser.parse_args()
    start_krita_if_needed()
    with KritaMCP(args.log_directory) as client:
        print(json.dumps(client.call(args.tool, **json.loads(args.arguments)), indent=2))

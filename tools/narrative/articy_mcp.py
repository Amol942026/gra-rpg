"""MCP JSON-RPC/STDIO server. Standard library only, strictly local Articy MDK."""
from __future__ import annotations

import json
import sys
from articy_bridge import ArticyBridge, BridgeError, JobTimeout


def tool(name, description, properties=None, required=(), read_only=False):
    return {"name": name, "description": description,
            "inputSchema": {"type": "object", "properties": properties or {},
                            "required": list(required), "additionalProperties": False},
            "annotations": {"readOnlyHint": read_only, "destructiveHint": not read_only,
                            "idempotentHint": read_only, "openWorldHint": False}}


S = {"type": "string"}
GUID = {"project_guid": {"type": "string", "description": "Exact active projectGuid from articy_status"}}
TOOLS = [
    tool("articy_status", "Check a real fresh MDK heartbeat and query the running articy client. Unavailable means NOT connected.", read_only=True),
    tool("articy_list_dialogues", "List EF_ dialogues from the real open articy project.", read_only=True),
    tool("articy_read_dialogue", "Read native lines, revision and connections.", {"dialogue": S}, ["dialogue"], True),
    tool("articy_export_story", "Export native EF_ dialogue data for validation/staged C++ generation.", read_only=True),
    tool("articy_upsert_dialogue", "Create/update a linear Emberfall dialogue. Existing dialogues require expected_revision. Native FREE limits remain in force.",
         {**GUID, "dialogue": {"type": "object", "properties": {"id": S, "title": S,
          "lines": {"type": "array", "minItems": 1, "maxItems": 120,
                    "items": {"type": "object", "properties": {"speaker": {"enum": ["Eldrin", "Wanderer"]}, "text": S}, "required": ["speaker", "text"], "additionalProperties": False}}},
          "required": ["id", "title", "lines"], "additionalProperties": False}, "expected_revision": S}, ["project_guid", "dialogue"]),
    tool("articy_set_line", "Edit one EF_ line with compare-and-set to preserve manual edits.",
         {**GUID, "line": S, "expected_text": S, "text": S}, ["project_guid", "line", "expected_text", "text"]),
    tool("articy_connect", "Connect two EF_ fragments in the same native dialogue. Branches are rejected by the current linear game exporter.",
         {**GUID, "from": S, "to": S}, ["project_guid", "from", "to"]),
    tool("articy_save", "Save the current explicitly identified Emberfall project.", GUID, ["project_guid"]),
    tool("articy_job_status", "Inspect a timed-out job by id. Never blindly replay a mutating job.", {"job_id": S}, ["job_id"], True),
]
BY_NAME = {t["name"]: t for t in TOOLS}


def dispatch(request, bridge):
    method, params = request.get("method"), request.get("params", {})
    if "id" not in request:
        return None
    rid = request["id"]
    if method == "initialize":
        version = params.get("protocolVersion", "2024-11-05")
        if version not in {"2024-11-05", "2025-03-26", "2025-06-18", "2025-11-25"}:
            version = "2024-11-05"
        result = {"protocolVersion": version, "capabilities": {"tools": {"listChanged": False}},
                  "serverInfo": {"name": "emberfall-articy-local", "version": "1.0.0"},
                  "instructions": "Local free MDK only. Start with articy_status. Preserve license limits. No automatic mutation retries."}
    elif method == "ping":
        result = {}
    elif method == "tools/list":
        result = {"tools": TOOLS}
    elif method == "tools/call":
        try:
            name, arguments = params["name"], params.get("arguments", {})
            schema = BY_NAME[name]["inputSchema"]
            if not isinstance(arguments, dict) or set(arguments) - set(schema["properties"]) or set(schema["required"]) - set(arguments):
                raise BridgeError("Invalid tool arguments")
            if name == "articy_job_status":
                value = bridge.job_status(arguments["job_id"])
            else:
                value = bridge.call(name.removeprefix("articy_"), arguments)
            result = {"content": [{"type": "text", "text": json.dumps(value, ensure_ascii=False)}], "isError": False}
        except (BridgeError, KeyError, TypeError, ValueError, OSError) as error:
            value = {"error": str(error)}
            if isinstance(error, JobTimeout):
                value.update(job_id=error.job_id, state=error.state, retry_mutation=False)
            result = {"content": [{"type": "text", "text": json.dumps(value, ensure_ascii=False)}], "isError": True}
    else:
        return {"jsonrpc": "2.0", "id": rid, "error": {"code": -32601, "message": "Method not found"}}
    return {"jsonrpc": "2.0", "id": rid, "result": result}


def main():
    sys.stdin.reconfigure(encoding="utf-8")
    sys.stdout.reconfigure(encoding="utf-8", newline="\n")
    bridge = ArticyBridge()
    for raw in sys.stdin:
        try:
            request = json.loads(raw)
            if not isinstance(request, dict):
                raise ValueError("JSON-RPC object expected")
            response = dispatch(request, bridge)
        except (ValueError, KeyError, TypeError) as error:
            response = {"jsonrpc": "2.0", "id": None, "error": {"code": -32700, "message": str(error)}}
        if response is not None:
            print(json.dumps(response, ensure_ascii=False), flush=True)


if __name__ == "__main__":
    main()

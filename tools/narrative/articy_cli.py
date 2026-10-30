"""Same local transport as the MCP server; useful before Codex reloads its tools."""
import argparse
import json
from pathlib import Path
from articy_bridge import ArticyBridge, BridgeError


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("operation", choices=["status", "list_dialogues", "read_dialogue", "export_story", "upsert_dialogue", "set_line", "connect", "save", "shutdown_owned", "job_status"])
    parser.add_argument("--parameters", type=Path, help="UTF-8 JSON parameters (no license keys)")
    parser.add_argument("--output", type=Path, help="Fresh output JSON path; existing files rejected")
    args = parser.parse_args()
    if args.output and args.output.exists():
        parser.error("Output exists")
    parameters = json.loads(args.parameters.read_text(encoding="utf-8-sig")) if args.parameters else {}
    bridge = ArticyBridge()
    try:
        result = bridge.job_status(parameters["job_id"]) if args.operation == "job_status" else bridge.call(args.operation, parameters)
    except (BridgeError, KeyError) as error:
        parser.exit(2, str(error) + "\n")
    text = json.dumps(result, ensure_ascii=False, indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("x", encoding="utf-8") as output:
            output.write(text + "\n")
    print(text)


if __name__ == "__main__":
    main()

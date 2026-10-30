"""Extract current dialogue plans; import via real MDK; stage validated C++ exports.

No command replaces the live game's sources automatically.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import uuid
from pathlib import Path
from articy_bridge import ArticyBridge

REPO = Path(__file__).resolve().parents[2]
ARRAY = re.compile(r"inline constexpr std::array<StoryLine,(\d+)>\s+(\w+)\s*=\s*(\{.*?\});", re.S)
ROW = re.compile(r'\{\s*(true|false)\s*,\s*("(?:[^"\\]|\\.)*")\s*\}', re.S)
TITLES = {
    "IntroDialogue": "Prolog — Pieczęć Popiołu",
    "TowerReturnDialogue": "Powrót z Lasu Mchu",
    "TowerTalkDialogue": "Eldrin — droga do Morza Piasków",
    "DesertReturnDialogue": "Powrót z Morza Piasków",
    "DesertCompletedTalk": "Eldrin — powrót do oazy",
    "VolcanoReturnDialogue": "Powrót z Kaldery Popiołu",
    "VolcanoTalkDialogue": "Eldrin — droga do Kaldery Popiołu",
}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def sources(repo=REPO):
    result = []
    for filename in ("intro_story.hpp", "tower_story.hpp"):
        path = repo / "src" / filename
        text = path.read_text(encoding="utf-8-sig")
        for match in ARRAY.finditer(text):
            count, name, body = match.groups()
            if name not in TITLES:
                raise ValueError(f"Unmapped story array {name}")
            rows = [{"speaker": "Eldrin" if speaker == "true" else "Wanderer", "text": json.loads(value)}
                    for speaker, value in ROW.findall(body)]
            if len(rows) != int(count):
                raise ValueError(f"Incomplete parse for {name}")
            result.append({"id": "EF_" + name, "title": TITLES[name], "array": name,
                           "source": "src/" + filename, "source_sha256": sha(path), "lines": rows})
    if {p["array"] for p in result} != set(TITLES):
        raise ValueError("Expected seven dialogue arrays")
    return result


def write_new(path, data):
    with Path(path).open("x", encoding="utf-8", newline="\n") as output:
        output.write(json.dumps(data, ensure_ascii=False, indent=2) + "\n")


def extract(destination, repo=REPO):
    plans = sources(repo)
    Path(destination).mkdir(parents=True, exist_ok=False)
    for plan in plans:
        write_new(Path(destination) / f"{plan['id']}.json", plan)
    manifest = {"schema": 1, "producer": "existing-game-source-snapshot", "native_articy_imported": False,
                "dialogues": [{k: v for k, v in p.items() if k != "lines"} | {"line_count": len(p["lines"])} for p in plans],
                "total_lines": sum(len(p["lines"]) for p in plans)}
    write_new(Path(destination) / "manifest.json", manifest)
    return manifest


def validate_export(data, repo=REPO):
    if data.get("schema") != 1 or data.get("producer") != "articy-mdk-live":
        raise ValueError("Expected native MDK export; source snapshots are not native exports")
    expected = {p["id"]: p for p in sources(repo)}
    dialogues = data.get("dialogues", [])
    if len(dialogues) != len(expected) or {d["id"] for d in dialogues} != set(expected):
        raise ValueError("Export must contain exactly the seven mapped dialogues")
    for dialogue in dialogues:
        rows = dialogue["lines"]
        if not 1 <= len(rows) <= 120:
            raise ValueError("Invalid dialogue size")
        for index, row in enumerate(rows):
            if row["id"] != f"{dialogue['id']}_L{index:04d}":
                raise ValueError("Unexpected or unordered native node id")
            wanted = [f"{dialogue['id']}_L{index+1:04d}"] if index + 1 < len(rows) else []
            if row.get("next") != wanted:
                raise ValueError("Game currently supports linear dialogue; resolve branches/cycles/missing links explicitly")
            if row["speaker"] not in ("Eldrin", "Wanderer"):
                raise ValueError("Unknown speaker")
            if not isinstance(row["text"], str) or not 1 <= len(row["text"]) <= 8000 or any(ord(c) < 32 and c not in "\n\r\t" for c in row["text"]):
                raise ValueError("Invalid line text")
    return {d["id"]: d for d in dialogues}


def stage(data, destination, repo=REPO):
    dialogues = validate_export(data, repo)
    destination = Path(destination)
    destination.mkdir(parents=True, exist_ok=False)
    receipt = {"schema": 1, "action": "staged-only", "production_sources_modified": False,
               "projectGuid": data["projectGuid"], "files": []}
    for filename in ("intro_story.hpp", "tower_story.hpp"):
        source = repo / "src" / filename
        original_bytes = source.read_bytes()
        source_text = original_bytes.decode("utf-8-sig")
        newline = "\r\n" if "\r\n" in source_text else "\n"
        def replace(match):
            name = match[2]
            rows = dialogues["EF_" + name]["lines"]
            current = [{"speaker": "Eldrin" if s == "true" else "Wanderer", "text": json.loads(t)} for s, t in ROW.findall(match[3])]
            if current == [{"speaker": r["speaker"], "text": r["text"]} for r in rows]:
                return match[0]
            lines = ["    {" + ("true" if r["speaker"] == "Eldrin" else "false") + "," + json.dumps(r["text"], ensure_ascii=False) + "}" for r in rows]
            return f"inline constexpr std::array<StoryLine,{len(rows)}> {name}=" + "{{" + newline + ("," + newline).join(lines) + newline + "}};"
        output = destination / filename
        rendered = ARRAY.sub(replace, source_text).encode("utf-8")
        if original_bytes.startswith(b"\xef\xbb\xbf"):
            rendered = b"\xef\xbb\xbf" + rendered
        output.write_bytes(rendered)
        receipt["files"].append({"path": filename, "original_sha256": sha(source), "staged_sha256": sha(output)})
    write_new(destination / "receipt.json", receipt)
    return receipt


def apply_export(data, destination, repo=REPO, expected_guid="cb99e03a-ae90-4443-b79f-7f8da1539c34"):
    """Compile a staged native export, back up, then replace only changed sources."""
    if data.get("projectGuid") != expected_guid:
        raise ValueError("Unexpected native project GUID")
    receipt = stage(data, destination, repo)
    destination = Path(destination)
    compiler = REPO / "tools/w64devkit/bin/g++.exe"
    probe = destination / "verify-native.cpp"
    probe.write_text('#include "tower_story.hpp"\nint main(){return ember::IntroDialogue.empty()?1:0;}\n', encoding="utf-8")
    exe = destination / "verify-native.exe"
    env = {**os.environ, "PATH": str(compiler.parent) + os.pathsep + os.environ.get("PATH", "")}
    flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    built = subprocess.run([str(compiler), "-std=c++17", "-finput-charset=UTF-8", "-fexec-charset=UTF-8",
                            "-I" + str(destination), str(probe), "-o", str(exe)], env=env, capture_output=True, timeout=45, creationflags=flags)
    if built.returncode:
        raise ValueError("Staged C++ compilation failed: " + built.stderr.decode("utf-8", errors="replace"))
    subprocess.run([str(exe), "--no-audio", "--background-test"], check=True, capture_output=True, timeout=5, creationflags=flags)
    originals, changed = {}, []
    backup = destination / "source-backup"
    backup.mkdir()
    for entry in receipt["files"]:
        source = repo / "src" / entry["path"]
        if sha(source) != entry["original_sha256"]:
            raise ValueError("Source changed during staging; refusing overwrite")
        originals[source] = source.read_bytes()
        (backup / source.name).write_bytes(originals[source])
    try:
        for entry in receipt["files"]:
            source = repo / "src" / entry["path"]
            staged = destination / entry["path"]
            if staged.read_bytes() == originals[source]:
                continue
            if sha(source) != entry["original_sha256"]:
                raise ValueError("Source changed before replacement")
            temp = source.with_name(source.name + ".articy-" + uuid.uuid4().hex + ".tmp")
            with temp.open("xb") as output:
                output.write(staged.read_bytes())
            temp.replace(source)
            changed.append(source)
    except Exception:
        for source in changed:
            if source.read_bytes() == (destination / source.name).read_bytes():
                source.write_bytes(originals[source])
        raise
    receipt.update(action="applied-native-export", production_sources_modified=bool(changed),
                   changed_files=[p.name for p in changed], staged_cpp_compiled=True,
                   source_backups=str(backup), native_data_sha256=hashlib.sha256(json.dumps(data, ensure_ascii=False, sort_keys=True).encode("utf-8")).hexdigest())
    write_new(destination / "apply-receipt.json", receipt)
    return receipt


def import_plans(directory):
    # Validate the complete batch before the first native mutation. A saved
    # source snapshot must not silently overwrite a newer game conversation.
    expected = {p["id"]: p for p in sources()}
    plans = []
    for path in sorted(Path(directory).glob("EF_*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        original = expected.get(data.get("id"))
        if original is None or data.get("source_sha256") != original["source_sha256"]:
            raise ValueError("Unmapped or stale source plan; compare and extract a fresh snapshot before import")
        plan = {k: data[k] for k in ("id", "title", "lines")}
        if not 1 <= len(plan["lines"]) <= 120 or not isinstance(plan["title"], str):
            raise ValueError("Invalid dialogue plan")
        for row in plan["lines"]:
            if row.get("speaker") not in ("Eldrin", "Wanderer") or not isinstance(row.get("text"), str) or not 1 <= len(row["text"]) <= 8000:
                raise ValueError("Invalid planned line")
        plans.append(plan)
    if len(plans) != len(expected) or {p["id"] for p in plans} != set(expected):
        raise ValueError("Expected all seven mapped source plans")
    bridge = ArticyBridge()
    status = bridge.call("status")
    heartbeat = status.get("result", {})
    if not heartbeat.get("projectOpen"):
        raise ValueError("No native project open; import was not attempted")
    guid = heartbeat["projectGuid"]
    existing = bridge.call("list_dialogues")["result"]
    revisions = {d["id"]: d["revision"] for d in existing}
    by_id = {d["id"]: d for d in existing}
    skipped = []
    for plan in plans:
        if plan["id"] in by_id:
            native = by_id[plan["id"]]
            native_rows = [{"speaker": row["speaker"], "text": row["text"]} for row in native["lines"]]
            if native["title"] != plan["title"] or native_rows != plan["lines"]:
                raise ValueError("Native dialogue differs from source plan; export/compare and edit explicitly through revision-protected MCP")
            skipped.append(plan["id"])
    jobs = []
    for plan in plans:
        if plan["id"] in skipped:
            continue
        parameters = {"project_guid": guid, "dialogue": plan}
        if plan["id"] in revisions:
            parameters["expected_revision"] = revisions[plan["id"]]
        jobs.append(bridge.call("upsert_dialogue", parameters))
    return {"jobs": jobs, "unchanged_skipped": skipped, "export": bridge.call("export_story")["result"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    p = sub.add_parser("extract"); p.add_argument("output", type=Path)
    p = sub.add_parser("import-plans"); p.add_argument("directory", type=Path); p.add_argument("--output", type=Path, required=True)
    p = sub.add_parser("stage"); p.add_argument("input", type=Path); p.add_argument("output", type=Path)
    p = sub.add_parser("apply"); p.add_argument("input", type=Path); p.add_argument("output", type=Path)
    args = parser.parse_args()
    if args.action == "extract":
        result = extract(args.output)
    elif args.action in ("stage", "apply"):
        data = json.loads(args.input.read_text(encoding="utf-8-sig"))
        if "result" in data: data = data["result"]
        if "export" in data: data = data["export"]
        result = stage(data, args.output) if args.action == "stage" else apply_export(data, args.output)
    else:
        if args.output.exists(): parser.error("Output exists")
        result = import_plans(args.directory)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        write_new(args.output, result)
    print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()

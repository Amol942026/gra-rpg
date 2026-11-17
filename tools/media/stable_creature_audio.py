"""Render Emberfall creature effects locally in ComfyUI, with no playback."""

import argparse
from copy import deepcopy
from http.client import HTTPConnection
import json
from pathlib import Path
import time
from urllib.parse import urlencode

from asset_tools import sha, write_json
from game_import import cue_names


REPO = Path(__file__).resolve().parents[2]
PRESETS = Path(__file__).with_name("presets")
EVENTS = ("attack", "hit", "fall", "dying")
REGIONAL = {"sporekin", "dune-eye", "cinderhorn"}
SECONDS = {"attack": 1.6, "hit": 1.3, "fall": 1.55, "dying": 2.0, "ability": 1.9}
SUFFIXES = ("", "-v2", "-v3")


def profiles():
    specs = {}
    text = (PRESETS / "creature_sfx.txt").read_text(encoding="utf-8")
    for raw in text.splitlines():
        if not raw or raw.startswith("#"):
            continue
        species, *descriptions = [field.strip() for field in raw.split("|")]
        if len(descriptions) == 1:
            specs[species] = ("ability", descriptions[0])
            continue
        for event, description in zip(EVENTS, descriptions, strict=True):
            marker = "" if event == "dying" or species in REGIONAL else "-voice"
            specs[f"{species}{marker}-{event}"] = (event, description)
    registered = set(cue_names())
    dying = {name for name in registered if name.endswith("-dying")}
    if not specs.keys() <= registered or dying != {name for name in specs if name.endswith("-dying")}:
        raise ValueError("Creature sound catalog does not cover the live runtime names")
    return specs


def request(route, payload=None):
    body = json.dumps(payload).encode("utf-8") if payload is not None else None
    connection = HTTPConnection("127.0.0.1", 8188, timeout=20)
    try:
        connection.request("POST" if body is not None else "GET", route, body,
                           headers={"Content-Type": "application/json"})
        response = connection.getresponse()
        data = response.read()
        if response.status != 200:
            raise RuntimeError(f"ComfyUI HTTP {response.status} on {route}: {data[:800]!r}")
        return data
    finally:
        connection.close()


def render_graph(directory, graph, client_id):
    workflow = directory / "workflow.json"
    queue_record = directory / "queued.json"
    history = directory / "history.json"
    if queue_record.exists():
        if json.loads(workflow.read_text(encoding="utf-8")) != graph:
            raise ValueError(f"Queued graph differs from requested graph: {workflow}")
        prompt_id = json.loads(queue_record.read_text(encoding="utf-8"))["prompt_id"]
        if history.is_file():
            return prompt_id, json.loads(history.read_text(encoding="utf-8"))
    elif workflow.exists():
        raise RuntimeError(f"Submission result unknown for {workflow}; check ComfyUI history")
    else:
        write_json(workflow, graph)
        queued = json.loads(request("/prompt", {"prompt": graph, "client_id": client_id}))
        prompt_id = queued["prompt_id"]
        write_json(queue_record, queued)

    deadline = time.monotonic() + 240
    while time.monotonic() < deadline:
        row = json.loads(request(f"/history/{prompt_id}")).get(prompt_id)
        if row:
            write_json(history, row)
            return prompt_id, row
        time.sleep(2)
    raise TimeoutError(f"ComfyUI prompt pending: {prompt_id}")


def render(root):
    stats = json.loads(request("/system_stats"))
    if stats["devices"][0]["type"] != "cpu":
        raise ValueError("Expected the dedicated CPU ComfyUI session")
    base = json.loads((PRESETS / "stable_audio_3_small_sfx_api.json").read_text(encoding="utf-8"))
    render_dir = root / "generated"
    render_dir.mkdir(parents=True, exist_ok=True)
    rows = []
    existing = root / "generation.json"
    if existing.is_file():
        rows = json.loads(existing.read_text(encoding="utf-8"))
    done = {row["name"]: row for row in rows}
    specs = profiles()
    for index, (name, (event, description)) in enumerate(specs.items()):
        if name in done:
            if any(sha(Path(take["path"])) != take["sha256"] for take in done[name]["takes"]):
                raise ValueError(f"Previously rendered source changed: {name}")
            continue
        graph = deepcopy(base)
        seconds = SECONDS[event]
        graph["3"]["inputs"]["text"] = (
            f"{description}. One isolated, dry, close-miked video game sound effect. "
            f"Detailed physical texture. No speech, no music, no ambience. Length: {seconds} seconds."
        )
        graph["5"]["inputs"].update(seconds=seconds, batch_size=len(SUFFIXES))
        graph["6"]["inputs"]["seed"] = 294601 + index * 203
        graph["8"]["inputs"]["filename_prefix"] = f"audio/{root.name}/{name}-%batch_num%"
        directory = render_dir / name
        directory.mkdir(exist_ok=True)
        prompt_id, row = render_graph(directory, graph, root.name)
        if row["status"]["status_str"] != "success":
            raise RuntimeError(f"ComfyUI render failed: {directory / 'history.json'}")
        rendered = row["outputs"]["8"]["audio"]
        if len(rendered) != len(SUFFIXES):
            raise ValueError(f"Incomplete variant set: {name}")
        takes = []
        for suffix, item in zip(SUFFIXES, rendered, strict=True):
            path = directory / f"{name}{suffix}.flac"
            data = request("/view?" + urlencode(item))
            if not data.startswith(b"fLaC"):
                raise ValueError(f"Invalid FLAC: {path}")
            if path.exists():
                if path.read_bytes() != data:
                    raise ValueError(f"Recorded render differs from ComfyUI output: {path}")
            else:
                with path.open("xb") as output:
                    output.write(data)
            takes.append({"path": str(path), "sha256": sha(path), "runtime": path.stem + ".wav"})
        rows.append({"name": name, "event": event, "prompt_id": prompt_id,
                     "workflow": str(directory / "workflow.json"), "takes": takes})
        staged = existing.with_suffix(".tmp")
        staged.write_text(json.dumps(rows, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        staged.replace(existing)
        print(f"{len(rows)}/{len(specs)} {name}", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    render(args.root.resolve())

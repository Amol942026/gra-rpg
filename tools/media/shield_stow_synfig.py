"""Author the left-hand shield transfer in Synfig and export the six control tracks."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import shutil
import time
import xml.etree.ElementTree as ET

from PIL import Image

from hero_synfig import curve, image, value
from sword_stow_synfig import animated
from synfig_cli import render

FRAMES = 36
FIELDS = ("right", "forward", "height", "yaw", "depart", "reach")
# Move past the left shoulder before turning toward the back. The latch
# precedes the hand's release so the same controls work in reverse.
POSES = (
    (-21, 24, 44,   0,   0, 1),
    (-25, 20, 50,  18, .55, 1),
    (-30, 10, 58,  55,   1, 1),
    (-30, -2, 63,  95,   1, 1),
    (-24,-13, 60, 135,   1, 1),
    (-14,-18, 53, 168,   1, 1),
    (  0,-15, 45, 180,   1, 1),
    (  0,-15, 45, 180,   1, .42),
    (  0,-15, 45, 180,   1, 0),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[2]
    parts = args.root / "parts"
    args.root.mkdir(parents=True, exist_ok=False)
    shutil.copytree(repo / "art/hero/stow/parts", parts)
    atlas = repo / "assets/sprites/equipment-icons"
    meta = json.loads(atlas.with_suffix(".json").read_text(encoding="utf-8"))
    entry = next(item for item in meta["items"] if item["name"] == "Tarcza dębowa")
    size = meta["cell"]
    x = entry["cell"] % meta["columns"] * size
    y = entry["cell"] // meta["columns"] * size
    with Image.open(atlas.with_suffix(".png")) as source:
        shield = source.crop((x, y, x + size, y + size)).convert("RGBA")
        shield.crop(shield.getchannel("A").getbbox()).save(parts / "shield.png")

    tracks = {name: [curve(column, frame / FRAMES) for frame in range(FRAMES + 1)]
              for name, column in zip(FIELDS, zip(*POSES))}
    canvas = ET.Element("canvas", version="1.2", width="320", height="320", fps="60",
                        **{"view-box": "-56 104 56 -8", "begin-time": "0f",
                           "end-time": f"{FRAMES - 1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall — tarcza na plecy i z powrotem"
    defs = ET.SubElement(canvas, "defs")
    for name, samples in tracks.items():
        animated(defs, name, "real", samples)
    image(canvas, "leg", (-13, 32, 1, 0))
    image(canvas, "leg", (1, 32, 15, 0))
    image(canvas, "arm", (12, 54, 27, 27))
    image(canvas, "body", (-20, 58, 20, 29))
    image(canvas, "face", (-9, 81, 9, 59))
    image(canvas, "hair_front", (-10, 84, 10, 61))
    plate = ET.SubElement(canvas, "layer", type="import", active="true", version="0.1", desc="Tarcza")
    value(plate, "filename", "string", "parts/shield.png")
    value(plate, "amount", "real", 1)
    value(plate, "blend_method", "integer", 0)
    tl, br = [], []
    for right, forward, height, yaw in zip(*(tracks[name] for name in FIELDS[:4])):
        center = right + forward * .34
        half = 2 + 8 * abs(math.cos(math.radians(yaw + 20)))
        tl.append((center - half, height + 10))
        br.append((center + half, height - 10))
    animated(ET.SubElement(plate, "param", name="tl"), "shield_tl", "vector", tl)
    animated(ET.SubElement(plate, "param", name="br"), "shield_br", "vector", br)
    path = args.root / "shield-stow.sif"
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)

    parsed = ET.parse(path)
    columns = [[float(w.find("real").get("value"))
                for w in parsed.findall(f"defs/animated[@id='{name}']/waypoint")]
               for name in FIELDS]
    assert all(len(column) == FRAMES + 1 for column in columns)
    header = ["#pragma once", "// Native Synfig control tracks from tools/media/shield_stow_synfig.py.",
              "namespace ember::shield_stow::data {",
              "struct Key {float " + ", ".join(FIELDS) + ";};", "inline constexpr Key keys[]={"]
    header.extend(" {" + ",".join(f"{number:.6f}f" for number in frame) + "}," for frame in zip(*columns))
    args.header.write_text("\n".join(header + ["};", "}"]) + "\n", encoding="utf-8")
    started = time.monotonic()
    receipt = render(path, output_root=args.root / "renders", threads=2,
                     expected_frames=FRAMES, require_motion=True)
    report = {"status": "PASS", "frames": FRAMES, "project_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
              "render_seconds": time.monotonic() - started, "render": receipt}
    (args.root / "report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

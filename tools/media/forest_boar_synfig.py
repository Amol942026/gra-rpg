"""Author the forest boar's staged weight transfer in native Synfig."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import time
import xml.etree.ElementTree as ET

from PIL import Image, ImageDraw

from hero_synfig import group, image
from sword_stow_synfig import animated
from synfig_cli import render

FRAMES = 32
FIELDS = ("lean", "drop", "pitch", "reach", "headPitch", "tail", "scrape")
REST = (0, 0, 0, 0, 0, 0, 0)
CROUCH = (1.5, -3.1, -7.5, -1.6, -21, 10.5, 0)
LOW_RUSH = (1.6, -2.0, -5, 1.8, -16.5, 13, 0)
CLIPS = {
    "melee": [
        (0, REST), (.25, (-1.4, -.8, -2, -1, -4, 2, 0)),
        (.51, (-3.1, -2.8, -4, -2.8, -16, 6, 0)),
        (.58, (-2.9, -2.5, -3, -2.4, -14, 6, 0)),
        (.65625, (3.25, -1.2, 3.2, 4.7, 9, -3, 0)),
        (.74, (2.3, -1.7, 2, 2.6, 5, -3.5, 0)),
        (.88, (.6, -.7, 0, .45, -4, 1.3, 0)), (1, REST),
    ],
    "windup": [
        (0, REST), (.23, (-1.3, -1.55, -2.2, -1.3, -8, 4, .65)),
        (.42, (-2.65, -2.8, -4.8, -2.6, -14, 7, .1)),
        (.56, (-1.55, -3.05, -5.65, -2.3, -16, 9, 1)),
        (.74, (-.7, -3.2, -6.65, -2, -19.5, 10, 0)), (1, CROUCH),
    ],
    "rush": [
        (0, CROUCH), (.10, (2.0, -1.4, -5.95, 3.1, -18.5, 12.5, 0)),
        (.23, LOW_RUSH), (.49, (1.3, -2.4, -4.3, 1.6, -15.9, 14.5, 0)),
        (.72, (1.8, -1.8, -4.9, 2, -16.8, 11.7, 0)), (1, LOW_RUSH),
    ],
    "recovery": [
        (0, LOW_RUSH), (.12, (2.8, -3.2, -7.3, 2.1, -17.8, 8.7, 0)),
        (.30, (-.6, -1.9, -2, -.65, -6, 4, 0)),
        (.60, (-.3, -.5, .8, .28, 4, -.35, 0)), (1, REST),
    ],
}


def interpolate(keys, frame):
    t = frame / FRAMES
    for (start, a), (end, b) in zip(keys, keys[1:]):
        if t <= end:
            u = (t - start) / (end - start)
            w = u * u * (3 - 2 * u)
            return tuple(x + (y - x) * w for x, y in zip(a, b))
    raise ValueError(f"No keyframe at {frame}")


def preview_parts(root):
    """Make a compact cutout from the existing painted boar material."""
    atlas_path = Path(__file__).resolve().parents[2] / "assets/sprites/creature-paints.png"
    with Image.open(atlas_path) as atlas:
        coat = atlas.crop((0, 128, 128, 256)).convert("RGBA")
        snout = atlas.crop((256, 384, 384, 512)).convert("RGBA")
    parts = root / "parts"
    parts.mkdir()
    for name, size, polygon, color, paint in [
        ("body", (190, 110), ((5, 73), (16, 39), (70, 27), (108, 16), (136, 15),
          (170, 28), (184, 49), (177, 85), (135, 101), (60, 98), (23, 86)), (104, 78, 59, 255), coat),
        ("head", (130, 112), ((8, 68), (23, 37), (25, 11), (46, 34), (83, 48),
          (119, 64), (123, 82), (107, 100), (56, 92), (25, 81)), (114, 80, 62, 255), snout),
        ("leg", (32, 80), ((3, 5), (29, 5), (24, 31), (20, 63), (25, 71),
          (23, 77), (7, 77), (6, 67), (12, 47), (10, 23)), (83, 61, 49, 255), coat),
    ]:
        shape = Image.new("L", size)
        ImageDraw.Draw(shape).polygon(polygon, fill=255)
        tile = Image.new("RGBA", size, color)
        tile.alpha_composite(paint.resize(size, Image.Resampling.LANCZOS))
        tile.putalpha(shape)
        draw = ImageDraw.Draw(tile)
        if name == "head":
            draw.ellipse((65, 51, 74, 56), fill=(210, 164, 104, 255))
            draw.polygon(((88, 90), (102, 91), (112, 79), (111, 94), (102, 104)), fill=(219, 205, 172, 255))
        tile.save(parts / (name + ".png"))


def translate(parent, name, coordinates):
    layer = ET.SubElement(parent, "layer", type="translate", active="true", version="0.1", desc=name)
    animated(ET.SubElement(layer, "param", name="origin"), name, "vector", coordinates)


def author(root, name, keys):
    rows = [interpolate(keys, frame) for frame in range(FRAMES + 1)]
    canvas = ET.Element("canvas", version="1.2", width="384", height="240", fps="60",
                        **{"view-box": "-53 56 59 -14", "begin-time": "0f", "end-time": f"{FRAMES - 1}f",
                           "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / dzik / " + name
    defs = ET.SubElement(canvas, "defs")
    for field, values in zip(FIELDS, zip(*rows)):
        animated(defs, field, "real", values)
    for number, x in enumerate((-19, -10, 13, 22)):
        leg = group(canvas, f"Kopyto {number + 1}")
        image(leg, "leg", (x - 3.25, 24, x + 3.25, 0))
        coordinates = []
        for frame, row in enumerate(rows):
            t = frame / FRAMES * 2.9 + number * .25
            stepping = name == "rush"
            forward = math.cos(t * math.tau) * 9 if stepping else -row[6] * 2.85 if number == 3 else 0
            lift = max(0, math.sin(t * math.tau)) ** 2 * 5 if stepping else row[6] * 3.4 if number == 3 else 0
            coordinates.append((forward, lift))
        translate(leg, f"kopyto_{number}", coordinates)
    body = group(canvas, "Ciężar tułowia")
    image(body, "body", (-32, 46, 26, 15))
    body_turn = ET.SubElement(body, "layer", type="rotate", active="true", version="0.1")
    pivot = ET.SubElement(ET.SubElement(body_turn, "param", name="origin"), "vector")
    ET.SubElement(pivot, "x").text = "0"
    ET.SubElement(pivot, "y").text = "26"
    animated(ET.SubElement(body_turn, "param", name="amount"), "body_preview", "angle", [row[2] for row in rows])
    translate(body, "body_shift", [(row[0], row[1]) for row in rows])
    head = group(canvas, "Łeb i szable")
    image(head, "head", (16, 47, 48, 19))
    head_turn = ET.SubElement(head, "layer", type="rotate", active="true", version="0.1")
    pivot = ET.SubElement(ET.SubElement(head_turn, "param", name="origin"), "vector")
    ET.SubElement(pivot, "x").text = "21"
    ET.SubElement(pivot, "y").text = "31"
    animated(ET.SubElement(head_turn, "param", name="amount"), "head_preview", "angle", [row[2] + row[4] for row in rows])
    translate(head, "head_shift", [(row[0] + row[3], row[1] + 21 * math.sin(math.radians(row[2]))) for row in rows])
    path = root / (name + ".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    explicitly_saved = ET.parse(path)
    columns = [tuple(float(point.find("real").get("value")) for point in explicitly_saved.findall(
        f"defs/animated[@id='{field}']/waypoint")) for field in FIELDS]
    if any(len(column) != FRAMES + 1 for column in columns):
        raise ValueError("Incomplete boar waypoint tracks")
    return path, tuple(zip(*columns))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    preview_parts(args.root)
    source = ["#pragma once", "// Native Synfig tracks exported by tools/media/forest_boar_synfig.py.",
              "namespace ember::forest_boar_motion::data {", f"inline constexpr int Frames={FRAMES};",
              "struct Key {float " + ", ".join(FIELDS) + ";};"]
    manifests = []
    for name, keys in CLIPS.items():
        project, rows = author(args.root, name, keys)
        source.append(f"inline constexpr Key {name}[]={{")
        source.extend(" {" + ",".join(f"{value:.7f}f" for value in row) + "}," for row in rows)
        source.append("};")
        started = time.monotonic()
        asset = render(project, output_root=args.root / "renders", threads=2, expected_frames=FRAMES,
                       require_motion=True)
        manifests.append({"name": name, "sha256": hashlib.sha256(project.read_bytes()).hexdigest(),
                          "render_seconds": time.monotonic() - started, "render": asset})
    args.header.write_text("\n".join(source + ["}"]) + "\n", encoding="utf-8")
    report = {"status": "PASS", "rendered_frames": FRAMES * len(CLIPS),
              "render_seconds": sum(entry["render_seconds"] for entry in manifests), "clips": manifests,
              "header_sha256": hashlib.sha256(args.header.read_bytes()).hexdigest()}
    (args.root / "report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

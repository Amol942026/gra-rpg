"""Render reversible carry gestures in Synfig and export their control tracks."""
import argparse
import json
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

from PIL import Image

from hero_synfig import curve, group, image, value
from sword_stow_synfig import animated
from synfig_cli import render

FRAMES = 36
FIELDS = ("right", "forward", "height", "yaw", "pitch", "reach", "seat")
# Angles describe the tip direction in the hero's local frame. Reach releases
# the grip after the blade or haft is seated. Dobywanie uses the same path in reverse.
CLIPS = {
    "dagger": [
        (23, 13, 44, 20, 70, 0, 0), (24, 20, 42, 48, 27, .66, 0),
        (25, 14, 48, 52, -32, 1, 0), (21, 11, 53, 35, -78, 1, 0),
        (19, 8, 48, 37, -78, 1, .37), (17, 6, 39, 37, -78, 1, 1),
        (17, 6, 39, 37, -78, .60, 1), (17, 6, 39, 37, -78, .17, 1),
        (17, 6, 39, 37, -78, 0, 1),
    ],
    "axe": [
        (23, 13, 44, 13, 72, 0, 0), (25, 17, 48, 52, 69, .58, 0),
        (25, 10, 59, 72, 72, 1, 0), (23, -3, 65, 130, 71, 1, 0),
        (13, -18, 47, 110, 78, 1, 0), (-2, -14, 31, 90, 90, 1, 1),
        (-2, -14, 31, 90, 90, .61, 1), (-2, -14, 31, 90, 90, .18, 1),
        (-2, -14, 31, 90, 90, 0, 1),
    ],
    "spear": [
        (18, 10, 40, 0, 23, 0, 0), (24, 18, 45, 38, 52, .54, 0),
        (25, 11, 58, 56, 75, 1, 0), (23, -2, 65, 130, 79, 1, 0),
        (13, -18, 53, 110, 74, 1, 0), (-3, -14, 43, 90, 70, 1, 1),
        (-3, -14, 43, 90, 70, .61, 1), (-3, -14, 43, 90, 70, .18, 1),
        (-3, -14, 43, 90, 70, 0, 1),
    ],
}


def weapon_preview(repo, root, name):
    if name == "spear":
        # The spear uses its own in-game texture panels. Reuse those colors
        # for the cutout, whose shaft and point are independent of the body.
        with Image.open(repo / "assets/sprites/hero-spear-paints.png") as source:
            colors = source.convert("RGBA")
            from PIL import ImageDraw
            weapon = Image.new("RGBA", (32, 160))
            draw = ImageDraw.Draw(weapon)
            draw.rounded_rectangle((14, 26, 18, 155), radius=2, fill=(122, 84, 51, 255))
            for y in (40, 89, 132, 150):
                draw.rectangle((13, y, 19, y + 3), fill=(180, 146, 82, 255))
            draw.polygon([(16, 1), (26, 23), (16, 35), (7, 23)], fill=(128, 158, 162, 255))
            draw.polygon([(16, 1), (16, 31), (7, 23)], fill=(202, 219, 209, 255))
            for y in range(7, 29):
                sample = colors.getpixel((colors.width // 2, y * colors.height // 32))
                if sample[3] > 200:
                    draw.point((16, y), fill=sample)
            weapon.save(root / "parts/weapon.png")
        return
    metadata = json.loads((repo / "assets/sprites/equipment-icons.json").read_text(encoding="utf-8"))
    prefix = "Topór strażnika" if name == "axe" else "Sztylet łowcy"
    index = next(item["cell"] for item in metadata["items"] if item["name"] == prefix)
    size, cols = metadata["cell"], metadata["columns"]
    x, y = index % cols * size, index // cols * size
    with Image.open(repo / "assets/sprites/equipment-icons.png") as atlas:
        weapon = atlas.crop((x, y, x + size, y + size)).rotate(30, resample=Image.Resampling.BICUBIC, expand=True)
        weapon.crop(weapon.getbbox()).save(root / "parts/weapon.png")


def build(root, name, keys):
    canvas = ET.Element("canvas", version="1.2", width="320", height="360", fps="60",
                        **{"view-box": "-53 112 53 -7", "begin-time": "0f",
                           "end-time": "35f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / " + name + " / chowanie i dobywanie"
    defs = ET.SubElement(canvas, "defs")
    tracks = {field: [curve(column, f / FRAMES) for f in range(FRAMES + 1)]
              for field, column in zip(FIELDS, zip(*keys))}
    for field, samples in tracks.items():
        animated(defs, field, "real", samples)
    image(canvas, "leg", (-13, 32, 1, 0))
    image(canvas, "leg", (1, 32, 15, 0))
    image(canvas, "body", (-20, 58, 20, 29))
    image(canvas, "face", (-9, 81, 9, 59))
    image(canvas, "hair_front", (-10, 84, 10, 61))
    image(canvas, "arm", (-26, 54, -12, 27))
    arm = group(canvas, "Ręka sięga po broń")
    image(arm, "arm", (12, 55, 25, 33))
    weapon = group(canvas, "Broń na torze chowania")
    length = {"dagger": 28, "axe": 45, "spear": 65}[name]
    width = {"dagger": 11, "axe": 20, "spear": 9}[name]
    image(weapon, "weapon", (-width / 2, length * .68, width / 2, -length * .32))
    rotate = ET.SubElement(weapon, "layer", type="rotate", active="true", version="0.1")
    value(rotate, "origin", "vector", (0, 0))
    coords, hand, angles = [], [], []
    for r, f, z, yaw, pitch, reach, seat in zip(*tracks.values()):
        x, y = r + f * .50, z - f * .20
        coords.append((x, y))
        hand.append(((x - 20) * reach, (y - 38) * reach))
        yaw, pitch = math.radians(yaw), math.radians(pitch)
        direction = math.cos(pitch) * (math.sin(yaw) + math.cos(yaw) * .5)
        angles.append(math.degrees(math.atan2(-direction, math.sin(pitch))))
    animated(ET.SubElement(rotate, "param", name="amount"), "preview_angle", "angle", angles)
    for node, points, identifier in ((arm, hand, "arm_shift"), (weapon, coords, "weapon_shift")):
        move = ET.SubElement(node, "layer", type="translate", active="true", version="0.1")
        animated(ET.SubElement(move, "param", name="origin"), identifier, "vector", points)
    path = root / (name + ".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    parsed = ET.parse(path)
    columns = [[float(w.find("real").get("value"))
                for w in parsed.findall(f"defs/animated[@id='{field}']/waypoint")]
               for field in FIELDS]
    assert all(len(column) == FRAMES + 1 for column in columns)
    return path, list(zip(*columns))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    repo = Path(__file__).resolve().parents[2]
    lines = ["#pragma once", "// Synfig curves exported by tools/media/weapon_stow_synfig.py.",
             "namespace ember::weapon_stow::data {",
             "struct Key {float " + ", ".join(FIELDS) + ";};"]
    receipts = {}
    for name, keys in CLIPS.items():
        root = args.root / name
        shutil.copytree(repo / "art/hero/stow/parts", root / "parts")
        weapon_preview(repo, root, name)
        path, rows = build(root, name, keys)
        lines.append(f"inline constexpr Key {name}[]={{")
        lines.extend(" {" + ",".join(f"{value:.6f}f" for value in row) + "}," for row in rows)
        lines.append("};")
        receipts[name] = render(path, output_root=root / "renders", threads=2,
                                expected_frames=FRAMES, require_motion=True)
    args.header.write_text("\n".join(lines + ["}"]) + "\n", encoding="utf-8")
    (args.root / "report.json").write_text(json.dumps(receipts, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

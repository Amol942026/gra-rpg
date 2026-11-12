"""Author axe and dagger attack curves and export the Synfig waypoints."""
import argparse
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

from PIL import Image

from hero_synfig import group, image, value
from spear_synfig import samples
from sword_stow_synfig import animated

FIELDS = ("handR", "handF", "handZ", "yaw", "pitch", "torsoYaw", "lean")
CLIPS = {
    # Wrist position, blade heading, blade elevation, and torso motion.
    "axe": (27, {
        0: (24, 14, 46, 0, 75, 0, 0),
        5: (25, 4, 49, 69, 64, -9, -2),
        8: (29, -5, 56, 128, 46, -22, -5),
        11: (28, 4, 60, 68, 64, -11, -1),
        14: (21, 22, 55, 10, 17, 13, 8),
        18: (15, 22, 42, -34, -32, 22, 11),
        23: (22, 17, 45, -12, 39, 6, 3),
        27: (24, 14, 46, 0, 75, 0, 0),
    }),
    "dagger": (19, {
        0: (24, 14, 46, 0, 75, 0, 0),
        3: (23, 8, 47, 6, 39, -7, -1),
        5: (23, 8, 47, 3, 16, -9, 0),
        8: (20, 27, 51, -5, 2, 16, 8),
        10: (20, 25, 50, -5, 3, 13, 6),
        14: (21, 14, 47, -2, 15, 3, 2),
        19: (24, 14, 46, 0, 75, 0, 0),
    }),
}


def author(root, parts, kind, frames, waypoints):
    root.mkdir()
    shutil.copytree(parts, root / "parts")
    # Use a copy of the existing game illustration for the cutout preview.
    # The game continues to render the equipped variant's own shape and paint.
    cell = 1 if kind == "axe" else 7
    with Image.open("assets/sprites/equipment-icons.png") as atlas:
        icon = atlas.crop((cell * 128, 0, (cell + 1) * 128, 128)).convert("RGBA")
        icon = icon.rotate(33, Image.Resampling.BICUBIC, expand=True)
        icon.crop(icon.getbbox()).save(root / f"parts/{kind}.png")
    canvas = ET.Element("canvas", version="1.2", width="384", height="352", fps="60",
                        **{"view-box": "-44 100 97 -14", "begin-time": "0f",
                           "end-time": f"{frames-1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = f"Emberfall / {kind}"
    track = samples(waypoints, frames)
    defs = ET.SubElement(canvas, "defs")
    for channel, values in zip(FIELDS, zip(*track)):
        animated(defs, channel, "real", values)
    image(canvas, "leg", (-13, 32, 1, 0))
    image(canvas, "leg", (1, 32, 15, 0))
    image(canvas, "body", (-16, 59, 16, 29))
    image(canvas, "face", (-8, 81, 8, 60))
    image(canvas, "hair_front", (-10, 84, 10, 61))
    weapon = group(canvas, "Broń i punkt chwytu")
    image(weapon, kind, (-8, 34, 8, -9) if kind == "axe" else (-5, 24, 5, -6))
    turn = ET.SubElement(weapon, "layer", type="rotate", active="true", version="0.1")
    value(turn, "origin", "vector", (0, 0))
    move = ET.SubElement(weapon, "layer", type="translate", active="true", version="0.1")
    angles, centers = [], []
    for right, forward, z, yaw, pitch, _, _ in track:
        yaw, pitch = math.radians(yaw), math.radians(pitch)
        projected_x = math.cos(pitch) * (.85 * math.cos(yaw) + .45 * math.sin(yaw))
        projected_z = math.sin(pitch) - .20 * math.cos(pitch) * math.cos(yaw)
        angles.append(math.degrees(math.atan2(-projected_x, projected_z)))
        centers.append((right * .45 + forward * .85, z - .20 * forward))
    animated(ET.SubElement(turn, "param", name="amount"), "preview_angle", "angle", angles)
    animated(ET.SubElement(move, "param", name="origin"), "preview_grip", "vector", centers)
    path = root / f"{kind}.sif"
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def export(root, header):
    lines = ["#pragma once", "// Exported from Synfig by tools/media/melee_attacks_synfig.py.",
             "namespace ember::hero_weapon_motion::data {", "struct Key {float "+",".join(FIELDS)+";};"]
    for kind, (frames, _) in CLIPS.items():
        canvas = ET.parse(root / kind / f"{kind}.sif").getroot()
        tracks = []
        for field in FIELDS:
            points = canvas.findall(f"defs/animated[@id='{field}']/waypoint")
            assert [int(w.get("time")[:-1]) for w in points] == list(range(frames + 1))
            tracks.append([float(w.find("real").get("value")) for w in points])
        lines += [f"inline constexpr Key {kind}[]={{"]
        lines += [" {"+",".join(f"{v:.7f}f" for v in row)+"}," for row in zip(*tracks)]
        lines += ["};"]
    header.write_text("\n".join(lines + ["}", ""]), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--parts", type=Path)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--export-only", action="store_true")
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    if not args.export_only:
        if args.parts is None:
            parser.error("--parts is required for authoring")
        args.root.mkdir(parents=True)
        for kind, (frames, waypoints) in CLIPS.items():
            author(args.root / kind, args.parts, kind, frames, waypoints)
    export(args.root, args.header)

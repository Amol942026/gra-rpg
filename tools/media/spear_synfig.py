"""Build and export Emberfall's two-handed spear controls from a Synfig project."""
import argparse
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

from PIL import Image, ImageDraw

from hero_synfig import group, image, value
from sword_stow_synfig import animated

FIELDS = ("rearR", "rearF", "rearZ", "yaw", "pitch", "lane", "lean", "spacing", "slide")
FRAMES = 30
KEYS = {
    0: (6, 14, 42, 0, 71, -.30, 0),
    6: (12, 12, 42.5, -16, 45, -.29, -3),
    9: (13.8, 10.8, 44, -28, 27, -.30, -2),
    12: (9.7, 22, 46, -36, 3, -.29, 11),
    14: (9.8, 21.2, 46, -36, 4, -.29, 9),
    20: (11, 11.5, 43.8, -24, 35, -.26, 1),
    26: (8, 13.5, 42.2, -6, 64, -.29, 0),
    30: (6, 14, 42, 0, 71, -.30, 0),
}
GRIP = {
    0: (13.4, 0), 9: (13.4, 0), 12: (9.6, 7.5),
    14: (10.4, 6.5), 20: (13.4, 0), 30: (13.4, 0),
}

# The five clips share the contact key and return to the same two-handed guard.
# Each weapon has its own preparation and follow-through.
VARIANTS = {
    "ash": {},
    "briar": {
        6: (11.6, 12.4, 41, -18, 42, -.30, -3),
        9: (13.2, 12, 42.3, -30, 15, -.30, 1),
        14: (9.8, 21.4, 45, -35, 7, -.29, 7),
        20: (9, 13, 42, -15, 48, -.28, 0),
    },
    "oasis": {
        6: (11, 13, 44.5, -13, 55, -.29, -2),
        9: (13.2, 11.8, 45.8, -25, 33, -.31, 0),
        14: (9.8, 21.5, 46.6, -36, 9, -.29, 8),
        20: (10, 12.8, 44, -20, 43, -.27, 0),
    },
    "basalt": {
        6: (12.4, 10.8, 42, -20, 53, -.28, -5),
        9: (14.4, 9.3, 43.2, -33, 31, -.29, -4),
        14: (10.1, 21.4, 45.5, -38, 3, -.29, 10),
        20: (12, 10.8, 42.4, -29, 24, -.28, 3),
        26: (8.7, 12.5, 42, -10, 58, -.29, 0),
    },
    "dawn": {
        6: (10.4, 12.6, 44.2, -20, 63, -.30, -4),
        9: (13.4, 11.2, 46, -32, 38, -.30, -1),
        14: (9.8, 21.8, 47, -36, 6, -.29, 9),
        20: (10.6, 12.4, 44.5, -23, 46, -.30, 1),
    },
}

def samples(keys, frames):
    out = []
    waypoints = sorted(keys)
    assert waypoints[0] == 0 and waypoints[-1] == frames
    for a, b in zip(waypoints, waypoints[1:]):
        for frame in range(a, b):
            t = (frame - a) / (b - a)
            t = t * t * (3 - 2 * t)
            out.append(tuple(x + (y - x) * t for x, y in zip(keys[a], keys[b])))
    return out + [keys[frames]]


def author(root, parts, variant):
    root.mkdir(parents=True, exist_ok=False)
    shutil.copytree(parts, root / "parts")
    png = Image.new("RGBA", (24, 200))
    brush = ImageDraw.Draw(png)
    brush.rounded_rectangle((10, 32, 14, 198), radius=2, fill="#8c623b")
    brush.rectangle((11, 36, 12, 196), fill="#c19765")
    brush.polygon(((12, 0), (20, 26), (14, 43), (10, 43), (4, 26)), fill="#93b5ad")
    brush.line(((12, 4), (12, 39)), fill="#dce8d2", width=2)
    brush.rectangle((9, 39, 15, 46), fill="#bb995e")
    png.save(root / "parts/spear.png")

    track = [pose + grip for pose, grip in zip(samples(KEYS | VARIANTS[variant], FRAMES),
                                               samples(GRIP, FRAMES))]
    canvas = ET.Element("canvas", version="1.2", width="384", height="352", fps="60",
                        **{"view-box": "-49 89 78 -11", "begin-time": "0f",
                           "end-time": "29f", "antialias": "2", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = f"Emberfall — dwuręczne pchnięcie włócznią ({variant})"
    ET.SubElement(canvas, "desc").text = "Rozbieg grotu, wyprost w klatce 12 i powrót do gardy."
    defs = ET.SubElement(canvas, "defs")
    for index, field in enumerate(FIELDS):
        animated(defs, field, "real", [row[index] for row in track])

    for side in (-1, 1):
        image(canvas, "leg", (side * 7 - 7, 32, side * 7 + 7, 0))
    image(canvas, "body", (-16, 59, 16, 29))
    image(canvas, "face", (-8, 81, 8, 60))
    image(canvas, "hair_front", (-10, 84, 10, 61))
    spear = group(canvas, "Drzewce i grot")
    image(spear, "spear", (-4, 44, 4, -22))
    rotation = ET.SubElement(spear, "layer", type="rotate", active="true", version="0.1", desc="Kąt drzewca")
    value(rotation, "origin", "vector", (0, 0))
    centers, angles = [], []
    for r, f, z, _, pitch, lane, _, _, slide in track:
        cosine, sine = math.cos(math.radians(pitch)), math.sin(math.radians(pitch))
        scale = math.sqrt(1 - lane * lane)
        axis_x, axis_z = lane + scale * cosine * .85, scale * (sine - cosine * .20)
        centers.append((r + f * .85 + slide * axis_x, z - f * .20 + slide * axis_z))
        angles.append(math.degrees(math.atan2(-axis_x, axis_z)))
    animated(ET.SubElement(rotation, "param", name="amount"), "spear_angle", "angle", angles)
    translate = ET.SubElement(spear, "layer", type="translate", active="true", version="0.1", desc="Prowadzenie chwytu")
    animated(ET.SubElement(translate, "param", name="origin"), "spear_grip", "vector", centers)
    path = root / "thrust.sif"
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def read_tracks(path):
    root = ET.parse(path).getroot()
    tracks = []
    for field in FIELDS:
        points = root.findall(f"defs/animated[@id='{field}']/waypoint")
        assert [int(point.get("time")[:-1]) for point in points] == list(range(FRAMES + 1))
        tracks.append([float(point.find("real").get("value")) for point in points])
    return ["  {"+",".join(f"{v:.7f}f" for v in row)+"}," for row in zip(*tracks)]


def export(projects, header):
    clips = []
    for variant, project in projects.items():
        clips.extend([f" {{ // {variant}", *read_tracks(project), " },"])
    header.write_text("\n".join([
        "#pragma once",
        "// Exact Synfig waypoints from tools/media/spear_synfig.py.",
        "namespace ember::spear_motion::data {",
        "struct Key {float "+",".join(FIELDS)+";};",
        f"inline constexpr Key keys[][{FRAMES + 1}]={{", *clips, "};", "}", ""
    ]), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--parts", type=Path)
    parser.add_argument("--header", required=True, type=Path)
    parser.add_argument("--export-only", action="store_true")
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    if args.export_only:
        projects = {variant: args.root / variant / "thrust.sif" for variant in VARIANTS}
    else:
        if args.parts is None:
            parser.error("--parts is required when authoring Synfig projects")
        args.root.mkdir(parents=True, exist_ok=False)
        projects = {variant: author(args.root / variant, args.parts, variant) for variant in VARIANTS}
    export(projects, args.header)
    for project in projects.values():
        print(project)

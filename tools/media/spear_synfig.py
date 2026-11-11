"""Build and export Emberfall's two-handed spear controls from a Synfig project."""
import argparse
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

from PIL import Image, ImageDraw

from hero_synfig import group, image, value
from sword_stow_synfig import animated

FIELDS = ("rearR", "rearF", "rearZ", "yaw", "pitch", "lane", "lean")
FRAMES = 30
KEYS = {
    0: (6, 14, 42, 0, 71, -.30, 0),
    6: (12, 12, 42.5, -16, 45, -.29, -3),
    9: (13.8, 10.8, 44, -28, 27, -.30, -2),
    12: (12.2, 12.2, 45, -42, 4, -.27, 8),
    14: (12, 12.1, 45, -40, 6, -.27, 6),
    20: (11, 11.5, 43.8, -24, 35, -.26, 1),
    26: (8, 13.5, 42.2, -6, 64, -.29, 0),
    30: (6, 14, 42, 0, 71, -.30, 0),
}


def samples():
    out = []
    frames = list(KEYS)
    for a, b in zip(frames, frames[1:]):
        for frame in range(a, b):
            t = (frame - a) / (b - a)
            t = t * t * (3 - 2 * t)
            out.append(tuple(x + (y - x) * t for x, y in zip(KEYS[a], KEYS[b])))
    return out + [KEYS[FRAMES]]


def author(root, parts):
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

    track = samples()
    canvas = ET.Element("canvas", version="1.2", width="384", height="352", fps="60",
                        **{"view-box": "-49 89 78 -11", "begin-time": "0f",
                           "end-time": "29f", "antialias": "2", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall — dwuręczne pchnięcie włócznią"
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
    for r, f, z, _, pitch, lane, _ in track:
        cosine, sine = math.cos(math.radians(pitch)), math.sin(math.radians(pitch))
        scale = math.sqrt(1 - lane * lane)
        axis_x, axis_z = lane + scale * cosine * .85, scale * (sine - cosine * .20)
        centers.append((r + f * .85, z - f * .20))
        angles.append(math.degrees(math.atan2(-axis_x, axis_z)))
    animated(ET.SubElement(rotation, "param", name="amount"), "spear_angle", "angle", angles)
    translate = ET.SubElement(spear, "layer", type="translate", active="true", version="0.1", desc="Prowadzenie chwytu")
    animated(ET.SubElement(translate, "param", name="origin"), "spear_grip", "vector", centers)
    path = root / "thrust.sif"
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def export(path, header):
    root = ET.parse(path).getroot()
    tracks = []
    for field in FIELDS:
        points = root.findall(f"defs/animated[@id='{field}']/waypoint")
        assert [int(point.get("time")[:-1]) for point in points] == list(range(FRAMES + 1))
        tracks.append([float(point.find("real").get("value")) for point in points])
    rows = [" {"+",".join(f"{v:.7f}f" for v in row)+"}," for row in zip(*tracks)]
    header.write_text("\n".join([
        "#pragma once",
        "// Exact Synfig waypoints from tools/media/spear_synfig.py.",
        "namespace ember::spear_motion::data {",
        "struct Key {float "+",".join(FIELDS)+";};",
        "inline constexpr Key keys[]={", *rows, "};", "}", ""
    ]), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--parts", required=True, type=Path)
    parser.add_argument("--header", required=True, type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    project = author(args.root, args.parts)
    export(project, args.header)
    print(project)

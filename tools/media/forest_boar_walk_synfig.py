"""Natywna pętla kroku dzika i krzywe do renderera gry."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import time
import xml.etree.ElementTree as ET

from forest_boar_synfig import preview_parts, translate
from hero_synfig import group, image
from sword_stow_synfig import animated
from synfig_cli import render


FRAMES = 38
SUPPORT = .60
STRIDE = 13.8
STAGGER = .19
# Długość podparcia jest związana z rzeczywistą drogą po mapie. Synfig
# wyznacza płynne wznoszenie i opadanie w trakcie przenoszenia nogi.
SWING = [
    (0, (0, 0)), (.13, (.19, 9)), (.32, (.72, 15)), (.50, (1, 8)),
    (.71, (.72, -3)), (.88, (.16, -4)), (1, (0, 0)),
]
BODY = [
    (0, (-.3, -.36)), (.19, (.46, .52)), (.35, (.02, .63)),
    (.5, (-.3, -.36)), (.69, (.46, .52)), (.85, (.02, .63)),
    (1, (-.3, -.36)),
]
CHANNELS = ("uniesienie", "przetoczenie", "ciezar", "pochylenie")


def sample(keys, phase):
    for (first, a), (last, b) in zip(keys, keys[1:]):
        if phase <= last:
            t = (phase - first) / (last - first)
            blend = t * t * (3 - 2 * t)
            return tuple(x + (y - x) * blend for x, y in zip(a, b))
    raise ValueError(f"Nieprawidłowa faza {phase}")


def foot(phase, stagger):
    local = (phase + stagger) % 1
    if local <= SUPPORT:
        return STRIDE * (1 - 2 * local / SUPPORT), 0, 0
    u = (local - SUPPORT) / (1 - SUPPORT)
    tangent = -2 * (1 - SUPPORT) / SUPPORT
    forward = -4*u**3 + 6*u*u - 1 + tangent*(2*u**3 - 3*u*u + u)
    lift, rock = sample(SWING, u)
    return forward * STRIDE, lift * 3.85, rock


def rotate(parent, name, pivot, angles):
    layer = ET.SubElement(parent, "layer", type="rotate", active="true", version="0.1", desc=name)
    point = ET.SubElement(ET.SubElement(layer, "param", name="origin"), "vector")
    ET.SubElement(point, "x").text = str(pivot[0])
    ET.SubElement(point, "y").text = str(pivot[1])
    animated(ET.SubElement(layer, "param", name="amount"), name, "angle", angles)


def author(root):
    canvas = ET.Element("canvas", version="1.2", width="384", height="240", fps="60",
                        **{"view-box": "-53 56 59 -14", "begin-time": "0f",
                           "end-time": f"{FRAMES-1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / dzik / chód czterotaktowy"
    phases = [n / FRAMES for n in range(FRAMES + 1)]
    rows = [sample(SWING, phase) + sample(BODY, phase) for phase in phases]
    defs = ET.SubElement(canvas, "defs")
    for name, column in zip(CHANNELS, zip(*rows)):
        animated(defs, name, "real", column)

    for number, (anchor, delay) in enumerate(((-14.1, STAGGER+.5), (13.8, .5),
                                              (-14.1, STAGGER), (13.8, 0))):
        stride = [foot(phase, delay) for phase in phases]
        leg = group(canvas, f"Krok racicy {number+1}")
        image(leg, "leg", (anchor - 3.2, 24, anchor + 3.2, 0))
        rotate(leg, f"staw_{number}", (anchor, 23), [math.degrees(math.atan2(x, 22)) for x, _, _ in stride])
        translate(leg, f"uniesienie_{number}", [(0, height) for _, height, _ in stride])
        hoof = group(canvas, f"Przetoczenie racicy {number+1}")
        image(hoof, "hoof", (anchor - 2.45, 4, anchor + 2.45, 0))
        rotate(hoof, f"racica_{number}", (anchor, 2), [angle for _, _, angle in stride])
        translate(hoof, f"tor_{number}", [(x, height) for x, height, _ in stride])
    torso = group(canvas, "Przenoszenie ciężaru")
    image(torso, "body", (-32, 46, 26, 15))
    image(torso, "head", (16, 47, 48, 19))
    rotate(torso, "kolysanie", (0, 26), [pitch for _, _, _, pitch in rows])
    translate(torso, "ciezar_tulowia", [(0, height) for _, _, height, _ in rows])
    path = root / "chod.sif"
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    saved = ET.parse(path)
    columns = [tuple(float(p.find("real").get("value")) for p in saved.findall(
        f"defs/animated[@id='{name}']/waypoint")) for name in CHANNELS]
    if any(len(column) != FRAMES + 1 for column in columns):
        raise ValueError("Niepełny eksport cyklu Synfig")
    return path, zip(*columns)


def main():
    from PIL import Image, ImageDraw

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    preview_parts(args.root)
    hoof = Image.new("RGBA", (32, 24))
    draw = ImageDraw.Draw(hoof)
    draw.rounded_rectangle((1, 3, 14, 22), radius=3, fill="#392c25")
    draw.rounded_rectangle((16, 2, 29, 22), radius=3, fill="#42342b")
    hoof.save(args.root / "parts/hoof.png")
    project, rows = author(args.root)
    started = time.monotonic()
    manifest = render(project, output_root=args.root / "renders", threads=2,
                      expected_frames=FRAMES, require_motion=True)
    elapsed = time.monotonic() - started
    header = ["#pragma once", "// Export from the editable Synfig cycle in forest_boar_walk_synfig.py.",
              "namespace ember::forest_boar_motion::data {",
              f"inline constexpr int WalkFrames={FRAMES};",
              "struct WalkKey {float lift, rock, bob, pitch;};",
              "inline constexpr WalkKey walkCycle[]={"]
    header.extend("    {" + ", ".join(f"{value:.7f}f" for value in row) + "}," for row in rows)
    args.header.write_text("\n".join(header + ["};", "}"]) + "\n", encoding="utf-8")
    report = {"status": "PASS", "rendered_frames": FRAMES, "render_seconds": elapsed,
              "source": str(project), "render": manifest,
              "header_sha256": hashlib.sha256(args.header.read_bytes()).hexdigest(),
              "rendered_without_audio": True}
    (args.root / "report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

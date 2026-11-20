"""Export hoof and horn tracks for the directional Rogacz rig from Synfig."""

import argparse
import json
import math
from pathlib import Path
from xml.etree import ElementTree as ET

from PIL import Image

from cinderhorn_paint import CELL, KEYS, ORDER, PARTS
from cinderhorn_visuals import hoof_motion
from fire_imp_synfig_rig import canvas, save
from hero_synfig import group, image
from caldera_puppet_motion import transform
import synfig_cli

FRAMES = 60


def author(root, paintings):
    project = canvas("Rogacz Żaru / klucze obrotu i chodu", 160, 160, (80, 144), FRAMES)
    project.find("name").text = "Rogacz Żaru / chód i pochylenie głowy"
    curves = ET.SubElement(project, "defs")
    for name, keys in {
        "walk": [hoof_motion(f / FRAMES, 0) for f in range(FRAMES + 1)],
        "rush": [hoof_motion(f / FRAMES, 0, 73, .33, 15) for f in range(FRAMES + 1)],
        "lower": [(0, 0), (4, -1.5), (9, -6.2), (12, -11), (11, -10), (2, -4.0), (0, 0)],
    }.items():
        node = ET.SubElement(curves, "animated", id=name, type="vector")
        last = len(keys) - 1
        for i, (x, y) in enumerate(keys):
            key = ET.SubElement(node, "waypoint", time=f"{i * FRAMES // last}f", before="linear", after="linear")
            v = ET.SubElement(key, "vector")
            ET.SubElement(v, "x").text = f"{x:.6f}"
            ET.SubElement(v, "y").text = f"{y:.6f}"
    atlas = Image.open(paintings)
    (root / "parts").mkdir(exist_ok=True)
    for part in ORDER[1]:
        x, y = KEYS.index(1) * CELL, part * CELL
        atlas.crop((x, y, x + CELL, y + CELL)).save(root / "parts" / f"{part}.png")
        layer = group(project, PARTS[part])
        image(layer, str(part), (-80, 144, 80, -16))
        layer[-1].find("./param[@name='filename']/string").text = f"parts/{part}.png"
        if part != 2 and part != 3:
            touchdown = {0: .5, 1: 0, 4: .75, 5: .25}[part]
            feet = [hoof_motion(f / FRAMES, touchdown) for f in range(FRAMES + 1)]
            transform(layer, "translate", "Tor kopyta", "origin",
                      [(x * .86, y - x * .43) for x, y in feet], "vector")
        else:
            transform(layer, "translate", "Ruch grzbietu", "origin",
                      [(0, 1.25 * math.cos(4 * math.pi * f / FRAMES)) for f in range(FRAMES + 1)], "vector")
    source = root / "rogacz-chod.sif"
    save(project, source)
    return source


def export(source, header):
    lines = ["#pragma once", "// Generated from Synfig by cinderhorn_turn_synfig.py.",
             "namespace ember::cinderhorn_synfig {", "struct Key {float time, along, lift;};"]
    for node in ET.parse(source).findall("./defs/animated"):
        keys = [f"{{{int(w.get('time')[:-1]) / FRAMES:.8f}f,{float(w.findtext('vector/x')):.6f}f,"
                f"{float(w.findtext('vector/y')):.6f}f}}" for w in node]
        if not keys:
            raise ValueError("Missing Synfig waypoints")
        lines += [f"inline constexpr Key {node.get('id')}[]={{", " " + ",\n ".join(keys) + "};"]
    header.write_text("\n".join(lines + ["}", ""]), encoding="utf-8")


if __name__ == "__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("source", type=Path)
    cli.add_argument("header", type=Path)
    cli.add_argument("--paintings", type=Path)
    cli.add_argument("--render", type=Path)
    cli.add_argument("--no-audio", action="store_true", required=True)
    a = cli.parse_args()
    if a.paintings:
        a.source.parent.mkdir(parents=True, exist_ok=True)
        assert a.source == author(a.source.parent, a.paintings)
    export(a.source, a.header)
    if a.render:
        receipt = synfig_cli.render(a.source.resolve(), output_root=a.render, threads=2, expected_frames=61, require_motion=True)
        (a.render / "synfig.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")

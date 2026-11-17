"""Build the desert gait controls in Synfig and export their actual waypoints."""

import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

import synfig_cli
from wolf_motion_synfig import control

FRAMES = 60
SUPPORT = .60


def gait(phase):
    if phase <= SUPPORT:
        return 1 - 2 * phase / SUPPORT, 0
    swing = (phase - SUPPORT) / (1 - SUPPORT)
    tangent = -2 * (1 - SUPPORT) / SUPPORT
    travel = -1 + 2 * swing**2 * (3 - 2 * swing) + tangent * swing * (1-swing) * (1-2*swing)
    return travel, math.sin(math.pi * swing)**2


def author(path):
    canvas = ET.Element("canvas", version="1.2", width="260", height="160", fps="60", **{
        "view-box": "-28 17 28 -17", "antialias": "2", "begin-time": "0f",
        "end-time": "60f", "bgcolor": "0 0 0 0",
    })
    ET.SubElement(canvas, "name").text = "Emberfall / Chód skarabeusza i wężykowanie kobry"
    phases = [(frame % FRAMES) / FRAMES for frame in range(FRAMES + 1)]
    foot = [gait(phase) for phase in phases]
    spine = [math.sin(math.tau * phase) + .09 * math.sin(2 * math.tau * phase) for phase in phases]
    control(canvas, "Skarabeusz / Rozkrok i oderwanie stopy",
            [(x * 16, y * 9 + 3) for x, y in foot], 2, (.72, .51, .29))
    control(canvas, "Kobra / Fala boczna",
            [(x * 18, -8) for x in spine], 2, (.79, .65, .35))
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def export(path, header):
    tracks = []
    times = [f"{frame}f" for frame in range(FRAMES + 1)]
    for layer in ET.parse(path).getroot().findall("layer"):
        points = layer.findall("param[@name='origin']/animated/waypoint")
        if [point.get("time") for point in points] != times:
            raise ValueError("Missing gait waypoint")
        tracks.append([(float(p.findtext("vector/x")), float(p.findtext("vector/y"))) for p in points])
    rows = [(x/16, (y-3)/9, sway/18) for (x, y), (sway, _) in zip(*tracks, strict=True)]
    if rows[0] != rows[-1]:
        raise ValueError("Gait does not close")
    lines = ["#pragma once", "// Exported from native Synfig waypoints by desert_locomotion_synfig.py.",
             "namespace ember::desert_motion::data {", "struct Key {float travel,lift,sway;};",
             f"inline constexpr float Support={SUPPORT:.2f}f;", "inline constexpr Key samples[]={"]
    lines.extend(" {" + ", ".join(f"{value:.7f}f" for value in row) + "}," for row in rows)
    lines.extend(("};", "}"))
    header.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True)
    source = args.root / "desert-locomotion.sif"
    author(source)
    receipt = synfig_cli.render(source, output_root=args.root / "renders", threads=2,
                                expected_frames=FRAMES+1, require_motion=True)
    export(source, args.header)
    receipt.update(HeaderSha256=synfig_cli.sha(args.header), Support=SUPPORT)
    (args.root / "report.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

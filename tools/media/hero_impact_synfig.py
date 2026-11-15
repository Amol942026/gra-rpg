"""Author the hero's contact flash and particle envelope in Synfig."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

from hero_hit_reaction_synfig import FPS, samples
import synfig_cli
from wolf_motion_synfig import control


FRAMES = 16
TRACKS = (
    ("Rozrzut drobin", (.91, .53, .28),
     ((0, 0), (1, .19), (3, .49), (5, .68), (8, .85), (11, .94), (14, .986), (16, 1))),
    ("Refleks w chwili kontaktu", (.97, .78, .42),
     ((0, 1), (1, .56), (2, .23), (3, .07), (4, .015), (5, 0), (16, 0))),
    ("Wygaszanie drobin", (.69, .84, .91),
     ((0, 1), (3, .99), (5, .96), (7, .82), (9, .59), (11, .38), (13, .20), (15, .055), (16, 0))),
)
CONTROL_MIN = -26
CONTROL_RANGE = 52


def author(path):
    canvas = ET.Element("canvas", version="1.2", width="240", height="192", fps=str(FPS), **{
        "view-box": "-34 30 34 -24.4", "antialias": "2", "begin-time": "0f",
        "end-time": f"{FRAMES}f", "bgcolor": "0 0 0 0",
    })
    ET.SubElement(canvas, "name").text = "Emberfall / Wędrowiec / Odprysk po trafieniu"
    ET.SubElement(canvas, "desc").text = "Sterowanie rozrzutem, poświatą i wygaszeniem efektu trafienia."
    for index, (label, color, marks) in enumerate(TRACKS):
        positions = [(CONTROL_MIN + value * CONTROL_RANGE, 18 - index * 15) for value in samples(marks)]
        control(canvas, label, positions, 2.6, color)
    ET.indent(canvas)
    with path.open("xb") as stream:
        ET.ElementTree(canvas).write(stream, encoding="utf-8", xml_declaration=True)


def export_header(source, header):
    times = [f"{frame}f" for frame in range(FRAMES + 1)]
    columns = []
    for layer, (label, _, _) in zip(ET.parse(source).getroot().findall("layer"), TRACKS, strict=True):
        points = layer.findall("param[@name='origin']/animated/waypoint")
        if layer.get("desc") != label or [point.get("time") for point in points] != times:
            raise ValueError(f"Unexpected Synfig waypoints for '{label}'")
        values = [(float(point.findtext("vector/x")) - CONTROL_MIN) / CONTROL_RANGE for point in points]
        if any(not 0 <= value <= 1 for value in values):
            raise ValueError(f"Synfig control '{label}' is outside 0–1")
        columns.append(values)
    lines = ["#pragma once", "// Generated from native Synfig waypoints by tools/media/hero_impact_synfig.py.",
             "namespace ember::hero_impact::data {", "struct Key {float spread,glow,fade;};",
             f"inline constexpr int Frames={FRAMES};", f"inline constexpr float Duration={FRAMES}.f/{FPS};",
             "inline constexpr Key samples[]={"]
    for row in zip(*columns, strict=True):
        lines.append(" {" + ", ".join(f"{value:.6f}f" for value in row) + "},")
    lines.extend(("};", "}"))
    header.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True)
    source = args.root / "hero-impact.sif"
    author(source)
    receipt = synfig_cli.render(source, output_root=args.root / "renders", threads=2,
                                expected_frames=FRAMES + 1, require_motion=True)
    export_header(source, args.header)
    receipt.update(DurationSeconds=FRAMES / FPS, ControlSamples=FRAMES + 1,
                   HeaderSha256=synfig_cli.sha(args.header))
    (args.root / "report.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

"""Author the treant's kneel and the bandit's backward fall as Synfig controls."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

import synfig_cli
from wolf_motion_synfig import control


# Keys are at 60 Hz and run up to the first full-body ground contact.
# Treant: near knee, far knee, trunk pitch, bracing wrist.
# Bandit: knee buckle, hip rotation, chest lead, empty wrist, sword wrist.
CLIPS = {
    "treant": (("Kolano bliskie", "Kolano dalekie", "Pochylenie pnia", "Podparcie ręką"), (
        (0, 0, 0, 0, 0), (2, .30, .12, .035, .05), (4, .94, .65, .10, .38),
        (6, 1, 1, .24, .83), (8, 1, 1, .59, 1), (10, 1, 1, .89, .71), (12, 1, 1, 1, .48),
    )),
    "bandit": (("Ugięcie kolan", "Obrót bioder", "Odchylenie klatki", "Wolna ręka", "Ręka z mieczem"), (
        (0, 0, 0, 0, 0, 0), (2, .12, .02, .085, .16, .04),
        (4, .58, .12, .26, .47, .16), (6, 1, .33, .47, .81, .36),
        (9, .75, .68, .79, 1, .65), (12, .28, .94, .96, .78, .94), (15, 0, 1, 1, .66, 1),
    )),
}


def sample(keys, frame):
    a, b = next((a, b) for a, b in zip(keys, keys[1:]) if frame <= b[0])
    t = (frame - a[0]) / (b[0] - a[0])
    t = t * t * (3 - 2 * t)
    return [start + (end - start) * t for start, end in zip(a[1:], b[1:])]


def author(directory, header, render):
    receipt = {}
    lines = ["#pragma once", "// Exported from Synfig by tools/media/forest_death_synfig.py."]
    for name, (labels, keys) in CLIPS.items():
        frames = keys[-1][0] + 1
        canvas = ET.Element("canvas", version="1.2", width="420", height="260", fps="60",
                            **{"view-box": "-7 9 31 -10", "antialias": "2", "begin-time": "0f",
                               "end-time": f"{frames - 1}f", "bgcolor": "0 0 0 0"})
        ET.SubElement(canvas, "name").text = f"Emberfall / {'Drzewiec' if name == 'treant' else 'Rozbójnik'}"
        for i, label in enumerate(labels):
            points = [(sample(keys, frame)[i] * 25, 6 - i * 3.4) for frame in range(frames)]
            control(canvas, label, points, .6, (.49 + i * .09, .72, .41 + i * .12))
        project = directory / f"{name}.sif"
        ET.indent(canvas)
        ET.ElementTree(canvas).write(project, encoding="utf-8", xml_declaration=True)
        if render:
            receipt[name] = synfig_cli.render(project, output_root=directory / "renders", threads=2,
                                               expected_frames=frames, require_motion=True)
        layers = ET.parse(project).findall("layer")
        points = [[float(w.findtext("vector/x")) / 25
                   for w in layer.findall("param[@name='origin']/animated/waypoint")] for layer in layers]
        lines.append(f"inline constexpr std::array<std::array<float,{len(labels)}>,{frames}> {name}Fall{{{{")
        lines.extend(" {"+",".join(f"{v:.6f}f" for v in row)+"}," for row in zip(*points))
        lines.append("}};")
    header.write_text("\n".join(lines) + "\n", encoding="utf-8")
    (directory / "report.json").write_text(json.dumps(receipt, indent=2, ensure_ascii=False), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    author(args.root, args.header, True)

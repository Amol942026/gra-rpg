"""Author and validate the imp's held pose, stride and cinder cast in Synfig."""

import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from hero_hit_reaction_synfig import samples
from wolf_motion_synfig import control, walking_foot
import synfig_cli


FRAMES = 60
CHANNELS = ("travel", "lift", "rise", "tilt", "tail", "guard", "cast", "reach", "ember")
CAST = {
    "rise": ((0, 0), (14, -3.2), (22, -5), (27, 3.1), (33, 1.2), (46, -1.6), (60, 0)),
    "tilt": ((0, 0), (18, -8), (22, -13), (27, 12), (35, 7), (49, -2.4), (60, 0)),
    "tail": ((0, 0), (15, 10), (23, 20), (29, -11), (36, -18), (48, 5), (60, 0)),
    "guard": ((0, 0), (11, 23), (22, 45), (28, 20), (36, -14), (50, 6), (60, 0)),
    "cast": ((0, 0), (8, 17), (17, 34), (22, 14), (27, 81), (32, 88), (41, 40), (60, 0)),
    "reach": ((0, 0), (15, -3), (22, -6), (27, 6), (33, 8.1), (45, 1.5), (60, 0)),
    "ember": ((0, 0), (10, .15), (18, .55), (24, 1), (26, .89), (27, 0), (60, 0)),
}


def poses(clip):
    if clip == "cast":
        axes = [list(samples(CAST[field])) if field in CAST else [0] * (FRAMES + 1) for field in CHANNELS]
        return list(zip(*axes, strict=True))
    result = []
    for frame in range(FRAMES):
        phase = frame / FRAMES
        a = math.tau * phase
        if clip == "walk":
            travel, lift = walking_foot(phase)
            result.append((travel * (8.12 / 17.6), lift * 1.08, .9 * math.sin(2*a - .6),
                           5.5 * math.sin(a), 8.2 * math.sin(a-.7), 14 * math.sin(a+.28),
                           -17 * math.sin(a+.28), 0, 0))
        else:
            result.append((0, 0, .75 * (1-math.cos(a)), 1.4*math.sin(a),
                           5.3*math.sin(a-.8), 2.1*math.sin(a-.2), 2.7*math.sin(a+.35), 0, 0))
    return [*result, result[0]]


def author(path, clip):
    canvas = ET.Element("canvas", version="1.2", width="280", height="236", fps="60",
                        **{"view-box": "-65 30 65 -65", "antialias": "2", "begin-time": "0f",
                           "end-time": f"{FRAMES}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / Ognisty chochlik / " + clip
    for index, (name, axis) in enumerate(zip(CHANNELS, zip(*poses(clip)), strict=True)):
        control(canvas, name, [(value*.52, 24-index*10) for value in axis], 2.1,
                ((.87, .49, .30), (.95, .75, .46), (.67, .72, .77))[index % 3])
    ET.indent(canvas)
    with path.open("xb") as stream:
        ET.ElementTree(canvas).write(stream, encoding="utf-8", xml_declaration=True)


def read_tracks(path):
    columns = []
    for name, layer in zip(CHANNELS, ET.parse(path).getroot().findall("layer"), strict=True):
        points = layer.findall("param[@name='origin']/animated/waypoint")
        if layer.get("desc") != name or [item.get("time") for item in points] != [f"{i}f" for i in range(61)]:
            raise ValueError(f"Incomplete Synfig track {name} in {path}")
        columns.append([float(point.findtext("vector/x"))/.52 for point in points])
    return list(zip(*columns, strict=True))


def export(header, clips):
    if clips["cast"][27][-1] or not all(rows[0] == rows[-1] for rows in clips.values()):
        raise ValueError("A cast must release at .45 and every timeline must return to its first pose")
    key = ",".join(CHANNELS)
    source = ["#pragma once", "// Exported by fire_imp_motion_synfig.py from native Synfig waypoints.",
              "namespace ember::fire_imp_motion {", f"struct Key {{float {key};}};"]
    for clip, rows in clips.items():
        source.append(f"inline constexpr Key {clip}[]={{")
        source.extend(" {" + ", ".join(f"{value:.5f}f" for value in row) + "}," for row in rows)
        source.append("};")
    source.append("}")
    header.write_text("\n".join(source) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("header", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    clips, reports = {}, {}
    for name in ("idle", "walk", "cast"):
        path = args.root / f"imp-{name}.sif"
        author(path, name)
        reports[name] = synfig_cli.render(path, output_root=args.root/"renders", threads=2,
                                          expected_frames=61, require_motion=True)
        clips[name] = read_tracks(path)
    export(args.header, clips)
    (args.root / "report.json").write_text(json.dumps(reports, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

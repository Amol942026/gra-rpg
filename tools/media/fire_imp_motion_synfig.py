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
    "rise": ((0,0),(8,-.5),(15,-1.55),(20,-2.15),(23,-1.5),(27,.75),(32,1.65),(40,.45),(49,-.4),(60,0)),
    "tilt": ((0,0),(9,-1.3),(16,-3.6),(20,-4.7),(23,-3.1),(27,2.8),(32,5.4),(39,3.7),(51,-.8),(60,0)),
    "tail": ((0,0),(8,-1),(16,5),(24,16),(29,15),(37,-12),(43,-15),(52,2),(60,0)),
    # Both shoulders rotate toward a shared target; guard is the hand-cupping weight.
    "guard": ((0,0),(4,.13),(8,.55),(13,.94),(17,1),(26,1),(28,.974),(32,.845),(37,.683),(42,.517),(48,.327),(55,.11),(60,0)),
    # The thrust advances the cupped hands as one, then lets the fingers fall away.
    "cast": ((0,0),(9,0),(17,-.18),(20,-.21),(23,.12),(27,.88),(30,1),(36,.81),(44,.38),(53,.055),(60,0)),
    "reach": ((0,0),(8,-.45),(16,-1.2),(20,-1.65),(23,-.9),(27,1.75),(32,2.6),(41,1.25),(50,-.22),(60,0)),
    "ember": ((0,0),(5,0),(8,.15),(12,.40),(16,.65),(20,.87),(23,.98),(26,1),(27,1),(28,0),(60,0)),
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
            # The supporting hoof covers the ground distance of one half-step.
            # The hips recover after heel contact, the long tail follows the pelvis.
            weight = 1.7 * math.cos(2*a-.72) + .28 * math.sin(4*a-.4)
            arm = math.sin(a+.19)
            result.append((travel * (8.12 / 17.6), lift * 1.22, weight,
                           4.8 * math.sin(a-.16), 9.6 * math.sin(a-.98)+2.2*math.sin(2*a-.75),
                           19.4*arm-1.8*math.sin(2*a), -21.2*arm+2.2*math.sin(2*a+.32), 0, 0))
        else:
            breath=.77*(1-math.cos(a))+.15*(1-math.cos(2*a))
            result.append((0, 0, breath, 1.5*math.sin(a-.30),
                           6.7*math.sin(a-.88)+1.15*math.sin(2*a-.54),
                           2.4*math.sin(a-.33), 3.4*math.sin(a+.39), 0, 0))
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
    if clips["cast"][27][-1]<.99 or clips["cast"][28][-1] or not all(rows[0] == rows[-1] for rows in clips.values()):
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

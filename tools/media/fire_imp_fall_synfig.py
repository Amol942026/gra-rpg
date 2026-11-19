"""Render and export the fire imp's staged fall from a native Synfig project."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

from enemy_death_synfig import CLIPS, PLAYBACK_FPS
from hero_hit_reaction_synfig import samples
from wolf_motion_synfig import control
import synfig_cli

CONTACT = next(clip[3] for clip in CLIPS if clip[0] == "Ognisty chochlik")
TRACKS = (
    ("Przechył na bok", "roll", 16, ((0, 0), (2, -.04), (5, -.009), (8, .17), (11, .43),
        (14, .69), (17, .88), (20, .972), (22, .997), (23, 1))),
    ("Utrata podparcia", "sag", 13, ((0, 0), (3, .13), (7, .82), (10, 1),
        (13, .74), (17, .21), (21, 0), (23, 0))),
    ("Skręt klatki piersiowej", "chest", .9, ((0, 0), (3, -8), (6, 2), (10, 17),
        (13, 15), (16, 2), (19, -3), (21, -.9), (23, 0))),
    ("Bliższe kolano", "nearKnee", 14, ((0, 0), (2, .10), (5, .82), (8, 1),
        (11, .69), (15, .25), (20, 0), (23, 0))),
    ("Dalsze kolano", "farKnee", 14, ((0, 0), (4, .03), (8, .46), (11, .83),
        (14, .47), (18, .15), (22, 0), (23, 0))),
    ("Bliższe ramię", "nearArm", .58, ((0, 0), (2, -12), (5, -40), (9, -28),
        (13, 16), (17, 9), (21, -2), (23, 0))),
    ("Dalsze ramię", "farArm", .53, ((0, 0), (4, 6), (8, 26), (12, 34),
        (16, 18), (19, -4), (22, 1), (23, 0))),
    ("Ogon", "tail", .65, ((0, 0), (4, 4), (8, 18), (10, 24), (13, 9),
        (16, -17), (18, -22), (20, -9), (22, 1.5), (23, 0))),
)


def author(path):
    canvas = ET.Element("canvas", version="1.2", width="260", height="220", fps=str(PLAYBACK_FPS),
                        **{"view-box": "-23 37 23 -37", "antialias": "2", "begin-time": "0f",
                           "end-time": f"{CONTACT}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / Ognisty chochlik / Upadek na bok"
    for index, (label, _, scale, marks) in enumerate(TRACKS):
        if marks[-1][0] != CONTACT:
            raise ValueError("Fire imp curve differs from the ground contact frame")
        points = [(value * scale, 31-index*9) for value in samples(marks)]
        control(canvas, label, points, 1.25, ((.91,.57,.34),(.76,.86,.91),(.86,.69,.38))[index%3])
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def export(path, header):
    axes = []
    times = [f"{frame}f" for frame in range(CONTACT+1)]
    for layer, (label, _, scale, _) in zip(ET.parse(path).getroot().findall("layer"), TRACKS, strict=True):
        points = layer.findall("param[@name='origin']/animated/waypoint")
        if layer.get("desc") != label or [point.get("time") for point in points] != times:
            raise ValueError(f"Incomplete Synfig waypoints for {label}")
        axes.append([float(point.findtext("vector/x"))/scale for point in points])
    poses = list(zip(*axes, strict=True))
    if any(poses[0]) or poses[-1] != (1,0,0,0,0,0,0,0):
        raise ValueError("Fire imp must begin in its living pose and finish in its fixed resting pose")
    fields = ",".join(track[1] for track in TRACKS)
    lines = ["#pragma once", "// Native Synfig controls exported by fire_imp_fall_synfig.py.",
             "namespace ember::fire_imp_fall {", f"struct Key {{float {fields};}};",
             f"inline constexpr int ContactFrame={CONTACT};", "inline constexpr Key keys[]={"]
    lines += [" {" + ", ".join(f"{value:.6f}f" for value in row) + "}," for row in poses]
    lines += ["};", "}"]
    header.write_text("\n".join(lines)+"\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True)
    path = args.root/"fire-imp-fall.sif"
    author(path)
    receipt = synfig_cli.render(path, output_root=args.root/"renders", threads=2,
                                expected_frames=CONTACT+1, require_motion=True)
    export(path, args.header)
    receipt.update(HeaderSha256=synfig_cli.sha(args.header), ContactSeconds=CONTACT/PLAYBACK_FPS)
    (args.root/"report.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

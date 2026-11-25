"""Author the death timing controls in Synfig and export the game's keys."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

from wolf_motion_synfig import control
import synfig_cli


# Name, duration, recoil peak, ground contact, and a hold waypoint in frames.
# Play the same control points slightly slower while preserving their rhythm.
PLAYBACK_FPS = 52
# Large, heavy bodies give way first. The contact pose is held while the
# material-specific landing particles disperse.
CLIPS = (
    ("Szlam", 17, 2, 11, 14), ("Wilk", 20, 2, 14, 17),
    ("Szkielet", 20, 2, 14, 17), ("Kamienny Strażnik", 26, 1, 16, 21),
    ("Dzik", 19, 2, 13, 16), ("Leśny pająk", 22, 2, 16, 19),
    ("Leśny rozbójnik", 21, 2, 15, 18), ("Ożywione drzewo", 18, 1, 12, 15),
    ("Skorpion bursztynowy", 20, 2, 14, 17), ("Skarabeusz miedziany", 38, 3, 25, 31),
    ("Kobra piaskowa", 22, 2, 16, 19), ("Żywa Wydma", 15, 1, 9, 12),
    ("Żużlowy pełzacz", 20, 2, 14, 17), ("Salamandra lawowa", 21, 2, 15, 18),
    ("Ognisty chochlik", 30, 2, 23, 26), ("Popielny upiór", 30, 2, 24, 27),
    ("Obsydianowy wojownik", 31, 1, 25, 28), ("Żarowy drak", 17, 1, 11, 14),
    ("Varkhor", 45, 1, 39, 42), ("Zarodnik kniei", 22, 2, 16, 19),
    ("Oko Wydm", 19, 2, 13, 16), ("Rogacz Żaru", 17, 1, 11, 14),
    ("Szakal wydmowy", 32, 2, 23, 27), ("Szklany żniwiarz", 38, 2, 27, 31),
)
GUARDIAN_SMOKE = (
    (0, 0, 0, 0), (4, 0, 0, 0), (12, .86, .36, .34),
    (17, 1, .62, .61), (27, .64, .88, .85), (47, 0, 1, 1),
)


def keys(clip):
    name, end, recoil, ground, hold = clip
    release = recoil + max(2, (ground-recoil)//2)
    # Let the weight shift onto the supporting joints before the ground cue.
    release_tilt = {
        "Ożywione drzewo": .39,
        "Leśny rozbójnik": .38,
        "Kamienny Strażnik": .36,
        "Skarabeusz miedziany": .30,
        "Żużlowy pełzacz": .46,
        "Salamandra lawowa": .44,
        "Ognisty chochlik": .47,
        "Popielny upiór": .43,
        "Obsydianowy wojownik": .48,
        "Żarowy drak": .43,
        "Varkhor": .42,
    }.get(name, .31)
    return (
        (0, 0, 0, 0), (recoil, .045, 1, 0), (release, release_tilt, .13, 0),
        (ground, 1, 0, 0), (hold, 1, 0, 0), (end, 1, 0, 0),
    )


def motion_tracks():
    for clip in CLIPS:
        yield clip[0], keys(clip), ("upadek", "reakcja", "osiadanie")
    yield "Kamienny Strażnik / dym", GUARDIAN_SMOKE, ("krycie", "rozrost", "unoszenie")


def author(path):
    frames = max(marks[-1][0] for _, marks, _ in motion_tracks()) + 1
    canvas = ET.Element("canvas", version="1.2", width="420", height="475", fps=str(PLAYBACK_FPS), **{
        "view-box": "-30 45 30 -54", "antialias": "2", "begin-time": "0f",
        "end-time": f"{frames-1}f", "bgcolor": "0 0 0 0",
    })
    ET.SubElement(canvas, "name").text = "Emberfall / Upadki przeciwników"
    ET.SubElement(canvas, "desc").text = "Upadki przeciwników i chmura dymu Kamiennego Strażnika."
    for i, (name, marks, axes) in enumerate(motion_tracks()):
        for index, axis in enumerate(axes):
            row = 42-(i//2)*8
            origin = -27+(i%2)*30
            values = [(origin+marks[0][index+1]*25, row-index*1.75)] * frames
            for a, b in zip(marks, marks[1:]):
                for frame in range(a[0], b[0]+1):
                    t = (frame-a[0])/(b[0]-a[0])
                    t = t*t*(3-2*t)
                    values[frame] = (origin+25*(a[index+1]+(b[index+1]-a[index+1])*t), row-index*1.75)
            for frame in range(marks[-1][0], frames):
                values[frame] = (origin+25*marks[-1][index+1], row-index*1.75)
            control(canvas, f"{name} / {axis}", values, .57,
                    ((.46,.78,.64),(.93,.59,.30),(.58,.56,.91))[index])
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path,encoding="utf-8",xml_declaration=True)
    return frames


def export(path, header):
    # Read visible native circle waypoints at the original authored timing
    # markers. The control trajectories remain independently editable in SIF.
    canvas = ET.parse(path).getroot()
    layers = canvas.findall("layer")
    fps = float(canvas.attrib["fps"])
    if fps <= 0:
        raise ValueError("Synfig frame rate must be positive")
    lines = ["#pragma once", "// Synfig controls exported by tools/media/enemy_death_synfig.py.",
             "namespace ember::enemy_death::data {", f"inline constexpr float framesPerSecond={fps:.5f}f;",
             "inline constexpr Track tracks[]={"]
    for i, (name, marks, _) in enumerate(motion_tracks()):
        layer_set = layers[3*i:3*i+3]
        tracks = [[float(w.findtext("vector/x")) for w in layer.findall("param[@name='origin']/animated/waypoint")]
                  for layer in layer_set]
        frames = [key[0] for key in marks]
        origin = tracks[0][0]
        if not frames or frames[-1] <= 0 or any(len(track) <= frames[-1] for track in tracks):
            raise ValueError(f"Incomplete controls for {name}")
        def number(x):
            return f"{x:.5f}f"
        records = ["{"+",".join([str(frame),*(number((track[frame]-origin)/25) for track in tracks)])+"}"
                   for frame in frames]
        if i < len(CLIPS):
            lines.append(" {{"+",".join(records)+"}}, // "+name)
        else:
            lines.extend(["};", "inline constexpr Track guardianSmoke={{"+",".join(records)+"}};"])
    lines.append("}")
    header.write_text("\n".join(lines)+"\n",encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--header",type=Path,required=True)
    parser.add_argument("--export-only",action="store_true")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    path = args.root/"upadki-przeciwnikow.sif"
    if not args.export_only:
        args.root.mkdir(parents=True,exist_ok=False)
        frames = author(path)
        receipt = synfig_cli.render(path,output_root=args.root/"renders",threads=2,
                                    expected_frames=frames,require_motion=True)
        (args.root/"render-receipt.json").write_text(json.dumps(receipt,indent=2),encoding="utf-8")
    export(path,args.header)


if __name__ == "__main__":
    main()

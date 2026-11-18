"""Author the Ash Wraith's cloth fold in Synfig and export six motion tracks."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

import synfig_cli
from wolf_motion_synfig import control


FPS = 52
CONTACT = 24
FRAMES = (0, 2, 6, 11, 17, CONTACT)
TRACKS = (
    ("Rąbek",             (0, 0, .29, .73, .97, 1)),
    ("Fałdy na tułowiu",  (0, 0, .09, .40, .84, 1)),
    ("Pusty kaptur",      (0, 0, .035, .23, .69, 1)),
    ("Rękawy",            (0, .02, .12, .40, .81, 1)),
    ("Falowanie",         (0, .11, .37, .66, .26, 0)),
    ("Zanik światła",     (0, .08, .66, .98, 1, 1)),
)


def author(path):
    canvas = ET.Element("canvas", version="1.2", width="480", height="280",
                        fps=str(FPS), antialias="2", **{
                            "view-box": "-24 14 24 -14", "begin-time": "0f",
                            "end-time": f"{CONTACT}f", "bgcolor": "0 0 0 0",
                        })
    ET.SubElement(canvas, "name").text = "Emberfall / Szata Popielnego Upiora"
    ET.SubElement(canvas, "desc").text = "Krzywe układania materiału od rąbka po pusty kaptur."
    for index, (name, marks) in enumerate(TRACKS):
        position = [0.] * (CONTACT + 1)
        for first, last, a, b in zip(FRAMES, FRAMES[1:], marks, marks[1:]):
            for frame in range(first, last + 1):
                t = (frame - first) / (last - first)
                weight = t*t*(3-2*t)
                if last == CONTACT:
                    # The hem is already supported. Let friction arrest the
                    # remaining long hood and sleeve travel before touchdown.
                    weight = 1-(1-weight)**2
                position[frame] = a + (b-a) * weight
        points = [(-20+30*x, 11-index*4.3) for x in position]
        color = (.46 + index*.051, .70 - index*.055, .82 - index*.048)
        control(canvas, name, points, .65, color)
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def export(path, header):
    tracks = []
    for layer in ET.parse(path).findall("layer"):
        values = layer.findall("param[@name='origin']/animated/waypoint/vector/x")
        tracks.append([(float(value.text)+20)/30 for value in values])
    if len(tracks) != len(TRACKS) or any(len(t) != CONTACT+1 for t in tracks):
        raise ValueError("Incomplete cloth controls")
    lines = ["#pragma once", "// Exported by tools/media/wraith_fold_synfig.py.",
             "namespace wraith_death_art::motion {", "inline constexpr Key keys[]={"]
    for frame, values in enumerate(zip(*tracks)):
        lines.append(" {"+", ".join(f"{value:.5f}f" for value in values)+ "},"
                     + (" // contact" if frame == CONTACT else ""))
    lines.extend(["};", "}"])
    header.write_text("\n".join(lines)+"\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    source = args.root / "szata-popielnego-upiora.sif"
    author(source)
    receipt = synfig_cli.render(source, output_root=args.root/"render", threads=2,
                                expected_frames=CONTACT+1, require_motion=True)
    export(source, args.header)
    (args.root/"render-receipt.json").write_text(json.dumps(receipt, indent=2),
                                                encoding="utf-8")


if __name__ == "__main__":
    main()

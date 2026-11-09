"""Export the forest creatures' secondary animation from native Synfig tracks."""
import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image

from bandit_motion import UNITS, translate
from hero_synfig import group, image
import synfig_cli


FRAMES = 36
SECTIONS = ("idle", "move", "strike")
TRACKS = ("body", "action", "follow")
SPECIES = {
    # Squash, body lean, attack extension, rise, secondary sway, twist.
    "slime": ((.045, .65, 0, .25, .65, .85), (.075, 2.3, 0, .5, 1.8, 1.1), (-.22, 8.5, 7.5, 1.8, 2, .9)),
    "wolf": ((.03, .38, .36, .45, 2.7, .85), (.05, 1.35, 1.2, 1.18, 3.1, 1.8), (-.08, 4.7, 8.3, -2.6, -2.8, 3.1)),
    "skeleton": ((.03, .46, .38, .34, .67, .87), (.04, 1, 5.1, .85, 1.8, 1.4), (-.09, 5.5, 19, 7, -3.3, 4.5)),
    "treant": ((.023, .5, .55, .45, 1.45, 1.0), (.035, 1.3, 4.1, 1.0, 3.3, 1.7), (-.07, 4.0, 23, 7.0, -4.2, 3.5)),
}


def keys(species):
    for section, amplitudes in zip(SECTIONS, SPECIES[species]):
        for index in range(FRAMES+1):
            t = index/FRAMES
            if section == "strike":
                gain = t*t*(3-2*t)
                weights = (gain,)*6
            else:
                phase = t*math.tau
                lag = .48 if section == "move" else .20
                weights = (math.cos(phase*2), math.sin(phase), math.sin(phase),
                           math.sin(phase*2), math.sin(phase-lag), math.cos(phase-lag))
            yield tuple(value*weight for value, weight in zip(amplitudes, weights))


def author(root, species):
    values = list(keys(species))
    canvas = ET.Element("canvas", version="1.2", width="240", height="256", fps="60",
                        **{"view-box": "-1.2 1.28 1.2 -1.28", "antialias": "2",
                           "begin-time": "0f", "end-time": f"{len(values)-1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / forest creatures / "+species
    ET.SubElement(canvas, "desc").text = "Controls for idle, locomotion, and the strike intensity."
    for index, track in enumerate(TRACKS):
        part = group(canvas, track)
        if track == "body":
            image(part, species, (-.73, 1.03, .73, -.95))
        else:
            image(part, species+"-"+track, (-.40+index*.14, .42, .19+index*.14, -.45))
        translate(part, track, [key[index*2:index*2+2] for key in values])
    path = root / (species+".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def read_tracks(path):
    tracks = {}
    for layer in ET.parse(path).getroot().iter("layer"):
        if layer.get("type") != "translate":
            continue
        name = layer.get("desc", "").removeprefix("runtime:")
        tracks[name] = [(float(w.findtext("vector/x"))*UNITS, float(w.findtext("vector/y"))*UNITS)
                        for w in layer.findall("param[@name='origin']/animated/waypoint")]
    count = len(SECTIONS)*(FRAMES+1)
    if set(tracks) != set(TRACKS) or any(len(track) != count for track in tracks.values()):
        raise ValueError(f"Missing native tracks in {path}")
    return [tuple(value for name in TRACKS for value in tracks[name][i]) for i in range(count)]


def cpp_array(species, path):
    rows = ["{"+",".join(f"{v:.6f}f" for v in key)+"}," for key in read_tracks(path)]
    return "\n".join([f"inline constexpr Key {species}[]={{", *rows, "};"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--concepts", type=Path, required=True)
    parser.add_argument("--synfig", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    parts = args.root/"parts"
    parts.mkdir()
    synfig_cli.SYNFIG = args.synfig
    lines = ["#pragma once", "// Native Synfig waypoints from tools/media/forest_motion.py.",
             "namespace ember::forest_motion {",
             f"inline constexpr int Frames={FRAMES};",
             "struct Key {float squash,lean,reach,rise,drift,twist;};"]
    renders = []
    for species in SPECIES:
        with Image.open(args.concepts/(species+".png")) as concept:
            concept.crop((60, 24, 452, 490)).save(parts/(species+".png"))
            concept.crop((188, 160, 311, 320)).save(parts/(species+"-action.png"))
            concept.crop((206, 84, 290, 193)).save(parts/(species+"-follow.png"))
        project = author(args.root, species)
        lines.append(cpp_array(species, project))
        renders.append(synfig_cli.render(project, output_root=args.root/"renders", threads=2,
                         expected_frames=len(SECTIONS)*(FRAMES+1), require_motion=True))
    lines.append("}")
    (args.root/"forest_motion_data.hpp").write_text("\n".join(lines)+"\n", encoding="utf-8")
    report = {"status":"PASS", "sections":SECTIONS, "renders":renders,
              "frames":sum(item["ActualFrames"] for item in renders), "audio_played":False}
    (args.root/"motion-receipt.json").write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

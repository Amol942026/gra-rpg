"""Author local Synfig motion tracks for the forest bandit's upper body."""
import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image

from hero_synfig import group, image
import synfig_cli


FRAMES = 48
UNITS = 28
CLIPS = ("idle", "walk", "windup", "strike")
CHANNELS = ("body", "arm", "cloth")


def offsets(clip, t):
    wave = math.sin(t*math.tau)
    if clip == "idle":
        return (.18*wave, .35*wave), (.25*wave, 0), (.24*wave, 0)
    if clip == "walk":
        return (1.2+.28*wave, .48*math.sin(t*math.tau*2)), (-5.4*wave, .5*wave), (1.5*math.sin((t-.12)*math.tau), 0)
    if clip == "windup":
        load = math.sin(t*math.pi)**2
        return (-2.3*load, .55*load), (-8*load, 4.6*load), (.9*load, -1.1*load)
    swing = t*t*(3-2*t)
    return (4.5*swing, -.55*swing), (20*swing, 7*swing), (-2.6*swing, -1.5*swing)


def translate(canvas, name, values):
    layer = ET.SubElement(canvas, "layer", type="translate", active="true", version="0.1", desc="runtime:"+name)
    track = ET.SubElement(ET.SubElement(layer, "param", name="origin"), "animated", type="vector")
    for i, (x, y) in enumerate(values):
        point = ET.SubElement(ET.SubElement(track, "waypoint", time=f"{i}f",
                                           before="linear", after="linear"), "vector")
        ET.SubElement(point, "x").text = f"{x/UNITS:.8f}"
        ET.SubElement(point, "y").text = f"{y/UNITS:.8f}"


def author(root, clip):
    canvas = ET.Element("canvas", version="1.2", width="240", height="288", fps="60",
                        **{"view-box": "-1.25 1.5 1.25 -1.5", "antialias": "2",
                           "begin-time": "0f", "end-time": f"{FRAMES-1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / forest bandit / "+clip
    values = [offsets(clip, i/FRAMES) for i in range(FRAMES+1)]
    for name, rect in [("cloth", (-.53, 1.3, .40, .52)), ("body", (-.61, 1.24, .50, -.24)),
                       ("arm", (.29, .65, .66, -.12))]:
        part = group(canvas, name)
        image(part, name, rect)
        translate(part, name, [sample[CHANNELS.index(name)] for sample in values])
    path = root / (clip+".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def read_tracks(path):
    tracks = {}
    for layer in ET.parse(path).getroot().iter("layer"):
        name = layer.get("desc", "").removeprefix("runtime:")
        if name in CHANNELS and layer.get("type") == "translate":
            tracks[name] = [(float(p.findtext("vector/x"))*UNITS, float(p.findtext("vector/y"))*UNITS)
                            for p in layer.findall("param[@name='origin']/animated/waypoint")]
    if set(tracks) != set(CHANNELS) or any(len(v) != FRAMES+1 for v in tracks.values()):
        raise ValueError(f"Incomplete native tracks: {path}")
    return [tuple(v for name in CHANNELS for v in tracks[name][i]) for i in range(FRAMES+1)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--concept", type=Path, required=True)
    parser.add_argument("--synfig", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    parts = args.root / "parts"
    parts.mkdir()
    with Image.open(args.concept) as source:
        for name, box in [("cloth", (220, 26, 296, 144)), ("body", (189, 78, 313, 270)),
                          ("arm", (310, 148, 341, 270))]:
            source.crop(box).save(parts / (name+".png"))
    synfig_cli.SYNFIG = args.synfig
    lines = ["#pragma once", "// Native Synfig waypoints from tools/media/bandit_motion.py.",
             "namespace ember::bandit_motion {", "struct Key {float lean,bob,arm,raise,cloth,hip;};"]
    renders = []
    for clip in CLIPS:
        project = author(args.root, clip)
        values = read_tracks(project)
        lines.append(f"inline constexpr Key {clip}[]={{")
        for row in values:
            lines.append("{"+",".join(f"{v:.6f}f" for v in row)+"},")
        lines.append("};")
        renders.append(synfig_cli.render(project, output_root=args.root/"renders", threads=2,
                                         expected_frames=FRAMES, require_motion=True))
    lines.append("}")
    (args.root/"bandit_motion_data.hpp").write_text("\n".join(lines)+"\n", encoding="utf-8")
    (args.root/"motion-receipt.json").write_text(json.dumps({"status": "PASS", "frames": FRAMES*len(CLIPS),
        "renders": renders, "audio_played": False}, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

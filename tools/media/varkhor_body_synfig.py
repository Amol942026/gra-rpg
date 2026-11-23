"""Author the plated boss's weight shifts and gestures in Synfig."""
from pathlib import Path
from xml.etree import ElementTree as ET
import argparse
import json
import math

from PIL import Image
from caldera_puppet_motion import transform
from fire_imp_synfig_rig import canvas, save
from hero_synfig import group, image
from varkhor_ash_synfig import sample, tangents
import synfig_cli

FRAMES = 60
CHANNELS = ("advance", "drop", "turn", "left", "right", "handReach", "handLift",
            "shoulder", "shoulderLift", "lateralRaise")
# The fist reaches the target at frame 27, the melee contact key.
TRACKS = {
    "walk": ((0, 0, 1.2, 0, 0, 0), (8, 1.0, 3.2, 2.8, -5, 6),
             (18, 1.4, .3, 3.4, -8, 9), (30, 0, 1.2, 0, 0, 0),
             (38, -1.0, 3.2, -2.8, 6, -5), (48, -1.4, .3, -3.4, 9, -8),
             (60, 0, 1.2, 0, 0, 0)),
    "strike": ((0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
               (8, -1.4, 1.2, -2.6, -5, 0, .02, 6.3, -.8, .8, .54),
               (16, -2.9, 2.6, -5.2, -10, 0, .08, 13.1, -2.05, 2.65, 1),
               (22, -3.2, 3.25, -6.7, -12, 0, .14, 14.7, -2.5, 3.0, 1),
               (24, -1.9, 3.3, -4.35, -8, 0, .31, 12.0, -1.45, 2.4, 1),
               (26, 2.7, 4.8, 4.2, 9, 0, .77, 5.6, 1.75, 1.55, 1),
               (27, 4.3, 5.3, 6.8, 13, 0, 1, 0, 2.9, 1.10, 1),
               (30, 4.0, 6.2, 7.4, 15, 0, .94, .65, 2.85, .65, 1),
               (37, 2.4, 4.2, 4.8, 9, 0, .61, 5.2, 1.35, .12, .94),
               (47, 1.0, 1.4, 1.5, 4, 0, .24, 7.5, .35, 0, .59),
               (54, .15, .3, .3, 1, 0, .04, 2.8, 0, 0, .18),
               (60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)),
    "eruption": ((0, 0, 0, 0, 0, 0), (11, -1, 1.6, -1.4, 13, -13),
                 (26, -1.6, -4.3, -2.1, 31, -31), (36, -.8, -6, 0, 44, -43),
                 (40, 1.5, -3, 1, 41, -40), (42, 5, 9, 3.5, -14, 15),
                 (47, 4, 7.7, 2.3, -11, 11), (54, 1.2, 3.5, .8, 4, -4),
                 (60, 0, 0, 0, 0, 0)),
}

def strike_angles(reach, lift, lateral_raise):
    # SE painting. Match the two rigid spans measured on the exported atlas.
    if lateral_raise <= 0:
        return 0., 0.
    upper = (2.5, 16.)
    forearm = (5.1, 20.7)
    upper_len, lower_len = math.hypot(*upper), math.hypot(*forearm)
    aim = math.atan2(.51 + .72 / math.sqrt(5), 2 / math.sqrt(5)) - math.pi / 2
    extension = upper_len + lower_len - .18
    hx = 7.6 + (-math.sin(aim) * extension - 7.6) * reach
    hy = 36.7 + (math.cos(aim) * extension - 36.7) * reach - lift
    norm = math.hypot(hx, hy)
    dx, dy = hx / norm, hy / norm
    folded = math.sqrt(upper_len**2 + lower_len**2 + 2 * upper_len * lower_len * math.cos(math.radians(104)))
    r = min(upper_len + lower_len, max(folded, norm))
    along = (upper_len**2 - lower_len**2 + r**2) / (2 * r)
    bend = math.sqrt(max(0., upper_len**2 - along**2))
    ex, ey = along * dx + bend * dy, along * dy - bend * dx
    ux = math.degrees(math.atan2(ey, ex) - math.atan2(upper[1], upper[0]))
    fx = math.degrees(math.atan2(r * dy - ey, r * dx - ex) - math.atan2(forearm[1], forearm[0]))
    return math.remainder(ux, 360.) * lateral_raise, math.remainder(fx - ux, 360.) * lateral_raise


def author(folder, atlas):
    folder.mkdir(parents=True, exist_ok=True)
    (folder / "parts").mkdir(exist_ok=True)
    with Image.open(atlas) as art:
        cell = art.width // 8
        for part in range(6):
            box = (cell, part * cell, cell * 2, (part + 1) * cell)
            plate = art.crop(box).convert("RGBA")
            plate.save(folder / "parts" / f"{part}.png")
            if part == 5:
                elbow = round(cell * 82.5 / 128)
                for label, top, bottom in (("ramie", 0, elbow+1), ("przedramie", elbow, cell)):
                    segment = Image.new("RGBA", plate.size)
                    segment.paste(plate.crop((0, top, cell, bottom)), (0, top))
                    segment.save(folder / "parts" / f"{label}.png")
    names = ("Pas i plecy", "Lewa noga", "Prawa noga", "Pancerz i głowa", "Lewa ręka", "Prawa ręka")
    for name, keys in TRACKS.items():
        project = canvas("Varkhor / " + name, 256, 256, (128, 230), FRAMES)
        defs = ET.SubElement(project, "defs")
        values = []
        for i, label in enumerate(CHANNELS, 1):
            points = tuple((key[0], key[i] if i < len(key) else 0) for key in keys)
            slopes = tangents(points)
            curve = [sample(points, slopes, frame) for frame in range(FRAMES + 1)]
            track = ET.SubElement(defs, "animated", id=label, type="real")
            for frame, value in enumerate(curve):
                w = ET.SubElement(track, "waypoint", time=f"{frame}f", before="linear", after="linear")
                ET.SubElement(w, "real", value=f"{value:.6f}")
            values.append(curve)
        for part in range(6):
            layer = group(project, names[part])
            rect = (-128, 230, 128, -26)
            punch_arm = name == "strike" and part == 5
            if punch_arm:
                image(layer, "ramie", rect)
                forearm = group(layer, "Przedramię i pięść")
                image(forearm, "przedramie", rect)
                angles = [strike_angles(reach, lift, raise_side)
                          for reach, lift, raise_side in zip(values[5], values[6], values[9])]
                transform(forearm, "rotate", "Łokieć prowadzony na zewnątrz", "amount",
                          [-bend for _, bend in angles],
                          "angle", origin=(57, 65))
            else:
                image(layer, str(part), rect)
            if part in (1, 2):
                continue
            angle = values[3 + (part == 5)] if part >= 4 else values[2]
            shifts = [(x * 1.36, -y * 2) for x, y in zip(values[0], values[1])]
            if punch_arm:
                angle = [-arm - .96 * turn for (arm, _), turn in zip(angles, values[2])]
                shifts = [(x + reach * 1.79, y - reach * .39 + lift * 2)
                          for (x, y), reach, lift in zip(shifts, values[7], values[8])]
            pivot = (-54 if part == 4 else 52 if part == 5 else 0, 96 if part >= 4 else 58)
            transform(layer, "rotate", "Przeniesienie ciężaru", "amount", angle, "angle", origin=pivot)
            transform(layer, "translate", "Bark" if punch_arm else "Tułów", "origin", shifts, "vector")
        save(project,folder / f"{name}.sif")


def export(folder, target):
    lines = ["#pragma once", "// Exported from the Synfig sources by varkhor_body_synfig.py.",
             "namespace varkhor_motion {", "struct Key {float " + ", ".join(CHANNELS) + ";};"]
    for clip in TRACKS:
        tracks = ET.parse(folder / f"{clip}.sif").findall("./defs/animated")
        frames = [[float(w.find("real").get("value")) for w in track] for track in tracks]
        assert len(frames) == len(CHANNELS) and all(len(channel) == FRAMES + 1 for channel in frames)
        lines.append(f"inline constexpr Key {clip}[]={{")
        lines.extend(" {" + ",".join(f"{value:.6f}f" for value in frame) + "}," for frame in zip(*frames))
        lines.append("};")
    target.write_text("\n".join(lines + ["}", ""]), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("header", type=Path)
    parser.add_argument("--atlas", type=Path)
    parser.add_argument("--render", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    if args.atlas:
        author(args.source, args.atlas)
    export(args.source, args.header)
    if args.render:
        for clip in TRACKS:
            folder = args.render / clip
            receipt = synfig_cli.render((args.source / f"{clip}.sif").resolve(), output_root=folder,
                                      threads=2, expected_frames=61, require_motion=True)
            (folder / "synfig.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")

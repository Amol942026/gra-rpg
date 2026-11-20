"""Natywne klucze animacji 2D Ognistego Chochlika dla Synfiga i Emberfall."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

from caldera_puppet_motion import transform
from hero_synfig import group, image, value


FRAMES = 60
TRACKS = {
    "idle": {
        "breath": ((0, 0), (10, .38), (20, 1.35), (28, 1.72), (34, 1.54), (46, .52), (60, 0)),
        "settle": ((0, 0), (15, -.65), (32, .8), (51, .28), (60, 0)),
        "tail": ((0, -.8), (14, .6), (32, 2.5), (47, 1.1), (60, -.8)),
    },
    "walk": {
        # Constant screen-plane travel while the hoof carries the body's weight.
        "reach": ((0, 8.4), (36, -8.4), (40, -8.0), (44, -4.9), (49, 1.3), (54, 6.7), (58, 8.7), (60, 8.4)),
        "lift": ((0, 0), (36, 0), (40, 1.5), (45, 5.8), (49, 8.2), (53, 7.0), (57, 3.1), (60, 0)),
        "hip": ((0, -.85), (5, -1.2), (12, .25), (19, 1.18), (26, .52), (30, -.85),
                (35, -1.2), (42, .25), (49, 1.18), (56, .52), (60, -.85)),
        "weight": ((0, -1.4), (8, -1.6), (17, -.57), (28, 1.12), (36, 1.64), (45, .66), (55, -1.07), (60, -1.4)),
        "arm": ((0, -5.2), (9, -4.6), (22, 2.8), (34, 6.05), (41, 4.8), (53, -2.9), (60, -5.2)),
        "tail": ((0, -2.25), (10, -4.3), (22, -.7), (36, 4.2), (49, 1.8), (60, -2.25)),
    },
    "turn": {
        "toe": ((0, 0), (12, -.9), (23, -2.05), (34, -2.3), (43, -.7), (50, 1.6), (56, 1.05), (60, 0)),
        "lift": ((0, 0), (31, 0), (38, 1.38), (45, 3.95), (50, 3.7), (56, 1.38), (60, 0)),
        "shift": ((0, -.65), (8, -.94), (18, -.67), (32, .7), (42, .92), (52, -.22), (60, -.65)),
    },
    "cast": {
        "crouch": ((0, 0), (7, .23), (15, .67), (21, .97), (25, .73), (28, .2), (36, .08), (51, 0), (60, 0)),
        "gather": ((0, 0), (5, .08), (10, .45), (16, .87), (21, 1), (24, .74), (27, .03), (30, 0), (60, 0)),
        "thrust": ((0, 0), (16, 0), (21, -.07), (23, .05), (25, .45), (27, 1), (30, .97),
                   (35, .78), (44, .34), (54, .04), (60, 0)),
        "release": ((0, 0), (27, 0), (33, .59), (37, 1), (45, .55), (54, .08), (60, 0)),
        "tail": ((0, 0), (13, -1.8), (20, -3.5), (26, -.7), (32, 3.2), (40, 4.1), (52, .9), (60, 0)),
    },
    "fall": {
        "roll": ((0, 0), (6, -.017), (12, .09), (18, .24), (27, .55), (35, .82), (45, .975), (54, 1), (60, 1)),
        "buckle": ((0, 0), (7, .08), (15, .61), (22, .98), (28, 1), (60, 1)),
        "land": ((0, 0), (25, 0), (34, .11), (41, .49), (49, .94), (55, 1), (60, 1)),
        "recoil": ((0, 0), (5, .62), (9, .91), (13, .7), (22, .19), (29, 0), (60, 0)),
        "relax": ((0, 0), (14, 0), (21, .15), (29, .63), (38, 1), (60, 1)),
        "curl": ((0, 0), (12, 0), (21, .19), (30, .57), (40, .96), (46, 1), (60, 1)),
    },
}


def canvas(name, width, height, anchor, last, fps=60):
    x, y = anchor
    project = ET.Element("canvas", version="1.2", width=str(width), height=str(height), fps=str(fps),
                         **{"view-box": f"{-x} {y} {width-x} {y-height}", "begin-time": "0f",
                            "end-time": f"{last}f", "antialias": "1", "bgcolor": "0 0 0 0"})
    ET.SubElement(project, "name").text = "Emberfall / Ognisty Chochlik / " + name
    return project


def save(project, path):
    ET.indent(project)
    ET.ElementTree(project).write(path, encoding="utf-8", xml_declaration=True)


def control_master(folder):
    project = canvas("Klucze ruchu", 135, 138, (67, 119), FRAMES)
    controls = ET.SubElement(project, "defs")
    for clip, channels in TRACKS.items():
        for name, points in channels.items():
            curve = ET.SubElement(controls, "animated", id=f"{clip}_{name}", type="real")
            for frame, number in points:
                knot = ET.SubElement(curve, "waypoint", time=f"{frame}f", before="linear", after="linear")
                ET.SubElement(knot, "real", value=str(number))
    path = folder / "klucze-chochlika.sif"
    save(project, path)
    return path


def export(master, header):
    lines = ["#pragma once", "// Exported from native Synfig waypoints by fire_imp_synfig_rig.py.",
             "namespace ember::fire_imp_synfig {", "struct Waypoint {float time,value;};"]
    curves = ET.parse(master).findall("./defs/animated")
    for curve in curves:
        keys = [(int(w.get("time")[:-1]), float(w.find("real").get("value"))) for w in curve]
        if sorted({frame for frame, _ in keys}) != [frame for frame, _ in keys] or keys[0][0] != 0 or keys[-1][0] != FRAMES:
            raise ValueError(curve.get("id"))
        data = ",".join(f"{{{frame/FRAMES:.9f}f,{number:.6f}f}}" for frame, number in keys)
        lines.append(f"inline constexpr Waypoint {curve.get('id')}[]={{ {data} }};")
    header.write_text("\n".join(lines + ["}", ""]), encoding="utf-8")


def draw_masters(folder, poses, body_path, core_path):
    from PIL import Image

    parts = folder / "parts"
    parts.mkdir(exist_ok=True)
    sheets = [Image.open(body_path), Image.open(core_path)]
    geometry = [(160, 160, 3), (84, 116, 24)]
    for name, frames in json.loads(poses.read_text(encoding="utf-8")).items():
        project = canvas(name, 232, 138, (116, 119), len(frames) - 1)
        # The production pose provides the same flat 2D transforms the game
        # reads. Synfig draws the painted layers with native animated transforms.
        # Layer ranks hide or reveal limbs as the single rig turns.
        for index in range(len(frames[0])):
            for tile, mirrored in dict.fromkeys((f[index][0], f[index][1]) for f in frames):
                active = [(p[0], p[1]) == (tile, mirrored) for p in (f[index] for f in frames)]
                indices = [f for f, visible in enumerate(active) if visible]
                track = [frames[f][index] for f in indices]
                art = f"{tile:03}" + ("-L" if mirrored else "")
                bitmap = parts / f"{art}.png"
                if not bitmap.exists():
                    atlas = int(tile >= 12)
                    local = tile - (12 if atlas else 0)
                    w, h, cols = geometry[atlas]
                    x, y = local % cols * w, local // cols * h
                    sprite = sheets[atlas].crop((x+1, y+1, x+w-1, y+h-1))
                    if mirrored:
                        sprite = sprite.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
                    sprite.save(bitmap)
                node = group(project, f"Warstwa {index + 1} / {art}")
                layer = project[-1]
                amount = layer.find("./param[@name='amount']")
                amount.remove(amount[0])
                animation = ET.SubElement(amount, "animated", type="real")
                for f, visible in enumerate(active):
                    if not f or visible != active[f-1]:
                        knot = ET.SubElement(animation, "waypoint", time=f"{f}f", before="constant", after="constant")
                        ET.SubElement(knot, "real", value="1" if visible else "0")
                # Synfig coordinates have an upward Y axis. Each group starts
                # with its position at (0,0), then stretches, rotates and moves.
                image(node, art, (-1, 1, 1, -1))
                node[-1].remove(node[-1].find("./param[@name='interpolation']"))
                transform(node, "stretch", "Skrót perspektywy", "amount", [(r[4], r[5]) for r in track], "vector", frames=indices)
                value(node[-1], "center", "vector", (0, 0))
                transform(node, "rotate", "Obrót przy mocowaniu", "amount", [-r[6] for r in track], "angle", (0, 0), frames=indices)
                transform(node, "translate", "Wspólne mocowanie", "origin", [(r[2], -r[3]) for r in track], "vector", frames=indices)
        save(project, folder / f"{name}.sif")
    for sheet in sheets:
        sheet.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("folder", type=Path)
    parser.add_argument("--header", type=Path)
    parser.add_argument("--from-master", type=Path)
    parser.add_argument("--poses", type=Path)
    parser.add_argument("--body", type=Path, default=Path("assets/volcano/fire-imp-puppet.png"))
    parser.add_argument("--core", type=Path, default=Path("assets/volcano/fire-imp-core.png"))
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.folder.mkdir(parents=True, exist_ok=True)
    if args.header:
        export(args.from_master or control_master(args.folder), args.header)
    if args.poses:
        draw_masters(args.folder, args.poses, args.body, args.core)

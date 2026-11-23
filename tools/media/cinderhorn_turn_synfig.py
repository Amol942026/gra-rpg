"""Export motion tracks for the directional Rogacz rig from Synfig."""

import argparse
import json
import math
from pathlib import Path
from xml.etree import ElementTree as ET

from PIL import Image

from cinderhorn_paint import CELL, KEYS, ORDER, PARTS
from fire_imp_synfig_rig import canvas, save
from hero_synfig import group, image
from caldera_puppet_motion import transform
import synfig_cli

FRAMES = 60
FOOT_CYCLES = {
    "rush": (73, .36, 11.75),
    "hindRush": (73, .44, 10.0),
}
WALK_STRIDE = 42
FORE_SUPPORT, HIND_SUPPORT, HIND_LEAD = 41/FRAMES, 40/FRAMES, .23
BODY_RUSH = (
    (0, -.016, -.52), (6, -.028, -.93), (13, .012, .52), (17, .026, 1.04),
    (24, .004, .36), (30, -.017, -.58), (35, -.026, -.91), (42, .013, .58),
    (47, .026, 1.02), (54, -.003, .04), (60, -.016, -.52),
)
BODY = (
    (0, -.025, -.91), (5, -.039, -1.52), (10, -.022, -.74), (15, .016, .69),
    (21, .033, 1.36), (26, .008, .10), (30, -.023, -.84), (35, -.037, -1.39),
    (40, -.019, -.62), (46, .019, .83), (51, .034, 1.23), (56, .007, .15), (60, -.025, -.91),
)
WALK_LEAN = (
    (0, .61, .024), (7, 1.48, .045), (14, 1.23, .033), (20, .52, .014),
    (27, -.44, -.025), (33, -1.24, -.044), (39, -1.42, -.039),
    (46, -.72, -.021), (54, .20, .011), (60, .61, .024),
)
TAIL_WALK = (
    (0, .38, .18), (10, 1.43, .65), (19, 1.02, .05), (28, -.54, -.74),
    (38, -1.34, .42), (48, -.65, -.23), (54, -.14, -.16), (60, .38, .18),
)
NECK_WALK = (
    (0, .51, -.28), (5, 1.15, -.83), (11, .44, -.54), (17, -.48, .28),
    (23, -.29, .99), (29, .40, .11), (34, 1.04, -.78), (41, .34, -.45),
    (47, -.55, .40), (53, -.24, .83), (60, .51, -.28),
)
NECK_RUSH = (
    (0, .22, -.20), (10, .08, .42), (20, .39, -.26), (28, .66, -.68),
    (36, .25, -.28), (44, .10, .42), (53, .32, -.12), (60, .22, -.20),
)
LOWER = (
    (0, 0, 0), (6, .35, -.18), (12, 1.15, -1.18), (20, 3.55, -4.24),
    (28, 6.3, -6.98), (30, 7.25, -7.25), (38, 6.0, -6.45), (46, 3.15, -4.25),
    (54, .85, -1.43), (60, 0, 0),
)
FALL_BALANCE = (
    (0, 0, 0), (5, 1, .045), (12, 7, .21), (19, 20, .46), (27, 39, .72),
    (35, 58, .9), (43, 72, .98), (50, 78, 1), (55, 79.2, 1), (60, 79.2, 1),
)
FALL_FRONT = (
    (0, 0, 0), (7, -.7, 1.5), (15, -2.6, 5.3), (25, -6.1, 9.8),
    (37, -8.6, 11.7), (49, -9.7, 10.2), (55, -9.4, 9.9), (60, -9.4, 9.9),
)
FALL_HIND = (
    (0, 0, 0), (8, .2, .3), (16, 1.4, 3.0), (26, 4.2, 6.8),
    (38, 7.2, 10.1), (48, 8.7, 9.1), (55, 8.7, 8.5), (60, 8.7, 8.5),
)
FALL_HEAD = (
    (0, 0, 0), (6, -.8, 1.1), (14, .2, -.95), (23, 1.1, -4.0),
    (35, 2.1, -4.85), (46, 1.7, -3.65), (55, 1.45, -3.2), (60, 1.45, -3.2),
)


def hoof(phase, stride, support, clearance):
    phase %= 1
    half = stride * support / 2
    if phase <= support:
        return half - stride * phase, 0
    t = (phase - support) / (1 - support)
    ease = t*t*(3 - 2*t)
    tangent = -stride * (1 - support) * t*(1 - t)*(1 - 2*t)
    return half*(2*ease - 1) + tangent, clearance * math.sin(math.pi*t)**1.55


def walk_hoof(phase, support, clearance):
    phase %= 1
    along, _ = hoof(phase, WALK_STRIDE, support, clearance)
    if phase <= support:
        return along, 0, 0
    t = (phase - support) / (1 - support)
    # The hoof folds under the knee early, then reaches and lowers toward its next hold.
    arc = t + .56*t*(1-t)
    lift = clearance * math.sin(math.pi*arc)**1.7
    pitch = math.sin(math.pi*t) * (30*(1-t) - 19*t)
    return along, lift, pitch


def smooth_cycle(keys):
    def slope(i, component):
        a = keys[i-1] if i else (-FRAMES+keys[-2][0], *keys[-2][1:])
        b = keys[i+1] if i+1 < len(keys) else (FRAMES+keys[1][0], *keys[1][1:])
        return (b[component]-a[component]) / (b[0]-a[0])

    samples = []
    for i, (a, b) in enumerate(zip(keys, keys[1:])):
        duration = b[0]-a[0]
        for frame in range(a[0], b[0]):
            t = (frame-a[0])/duration
            values = []
            for component in (1, 2):
                p, q = a[component], b[component]
                dp, dq = duration*slope(i, component), duration*slope(i+1, component)
                values.append(p+t*(dp+t*(3*(q-p)-2*dp-dq+t*(2*(p-q)+dp+dq))))
            samples.append((frame, *values))
    return [*samples, keys[-1]]


def sample(keys, frame):
    for (start, x0, y0), (end, x1, y1) in zip(keys, keys[1:]):
        if frame <= end:
            t = (frame - start) / (end - start)
            return x0+(x1-x0)*t, y0+(y1-y0)*t
    return keys[-1][1:]


def author(root, paintings, falling=False):
    project = canvas("Rogacz Żaru / klucze obrotu i chodu", 160, 160, (80, 144), FRAMES)
    project.find("name").text = ("Rogacz Żaru / przewrócenie i podparcie boku" if falling else
                                 "Rogacz Żaru / podparcie, przeniesienie ciężaru, kark i ogon")
    curves = ET.SubElement(project, "defs")
    tracks = {name: [(f,*hoof(f/FRAMES,*cycle)) for f in range(FRAMES+1)]
              for name,cycle in FOOT_CYCLES.items()}
    tracks.update(body=smooth_cycle(BODY), bodyRush=BODY_RUSH, walkLean=smooth_cycle(WALK_LEAN),
                  neckWalk=smooth_cycle(NECK_WALK), neckRush=NECK_RUSH,
                  tailWalk=smooth_cycle(TAIL_WALK), lower=LOWER)
    for name, support, clearance in (("walk", FORE_SUPPORT, 8.6), ("hindWalk", HIND_SUPPORT, 7.6)):
        feet = [walk_hoof(f/FRAMES, support, clearance) for f in range(FRAMES+1)]
        tracks[name] = [(f, x, lift) for f, (x, lift, _) in enumerate(feet)]
        tracks[f"{name}Toe"] = [(f, pitch, 0) for f, (_, _, pitch) in enumerate(feet)]
    if falling:
        tracks = dict(fallBalance=FALL_BALANCE, fallFront=FALL_FRONT, fallHind=FALL_HIND, fallHead=FALL_HEAD)
    for name, keys in tracks.items():
        node = ET.SubElement(curves, "animated", id=name, type="vector")
        for f, x, y in keys:
            key = ET.SubElement(node, "waypoint", time=f"{f}f", before="linear", after="linear")
            v = ET.SubElement(key, "vector")
            ET.SubElement(v, "x").text = f"{x:.6f}"
            ET.SubElement(v, "y").text = f"{y:.6f}"
    atlas = Image.open(paintings)
    (root / "parts").mkdir(exist_ok=True)
    layers = group(project, "Przewrócenie") if falling else project
    for part in ORDER[1]:
        x, y = KEYS.index(1) * CELL, part * CELL
        atlas.crop((x, y, x + CELL, y + CELL)).save(root / "parts" / f"{part}.png")
        layer = group(layers, PARTS[part])
        image(layer, str(part), (-80, 144, 80, -16))
        layer[-1].find("./param[@name='filename']/string").text = f"parts/{part}.png"
        if falling:
            keys = FALL_HEAD if part == 2 else FALL_HIND if part < 2 else FALL_FRONT
            if part != 3:
                transform(layer, "translate", "Zgięcie w trakcie upadku", "origin",
                          [sample(keys, f) for f in range(FRAMES+1)], "vector")
        elif part != 2 and part != 3:
            phase = {0: .5+HIND_LEAD, 1: HIND_LEAD, 4: .5, 5: 0}[part]
            support, clearance = (HIND_SUPPORT, 7.6) if part < 2 else (FORE_SUPPORT, 8.6)
            feet = [walk_hoof(f / FRAMES + phase, support, clearance) for f in range(FRAMES + 1)]
            transform(layer, "translate", "Tor kopyta", "origin",
                      [(x * .86, y - x * .43) for x, y, _ in feet], "vector")
        else:
            positions = []
            for f in range(FRAMES+1):
                _, bob = sample(tracks["body"], f)
                sway, _ = sample(tracks["walkLean"], f)
                x, y = sample(tracks["neckWalk"], f) if part == 2 else (0,0)
                positions.append((x,bob+y+.24*sway))
            transform(layer, "translate", "Przeniesienie ciężaru i kark", "origin", positions, "vector")
    if falling:
        balance = [sample(FALL_BALANCE, f) for f in range(FRAMES+1)]
        transform(layers, "rotate", "Przechył", "amount", [angle for angle, _ in balance],
                  "angle", origin=(0, 35))
        transform(layers, "translate", "Oparcie boku", "origin", [(0, -30 * ground) for _, ground in balance], "vector")
    source = root / ("rogacz-upadek.sif" if falling else "rogacz-chod.sif")
    save(project, source)
    return source


def export(source, header, falling=False):
    lines = ["#pragma once", "// Generated from Synfig by cinderhorn_turn_synfig.py."]
    if falling:
        lines += ['#include "cinderhorn_synfig.hpp"', "namespace ember::cinderhorn_synfig {"]
    else:
        lines += ["namespace ember::cinderhorn_synfig {", "struct Key {float time, along, lift;};"]
    for node in ET.parse(source).findall("./defs/animated"):
        keys = [f"{{{int(w.get('time')[:-1]) / FRAMES:.8f}f,{float(w.findtext('vector/x')):.6f}f,"
                f"{float(w.findtext('vector/y')):.6f}f}}" for w in node]
        if not keys:
            raise ValueError("Missing Synfig waypoints")
        lines += [f"inline constexpr Key {node.get('id')}[]={{", " " + ",\n ".join(keys) + "};"]
    header.write_text("\n".join(lines + ["}", ""]), encoding="utf-8")


if __name__ == "__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("source", type=Path)
    cli.add_argument("header", type=Path)
    cli.add_argument("--paintings", type=Path)
    cli.add_argument("--render", type=Path)
    cli.add_argument("--fall", action="store_true")
    cli.add_argument("--no-audio", action="store_true", required=True)
    a = cli.parse_args()
    if a.paintings:
        a.source.parent.mkdir(parents=True, exist_ok=True)
        if a.source != author(a.source.parent, a.paintings, a.fall):
            cli.error("the source must use the filename for the selected animation")
    export(a.source, a.header, a.fall)
    if a.render:
        receipt = synfig_cli.render(a.source.resolve(), output_root=a.render, threads=2, expected_frames=61, require_motion=True)
        (a.render / "synfig.json").write_text(json.dumps(receipt, indent=2), encoding="utf-8")

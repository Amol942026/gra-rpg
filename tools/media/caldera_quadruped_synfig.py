"""Author and export native Synfig curves for the two continuous Caldera rigs."""

import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

from fire_imp_synfig_rig import canvas, save
from hero_synfig import value


TRACKS = {
    "drake": {
        # In support the paw moves backwards by the exact ground distance
        # the actor travels. During transfer each pair has its own arc.
        "foreReach": ((0, 12.0), (40, -12.0), (43, -12.45), (46, -9.2), (50, -.8),
                      (54, 8.0), (57, 12.2), (59, 12.55), (60, 12.0)),
        "foreLift": ((0, 0), (40, 0), (43, 1.45), (46, 5.0), (50, 9.6),
                     (53, 10.2), (56, 6.7), (59, .7), (60, 0)),
        "foreCurl": ((0, 0), (40, 0), (44, 1.25), (48, 3.2), (52, 2.35),
                     (55, .54), (57, -.50), (60, 0)),
        "hindReach": ((0, 11.4), (38, -11.4), (41, -12.0), (44, -9.9), (49, -.7),
                      (54, 8.9), (57, 11.75), (59, 11.93), (60, 11.4)),
        "hindLift": ((0, 0), (38, 0), (41, 1.1), (44, 3.4), (48, 6.6),
                     (52, 7.65), (55, 5.5), (59, .62), (60, 0)),
        "hindCurl": ((0, 0), (38, 0), (42, 1.85), (46, 3.8), (50, 2.6),
                     (55, .65), (58, -.55), (60, 0)),
        "weight": ((0, -.9), (5, -1.28), (11, -.18), (16, 1.32), (21, .58), (27, -.4),
                   (32, -.96), (37, -1.10), (43, .34), (47, 1.22), (51, .48), (56, -.45), (60, -.9)),
        "pitch": ((0, -.033), (6, -.017), (12, .035), (18, .030), (24, -.013),
                  (30, -.035), (37, -.014), (42, .035), (49, .029), (55, -.018), (60, -.033)),
        "roll": ((0, -.033), (8, .038), (15, .081), (23, .040), (30, .008),
                 (38, -.051), (45, -.090), (53, -.058), (60, -.033)),
        "lead": ((0, -.86), (7, .12), (15, 1.22), (23, .45), (30, -.65),
                 (36, -.12), (45, 1.04), (53, .16), (60, -.86)),
        "wingYield": ((0, -1.86), (9, -2.91), (18, -.90), (27, 1.83),
                      (36, 2.35), (45, .87), (53, -.59), (60, -1.86)),
        "tailSwing": ((0, -.37), (8, .42), (16, .90), (23, .64), (30, .09),
                      (38, -.65), (46, -.92), (53, -.76), (60, -.37)),
        "idle": ((0, 0), (14, .28), (29, .95), (39, .71), (51, .13), (60, 0)),
        "bite": ((0, 0), (10, -.42), (19, -.75), (23, -.49), (28, .95), (31, 1), (37, .62), (51, .09), (60, 0)),
        "biteJaw": ((0, 0), (11, .16), (20, .75), (26, 1), (31, .09), (38, .04), (60, 0)),
        "breath": ((0, 0), (13, -.41), (24, -.72), (27, -.42), (32, .88), (40, .91), (50, .26), (60, 0)),
        "breathJaw": ((0, 0), (17, .14), (25, .53), (31, 1), (45, .91), (53, .3), (60, 0)),
        "fall": ((0, 0), (8, .016), (17, .12), (25, .43), (37, .85), (48, .99), (56, 1), (60, 1)),
        "fold": ((0, 0), (9, .01), (18, .28), (28, .7), (40, .91), (56, 1), (60, 1)),
    },
    "crawler": {
        "reach": ((0, 5.8), (39, -5.8), (43, -5.2), (47, -2.6), (51, 1.7), (56, 5.3), (60, 5.8)),
        "lift": ((0, 0), (39, 0), (42, 1.5), (47, 3.8), (50, 4.2), (55, 2.7), (58, .8), (60, 0)),
        "weight": ((0, -.24), (8, .30), (15, .54), (23, .16), (30, -.24), (38, .30), (45, .54), (53, .16), (60, -.24)),
        "idle": ((0, 0), (14, .18), (31, .53), (42, .38), (55, .05), (60, 0)),
        "bite": ((0, 0), (12, -.70), (19, -.91), (24, -.55), (29, 1), (33, .86), (40, .37), (53, .08), (60, 0)),
        "biteJaw": ((0, 0), (12, .14), (22, .49), (27, .69), (31, .06), (44, 0), (60, 0)),
        "fall": ((0, 0), (8, .026), (16, .16), (23, .48), (34, .91), (44, .985), (54, 1), (60, 1)),
        "fold": ((0, 0), (8, .11), (16, .38), (26, .80), (39, 1), (60, 1)),
    },
}


def author(folder, species):
    project = canvas("", 256, 160, (16, 145), 60)
    creature = "Żarowy Drak" if species == "drake" else "Żużlowy Pełzacz"
    project.find("name").text = f"Emberfall / Kaldera / {creature} / tory ruchu"
    controls = ET.SubElement(project, "defs")
    for index, (channel, points) in enumerate(TRACKS[species].items()):
        curve = ET.SubElement(controls, "animated", id=channel, type="real")
        for frame, amount in points:
            key = ET.SubElement(curve, "waypoint", time=f"{frame}f", before="linear", after="linear")
            ET.SubElement(key, "real", value=str(amount))
        # Editable visual probes allow inspection of every curve in Synfig.
        layer = ET.SubElement(project, "layer", type="circle", active="true", version="0.2", desc=channel)
        value(layer, "origin", "vector", (16 + 25 * (index % 9), 20 + 38 * (index // 9)))
        radius = ET.SubElement(ET.SubElement(layer, "param", name="radius"), "animated", type="real")
        for frame, amount in points:
            key = ET.SubElement(radius, "waypoint", time=f"{frame}f", before="linear", after="linear")
            ET.SubElement(key, "real", value=f"{2 + .6 * abs(amount):.5f}")
    path = folder / f"{species}-ruch.sif"
    save(project, path)
    return path


def export(inputs, header):
    code = ["#pragma once", "// Exported from native Synfig curves by caldera_quadruped_synfig.py.",
            "namespace caldera_quadruped_curves {", "struct Key { float time, value; };"]
    for species, path in inputs.items():
        code.append(f"namespace {species} {{")
        for curve in ET.parse(path).findall("./defs/animated"):
            keys = [(int(mark.get("time")[:-1]), float(mark.find("real").get("value"))) for mark in curve]
            if sorted({frame for frame, _ in keys}) != [frame for frame, _ in keys] or keys[0][0] != 0 or keys[-1][0] != 60:
                raise ValueError(f"Invalid Synfig curve: {species}/{curve.get('id')}")
            row = ",".join(f"{{{frame / 60:.7f}f,{amount:.6f}f}}" for frame, amount in keys)
            code.append(f"inline constexpr Key {curve.get('id')}[]={{ {row} }};")
        code.append("}")
    header.write_text("\n".join(code + ["}", ""]), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("folder", type=Path)
    parser.add_argument("header", type=Path)
    parser.add_argument("--author", action="store_true")
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    if args.author:
        args.folder.mkdir(parents=True, exist_ok=False)
    masters = {species: author(args.folder, species) if args.author else args.folder / f"{species}-ruch.sif" for species in TRACKS}
    export(masters, args.header)

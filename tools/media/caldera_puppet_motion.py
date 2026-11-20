"""Edytowalne tory dwóch animacji wycinankowych Synfiga i eksport kluczy do gry."""
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image

from hero_synfig import group, image, value


# Czas w klatkach roboczych. W grze fazę rzutu i upadku określa logika walki.
TRACKS = {
    "impCast": {
        "crouch": ((0, 0), (8, .40), (17, 1), (22, .97), (37, 0), (60, 0)),
        "gather": ((0, 0), (5, .12), (16, .92), (19, 1), (24, .48), (27, 0), (60, 0)),
        "throwing": ((0, 0), (17, 0), (22, .23), (27, 1), (31, .94), (42, .36), (54, 0), (60, 0)),
        "unwind": ((0, 0), (25, 0), (34, 1), (46, .32), (60, 0)),
    },
    "wraithCast": {
        "crouch": ((0, 0), (9, .22), (19, .9), (23, .7), (36, 0), (60, 0)),
        "gather": ((0, 0), (8, .19), (17, .91), (21, 1), (27, 0), (60, 0)),
        "throwing": ((0, 0), (18, 0), (24, .46), (27, 1), (34, .70), (49, .07), (57, 0), (60, 0)),
        "unwind": ((0, 0), (24, 0), (35, 1), (43, .8), (59, 0), (60, 0)),
    },
    "impFall": {
        "buckles": ((0, 0), (8, .12), (19, .85), (25, 1), (60, 1)),
        "topples": ((0, 0), (9, .04), (18, .24), (32, .73), (45, .98), (52, 1), (60, 1)),
        "landed": ((0, 0), (27, 0), (38, .27), (48, .91), (55, 1), (60, 1)),
        "recoil": ((0, 0), (6, 1), (15, .43), (24, 0), (60, 0)),
    },
    "wraithFall": {
        "buckles": ((0, 0), (10, .14), (19, .36), (29, .74), (44, .97), (52, 1), (60, 1)),
        "topples": ((0, 0), (12, .01), (18, .14), (31, .71), (39, .92), (53, 1), (60, 1)),
        "landed": ((0, 0), (27, 0), (37, .34), (48, .90), (55, 1), (60, 1)),
        "recoil": ((0, 0), (12, .80), (26, .36), (43, 0), (60, 0)),
    },
    "impWalk": {
        "stride": ((0, 1), (15, 0), (30, -1), (45, -.08), (60, 1)),
        "lift": ((0, 0), (7, .50), (15, 1), (23, .54), (30, 0), (60, 0)),
        "bob": ((0, .64), (15, -.61), (30, .64), (45, -.61), (60, .64)),
    },
    "wraithFlight": {
        "wave": ((0, 1), (15, .08), (30, -1), (45, -.18), (60, 1)),
        "hem": ((0, 0), (12, .91), (22, 1), (33, -.04), (48, -.86), (60, 0)),
    },
}


def write_track(parent, track, knots):
    curve = ET.SubElement(parent, "animated", id=track, type="real")
    for frame, number in knots:
        key = ET.SubElement(curve, "waypoint", time=f"{frame}f", before="halt", after="halt")
        ET.SubElement(key, "real", value=str(number))


def sample(knots, frame):
    for (start, a), (end, b) in zip(knots, knots[1:]):
        if frame <= end:
            u = (frame-start)/(end-start)
            return a + (b-a)*u*u*(3-2*u)
    return knots[-1][1]


def transform(parent, kind, name, channel, points, value_type, origin=None, frames=None):
    layer = ET.SubElement(parent, "layer", type=kind, active="true", version="0.1", desc=name)
    if origin is not None:
        value(layer, "origin", "vector", origin)
    curve = ET.SubElement(ET.SubElement(layer, "param", name=channel), "animated", type=value_type)
    for frame, number in zip(frames, points, strict=True) if frames is not None else enumerate(points):
        key = ET.SubElement(curve, "waypoint", time=f"{frame}f", before="linear", after="linear")
        if value_type == "vector":
            vec = ET.SubElement(key, "vector")
            ET.SubElement(vec, "x").text = str(number[0])
            ET.SubElement(vec, "y").text = str(number[1])
        else:
            ET.SubElement(key, value_type, value=str(number))


def assemble(canvas, imp, tracks):
    def samples(name, default=0):
        return [sample(tracks[name], t) for t in range(60)] if name in tracks else [default]*60

    def paint(parent, tile, x, y, width, height):
        image(parent, ("i" if imp else "w")+str(tile), (x-width/2, y+height/2, x+width/2, y-height/2))

    crouch = samples("crouch")
    gathered = samples("gather")
    throws = samples("throwing")
    body = group(canvas, "Tułów i mocowania")
    paint(body, 9 if imp else 4, -.21, .19 if imp else .40, .37, .23 if imp else .56)
    for side in (-1, 1):
        arm = group(body, "Ręka prawa" if side>0 else "Ręka lewa")
        paint(arm, 4 if imp else 6, side*.18, .57, .13, .22)
        paint(arm, 5 if imp else 7, side*.21, .39, .12 if imp else .18, .20 if imp else .30)
        stride = [sample(tracks["stride"], (t+(30 if side<0 else 0))%60) for t in range(60)] if "stride" in tracks else [0]*60
        angles = [side*(9+37*a+87*b)+14*c for a,b,c in zip(gathered,throws,stride)]
        transform(arm,"rotate","Ramię i przedramię","amount",angles,"angle",(side*.18,.67))
        if imp:
            leg = group(body, "Noga prawa" if side>0 else "Noga lewa")
            paint(leg, 6, side*.09, .25, .12, .19)
            paint(leg, 7, side*.10, .10, .095, .22)
            transform(leg,"rotate","Krok","amount",[19*x for x in stride],"angle",(side*.08,.28))
        else:
            fabric = group(body, "Poła z opóźnieniem")
            paint(fabric,5,side*.15,.41,.19,.45)
            transform(fabric,"rotate","Kołysanie brzegu","amount",[side*7+10*a for a in samples("hem")],"angle",(side*.12,.64))
    paint(body,3 if imp else 2,0,.46 if imp else .64,.29 if imp else .38,.32 if imp else .20)
    paint(body,0,0,.71 if imp else .78,.21 if imp else .23,.20 if imp else .25)
    if imp:
        paint(body,1,-.085,.83,.13,.19)
        paint(body,1,.085,.83,.13,.19)
    paint(body,2 if imp else 1,.025,.67 if imp else .75,.18,.17 if imp else .21)
    bob = samples("bob" if imp else "wave")
    falls = samples("landed")
    tilt = [(-11*a+14*b+74*c) for a,b,c in zip(gathered,throws,samples("topples"))]
    transform(body,"rotate","Przeniesienie ciężaru","amount",tilt,"angle",(0,.06))
    vertical = [(.025*y-.047*g-.13*f) for y,g,f in zip(bob,crouch,falls)]
    transform(body,"translate","Wysokość i osiadanie","origin",[(0,y) for y in vertical],"vector")


def author(folder, name, tracks):
    imp = name.startswith("imp")
    canvas = ET.Element("canvas", version="1.2", width="170", height="140", fps="60",
                        **{"view-box": "-.85 1.20 .85 -.20", "begin-time": "0f", "end-time": "59f",
                           "bgcolor": "0 0 0 0", "antialias": "1"})
    ET.SubElement(canvas,"name").text = "Emberfall / "+name
    controls = ET.SubElement(canvas,"defs")
    for channel, clip in tracks.items():
        write_track(controls, channel, clip)
    assemble(canvas, imp, tracks)
    path = folder / (name+".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def export(projects, header):
    lines = ["#pragma once", "// Generated from the Synfig waypoint definitions.",
             "namespace caldera_puppets::motion {", "struct Waypoint {float time,value;};"]
    for project in projects:
        for channel in ET.parse(project).findall("defs/animated"):
            keys = [(int(w.get("time")[:-1])/60, float(w.find("real").get("value"))) for w in channel]
            assert keys[0][0] == 0 and keys[-1][0] == 1
            values = ",".join(f"{{{t:.8f}f,{v:.8f}f}}" for t,v in keys)
            key_name = channel.get("id")
            lines.append(f"inline constexpr Waypoint {project.stem}_{key_name}[]={{ {values} }};")
    header.write_text("\n".join(lines + ["}", ""]), encoding="utf-8")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--atlases", type=Path)
    p.add_argument("--header", type=Path, required=True)
    p.add_argument("--no-audio", required=True, action="store_true")
    a = p.parse_args()
    if a.atlases:
        a.output.mkdir(parents=True, exist_ok=True)
        parts = a.output / "parts"
        parts.mkdir(exist_ok=True)
        for atlas, prefix in (("fire-imp-puppet", "i"),("ash-wraith-puppet", "w")):
            with Image.open(a.atlases / (atlas+".png")) as art:
                for n in range(12):
                    x,y = n%3*160,n//3*160
                    art.crop((x+1,y+1,x+159,y+159)).save(parts / f"{prefix}{n}.png")
        projects = [author(a.output,clip,tracks) for clip,tracks in TRACKS.items()]
    else:
        projects = [a.output / (clip+".sif") for clip in TRACKS]
    export(projects,a.header)


if __name__ == "__main__":
    main()

"""Author the forest wolf's 2D Synfig pose controls and export runtime keys."""
import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from guardian_throw_synfig import vector
from hero_synfig import group, value
import synfig_cli


TRACKS = (
    ("withers", "chest", "hip"), ("head", "reach", "lift"),
    ("face", "pitch", "jaw"), ("follow", "tail", "ears"),
    ("paw", "stride", "clearance"), ("weight", "weight", "sway"),
)
# At 60 Hz the real game's .42 s melee tell ends at 25.2f.
ATTACK = (
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    (8, -1, -.6, -1, -.5, -5, 1, -1, -8, 0, 0, -1, -.2),
    (17, -3.8, -2.5, -3.2, -2.6, -9, 8, -3, -23, 0, 0, -2.3, -.5),
    (22, -4.2, -3.6, -4.2, -2, -8, 14, -4, -28, -.5, .4, -2.7, -.6),
    (24, -1.5, -2.3, .5, .3, 4, 30, -2, -18, 2.6, 4, -.4, 0),
    (25.2, .2, -1.3, 7.6, .6, -5, 7, -.3, -13, 6.4, 1, 2.7, .4),
    (27.5, -2.2, -.7, 8.5, -1.2, -10, 1, 1.7, -9, 6.6, 0, 3.3, .6),
    (33, -.8, .5, 2.6, -.5, -3, .6, 1.9, -4, 3.2, 1.1, 1.1, .25),
    (39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
)


def attack_at(frame):
    for a, b in zip(ATTACK, ATTACK[1:]):
        if frame <= b[0]:
            t = (frame-a[0])/(b[0]-a[0])
            t = t*t*(3-2*t)
            return tuple(x+(y-x)*t for x, y in zip(a[1:], b[1:]))
    return ATTACK[-1][1:]


def walk_at(frame, frames):
    a = frame/frames*math.tau
    return (-.52*math.cos(2*a+.5), -.36*math.cos(2*a-1.1),
            .32*math.sin(2*a+.6), -.21*math.cos(2*a-.3),
            -1.62*math.sin(2*a+.25), 0, 2.3*math.sin(a-.8),
            1.35*math.sin(2*a-1), 0, 0, 0, .32*math.sin(a))


def walking_foot(phase):
    """Projection of the distance-driven paw path used by forest_gait.hpp."""
    support, stride, clearance = .58, 17.6, 4.8
    if phase <= support:
        return stride*(1-2*phase/support), 0
    swing = (phase-support)/(1-support)
    tangent = -2*(1-support)/support
    ramp = .23
    cruise = (2-ramp*tangent)/(1-ramp)

    def ramp_travel(time):
        u = time/ramp
        return -1+tangent*time+(cruise-tangent)*ramp*u**3*(1-.5*u)

    if swing < ramp:
        travel = ramp_travel(swing)
    elif swing > 1-ramp:
        travel = -ramp_travel(1-swing)
    else:
        travel = ramp_travel(ramp)+cruise*(swing-ramp)
    return travel*stride, clearance*math.sin(math.pi*swing)**3


def control(parent, name, positions, radius, color):
    layer = ET.SubElement(parent, "layer", type="circle", active="true",
                          version="0.2", desc=name)
    value(layer, "amount", "real", 1)
    value(layer, "blend_method", "integer", 0)
    value(layer, "radius", "real", radius)
    rgb = ET.SubElement(ET.SubElement(layer, "param", name="color"), "color")
    for key, channel in zip("rgba", (*color, 1)):
        ET.SubElement(rgb, key).text = str(channel)
    vector(layer, "origin", positions)


def author(path, frames, rows):
    canvas = ET.Element("canvas", version="1.2", width="352", height="220", fps="60",
                        **{"view-box": "-64 68 64 -12", "antialias": "2",
                           "begin-time":"0f", "end-time":f"{frames-1}f", "bgcolor":"0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / Wilk / "+path.stem
    ET.SubElement(canvas, "desc").text = "Punkty kontrolne sylwetki, pyska, szczęki i kontaktu łap."
    for index, (track, _, _) in enumerate(TRACKS):
        layer = ET.SubElement(group(canvas, track), "layer", type="translate",
                              active="true", version="0.1", desc="runtime:"+track)
        vector(layer, "origin", [row[index*2:index*2+2] for row in rows])
    control(canvas, "Biodra", [(-15+w, 26+h) for c,h,r,l,p,j,t,e,s,k,w,y in rows], 9, (.37,.37,.34))
    control(canvas, "Kłąb", [(14+w, 29+c) for c,h,r,l,p,j,t,e,s,k,w,y in rows], 11, (.33,.35,.34))
    control(canvas, "Głowa", [(27+r, 39+c+l) for c,h,r,l,p,j,t,e,s,k,w,y in rows], 7.6, (.47,.48,.42))
    control(canvas, "Nos", [(42+r, 35+c+l+p*.15) for c,h,r,l,p,j,t,e,s,k,w,y in rows], 4.9, (.62,.60,.53))
    control(canvas, "Szczęka", [(37+r, 32+c+l-j*.16) for c,h,r,l,p,j,t,e,s,k,w,y in rows], 3.8, (.32,.29,.24))
    control(canvas, "Ogon", [(-35,19+t) for c,h,r,l,p,j,t,e,s,k,w,y in rows], 5.1, (.26,.27,.26))
    for index, offset in enumerate((.18,0,.68,.50)):
        front=index%2==1
        points=[]
        for frame,row in enumerate(rows):
            if path.stem=="chód":
                f,z=walking_foot((frame/frames+offset)%1)
            else:
                f=row[8] if front else 0
                z=row[9] if front else 0
            points.append(((14 if front else -16)+f, 3+z+(index>=2)))
        control(canvas, f"Łapa {index+1}", points, 2.5,
                (.66,.65,.55) if index<2 else (.50,.49,.40))
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def read_tracks(project):
    data = {}
    for layer in ET.parse(project).getroot().iter("layer"):
        name = layer.get("desc", "").removeprefix("runtime:")
        if name in {track[0] for track in TRACKS}:
            data[name] = [(float(w.findtext("vector/x")), float(w.findtext("vector/y")))
                          for w in layer.findall("param[@name='origin']/animated/waypoint")]
    return [tuple(k for pair in row for k in pair)
            for row in zip(*(data[name] for name, _, _ in TRACKS), strict=True)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    fields = [field for _, x, y in TRACKS for field in (x, y)]
    lines = ["#pragma once", "// Native Synfig waypoints from tools/media/wolf_motion_synfig.py.",
             "namespace ember::wolf_motion {",
             "struct Key {float "+",".join(fields)+";};"]
    receipts = []
    for name, stem, frames, fn in (("walk","chód",42,lambda f:walk_at(f,42)),
                                    ("bite","ugryzienie",39,attack_at)):
        path=args.root/(stem+".sif")
        author(path, frames, [fn(f) for f in range(frames+1)])
        rows=read_tracks(path)
        if len(rows)!=frames+1:
            raise ValueError(f"{path} is missing pose controls")
        lines.append("inline constexpr Key "+name+"[]={")
        lines.extend("{"+",".join(f"{v:.6f}f" for v in row)+"}," for row in rows)
        lines.append("};")
        receipts.append(synfig_cli.render(path, output_root=args.root/"renders",
                        threads=2, expected_frames=frames, require_motion=True))
    lines.append("}")
    args.header.write_text("\n".join(lines)+"\n", encoding="utf-8")
    (args.root/"render-receipt.json").write_text(json.dumps(receipts,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main()

"""Author two non-looping melee clips and export the native Synfig controls."""
import argparse
import json
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

from guardian_throw_synfig import fixed_arm, vector
from hero_synfig import group, image, rotate
import synfig_cli

# Frame, right hand target relative to shoulder, torso forward/up, twist, crown.
CLIPS = {
    "guardianPunch": [
        (0, 4, 5, -20, 0, 0, 0, 0), (12, 9, -7, -13, -1, -2, -15, 0),
        (26, 9, -17, -5, -4, -5, -34, 0), (32, 8, -18, -5, -4, -5, -37, 0),
        (34, 9, 0, -3, 0, -4, -11, 0), (36, -8, 30, -5, 9, -3, 27, 0),
        (39, -13, 26, -10, 9, -6, 32, 0), (43, -8, 16, -13, 6, -5, 21, 0),
        (50, 2, 8, -18, 2, -2, 6, 0), (58, 4, 5, -20, 0, 0, 0, 0),
    ],
    "treantSwing": [
        (0, 15, 5, -29, 0, 0, 0, 0), (13, 21, -8, -10, -1, -1, -3, -2),
        (32, 18, -17, 20, -4, 1, -10, -6), (39, 17, -18, 24, -4, 1, -11, -6),
        (43, 11, -5, 26, 0, 2, -6, -5), (47, -4, 31, -19, 8, -6, 9, 5),
        (52, -9, 28, -23, 8, -8, 11, 8), (58, 3, 19, -27, 5, -5, 6, 3),
        (72, 15, 5, -29, 0, 0, 0, 0),
    ],
}
TRACKS = ("handPlane", "handSide", "body", "follow")


def author(root, name, keys):
    rows = []
    for a, b in zip(keys, keys[1:]):
        for f in range(a[0], b[0]):
            t = (f-a[0])/(b[0]-a[0])
            t = t*t*(3-2*t)
            rows.append(tuple(x+(y-x)*t for x,y in zip(a[1:],b[1:])))
    rows.append(keys[-1][1:])
    frames = len(rows)-1
    canvas = ET.Element("canvas", version="1.2", width="420", height="370", fps="60",
                        **{"view-box": "-100 160 120 -32", "begin-time": "0f",
                           "end-time": f"{frames-1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / "+name
    tree = name == "treantSwing"
    height, upper, forearm = (70, 21, 20) if tree else (53, 17, 16)
    center, shoulder, elbow, hand, crown = [], [], [], [], []

    def project(p):
        r,f,z = p
        return (r*.74+f*.64,z-f*.26)

    for right,forward,up,body,rise,twist,lag in rows:
        pivot = (16 if tree else 23, body+(twist*.6 if tree else 0), height+rise)
        tip = (pivot[0]+right,pivot[1]+forward,pivot[2]+up)
        joint, fist = fixed_arm(pivot,tip,upper,forearm)
        yaw = math.radians(0 if tree else -twist)
        c,s = math.cos(yaw),math.sin(yaw)
        def body_point(p):
            r,f,z = p
            return project((r*c+f*s,f*c-r*s,z))
        shoulder.append(body_point(pivot))
        elbow.append(body_point(joint))
        hand.append(body_point(fist))
        center.append(body_point((0,body,height-10+rise)))
        crown.append(body_point((lag,body,height+(36 if tree else 14)+rise)))

    def part(name, width, height, positions):
        layer = group(canvas, name)
        image(layer, "surface", (-width/2,height/2,width/2,-height/2))
        move = ET.SubElement(layer,"layer",type="translate",active="true",version="0.1")
        vector(move,"origin",positions)

    def limb(name, width, a, b):
        length = max(math.dist(x,y) for x,y in zip(a,b))
        layer = group(canvas,name)
        image(layer,"surface",(-width/2,length/2,width/2,-length/2))
        stretch = ET.SubElement(layer,"layer",type="stretch",active="true",version="0.1")
        vector(stretch,"amount",[(1,math.dist(x,y)/length) for x,y in zip(a,b)])
        rotate(layer,"preview "+name,(0,0),[-math.degrees(math.atan2(y[0]-x[0],y[1]-x[1])) for x,y in zip(a,b)])
        move = ET.SubElement(layer,"layer",type="translate",active="true",version="0.1")
        vector(move,"origin",[((x[0]+y[0])/2,(x[1]+y[1])/2) for x,y in zip(a,b)])

    part("Tułów",36,44,center)
    part("Głowa i korona" if tree else "Głowa",30,26,crown)
    limb("Prawe ramię",13,shoulder,elbow)
    limb("Prawe przedramię",12,elbow,hand)
    part("Konar" if tree else "Prawa pięść",16,16,hand)
    for track,columns in zip(TRACKS,((1,2),(0,None),(3,4),(5,6))):
        layer = ET.SubElement(group(canvas,track),"layer",type="translate",active="true",
                              version="0.1",desc="runtime:"+track)
        vector(layer,"origin",[(row[columns[0]],row[columns[1]] if columns[1] is not None else 0)
                               for row in rows])
    path = root/(name+".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path,encoding="utf-8",xml_declaration=True)
    tracks = {}
    for layer in ET.parse(path).getroot().iter("layer"):
        name = layer.get("desc","").removeprefix("runtime:")
        if name in TRACKS:
            tracks[name] = [(float(w.findtext("vector/x")),float(w.findtext("vector/y")))
                            for w in layer.findall("param[@name='origin']/animated/waypoint")]
    assert all(len(tracks[name])==frames+1 for name in TRACKS)
    return path,[(s[0],*p,*b,*f) for p,s,b,f in zip(*(tracks[name] for name in TRACKS))]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("root","stone","bark","header"):
        parser.add_argument("--"+name,type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True,exist_ok=False)
    lines = ["#pragma once", "// Exported from native Synfig by tools/media/forest_melee_synfig.py.",
             "namespace forest_melee {", "struct Key {float side,forward,up,lean,drop,twist,follow;};"]
    receipts = []
    for (name,keys),surface in zip(CLIPS.items(),(args.stone,args.bark)):
        root = args.root/name
        (root/"parts").mkdir(parents=True)
        shutil.copyfile(surface,root/"parts/surface.png")
        path,rows = author(root,name,keys)
        lines.append("inline constexpr Key "+name+"[]={")
        lines.extend("{"+",".join(f"{v:.6f}f" for v in row)+"}," for row in rows)
        lines.append("};")
        receipts.append(synfig_cli.render(path,output_root=root/"renders",threads=2,
                                          expected_frames=len(rows)-1,require_motion=True))
    fields = ("side","forward","up","lean","drop","twist","follow")
    lines.extend(["template<size_t N> inline Key sample(const Key (&clip)[N],float phase){",
                  " float f=std::clamp(phase,0.f,1.f)*(N-1);size_t i=std::min(size_t(f),N-2);float t=f-i;",
                  " auto a=clip[i],b=clip[i+1];return {"+",".join(f"a.{field}+(b.{field}-a.{field})*t" for field in fields)+"};",
                  "}", "}"])
    args.header.write_text("\n".join(lines)+"\n",encoding="utf-8")
    (args.root/"render-receipt.json").write_text(json.dumps(receipts,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main()

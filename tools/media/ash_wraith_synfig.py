"""Author the Ash Wraith's layered cloth rig, render it with Synfig and export its keys."""
import argparse
import json
import math
from pathlib import Path
import re
import shutil
import xml.etree.ElementTree as ET

import synfig_cli
from hero_synfig import group, value
from PIL import Image


# Each handle is a sewn point on the robe. Positions are in world-art pixels.
HANDLES = (
    ("Piersi", (0, 0)), ("Kaptur", (0, 21)), ("Tylna poła", (0, -45)),
    ("Lewa poła", (-10, -39)), ("Prawa poła", (12, -36)),
    ("Lewy rękaw", (-24, -8)), ("Prawy rękaw", (24, -9)),
)
DURATIONS = {"float": 108, "flight": 72, "cast": 60}


def loop(name, t):
    phase = 2 * math.pi * t
    wave = lambda lag: math.sin(phase - lag)
    travel = name == "flight"
    return [(1.6*wave(.3), 2.8*wave(0)),
            (-1.8*wave(.66), 1.0*wave(.75)),
            ((7 if travel else 4.3)*wave(1.8), 2.2*wave(1.3)),
            ((5.8 if travel else 3.7)*wave(1.4), 2.1*wave(1.9)),
            ((5.4 if travel else 3.4)*wave(2.6), 2.0*wave(2.8)),
            ((6.5 if travel else 3.6)*wave(.9), 2.4*wave(1.25)),
            ((7.2 if travel else 4.2)*wave(2.15), 2.8*wave(2.4))]


# Shoulders bow in preparation, the hands gather, then the right cuff snaps
# along the aim on frame 27. The projectile is created at exactly 27/60.
CAST = {
    0:  [(0,0)]*7,
    10: [(-2,1),(-1,0),(2,-1),(1,-2),(-1,-1),(5,4),(-4,4)],
    19: [(-4,3),(-1,-1),(4,-2),(1,-5),(-2,-4),(10,11),(-8,15)],
    23: [(-4,3),(-1,0),(4,-1),(2,-5),(-3,-3),(11,12),(-10,16)],
    27: [(4,1),(2,-1),(-6,2),(-4,1),(-6,0),(3,7),(20,9)],
    33: [(5,-1),(2,-2),(-8,3),(-6,2),(-8,1),(1,2),(21,6)],
    43: [(1,0),(0,1),(-3,-1),(-2,2),(1,1),(-2,-1),(8,2)],
    60: [(0,0)]*7,
}


def painted_parts(keys, rig, source, work):
    # SE shows both sleeves, the hood opening and the split front skirt.
    view=1
    metadata=rig.read_text(encoding="utf-8")
    def rows(key):
        body=metadata.split(key,1)[1].split("};",1)[0].split("={",1)[1]
        return [re.findall(r"-?\d+(?:\.\d+)?",line) for line in body.splitlines() if "{" in line]
    order=list(map(int,rows(" order[")[view]))
    pivots=list(map(float,rows(" pivots[")[view]))
    hands=list(map(float,rows(" palms[")[view]))
    layers=[]
    for i in order:
        im=Image.open(keys/f"{view}-{i}.png").convert("RGBA")
        box=im.getchannel("A").getbbox()
        file=f"parts/cloth-{i}.png"
        (work/"parts").mkdir(exist_ok=True)
        (source/"parts").mkdir(exist_ok=True)
        im.crop(box).save(work/file)
        shutil.copyfile(work/file,source/file)
        layers.append((i,[v*128/im.width for v in box],pivots[2*i:2*i+2],
                       hands[2*(i-4):2*(i-4)+2] if i>=4 else None,file))
    return layers


def painted_layer(root, layer, poses, cast):
    i,(left,top,right,bottom),pivot,hand,file=layer
    n=1 if i==3 else i+2 if i<3 else i+1
    canvas=group(root,HANDLES[n][0])
    paint=ET.SubElement(canvas,"layer",type="import",version="0.1",active="true",desc=file)
    value(paint,"amount","real",1);value(paint,"blend_method","integer",0)
    value(paint,"filename","string",file)
    value(paint,"tl","vector",(left-64,64-top));value(paint,"br","vector",(right-64,64-bottom))
    warp=ET.SubElement(canvas,"layer",type="warp",version="0.1",active="true",desc="Ugięcie tkaniny")
    value(warp,"src_tl","vector",(left-64,64-top))
    value(warp,"src_br","vector",(right-64,64-bottom))
    value(warp,"clip","bool","false")
    def window(x,a,b):
        w=max(0,min(1,(x-a)/(b-a)))
        return w*w*(3-2*w)
    for corner,x,y in [("tl",left,top),("tr",right,top),("br",right,bottom),("bl",left,bottom)]:
        track=ET.SubElement(ET.SubElement(warp,"param",name="dest_"+corner),"animated",type="vector")
        if i==3:
            weight=1-window(y,pivot[1]-8,pivot[1]+19)
        elif i<3:
            weight=window(y,pivot[1]-3,110)
        else:
            dx,dy=hand[0]-pivot[0],hand[1]-pivot[1]
            weight=window((x-pivot[0])*dx+(y-pivot[1])*dy,0,max(1,dx*dx+dy*dy))
        for frame,pose in poses.items():
            rx,ry=pose[0]
            dx,dy=pose[n]
            if cast:
                rx,ry=.72*rx,ry-.36*rx
                dx,dy=.70*dx,dy+(.37 if i==4 else -.36)*dx
            dest=(x-64+rx+weight*dx,64-y+ry+weight*dy)
            interp="halt" if cast else "clamped"
            wp=ET.SubElement(track,"waypoint",time=f"{frame}f",before=interp,after=interp)
            point=ET.SubElement(wp,"vector")
            for axis,v in zip("xy",dest):ET.SubElement(point,axis).text=f"{v:.5f}"


def project(path, name, layers):
    end = DURATIONS[name]
    root = ET.Element("canvas", version="1.2", width="240", height="264", fps="60",
                      antialias="2", **{"view-box":"-60 55 60 -77", "begin-time":"0f",
                      "end-time":f"{end}f", "bgcolor":"0 0 0 0"})
    ET.SubElement(root, "name").text = "Emberfall / Popielny Upiór / " + name
    values = CAST if name == "cast" else {
        round(end*i/12):loop(name, i/12) for i in range(13)
    }
    for layer in layers:painted_layer(root,layer,values,name=="cast")
    for index,(label,(x,y)) in enumerate(HANDLES):
        node=ET.SubElement(root,"layer",type="circle",version="0.2",active="false",desc=label)
        value(node,"amount","real",1); value(node,"blend_method","integer",0)
        value(node,"radius","real",2.3 if index<2 else 1.6)
        color=ET.SubElement(ET.SubElement(node,"param",name="color"),"color")
        for channel,v in zip("rgba",(.78 if index==1 else .47,.59,.66,1)):
            ET.SubElement(color,channel).text=str(v)
        track=ET.SubElement(ET.SubElement(node,"param",name="origin"),"animated",type="vector")
        for frame,pose in values.items():
            interp="halt" if name=="cast" else "clamped"
            wp=ET.SubElement(track,"waypoint",time=f"{frame}f",before=interp,after=interp)
            point=ET.SubElement(wp,"vector")
            ET.SubElement(point,"x").text=f"{x+pose[index][0]:.6f}"
            ET.SubElement(point,"y").text=f"{y+pose[index][1]:.6f}"
    ET.indent(root)
    ET.ElementTree(root).write(path,encoding="utf-8",xml_declaration=True)


def export(source, name):
    tracks=[]
    for node,(_,origin) in zip(ET.parse(source).findall("layer[@type='circle']"),HANDLES):
        tracks.append([(int(p.get("time")[:-1]),tuple(float(p.findtext("vector/"+axis))-base
                        for axis,base in zip("xy",origin))) for p in node.findall(
                            "param[@name='origin']/animated/waypoint")])
    result=[f"inline constexpr Key {name}Keys[]={{"]
    for idx,(frame,_) in enumerate(tracks[0]):
        points=", ".join("{"+", ".join(f"{k:.4f}f" for k in t[idx][1])+"}" for t in tracks)
        result.append(f" {{{frame}.f/{DURATIONS[name]}.f,{{{points}}}}},")
    return result+["};"]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--header",type=Path,required=True)
    parser.add_argument("--source",type=Path,required=True)
    parser.add_argument("--keys",type=Path,required=True)
    parser.add_argument("--rig",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    args.root.mkdir(parents=True,exist_ok=False)
    args.source.mkdir(parents=True,exist_ok=True)
    layers=painted_parts(args.keys,args.rig,args.source,args.root)
    lines=["#pragma once","// Exported from Synfig by tools/media/ash_wraith_synfig.py.",
           "namespace ash_wraith_motion {","struct Key {float time; Vector2 point[7];};"]
    receipts={}
    for name,end in DURATIONS.items():
        path=args.root/(name+".sif")
        project(path,name,layers)
        lines+=export(path,name)
        shutil.copyfile(path,args.source/path.name)
        receipts[name]=synfig_cli.render(path,output_root=args.root/("render-"+name),
                                         threads=2,expected_frames=end+1,require_motion=True)
    args.header.write_text("\n".join(lines+["}",""]),encoding="utf-8")
    (args.root/"render-receipt.json").write_text(json.dumps(receipts,indent=2),encoding="utf-8")


if __name__=="__main__":
    main()

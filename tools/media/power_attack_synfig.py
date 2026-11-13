"""Author and export the five charged attack timing tracks for Emberfall."""
import argparse
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

from hero_synfig import group, image, rotate
from sword_stow_synfig import animated

# Phase of the equipped weapon rig, body lean, chest turn and bow recoil.
CLIPS = {
    "sword": [(0,.30,-5,-4,0),(.18,.40,-1,9,0),(.359375,.50,4,-3,0),
              (.56,.75,6,-17,0),(.77,.86,3,-7,0),(1,1,0,0,0)],
    "axe": [(0,.308,-7,-3,0),(.25,.42,-2,0,0),(.419,.519,13,2,0),(.57,.72,10,1,0),(1,1,0,0,0)],
    "dagger": [(0,.19,-4,-10,0),(.30,.421,8,-12,0),(.45,1,0,0,0),(.48,0,0,0,0),
               (.6333,.421,12,-14,0),(.84,.83,4,-4,0),(1,1,0,0,0)],
    "spear": [(0,.28,-6,-5,0),(.21,.33,1,-5,0),(.38235,.40,12,-7,0),(.57,.56,8,-3,0),(1,1,0,0,0)],
    "bow": [(0,0,-5,0,1),(.17,0,-11,7,.57),(.43,0,3,2,.18),(.70,0,1,0,.035),(1,0,0,0,0)],
}
# Readying the existing weapon rigs. Their last pose matches the release clip.
# The bow's phase lifts its front hand, while recoil sets the visual draw boost.
WINDUPS = {
    "sword": [(0,0,0,0,0),(.23,.055,-1,-1,0),(.53,.18,-3,-2,0),(.80,.27,-4,-3,0)],
    "axe": [(0,0,0,0,0),(.20,.045,-2,0,0),(.50,.16,-4,-1,0),(.77,.27,-6,-2,0)],
    "dagger": [(0,0,0,0,0),(.25,.055,-1,-3,0),(.55,.12,-2,-6,0),(.80,.172,-3,-9,0)],
    "spear": [(0,0,0,0,0),(.21,.042,-1,-1,0),(.51,.146,-3,-3,0),(.79,.24,-5,-4,0)],
    "bow": [(0,0,0,0,0),(.17,.27,-1,-2,.08),(.42,.83,-2,-3,.31),
            (.67,1,-4,-1,.68),(.86,1,-4.8,0,.92),(1,1,-5,0,1)],
}
FIELDS = ("phase", "lean", "turn", "recoil")
FRAMES = 30

# Grip in the pelvis frame, shaft pitch and yaw, and weight over the walking pose.
OVERHEAD_FIELDS = ("handR", "handF", "handZ", "pitch", "yaw", "weight")
OVERHEAD_RELEASE = {
    "sword": [(0,19,4,76,122,0,1),(.10,22,8,72,87,65,1),
              (.22,24,17,61,16,62,1),(.359375,16,24,56,5,7,1),
              (.47,8,24,55,-4,-39,1),(.56,3,22,55,-12,-73,1),
              (.70,13,22,53,18,-51,.86),(.86,22,17,48,56,-17,.36),
              (1,23,14,46,76,0,0)],
    "axe": [(0,19,5,75,113,0,1),(.17,19,10,75,91,0,1),(.27,18,17,67,60,0,1),
            (.419,18,23,49,12,0,1),(.56,19,20,39,-24,0,1),(.72,22,16,44,16,0,.79),
            (.90,24,14,46,66,0,.27),(1,24,14,46,76,0,0)],
}
OVERHEAD_WINDUP = {
    "sword": [(0,23,14,46,76,0,0),(.23,23,15,55,77,0,.37),(.53,22,12,66,93,0,.86),
              (.80,20,7,74,112,0,1),(1,*OVERHEAD_RELEASE["sword"][0][1:])],
    "axe": [(0,24,14,46,76,0,0),(.21,25,15,53,76,0,.34),(.51,24,13,64,89,0,.81),
            (.79,21,8,72,103,0,1),(1,*OVERHEAD_RELEASE["axe"][0][1:])],
}

# The native preview runs at the in-game release speed while the exported rig
# values stay normalized to the original swing clock.
RELEASE_PREVIEW_FRAMES = {"sword": 19, "axe": 32}


def sample(keys, t, settle=False):
    for a,b in zip(keys,keys[1:]):
        if t<=b[0]:
            w=(t-a[0])/(b[0]-a[0])
            if settle and b[0]==1:
                w=w+w*w-w*w*w
            return tuple(x+(y-x)*w for x,y in zip(a[1:],b[1:]))
    return keys[-1][1:]


def author(root, parts):
    root.mkdir(parents=True)
    shutil.copytree(parts,root/"parts")
    for filename in (*CLIPS, *(name+"-windup" for name in WINDUPS)):
        name=filename.removesuffix("-windup")
        windup=filename!=name
        keys=WINDUPS[name] if windup else CLIPS[name]
        if windup and name!="bow":
            keys=[*keys,(1,*CLIPS[name][0][1:])]
        overhead=(OVERHEAD_WINDUP if windup else OVERHEAD_RELEASE).get(name)
        preview_frames=FRAMES if windup else RELEASE_PREVIEW_FRAMES.get(name,FRAMES)
        canvas=ET.Element("canvas",version="1.2",width="300",height="336",fps="60",
                          **{"view-box":"-56 119 65 -12" if overhead else "-50 103 58 -12",
                             "begin-time":"0f","end-time":f"{preview_frames-1}f","bgcolor":"0 0 0 0"})
        ET.SubElement(canvas,"name").text="Emberfall — mocny atak / "+filename
        defs=ET.SubElement(canvas,"defs")
        tracks=list(zip(*(sample(keys,i/FRAMES,windup) for i in range(FRAMES+1))))
        for field,track in zip(FIELDS,tracks):
            animated(defs,field,"real",track)
        overhead_keys=[sample(overhead,i/FRAMES,windup) for i in range(FRAMES+1)] if overhead else []
        for field,track in zip(OVERHEAD_FIELDS,zip(*overhead_keys)):
            animated(defs,field,"real",track)
        image(canvas,"leg",(-13,32,1,0))
        image(canvas,"leg",(1,32,15,0))
        body=group(canvas,"Pochylenie i powrót do gardy")
        image(body,"body",(-17,58,17,29))
        arm=group(body,"Ramię i broń")
        image(arm,"arm",(10,54,24,28))
        if overhead_keys:
            preview=[sample(overhead,i/preview_frames,windup) for i in range(preview_frames+1)]
            weapon=group(body,"Chwyt nad głową")
            image(weapon,"sword",(18,80,29,38))
            rotate(weapon,"preview_pitch",(23,46),[(pitch-76+yaw*.65)*weight for _,_,_,pitch,yaw,weight in preview])
            translation=ET.SubElement(weapon,"layer",type="translate",active="true",version="0.1",desc="Tor dłoni")
            animated(ET.SubElement(translation,"param",name="origin"),"preview_grip","vector",
                     [((r-23+(f-14)*.45)*weight,(z-46)*weight) for r,f,z,_,_,weight in preview])
            rotate(arm,"preview_arm",(13,52),[150*weight for *_,weight in preview])
        else:
            image(arm,"sword",(20,75,29,34))
            rotate(arm,"preview_arm",(13,52),[20+85*p if name!="bow" else 65+35*r for p,r in zip(tracks[0],tracks[3])])
        image(body,"face",(-9,81,9,60))
        image(body,"hair_front",(-10,85,10,61))
        rotate(body,"preview_lean",(0,35),[-sample(keys,i/preview_frames,windup)[1] for i in range(preview_frames+1)])
        ET.indent(canvas)
        ET.ElementTree(canvas).write(root/(filename+".sif"),encoding="utf-8",xml_declaration=True)


def export(root, header):
    lines=["#pragma once","// Exported from the Synfig control tracks.",
           "namespace ember::power_motion::data {","struct Key {float phase,lean,turn,recoil;};",
           "struct OverheadKey {float "+",".join(OVERHEAD_FIELDS)+";};",f"inline constexpr int Frames={FRAMES};"]
    for array,suffix,key_type,fields,names in (
        ("clips","","Key",FIELDS,CLIPS),("windups","-windup","Key",FIELDS,CLIPS),
        ("overheadRelease","","OverheadKey",OVERHEAD_FIELDS,OVERHEAD_RELEASE),
        ("overheadWindup","-windup","OverheadKey",OVERHEAD_FIELDS,OVERHEAD_WINDUP),
    ):
        lines.append(f"inline constexpr {key_type} {array}[][Frames+1]={{")
        for name in names:
            defs=ET.parse(root/(name+suffix+".sif")).getroot().find("defs")
            tracks=[]
            for field in fields:
                points=defs.findall(f"animated[@id='{field}']/waypoint")
                assert [int(p.get("time")[:-1]) for p in points]==list(range(FRAMES+1))
                tracks.append([float(p.find("real").get("value")) for p in points])
            lines.append(" {")
            lines.extend("  {"+",".join(f"{x:.7f}f" for x in key)+"}," for key in zip(*tracks))
            lines.append(" },")
        lines.append("};")
    header.write_text("\n".join(lines+["}",""]),encoding="utf-8")


if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--parts",type=Path,required=True)
    parser.add_argument("--header",type=Path,required=True)
    parser.add_argument("--export-only",action="store_true")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    if not args.export_only:
        author(args.root,args.parts)
    export(args.root,args.header)

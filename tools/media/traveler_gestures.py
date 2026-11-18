"""Krita cutouts and the editable Synfig gesture finishing pass for the traveler."""
import argparse
import gzip
import json
import time
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image, ImageDraw, ImageOps

from hero_synfig import group, value
import synfig_cli

ZERO = (0, 0, 0, 0)
FRAMES = 24
# Four curves (hem sideways, hem lift, chin pitch, head roll). Frames are
# normalised to the real gameplay clock at playback, including slowed releases.
GESTURES = {
    "Axe": {0:ZERO,6:(-2,-1.5,-2.8,-1),11:(-4,1,-1.8,-2),14:(1,3,1.4,1.7),18:(4.6,4.7,2.8,1.3),24:ZERO},
    "Dagger": {0:ZERO,6:(-.8,-.75,-1.15,-.8),10:(-1.6,1.8,2.6,-1.6),13:(2.2,2,1.1,.7),18:(.6,-.5,-.5,.3),24:ZERO},
    "Spear": {0:ZERO,7:(-1.3,-1.5,-2,-1),10:(-2.8,2.2,3.3,-1.3),14:(1.7,5,2.1,.1),19:(1.4,1.1,-.6,.55),24:ZERO},
    "BowDraw": {0:ZERO,6:(.6,-.35,-.75,-.55),14:(2.5,.8,-1.8,-2.1),20:(3.1,1.5,-2.15,-2.8),24:(3,1.4,-2,-2.6)},
    "BowLoose": {0:(3,1.4,-2,-2.6),3:(.8,3.8,-2.7,-3.3),8:(-2.6,4.4,.8,-.75),14:(.65,.8,.85,.5),24:ZERO},
    "SwordCharge": {0:ZERO,7:(-.65,-.6,-1.1,-.4),16:(-3.1,.8,-2.5,-1.7),24:(-4.2,2.4,-3,-2.4)},
    "AxeCharge": {0:ZERO,8:(-.8,-1.4,-1.5,-.6),16:(-3.9,.5,-3.1,-2.2),24:(-5.3,2.8,-4.1,-2.75)},
    "DaggerCharge": {0:ZERO,8:(-.6,-.6,-1.1,-.3),16:(-1.5,-1.4,-1.8,-1.1),24:(-2.4,-2.1,-2.3,-1.3)},
    "SpearCharge": {0:ZERO,8:(.7,-.8,-1,-.55),17:(2.1,-2,-2.4,-1.25),24:(2.7,-2.6,-3,-1.7)},
    "SwordPower": {0:(-4.2,2.4,-3,-2.4),6:(-4.9,1.4,-2,-2.85),9:(1.3,5,2.7,1),15:(6.6,5.7,2.3,3.1),20:(1.7,1.25,-.5,.7),24:ZERO},
    "AxePower": {0:(-5.3,2.8,-4.1,-2.75),8:(-5.7,.5,-2.5,-2.9),12:(1.1,6,3.8,1.4),16:(6,7.2,3.9,2.7),21:(1.5,1.8,-.35,.45),24:ZERO},
    "DaggerPower": {0:(-2.4,-2.1,-2.3,-1.3),6:(-2.6,2.3,2.2,-1.2),9:(2.4,2.6,.6,1.2),13:(1.5,-.6,-1.8,.7),16:(-2,3.5,2,-.7),20:(1.2,1.4,.55,.3),24:ZERO},
    "SpearPower": {0:(2.7,-2.6,-3,-1.7),7:(3,-1.9,-2.7,-2.1),10:(.05,4.4,3.2,-1.1),15:(-3.4,6,2.4,.8),21:(.9,1.7,.45,.3),24:ZERO},
    "BowPower": {0:(3,1.4,-2,-2.6),4:(2,5.5,-3.4,-3.6),8:(-3.3,6.1,1.15,-1.5),15:(.35,1.5,.85,.6),24:ZERO},
    "BeltStow": {0:ZERO,8:(1.6,-.7,2.3,.9),14:(2.8,1.65,1.9,1.6),19:(-.95,1.4,-.55,.55),24:ZERO},
    "BeltDraw": {0:ZERO,5:(.55,-.75,1.5,.65),12:(2.4,.6,2.2,1.25),17:(-.85,2.2,-.8,-.45),24:ZERO},
    "BackStow": {0:ZERO,6:(-.7,.6,-1.5,-1.5),13:(-3.9,2.7,-2.6,-2.7),18:(1.2,3.4,.5,.85),24:ZERO},
    "BackDraw": {0:ZERO,7:(-2.15,1.15,-2.1,-2),13:(-2.75,2.8,-1.25,-2.6),18:(1.75,3.9,.75,.8),24:ZERO},
    "Flinch": {0:ZERO,3:(.55,-2.5,-3.4,-1.7),7:(2.5,2.3,-1.8,-1.1),12:(-1.9,3.3,1.4,.8),18:(.35,.8,.45,.4),24:ZERO},
    "Fall": {0:ZERO,4:(-1.6,-2.5,-1.6,-1),8:(-2.2,5.9,-1.3,-2.3),10:(1,2.2,2.2,1.25),12:(.5,0,.5,.4),15:(.3,-.2,.2,.3),19:(2.7,1.9,2.7,1.75),22:(-.8,1.4,-.6,-.5),24:ZERO},
}
CHANNELS = ("hemSway","hemLift","neckPitch","headRoll")


def image(canvas, name, bounds):
    layer=ET.SubElement(canvas,"layer",type="import",active="true",version="0.1",desc=name)
    value(layer,"filename","string",f"parts/{name}.png")
    value(layer,"tl","vector",bounds[:2])
    value(layer,"br","vector",bounds[2:])
    # The parameter ID in Synfig 1.4.5 is 'c'. Its display name is Interpolation.
    value(layer,"c","integer",1)


def cutouts(folder, atlas_path, heads_path):
    folder.mkdir()
    head=Image.open(heads_path).convert("RGBA").crop((128,0,256,128))
    head.crop(head.getbbox()).save(folder/"head.png")
    with Image.open(atlas_path) as atlas:
        for label, cell, dark, light, outline in (
            ("coat",3,"#223642","#7193a0",[(19,4),(45,0),(80,0),(110,9),(98,66),(109,115),(79,127),(63,112),(50,126),(16,116),(28,65)]),
            ("mantle",7,"#1d4140","#5d968e",[(37,8),(90,4),(105,15),(100,42),(119,111),(88,127),(69,118),(44,126),(11,109),(23,47),(21,21)]),
        ):
            x,y=cell%4*128,cell//4*128
            tex=ImageOps.colorize(atlas.crop((x,y,x+128,y+128)).convert("L"),dark,light)
            alpha=Image.new("L",(128,128));ImageDraw.Draw(alpha).polygon(outline,fill=255)
            tex.putalpha(alpha);tex.save(folder/(label+".png"))


def track(layer,name,field,rows):
    vector=name=="translate"
    xf=ET.SubElement(layer,"layer",type=name,version="0.1",active="true",desc="runtime:"+field)
    if not vector:
        value(xf,"origin","vector",(0, 10 if field=="hemSway" else 17))
    node=ET.SubElement(ET.SubElement(xf,"param",name="origin" if vector else "amount"),
                       "animated",type="vector" if vector else "angle")
    for t,v in rows:
        p=ET.SubElement(node,"waypoint",time=f"{t}f",before="linear",after="linear")
        if vector:
            xy=ET.SubElement(p,"vector")
            ET.SubElement(xy,"x").text="0"
            ET.SubElement(xy,"y").text=f"{v:.6f}"
        else:
            ET.SubElement(p,"angle",value=f"{v:.6f}")


def author(folder):
    count=len(GESTURES)*(FRAMES+1)
    root=ET.Element("canvas",version="1.2",width="112",height="144",fps="60",
                    **{"view-box":"-29 47 29 -28","begin-time":"0f","end-time":f"{count-1}f","bgcolor":"0 0 0 0"})
    ET.SubElement(root,"name").text="Wędrowiec / gesty ekwipunku, trafienie i upadek"
    rows=[]
    for idx,(name,keys) in enumerate(GESTURES.items()):
        start=idx*(FRAMES+1)
        ET.SubElement(root,"keyframe",time=f"{start}f",active="true").text=name
        marks=sorted(keys)
        for a,b in zip(marks,marks[1:]):
            for frame in range(a,b):
                t=(frame-a)/(b-a); t=t*t*(3-2*t)
                rows.append((start+frame,[v+(w-v)*t for v,w in zip(keys[a],keys[b])]))
        rows.append((start+FRAMES,keys[FRAMES]))
    coat=group(root,"Półpłaszcz – opóźniony brzeg")
    image(coat,"mantle",(-17,13,17,-23))
    track(coat,"rotate","hemSway",[(t,v[0]) for t,v in rows])
    track(coat,"translate","hemLift",[(t,v[1]) for t,v in rows])
    image(root,"coat",(-12,17,12,-10))
    head=group(root,"Głowa – prowadzenie wzroku")
    image(head,"head",(-10,35,10,13))
    track(head,"rotate","neckPitch",[(t,v[2]) for t,v in rows])
    track(head,"rotate","headRoll",[(t,v[3]) for t,v in rows])
    source=folder/"gestures.sifz"
    with source.open("xb") as stream,gzip.GzipFile(filename="",mode="wb",fileobj=stream,mtime=0) as z:
        ET.ElementTree(root).write(z,encoding="utf-8",xml_declaration=True)
    return source


def export(source,header):
    with gzip.open(source) as z:
        canvas=ET.parse(z).getroot()
    labels=[(int(k.get("time")[:-1]),k.text) for k in canvas.findall("keyframe")]
    assert [name for _,name in labels]==list(GESTURES)
    columns={}
    for prop in canvas.iter("layer"):
        desc=prop.get("desc","")
        if desc.startswith("runtime:"):
            vals={}
            for p in prop.findall("param/animated/waypoint"):
                item=p[0]
                vals[int(p.get("time")[:-1])]=float(item.findtext("y") if item.tag=="vector" else item.get("value"))
            columns[desc[8:]]=vals
    assert set(columns)==set(CHANNELS)
    lines=["#pragma once","// Exported from gestures.sifz by tools/media/traveler_gestures.py.",
           "namespace ember::traveler_gesture {","enum class Gesture {"+",".join(n for _,n in labels)+",Count};",
           "struct Key {float "+",".join(CHANNELS)+";};",f"inline constexpr int Frames={FRAMES};",
           "inline constexpr Key keys[][Frames+1]={"]
    for start,name in labels:
        lines.append(" { // "+name)
        for t in range(start,start+FRAMES+1):
            lines.append("  {"+",".join(f"{columns[c][t]:.6f}f" for c in CHANNELS)+"},")
        lines.append(" },")
    lines+=["};","static_assert(sizeof(keys)/sizeof(keys[0])==int(Gesture::Count));","}"]
    header.write_text("\n".join(lines)+"\n",encoding="utf-8")


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--directory",type=Path,required=True)
    p.add_argument("--header",type=Path,required=True)
    p.add_argument("--export-only",action="store_true")
    p.add_argument("--render",action="store_true")
    p.add_argument("--no-audio",action="store_true",required=True)
    a=p.parse_args()
    a.directory.mkdir(parents=True,exist_ok=True)
    source=a.directory/"gestures.sifz"
    if not a.export_only:
        assets=Path(__file__).resolve().parents[2]/"assets/sprites"
        cutouts(a.directory/"parts",assets/"hero-paints.png",assets/"hero-heads.png")
        author(a.directory)
    export(source,a.header)
    if a.render:
        out=a.directory/"render-gestures";out.mkdir()
        start=time.perf_counter()
        synfig_cli._run(["-i",source.resolve(),"-T","2","-t","png","-o",(out/"frame.png").resolve(),
                         "--sequence-separator","_","-a","1","--quiet"],out,"render",cwd=a.directory)
        render_s=time.perf_counter()-start
        paths=sorted(out.glob("frame_*.png"))
        assert len(paths)==len(GESTURES)*(FRAMES+1)
        distinct={}
        for i,name in enumerate(GESTURES):
            hashes=set()
            for path in paths[i*(FRAMES+1):(i+1)*(FRAMES+1)]:
                with Image.open(path) as im:
                    rgba=im.convert("RGBA"); assert rgba.getchannel("A").getbbox()
                    hashes.add(rgba.tobytes())
            assert len(hashes)>4,name
            distinct[name]=len(hashes)
        receipt=dict(status="PASS",audio=False,source_sha256=synfig_cli.sha(source),
                     header_sha256=synfig_cli.sha(a.header),synfig_render_s=round(render_s,3),
                     frames=len(paths),distinct_pixel_frames=distinct)
        (a.directory/"gesture-receipt.json").write_text(json.dumps(receipt,indent=2))
        print(json.dumps(receipt))


if __name__=="__main__":
    main()

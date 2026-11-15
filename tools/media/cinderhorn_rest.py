"""Author the two resting flanks from the existing painted Rogacz cuts."""
from __future__ import annotations

import argparse
from collections import deque
import hashlib
import json
import math
from pathlib import Path
import time
from xml.etree import ElementTree as ET

from PIL import Image, ImageChops, ImageDraw, ImageFilter

from cinderhorn_visuals import HOOF, split
from hero_synfig import group, image, value
from regional_enemy_visuals import ANCHOR, PAD, SIZE, STRIDE, masked, polygon_mask
import synfig_cli


# Heading selects the painted side of the animal, body turn and foreshortening.
VIEWS = (
    ("front", -8, 1.05, False), ("front", -5, 1.00, False),
    ("front", -2, .93, False), ("front", -5, 1.00, True),
    ("front", -8, 1.05, True), ("back", -4, .97, True),
    ("back", -1, .90, False), ("back", -4, .97, False),
)

# The original walking cuts contain overlap useful for covering upright
# joints. A clean flank keeps those hooves and horn tips out of the torso.
TORSO_CONTOUR = {
    "front": ((62,265),(118,253),(192,231),(278,199),(326,191),(371,145),(437,140),
              (492,167),(548,203),(587,255),(621,326),(628,399),(592,485),(566,555),
              (533,594),(497,606),(453,594),(414,561),(361,558),(333,533),(299,528),
              (265,506),(229,526),(196,556),(153,562),(116,543),(89,513),(64,439),(76,364)),
    "back": ((131,364),(200,328),(255,267),(326,205),(373,187),(457,171),(522,147),
             (576,162),(628,201),(661,262),(683,301),(677,365),(649,414),(604,454),
             (581,493),(549,539),(507,559),(476,590),(445,613),(416,617),(385,598),
             (352,584),(329,598),(289,583),(255,560),(219,589),(181,548),(150,483),(132,424)),
}


def units(point):
    return (point[0]-72)/36, (72-point[1])/36


def minus(a,b):
    return a[0]-b[0],a[1]-b[1]


def layer(parent,kind,label,**params):
    result=ET.SubElement(parent,"layer",type=kind,active="true",version="0.1",desc=label)
    for key,(kind,data) in params.items():
        value(result,key,kind,data)
    return result


def static_pose(canvas,part,box,pivot,fit,scale,angle,target):
    drawing=group(canvas,part)
    image(drawing,part,(*units(fit(*box[:2])),*units(fit(*box[2:]))))
    layer(drawing,"stretch","Skrót perspektywiczny",amount=("vector",scale),center=("vector",units(pivot)))
    layer(drawing,"rotate","Oparcie",origin=("vector",units(pivot)),amount=("angle",-angle))
    dx,dy=minus(target,pivot)
    layer(drawing,"translate","Przesunięcie stawu",origin=("vector",(dx/36,-dy/36)))


def load_cuts(source,root):
    (root/"parts").mkdir()
    views={}
    for view in ("front","back"):
        painting=Image.open(source/f"{view}.png").convert("RGBA")
        x0,y0,x1,y1=painting.getchannel("A").point(lambda a:255 if a>=40 else 0).getbbox()
        scale=min(92/(y1-y0),97/(x1-x0))
        fit=lambda x,y,center=(x0+x1)/2,bottom=y1,s=scale: (72+(x-center)*s,121+(y-bottom)*s)
        parts={}
        support=[]
        for name,cut,pivot in split(view,painting):
            def save(piece,part,point):
                box=piece.getchannel("A").getbbox()
                if not box:
                    raise ValueError(f"Empty painted part {view}/{part}")
                piece.crop(box).save(root/"parts"/f"{view}-{part}.png")
                parts[part]=(box,fit(*point))

            if name=="body":
                cut=masked(painting,polygon_mask(painting.size,TORSO_CONTOUR[view]).filter(ImageFilter.GaussianBlur(4)))
                alpha=cut.getchannel("A")
                x0,y0,x1,y1=alpha.point(lambda a:255 if a>=134 else 0).getbbox()
                opacity=alpha.load()
                for x in range(x0+int((x1-x0)*.20),x0+int((x1-x0)*.82),4):
                    bottom=next((y for y in range(y1-1,y0-1,-1) if opacity[x,y]>=134),None)
                    if bottom is not None:
                        support.append(fit(x,bottom))
                save(cut,name,pivot)
                continue
            if name not in HOOF[view]:
                save(cut,name,pivot)
                continue
            hoof=HOOF[view][name]
            knee=(pivot[0]+.47*(hoof[0]-pivot[0])+(-12 if name.startswith("fore") else 12),
                  int(pivot[1]+.49*(hoof[1]-pivot[1])))
            upper=cut.copy()
            upper.paste((0,0,0,0),(0,knee[1]+17,cut.width,cut.height))
            lower=cut.copy()
            lower.paste((0,0,0,0),(0,0,cut.width,knee[1]-15))
            save(upper,name,pivot)
            save(lower,name+"-hoof",knee)
        views[view]=(fit,parts,support)
    return views


def amount_at(layer_node, frame):
    amount=layer_node.find("param[@name='amount']")
    amount.clear()
    amount.set("name","amount")
    tracks=ET.SubElement(amount,"animated",type="real")
    for index in range(len(VIEWS)):
        key=ET.SubElement(tracks,"waypoint",time=f"{index}f",before="constant",after="constant")
        ET.SubElement(key,"real",value=str(int(index==frame)))


def author(path,cuts,side):
    canvas=ET.Element("canvas",version="1.2",width="384",height="384",fps="60",**{
        "view-box":"-2.6666667 2.6666667 2.6666667 -2.6666667","antialias":"3","begin-time":"0f","end-time":"7f","bgcolor":"0 0 0 0",
    })
    ET.SubElement(canvas,"name").text=f"Emberfall / Rogacz Żaru / {'Prawy' if side>0 else 'Lewy'} bok"
    ET.SubElement(canvas,"desc").text="Malowane warstwy Krity. Kopyta, płyty pancerza, kark i rogi mają osobne położenie."
    contacts=[]
    for index,(view,angle,depth,mirror) in enumerate(VIEWS):
        fit,parts,support=cuts[view]
        pose=group(canvas,f"Kierunek {index+1}")
        amount_at(canvas.findall("layer")[-1],index)
        body=parts["body"][1]
        roll=math.radians(angle+side*8)
        torso_scale=(depth*(1.04 if side>0 else 1.08),.80 if side>0 else .83)
        c,s=math.cos(roll),math.sin(roll)
        edge=sorted((p[0]-body[0])*torso_scale[0]*s+(p[1]-body[1])*torso_scale[1]*c for p in support)
        torso=(body[0]-side*2,ANCHOR[1]-.8-edge[round((len(edge)-1)*.80)])

        def mount(point):
            x,y=minus(point,body)
            a,b=x*torso_scale[0],y*torso_scale[1]
            return torso[0]+a*c-b*s,torso[1]+a*s+b*c

        contact=[(144-x if mirror else x,y) for x,y in map(mount,support) if y>=ANCHOR[1]-4]
        contacts.append({
            "width": round(max(p[0] for p in contact)-min(p[0] for p in contact),2),
            "depth": [round(min(p[1] for p in contact),2),round(max(p[1] for p in contact),2)],
        })

        # One pair remains partly under the chest. The upper pair rests across
        # the ground with foreshortened legs. Joint origins stay on the hide.
        folded=(side>0) if view=="front" else (side<0)
        order=("hind-far","fore-far","body","hind-near","fore-near","head") if folded else (
            "hind-near","fore-near","body","hind-far","fore-far","head")
        for part in order:
            box,pivot=parts[part]
            target=mount(pivot)
            if part=="body":
                scale=torso_scale
                turn=math.degrees(roll)
            elif part=="head":
                neck=(7.5,13.4) if view=="front" else (-3.5,6.8)
                target=(target[0]+neck[0]*depth,target[1]+neck[1])
                scale=(depth*1.02,.86 if side>0 else .90)
                turn=angle+(58 if view=="front" else 38)+side*7
            else:
                fore=part.startswith("fore")
                near=part.endswith("near")
                upper=(near==folded)
                hoof=fit(*HOOF[view][part])
                foot_y=(115.5 if upper else 120)+(0.8 if fore else 0)
                direction=1 if fore else -1
                stride=direction*(23 if upper else 12)*depth
                knee_target=(target[0]+(stride*.56-direction*5),target[1]+(2.5 if upper else 5.2))
                hoof_target=(target[0]+stride,foot_y)
                lower_box,knee=parts[part+"-hoof"]

                def shape(a,b,c,d):
                    vx,vy=minus(b,a)
                    ax,ay=minus(d,c)
                    turn=math.degrees(math.atan2(ay,ax)-math.atan2(vy,vx))
                    length=math.hypot(ax,ay)/math.hypot(vx,vy)
                    return (length,length),turn

                shin_scale,shin_turn=shape(knee,hoof,knee_target,hoof_target)
                static_pose(pose,f"{view}-{part}-hoof",lower_box,knee,fit,shin_scale,shin_turn,knee_target)
                scale,turn=shape(pivot,knee,target,knee_target)
            static_pose(pose,f"{view}-{part}",box,pivot,fit,scale,turn,target)
        if mirror:
            layer(pose,"stretch","Odbicie kierunku gry",amount=("vector",(-1,1)),center=("vector",units(ANCHOR)))
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path,encoding="utf-8",xml_declaration=True)
    return contacts


def digest(file):
    return hashlib.sha256(file.read_bytes()).hexdigest()


def trim_detached_cut_edges(body):
    alpha=body.getchannel("A")
    width,height=body.size
    remaining={i for i,a in enumerate(alpha.tobytes()) if a>=48}
    largest=[]
    while remaining:
        first=remaining.pop()
        group=[first]
        queue=deque(group)
        while queue:
            i=queue.popleft()
            x,y=i%width,i//width
            for index in ((i-1 if x>0 else -1),(i+1 if x<width-1 else -1),
                          (i-width if y>0 else -1),(i+width if y<height-1 else -1)):
                if index in remaining:
                    remaining.remove(index)
                    queue.append(index)
                    group.append(index)
        if len(group)>len(largest):
            largest=group
    # Preserve antialiasing around connected anatomy, excluding visible
    # slivers left over from cuts in the original painted leg or horn.
    mask=bytearray(width*height)
    for i in largest:
        mask[i]=255
    protected=Image.frombytes("L",body.size,bytes(mask)).filter(ImageFilter.MaxFilter(5))
    body.putalpha(ImageChops.multiply(alpha,protected))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source",type=Path,required=True)
    parser.add_argument("--animation",type=Path,required=True)
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    args.animation.mkdir(parents=True,exist_ok=False)
    args.run.mkdir(parents=True,exist_ok=False)
    cuts=load_cuts(args.source,args.animation)
    atlas=Image.new("RGBA",(len(VIEWS)*STRIDE,2*STRIDE))
    preview=Image.new("RGBA",atlas.size,(23,30,34,255))
    draw=ImageDraw.Draw(preview)
    runs=[]
    atlas_start=time.perf_counter()
    for row,side in enumerate((1,-1)):
        project=args.animation/f"rogacz-bok-{side:+d}.sif"
        contacts=author(project,cuts,side)
        receipt=synfig_cli.render(project,output_root=args.animation/"renders",threads=2,
                                  expected_frames=len(VIEWS),require_motion=True)
        bounds=[]
        for i,path in enumerate(sorted(Path(receipt["OutputDirectory"]).glob("frame*.png"))):
            body=Image.open(path).convert("RGBA").resize((192,192),Image.Resampling.LANCZOS)
            trim_detached_cut_edges(body)
            alpha=body.getchannel("A")
            box=alpha.point(lambda a:255 if a>=80 else 0).getbbox()
            if not box or min(box[:2])<3 or max(box[2:])>189 or box[2]-box[0]<42 or box[3]-box[1]<30:
                raise ValueError(f"Clipped or too small {project.name} frame {i}: {box}")
            # Preserve the authored ribcage-height anchor. A hoof tip can sit
            # in front of it in the isometric view without lifting the torso.
            body_new=Image.new("RGBA",(SIZE,SIZE))
            body_new.paste(body,((SIZE-body.width)//2,(SIZE-body.height)//2))
            box=body_new.getchannel("A").point(lambda a:255 if a>=80 else 0).getbbox()
            if min(box[:2])<3 or max(box[2:])>SIZE-3:
                raise ValueError(f"Grounded cell leaks outside its padding: {project.name} {i}: {box}")
            offset=(i*STRIDE+PAD,row*STRIDE+PAD)
            atlas.paste(body_new,offset)
            preview.alpha_composite(body_new,offset)
            draw.text((offset[0]+5,offset[1]+6),f"{side:+d} / {i+1}",fill=(210,190,141,255))
            bounds.append(box)
        receipt["OpaqueBounds"]=bounds
        receipt["TorsoContact"]=contacts
        runs.append(receipt)
    output=args.run/"cinderhorn-rest.png"
    atlas.save(output)
    preview.save(args.run/"cinderhorn-rest-preview.png")
    receipt={"status":"PASS","output":str(output),"sha256":digest(output),"size":atlas.size,
             "anchor":ANCHOR,"views":len(VIEWS),"render_export_seconds":time.perf_counter()-atlas_start,
             "source":[{"path":str(args.source/f"{v}.kra"),"sha256":digest(args.source/f"{v}.kra")} for v in cuts],
             "synfig":[{k:r[k] for k in ("Project","ProjectSha256","ActualFrames","UniquePixelFrames","ElapsedSeconds","OpaqueBounds","TorsoContact")}
                        for r in runs]}
    (args.run/"report.json").write_text(json.dumps(receipt,indent=2,ensure_ascii=False),encoding="utf-8")


if __name__=="__main__":
    main()

"""Finish local ComfyUI creature art in Krita and bake native Synfig clips."""
from __future__ import annotations

import argparse
import hashlib
import io
import json
import math
from pathlib import Path
import shutil
from xml.etree import ElementTree as ET
from zipfile import ZipFile

import numpy as np
from PIL import Image, ImageChops, ImageDraw, ImageFilter

from hero_synfig import group, image, rotate, value
from krita_mcp_client import KritaMCP, start_krita_if_needed
import synfig_cli


NAMES = ("sporekin", "dune-eye", "cinderhorn")
CLIPS = {"idle": 8, "move": 12, "attack": 12, "hit": 6, "fall": 8}
SIZE, PAD, ANCHOR = 144, 2, (72, 121)
STRIDE = SIZE + PAD * 2
ROOT = Path(__file__).resolve().parents[2]
COMFY = Path.home() / "AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI"

# Rogacz is traced around the painted body to remove the concept render's base.
HORN_MASK = [
    (38,195),(49,174),(70,164),(91,166),(112,150),(119,121),(134,111),
    (149,95),(160,91),(166,77),(162,59),(169,46),(184,41),(179,63),
    (188,73),(194,37),(204,31),(211,44),(216,68),(223,61),(216,47),
    (226,48),(239,57),(244,72),(237,85),(248,94),(271,92),(289,102),
    (301,124),(318,143),(332,165),(342,186),(342,216),(332,232),
    (315,233),(302,227),(296,207),(292,194),(282,195),(283,214),
    (297,230),(304,254),(311,276),(309,289),(299,296),(302,307),
    (285,312),(263,311),(254,301),(258,288),(246,283),(231,284),
    (217,278),(216,287),(191,288),(181,280),(177,281),(153,289),
    (153,304),(145,313),(122,314),(112,308),(112,296),(98,296),
    (81,285),(68,271),(66,254),(72,239),(87,229),(103,227),
    (111,213),(108,198),(97,204),(82,222),(62,225),(48,216),
]


def polygon_mask(size, points):
    mask = Image.new("L", size)
    ImageDraw.Draw(mask).polygon(points, fill=255)
    return mask


def masked(source, mask):
    out = source.copy()
    out.putalpha(ImageChops.multiply(source.getchannel("A"), mask))
    return out


def write_ora(path, sprite):
    canvas = ET.Element("image", w=str(sprite.width), h=str(sprite.height), version="0.0.3")
    stack = ET.SubElement(canvas, "stack")
    ET.SubElement(stack, "layer", name="ComfyUI · farba i materiały", src="data/body.png",
                  x="0", y="0", opacity="1.0", visibility="visible", **{"composite-op": "svg:src-over"})
    with io.BytesIO() as buffer, ZipFile(path, "x") as archive:
        sprite.save(buffer, format="PNG")
        archive.writestr("mimetype", "image/openraster")
        archive.writestr("stack.xml", ET.tostring(canvas, encoding="utf-8"))
        archive.writestr("data/body.png", buffer.getvalue())
        archive.writestr("mergedimage.png", buffer.getvalue())


def extract(name, raw):
    pixels = np.asarray(raw).astype(float)
    if name=="cinderhorn":
        limit=polygon_mask(raw.size,[(x*1.5,y*1.5) for x,y in HORN_MASK])
        r,g,b=pixels[:,:,:3].transpose(2,0,1)
        # The concept's background is orange. Preserve the obsidian and scarf.
        chroma=np.clip((47-(r-g))/28,0,1)
        chroma=np.maximum(chroma,np.clip((b-r+24)/32,0,1))
        mask=Image.fromarray((np.asarray(limit)*chroma).astype("uint8")).filter(ImageFilter.GaussianBlur(.75))
    else:
        corners=np.concatenate([pixels[:42,:42,:3].reshape(-1,3),pixels[:42,-42:,:3].reshape(-1,3)])
        backdrop=np.median(corners,axis=0)
        distance=np.sqrt(np.sum((pixels[:,:,:3]-backdrop)**2,axis=2))
        r,g,b=pixels[:,:,:3].transpose(2,0,1)
        color=np.maximum(r-b,g-r+10)
        confidence=np.clip((distance-16)/22,0,1)*np.clip((color+8)/30,0,1)
        y,x=np.mgrid[:raw.height,:raw.width]
        limit=(x>60)&(x<519)&(y>62)&(y<512)
        mask=Image.fromarray((255*confidence*limit).astype("uint8")).filter(ImageFilter.GaussianBlur(.6))
    out=raw.copy()
    out.putalpha(mask)
    return out


def paint_native(run, work):
    work.mkdir(exist_ok=False)
    output=COMFY/"output"/run.name
    candidates=("sporekin-painted","dune-eye-painted","cinderhorn-2")
    start_krita_if_needed()
    with KritaMCP(work/"krita-logs") as krita:
        for name,candidate in zip(NAMES,candidates):
            source,=sorted(output.glob(candidate+"_*.png"))
            shutil.copyfile(source,work/f"{name}-comfy.png")
            sprite=extract(name,Image.open(source).convert("RGBA"))
            ora=work/f"{name}.ora"
            write_ora(ora,sprite)
            document=krita.call("krita_document",action="open",path=str(ora))["affected_document_id"]
            state=krita.call("krita_state",document_id=document)
            if state["runtime_compatibility"]["status"]!="compatible":
                raise RuntimeError(state["runtime_compatibility"])
            layer=krita.call("krita_layers",action="create",document_id=document,
                             name="Krita · refleksy i ślady żywiołu")["active_layer_id"]
            marks={
                "sporekin":[("#ffe3a3",2.4,[(239,304),(244,300),(248,301)]),
                            ("#fff1b7",2.4,[(302,314),(306,311),(310,312)]),
                            ("#edaa74",2.6,[(209,180),(221,172),(232,174)]),
                            ("#be8f66",2.0,[(213,363),(211,374),(216,391)])],
                "dune-eye":[("#dcfff0",2.5,[(288,213),(282,227),(276,240)]),
                            ("#8ff4dd",2.0,[(316,243),(333,251),(342,260)]),
                            ("#ffe2a8",2.5,[(276,131),(285,119),(295,125)]),
                            ("#eed69b",1.9,[(382,206),(391,195),(404,199)])],
                "cinderhorn":[("#ff8654",3.5,[(301,188),(291,200),(300,209),(292,218)]),
                              ("#ffb067",2.4,[(328,233),(343,228),(357,236)]),
                              ("#ed633c",3.1,[(215,255),(224,271),(222,283),(234,300)]),
                              ("#ff9960",2.6,[(410,376),(400,387),(404,397)]),
                              ("#fc6947",2.4,[(148,392),(158,404),(155,416)])],
            }[name]
            for color,width,points in marks:
                valid=[{"x":x,"y":y} for x,y in points if sprite.getpixel((x,y))[3]>130]
                if len(valid)<2:
                    continue
                krita.call("krita_draw",shape="path",document_id=document,layer_id=layer,
                           points=valid,foreground=color,fill_style="None",
                           stroke_style="ForegroundColor",brush_size=width)
            krita.call("krita_preview",document_id=document,max_width=576,max_height=576)
            krita.call("krita_save_export",document_id=document,kra_path=str(work/f"{name}.kra"),
                       png_path=str(work/f"{name}.png"),overwrite=False)
            krita.call("krita_document",action="close",document_id=document,discard_changes=False)
            print("Krita saved",name,flush=True)


def split_parts(name,sprite):
    size=sprite.size
    # All part masks are in coordinates of the actual 576 px native painting.
    if name=="sporekin":
        parts=[
            ("left-leg",[(184,402),(268,410),(273,500),(169,500)],(236,418)),
            ("right-leg",[(278,410),(362,407),(409,510),(282,510)],(321,423)),
            ("left-arm",[(66,294),(194,282),(207,340),(187,390),(65,398)],(181,296)),
            ("right-arm",[(365,294),(446,312),(483,409),(363,408),(357,343)],(370,302)),
            ("cap",[(79,64),(489,66),(499,260),(387,289),(249,291),(79,278)],(284,270)),
        ]
    elif name=="cinderhorn":
        parts=[
            ("left-leg",[(95,337),(248,340),(255,406),(230,478),(81,478)],(198,357)),
            ("right-leg",[(336,336),(454,335),(476,471),(358,478),(331,423)],(367,355)),
            ("left-arm",[(50,236),(141,225),(172,205),(207,192),(193,268),(164,297),(118,345),(47,345)],(173,230)),
            ("right-arm",[(425,198),(490,201),(526,355),(438,362),(432,286),(412,255)],(439,222)),
            ("head",[(226,38),(371,39),(383,147),(347,196),(269,188),(225,146)],(306,153)),
        ]
    else:
        # Each stone and the pendant have their own hinge and orbit around the eye.
        pivots=[(288,145),(162,221),(414,220),(414,337),(288,404),(162,336),(288,479)]
        center=(288,269)
        y,x=np.mgrid[:size[1],:size[0]]
        samples=[center]+pivots
        distances=np.stack([(x-cx)**2+(y-cy)**2 for cx,cy in samples]).astype(float)
        # The horizontal iris overlaps the nearest-stone Voronoi boundaries.
        distances[0]*=.61
        nearest=np.argmin(distances,axis=0)
        return [("eye",masked(sprite,Image.fromarray((nearest==0).astype("uint8")*255)),center)]+[
            (f"stone-{i}",masked(sprite,Image.fromarray((nearest==i+1).astype("uint8")*255)),p)
            for i,p in enumerate(pivots)
        ]
    combined=Image.new("L",size)
    result=[]
    for label,points,origin in parts:
        mask=polygon_mask(size,points)
        # A few pixels of shared art behind each hinge prevent cracks in motion.
        result.append((label,masked(sprite,mask.filter(ImageFilter.MaxFilter(7))),origin))
        combined=ImageChops.lighter(combined,mask)
    torso=masked(sprite,ImageChops.invert(combined.filter(ImageFilter.MinFilter(15))))
    return result[:2]+[("body",torso,(288,350))]+result[2:]


def animated_layer(canvas,kind,parameter,value_type,values):
    layer=ET.SubElement(canvas,"layer",type=kind,active="true",version="0.1")
    animated=ET.SubElement(ET.SubElement(layer,"param",name=parameter),"animated",type=value_type)
    for frame,val in enumerate(values):
        waypoint=ET.SubElement(animated,"waypoint",time=f"{frame}f",before="linear",after="linear")
        node=ET.SubElement(waypoint,value_type)
        if value_type=="vector":
            ET.SubElement(node,"x").text=f"{val[0]:.6f}"
            ET.SubElement(node,"y").text=f"{val[1]:.6f}"
        else:
            node.set("value",f"{val:.6f}")
    return layer


def pulse(t,peak):
    return math.sin(math.pi*min(1,t/peak)*.5)**2 if t<=peak else math.cos(math.pi*(t-peak)/(1-peak)*.5)**2


def motion(name,part,clip,t):
    w=math.tau*t
    left=part.startswith("left")
    sign=-1 if left else 1
    limb="leg" in part or "arm" in part
    stone=part.startswith("stone")
    if clip=="idle":
        return (0,1.4*math.sin(w),math.sin(w)*(2 if limb or stone else 1))
    if clip=="move":
        gait=math.sin(w+sign*math.pi/2)
        if stone:
            a=int(part[-1])
            return (2*math.cos(w+a),3*math.sin(w+a),5*math.sin(w+a))
        return (gait*(3.8 if limb else .5),max(0,gait)*4 if "leg" in part else 2*math.cos(2*w),
                gait*(10 if limb else 3))
    if clip=="attack":
        charge=pulse(min(t/.66,1),.68) if t<.66 else 0
        impact=pulse(t,.66)
        thrust=max(0,impact-charge*.8)
        if name=="dune-eye":
            angle=int(part[-1])*math.tau/6 if stone else 0
            return ((math.cos(angle)*11 if stone else 11)*thrust-3*charge,
                    (math.sin(angle)*8 if stone else 4)*thrust+charge*4,
                    (15 if stone else -9)*thrust-6*charge)
        drive=10 if name=="cinderhorn" else 8
        side=-1 if name=="sporekin" else 1
        return (side*((drive+(3 if "arm" in part else 0))*thrust-4*charge),
                (8 if name=="sporekin" else -3)*thrust-5*charge,
                side*((18 if "arm" in part else 12)*thrust-(10 if limb else 5)*charge))
    if clip=="hit":
        decay=1-t
        return (-7*decay,3*decay,(-12 if not left else 10)*decay)
    fall=t*t*(3-2*t)
    if name=="dune-eye":
        angle=int(part[-1])*math.tau/6 if stone else 0
        return (13*math.cos(angle)*fall,(5+6*math.sin(angle))*fall,50*math.cos(angle)*fall)
    return (3*sign*fall,0,8*sign*fall if limb else 0)


def author_clip(path,name,clip,parts,fit):
    count=CLIPS[clip]
    canvas=ET.Element("canvas",version="1.2",width=str(SIZE),height=str(SIZE),
                      xres="2834.645669",yres="2834.645669",fps="24",
                      **{"view-box":"-2 2 2 -2","antialias":"2","begin-time":"0f",
                         "end-time":f"{count-1}f","bgcolor":"0 0 0 0"})
    ET.SubElement(canvas,"name").text=f"Emberfall / {name} / {clip}"
    rig=group(canvas,"Warstwy Krity / animacja wycinankowa")
    loop=clip in ("idle","move")
    times=[f/(count if loop else count-1) for f in range(count+int(loop))]
    def point(x,y):
        px,py=fit(x,y)
        return ((px-72)/36,(72-py)/36)
    for part,box,origin in parts:
        drawing=group(rig,part)
        tl=point(box[0],box[1]);br=point(box[2],box[3])
        image(drawing,part,(*tl,*br))
        samples=[motion(name,part,clip,t) for t in times]
        rotate(drawing,part,point(*origin),[a for _,_,a in samples])
        animated_layer(drawing,"translate","origin","vector",[(x/36,y/36) for x,y,_ in samples])
    if clip=="fall":
        end=times
        # Scale the fallen stack vertically around its ground contact while it tilts.
        angles=[(18 if name=="dune-eye" else 36)*pulse(t,.999) for t in end]
        rotate(rig,"fall-tilt",point(288,459),angles)
        transform=animated_layer(rig,"stretch","amount","vector",[(1,1-(.63 if name=="dune-eye" else .68)*t*t) for t in end])
        value(transform,"center","vector",((ANCHOR[0]-72)/36,(72-ANCHOR[1])/36))
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path,encoding="utf-8",xml_declaration=True)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def bake(run,work,animation):
    animation.mkdir(exist_ok=False)
    delivery=run/"visual-delivery"
    delivery.mkdir(exist_ok=False)
    receipts=[]
    contact=Image.new("RGBA",(SIZE*len(CLIPS),SIZE*len(NAMES)),(29,34,47,255))
    for row,name in enumerate(NAMES):
        native=work/f"{name}.kra"
        with ZipFile(native) as archive:
            assert archive.testzip() is None and b"Krita" in archive.read("maindoc.xml"),native
        sprite=Image.open(work/f"{name}.png").convert("RGBA")
        silhouette=sprite.getchannel("A").point(lambda a:255 if a>=55 else 0)
        x0,y0,x1,y1=silhouette.getbbox()
        width,height=x1-x0,y1-y0
        max_height=(73,82,91)[row]
        max_width=(94,91,88)[row]
        scale=min(max_width/width,max_height/height)
        lift=14 if name=="dune-eye" else 0
        def fit(x,y):
            return ANCHOR[0]+(x-(x0+x1)/2)*scale,ANCHOR[1]-lift+(y-y1)*scale
        folder=animation/name
        (folder/"parts").mkdir(parents=True)
        parts=[]
        for part,layer,pivot in split_parts(name,sprite):
            bounds=layer.getbbox()
            if not bounds:
                continue
            layer.crop(bounds).save(folder/"parts"/f"{part}.png")
            parts.append((part,bounds,pivot))
        atlas=Image.new("RGBA",(STRIDE*8,STRIDE*6))
        offset=0
        clips=[]
        for col,(clip,count) in enumerate(CLIPS.items()):
            source=folder/f"{clip}.sif"
            author_clip(source,name,clip,parts,fit)
            rendered=synfig_cli.render(source,output_root=folder/"renders",threads=2,
                                       expected_frames=count,require_motion=True)
            frame_paths=sorted(Path(rendered["OutputDirectory"]).glob("frame*.png"))
            assert len(frame_paths)==count,rendered.keys()
            for frame,file in enumerate(frame_paths):
                rgba=Image.open(file).convert("RGBA")
                box=rgba.getchannel("A").point(lambda a:255 if a>=45 else 0).getbbox()
                assert box and box[0]>0 and box[1]>0 and box[2]<SIZE and box[3]<SIZE,(name,clip,frame,box)
                n=offset+frame
                atlas.paste(rgba,((n%8)*STRIDE+PAD,(n//8)*STRIDE+PAD))
                if frame==round((count-1)*(.66 if clip=="attack" else 1 if clip=="fall" else .35)):
                    contact.alpha_composite(rgba,(col*SIZE,row*SIZE))
            clips.append({"name":clip,"start":offset,"frames":count,"source":str(source),"source_sha256":digest(source)})
            offset+=count
            print(name,clip,count,"native Synfig frames",flush=True)
        destination=delivery/f"{name}.png"
        atlas.save(destination)
        receipts.append({"name":name,"master":str(native),"master_sha256":digest(native),
                         "comfy_sha256":digest(work/f"{name}-comfy.png"),"atlas_sha256":digest(destination),
                         "parts":len(parts),"clips":clips,"atlas_rgba8_bytes":atlas.width*atlas.height*4})
    contact.convert("RGB").save(run/"animation-preview.png")
    (run/"visual-receipt.json").write_text(json.dumps(receipts,indent=2)+"\n",encoding="utf-8")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action",choices=["paint","bake"])
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    master=Path("C:/AI2D/sprites")/args.run.name
    if args.action=="paint":
        paint_native(args.run,master)
    else:
        bake(args.run,master,Path("C:/AI2D/animations")/args.run.name)

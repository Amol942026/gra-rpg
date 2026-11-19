"""Register the same six anatomical pieces around a closed turn using local FILM."""
import argparse
import json
from pathlib import Path
import time
from urllib.parse import urlencode

import numpy as np
from PIL import Image
import requests

from caldera_corpse_paint import cell_paint
from ember_drake_synfig import NAMES
from ember_drake_turn import Paint, UV, inbetween, CELL
from obsidian_sentinel_paint import PARTS

ORDER=((0,1,2,4,3,5),(0,2,1,3,4,5),(0,1,2,3,5,4),(0,1,2,3,5,4),
       (0,1,5,3,2,4),(0,2,5,1,3,4),(0,1,2,3,4,5),(0,1,2,4,3,5))


def prepare(root):
    keys=[[Paint(Image.open(root/"cells"/f"{d}-{p}.png")) for p in PARTS] for d in NAMES]
    frames=[]
    for pieces in keys+keys[:1]:
        image=Image.new("RGB",(3*UV,2*UV))
        for p,part in enumerate(pieces):
            swatch=Image.fromarray(np.uint8(np.clip(np.rint(part.texture),0,255)))
            image.paste(swatch,(p%3*UV,p//3*UV))
        frames.append(image)
    source=root/"sentinel-materials.png"
    frames[0].save(source,save_all=True,append_images=frames[1:],duration=100,loop=0,format="PNG")
    return keys,source


def film(root,host,source,asset_id="obsidian-sentinel"):
    key=f"emberfall-{asset_id}-{root.name}"
    with source.open("rb") as file:
        sent=requests.post(host+"/upload/image",files={"image":(source.name,file,"image/png")},data={"subfolder":key,"overwrite":"true"},timeout=40)
    sent.raise_for_status()
    workflow={
        "1":{"class_type":"LoadImage","inputs":{"image":key+"/"+sent.json()["name"]}},
        "2":{"class_type":"FrameInterpolationModelLoader","inputs":{"model_name":"film_net_fp16.safetensors"}},
        "3":{"class_type":"FrameInterpolate","inputs":{"images":["1",0],"interp_model":["2",0],"multiplier":4}},
        "4":{"class_type":"SaveImage","inputs":{"images":["3",0],"filename_prefix":key+"/turn"}},
    }
    (root/"film-workflow.json").write_text(json.dumps(workflow,indent=2))
    posted=requests.post(host+"/prompt",json={"prompt":workflow},timeout=30)
    posted.raise_for_status();ident=posted.json()["prompt_id"]
    print("FILM",ident,flush=True)
    start=time.monotonic()
    while time.monotonic()-start<1500:
        row=requests.get(host+"/history/"+ident,timeout=20).json().get(ident)
        if row:break
        time.sleep(2)
    if not row or "4" not in row["outputs"]:raise RuntimeError(str(row))
    frames=row["outputs"]["4"]["images"]
    if len(frames)!=33:raise ValueError(len(frames))
    out=root/"film";out.mkdir(exist_ok=True)
    for index,frame in enumerate(frames):
        response=requests.get(host+"/view?"+urlencode(frame),timeout=25)
        response.raise_for_status();(out/f"{index:02}.png").write_bytes(response.content)
    (root/"film.json").write_text(json.dumps({"frames":len(frames),"id":ident,"execution":row.get("status"),"elapsed_including_queue_and_download":round(time.monotonic()-start,3)},indent=2))


def pack(root,views,canonical,header):
    entries=[]
    for d,row in enumerate(views):
        for p,piece in enumerate(row):
            box=piece.getchannel("A").getbbox()
            if not box:raise ValueError((d,p))
            entries.append((d,p,box,piece.crop(box)))
    page_width=1536;shelves=[];height=0;rects={}
    for d,p,box,crop in sorted(entries,key=lambda e:-e[3].height):
        w,h=crop.width+4,crop.height+4
        valid=[row for row in shelves if row[1]>=h and row[2]+w<=page_width]
        if valid:shelf=min(valid,key=lambda row:row[1])
        else:shelf=[height,h,0];shelves.append(shelf);height+=h
        rects[d,p]=(shelf[2]+2,shelf[0]+2);shelf[2]+=w
    if height>2048:raise ValueError(height)
    canvas=Image.new("RGBA",(page_width,height))
    result=[[] for _ in range(32)]
    for d,p,box,crop in entries:
        x,y=rects[d,p];canvas.paste(crop,(x,y))
        result[d].append((0,x,y,crop.width,crop.height,box[0],box[1]))
    folder=canonical/"sentinel-turn";folder.mkdir(exist_ok=True)
    canvas.save(folder/"parts-0.png",optimize=True)
    lines=["#pragma once",'#include "../volcano_turn_tile.hpp"',"namespace obsidian_sentinel_turn_atlas {",
           "using Tile=volcano_turn_data::Tile;","inline constexpr int Count=32, Parts=6, Cell=160, PageCount=1;",
           "inline constexpr int authoredPart(int,int part){return part;}","inline constexpr Tile tiles[Count][Parts]={"]
    lines+=[" {"+",".join("{"+",".join(map(str,item))+"}" for item in row)+"}," for row in result]
    lines+=["};","}"]
    header.write_text("\n".join(lines)+"\n")
    live=Image.new("RGBA",(8*CELL,6*CELL));cooled=Image.new("RGBA",(8*128,6*128))
    for d,parts in enumerate(views[::4]):
        for p,part in enumerate(parts):
            live.paste(part,(d*CELL,p*CELL))
            _,material=cell_paint(part,4,p,d)
            cooled.paste(material,(d*128,p*128))
    live.save(canonical/"obsidian-sentinel.png",optimize=True)
    cooled.save(canonical/"corpses"/"obsidian-sentinel.png",optimize=True)
    from caldera_turn_registration import register
    register(folder/"parts-0.png",header,folder)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root",type=Path);parser.add_argument("--comfy")
    parser.add_argument("--pack",type=Path);parser.add_argument("--header",type=Path)
    parser.add_argument("--edited",type=Path)
    args=parser.parse_args();root=args.root
    keys,source=prepare(root)
    if args.comfy:film(root,args.comfy,source)
    if args.pack:
        views=[]
        out=root/"turn";out.mkdir(exist_ok=True)
        for v in range(32):
            k,sub=divmod(v,4);time=sub/4
            f=Image.open(root/"film"/f"{v:02}.png").convert("RGB")
            row=[]
            for p,a in enumerate(keys[k]):
                if args.edited:
                    im=Image.open(args.edited/f"{v:02}-{p}.png").convert("RGBA")
                elif not sub:im=a.image
                else:
                    x,y=p%3*UV,p//3*UV
                    pigment=np.array(f.crop((x,y,x+UV,y+UV)),dtype=np.float32)
                    im=inbetween(a,keys[(k+1)%8][p],time,pigment)
                row.append(im);im.save(out/f"{v:02}-{p}.png")
            views.append(row)
        pack(root,views,args.pack,args.header)

if __name__=="__main__":main()

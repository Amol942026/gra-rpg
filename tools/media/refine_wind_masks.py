"""Refine the two painted wind masks while preserving the desert texture."""
import argparse
import hashlib
import json
import math
from pathlib import Path
from random import Random

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

from krita_mcp_client import KritaMCP, start_krita_if_needed


SIZE = (640, 320)
SOURCE = Path("C:/AI2D/sprites/region-wind-20261111")


def stamp(target, shape, x, y):
    height,width=target.shape
    ys=(np.arange(shape.shape[0])+y)%height
    xs=(np.arange(shape.shape[1])+x)%width
    pixels=np.ix_(ys,xs)
    target[pixels]=np.maximum(target[pixels],shape)


def forest_mask():
    rng=Random(3161)
    width,height=SIZE
    previous=np.asarray(Image.open(SOURCE/"forest-comfy.png").convert("L"),dtype=float)
    result=np.zeros((height,width),dtype=np.uint8)
    for i in range(20):
        length=rng.randrange(74,214)
        local=Image.new("L",(length,28))
        line=ImageDraw.Draw(local)
        variation=previous[74+i%7:102+i%7,210:210+length].mean(axis=0)
        for x in range(length):
            t=x/(length-1)
            taper=math.sin(t*math.pi)**.9
            bend=9+4*math.sin(t*math.pi*1.2+i)+t*5
            alpha=int(min(212,132+variation[x]*3)*taper)
            line.line((x,bend-taper*1.6,x,bend+taper*1.6),fill=alpha,width=1)
        streak=np.asarray(local.filter(ImageFilter.GaussianBlur(.65)))
        stamp(result,streak,rng.randrange(width),rng.randrange(height))
    for i in range(94):
        leaf=Image.new("L",(9,7))
        draw=ImageDraw.Draw(leaf)
        light=rng.randrange(120,239)
        draw.polygon([(0,5),(2,2),(7,1),(8,2),(6,4),(2,5)],fill=light)
        draw.line((1,5,6,2),fill=min(light+25,255))
        leaf=leaf.resize((rng.randrange(4,9),rng.randrange(2,5)),Image.Resampling.LANCZOS)
        stamp(result,np.asarray(leaf),rng.randrange(width),rng.randrange(height))
    return result


def ash_mask():
    rng=Random(8104)
    width,height=SIZE
    source=Image.open(SOURCE/"caldera-comfy.png").convert("L")
    # Sample isolated flecks from the original generated source. Broad haze
    # and streaks are excluded when building the particle field.
    boxes=[(24,2,42,21),(39,136,64,160),(86,95,108,123),
           (120,63,142,80),(194,21,219,39),(438,46,469,68),
           (34,266,73,304),(564,195,591,221)]
    flakes=[]
    for box in boxes:
        gray=np.asarray(source.crop(box),dtype=float)
        gray=np.maximum(gray-float(np.quantile(gray,.55)),0)
        if gray.max()>0:
            flakes.append(Image.fromarray(np.rint(gray/gray.max()*255).astype(np.uint8)))
    if not flakes:
        raise RuntimeError("The ash source contains no usable flecks.")
    result=np.zeros((height,width),dtype=np.uint8)
    for i in range(1160):
        flake=rng.choice(flakes)
        w=rng.choices((2,3,4,5),(10,33,42,15))[0]
        flake=flake.resize((w,rng.choice((2,2,3,3))),Image.Resampling.LANCZOS)
        alpha=np.asarray(flake,dtype=float)*rng.uniform(.82,1.30)
        stamp(result,np.rint(np.clip(alpha,0,255)).astype(np.uint8),rng.randrange(width),rng.randrange(height))
    return result


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def make(run):
    masters=Path("C:/AI2D/sprites")/run.name
    masters.mkdir()
    delivery=run/"delivery"
    delivery.mkdir()
    receipt={}
    start_krita_if_needed()
    with KritaMCP(masters/"logs") as client:
        for i,(name,alpha) in enumerate((("forest",forest_mask()),("caldera",ash_mask()))):
            seed=Image.new("RGBA",SIZE,(255,255,255,0))
            seed.putalpha(Image.fromarray(alpha))
            staged=masters/f"{name}-base.png"
            seed.save(staged)
            doc=client.call("krita_document",action="open",path=str(staged))["affected_document_id"]
            state=client.call("krita_state",document_id=doc)
            if state["runtime_compatibility"]["status"]!="compatible":
                raise RuntimeError(state["runtime_compatibility"])
            layer=client.call("krita_layers",action="create",document_id=doc,
                              name="Krita · refleksy liści i drobinki")["active_layer_id"]
            rng=Random(7111+i)
            for _ in range(24):
                x,y=rng.randrange(10,623),rng.randrange(10,309)
                length=rng.randrange(2,5) if i==0 else rng.randrange(1,3)
                client.call("krita_draw",shape="path",document_id=doc,layer_id=layer,foreground="#ffffff",
                            points=[{"x":x,"y":y},{"x":x+length,"y":y+.6}],
                            fill_style="None",stroke_style="ForegroundColor",brush_size=.9 if i==0 else .8)
            client.call("krita_preview",document_id=doc,max_width=640,max_height=320)
            kra=masters/f"{name}.kra"
            png=delivery/f"{name}.png"
            client.call("krita_save_export",document_id=doc,kra_path=str(kra),png_path=str(masters/png.name),overwrite=False)
            client.call("krita_document",action="close",document_id=doc,discard_changes=False)
            seed.putalpha(Image.open(masters/png.name).convert("RGBA").getchannel("A"))
            seed.save(png)
            original=SOURCE/f"{name}-comfy.png"
            receipt[name]={"original_comfy":str(original),"original_sha256":digest(original),
                           "kra":str(kra),"kra_sha256":digest(kra),"png_sha256":digest(png)}
            print(name,"Krita master and wind mask saved",flush=True)
    (run/"art-receipt.json").write_text(json.dumps(receipt,indent=2)+"\n",encoding="utf-8")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    make(parser.parse_args().run)

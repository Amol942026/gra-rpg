"""Generate wind masks in local ComfyUI and finish their alpha in Krita."""
import argparse
import hashlib
import json
import math
from pathlib import Path
from random import Random
import shutil
from urllib.request import urlopen

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

from krita_mcp_client import KritaMCP, start_krita_if_needed


COMFY = Path.home() / "AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI"
MODEL = "DreamShaper_8_pruned.safetensors"
SIZE = (640, 320)
REGIONS = {
    "forest": ("fine airy mist threads, sparse fern pollen, wispy filaments with clean spaces between", 7, 48),
    "desert": ("pale wind-blown sand, thin horizontal flowing dust trails and tiny granular flecks", 12, 126),
    "caldera": ("charcoal ash and pale gray drifting volcanic dust, narrow layered billows and scattered irregular ash flakes", 10, 155),
}


def guide(count, specks, index):
    width,height=SIZE
    canvas=Image.new("L",SIZE)
    rng=Random(811110+index)
    brush=ImageDraw.Draw(canvas)
    for _ in range(count):
        x,y=rng.randrange(-60,width-50),rng.randrange(10,height-10)
        length=rng.randrange(65,270)
        bend=rng.uniform(2,12)
        points=[(x+length*u/20,y+bend*math.sin(u/20*math.tau)+length*u/20*.065) for u in range(21)]
        brush.line(points,fill=rng.randrange(46,116),width=rng.randrange(2,10))
    canvas=canvas.filter(ImageFilter.GaussianBlur(3.5))
    brush=ImageDraw.Draw(canvas)
    for _ in range(specks):
        x,y=rng.randrange(6,width-5),rng.randrange(6,height-5)
        brush.line((x,y,x+rng.randrange(1,7),y+1),fill=rng.randrange(18,102),width=rng.choice((1,1,2)))
    return canvas.convert("RGB")


def prepare(run):
    stage=COMFY/"input"/run.name
    stage.mkdir(exist_ok=False)
    graph={"1":{"class_type":"CheckpointLoaderSimple","inputs":{"ckpt_name":MODEL}}}
    def node(key,kind,**values):
        graph[str(key)]={"class_type":kind,"inputs":values}
        return [str(key),0]
    outputs={}
    for index,(name,(subject,count,specks)) in enumerate(REGIONS.items()):
        n=10+index*10
        guide(count,specks,index).save(stage/f"{name}.png")
        image=node(n,"LoadImage",image=f"{run.name}/{name}.png")
        positive=node(n+1,"CLIPTextEncode",clip=["1",1],
                      text="isolated VFX grayscale soft opacity mask for fantasy game wind overlay, "+subject+
                      ", wisps stretched from left to right, detailed feathered edges, very dark pure black background, closeup texture only")
        negative=node(n+2,"CLIPTextEncode",clip=["1",1],
                      text="landscape, horizon, sky, mountains, trees, people, text, border, thick opaque cloud, fire, colored background")
        latent=node(n+3,"VAEEncode",pixels=image,vae=["1",2])
        sample=node(n+4,"KSampler",model=["1",0],positive=positive,negative=negative,latent_image=latent,
                    seed=10261117+index,steps=23,cfg=6.6,sampler_name="dpmpp_2m",scheduler="karras",denoise=.56)
        decoded=node(n+5,"VAEDecode",samples=sample,vae=["1",2])
        outputs[name]=node(n+6,"SaveImage",images=decoded,filename_prefix=f"{run.name}/{name}")[0]
    (run/"workflow.json").write_text(json.dumps(graph,indent=2),encoding="utf-8")
    (run/"output-nodes.json").write_text(json.dumps(outputs,indent=2),encoding="utf-8")


def hash_file(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def finish(run,prompt):
    with urlopen(f"http://127.0.0.1:8188/history/{prompt}",timeout=15) as response:
        history=json.load(response)[prompt]
    if history["status"]["status_str"]!="success":
        raise RuntimeError(history["status"])
    (run/"comfy-history.json").write_text(json.dumps(history,indent=2),encoding="utf-8")
    outputs=json.loads((run/"output-nodes.json").read_text(encoding="utf-8"))
    masters=Path("C:/AI2D/sprites")/run.name
    masters.mkdir()
    delivery=run/"delivery"
    delivery.mkdir()
    records={}
    start_krita_if_needed()
    with KritaMCP(masters/"logs") as client:
        for index,(name,node) in enumerate(outputs.items()):
            file=history["outputs"][node]["images"][0]
            raw=masters/f"{name}-comfy.png"
            shutil.copyfile(COMFY/"output"/file["subfolder"]/file["filename"],raw)
            gray=np.asarray(Image.open(raw).convert("L"),dtype=float)
            floor=float(np.quantile(gray,.60))
            white=max(floor+16,float(np.quantile(gray,.992)))
            alpha=np.clip((gray-floor)/(white-floor),0,1)**1.35
            grid_y,grid_x=np.mgrid[:SIZE[1],:SIZE[0]]
            if name=="desert":
                # Discard a stamp-like shape in the model's bottom-right corner.
                corner=np.clip(np.maximum((593-grid_x)/14,(287-grid_y)/14),0,1)
                alpha*=corner*corner*(3-2*corner)
            mask=Image.new("RGBA",SIZE,(255,255,255,0))
            mask.putalpha(Image.fromarray(np.rint(alpha*220).astype("uint8")))
            staged=masters/f"{name}-opacity.png"
            mask.save(staged)
            doc=client.call("krita_document",action="open",path=str(staged))["affected_document_id"]
            state=client.call("krita_state",document_id=doc)
            if state["runtime_compatibility"]["status"]!="compatible":
                raise RuntimeError(state["runtime_compatibility"])
            layer=client.call("krita_layers",action="create",document_id=doc,name="Krita · ziarenka i pasma wiatru")["active_layer_id"]
            rng=Random(1411+index)
            for _ in range((10,22,19)[index]):
                x,y=rng.randrange(24,590),rng.randrange(18,290)
                length=rng.randrange(3,13 if index else 28)
                client.call("krita_draw",shape="path",document_id=doc,layer_id=layer,foreground="#ffffff",
                            points=[{"x":x,"y":y},{"x":x+length//2,"y":y+.4},{"x":x+length,"y":y+1.3}],
                            fill_style="None",stroke_style="ForegroundColor",brush_size=rng.uniform(.65,1.35))
            client.call("krita_preview",document_id=doc,max_width=640,max_height=320)
            kra=masters/f"{name}.kra"
            edited=masters/f"{name}.png"
            client.call("krita_save_export",document_id=doc,kra_path=str(kra),png_path=str(edited),overwrite=False)
            client.call("krita_document",action="close",document_id=doc,discard_changes=False)
            alpha=np.asarray(Image.open(edited).convert("RGBA").getchannel("A"),dtype=float)/255
            edge=np.clip(np.minimum(np.minimum(grid_x,SIZE[0]-1-grid_x),
                                    np.minimum(grid_y,SIZE[1]-1-grid_y))/22,0,1)
            alpha*=edge*edge*(3-2*edge)
            png=delivery/f"{name}.png"
            mask.putalpha(Image.fromarray(np.rint(alpha*255).astype("uint8")))
            mask.save(png)
            assert .025<float(alpha.mean())<.27 and alpha.max()>.75
            assert not (alpha[0].any() or alpha[-1].any() or alpha[:,0].any() or alpha[:,-1].any())
            records[name]={"texture_sha256":hash_file(png),"comfy_sha256":hash_file(raw),
                           "kra_sha256":hash_file(kra),"kra":str(kra),"mean_alpha":float(alpha.mean())}
            print(name,"ComfyUI mask and native Krita master saved",flush=True)
    (run/"art-receipt.json").write_text(json.dumps({"prompt":prompt,"model":MODEL,"textures":records},indent=2),encoding="utf-8")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action",choices=["prepare","finish"])
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--prompt")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    if args.action=="prepare":
        prepare(args.run)
    else:
        finish(args.run,args.prompt)

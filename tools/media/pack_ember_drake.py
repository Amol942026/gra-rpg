"""Validate the Krita master, cool the death materials, and pack the Ember Drake."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET
from zipfile import ZipFile

import numpy as np
from PIL import Image

from caldera_corpse_paint import height_field
from ember_drake_cutouts import CELL, ROWS, ORDER, REAR_ORDER


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def cooled_material(tile,row):
    # The shader stores signed pigment offsets in RGB, with independent relief
    # in A. Its visibility always comes from the live cutout, also during falls.
    rgb=np.asarray(tile,dtype=np.float32)[...,:3]
    alpha=np.asarray(tile.getchannel("A"),dtype=np.float32)
    relief=height_field(tile.getchannel("A"))
    lum=rgb @ np.array([.26,.58,.16],dtype=np.float32)
    copper=np.clip((rgb[...,0]-rgb[...,2]-16)/84,0,1)
    fire=copper*np.clip((rgb[...,0]-112)/105,0,1)*np.clip((rgb[...,0]-rgb[...,1]-20)/60,0,1)
    cold=(.22+.32*copper)[...,None]
    stone=lum[...,None]*np.array([.86,.91,.97])
    rest=(rgb*(1-cold)+stone*cold)*(.98 if row in (0,4) else 1.)
    # The living eye and jaw slits lose their incandescent core. Bone and
    # pale plate edges retain their naturally bright colour in the body.
    rest-=fire[...,None]*np.array([58,40,18])*(1 if row==5 else .52)
    strength=np.clip((alpha-10)/145,0,1)[...,None]
    diff=np.clip(128+(rest-rgb)*strength,0,255).astype(np.uint8)
    material=Image.fromarray(diff).resize((128,128),Image.Resampling.LANCZOS)
    material.putalpha(relief.resize((128,128),Image.Resampling.LANCZOS))
    return material


def pack(root,master_dir="krita"):
    path=root/master_dir/"zarowy-drak.png"
    master=root/master_dir/"zarowy-drak.kra"
    with ZipFile(master) as z:
        assert z.testzip() is None
        tree=ET.fromstring(z.read("maindoc.xml"))
        layers=[n.get("name") for n in tree.iter() if n.get("nodetype")=="paintlayer"]
        assert len(layers)==6
    live=Image.open(path).convert("RGBA")
    assert live.size==(CELL*8,CELL*6)
    target=root/"runtime"
    (target/"corpses").mkdir(parents=True,exist_ok=True)
    sheet=Image.new("RGBA",(1024,768),(128,128,128,0))
    gallery=Image.new("RGBA",(CELL*4,CELL*2),(38,43,49,255))
    margins=[]
    differences=[]
    traces=[Image.open(root/"cutouts"/(r+".png")).convert("RGBA") for r in ROWS]
    for d in range(8):
        combined=Image.new("RGBA",(CELL,CELL))
        for row in REAR_ORDER if d>=5 else ORDER:
            bounds=(d*CELL,row*CELL,(d+1)*CELL,(row+1)*CELL)
            cell=live.crop(bounds)
            reference=traces[row].crop(bounds)
            mask=cell.getchannel("A").point(lambda a:255 if a>90 else 0)
            x,y,b,h=mask.getbbox()
            margins.append(min(x,y,CELL-b,CELL-h))
            original,final=np.asarray(reference).astype(int),np.asarray(cell).astype(int)
            solid=np.asarray(mask)>0
            assert solid.sum()>160
            differences.append(float(np.abs(original[...,:3]-final[...,:3])[solid].mean()))
            sheet.paste(cooled_material(cell,row),(d*128,row*128))
            combined.alpha_composite(cell)
        gallery.alpha_composite(combined,((d%4)*CELL,(d//4)*CELL))
    assert min(margins)>=5 and min(differences)>.8
    shutil.copyfile(path,target/"ember-drake.png")
    sheet.save(target/"corpses/ember-drake.png")
    gallery.convert("RGB").save(root/"krita-review.png")
    assert Image.open(target/"corpses/ember-drake.png").size==(1024,768)
    record={"master":str(master),"layers":layers,"source_art_sha256":{
        f.name:digest(f) for f in [*sorted(root.glob("sunburst-*.png")),root/"comfy/drake-paint.png"]},
        "native_krita_sha256":digest(master),"runtime_sha256":digest(target/"ember-drake.png"),
        "corpse_sha256":digest(target/"corpses/ember-drake.png"),
        "atlas_dimensions":list(live.size),"corpse_dimensions":list(sheet.size),
        "checked_parts":len(margins),"min_edge_padding_pixels":min(margins),
        "min_mean_rgb_change_from_material_brushes":round(min(differences),2),
        "max_mean_rgb_change_from_material_brushes":round(max(differences),2)}
    (root/"art-receipt.json").write_text(json.dumps(record,ensure_ascii=False,indent=2),encoding="utf-8")
    return record


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root",type=Path)
    parser.add_argument("--master-dir",default="krita")
    args=parser.parse_args()
    print(pack(args.root,args.master_dir))

"""Prepare the painted 2D parts and new Synfig motion tracks for the jackal."""

import argparse
import io
import json
from pathlib import Path
import zipfile
from xml.etree import ElementTree as ET

import numpy as np
from PIL import Image
from scipy.interpolate import CubicSpline, PchipInterpolator
from scipy.ndimage import distance_transform_edt, gaussian_filter1d, map_coordinates

from desert_enemy_motion import jackal_stride
from fire_imp_synfig_rig import canvas, save
from wolf_motion_synfig import control
import synfig_cli


TILE = 160
PAINT = (
    ("Grzbiet", ((91,123,217,181),(773,137,833,195),(773,416,826,477))),
    ("Kark i obroża", ((183,123,231,167),(774,137,834,174),(773,397,823,442))),
    ("Głowa", ((205,87,257,125),(782,87,834,135),(778,374,820,400))),
    ("Pysk", ((235,107,270,122),(793,114,821,132),(786,385,811,400))),
    ("Uszy", ((217,71,242,104),(785,71,804,105),(778,360,797,401))),
    ("Ogon", ((74,167,110,207),(74,167,110,207),(787,462,808,506))),
)
TRACKS = {
    "step": ((0,1),(38,-1),(47,-.34),(52,.5),(60,1)),
    "lift": ((0,0),(38,0),(41,.13),(46,.70),(50,1),(55,.47),(58,.07),(60,0)),
    "weight": ((0,-.28),(7,.62),(15,.21),(23,-.65),(30,-.28),(37,.62),(45,.21),(53,-.65),(60,-.28)),
    "support": ((0,.20),(4,.89),(9,.67),(14,-.02),(20,-.69),(25,-.48),(30,.20),
                (34,.89),(39,.67),(44,-.02),(50,-.69),(55,-.48),(60,.20)),
    "spine": ((0,-.27),(5,.20),(11,.46),(17,.04),(23,-.48),(30,-.27),
              (35,.20),(41,.46),(47,.04),(53,-.48),(60,-.27)),
    "head": ((0,.17),(8,-.29),(17,-.47),(25,.24),(30,.17),(38,-.29),(47,-.47),(55,.24),(60,.17)),
    "tail": ((0,-.37),(8,-.9),(18,-.49),(27,.60),(35,.88),(46,.32),(54,-.32),(60,-.37)),
    "crouch": ((0,0),(8,.06),(17,.43),(22,1),(26,.35),(31,.03),(40,.46),(47,.19),(60,0)),
    "pounce": ((0,0),(12,-.09),(21,-.30),(25,.06),(29,.85),(32,1),(38,.73),(47,.20),(60,0)),
    "air": ((0,0),(22,0),(26,.34),(30,1),(34,.54),(42,0),(60,0)),
    "jaw": ((0,0),(19,0),(23,.45),(26,.96),(29,1),(33,.15),(36,0),(60,0)),
    "collapse": ((0,0),(6,.025),(14,.13),(23,.34),(32,.72),(43,.94),(52,1),(60,1)),
    "roll": ((0,0),(8,-.055),(18,.16),(28,.52),(39,.88),(48,.98),(56,1),(60,1)),
}


def flank(source):
    # Use the painted back and actual belly as the UV edges of a single
    # flank. Sampling the rectangular bounding box would include the legs.
    steps = np.linspace(0,1,13)
    along = np.linspace(0,1,TILE-4)
    fraction = np.linspace(.025,.975,TILE-4)[:,None]
    back = PchipInterpolator(steps,(153,139,132,127,125,125,126,127,124,123,121,126,133))(along)
    belly = PchipInterpolator(steps,(177,180,179,171,164,160,166,171,175,178,177,173,164))(along)
    x = np.broadcast_to(90+128*along,(TILE-4,TILE-4))
    y = back+(belly-back)*fraction
    return Image.fromarray(np.stack([np.uint8(map_coordinates(source[...,i],(y,x),order=1))
                                     for i in range(3)],axis=-1))


def extend_paint(pixels):
    solid = pixels[...,3]>=160
    if not solid.any():
        raise ValueError("Pusty fragment atlasu szakala")
    nearest = distance_transform_edt(~solid,return_distances=False,return_indices=True)
    return pixels[tuple(nearest)][...,:3]


def leg_paint(root, fore):
    # Reuse the original painted segments on one UV strip per leg. The rig
    # bends that strip. Every joint uses the same edge on the adjacent spans.
    with Image.open(root/"turn-limbs.png") as image:
        ink = np.asarray(image.convert("RGBA"))
    header = (root/"turn-rig.bin").read_bytes()
    if header[:4]!=b"JTR1":
        raise ValueError("Nieznany układ źródłowych warstw łap")
    descriptors = np.frombuffer(header,dtype="<f4",count=56,offset=4).reshape(2,4,7)[int(fore)]
    pieces=[]
    for (x,y,w,h),width in zip(descriptors[:,:4].astype(int),(54,44,37,21)):
        colors = extend_paint(ink[y:y+h,x:x+w])
        rotated = Image.fromarray(colors.transpose(1,0,2))
        pieces.append(np.asarray(rotated.resize((width,TILE-4),Image.Resampling.LANCZOS)))
    return Image.fromarray(gaussian_filter1d(np.concatenate(pieces,axis=1),1.05,axis=1,mode="nearest"))


def make_paint(source, output):
    with Image.open(source) as original:
        original = original.convert("RGBA")
    rows = [*(name for name,_ in PAINT),"Przednia łapa","Tylna łapa"]
    atlas = Image.new("RGBA",(TILE*3,TILE*len(rows)))
    document = ET.Element("image",w=str(atlas.width),h=str(atlas.height),name="Szakal wydmowy - części 2D",version="0.0.3")
    stack = ET.SubElement(document,"stack")
    painted = []
    for row,name in enumerate(rows):
        leg = leg_paint(source.parent,fore=row==len(PAINT)) if row>=len(PAINT) else None
        for column in range(3):
            if leg is not None:
                crop = leg
            elif row==0 and column==0:
                crop = flank(np.asarray(original))
            else:
                rgb = extend_paint(np.asarray(original.crop(PAINT[row][1][column])))
                crop = Image.fromarray(rgb).resize((TILE-4,TILE-4),Image.Resampling.LANCZOS)
            rgb = np.pad(np.asarray(crop),((2,2),(2,2),(0,0)),mode="edge")
            part = Image.fromarray(rgb).convert("RGBA")
            x,y = column*TILE,row*TILE
            atlas.paste(part,(x,y))
            painted.append((part,f"{name} / {('profil','przód','tył')[column]}",x,y))
    png = output/"rig-paint.png"
    atlas.save(png)
    with zipfile.ZipFile(output/"czesci-szakala.ora","x",compression=zipfile.ZIP_DEFLATED) as package:
        package.writestr("mimetype","image/openraster",compress_type=zipfile.ZIP_STORED)
        for i,(part,name,x,y) in enumerate(reversed(painted)):
            path = f"data/{i}.png"
            ET.SubElement(stack,"layer",name=name,src=path,x=str(x),y=str(y),visibility="visible",opacity="1.0",
                          **{"composite-op":"svg:src-over"})
            data = io.BytesIO()
            part.save(data,format="PNG")
            package.writestr(path,data.getvalue())
        package.writestr("mergedimage.png",png.read_bytes())
        package.writestr("stack.xml",ET.tostring(document,encoding="utf-8"))
    return png


def make_motion(output):
    project = canvas("Szakal / rig 2D",360,310,(130,12),60)
    path = output/"ruch-szakala.sif"
    for row,(name,keys) in enumerate(TRACKS.items()):
        frames,values = zip(*keys)
        interpolate = (CubicSpline(frames,values,bc_type="periodic")
                       if name in ("weight","support","spine","head","tail") else PchipInterpolator(frames,values))
        samples = [(100*(jackal_stride(i/60) if name=="step" else float(interpolate(i))),-20*row) for i in range(61)]
        control(project,name,samples,3.8,(.58+.04*(row%3),.63,.44+.06*(row%4)))
    save(project,path)
    arrays = ["#pragma once","// Sampled from the jackal's Synfig rig tracks.","namespace ember::jackal_motion {"]
    for layer in ET.parse(path).findall("layer"):
        values = [float(v.text)/100 for v in layer.findall("param[@name='origin']/animated/waypoint/vector/x")]
        if len(values)!=61:
            raise ValueError("Każda krzywa Synfiga powinna zawierać 61 próbek")
        lines = [" "+",".join(f"{v:.5f}f" for v in values[i:i+10])+"," for i in range(0,61,10)]
        arrays.extend([f"inline constexpr float {layer.get('desc')}[61]={{",*lines,"};"])
    (output/"jackal_rig_motion.hpp").write_text("\n".join([*arrays,"}",""]),encoding="utf-8")
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True,exist_ok=False)
    source = Path("assets/desert/jackal-turn/painted-views.png")
    paint = make_paint(source,args.root)
    motion = make_motion(args.root)
    render = synfig_cli.render(motion,output_root=args.root/"ruch",threads=2,expected_frames=61,require_motion=True)
    report = {"source_sha256":synfig_cli.sha(source),"atlas_sha256":synfig_cli.sha(paint),"render":render}
    (args.root/"receipt.json").write_text(json.dumps(report,indent=2),encoding="utf-8")


if __name__=="__main__":
    main()

"""Pack the unwarped painted views of the Wraith with its cloth animation metadata."""
import argparse
import json
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage

from ash_wraith_paint import LANDMARKS, MIRRORED, ORDER, SOURCE, SWAP
from ash_wraith_turn import material

ROOT = Path(__file__).resolve().parents[2]
SOURCE_ART = ROOT / "art/volcano/ash-wraith/paint/turn-keys"
CELL = 192
SIDE = 2048
ORIGINAL_ANGLES = (0, 61, 90, 119, 180, 233, 270, 307, 360)
MID_SOURCE = (0, 1, 1, 0, 7, 6, 6, 7)
MID_MIRROR = (2, 3, 4, 5)
QUADRANTS = {0: (1, 0), 1: (0, 0), 6: (1, 1), 7: (0, 1)}


def canonical(source, direction):
    parts = [source.crop((direction*CELL,p*CELL,(direction+1)*CELL,(p+1)*CELL)) for p in range(6)]
    pivots, palms = LANDMARKS[SOURCE[direction]]
    if direction in MIRRORED:
        pivots = [(CELL-1-pivots[i][0],pivots[i][1]) for i in SWAP]
        palms = [(CELL-1-x,y) for x,y in reversed(palms)]
    return parts, pivots, palms, ORDER[direction]


def select_pose(image, quadrant):
    x,y = quadrant
    w,h = image.size
    crop = np.array(image.crop((w*x//2,h*y//2,w*(x+1)//2,h*(y+1)//2)))
    alpha = crop[...,3]
    labels,_ = ndimage.label(alpha>70)
    areas = np.bincount(labels.ravel())
    areas[0] = 0
    alpha[~ndimage.binary_dilation(labels==areas.argmax(),iterations=4)] = 0
    sprite = Image.fromarray(crop)
    sprite = sprite.crop(sprite.getchannel("A").getbbox())
    sprite = sprite.resize((round(sprite.width*152/sprite.height),152),Image.Resampling.LANCZOS)
    tile = Image.new("RGBA",(CELL,CELL))
    tile.paste(sprite,((CELL-sprite.width)//2,18))
    return tile


def painted_layers(image, guide):
    rgba = np.asarray(image)
    alpha = rgba[...,3]
    visible = alpha>0
    rows,cols = np.indices(alpha.shape)
    seam_x,seam_y = np.transpose(guide["seam"])
    seam = np.interp(cols,seam_x,seam_y)
    center = guide["hinges"][0][0]
    half = 8+np.clip((rows-75)*.1,0,9)
    low = rows>seam
    owner = np.full(alpha.shape,3,dtype=np.uint8)
    owner[low] = 0
    owner[low & (cols<center-half)] = 2 if guide["front"] else 1
    owner[low & (cols>center+half)] = 1 if guide["front"] else 2
    for p,label in ((4,"far"),(5,"near")):
        mask = Image.new("L",image.size)
        ImageDraw.Draw(mask).polygon([tuple(q) for q in guide[label]],fill=255)
        owner[np.asarray(mask)>0] = p
    owner[~low] = 3
    parts = [None]*6
    for depth,p in enumerate(guide["depth"]):
        section = visible & (owner==p)
        nearest = ndimage.distance_transform_edt(~section,return_distances=False,return_indices=True)
        covered = np.isin(owner,guide["depth"][depth+1:]) & (alpha==255)
        inside = section | (ndimage.binary_dilation(section,iterations=5) & covered)
        art = rgba[tuple(nearest)].copy()
        art[...,3] = np.where(inside,alpha,0)
        art[~inside] = 0
        parts[p] = Image.fromarray(art)
    combined = Image.new("RGBA",image.size)
    for p in guide["depth"]:
        combined.alpha_composite(parts[p])
    rebuilt = np.asarray(combined)
    assert np.array_equal(rebuilt[visible],rgba[visible])
    return parts


def mirror_view(view):
    parts,pivots,palms,depth = view
    mirror = lambda point: (CELL-1-point[0],point[1])
    return ([parts[p].transpose(Image.Transpose.FLIP_LEFT_RIGHT) for p in SWAP],
            [mirror(pivots[p]) for p in SWAP],
            [mirror(point) for point in reversed(palms)],
            [SWAP[p] for p in depth])


def prepared_keys(layer_exports):
    directions = Image.open(ROOT/"assets/volcano/ash-wraith.png").convert("RGBA")
    authored = Image.open(SOURCE_ART/"sunburst.png").convert("RGBA")
    guides = json.loads((SOURCE_ART/"layers.json").read_text("utf-8"))
    additional = {}
    for n,xy in QUADRANTS.items():
        data = guides[str(n)]
        tile = select_pose(authored,xy)
        layers = painted_layers(tile,data)
        if layer_exports:
            layer_exports.mkdir(parents=True,exist_ok=True)
            for p,layer in enumerate(layers):
                layer.save(layer_exports/f"{n}-{p}.png")
        additional[n] = layers,data["hinges"],data["palms"],data["depth"]
    result = []
    for n in range(8):
        result.append(canonical(directions,n))
        added = additional[MID_SOURCE[n]]
        result.append(mirror_view(added) if n in MID_MIRROR else added)
    return result


def pack(views, output):
    paint = Image.new("RGBA",(SIDE,SIDE))
    relief = Image.new("RGBA",paint.size,(128,128,128,0))
    tiles = {}
    entries = []
    shelves = []
    for d,(parts,*_) in enumerate(views):
        for p,part in enumerate(parts):
            bounds = part.getchannel("A").getbbox()
            if not bounds:
                raise ValueError(f"Empty {d}/{p} paint")
            entries.append((d,p,part,bounds))
    for d,p,part,(left,top,right,bottom) in sorted(entries,key=lambda item: -item[-1][3]+item[-1][1]):
        w,h = right-left,bottom-top
        row = next((record for record in shelves if record[2]>=h+4 and record[0]+w+4<=SIDE),None)
        if row is None:
            row = [0,sum(x[2] for x in shelves),h+4]
            shelves.append(row)
        if row[1]+h+4>SIDE:
            raise ValueError("Painted keys exceed one texture")
        x,y = row[0]+2,row[1]+2
        row[0] += w+4
        paint.paste(part.crop((left,top,right,bottom)),(x,y))
        relief.paste(material(part).crop((left,top,right,bottom)),(x,y))
        tiles[d,p] = x,y,w,h,left,top
    output.mkdir(parents=True,exist_ok=True)
    extent = (0,0,SIDE,sum(x[2] for x in shelves))
    paint.crop(extent).save(output/"painted-turn.png",optimize=True)
    relief.crop(extent).save(output/"painted-turn-relief.png",optimize=True)
    angles = [x for a,b in zip(ORIGINAL_ANGLES,ORIGINAL_ANGLES[1:]) for x in (a,(a+b)/2)]
    lines = ["#pragma once","namespace ash_wraith_painted {",
             "constexpr int Count=16, Cell=192;","struct Tile {short x,y,w,h,ox,oy;};",
             "constexpr float angles[Count]={"+",".join(f"{x:.3f}f*DEG2RAD" for x in angles)+"};",
             "constexpr Tile tiles[Count][6]={"]
    for d in range(len(views)):
        lines.append("  {"+",".join("{"+",".join(map(str,tiles[d,p]))+"}" for p in range(6))+"},")
    lines.extend(["};","constexpr float pivots[Count][6][2]={"])
    def points(row):
        return "  {"+",".join("{"+f"{x*2/3:.4f}f,{y*2/3:.4f}f"+"}" for x,y in row)+"},"
    lines.extend(points(pivots) for _,pivots,_,_ in views)
    lines.extend(["};","constexpr float palms[Count][2][2]={"])
    lines.extend(points(palms) for _,_,palms,_ in views)
    lines.extend(["};","constexpr int order[Count][6]={"])
    lines.extend("  {"+",".join(map(str,depth))+"}," for _,_,_,depth in views)
    lines.extend(["};","}",""])
    (output/"ash_wraith_painted.hpp").write_text("\n".join(lines),"ascii")


if __name__=="__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("output",type=Path)
    cli.add_argument("--layers",type=Path)
    cli.add_argument("--no-audio",required=True,action="store_true")
    args = cli.parse_args()
    cv2.setNumThreads(2)
    pack(prepared_keys(args.layers),args.output)

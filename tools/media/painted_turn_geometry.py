"""Register the edge of each articulated cutout. The artwork stays in its original pose."""
import argparse
import re
from pathlib import Path

import cv2
import numpy as np
from PIL import Image
from scipy import ndimage

from caldera_turn_registration import optical, remap, xy


ROOT = Path(__file__).resolve().parents[2]
GRID = 16
POSES = 16
PARTS = 6


def signed_distance(alpha):
    inside = alpha >= 128
    result = np.where(inside,ndimage.distance_transform_edt(inside)-.5,
                      .5-ndimage.distance_transform_edt(~inside))
    soft = (alpha > 5) & (alpha < 250)
    result[soft] = alpha[soft]/255-.5
    return result


def edge_guide(alpha):
    size = xy.shape[0]
    resized = cv2.resize(alpha,(size,size),interpolation=cv2.INTER_LINEAR)
    field = signed_distance(resized)
    # Describe the boundary alone. Printed embroidery and eye highlights
    # must not drag the limb or its visible edge during registration.
    return np.uint8(np.clip(128+field*15,0,255))


def edge_transport(first,second):
    forward = optical(first,second)
    backward = optical(second,first)
    a,b = forward,backward
    for _ in range(3):
        a = remap(forward,xy-.5*a)
        b = remap(backward,xy-.5*b)
    stride = .5*(a-b)
    stride = cv2.resize(stride,(GRID,GRID),interpolation=cv2.INTER_AREA)
    stride = cv2.GaussianBlur(stride,(3,3),.68)
    # A cutout may move across the screen with its rig. Its local contour
    # cannot be pulled farther than a small fraction of its own rectangle.
    return np.rint(128+3*np.clip(stride,-20,20)).astype(np.uint8)


def prepare(image,tiles,out,stem):
    ink = np.zeros((image.height,image.width,4),dtype=np.uint8)
    motion = np.full((PARTS*GRID,POSES*GRID,3),128,dtype=np.uint8)
    for part in range(PARTS):
        guide = []
        for direction in range(POSES):
            x,y,w,h,*_ = tiles[direction*PARTS+part]
            # One padded texel lets an antialiased edge pass between pixels.
            # The packing tools reserve two transparent texels around a tile.
            crop = np.asarray(image.crop((x-1,y-1,x+w+1,y+h+1))).copy()
            alpha = crop[...,3]
            field = signed_distance(alpha)
            nearest = ndimage.distance_transform_edt(alpha<120,return_distances=False,return_indices=True)
            empty = alpha<120
            crop[empty,:3] = crop[tuple(nearest)][empty,:3]
            crop[...,3] = np.rint(np.clip(128+field*10,0,255))
            ink[y-1:y+h+1,x-1:x+w+1] = crop
            guide.append(edge_guide(alpha[1:-1,1:-1]))
        for n,first in enumerate(guide):
            motion[part*GRID:(part+1)*GRID,n*GRID:(n+1)*GRID,:2] = edge_transport(first,guide[(n+1)%POSES])
    Image.fromarray(ink).save(out/f"{stem}-ink.png",optimize=True)
    Image.fromarray(motion).save(out/f"{stem}-flow.png",optimize=True)


if __name__=="__main__":
    cli=argparse.ArgumentParser(description=__doc__)
    cli.add_argument("output",type=Path)
    cli.add_argument("--no-audio",required=True,action="store_true")
    args=cli.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    cv2.setNumThreads(2)
    for name,path,header,fields in (
        ("imp","imp-turn/directional-cels.png","fire_imp_cels.hpp",8),
        ("wraith","wraith-turn/painted-turn.png","ash_wraith_painted.hpp",6),
    ):
        data=(ROOT/"src/generated"/header).read_text("utf-8")
        if name=="wraith":
            data=data.split("constexpr float pivots",1)[0]
        tiles=[tuple(map(int,match.split(","))) for match in
               re.findall(r"\{(\d+(?:,\d+){"+str(fields-1)+r"})\}",data)]
        if len(tiles)!=POSES*PARTS:
            raise ValueError(f"{name}: expected {POSES*PARTS} rectangles, found {len(tiles)}")
        image=Image.open(ROOT/"assets/volcano"/path).convert("RGBA")
        prepare(image,tiles,args.output,name)

import argparse
from pathlib import Path
from PIL import Image, ImageFilter
import json
import re
import numpy as np

parser=argparse.ArgumentParser(description="Pack the hidden Emberfall Wraith capture into its directional atlas.")
parser.add_argument("capture",type=Path)
parser.add_argument("output",type=Path)
parser.add_argument("--no-audio",action="store_true",required=True)
args=parser.parse_args()
raw=args.capture
out=args.output
out.mkdir(parents=True,exist_ok=True)
header=Path(__file__).resolve().parents[2]/"src/generated/ash_wraith_turn_atlas.hpp"
atlas_header=header.read_text(encoding="utf-8")
records=re.findall(r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}",atlas_header)
assert len(records)==32*6,len(records)
records=np.array(records,dtype=float).reshape((32,6,6))
angles=np.array([0,61,90,119,180,233,270,307,360])
count=96
pieces=[]
for n in range(count):
    angle=n*360/count
    sector=int(np.searchsorted(angles,angle,side='right'))-1
    t=(angle-angles[sector])/(angles[sector+1]-angles[sector])
    k=4*(sector+t)
    a=int(k)%32;b=(a+1)%32
    dims=(1-(k%1))*records[a,:,2:4]+k%1*records[b,:,2:4]
    for part in range(6):
        image=Image.open(raw/f"{n}-{part}.png").convert("RGBA")
        assert image.width==image.height==216
        w,h=map(int,np.rint(dims[part]*1.27))
        image=image.resize((max(12,w),max(12,h)),Image.Resampling.LANCZOS)
        alpha=image.getchannel("A")
        # Restore the fine woven pigment lost in resizing the rectangular
        # cutout. Preserve the exact coverage and leave its edge unsharpened.
        inset=alpha.filter(ImageFilter.MinFilter(5))
        paint=image.convert("RGB")
        detail=paint.filter(ImageFilter.UnsharpMask(radius=1.25,percent=140,threshold=2))
        image=Image.composite(detail,paint,inset)
        image.putalpha(alpha)
        rect=image.getchannel("A").getbbox()
        assert rect is not None,(n,part)
        pieces.append((n,part,image.crop(rect),rect[:2],image.size))
pages=[]
shelves=[]
tiles={}
for n,part,sprite,(ox,oy),(w,h) in sorted(pieces,key=lambda p:(-p[2].height,-p[2].width,p[0],p[1])):
    border=2
    rw,rh=sprite.width+border*2,sprite.height+border*2
    p=0
    while True:
        if p==len(pages):
            pages.append(Image.new("RGBA",(2048,2048)))
            shelves.append([])
        rows=shelves[p]
        row=next((r for r in rows if r[2]>=rh and r[0]+rw<=2048),None)
        if row is None:
            y=sum(r[2] for r in rows)
            if y+rh<=2048:
                row=[0,y,rh];rows.append(row)
        if row is not None:break
        p+=1
    x,y=row[0]+border,row[1]+border
    row[0]+=rw
    pages[p].paste(sprite,(x,y))
    # Extend color under transparent atlas gutters for bilinear edge sampling.
    for dx,dy in [(0,-1),(0,1),(-1,0),(1,0)]:
        if dx:
            col=sprite.crop((0 if dx<0 else sprite.width-1,0,1 if dx<0 else sprite.width,sprite.height))
            cx=x-1 if dx<0 else x+sprite.width;cy=y
        else:
            row_y=0 if dy<0 else sprite.height-1
            col=sprite.crop((0,row_y,sprite.width,row_y+1))
            cx=x;cy=y-1 if dy<0 else y+sprite.height
        col.putalpha(0);pages[p].paste(col,(cx,cy))
    tiles[n,part]=(p,x,y,sprite.width,sprite.height,ox,oy,w,h)
if len(pages)!=1:
    raise ValueError("The robe cel atlas must fit in one 2048 by 2048 texture.")
height=sum(row[2] for row in shelves[0])
pages[0].crop((0,0,2048,height)).save(out/"directional-robe.png",optimize=True)
table=["#pragma once","namespace ember::ash_wraith_cels {",
       f"inline constexpr int Directions={count},PageCount={len(pages)};",
       "struct Tile { short page,x,y,w,h,ox,oy,canvasWidth,canvasHeight; };",
       "inline constexpr Tile tiles[Directions][6]={"]
for n in range(count):
    table.append("  {"+",".join("{"+",".join(map(str,tiles[n,p]))+"}" for p in range(6))+"},")
(out/"ash_wraith_cels.hpp").write_text("\n".join(table+["};","}",""]),encoding="ascii")
print(json.dumps({"frames":count,"parts":len(pieces),"pages":[[2048,sum(row[2] for row in rows)] for rows in shelves]}))


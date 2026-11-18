"""Separate and articulate the lower jaw of the painted ember drake for the close views."""

import argparse
from math import cos, radians, sin
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw


TILE = 96
LEVELS = 8
ORIGINS = ((91, 70), (89, 43), (47, 42), (7, 43), (5, 70))
# Registration in the original 192 px cutouts. Each crease runs along the closed lips.
SEAMS = (
    ((150, 115), (155, 114.3), (160, 115.5), (167, 116.5), (173, 118.2), (178.5, 120.4)),
    ((148, 99), (153, 100.8), (158.5, 104), (163, 107.2), (168, 110.4), (173.5, 115.5)),
    ((85, 100), (89, 101.8), (93.5, 104), (97.5, 104.3), (101, 102), (105.5, 98.5)),
)
QUALITY = 4
LAYERS = ("upper-head", "throat-teeth", "mandible")


def geometry(view, level):
    phase = level/LEVELS
    hx, hy = SEAMS[view][0]
    if view == 2:
        # Foreshortened opening towards the observer. The corners stay under the cheeks.
        return (1-.055*phase, 0, .055*phase*95, 0, 1+.13*phase, (8.8-.13*103)*phase)
    angle = radians((25 if view == 0 else 22)*phase)
    c, s = cos(angle), sin(angle)
    return c, -s, hx-c*hx+s*hy, s, c, hy-s*hx-c*hy


def transform(affine, point):
    a,b,c,d,e,f = affine
    x,y = point
    return a*x+b*y+c, d*x+e*y+f


def render(view, head, level):
    seam = SEAMS[view]
    q = QUALITY
    size = (192*q,192*q)
    head = head.resize(size, Image.Resampling.BICUBIC)
    def poly(draw, pts, color):
        draw.polygon([(round(x*q),round(y*q)) for x,y in pts], fill=color)

    mask = Image.new("L",size)
    poly(ImageDraw.Draw(mask), [*seam, (seam[-1][0]+2,137), (seam[0][0],137)], 255)
    jaw = head.copy()
    jaw.putalpha(ImageChops.multiply(head.getchannel("A"),mask))
    top = head.copy()
    top.putalpha(ImageChops.subtract(head.getchannel("A"),jaw.getchannel("A")))
    aff = geometry(view,level)
    a,b,c,d,e,f = aff
    det = a*e-b*d
    inverse = (e/det,-b/det,(b*f-e*c)*q/det,-d/det,a/det,(d*c-a*f)*q/det)
    jaw = jaw.transform(size,Image.Transform.AFFINE,inverse,Image.Resampling.BICUBIC)
    mouth = Image.new("RGBA",size)
    ink = ImageDraw.Draw(mouth)
    bottom = [transform(aff,point) for point in seam]
    def edge(u,v,t):
        return u[0]+(v[0]-u[0])*t, u[1]+(v[1]-u[1])*t
    for i in range(len(seam)-1):
        topA,topB,lowA,lowB = seam[i],seam[i+1],bottom[i],bottom[i+1]
        if view == 2 or i < 3:
            # The dark cheek membrane ends before the canine. Beyond it the
            # open tips show the terrain in profile, with a narrow lower gum.
            rear = lowB if view == 2 or i < 2 else edge(lowA,lowB,.18)
            poly(ink, [topA,topB,rear,lowA], (48,24,33,255))
            poly(ink, [edge(topA,lowA,.64),edge(topB,rear,.64),rear,lowA],(107,41,44,255))
        poly(ink, [edge(topA,lowA,.84),edge(topB,lowB,.88),lowB,lowA],(154,65,52,255))
        if i in (1,2,3,4):
            t = .43
            root=edge(topA,topB,t)
            inner=edge(lowA,lowB,t)
            length=min(.68 if i in (1,3) else .38, (3.2 if i in (1,3) else 1.65)/max(.01,inner[1]-root[1]))
            poly(ink,[edge(topA,topB,.20),edge(topA,topB,.68),edge(root,inner,length)],(223,192,153,255))
    return [im.resize((192,192),Image.Resampling.LANCZOS) for im in (top,mouth,jaw)]


def prepare(atlas, root):
    full = Image.open(atlas).convert("RGBA")
    if full.size != (1536,1152):
        raise ValueError("Jaw registration requires the original 192 px painted drake cutouts")
    root.mkdir(parents=True,exist_ok=True)
    layers = [Image.new("RGBA",(TILE*5, TILE*LEVELS)) for _ in LAYERS]
    for direction,(left,top) in enumerate(ORIGINS):
        view = (0,1,2,1,0)[direction]
        head = full.crop((direction*192,5*192,(direction+1)*192,6*192))
        mirror = direction > 2
        if mirror:
            head = head.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
        for level in range(1, LEVELS+1):
            for destination,part in zip(layers,render(view,head,level),strict=True):
                if mirror:
                    part = part.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
                crop = part.crop((left,top,left+TILE,top+TILE))
                if not crop.getbbox():
                    raise ValueError(f"Empty jaw paint at {direction}/{level}")
                destination.paste(crop,(direction*TILE,(level-1)*TILE))
    combined=Image.new("RGBA",layers[0].size)
    for name,layer in zip(LAYERS,layers,strict=True):
        layer.save(root/(name+".png"))
    for layer in (layers[1],layers[0],layers[2]):
        combined.alpha_composite(layer)
    combined.save(root/"jaw-proof.png")
    return combined


if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("atlas",type=Path)
    parser.add_argument("directory",type=Path)
    args=parser.parse_args()
    prepare(args.atlas,args.directory)

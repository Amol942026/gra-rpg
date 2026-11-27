"""Extract the overlapping painted pieces of the 2D swamp turnaround.

Coordinates are percentages of the original Krita paintings. Rebuild in the
project's media Python; image sampling and keying take place in Blender.
"""
import argparse
import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter


def pieces():
    # Each piece includes pigment under its neighbour for continuous occlusion.
    return (
        ("toad-back", 0, "back", "21,44 21,33 27,22 33,16 42,12 56,11 66,14 73,21 78,34 79,46 75,56 72,67 64,78 54,84 44,85 35,79 28,69 25,56"),
        ("toad-nape", 0, "back", "25,42 24,29 31,17 35,7 40,2 45,3 49,9 56,7 60,1 66,3 70,13 76,24 77,40 70,52 58,59 43,60 30,54"),
        ("toad-chin", 0, "front", "23,41 23,33 28,27 30,22 36,19 44,20 50,21 58,19 64,20 69,24 72,32 75,40 72,46 70,57 64,65 60,72 52,78 44,77 36,72 31,62"),
        ("toad-cheek", 0, "side", "1,34 4,28 7,25 9,18 13,15 18,16 23,18 29,19 34,24 37,32 39,40 36,49 32,56 28,62 23,64 17,60 13,55 10,48 7,45 3,41 1,39"),
        ("toad-flank", 0, "side", "30,31 38,26 47,24 57,23 68,26 76,33 76,41 70,48 66,55 66,64 61,71 58,78 49,79 42,77 39,70 38,62 40,56 39,47 34,41"),
        ("toad-brow", 0, "front", "29,32 28,27 30,23 34,20 40,21 45,26 43,33 39,38 33,37"),
        ("toad-hip", 0, "side", "64,50 69,44 77,41 83,41 88,46 92,53 94,64 90,71 83,76 73,78 66,75 63,68"),
        ("toad-hindfoot", 0, "back", "26,70 29,72 29,78 30,81 33,85 30,89 23,92 21,97 18,98 19,92 12,94 9,98 7,98 8,94 14,88 6,90 1,88 1,86 11,83 3,82 2,79 5,78 17,81 22,78"),
        ("toad-arm", 0, "front", "15,60 16,55 20,50 25,49 29,52 31,58 30,68 29,74 28,78 33,80 34,84 30,83 28,81 31,88 36,90 37,94 34,94 28,89 29,96 28,99 26,98 25,90 21,86 18,94 16,94 15,92 18,84 19,81 16,74"),
        ("leech-ring", 1, "back", "6,37 17,31 30,29 48,28 66,28 82,30 92,34 90,42 76,47 56,49 38,48 18,45 6,43"),
        ("leech-ring-dark", 1, "back", "13,51 24,48 33,48 44,46 58,46 71,48 85,51 88,54 85,60 72,63 54,65 36,64 23,61 16,58"),
        ("leech-ring-rim", 1, "back", "11,22 22,20 39,17 55,15 72,15 83,17 91,20 88,26 75,30 60,32 37,32 19,30 12,28"),
        ("leech-spine", 1, "side", "38,22 41,19 44,13 47,8 50,5 48,10 47,15 47,18 46,23 44,26"),
        ("leech-mouth", 1, "front", "12,78 13,71 17,66 24,65 31,67 36,73 38,80 34,89 30,96 27,97 23,93 18,92 14,88"),
        ("wisp-crown", 2, "front", "13,36 20,28 29,22 36,18 38,11 39,6 44,2 51,1 55,4 60,7 68,5 71,6 72,10 76,12 82,12 81,15 73,14 68,10 62,10 56,11 48,5 45,6 44,11 46,16 51,19 57,22 62,28 64,37 62,44 55,48 42,52 31,51 22,48 17,43"),
        ("wisp-lamp", 2, "back", "18,36 20,31 24,26 31,22 37,17 42,15 48,18 54,23 60,30 61,36 57,43 51,47 43,49 35,48 27,46 22,43"),
        ("wisp-eye", 2, "front", "38,35 40,32 43,31 47,32 50,35 48,39 46,41 42,41 39,39"),
        ("wisp-leaf", 2, "front", "53,45 59,42 65,44 66,50 65,54 63,59 57,62 53,66 48,67 44,70 42,73 43,77 47,80 46,83 42,82 38,79 36,76 35,72 36,69 41,65 45,63 50,58 54,51"),
        ("wisp-tail", 2, "front", "61,49 65,51 63,57 57,62 52,66 46,69 44,73 46,77 50,78 51,81 50,84 46,85 43,88 40,89 38,94 40,97 38,99 36,96 35,93 37,88 41,85 44,84 47,81 43,80 39,78 37,75 37,71 40,68 46,65 51,61 57,56"),
        ("wisp-fan", 2, "side", "29,35 34,33 41,35 48,36 55,37 62,39 69,41 77,44 82,47 90,48 94,51 98,52 99,55 95,54 90,52 85,52 80,50 76,49 73,48 70,48 66,46 62,45 56,45 50,43 45,42 40,42 36,40 30,39"),
        ("keeper-mantle", 3, "back", "14,36 16,27 23,18 29,12 37,9 46,5 59,6 65,9 74,16 78,20 84,28 87,34 82,34 83,43 78,43 80,49 73,46 71,58 65,54 60,65 55,60 49,66 44,59 40,66 35,58 31,61 28,47 23,52 23,43 18,47 19,37"),
        ("keeper-chain", 3, "back", "39,13 38,8 39,4 42,3 45,4 47,8 46,13 49,21 49,31 51,41 53,51 54,60 55,67 56,76 58,80 58,86 55,88 52,85 51,78 50,68 47,60 47,50 45,43 44,36 43,29 42,23"),
        ("keeper-bell", 3, "front", "19,37 21,32 24,28 26,22 27,16 29,12 34,10 34,7 36,4 39,3 42,4 43,6 42,10 46,11 49,15 49,21 50,27 53,32 54,37 51,40 45,42 41,42 40,46 38,47 36,45 36,42 30,41 24,40"),
        ("keeper-shield", 3, "front", "60,11 67,8 71,6 70,12 81,9 77,15 85,12 82,19 88,20 83,24 88,26 83,27 87,31 81,30 84,36 78,33 81,41 76,35 77,44 71,38 73,45 67,39 65,36 61,30 58,24"),
        ("keeper-upper", 3, "back", "18,23 23,21 26,25 24,33 21,40 17,47 15,54 12,57 8,54 8,49 12,41 13,33"),
        ("keeper-hand", 3, "front", "9,44 15,42 17,46 17,55 15,61 13,66 15,70 17,74 18,77 16,81 14,82 15,76 12,73 11,78 8,84 7,85 7,80 9,75 6,77 5,82 7,88 4,85 2,81 2,75 3,70 2,66 4,56"),
        ("keeper-shin", 3, "back", "27,59 34,56 40,60 41,65 37,74 38,79 41,85 40,90 41,94 35,95 31,97 28,98 29,95 21,98 17,98 21,93 12,96 14,92 21,88 23,83 23,76 22,71 24,65"),
        ("keeper-skirt", 3, "back", "24,28 31,23 37,24 47,23 57,24 67,27 74,34 72,43 76,48 70,47 70,54 67,51 64,66 61,61 57,69 56,62 52,66 49,62 48,72 45,66 42,67 39,60 33,65 31,57 27,61 28,48 24,52 25,43"),
        ("keeper-foot", 3, "side", "66,86 71,83 78,81 82,81 85,86 91,89 97,94 94,96 85,95 79,95 73,97 67,97 66,96 56,97 55,95 60,92 54,94 54,92 61,88"),
        ("keeper-crown", 3, "back", "17,25 18,12 28,8 29,1 40,0 55,0 68,0 70,4 81,3 76,11 84,9 80,16 75,18 70,22 64,20 58,25 51,23 42,27 37,24 29,27 28,22 22,27"),
    )


def filter_dorsal_skin(brush):
    """Band-limit the pigment at its actual in-game footprint; keep the cutout edge."""
    import numpy as np
    premultiplied=brush.convert("RGBa")
    def band(radius):
        # The rig places this painting on a 74 x 56 cutout at (1.24, 1.025) scale.
        footprint=(radius*brush.width/(74*1.24),radius*brush.height/(56*1.025))
        ink=premultiplied.filter(ImageFilter.GaussianBlur(footprint)).convert("RGBA")
        return np.asarray(ink,dtype='f4')[...,:3]
    fine=band(.73)
    broader=band(1.75)
    # Preserve relief from larger warts and folds independently of the fine
    # bright grains that scintillate when the sprite moves across screen pixels.
    rgb=np.rint(np.clip(fine+.26*(fine-broader),0,255)).astype('u1')
    result=Image.fromarray(rgb)
    result.putalpha(brush.getchannel("A"))
    return result


def torso_paint(repo, out):
    """Paint a periodic 2D strip in the order flank, chest, flank, back."""
    import numpy as np
    artwork=repo/"art/creatures/marsh-turn"
    cuts={
        "toad-wrap":(0,("side",(.36,.28,.65,.62)),("front",(.35,.48,.65,.74)),("back",(.36,.20,.67,.63))),
        "keeper-wrap":(3,("side",(.52,.25,.70,.47)),("front",(.43,.30,.59,.62)),("back",(.37,.21,.52,.48))),
    }
    for name,(species,flank,chest,back) in cuts.items():
        def fragment(spec):
            heading,box=spec
            with Image.open(artwork/f"{species}-{heading}.png") as source:
                x0,y0,x1,y1=box
                area=source.convert("RGBA").crop((round(x0*source.width),round(y0*source.height),
                                                  round(x1*source.width),round(y1*source.height)))
            if area.getchannel("A").getextrema()[0]<240:
                raise ValueError(f"Transparent source in torso strip {species}/{heading}")
            return np.asarray(area.convert("RGB").resize((256,192),Image.Resampling.LANCZOS),dtype='f4')
        side=fragment(flank)
        front=fragment(chest)
        if species==0:
            # The front-facing toad still has a warty crown. The pale
            # throat appears towards the bottom where the snout overlaps it.
            t=np.clip((np.arange(front.shape[0],dtype='f4')/front.shape[0]-.23)/.35,0,1)
            t=(t*t*(3-2*t))[:,None,None]
            front=side*.95*(1-t)+front*t
        patterns=(side,front,side[:,::-1],fragment(back))
        sheet=np.empty((192,1024,3),dtype='f4')
        for x in range(sheet.shape[1]):
            angle=x/256.
            sector=math.floor(angle+.5)
            local=angle-sector
            u=min(255,round((local+.5)*255))
            paint=patterns[sector%4][:,u]
            # Weld the edges in texture space. This stationary seam follows the body.
            t=max(0.,(abs(local)-.38)/.12)
            mix=.5*t*t*(3-2*t)
            if mix:
                near=patterns[(sector+(1 if local>0 else -1))%4]
                paint=paint*(1-mix)+near[:,255-u]*mix
            sheet[:,x]=paint
        pixels=np.rint(sheet).clip(0,255).astype('u1')
        # A padded copy at each end gives the bake and Blender identical seam filtering.
        periodic=np.concatenate((pixels,pixels),axis=1)
        Image.fromarray(periodic).save(out/f"{name}.png")


def cut(repo, out):
    out.mkdir(parents=True, exist_ok=True)
    for name, species, direction, polygon in pieces():
        source = Image.open(repo / "art/creatures/marsh-turn" / f"{species}-{direction}.png").convert("RGBA")
        w, h = source.size
        points = [tuple(map(float, pair.split(","))) for pair in polygon.split()]
        mask = Image.new("L", (w, h))
        ImageDraw.Draw(mask).polygon([(x*w/100, y*h/100) for x, y in points], fill=255)
        from PIL import ImageChops
        source.putalpha(ImageChops.multiply(mask.filter(ImageFilter.GaussianBlur(1.7)), source.getchannel("A")))
        box = source.getchannel("A").getbbox()
        brush=source.crop(box)
        if name=="toad-back":
            brush=filter_dorsal_skin(brush)
        brush.save(out / (name+".png"))
    torso_paint(repo,out)
    # A stripped root cut from the keeper's own bark, with bound ends. Sample
    # the interior along the sloping root so a second hand is not painted on it.
    root = Image.open(out / "keeper-upper.png").convert("RGBA")
    w,h=root.size
    timber=Image.new("RGBA",(40,256))
    for y in range(256):
        source_y=round(h*(.12+.74*y/255))
        interior=[x for x in range(w) if root.getpixel((x,source_y))[3]>245]
        if not interior:
            raise ValueError("Missing bark for the yoke")
        lo,hi=min(interior),max(interior)
        edge=3+int(1.2*math.sin(y*.062)+1.1*math.cos(y*.11))
        for x in range(edge,40-edge):
            u=(x-edge)/(39-2*edge)
            row=root.getpixel((round(lo+(hi-lo)*(.16+.68*u)),source_y))
            light=.71+.31*math.sin(math.pi*u)
            rgb=tuple(round(v*light) for v in row[:3])
            if y in range(20,28) or y in range(224,232):
                rgb=tuple(round(v*light) for v in (148,107,60))
            timber.putpixel((x,y),(*rgb,255))
    timber.save(out / "keeper-yoke.png")


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--repo", type=Path, required=True)
    p.add_argument("--output", type=Path, required=True)
    a = p.parse_args()
    cut(a.repo, a.output)

"""Paint the observatory masonry and Sporekin's rear view in local Krita."""
import argparse
import hashlib
import json
import math
from pathlib import Path
from random import Random

from PIL import Image, ImageDraw, ImageFilter

from krita_mcp_client import KritaMCP, start_krita_if_needed
from regional_enemy_visuals import (
    ANCHOR, CLIPS, PAD, SIZE, STRIDE, author_clip, split_parts, synfig_cli,
)


WALL_LEFT, WALL_TOP, WALL_WIDTH, WALL_HEIGHT = -409, -510, 554, 480
STONE_LEFT, STONE_RIGHT, WALL_RISE = -392, 113, 290
SPORE_SOURCE = Path("C:/AI2D/sprites/region-enemies-20261111/sporekin.png")


def footing(x):
    return 4-7.2*28.2843*math.sqrt(max(0,1-(x/(7.2*56.5685))**2))


def canvas_point(x,y):
    return (round((x-WALL_LEFT)*2),round((y-WALL_TOP)*2))


def wall_base(path):
    image=Image.new("RGBA",(WALL_WIDTH*2,WALL_HEIGHT*2))
    draw=ImageDraw.Draw(image)
    rng=Random(4175)

    def stone(x1,x2,low,high,color):
        steps=max(1,math.ceil((x2-x1)/8))
        curve=[x1+(x2-x1)*i/steps for i in range(steps+1)]
        polygon=[canvas_point(x,footing(x)-high) for x in curve]
        polygon += [canvas_point(x,footing(x)-low) for x in reversed(curve)]
        draw.polygon(polygon,fill=color)

    stone(STONE_LEFT,STONE_RIGHT,0,WALL_RISE,(34,42,55))
    # Stagger the joints and follow the floor ellipse on every course.
    for row in range(12):
        x=STONE_LEFT
        top=min(WALL_RISE,(row+1)*25)
        while x<STONE_RIGHT:
            end=min(STONE_RIGHT,x+rng.randint(43,74))
            low=row*25
            light=rng.randint(-5,6)+int(7*math.sin((x+275)*.006))
            stone(x+.8,end-.7,low+.6,top-.7,(61+light,72+light,86+light))
            stone(x+1.1,end-1.2,top-3,top-1,(79+light,90+light,103+light))
            stone(x+1.1,end-1.2,low+1,low+2.5,(47+light,55+light,67+light))
            x=end

    for low,high,color in [
        (0,9,"#3c4b60"),(9,12,"#708294"),(12,16,"#46566b"),
        (265,271,"#46566b"),(271,274,"#798a97"),
        (281,291,"#4b5a70"),(291,296,"#768596"),(296,299,"#9aa6a9"),
    ]:
        stone(STONE_LEFT-3,STONE_RIGHT+3,low,high,color)
    for x in (STONE_LEFT+6,STONE_RIGHT-5):
        stone(x-10,x+10,3,287,"#54667a")
        stone(x-9,x-5,9,283,"#8393a0")
        stone(x+5,x+10,9,283,"#3e4c60")
        for h in range(25,280,25):
            stone(x-9,x+10,h-1,h+1,"#354152")
    image.save(path)


def spore_back_base(path):
    source=Image.open(SPORE_SOURCE).convert("RGBA")
    # Reuse clean stem material to cover the face. Keep the original cap,
    # feet, limbs and silhouette so their anchors stay registered.
    donor=source.crop((220,372,355,423)).resize((141,132),Image.Resampling.BICUBIC)
    coverage=Image.new("L",donor.size)
    ImageDraw.Draw(coverage).rounded_rectangle((4,1,136,127),radius=18,fill=255)
    coverage=coverage.filter(ImageFilter.GaussianBlur(2))
    source.paste(donor,(207,282),coverage)
    source.putalpha(Image.open(SPORE_SOURCE).getchannel("A"))
    source.save(path)


def paint(work):
    work.mkdir(exist_ok=False)
    wall=work/"wall-base.png"
    rear=work/"sporekin-back-base.png"
    wall_base(wall)
    spore_back_base(rear)
    start_krita_if_needed()
    with KritaMCP(work/"krita-logs") as client:
        for name,source in (("tower-observatory-wall",wall),("sporekin-back",rear)):
            doc=client.call("krita_document",action="open",path=str(source))["affected_document_id"]
            state=client.call("krita_state",document_id=doc)
            if state["runtime_compatibility"]["status"]!="compatible":
                raise RuntimeError(state["runtime_compatibility"])
            layer=client.call("krita_layers",action="create",document_id=doc,
                              name="Krita · patyna i krawędzie")["active_layer_id"]
            marks=[]
            if name=="tower-observatory-wall":
                for x in range(-367,97,27):
                    h=33+(abs(x)*11)%217
                    points=[canvas_point(x,footing(x)-h),canvas_point(x+6,footing(x+6)-h-2),
                            canvas_point(x+15,footing(x+15)-h)]
                    marks.append(("#798896",1.6,points))
                for x in (STONE_LEFT+6,STONE_RIGHT-5):
                    foot=footing(x)
                    marks.append(("#a0acb2",2.4,[canvas_point(x-6,foot-10),canvas_point(x-6,foot-272)]))
            else:
                marks=[
                    ("#584331",5,[(284,287),(279,315),(287,340),(280,368),(286,399),(282,423)]),
                    ("#9b7855",2.8,[(288,289),(284,316),(292,338),(284,368),(291,399),(287,422)]),
                    ("#735237",3,[(244,296),(241,319),(247,344),(242,375)]),
                    ("#836349",2.5,[(320,305),(312,333),(318,357),(310,387)]),
                ]
            for color,width,points in marks:
                client.call("krita_draw",shape="path",document_id=doc,layer_id=layer,
                            points=[{"x":x,"y":y} for x,y in points],foreground=color,
                            fill_style="None",stroke_style="ForegroundColor",brush_size=width)
            client.call("krita_preview",document_id=doc,max_width=850,max_height=650)
            client.call("krita_save_export",document_id=doc,kra_path=str(work/f"{name}.kra"),
                        png_path=str(work/f"{name}.png"),overwrite=False)
            client.call("krita_document",action="close",document_id=doc,discard_changes=False)


def bake_spore(name,source,folder,destination):
    (folder/"parts").mkdir(parents=True)
    sprite=Image.open(source).convert("RGBA")
    x0,y0,x1,y1=sprite.getchannel("A").point(lambda a:255 if a>=55 else 0).getbbox()
    scale=min(94/(x1-x0),73/(y1-y0))

    def fit(x,y):
        return ANCHOR[0]+(x-(x0+x1)/2)*scale,ANCHOR[1]+(y-y1)*scale

    parts=[]
    for part,layer,pivot in split_parts("sporekin",sprite):
        bounds=layer.getbbox()
        layer.crop(bounds).save(folder/"parts"/f"{part}.png")
        parts.append((part,bounds,pivot))
    atlas=Image.new("RGBA",(STRIDE*8,STRIDE*6))
    n=0
    for clip,count in CLIPS.items():
        project=folder/f"{clip}.sif"
        author_clip(project,"sporekin",clip,parts,fit)
        rendered=synfig_cli.render(project,threads=2,expected_frames=count,require_motion=True)
        for file in sorted(Path(rendered["OutputDirectory"]).glob("frame*.png")):
            frame=Image.open(file).convert("RGBA")
            box=frame.getchannel("A").point(lambda a:255 if a>=45 else 0).getbbox()
            assert box and min(box[:2])>0 and max(box[2:])<SIZE,(name,clip,box)
            atlas.paste(frame,((n%8)*STRIDE+PAD,(n//8)*STRIDE+PAD))
            n+=1
    assert n==sum(CLIPS.values())
    atlas.save(destination)


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    master=Path("C:/AI2D/sprites")/args.run.name
    animation=Path("C:/AI2D/animations")/args.run.name
    paint(master)
    delivery=args.run/"delivery"
    delivery.mkdir()
    for name,source in (("sporekin",SPORE_SOURCE),("sporekin-back",master/"sporekin-back.png")):
        bake_spore(name,source,animation/name,delivery/f"{name}.png")
    wall=delivery/"tower-observatory-wall.png"
    wall.write_bytes((master/wall.name).read_bytes())
    hashes={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in delivery.iterdir()}
    (args.run/"art-receipt.json").write_text(json.dumps(hashes,indent=2)+"\n",encoding="utf-8")

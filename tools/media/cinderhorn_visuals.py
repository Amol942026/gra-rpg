"""Finish the two-view Rogacz painting in Krita and bake articulated Synfig sprites."""
from __future__ import annotations

import argparse
import io
import json
import math
from pathlib import Path
from xml.etree import ElementTree as ET
from zipfile import ZipFile

from PIL import Image, ImageChops, ImageFilter

from hero_synfig import group, image, rotate, value
from krita_mcp_client import KritaMCP
from regional_enemy_visuals import ANCHOR, CLIPS, PAD, SIZE, STRIDE, animated_layer, digest, masked, polygon_mask, pulse
import synfig_cli

# Coordinates are in the original concept, with each view 887 pixels wide.
# Parts are listed back-to-front. Mask overlap keeps the joints covered.
RIG = {
    "front": [
        ("hind-far", (269, 484), [(234,432),(330,447),(357,567),(333,623),(358,690),(228,704),(216,580)]),
        ("fore-far", (588, 520), [(539,453),(662,450),(674,529),(658,614),(767,793),(595,804),(563,686),(529,571)]),
        ("hind-near", (150, 468), [(128,432),(210,453),(212,513),(164,571),(134,631),(178,759),(31,766),(24,632),(75,527)]),
        ("fore-near", (385, 508), [(334,464),(429,467),(466,546),(476,668),(465,727),(514,849),(352,860),(352,721),(331,644),(299,545)]),
        ("head", (619, 367), [(241,10),(706,8),(750,221),(813,327),(829,473),(726,503),(685,528),
            (664,552),(611,574),(554,558),(553,487),(568,434),(579,362),(610,315),(533,276),
            (484,231),(443,192),(383,164),(316,151),(241,174)]),
    ],
    "back": [
        ("hind-far", (211, 493), [(162,440),(258,471),(238,559),(212,635),(245,815),(78,819),(75,658),(110,548)]),
        ("fore-far", (468, 478), [(412,432),(493,442),(532,532),(508,577),(525,688),(404,688),(405,564)]),
        ("hind-near", (325, 542), [(280,493),(399,514),(400,589),(371,653),(354,725),(407,876),(239,876),(232,721),(242,625)]),
        ("fore-near", (605, 465), [(560,419),(674,435),(682,535),(665,593),(746,737),(586,750),(585,634),(549,538)]),
        ("head", (689, 257), [(335,0),(864,0),(866,267),(782,275),(754,341),(697,359),
            (649,353),(635,309),(645,257),(660,205),(552,203),(478,162),(392,212),(341,209)]),
    ],
}

# Screen coordinates match the direction order in AnimationState. The two
# original paintings provide opposite elevations. Reflections cover the west.
WALK_DIRECTIONS = (("front",0),("front",45),("front",90),("back",270),("back",315))
HOOF = {
    "front": {"hind-far":(304,674), "fore-far":(705,782),
              "hind-near":(112,738), "fore-near":(468,841)},
    "back": {"hind-far":(156,791), "fore-far":(474,674),
             "hind-near":(345,864), "fore-near":(679,730)},
}
FOOTFALLS = {"hind-near":0, "fore-near":.25, "hind-far":.5, "fore-far":.75}
WALK_FRAMES, WALK_STRIDE, WALK_SUPPORT = 32, 35, .625
CHARGE_FRAMES, CHARGE_STRIDE, CHARGE_SUPPORT = 16, 73, .33


def split(view, sprite):
    pieces = {}
    removed = Image.new("L", sprite.size)
    for name, pivot, coords in RIG[view]:
        mask = polygon_mask(sprite.size, coords)
        pieces[name] = (masked(sprite, mask.filter(ImageFilter.MaxFilter(9))), pivot)
        removed = ImageChops.lighter(removed, mask)
    pieces["body"] = (masked(sprite, ImageChops.invert(removed.filter(ImageFilter.MinFilter(41)))),
                      (425 if view == "front" else 467, 475))
    order = ["hind-far", "fore-far", "body", "hind-near", "fore-near", "head"]
    return [(name, *pieces[name]) for name in order]


def paint(art, run, view):
    concept = Image.open(art/"concept.png").convert("RGBA")
    half = concept.width//2
    left = 0 if view=="front" else half
    sprite = concept.crop((left,0,left+half,concept.height))
    grain = Image.open(art/"basalt-grain.png").convert("L").resize(sprite.size,Image.Resampling.LANCZOS)
    detail = ImageChops.subtract(grain,grain.filter(ImageFilter.GaussianBlur(13)),offset=128)
    neutral = Image.new("L",sprite.size,128)
    # A low-strength mineral texture follows the original painted silhouette.
    channels = [ImageChops.add(channel,Image.blend(neutral,detail,.18),offset=-128)
                for channel in sprite.split()[:3]]
    sprite = Image.merge("RGBA",(*channels,sprite.getchannel("A")))
    parts = split(view, sprite)
    ora = art/f"{view}.ora"
    stack = ET.Element("image", w=str(sprite.width), h=str(sprite.height), version="0.0.3")
    layers = ET.SubElement(stack, "stack")
    with ZipFile(ora, "x") as archive:
        archive.writestr("mimetype", "image/openraster")
        for name, layer, _ in reversed(parts):
            path = f"data/{name}.png"
            ET.SubElement(layers, "layer", name=name, src=path, x="0", y="0", opacity="1.0",
                          visibility="visible", **{"composite-op": "svg:src-over"})
            with io.BytesIO() as buffer:
                layer.save(buffer, format="PNG")
                archive.writestr(path, buffer.getvalue())
        archive.writestr("stack.xml", ET.tostring(stack, encoding="utf-8"))
        with io.BytesIO() as buffer:
            sprite.save(buffer, format="PNG")
            archive.writestr("mergedimage.png", buffer.getvalue())
    with KritaMCP(run/"krita"/view) as krita:
        document = krita.call("krita_document", action="open", path=str(ora))["affected_document_id"]
        state = krita.call("krita_state", document_id=document)
        if state["runtime_compatibility"]["status"] != "compatible":
            raise RuntimeError(state["runtime_compatibility"])
        accent = krita.call("krita_layers", action="create", document_id=document,
                            name="Odbicia lawy na pancerzu")["active_layer_id"]
        marks = {
            "front": [
                ("#d97b48", [(437,395),(453,411),(473,423),(482,439)]),
                ("#f0a060", [(153,407),(169,397),(184,388)]),
                ("#bf9271", [(573,250),(586,266),(601,270)]),
            ],
            "back": [
                ("#dd824a", [(341,397),(359,393),(377,397),(391,409)]),
                ("#d89261", [(688,317),(703,326),(715,338)]),
                ("#bf9271", [(599,81),(621,74),(640,71)]),
            ],
        }
        for color, points in marks[view]:
            inside = [{"x": x, "y": y} for x, y in points if sprite.getpixel((x,y))[3] > 210]
            if len(inside) >= 2:
                krita.call("krita_draw", document_id=document, layer_id=accent, shape="path", points=inside,
                           foreground=color, fill_style="None", stroke_style="ForegroundColor", brush_size=3)
        krita.call("krita_save_export", document_id=document, kra_path=str(art/f"{view}.kra"),
                   png_path=str(art/f"{view}.png"), overwrite=False)
        krita.call("krita_preview", document_id=document, max_width=384, max_height=384)


def lowered_head(t):
    return (-6*t, -16*t, -35*t)


def motion(part, clip, t):
    angle = math.tau*t
    leg = "-" in part
    fore = part.startswith("fore")
    near = part.endswith("near")
    if clip == "idle":
        breath = math.sin(angle)
        return (0, .6*breath, 1.8*breath) if part == "head" else (0, 0 if leg else .8*breath, 0)
    if clip == "move":
        if leg:
            phase = angle + (0 if fore == near else math.pi) + (0 if fore else .28)
            stride = math.sin(phase)
            return (2.8*stride, max(0, math.cos(phase))*3.9, -stride*(8.7 if near else 7))
        return (.9*math.sin(angle), 1.6*math.cos(angle*2), 2.7*math.sin(angle*2) if part=="head" else 1.2*math.sin(angle))
    if clip == "attack":
        if t < .45:
            draw = min(1,t/.45)
            draw = draw*draw*(3-2*draw)
            drive = 0
        else:
            draw = max(0,1-(t-.64)/.36)
            drive = pulse(t,.68)*min(1,(t-.45)/.20)
        if part == "head":
            x,y,angle = lowered_head(draw)
            return (x+20*drive, y+6*drive, angle+13*drive)
        if leg:
            return ((10 if fore else 2)*drive, (4 if fore else 1.5)*drive, -11*drive+3*draw)
        return (7*drive-2.7*draw, -3.6*draw+drive, -4.8*draw-3.5*drive)
    if clip in ("windup","recovery","impact"):
        q = t*t*(3-2*t)
        crouch = q if clip=="windup" else 1-q
        impact = math.sin(math.pi*min(1,t/.65)) if clip=="impact" else 0
        if part == "head":
            x,y,angle = lowered_head(crouch)
            return (x+17*impact,y-4*impact,angle+15*impact)
        if leg:
            return ((-2 if fore else 3)*crouch+(7 if fore else -1)*impact,
                    2.5*math.sin(math.pi*t) if fore else 0, (6 if fore else -4)*crouch-10*impact)
        return (-3*crouch+6*impact,-3.7*crouch+1.2*impact,-4.8*crouch-3.5*impact)
    if clip == "hit":
        recoil = pulse(t,.26)
        return (-3.8*recoil, 2.4*recoil, (11 if part=="head" else -3 if leg else 4)*recoil)
    settle = t*t*(3-2*t)
    if leg:
        return ((7 if fore else -5)*settle, 11.5*settle, (28 if fore else -22)*settle)
    return (6*settle, -9*settle if part=="head" else 0, -18*settle if part=="head" else -8*settle)


def author(path, view, clip, parts, fit, count=None):
    count = count if count is not None else CLIPS[clip]
    loop = clip in ("idle","move")
    times = [i/(count if loop else count-1) for i in range(count+int(loop))]
    canvas = ET.Element("canvas", version="1.2", width=str(SIZE), height=str(SIZE), fps="24",
                        **{"view-box":"-2 2 2 -2", "antialias":"2", "begin-time":"0f",
                           "end-time":f"{count-1}f", "bgcolor":"0 0 0 0"})
    ET.SubElement(canvas, "name").text = f"Emberfall / Rogacz Żaru / {view} / {clip}"
    rig = group(canvas, "Cztery nogi, tułów i łeb")

    def point(x,y):
        px,py = fit(x,y)
        return (px-72)/36, (72-py)/36

    for part, box, origin in parts:
        drawing = group(rig, part)
        image(drawing, part, (*point(*box[:2]),*point(*box[2:])))
        keys = [motion(part,clip,t) for t in times]
        if view == "back" and part == "head" and clip in ("attack","windup","recovery","impact"):
            keys = [(x,y*.82,a*.87) for x,y,a in keys]
        rotate(drawing, part, point(*origin), [a for _,_,a in keys])
        animated_layer(drawing,"translate","origin","vector",[(x/36,y/36) for x,y,_ in keys])
    if clip == "fall":
        t = [p*p*(3-2*p) for p in times]
        collapse = animated_layer(rig,"stretch","amount","vector",[(1+.05*p,1-.47*p) for p in t])
        value(collapse,"center","vector",((ANCHOR[0]-72)/36,(72-ANCHOR[1])/36))
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def bake(art, run, animation, view, base=None):
    folder = animation/view
    (folder/"parts").mkdir(parents=True)
    painted = Image.open(art/f"{view}.png").convert("RGBA")
    x0,y0,x1,y1 = painted.getchannel("A").point(lambda a:255 if a>=40 else 0).getbbox()
    scale = min(92/(y1-y0), 97/(x1-x0))

    def fit(x,y):
        return ANCHOR[0]+(x-(x0+x1)/2)*scale, ANCHOR[1]+(y-y1)*scale

    parts = []
    for name, layer, origin in split(view,painted):
        box = layer.getbbox()
        if not box:
            raise ValueError(f"Empty {view} part {name}")
        layer.crop(box).save(folder/"parts"/f"{name}.png")
        parts.append((name,box,origin))
    name = "cinderhorn" if view=="front" else "cinderhorn-back"
    atlas = Image.open(base/f"{name}.png").convert("RGBA") if base else Image.new("RGBA",(8*STRIDE,6*STRIDE))
    clips = []
    frame = 0
    for clip,count in CLIPS.items():
        if base and clip!="attack":
            frame+=count
            continue
        source = folder/f"{clip}.sif"
        author(source,view,clip,parts,fit)
        manifest = synfig_cli.render(source,output_root=folder/"renders",threads=2,
                                     expected_frames=count,require_motion=True)
        images = sorted(Path(manifest["OutputDirectory"]).glob("frame*.png"))
        for picture in images:
            sprite = Image.open(picture).convert("RGBA")
            box = sprite.getchannel("A").point(lambda a:255 if a>=40 else 0).getbbox()
            if not box or min(box[:2])<2 or max(box[2:])>SIZE-2:
                raise ValueError(f"Clipped frame {picture}: {box}")
            atlas.paste(sprite,((frame%8)*STRIDE+PAD,(frame//8)*STRIDE+PAD))
            frame += 1
        clips.append({"clip":clip,"frames":count,"unique":manifest["UniquePixelFrames"],
                      "seconds":manifest["ElapsedSeconds"],"manifest":manifest["OutputDirectory"]+"/render-manifest.json"})
        print(view, clip, count, flush=True)
    output = run/("cinderhorn.png" if view=="front" else "cinderhorn-back.png")
    atlas.save(output)
    if base:
        phases = Image.new("RGBA",(8*STRIDE,4*STRIDE))
        for section,(clip,count) in enumerate((("windup",12),("recovery",8),("impact",12))):
            source = folder/f"{clip}.sif"
            author(source,view,clip,parts,fit,count)
            manifest = synfig_cli.render(source,output_root=folder/"renders",threads=2,
                                         expected_frames=count,require_motion=True)
            offset = (0,12,20)[section]
            for index,picture in enumerate(sorted(Path(manifest["OutputDirectory"]).glob("frame*.png"))):
                sprite = Image.open(picture).convert("RGBA")
                box = sprite.getchannel("A").point(lambda a:255 if a>=40 else 0).getbbox()
                if not box or min(box[:2])<2 or max(box[2:])>SIZE-2:
                    raise ValueError(f"Clipped frame {picture}: {box}")
                frame=offset+index
                phases.paste(sprite,((frame%8)*STRIDE+PAD,(frame//8)*STRIDE+PAD))
            clips.append({"clip":clip,"frames":count,"unique":manifest["UniquePixelFrames"],
                          "seconds":manifest["ElapsedSeconds"],"manifest":manifest["OutputDirectory"]+"/render-manifest.json"})
        phases.save(run/f"cinderhorn-charge-{view}.png")
    master = art/f"{view}.kra"
    with ZipFile(master) as native:
        if native.testzip() is not None:
            raise ValueError(f"Invalid Krita master {master}")
    (run/f"{view}-receipt.json").write_text(json.dumps({"status":"PASS","atlas":str(output),
        "sha256":digest(output),"master":str(master),"master_sha256":digest(master),
        "concept_sha256":digest(art/"concept.png"),"comfy_sha256":digest(art/"basalt-grain.png"),
        "clips":clips},indent=2),encoding="utf-8")


def hoof_motion(phase, touchdown, stride=WALK_STRIDE, support=WALK_SUPPORT, clearance=10.5):
    t = (phase-touchdown)%1
    half = stride*support*.5
    if t <= support:
        return half-stride*t, 0
    s = (t-support)/(1-support)
    # Ground-relative backward travel exactly cancels root motion in support.
    # The foot accelerates on lift, clears the ground, and settles at contact.
    return half*(2*s*s*(3-2*s)-1), clearance*math.sin(math.pi*s)**1.35


def bake_walk(art, run, animation, direction, rushing=False):
    view, degrees = WALK_DIRECTIONS[direction]
    heading = math.cos(math.radians(degrees)), math.sin(math.radians(degrees))
    slug = "charge" if rushing else "walk"
    frames,stride,support = (CHARGE_FRAMES,CHARGE_STRIDE,CHARGE_SUPPORT) if rushing else (WALK_FRAMES,WALK_STRIDE,WALK_SUPPORT)
    folder = animation/f"{slug}-{direction}"
    (folder/"parts").mkdir(parents=True)
    sprite = Image.open(art/f"{view}.png").convert("RGBA")
    x0,y0,x1,y1 = sprite.getchannel("A").point(lambda a:255 if a>=40 else 0).getbbox()
    scale = min(92/(y1-y0),97/(x1-x0))
    center = (x0+x1)*.5
    # Remove the remaining pixels below the hip overlap from the static body.
    parts = split(view,sprite)
    for index,(name,part,hinge) in enumerate(parts):
        if name!="body":
            continue
        cutoff = (485,520,550,590,560,515) if view=="front" else (522,573,565,524,430,351)
        xs = (0,158,315,515,696,887)
        outline = [(0,0),(sprite.width,0),*reversed(list(zip(xs,cutoff)))]
        parts[index] = name,masked(part,polygon_mask(sprite.size,outline)),hinge

    def fit(x,y):
        return ANCHOR[0]+(x-center)*scale,ANCHOR[1]+(y-y1)*scale

    def units(x,y):
        return (x-72)/36,(72-y)/36

    canvas = ET.Element("canvas",version="1.2",width=str(SIZE),height=str(SIZE),fps="60",
                        **{"view-box":"-2 2 2 -2","antialias":"2","begin-time":"0f",
                           "end-time":f"{frames-1}f","bgcolor":"0 0 0 0"})
    ET.SubElement(canvas,"name").text=f"Emberfall / Rogacz Żaru / {slug} / {direction}"
    rig=group(canvas,"Cztery niezależne kopyta")
    phase=[i/frames for i in range(frames+1)]
    tracks={}

    # Create each leg's transforms around the shoulder/hip. Local scaling and
    # rotation map the original hoof to its exact stance or swing target.
    for name,part,pivot in parts:
        box=part.getbbox()
        part.crop(box).save(folder/"parts"/f"{name}.png")
        drawing=group(rig,name)
        image(drawing,name,(*units(*fit(*box[:2])),*units(*fit(*box[2:]))))
        joint=fit(*pivot)
        if name not in FOOTFALLS:
            bob=[(2.6 if rushing else .82)*math.cos(math.tau*2*t) for t in phase]
            dx,dy,tilt = lowered_head(.78) if rushing and name=="head" else (0,-2.5 if rushing else 0,0)
            if name=="head":
                rotate(drawing,name,units(*joint),[tilt+2.3*math.sin(math.tau*2*t+.6) for t in phase])
            animated_layer(drawing,"translate","origin","vector",[(dx/36,(y+dy)/36) for y in bob])
            continue
        hoof=fit(*HOOF[view][name])
        baseline=hoof[0]-joint[0],hoof[1]-joint[1]
        rest_angle=math.atan2(baseline[1],baseline[0])
        rest_length=math.hypot(*baseline)
        footfall=({"hind-near":0,"hind-far":.14,"fore-near":.50,"fore-far":.65} if rushing else FOOTFALLS)[name]
        angles,stretch,translation,keys=[],[],[],[]
        for t in phase:
            along,lift=hoof_motion(t,footfall,stride,support,15 if rushing else 10.5)
            x=along*heading[0]
            y=along*heading[1]-lift
            bob=(2.6 if rushing else .82)*math.cos(math.tau*2*t)
            target=baseline[0]+x,baseline[1]+y+bob
            delta=math.atan2(math.sin(math.atan2(target[1],target[0])-rest_angle),
                             math.cos(math.atan2(target[1],target[0])-rest_angle))
            angles.append(-math.degrees(delta))
            ratio=math.hypot(*target)/rest_length
            stretch.append((ratio,ratio))
            translation.append((0,bob/36))
            keys.append({"x":round(hoof[0]+x,5),"y":round(hoof[1]+y,5),"along":round(along,5),"lift":round(lift,5)})
        layer=animated_layer(drawing,"stretch","amount","vector",stretch)
        value(layer,"center","vector",units(*joint))
        rotate(drawing,name,units(*joint),angles)
        animated_layer(drawing,"translate","origin","vector",translation)
        tracks[name]=keys

    ET.indent(canvas)
    project=folder/f"{slug}.sif"
    ET.ElementTree(canvas).write(project,encoding="utf-8",xml_declaration=True)
    rendered=synfig_cli.render(project,output_root=folder/"renders",threads=2,
                               expected_frames=frames,require_motion=True)
    atlas=Image.new("RGBA",(8*STRIDE,(frames//8)*STRIDE))
    for frame,file in enumerate(sorted(Path(rendered["OutputDirectory"]).glob("frame*.png"))):
        cell=Image.open(file).convert("RGBA")
        box=cell.getchannel("A").point(lambda a:255 if a>=40 else 0).getbbox()
        if not box or min(box[:2])<2 or max(box[2:])>SIZE-2:
            raise ValueError(f"Clipped frame {file}: {box}")
        atlas.paste(cell,((frame%8)*STRIDE+PAD,(frame//8)*STRIDE+PAD))
    atlas_path=run/f"cinderhorn-{slug}-{direction}.png"
    atlas.save(atlas_path)
    (run/f"{slug}-{direction}.json").write_text(json.dumps({"atlas":str(atlas_path),
        "sha256":digest(atlas_path),"project":str(project),"frames":frames,
        "seconds":rendered["ElapsedSeconds"],"stride":stride,"support":support,
        "tracks":tracks},indent=2),encoding="utf-8")
    print(slug,direction,frames,flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action",choices=("paint","bake","walk","charge"))
    parser.add_argument("--art",type=Path,required=True)
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--animation",type=Path,required=True)
    parser.add_argument("--view",choices=tuple(RIG))
    parser.add_argument("--base-atlases",type=Path)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    if args.action=="walk":
        for direction in range(len(WALK_DIRECTIONS)):
            bake_walk(args.art,args.run,args.animation,direction)
        raise SystemExit
    if args.action=="charge":
        if not args.base_atlases:
            parser.error("--base-atlases is required for charge")
        for view in RIG:
            bake(args.art,args.run,args.animation,view,args.base_atlases)
        for direction in range(len(WALK_DIRECTIONS)):
            bake_walk(args.art,args.run,args.animation,direction,True)
        raise SystemExit
    for view in (args.view,) if args.view else RIG:
        if args.action=="paint":
            paint(args.art,args.run,view)
        else:
            bake(args.art,args.run,args.animation,view)

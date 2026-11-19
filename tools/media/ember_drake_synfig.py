"""Author the eight-facing ember drake cutout in Synfig and compile its saved transforms."""

import argparse
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image

from ember_drake_jaw import LEVELS, ORIGINS, TILE
from hero_hit_reaction_synfig import samples
from hero_synfig import group, image, value
from wolf_motion_synfig import walking_foot
import synfig_cli


FACINGS = (0, math.atan(.5), math.pi/2, math.pi-math.atan(.5),
           math.pi, math.pi+math.atan(.5), 1.5*math.pi, math.tau-math.atan(.5))
NAMES = ("E", "SE", "S", "SW", "W", "NW", "N", "NE")
PARTS = ("far-wing", "feet-a", "feet-b", "trunk-tail", "near-wing", "neck-skull")
GRID = 2.5
MARGIN = .25
# Landmarks in the three-quarter and profile paintings, measured in 128-unit cells.
# Far wing, near wing, neck, lips. The complementary directions use mirrored paintings.
LANDMARKS = {
    0: ((76, 78), (83, 80), (90, 82), (119, 79)),
    1: ((80, 56), (70, 64), (84, 74), (116, 77)),
    2: ((51, 68), (77, 68), (64, 68), (64, 71)),
    6: ((51, 70), (76, 70), (64, 51), (64, 36)),
    7: ((76, 61), (83, 78), (105, 61), (115, 52)),
}
CLIPS = {
    "idle": (24, list(range(0, 49, 4))),
    "walk": (60, list(range(0, 41, 2))),
    "bite": (60, [0, 5, 10, 15, 20, 23, 25, 27, 30, 34, 39, 45, 52, 60]),
    "breath": (20, [0, 4, 9, 13, 16, 18, 19, 21, 24, 26, 28, 31, 33, 35, 38, 40, 44, 48, 53]),
    "fall": (52, list(range(12))),
    "bank": (24, [0, 4, 8, 12, 16]),
}
# Shift, rotation and scale about each anatomical attachment at ground contact.
REST = ((1, 4, 9, .97, .73), (2, -6, -15, 1, .91), (-2, -7, 17, 1, .91),
        (0, 0, -9, 1.055, 1.22), (2, 5, -10, 1.03, .72), (3, 6, 9, 1.02, 1.02))


def landmark(direction, index):
    source = {3: 1, 4: 0, 5: 7}.get(direction, direction)
    x, y = LANDMARKS[source][index]
    return (128-x if source != direction else x), y


def pivot(part, direction):
    if part in (0, 4, 5):
        return landmark(direction, (0, 4, 5).index(part))
    if part in (1, 2):
        f, s = math.cos(FACINGS[direction]), math.sin(FACINGS[direction])
        return 64-f*2, 84-s
    return 64, 84


def order(direction):
    return (0, 1, 2, 5, 3, 4) if direction >= 5 else (0, 1, 2, 3, 4, 5)


def ease(t):
    x = min(1., max(0., t))
    return x*x*(3-2*x)


# Positive values lift or advance the head, flare wings, and advance the breast.
# The bite lands at normalized .45. At 20 fps the fire pulses land on 19, 26 and 33.
BITE = ((0, 0, 0, 0, 0), (14, -1.2, -2.4, 2, -.9), (22, -4, -5.4, 10, -2.6),
        (25, -.4, .6, 6, .6), (27, 1.4, 7.8, -4.5, 3.9), (30, -.2, 8, -6, 3.5),
        (35, -1.2, 4.3, -1.8, 1.8), (43, .8, -.8, 2, -.4), (60, 0, 0, 0, 0))
BREATH = ((0, 0, 0, 0, 0), (9, 2, -1.8, 5, -.7), (16, 4.8, -4, 16, -1.6),
          (18, 4.4, -3.8, 16.6, -1.4), (19, 1.9, 5.2, 10, 2.8), (21, .4, 6.1, 6, 3),
          (24, 1.3, 4, 7, 1.5), (26, .3, 5.5, 4.4, 2.8), (31, 1, 3.9, 6.2, 1.4),
          (33, .1, 5.4, 4, 2.7), (38, .9, 3.8, 5.3, 1.4), (40, .4, 4, 4, 1.6),
          (44, -1.8, 1, -3, -.2), (48, -1.2, -.8, -1.8, -.4), (53, 0, 0, 0, 0))


def gestures(marks):
    return list(zip(*(samples([(r[0], r[col]) for r in marks]) for col in range(1, 5)), strict=True))


GESTURES = {"bite": gestures(BITE), "breath": gestures(BREATH)}
GAPE = {
    "bite": list(samples(((0,0),(8,0),(16,.29),(21,.70),(25,1),(27,.51),(30,.035),(36,0),(60,0)))),
    "breath": list(samples(((0,0),(9,.06),(16,.34),(18,.68),(19,1),(22,.90),(25,.80),(26,.97),
                            (30,.79),(33,1),(36,.91),(40,.79),(44,.30),(48,.035),(53,0)))),
}


def pose(clip, frame, part, direction):
    phase = frame / CLIPS[clip][1][-1]
    f, s = math.cos(FACINGS[direction]), math.sin(FACINGS[direction])
    wing = part == 0 or part == 4
    near = (1 if part == 4 else -1)*(-1 if direction in (3, 4, 5) else 1)
    if clip == "bank":
        # The middle frame is neutral. The signed turning speed places the
        # dragon between these two banks without restarting the walking cycle.
        lean = 2*phase-1
        outer = near*lean
        if wing:
            return (-s*.75*lean, -outer*1.25, lean*(2.4+.9*f),
                    1+.014*outer, 1+.072*outer)
        if part == 5:
            return (-s*2.4*lean, f*1.15*lean, 2.6*lean, 1, 1)
        if part == 3:
            return (-s*.8*lean, f*.38*lean, lean*1.45, 1, 1)
        return (0, 0, 0, 1, 1)
    if clip == "fall":
        delay = (.15, .08, 0, .04, .20, .14)[part]
        weight = ease((phase-delay)/(.94-delay))
        x, y, angle, sx, sy = REST[part]
        recoil = math.sin(math.pi*ease(phase/.84)) if phase < .84 else 0.
        # The hind claws yield, the throat dips and the upper membrane catches air last.
        return (x*weight, y*weight+(-1 if wing else 1)*recoil*1.3,
                angle*weight+(7, -4, 5, 2, -6, 3)[part]*recoil,
                1+(sx-1)*weight, 1+(sy-1)*weight)

    a = phase * math.tau
    dx = dy = angle = 0.
    sx = sy = 1.
    if clip == "walk" and part in (1, 2):
        # The diagonally opposed feet share a stance. Rotate about the leg roots so
        # the shoulder attachment stays under the plates during the projected stride.
        travel, clearance = walking_foot((phase+(.5 if part == 2 else 0)) % 1.)
        travel *= 36*.58/(2*17.6)
        dx = f*travel*.32
        dy = s*travel*.28-clearance*.46
        angle = -f*travel*1.22
        sy += (s*travel*.72-clearance*.54)/29
    elif clip == "walk":
        rise = .97*math.cos(2*a-.35)
        wave = math.sin(a-.6)
        dy = -rise*(.56 if part == 5 else 1)
        angle = f*1.35*math.sin(2*a-.4)
        if wing:
            angle += near*(3.3*wave+1.5*math.sin(2*a-.61))
            sy += .047*math.cos(a-.78*near)
        if part == 5:
            dx += f*1.25*math.sin(2*a-.65)
            angle -= f*2.4*math.sin(2*a-.62)
    elif clip == "idle":
        inhale = (1-math.cos(a))*.5
        dy = -.95*inhale if part not in (1, 2) else 0
        if wing:
            sy += .054*inhale
            angle = near*(2.4*inhale+.66*math.sin(a))
        if part == 3:
            sy += .025*inhale
        if part == 5:
            dy -= .7*inhale
            angle = -f*(1.65*inhale+.5*math.sin(a))
    else:
        lift, reach, flare, lunge = GESTURES[clip][frame]
        if part in (1, 2):
            # Feet remain planted while the shoulder loads and unloads.
            angle = -f*lunge*(.48 if part == 1 else .77)
            sy -= lunge*.006
        else:
            dx, dy = f*lunge, .57*s*lunge-lift*.42
            if wing:
                angle = near*flare*(.59+.30*abs(s))+f*lift*.32
                sy += flare*.019
                sx += flare*.005
            if part == 3:
                angle = -f*lift*.68
                sx += lift*.004
            if part == 5:
                dx += f*reach*.86
                dy += s*reach*.52-lift*.36
                angle = -f*(lift*1.2-reach*.27)
    return dx, dy, angle, sx, sy


def track(parent, kind, param, label, times, data):
    layer = ET.SubElement(parent, "layer", type=kind, active="true", version="0.1", desc=label)
    vector_track = kind != "rotate"
    dtype = "vector" if vector_track else "angle"
    if kind in ("stretch", "rotate"):
        value(layer, "center" if kind == "stretch" else "origin", "vector", (0, 0))
    dest = ET.SubElement(ET.SubElement(layer, "param", name=param), "animated", type=dtype)
    for frame, item in zip(times, data, strict=True):
        waypoint = ET.SubElement(dest, "waypoint", time=f"{frame}f", before="linear", after="linear")
        node = ET.SubElement(waypoint, dtype)
        if vector_track:
            ET.SubElement(node, "x").text, ET.SubElement(node, "y").text = (f"{n:.7f}" for n in item)
        else:
            node.set("value", f"{item:.7f}")


def extract(source, root):
    atlas = Image.open(source).convert("RGBA")
    cell = atlas.width//8
    if atlas.size != (cell*8, cell*6):
        raise ValueError("The drake atlas must contain eight directions and six paint rows")
    parts = root / "parts"
    parts.mkdir(exist_ok=True)
    for d, name in enumerate(NAMES):
        for p, label in enumerate(PARTS):
            piece = atlas.crop((d*cell, p*cell, (d+1)*cell, (p+1)*cell))
            if not piece.getbbox():
                raise ValueError(f"Empty artwork: {name} {label}")
            piece.save(parts/f"{name}-{label}.png")


def extract_jaws(source, root):
    atlas=Image.open(source).convert("RGBA")
    if atlas.size != (TILE*len(ORIGINS),TILE*LEVELS):
        raise ValueError("Unexpected jaw atlas dimensions")
    for d in range(len(ORIGINS)):
        for level in range(1,LEVELS+1):
            atlas.crop((d*TILE,(level-1)*TILE,(d+1)*TILE,level*TILE)).save(root/"parts"/f"jaw-{NAMES[d]}-{level}.png")


def cutout(layer, name, rect, visibility=None):
    image(layer,name,rect)
    imported=layer.findall("layer")[-1]
    imported.remove(imported.find("param[@name='interpolation']"))
    if visibility is None:
        return
    amount=imported.find("param[@name='amount']")
    amount.remove(amount.find("real"))
    keys=ET.SubElement(amount,"animated",type="real")
    for frame,visible in enumerate(visibility):
        if frame and visible==visibility[frame-1]:
            continue
        wp=ET.SubElement(keys,"waypoint",time=f"{frame}f",before="constant",after="constant")
        ET.SubElement(wp,"real",value=str(int(visible)))


def author(root, clip):
    fps, times = CLIPS[clip]
    canvas = ET.Element("canvas", version="1.2", width="384", height="192", fps=str(fps),
                        **{"view-box": "0 0 10 -5", "antialias":"2", "begin-time":"0f",
                           "end-time":f"{times[-1]}f", "bgcolor":"0 0 0 0"})
    ET.SubElement(canvas, "name").text = f"Emberfall / Żarowy drak / {clip}"
    ET.SubElement(canvas, "desc").text = "Osiem widoków. Obraz części, skala, obrót i przesunięcie. Początek komórki w lewym górnym rogu."
    for d, name in enumerate(NAMES):
        facing = group(canvas, name)
        for p in order(d):
            label = PARTS[p]
            layer = group(facing, label)
            px, py = pivot(p, d)
            rect=(-px/64,py/64,(128-px)/64,(py-128)/64)
            if p==5 and d<len(ORIGINS) and clip in GAPE:
                gape=[int(v*LEVELS+.5) for v in GAPE[clip]]
                cutout(layer,name+"-"+label,rect,[v==0 for v in gape])
                left,top=ORIGINS[d]
                crop=((left/1.5-px)/64,(py-top/1.5)/64,((left+TILE)/1.5-px)/64,(py-(top+TILE)/1.5)/64)
                for level in range(1,LEVELS+1):
                    cutout(layer,f"jaw-{name}-{level}",crop,[v==level for v in gape])
            else:
                cutout(layer,name+"-"+label,rect)
            poses = [pose(clip, frame, p, d) for frame in times]
            track(layer, "stretch", "amount", "scale", times, [(r[3], r[4]) for r in poses])
            track(layer, "rotate", "amount", "turn", times, [-r[2] for r in poses])
            track(layer, "translate", "origin", "place", times,
                  [((r[0]+px)/64+d%4*GRID+MARGIN, -(r[1]+py)/64-d//4*GRID-MARGIN) for r in poses])
    if clip in GAPE:
        ctrl=ET.SubElement(canvas,"layer",type="circle",active="false",version="0.2",desc="runtime:gape")
        keys=ET.SubElement(ET.SubElement(ctrl,"param",name="radius"),"animated",type="real")
        for frame,gape in enumerate(GAPE[clip]):
            wp=ET.SubElement(keys,"waypoint",time=f"{frame}f",before="linear",after="linear")
            ET.SubElement(wp,"real",value=f"{gape:.6f}")
    path = root / (clip+".sif")
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return path


def read_keys(path, times, part_names=PARTS, pivot_for=pivot, grid=GRID, margin=MARGIN):
    frames = [[{} for _ in NAMES] for _ in times]
    for facing in ET.parse(path).getroot().findall("layer"):
        if facing.get("desc")=="runtime:gape":
            continue
        d = NAMES.index(facing.get("desc"))
        for part in facing.findall("param[@name='canvas']/canvas/layer"):
            p = part_names.index(part.get("desc"))
            tracks = {}
            for layer in part.findall("param[@name='canvas']/canvas/layer"):
                key = layer.get("desc")
                if key not in ("scale", "turn", "place"):
                    continue
                marks = layer.findall("param/animated/waypoint")
                if [point.get("time") for point in marks] != [f"{i}f" for i in times]:
                    raise ValueError(f"{path}: inconsistent controls for {d}/{p}/{key}")
                tracks[key] = [float(m.find("angle").get("value")) if key == "turn" else
                               tuple(float(m.findtext("vector/"+axis)) for axis in ("x", "y")) for m in marks]
            px, py = pivot_for(p, d)
            for frame, (angle, (x, y), (sx, sy)) in enumerate(zip(tracks["turn"], tracks["place"], tracks["scale"], strict=True)):
                row = ((x-d%4*grid-margin)*64-px, (-y-d//4*grid-margin)*64-py, -angle, sx-1, sy-1)
                frames[frame][d][p] = tuple(round(v*k) for v, k in zip(row, (128, 128, 128, 4096, 4096)))
    return frames


def compile_header(root, header):
    out = ["#pragma once", "// Generated from editable Synfig sprite transforms by ember_drake_synfig.py.",
           "namespace ember::ember_drake_motion {", "struct Key {short x,y,turn,scaleX,scaleY;};",
           "struct Clip {const Key (*frames)[8][6]; const unsigned char* times; int count;};",
           "inline constexpr float pivots[8][6][2]={"]
    for d in range(8):
        out.append(" {"+",".join("{%.5ff,%.5ff}"%pivot(p,d) for p in range(6))+"},")
    out.extend(("};", "inline constexpr int mouthTip[8][2]={"+
                ",".join("{%d,%d}"%landmark(d,3) for d in range(8))+"};"))
    for clip, (_, times) in CLIPS.items():
        frames = read_keys(root/(clip+".sif"), times)
        if clip in ("walk", "idle") and frames[0] != frames[-1]:
            raise ValueError(f"Open Synfig loop: {clip}")
        out.append("inline constexpr unsigned char "+clip+"Times[]={"+",".join(map(str,times))+"};")
        out.append("inline constexpr Key "+clip+"Keys[][8][6]={")
        for views in frames:
            out.append(" {")
            for view in views:
                out.append("  {"+",".join("{"+",".join(map(str,view[p]))+"}" for p in range(6))+"},")
            out.append(" },")
        out.extend(("};", f"inline constexpr Clip {clip}={{{clip}Keys,{clip}Times,{len(times)}}};"))
        if clip in GAPE:
            nodes=ET.parse(root/(clip+".sif")).findall("layer[@desc='runtime:gape']/param/animated/waypoint/real")
            out.append("inline constexpr unsigned char "+clip+"Gape[]={"+
                       ",".join(str(round(float(node.get("value"))*255)) for node in nodes)+"};")
    out.append("}")
    header.write_text("\n".join(out)+"\n", encoding="utf-8")


def verify_views(receipts):
    for clip, result in receipts.items():
        first, last = None, None
        margin = 1000
        variants = [set() for _ in NAMES]
        for path in sorted(Path(result["OutputDirectory"]).glob("frame_*.png")):
            with Image.open(path) as frame:
                rgba=frame.convert("RGBA")
            if first is None:
                first=rgba.tobytes()
            last=rgba.tobytes()
            width,height=rgba.width//4,rgba.height//2
            for d in range(8):
                left,top=d%4*width,d//4*height
                cell=rgba.crop((left,top,left+width,top+height))
                box=cell.getchannel("A").point(lambda a: 255 if a>=16 else 0).getbbox()
                if not box:
                    raise ValueError(f"Missing facing {NAMES[d]} in {path}")
                margin=min(margin,box[0],box[1],width-box[2],height-box[3])
                variants[d].add(hash(cell.tobytes()))
        if margin<1 or min(map(len,variants))<2:
            raise ValueError(f"{clip}: clipped or stationary native cutout")
        if clip in ("walk","idle") and first!=last:
            raise ValueError(f"{clip}: native endpoint differs from its first pose")
        result["VisibleMarginPixels"]=margin
        result["DistinctFramesByFacing"]=[len(v) for v in variants]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--atlas", type=Path)
    parser.add_argument("--jaws", type=Path)
    parser.add_argument("--author", action="store_true")
    parser.add_argument("--header", type=Path)
    parser.add_argument("--render", action="store_true")
    parser.add_argument("--verify", action="store_true")
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=True)
    if args.atlas:
        extract(args.atlas, args.root)
    if args.jaws:
        extract_jaws(args.jaws,args.root)
    if args.author:
        for clip in CLIPS:
            author(args.root, clip)
    if args.header:
        compile_header(args.root, args.header)
    if args.render:
        receipts = {}
        for clip, (_, times) in CLIPS.items():
            path = args.root / (clip+".sif")
            receipts[clip] = synfig_cli.render(path, threads=2, expected_frames=times[-1]+1, require_motion=True)
            print(clip, receipts[clip].get("ElapsedSeconds"), flush=True)
        verify_views(receipts)
        (args.root/"report.json").write_text(json.dumps(receipts, indent=2), encoding="utf-8")
    elif args.verify:
        receipts=json.loads((args.root/"report.json").read_text(encoding="utf-8"))
        verify_views(receipts)
        (args.root/"report.json").write_text(json.dumps(receipts, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

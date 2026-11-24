"""Export a layered Synfig cutout of the desert jackal to fixed-size game cels."""

import argparse
import json
import math
from pathlib import Path
from xml.etree import ElementTree as ET

import numpy as np
from PIL import Image
from scipy.interpolate import PchipInterpolator
from scipy.ndimage import distance_transform_edt, map_coordinates

from desert_enemy_motion import TRACKS, jackal_stride
from forest_boar_synfig import translate
from hero_synfig import group, image, rotate
import jackal_intermediate_views as turns
from jackal_skin_bake import POSES, prepare, VIEW_ANGLES
from sword_stow_synfig import animated
from synfig_cli import render, sha


VIEWS = (0, 1, 2, 6, 7)
ANGLES = dict(enumerate(VIEW_ANGLES)) | turns.ANGLES
WALK, ATTACK, FALL = 28, 17, 14
FRAMES = WALK + 1 + ATTACK + 2 * FALL
ORIGIN = np.array((80, 119))
LEFT, TOP = 11, 15
WIDTH, HEIGHT = 138, 120
PIXELS = (172, 150)
COLS, PAD = 9, 2

# The lower edge of the main cutout follows the belly and leaves the four
# separate legs underneath it. Coordinates are in the original Krita art.
BELLY = {
    0: ((0,375),(59,404),(75,343),(108,299),(142,285),(182,260),(227,257),
        (277,280),(318,280),(348,275),(391,262),(445,222),(550,232)),
    1: ((555,350),(591,351),(607,316),(633,289),(661,266),(700,249),
        (742,268),(776,273),(819,275),(853,276),(901,264),(974,264)),
    2: ((1110,268),(1181,279),(1212,271),(1246,280),(1280,282),(1313,276),
        (1338,267),(1404,261)),
    6: ((653,803),(698,819),(725,812),(742,827),(750,927),(769,931),
        (783,827),(798,824),(829,817),(875,807)),
    7: ((73,890),(116,935),(138,900),(164,830),(195,797),(220,766),
        (249,774),(288,785),(324,784),(367,782),(396,758),(488,758)),
}
TAIL = {
    0: ((117,226), ((179,127),(222,127),(269,111),(306,104),(359,78),(440,78))),
    1: ((631,204), ((142,654),(185,646),(226,629),(269,616),(305,607),(412,607))),
    7: ((175,756), ((675,195),(718,194),(758,171),(800,164),(855,144),(963,144))),
}


def track(name, phase):
    return float(MOTION[name](phase * 60))


MOTION = {name: PchipInterpolator(*zip(*keys)) for name, keys in TRACKS.items()}


def unit(vector):
    return vector / max(.01, np.linalg.norm(vector))


def angle(vector):
    return math.degrees(math.atan2(vector[0], vector[1]))


def rotated(vector, radians):
    c, s = math.cos(radians), math.sin(radians)
    return np.array((c*vector[0]-s*vector[1], s*vector[0]+c*vector[1]))


def cutout(pixels, alpha):
    return Image.fromarray(np.dstack((pixels, np.uint8(np.rint(np.clip(alpha, 0, 1)*255)))))


def extract_body(sprite, bones, view, folder):
    raw = np.asarray((POSES | turns.POSES)[view][1], dtype=float)
    scale = (bones[0, 4]-bones[0, 0])/(raw[0, 4]-raw[0, 0])
    offset = bones[0, 0]-raw[0, 0]*scale
    lower_edge = PchipInterpolator(*zip(*(BELLY | turns.BELLY)[view]))
    y, x = np.indices((sprite.height, sprite.width), dtype=float)/2
    edge = lower_edge((x-offset[0])/scale[0])*scale[1]+offset[1]
    separation = np.clip((edge-y)/1.4+.63, 0, 1)
    rgba = np.asarray(sprite)
    alpha = separation*rgba[..., 3]/255
    tail_pivot = None
    tails = TAIL | turns.TAIL
    if view in tails:
        pivot, seam = tails[view]
        height, width = np.asarray(seam).T
        boundary = np.interp((y-offset[1])/scale[1],height,width)*scale[0]+offset[0]
        split = np.clip((boundary-x)/1.65+.62,0,1)
        cutout(rgba[..., :3], alpha*split).save(folder/f"tail-{view}.png")
        tail_pivot = np.array(pivot)*scale+offset
        alpha *= np.clip((x-boundary)/1.65+1,0,1)
    body = cutout(rgba[..., :3], alpha)
    body.save(folder / f"body-{view}.png")
    contour = np.column_stack(np.where(separation[::2,::2]*rgba[::2,::2,3] > 96))[:, ::-1]
    return contour,tail_pivot


def extract_bone(stem, ink, start, end, radii, caps, scale, folder):
    length = float(np.linalg.norm(end-start))
    across = max(radii) + 1.1
    lead, trail = caps
    w, h = math.ceil(2*across*3), math.ceil((length+lead+trail)*3)
    y, x = np.indices((h, w), dtype=float)
    y = (y+.5)/h*(length+lead+trail)-lead
    x = ((x+.5)/w*2-1)*across
    axis = unit(end-start)
    normal = np.array((axis[1], -axis[0]))
    along = np.clip(y/length, 0, 1)
    width = PchipInterpolator((0, .5, 1), radii)(along)
    rounding = np.maximum(np.maximum(-y/lead, (y-length)/trail), 0)
    alpha = np.clip((1-np.hypot(x/width, rounding))*width*2.5+.08, 0, 1)
    points = start[:, None, None] + axis[:, None, None]*y + normal[:, None, None]*x
    sample = [(points[1]*2)-.5, (points[0]*2)-.5]
    colors = np.dstack([map_coordinates(ink[..., channel], sample, order=1, mode="nearest")
                        for channel in range(3)])
    colors = np.uint8(np.clip(colors*scale, 0, 255))
    cutout(colors, alpha).save(folder / f"{stem}.png")
    return across, lead, trail


def extract_paw(stem, ink, toe, fore, tone, folder):
    width, height = (3.7, 3.2) if fore else (3.9, 3.5)
    u,v = np.meshgrid(np.linspace(-1,1,25), np.linspace(-1,1,20))
    uv = [(toe[1]-height*.58+height*.64*v)*2-.5,
          (toe[0]+width*.61*u)*2-.5]
    color = np.dstack([map_coordinates(ink[..., channel], uv, order=1, mode="nearest") for channel in range(3)])
    pad = np.clip((1-np.hypot(u*.96,(v+.10)*.92))*6,0,1)
    cutout(np.uint8(np.clip(color*tone,0,255)),pad).save(folder/f"{stem}.png")
    return (-width, height*1.25, width, -height*.14)


def extract(sprite, bones, view, folder):
    contour,tail = extract_body(sprite, bones, view, folder)
    art = np.asarray(sprite)
    # Extend the nearest painted pixel under the parts' antialiased edges.
    nearest = distance_transform_edt(art[..., 3] < 155, return_distances=False, return_indices=True)
    ink = art[tuple(nearest)][..., :3]
    parts = []
    depth = bones[:, -1, 1]
    for slot, leg in enumerate(bones):
        fore = slot >= 2
        chain = leg[[1 if fore else 0, 2, 3, 4]]
        near = depth[slot] >= depth[slot ^ 1]
        tone = 1.0 if near else .82
        rear_width = 6.65 if view in (0, 1, 7, 8, 11) else 5.15
        shape = ((5.05,4.48,3.05), (3.05,2.48,2.54), (2.55,2.79,2.40)) if fore else (
                 (rear_width,4.7,3.08), (3.15,2.53,2.58), (2.48,2.78,2.40))
        feet = ((3.15,2.15),(2.4,1.55),(1.50,.88))
        segments = [extract_bone(f"leg-{view}-{slot}-{i}", ink, a, b, shape[i], feet[i], tone, folder)
                    for i, (a, b) in enumerate(zip(chain, chain[1:]))]
        paw = extract_paw(f"paw-{view}-{slot}", ink, chain[-1], fore, tone, folder)
        parts.append((segments,paw))
    return parts,contour,tail


def step(phase, slot):
    fore = slot >= 2
    phase = (phase + .5*(slot % 2) + (.752 if fore else 0)) % 1
    support = .66 if fore else .61
    t = phase/support*(38/60) if phase < support else (38+22*(phase-support)/(1-support))/60
    stride = jackal_stride(t)*support*27
    lift = track("jackal_lift", t)*(8.5 if fore else 7.2)
    bend = track("jackal_fore_flex" if fore else "jackal_hind_flex", t)
    return stride, lift, bend


def action(frame):
    if frame < WALK:
        return "walk", frame/WALK, 1
    frame -= WALK
    if frame == 0:
        return "idle", 0, 1
    frame -= 1
    if frame < ATTACK:
        return "bite", frame/(ATTACK-1), 1
    frame -= ATTACK
    return "fall", (frame % FALL)/(FALL-1), -1 if frame < FALL else 1


def pose(bones, body_contour, view, frame):
    kind, phase, side = action(frame)
    theta = ANGLES[view]
    projection = unit(np.array((math.cos(theta),math.sin(theta)/.52)))
    forward = projection*np.array((1, .52))
    force = track("jackal_pounce",phase) if kind == "bite" else 0
    crouch = track("jackal_crouch",phase) if kind == "bite" else 0
    launch = track("jackal_land",phase) if kind == "bite" else 0
    roll = track("jackal_roll",phase) if kind == "fall" else 0
    tuck = track("jackal_fold",phase) if kind == "fall" else 0
    pivot = np.array((80.,80.))
    dy = -1.05*math.cos(4*math.pi*(phase-.095)) if kind == "walk" else 0
    drift = forward*(5.8*force - 2.8*crouch) + np.array((0,dy+5.5*crouch-10.2*launch))
    turn = (math.sin(4*math.pi*phase)*.026*projection[0] if kind == "walk"
            else projection[0]*(.137*crouch-.148*launch))
    turn += roll*side*(.36+.92*abs(projection[1]))
    positions = []
    for slot, rest in enumerate(bones):
        fore = slot >= 2
        chain = rest[[1 if fore else 0, 2, 3, 4]].copy()
        stride, lift, bend = step(phase,slot) if kind == "walk" else (0,0,0)
        foot = chain[3] + forward*(stride + force*(8 if fore else -5))
        foot[1] -= lift + launch*(12 if fore else 5)
        root = pivot+rotated(chain[0]-pivot,turn)+drift
        toe = rotated(chain[3]-chain[2],projection[0]*bend*.35)
        wrist = foot-toe
        root_shift, foot_shift = root-chain[0], wrist-chain[2]
        ratio = .51 if fore else .72
        hinge = chain[1]+root_shift*(1-ratio)+foot_shift*ratio
        hinge += (-projection[0]*bend*(4.3 if fore else 2.75),-1.15*bend)
        if kind == "fall":
            base = chain[0]
            folded = base + np.array(((7.0 if fore else -9.5)*projection[0]+side*(slot%2-0.5)*3, 14.5))
            foot = pivot + rotated((foot*(1-tuck)+folded*tuck)-pivot,turn)
            wrist = foot-rotated(toe,turn)
            hinge = pivot + rotated(chain[1]*(1-tuck)+(.56*base+.44*folded)*tuck-pivot,turn)
        positions.append(np.array((root, hinge, wrist, foot)))

    if kind == "fall":
        # Rigid body roll, then a single ground correction shared by every part.
        relative = body_contour-pivot
        torso_bottom = max(relative[:,0]*math.sin(turn)+relative[:,1]*math.cos(turn))+pivot[1]+drift[1]
        paw_bottom = max(max(leg[:, 1])+1.0 for leg in positions)
        far_edge = max(torso_bottom,paw_bottom)
        dy = (123.5-far_edge)*(tuck if far_edge<123.5 else 1)
        drift[1] += dy
        for leg in positions:
            leg[:, 1] += dy
    body_offset = pivot - rotated(pivot, turn) + drift
    return body_offset, turn, positions


def vector(point):
    return point[0], -point[1]


def layer_import(parent, asset, corners, key):
    image(parent, asset, corners[0])
    layer = parent[-1]
    if len(corners) > 1:
        for i, label in ((0, "tl"), (2, "br")):
            param = layer.find(f"param[@name='{label}']")
            param.remove(param[0])
            animated(param,f"{key}-{label}","vector",[p[i:i+2] for p in corners])


def author(root, positions, body_only=False):
    canvas = ET.Element("canvas", version="1.2", width=str(PIXELS[0]*len(positions)),height=str(PIXELS[1]), fps="36",
        **{"view-box": f"0 0 {WIDTH*len(positions)} {-HEIGHT}", "begin-time":"0f", "end-time":f"{FRAMES-1}f", "bgcolor":"0 0 0 0"})
    ET.SubElement(canvas,"name").text = "Emberfall / Szakal wydmowy / warstwowe klatki"
    for index, view in enumerate(positions):
        bones, (parts,contour,tail) = positions[view]
        poses = [pose(bones,contour,view,i) for i in range(FRAMES)]
        page = group(canvas, f"Kierunek {view}")
        offset = np.array((index*WIDTH-LEFT,-TOP))
        if tail is not None:
            layer = group(page,"Ogon / opóźniony ruch")
            image(layer, f"tail-{view}", (0,0,160,-144))
            wag = [math.sin(2*math.pi*(i/WALK-.16))*4.5 if i<WALK else 0 for i in range(FRAMES)]
            rotate(layer,f"ogon-{view}",vector(tail),wag)
            rotate(layer,f"obrot-ogona-{view}",(0,0),[-math.degrees(p[1]) for p in poses])
            translate(layer,f"ciężar-ogona-{view}",[vector(p[0]+offset) for p in poses])
        # Low screen position determines which leg is nearest in the painting.
        for slot in (() if body_only else np.argsort(bones[:,-1,1])):
            leg = group(page, f"Łapa {slot+1}")
            segments,pad = parts[slot]
            points = [p[2][slot] for p in poses]
            for i, (half, lead, trail) in enumerate(segments):
                stem = f"leg-{view}-{slot}-{i}"
                piece = group(leg, stem)
                delta = [p[i+1]-p[i] for p in points]
                layer_import(piece, stem, [(-half,lead,half,-np.linalg.norm(d)-trail) for d in delta], stem)
                rotate(piece, stem+"-obrot", (0,0), [angle(d) for d in delta])
                translate(piece, stem+"-przesuniecie", [vector(p[i]+offset) for p in points])
            stem = f"paw-{view}-{slot}"
            paw = group(leg, stem)
            image(paw, stem, pad)
            roll = [math.degrees(p[1]) if action(i)[0]=="fall" else
                    -step(i/WALK,slot)[2]*12*math.cos(ANGLES[view]) if i<WALK else 0
                    for i,p in enumerate(poses)]
            rotate(paw, stem+"-obrot", (0,0), roll)
            translate(paw, stem+"-przesuniecie", [vector(p[-1]+offset) for p in points])

        body = group(page, "Tułów i kark")
        image(body, f"body-{view}", (0,0,160,-144))
        rotate(body, f"tulow-{view}", (0,0), [-math.degrees(p[1]) for p in poses])
        translate(body, f"ciezar-{view}", [vector(p[0]+offset) for p in poses])
    target = root / "szakal-klatki.sif"
    ET.indent(canvas)
    ET.ElementTree(canvas).write(target,encoding="utf-8",xml_declaration=True)
    return target


def pack(receipt, output, views, body_only=False):
    frames = sorted(Path(receipt["OutputDirectory"]).glob("frame_*.png"))
    if len(frames) != FRAMES:
        raise ValueError("Niepełna liczba klatek Synfiga")
    w,h = PIXELS
    row = math.ceil(FRAMES/COLS)
    pages = [Image.new("RGBA", (COLS*(w+PAD),row*(h+PAD))) for _ in views]
    for i, path in enumerate(frames):
        with Image.open(path) as src:
            for page, atlas in enumerate(pages):
                clip = src.crop((page*w,0,(page+1)*w,h)).convert("RGBA")
                bounds = clip.getchannel("A").point(lambda a: 255 if a > 96 else 0).getbbox()
                if not bounds or min(bounds[0],bounds[1],w-bounds[2],h-bounds[3]) < 1:
                    raise ValueError(f"Pusta lub ucięta klatka {i}, kierunek {views[page]}, obrys {bounds}")
                atlas.paste(clip, (i%COLS*(w+PAD)+1,i//COLS*(h+PAD)+1))
    output.mkdir()
    for page, atlas in enumerate(pages):
        atlas.save(output / f"{'body' if body_only else 'cel'}-{views[page]}.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--painted-views", type=Path, required=True)
    parser.add_argument("--intermediate-views", type=Path)
    parser.add_argument("--body-only", action="store_true")
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    root = args.root
    root.mkdir(exist_ok=False, parents=True)
    folder = root / "parts"
    folder.mkdir()
    positions = {}
    views = (VIEWS if args.body_only or not args.intermediate_views else ())
    if args.intermediate_views:
        views += tuple(turns.POSES)
    with Image.open(args.source) as source, Image.open(args.painted_views) as painted:
        for view in set(views).intersection(VIEWS):
            _, bones, _, _ = prepare(source, POSES[view], view)
            x, y = (view % 4)*320, (view // 4)*288
            sprite = painted.crop((x,y,x+320,y+288)).convert("RGBA")
            positions[view] = bones,extract(sprite,bones,view,folder)
        if args.intermediate_views:
            with Image.open(args.intermediate_views) as added:
                for view in turns.POSES:
                    sprite,bones = turns.register(added,source,painted,view)
                    positions[view] = bones,extract(sprite,bones,view,folder)
    project = author(root,{view:positions[view] for view in views},args.body_only)
    receipt = render(project, output_root=root/"renders", threads=2,
                     expected_frames=FRAMES, require_motion=True)
    pack(receipt,root/"atlases",views,args.body_only)
    manifest = {"source_sha256": sha(args.source), "painted_sha256": sha(args.painted_views), "render": receipt,
                "walk": WALK, "attack": ATTACK, "fall": FALL, "views": views,
                "atlases": {p.name: sha(p) for p in (root/"atlases").glob("*.png")}}
    if args.intermediate_views:
        manifest["intermediate_sha256"] = sha(args.intermediate_views)
    (root/"receipt.json").write_text(json.dumps(manifest,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main()

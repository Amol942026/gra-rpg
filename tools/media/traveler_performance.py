"""Author, export and silently render the traveler's eight-view Synfig motion studies."""
import argparse
import gzip
import hashlib
import json
import math
import re
import time
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image, ImageOps

import synfig_cli
import walk_synfig as joints
from hero_synfig import group, value
from traveler_gestures import cutouts, image


REPO = Path(__file__).resolve().parents[2]
DIRECTIONS = ("E", "SE", "S", "SW", "W", "NW", "N", "NE")
CHANNELS = ("spineRoll", "spinePitch", "headRoll", "armSwing", "capeSway", "capeLift",
            "spineYaw", "headYaw", "freeElbow")
# Normalised time is shared with the existing attack contact, gait distance and roll timers.
# The guard arm folds ahead of the cut. The coat follows the turn through the recovery.
CLIPS = {
    "idle": (120, True, {
        0: (0, -.55, 0, 0, -.28, 0, -.65, .9, -.7),
        .20: (.55, -.10, -.3, .4, -.05, .12, -.35, 1.25, -.35),
        .40: (.82, 1.28, -.62, .95, .86, .83, .72, -.3, 1.2),
        .51: (.65, 1.35, -.74, .82, 1.22, 1.10, .94, -.7, 1.75),
        .68: (.16, .48, -.25, .1, .64, .95, .21, -.45, 1.05),
        .83: (-.32, -.40, .31, -.52, -.23, .31, -.53, .11, -.5),
        1: (0, -.55, 0, 0, -.28, 0, -.65, .9, -.7),
    }),
    "walk": (60, True, {
        0: (0, .14, .05, 0, -1.5, .5, .95, -.6, 0),
        .12: (.60, .68, -.27, 0, .08, 1.56, 1.05, -.78, 2.2),
        .26: (.86, -.18, -.53, 0, 3.88, 3.35, -.05, .12, 4.3),
        .40: (.42, -.28, -.22, 0, 4.80, 2.1, -.64, .45, 2.1),
        .5: (0, .14, -.05, 0, 1.3, .5, -.95, .60, -.5),
        .62: (-.60, .68, .27, 0, -.10, 1.62, -1.05, .78, -1.6),
        .76: (-.86, -.18, .53, 0, -3.70, 3.05, .05, -.12, -2.9),
        .90: (-.42, -.28, .22, 0, -4.52, 1.90, .64, -.45, -1.65),
        1: (0, .14, .05, 0, -1.5, .5, .95, -.6, 0),
    }),
    "slash": (20, False, {
        0: (0, 0, 0, 0, 0, 0, 0, 0, 0),
        .16: (-1.2, -2.1, .8, -.75, -.55, -.22, -4, 3.2, 8),
        .26: (-1.5, -2.65, .9, -1.55, -3.65, -1.25, -5.3, 3.6, 12),
        .43: (1.2, 1.45, -.8, .75, -5.50, -.48, .40, -.53, 6.5),
        .59: (2.75, 3.55, -1.48, 2.5, -.7, 3.12, 5.30, -3.6, -2.8),
        .71: (1.50, 2.05, -.90, 1.42, 6.40, 6.0, 3.65, -2.4, -4.6),
        .86: (.15, .16, -.17, .35, 3.75, 2.78, .42, -.40, -1.45),
        1: (0, 0, 0, 0, 0, 0, 0, 0, 0),
    }),
    "run": (48, True, {
        0: (0, .1, 0, 0, -2.4, 2, 0, .35, 0),
        .16: (2.75, 1.7, -1.7, 0, 1.6, 4.7, 1.15, -.78, 0),
        .31: (2.8, 2.5, -1.85, 0, 7.1, 7.5, -.2, .16, 0),
        .5: (0, .1, 0, 0, 2.4, 2, 0, -.35, 0),
        .66: (-2.75, 1.7, 1.7, 0, -1.6, 4.7, -1.15, .78, 0),
        .81: (-2.8, 2.5, 1.85, 0, -7.1, 7.5, .2, -.16, 0),
        1: (0, .1, 0, 0, -2.4, 2, 0, .35, 0),
    }),
    "heavy": (30, False, {
        0: (0, 0, 0, 0, 0, 0, 0, 0, 0),
        .21: (-2.4, -3.1, 1.36, -2, -2.2, -1.8, -3.9, 2.7, 6.8),
        .35: (-3.3, -4.5, 1.9, -2.35, -6.8, -3.3, -5.5, 3.95, 10.4),
        .55: (3.2, 4.7, -1.9, 2.65, -2.5, 2.2, 2.75, -2.1, .7),
        .70: (2.45, 3, -1.35, 2.2, 8.3, 8.1, 4.3, -2.9, -3.6),
        .87: (.40, .5, -.30, .4, 4, 4.4, .81, -.59, -.9),
        1: (0, 0, 0, 0, 0, 0, 0, 0, 0),
    }),
    "roll": (24, False, {
        0: (0, 0, 0, 0, 0, 0, 0, 0, 0),
        .15: (0, 0, 0, 0, -1.4, -3.2, 0, 0, 2),
        .35: (0, 0, 0, 0, -6.1, -8.8, 0, 0, 3.5),
        .55: (0, 0, 0, 0, -4.7, -6.1, 0, 0, 2.6),
        .73: (0, 0, 0, 0, 2.2, 1.7, 0, 0, .6),
        .88: (0, 0, 0, 0, 2.5, 3.6, 0, 0, -.5),
        1: (0, 0, 0, 0, 0, 0, 0, 0, 0),
    }),
}


def samples(keys, frames, loop):
    """Continuous, shape-preserving cubic tangents, including the wrap of a cyclic curve."""
    marks = sorted(keys)
    points = [keys[t] for t in marks]

    def tangent(slope_a, slope_b):
        if slope_a * slope_b <= 0:
            return 0.
        return 2 * slope_a * slope_b / (slope_a + slope_b)

    slopes = [tuple((v-u)/(b-a) for u, v in zip(keys[a], keys[b]))
              for a, b in zip(marks, marks[1:])]
    knots = []
    for i in range(len(marks)):
        if loop or 0 < i < len(marks)-1:
            knots.append(tuple(tangent(a, b) for a, b in zip(slopes[(i-1) % len(slopes)],
                                                               slopes[i % len(slopes)])))
        else:
            knots.append((0.,) * len(CHANNELS))
    result = []
    segment = 0
    for frame in range(frames + 1):
        phase = frame / frames
        while phase > marks[segment+1]:
            segment += 1
        step = marks[segment+1] - marks[segment]
        t = (phase - marks[segment]) / step
        a, b = points[segment:segment+2]
        m, n = knots[segment:segment+2]
        result.append(tuple((2*t**3-3*t*t+1)*x + (t**3-2*t*t+t)*step*p
                            + (-2*t**3+3*t*t)*y + (t**3-t*t)*step*q
                            for x, y, p, q in zip(a, b, m, n)))
    return result


def base_frames(name):
    root = REPO / "src/generated"
    path = root / ("hero_walk_data.hpp" if name == "walk" else "hero_motion_data.hpp")
    identifier = "keys" if name == "walk" else name.capitalize() + "Keys"
    block = re.search(r"Key " + identifier + r"\[\] = \{(.*?)\};", path.read_text(), re.S)[1]
    rows = [tuple(map(float, re.findall(r"-?\d+\.\d+(?=f)", row))) for row in block.splitlines() if "{{" in row]
    assert all(len(row) == 7 for row in rows) and len(rows) % 17 == 0
    return [rows[i:i+17] for i in range(0, len(rows), 17)]


def world_pose(local, detail, layout):
    heads, rest, parents = layout
    positions, orientations = [], []
    for bone, row in enumerate(local):
        p, q = row[:3], joints.unit(row[3:])
        parent = parents[bone]
        if parent >= 0:
            p = joints.add(positions[parent], joints.rotate(orientations[parent], p))
            q = joints.qmul(orientations[parent], q)
        positions.append(p)
        orientations.append(q)

    def turn(bone, q):
        pivot = positions[bone]
        for i in range(bone, len(parents)):
            parent = i
            while parent >= 0 and parent != bone:
                parent = parents[parent]
            if parent == bone:
                positions[i] = joints.add(pivot, joints.rotate(q, joints.sub(positions[i], pivot)))
                orientations[i] = joints.qmul(q, orientations[i])

    roll, pitch, neck, arm, _, _, yaw, look, elbow = detail
    turn(2, joints.qmul(joints.axis((0, 0, 1), yaw),
                        joints.qmul(joints.axis((0, 1, 0), roll), joints.axis((1, 0, 0), -pitch))))
    turn(3, joints.qmul(joints.axis((0, 0, 1), look), joints.axis((0, 1, 0), neck)))
    turn(4, joints.axis((1, 0, 0), arm))
    turn(7, joints.axis((1, 0, 0), -arm))
    turn(5, joints.axis((1, 0, 0), elbow))

    def point(bone, offset):
        skin = joints.qmul(orientations[bone], joints.conj(rest[bone]))
        return joints.add(positions[bone], joints.rotate(skin, offset))
    return positions, point


def ribbon(a, b, w, v):
    dx, dy = b[0]-a[0], b[1]-a[1]
    length = max(.01, math.hypot(dx, dy))
    nx, ny = -dy/length, dx/length
    return [(a[0]+w*nx, a[1]+w*ny), (a[0]-w*nx, a[1]-w*ny),
            (b[0]-v*nx, b[1]-v*ny), (b[0]+v*nx, b[1]+v*ny)]


def studies(direction, pose, detail, name, phase):
    angle = math.pi * direction / 4
    fx, fy = joints.unit((math.cos(angle), math.sin(angle)/.52))
    pos, point = pose
    def project(p):
        r, f, z = p
        if name == "roll":
            spin = phase * math.tau
            tuck = min(1., phase/.16, (1-phase)/.16)
            z, f = 38-9*tuck+(z-38)*(1-.32*tuck)*math.cos(spin)-f*math.sin(spin), f*math.cos(spin)+(z-38)*(1-.32*tuck)*math.sin(spin)
        return r*fy+f*fx, z-(f*fy-r*fx)*.52

    mesh = {}
    def limb(tag, a, b, w, v, color):
        mesh[tag] = (color, ribbon(project(pos[a]), project(pos[b]), w, v))
    def arm(side):
        left = side == "L"
        a, b, c = (4, 5, 6) if left else (7, 8, 9)
        limb("Ramię " + side, a, b, 4, 3.5, "#526b69")
        limb("Karwasz " + side, b, c, 3.5, 2.15, "#906d49")
        if not left and name in ("slash", "heavy"):
            mesh["Głownia"] = ("#d3d9c6", ribbon(project(pos[16]), project(point(16, (0, 9.25, 35.82))), 1.85, .22))
    far, near = ("L", "R") if fx < 0 else ("R", "L")
    for side in (far, near):
        a,b,c = (10,11,12) if side == "L" else (13,14,15)
        limb("Udo " + side, a,b,4.4,3.6,"#5e594d")
        limb("Cholewa " + side,b,c,3.9,3.05,"#735640")
        mesh["Stopa "+side] = ("#57423a",ribbon(project(point(c,(0,-3,-1))),project(point(c,(0,8,-2))),3.15,2.6))
    arm(far)
    def painted(tag, part, bone, center, top):
        mesh[tag] = ("@"+part,[project(point(bone,center)),project(point(bone,top))])
    def cape():
        sway, lift = detail[4:6]
        painted("Płaszcz","mantle",2,(sway*.24,-5-lift*.13,-2+lift*.1),(0,-3,16))
    if fy >= 0:
        cape()
    painted("Kaftan","coat",2,(0,0,0),(0,0,18))
    if fy < 0:
        cape()
    painted("Głowa","head-"+DIRECTIONS[direction],3,(0,0,10),(0,0,20))
    arm(near)
    return mesh


def animated(parent, kind, keys, identifier=None):
    attr = {"id": identifier} if identifier else {}
    node = ET.SubElement(parent, "animated", type=kind, **attr)
    for frame, item in keys:
        waypoint = ET.SubElement(node, "waypoint", time=f"{frame}f", before="linear", after="linear")
        if kind == "vector":
            vector = ET.SubElement(waypoint, "vector")
            for key, component in zip("xy", item):
                ET.SubElement(vector, key).text = f"{component:.3f}"
        elif kind=="angle":
            ET.SubElement(waypoint,"angle",value=f"{item:.6f}")
        else:
            ET.SubElement(waypoint, "real", value=f"{item:.6f}")


def prepare_parts(folder):
    assets=REPO/"assets/sprites"
    cutouts(folder,assets/"hero-paints.png",assets/"hero-heads.png")
    with Image.open(assets/"hero-heads.png") as heads:
        for direction,view in zip(DIRECTIONS,(2,1,0,1,2,3,4,3)):
            tile=heads.crop((128*view,0,128*(view+1),128))
            if direction in ("SW","W","NW"):
                tile=ImageOps.mirror(tile)
            tile.save(folder/("head-"+direction+".png"))


def painted_layer(body,tag,part,views,frames,x,y,direction):
    width=(10 if part.startswith("head-") else 13 if part=="coat" else 17)
    height=(12 if part.startswith("head-") else 20)
    if not part.startswith("head-"):
        width*=.60+.4*abs(math.sin(direction*math.pi/4))
    cut=group(body,tag)
    image(cut,part,(-width,height,width,-height))
    angles,origins=[],[]
    for frame,view in zip(frames,views):
        center,top=view[tag][1]
        deg=math.degrees(math.atan2(center[0]-top[0],top[1]-center[1]))
        if angles:
            deg=angles[-1][1]+math.remainder(deg-angles[-1][1],360)
        angles.append((frame,deg))
        origins.append((frame,(x+center[0],y+center[1])))
    rot=ET.SubElement(cut,"layer",type="rotate",active="true",version="0.1")
    value(rot,"origin","vector",(0,0))
    animated(ET.SubElement(rot,"param",name="amount"),"angle",angles)
    pos=ET.SubElement(cut,"layer",type="translate",active="true",version="0.1")
    animated(ET.SubElement(pos,"param",name="origin"),"vector",origins)


def run_study_arms(local, frame, arm_keys, layout):
    # Match the running arm rig attached to the chest in hero_motion.hpp.
    heads, rest, _ = layout
    pose = list(local)
    spine_inverse = joints.conj(rest[2])
    for side, upper in enumerate((4, 7)):
        sign = 1 if side else -1
        swing, flex, tuck, shoulder = arm_keys[(frame + side*24) % 48]
        def segment(angle, tilt):
            q=joints.qmul(joints.axis((1,0,0),angle), joints.axis((0,1,0),sign*tilt))
            return joints.rotate(q, (0,0,-13))
        a, b = segment(swing,tuck), segment(swing+flex,3.5)
        qa=joints.qmul(joints.from_to(joints.sub(heads[upper+1],heads[upper]),a), rest[upper])
        qb=joints.qmul(joints.from_to(joints.sub(heads[upper+2],heads[upper+1]),b), rest[upper+1])
        pose[upper]=(*joints.add(local[upper][:3],joints.rotate(spine_inverse,(0,shoulder,0))),*joints.qmul(spine_inverse,qa))
        pose[upper+1]=(*joints.rotate(joints.conj(qa),a),*joints.qmul(joints.conj(qa),qb))
        pose[upper+2]=(*joints.rotate(joints.conj(qb),b),*local[upper+2][3:])
    return pose


def read_run_gait(source):
    with gzip.open(source) as stream:
        root = ET.parse(stream).getroot()
    count = CLIPS["run"][0]
    channels = {}
    for channel in joints.NAMES:
        points = root.findall(f"defs/animated[@id='gait_{channel}']/waypoint")
        assert [w.get("time") for w in points] == [f"{i}f" for i in range(count+1)], channel
        values = [float(w.find("real").get("value")) for w in points]
        assert all(math.isfinite(x) for x in values) and values[0] == values[-1], channel
        channels[channel] = values
    return [{channel:values[f] for channel,values in channels.items()} for f in range(count+1)]


def author(source, name, edited=None, gait=None):
    frame_count, loop, keys = CLIPS[name]
    rows = edited or samples(keys, frame_count, loop)
    layout = joints.rig_layout()
    if name == "run":
        gait = gait or [joints.controls(f/frame_count, running=True) for f in range(frame_count+1)]
        arm_header = (REPO/"src/generated/hero_run_arms_data.hpp").read_text()
        arm_keys = [tuple(map(float,re.findall(r"-?\d+\.\d+(?=f)", line)))
                    for line in arm_header.splitlines() if line.lstrip().startswith("{")]
        assert len(arm_keys) == 49 and all(len(k)==4 for k in arm_keys)
        base = [run_study_arms(joints.pose(row,layout)[0], f, arm_keys,layout) for f,row in enumerate(gait)]
    else:
        base = base_frames(name)
    canvas = ET.Element("canvas", version="1.2", width="440", height="216", fps="60",
                        **{"view-box": "-62 92 434 -151", "begin-time":"0f",
                           "end-time":f"{frame_count-1}f", "antialias":"1", "bgcolor":"0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Wędrowiec / " + name + " / E SE S SW W NW N NE"
    ET.SubElement(canvas, "desc").text = "Osiem rzutów kontrolnych rigu. Kanały eksportowane do gry znajdują się w bibliotece wartości."
    defs = ET.SubElement(canvas, "defs")
    for channel, values in zip(CHANNELS, zip(*rows)):
        animated(defs, "real", enumerate(values), channel)
    if name == "run":
        for channel in joints.NAMES:
            animated(defs, "real", ((f, row[channel]) for f,row in enumerate(gait)), "gait_"+channel)
    sample_frames = sorted({round(i*frame_count/16) for i in range(17)})
    poses = [world_pose(base[round(f*(len(base)-1)/frame_count)], rows[f], layout) for f in sample_frames]
    for direction, facing in enumerate(DIRECTIONS):
        body = group(canvas, facing)
        views = [studies(direction, pose, rows[f], name, f/frame_count) for f, pose in zip(sample_frames, poses)]
        x, y = (direction % 4)*124, -(direction//4)*117
        for tag, (color, polygon) in views[0].items():
            if color.startswith("@"):
                painted_layer(body,tag,color[1:],views,sample_frames,x,y,direction)
                continue
            layer = ET.SubElement(body, "layer", type="polygon", version="0.1", active="true", desc=tag)
            paint = ET.SubElement(ET.SubElement(layer,"param",name="color"),"color")
            for k, i in zip("rgb",(1,3,5)):
                ET.SubElement(paint,k).text = str((int(color[i:i+2],16)/255)**2.2)
            ET.SubElement(paint,"a").text = "1"
            points = ET.SubElement(ET.SubElement(layer,"param",name="vector_list"),"dynamic_list",type="vector")
            for vertex in range(len(polygon)):
                coords = ((f,(view[tag][1][vertex][0]+x,view[tag][1][vertex][1]+y)) for f,view in zip(sample_frames, views))
                animated(ET.SubElement(points,"entry"),"vector",coords)
    with source.open("xb") as stream, gzip.GzipFile(filename="",mode="wb",fileobj=stream,mtime=0) as zipped:
        ET.ElementTree(canvas).write(zipped, encoding="utf-8", xml_declaration=True)


def read_controls(source, count, loop):
    with gzip.open(source) as stream:
        root = ET.parse(stream).getroot()
    assert [g.get("desc") for g in root.findall("layer")] == list(DIRECTIONS)
    cols = []
    for channel in CHANNELS:
        points = root.findall(f"defs/animated[@id='{channel}']/waypoint")
        assert [w.get("time") for w in points] == [f"{i}f" for i in range(count+1)]
        vals = [float(w.find("real").get("value")) for w in points]
        assert all(math.isfinite(v) for v in vals) and (not loop or vals[0] == vals[-1])
        cols.append(vals)
    return list(zip(*cols))


def export(directory, output):
    lines = ["#pragma once", "// Natywne krzywe Synfig. Eksport: tools/media/traveler_performance.py.",
             "namespace ember::hero_secondary {",
             "struct Key {float "+",".join(CHANNELS)+";};",
             "struct Clip {const Key* keys;int frames;float duration;bool loop;};"]
    for name, (count, loop, _) in CLIPS.items():
        rows=read_controls(directory / (name+".sifz"),count,loop)
        lines.append(f"inline constexpr Key {name}Keys[]={{")
        lines.extend(" {"+",".join(f"{v:.6f}f" for v in row)+"}," for row in rows)
        lines.extend(["};",f"inline constexpr Clip {name}={{{name}Keys,{count+1},{count}/60.f,{str(loop).lower()}}};"])
    output.write_text("\n".join(lines+["}",""]),encoding="utf-8")


def render_and_check(source, directory):
    count = CLIPS[source.stem][0]
    directory.mkdir()
    start = time.perf_counter()
    synfig_cli._run(["-i",source,"-T","2","-t","png","-o",directory/"frame.png","--sequence-separator","_",
                     "-a","1","--quiet"], directory, "render", cwd=source.parent)
    elapsed = time.perf_counter() - start
    images = sorted(directory.glob("frame_*.png"))
    assert len(images) == count, (source, len(images), count)
    unique = [set() for _ in DIRECTIONS]
    for path in images:
        with Image.open(path) as im:
            im.load()
            assert im.size == (440,216)
            for i, variants in enumerate(unique):
                x,y = i%4*110,i//4*105
                frame = im.crop((x,y,x+110,y+105)).convert("RGBA")
                assert frame.getchannel("A").getbbox(), (path,i)
                variants.add(hashlib.sha256(frame.tobytes()).digest())
    assert all(len(arr)>3 for arr in unique), (source, list(map(len,unique)))
    return dict(clip=source.stem,frames=count,directions=8,unique_directional_frames=list(map(len,unique)),
                synfig_render_s=round(elapsed,3),sha256=synfig_cli.sha(source))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--directory",type=Path,required=True)
    parser.add_argument("--header",type=Path,required=True)
    parser.add_argument("--run-gait-header",type=Path,help="Export the foot contacts, hip and spine controls of the run to C++")
    parser.add_argument("--export-only",action="store_true")
    parser.add_argument("--refresh-from",type=Path,help="Regenerate the eight-view studies using an existing edited master directory")
    parser.add_argument("--render",action="store_true")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    args.directory.mkdir(exist_ok=True,parents=True)
    if not args.export_only:
        prepare_parts(args.directory/"parts")
        for name,(count,loop,_) in CLIPS.items():
            edited=read_controls(args.refresh_from/(name+".sifz"),count,loop) if args.refresh_from else None
            gait=read_run_gait(args.refresh_from/"run.sifz") if name=="run" and args.refresh_from else None
            author(args.directory/(name+".sifz"),name,edited,gait)
    export(args.directory,args.header)
    if args.run_gait_header:
        joints.write_clip(read_run_gait(args.directory/"run.sifz"),args.run_gait_header,"run_gait_synfig","run.sifz")
    if args.render:
        results=[]
        for name in CLIPS:
            result=render_and_check(args.directory/(name+".sifz"),args.directory/("render-"+name))
            results.append(result)
            print(json.dumps(result),flush=True)
        receipt=dict(status="PASS",version=synfig_cli.version(args.directory/"version"),audio=False,
                     header_sha256=synfig_cli.sha(args.header),clips=results,
                     synfig_render_s=round(sum(r["synfig_render_s"] for r in results),3))
        (args.directory/"motion-receipt.json").write_text(json.dumps(receipt,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main()

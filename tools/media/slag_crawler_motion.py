"""Author, compile and verify the planted walk and combat poses of the slag crawler."""
import argparse
import json
import math
import shutil
import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np
from PIL import Image

from ember_drake_synfig import FACINGS, GRID, MARGIN, NAMES, cutout, group, read_keys, track, verify_views
from wolf_motion_synfig import walking_foot
import synfig_cli


PARTS = ("tail", "hind-left", "hind-right", "shell-head", "fore-left", "fore-right")
ORDER = ((0,1,4,3,2,5),(0,1,4,3,2,5),(0,1,2,3,4,5),(0,2,5,3,1,4),
         (0,2,5,3,1,4),(5,2,4,3,0,1),(4,5,3,1,2,0),(4,1,5,3,0,2))
FEET = (1, 2, 4, 5)
FOOT_PHASE = {1:0, 2:.5, 4:.545, 5:.045}
STRIDE = 34
CLIPS = {
    "idle": (20, tuple(range(0, 41, 4))),
    "walk": (40, tuple(range(0, 41, 2))),
    "bite": (40, (0, 5, 10, 14, 16, 18, 20, 23, 27, 32, 36, 40)),
    "bank": (20, (0, 4, 8, 12, 16)),
    "fall": (40, (0, 2, 4, 6, 8, 10, 12, 15, 18, 20)),
    "pivot_cw": (32, tuple(range(0, 33, 2))),
    "pivot_ccw": (32, tuple(range(0, 33, 2))),
}


def anatomy(root):
    pivots, toes = {}, {}
    for d, name in enumerate(NAMES):
        points = []
        for label in PARTS:
            with Image.open(root/"parts"/f"{name}-{label}.png") as image:
                alpha = np.asarray(image.convert("RGBA"))[:, :, 3]
                y, x = np.where(alpha > 110)
                if not len(x) or image.width != image.height:
                    raise ValueError(f"Missing opaque anatomy: {name}-{label}")
                points.append((np.stack((x+.5, y+.5), axis=1) * (128/image.width)))
        chest = np.median(points[3], axis=0)
        for p, cloud in enumerate(points):
            y = cloud[:, 1]
            if p in FEET:
                # The first part of each painted limb is tucked under a scute.
                root_points = cloud[y <= np.quantile(y, .17)]
            elif p == 0:
                distance = np.linalg.norm(cloud-chest, axis=1)
                root_points = cloud[distance <= np.quantile(distance, .12)]
            else:
                root_points = cloud
            pivots[p,d] = np.median(root_points, axis=0)
            toes[p,d] = np.median(cloud[y >= np.quantile(y, .94)], axis=0)
    return pivots, toes


def pose(clip, t, p, d, pivot, toe, center):
    cs, sn = math.cos(FACINGS[d]), math.sin(FACINGS[d])
    beat = math.tau*t
    foot = p in FEET
    if clip == "idle":
        breath = .5-.5*math.cos(beat)
        if foot:
            return 0, 0, 0, 1, 1
        if p == 0:
            return 0, -.25*breath, cs*1.65*math.sin(beat), 1, 1
        return 0, -.62*breath, .4*cs*math.sin(beat), 1, 1+.013*breath
    if clip == "bank":
        lean = 2*t-1
        if foot:
            # Weight transfer belongs to the trunk; the supporting claw stays put.
            return 0, 0, 0, 1, 1
        if p == 0:
            return .32*sn*lean, -.32*cs*lean, -7.5*lean, 1, 1
        return -.8*sn*lean, .33*cs*lean, 2.5*lean, 1, 1
    if clip.startswith("pivot_"):
        if not foot:
            return 0, 0, 0, 1, 1
        # The contact travels along a tangent to the footprint on the
        # isometric floor. Changing turn sense reverses that tangent, while
        # the airborne claw still lifts away from the floor in both directions.
        x, y = toe-center
        tangent = np.array((-y/.52,x*.52))
        tangent /= max(10,np.linalg.norm(tangent))
        travel, lift = walking_foot((t+FOOT_PHASE[p])%1)
        advance = travel*(.21 if clip=="pivot_cw" else -.21)
        return (tangent[0]*advance, tangent[1]*advance-lift*.60,
                -tangent[0]*advance*.32, 1, 1-.009*lift)
    if clip == "fall":
        # A short impulse on top of the game's impact-dependent roll. One
        # forepaw yields first, the rear feet trail, then the tail settles.
        delay = (.24, .14, .21, .09, .0, .07)[p]
        u = min(1., max(0., (t-delay)/(.94-delay)))
        weight = math.sin(math.pi*u)**2
        if foot:
            return (-cs*weight*1.45, -2.3*weight, cs*weight*(5.8 if p>=4 else -4.2),
                    1+.012*weight, 1-.08*weight)
        if p == 0:
            return 0, -weight, -5.4*weight, 1, 1
        return cs*.7*weight, 1.55*weight, cs*1.5*weight, 1+.015*weight, 1-.022*weight
    if clip == "bite":
        # Anticipation, contact at 45% (the game's actual hit), then recovery.
        stops = (0, .15, .32, .4, .45, .53, .66, .83, 1)
        reach = np.interp(t, stops, (0,-1.8,-3.1,-1.3,5.6,5.1,2.4,.4,0))
        dip = np.interp(t, stops, (0,.4,2.4,1.8,-1.1,-.65,.55,.14,0))
        if foot:
            front = p >= 4
            return (cs*reach*(.34 if front else .045), sn*reach*(.22 if front else .03),
                    cs*reach*(-.96 if front else .2), 1, 1+.012*dip)
        if p == 0:
            return cs*reach*.58, sn*reach*.4+dip*.4, -cs*(reach*.68+dip), 1, 1
        return cs*reach, sn*reach*.74+dip, cs*(.8*dip-.38*reach), 1+reach*.012, 1-dip*.012
    if foot:
        offset = FOOT_PHASE[p]
        travel, lift = walking_foot((t+offset) % 1.)
        # The stride is 34 projected pixels. During the 58% support interval
        # the planted toe travels back by exactly the ground covered by the body.
        travel *= STRIDE*.58/(2*17.6)
        lift *= .85 if p >= 4 else .72
        dx, dy = toe-pivot
        length = max(8, dy)
        angle = np.clip(-math.degrees(math.atan2(cs*travel, length)), -23, 23)
        sy = float(np.clip(1+(sn*travel-lift)/length, .75, 1.22))
        a = math.radians(angle)
        shift_x = cs*travel+dx-(math.cos(a)*dx-math.sin(a)*dy*sy)
        shift_y = sn*travel-lift+dy-(math.sin(a)*dx+math.cos(a)*dy*sy)
        return shift_x, shift_y, angle, 1, sy
    weight = math.sin(2*beat-.22)
    if p == 0:
        return cs*.35*weight, -.8*weight, math.sin(beat-.65)*3.9, 1, 1
    return -.3*sn*math.sin(beat), -.85*weight, cs*.8*math.sin(beat+.2), 1, 1


def author(root, clip, pivots, toes):
    fps, times = CLIPS[clip]
    canvas = ET.Element("canvas", version="1.2", width="512", height="256", fps=str(fps),
        **{"view-box":"0 0 10 -5", "antialias":"2", "begin-time":"0f", "end-time":f"{times[-1]}f"})
    ET.SubElement(canvas,"name").text = f"Żużlowy Pełzacz / {clip}"
    for d, name in enumerate(NAMES):
        facing = group(canvas, name)
        center = np.mean([toes[p,d] for p in FEET], axis=0)
        # On the far flank the shell hides the upper leg, on the near flank
        # the plated shoulder and claws are visible in front of it.
        for p in ORDER[d]:
            layer = group(facing, PARTS[p])
            px, py = pivots[p,d]
            cutout(layer, f"{name}-{PARTS[p]}", (-px/64,py/64,(128-px)/64,(py-128)/64))
            keys = [pose(clip, frame/times[-1], p, d, pivots[p,d], toes[p,d],center) for frame in times]
            track(layer, "stretch", "amount", "scale", times, [(k[3],k[4]) for k in keys])
            track(layer, "rotate", "amount", "turn", times, [-k[2] for k in keys])
            track(layer, "translate", "origin", "place", times,
                  [((k[0]+px)/64+d%4*GRID+MARGIN, -(k[1]+py)/64-d//4*GRID-MARGIN) for k in keys])
    ET.indent(canvas)
    ET.ElementTree(canvas).write(root/(clip+".sif"), encoding="utf-8", xml_declaration=True)


def compile_header(root, header, pivots):
    lines = ['#pragma once', '#include "ember_drake_motion.hpp"',
             '// Compiled from the native slag crawler Synfig projects.',
             'namespace ember::slag_crawler_motion {', 'using ember_drake_motion::Clip; using ember_drake_motion::Key;',
             'inline constexpr float pivots[8][6][2]={']
    for d in range(8):
        lines.append(" {"+",".join("{%.4ff,%.4ff}"%tuple(pivots[p,d]) for p in range(6))+"},")
    lines.append("};")
    for clip, (_, times) in CLIPS.items():
        frames = read_keys(root/(clip+".sif"), times, PARTS, lambda p,d:pivots[p,d])
        if (clip in ("idle","walk") or clip.startswith("pivot_")) and frames[0] != frames[-1]:
            raise ValueError("Open animation loop: "+clip)
        lines.append(f"inline constexpr unsigned char {clip}Times[]={{"+",".join(map(str,times))+"};")
        lines.append(f"inline constexpr Key {clip}Keys[][8][6]={{")
        for frame in frames:
            views = ["{"+",".join("{"+",".join(map(str,view[p]))+"}" for p in range(6))+"}" for view in frame]
            lines.append(" {\n  "+",\n  ".join(views)+"\n },")
        lines.extend(["};",f"inline constexpr Clip {clip}={{{clip}Keys,{clip}Times,{len(times)}}};"])
    header.write_text("\n".join(lines)+"\n}\n", encoding="utf-8")


def check_feet(root, pivots, toes):
    times = CLIPS["walk"][1]
    frames = read_keys(root/"walk.sif",times,PARTS,lambda p,d:pivots[p,d])
    max_slip = 0.
    clearances = []
    support_samples = 0
    for d, heading in enumerate(FACINGS):
        axis = np.array((math.cos(heading),math.sin(heading)))
        for p in FEET:
            vector = toes[p,d]-pivots[p,d]
            track = np.array([f[d][p] for f in frames])/(128,128,128,4096,4096)
            peak = 0.
            for t in np.arange(240)/240:
                stance = (t+FOOT_PHASE[p]) % 1
                x,y,angle,sx,sy = [np.interp(t*times[-1],times,col) for col in track.T]
                ang = math.radians(angle)
                dx,dy = vector*(1+sx,1+sy)
                shift = np.array((x+dx*math.cos(ang)-dy*math.sin(ang),
                                  y+dx*math.sin(ang)+dy*math.cos(ang)))-vector
                travel,lift = walking_foot(stance)
                expected = axis*travel*(STRIDE*.58/(2*17.6))
                if .025<stance<.55:
                    max_slip=max(max_slip,float(np.linalg.norm(shift-expected)))
                    support_samples+=1
                elif stance>.65:
                    peak=max(peak,float(expected[1]-shift[1]))
            clearances.append(peak)
    if max_slip>.55 or min(clearances)<2.0:
        raise ValueError(f"Planted toe drift {max_slip:.3f}; minimum swing height {min(clearances):.3f}")
    turns = [read_keys(root/(clip+".sif"), CLIPS[clip][1], PARTS, lambda p,d:pivots[p,d])
             for clip in ("pivot_cw","pivot_ccw")]
    pivot_clearance = min(max(-(a[d][p][1]+b[d][p][1])/256 for a,b in zip(*turns,strict=True))
                          for d in range(8) for p in FEET)
    if pivot_clearance < 2:
        raise ValueError(f"Claws drag during a pivot: {pivot_clearance:.2f}")
    return {"support_samples":support_samples,"max_planted_toe_drift_px":round(max_slip,3),
            "minimum_swing_lift_px":round(min(clearances),3),"minimum_pivot_lift_px":round(pivot_clearance,3)}


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("root", type=Path)
    cli.add_argument("--cells", type=Path, help="Import final full-cell RGBA artwork and author native clips")
    cli.add_argument("--header", type=Path)
    cli.add_argument("--renders", type=Path)
    cli.add_argument("--no-audio", action="store_true", required=True)
    args = cli.parse_args()
    if args.cells:
        (args.root/"parts").mkdir(parents=True,exist_ok=True)
        for d, name in enumerate(NAMES):
            for p, label in enumerate(PARTS):
                shutil.copyfile(args.cells/f"{4*d:02}-{p}.png", args.root/"parts"/f"{name}-{label}.png")
    pivots, toes = anatomy(args.root)
    if args.cells:
        for clip in CLIPS:
            author(args.root,clip,pivots,toes)
    if args.header:
        compile_header(args.root,args.header,pivots)
    if args.renders:
        receipts = {}
        args.renders.mkdir(parents=True,exist_ok=True)
        (args.renders/"feet.json").write_text(json.dumps(check_feet(args.root,pivots,toes),indent=2))
        for clip, (_, times) in CLIPS.items():
            receipts[clip] = synfig_cli.render(args.root/(clip+".sif"),output_root=args.renders,threads=2,
                                               expected_frames=times[-1]+1,require_motion=True)
        verify_views(receipts)
        (args.renders/"synfig.json").write_text(json.dumps(receipts,indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

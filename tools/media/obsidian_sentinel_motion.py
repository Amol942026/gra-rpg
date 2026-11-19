"""Author the warrior's eight-direction Synfig rig and compile its saved keyframes."""
import argparse
import json
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

import numpy as np
from PIL import Image
from scipy.interpolate import PchipInterpolator

from ember_drake_synfig import FACINGS, NAMES, cutout, group, read_keys, track, verify_views
from obsidian_sentinel_paint import PARTS
from obsidian_sentinel_turn import ORDER
from wolf_motion_synfig import walking_foot
import synfig_cli

CLIPS={"idle":(24,tuple(range(0,49,4))),"walk":(40,tuple(range(0,41,2))),
       "strike":(60,(0,4,8,12,16,20,22,23,24,25,26,27,28,29,31,34,38,44,52,60)),
       "cleave":(48,(0,6,12,18,24,30,36,40,42,43,44,45,46,47,48,49,50,52,54,58,64,73,83,96))}
# Wind, contact, follow-through, elbow flexion, hip travel, knee compression,
# rotation of breastplate, and shield counterweight. Impact is .45 / 1.05/2.08.
MARKS={
    "strike":((0,0,0,0,0,0,0,0,0),(.10,.26,0,0,18,-1.1,1.2,-4,-5),
              (.23,.8,0,0,42,-4.2,2.2,-11,-12),(.32,1,0,0,57,-5.8,2.6,-15,-18),
              (.36,1,0,0,56,-4.4,1.6,-11,-16),(.39,.82,.18,0,47,-1,.8,-3,-11),
              (.42,.48,.52,0,29,3.8,-.6,8,0),(.45,0,1,0,7,8,-1,15,9),
              (.50,0,.48,.52,-8,10,2.4,18,15),(.56,0,0,1,-14,10,3.1,17,18),
              (.65,0,0,.86,-10,7,2.5,10,12),(.8,0,0,.38,1,2.2,1,3,4),(1,0,0,0,0,0,0,0,0)),
    "cleave":((0,0,0,0,0,0,0,0,0),(.12,.26,0,0,21,-1.7,2,-4,-10),
              (.27,.75,0,0,47,-4,4,-12,-24),(.39,1,0,0,64,-6,4.6,-17,-27),
              (.445,1,0,0,64,-4.6,2.8,-12,-25),(.47,.87,.13,0,54,-.5,.4,-1,-18),
              (.49,.47,.53,0,33,5,-1.8,9,-5),(.505,0,1,0,10,10,-1.4,17,7),
              (.54,0,.36,.64,-8,12,2.5,20,21),(.60,0,0,1,-17,11.3,4.8,18,25),
              (.68,0,0,.92,-13,8.5,3.7,14,18),(.80,0,0,.52,.5,4.6,1.8,7,10),
              (1,0,0,0,0,0,0,0,0)),
}
GESTURE={name:PchipInterpolator(np.array(rows)[:,0],np.array(rows)[:,1:],axis=0) for name,rows in MARKS.items()}
STRIDE=32/1.1
LIFT=(-1,1,1,1,1,-1,-1,-1)
# The painted resting swords point in different screen directions. For each
# facing aim the edge at the opponent at contact, then let it cross the guard.
# Values are absolute deltas from the rest shoulder-to-blade direction.
SWING={
    "strike":((167,166,169,183,188,95,105,171),
              (41,-78,-55,22,44,172,158,41),(-28,-119,-105,-51,-32,226,214,-27)),
    "cleave":((183,184,188,194,200,99,111,184),
              (42,-72,-48,28,45,182,167,40),(-29,-121,-104,-35,-24,237,223,-30)),
}
ELBOW_Y=(63,63,63,62,62,60,63,63)
# The raising arm reaches outside the standing paint cell. Provide gutters
# between Synfig turntable views without shrinking the weapon's working arc.
GRID=3.8
MARGIN=(GRID-2)/2


def rotate(vec,angle):
    a=math.radians(angle)
    return np.array((vec[0]*math.cos(a)-vec[1]*math.sin(a),vec[0]*math.sin(a)+vec[1]*math.cos(a)))


def sample(name,t,p,d,joints,toes):
    f=np.array((math.cos(FACINGS[d]),math.sin(FACINGS[d])))
    angle=t*math.tau
    hand=p in (4,5)
    if name=="idle":
        breath=(1-math.cos(angle))*.5
        if p in (1,2):return 0,0,0,1,1
        sway=math.sin(angle)
        return -.12*f[1]*sway,-.78*breath,(.77*sway if hand else .42*sway)*LIFT[d],1,1+(.009*breath if p==3 else 0)
    if name=="walk":
        if p in (1,2):
            travel,clearance=walking_foot((t+(.5 if p==2 else 0))%1)
            travel*=STRIDE*.58/35.2
            clearance*=.83
            desired=f*travel-(0,clearance)
            leg=toes[p,d]-joints[d,p]
            turn=-math.degrees(math.atan2(desired[0],max(9,leg[1])))*.95
            sy=np.clip(1+desired[1]/max(10,leg[1]),.77,1.21)
            dx,dy=desired+leg-rotate(leg*(1,sy),turn)
            return dx,dy,turn,1,sy
        weight=math.sin(angle)
        vertical=1.18*math.cos(2*angle-.34)
        turn=-f[1]*1.8*weight+f[0]*1.2*math.sin(2*angle)
        if p==0:turn+=2.1*math.sin(angle-.6)
        if hand:
            turn+=(-1 if p==4 else 1)*(f[0]*5.8+f[1]*1.9)*math.sin(angle-.18)
        return -.65*f[1]*weight,vertical,turn,1,1
    wind,hit,finish,bend,advance,compression,twist,brace=GESTURE[name](t)
    center=joints[d,3]
    facing_twist=(1 if d in (0,1,2,7) else -1)*twist
    if p in (1,2):
        # Tilt each greave around its grounded sole, the pelvis can lean into the blow.
        leg=toes[p,d]-joints[d,p]
        turn=-f[0]*advance*(.91 if p==1 else .64)
        sy=1-compression*.014
        dx,dy=leg-rotate(leg*(1,sy),turn)
        return dx,dy,turn,1,sy
    turn=facing_twist
    # The chest drives the swing over the pelvis. The skirt follows the hips
    # rather than being pulled off the tops of the planted greaves.
    carry=.23 if p==0 else .53
    delta=f*advance*(carry,.68*carry)+(0,compression*(.46 if p==0 else .85))
    if hand or p==0:
        anchor=joints[d,p]-center
        delta+=rotate(anchor,turn)-anchor
    if p==5:
        prime,contact,through=SWING[name]
        # The elbow folds before the shoulder finishes loading, and extends
        # as the hips drive the sword forward. Their rotations add at the tip.
        turn=LIFT[d]*(prime[d]*wind+contact[d]*hit+through[d]*finish-bend)
    if p==4:turn+=brace*LIFT[d]*-.65
    if p==0:turn*=.67
    return *delta,turn,1,1


def elbow_angle(name,t,d):
    return GESTURE[name](t)[3]*LIFT[d]


def elbow_landmarks(parts):
    elbows=[]
    for d,key in enumerate(NAMES):
        a=np.asarray(Image.open(parts/(key+'-sword-arm.png')))[:,:,3]
        y,x=np.where(a>145)
        points=np.column_stack((x+.5,y+.5))*128/a.shape[0]
        joint=points[np.abs(points[:,1]-ELBOW_Y[d])<=1.25]
        elbows.append((np.median(joint[:,0]),ELBOW_Y[d]))
    return np.array(elbows)


def separate_sleeves(parts,elbows,blade):
    for d,key in enumerate(NAMES):
        im=np.array(Image.open(parts/(key+'-sword-arm.png')).convert('RGBA'))
        yy,xx=np.indices(im.shape[:2])
        points=(np.stack((xx,yy),axis=-1)+.5)*128/im.shape[0]-elbows[d]
        shaft=blade[d]-elbows[d]
        shaft/=np.linalg.norm(shaft)
        along=points@shaft
        for suffix,keep in (('upper-arm',along<2.8),('gauntlet-and-blade',along>=-1.6)):
            layer=im.copy();layer[~keep]=0
            Image.fromarray(layer).save(parts/(key+'-'+suffix+'.png'))


def landmarks(cells):
    source=json.loads((cells/"pivots.json").read_text())
    joints=np.array([source[d] for d in NAMES]);toes={};blade=[]
    for d,key in enumerate(NAMES):
        for p in (1,2,5):
            a=np.asarray(Image.open(cells/f"{key}-{PARTS[p]}.png"))[:,:,3]
            y,x=np.where(a>126)
            xy=np.column_stack((x+.5,y+.5))*128/a.shape[0]
            if p!=5:
                toes[p,d]=np.median(xy[y>=np.quantile(y,.94)],axis=0)
            else:
                dist=np.linalg.norm(xy-joints[d,p],axis=1)
                tip=np.mean(xy[dist>=np.quantile(dist,.986)],axis=0)
                target=joints[d,p]+(tip-joints[d,p])*.85
                blade.append(xy[np.argmin(np.linalg.norm(xy-target,axis=1))])
    return joints,toes,blade


def author(root,name,joints,toes,elbows):
    fps,frames=CLIPS[name]
    canvas=ET.Element("canvas",version="1.2",width="768",height="384",fps=str(fps),
                      **{"view-box":f"0 0 {GRID*4:g} {-GRID*2:g}","antialias":"2","begin-time":"0f","end-time":f"{frames[-1]}f"})
    ET.SubElement(canvas,"name").text="Obsydianowy wojownik / "+name
    for d,key in enumerate(NAMES):
        facing=group(canvas,key)
        for p in ORDER[d]:
            layer=group(facing,PARTS[p]);px,py=joints[d,p]
            if p==5 and name in GESTURE:
                ex,ey=elbows[d]
                forearm=group(layer,"gauntlet")
                cutout(forearm,key+"-gauntlet-and-blade",(-ex/64,ey/64,(128-ex)/64,(ey-128)/64))
                track(forearm,"rotate","amount","elbow",frames,[-elbow_angle(name,i/frames[-1],d) for i in frames])
                track(forearm,"translate","origin","joint",frames,[((ex-px)/64,(py-ey)/64)]*len(frames))
                cutout(layer,key+"-upper-arm",(-px/64,py/64,(128-px)/64,(py-128)/64))
            else:
                cutout(layer,key+"-"+PARTS[p],(-px/64,py/64,(128-px)/64,(py-128)/64))
            poses=[sample(name,i/frames[-1],p,d,joints,toes) for i in frames]
            track(layer,"stretch","amount","scale",frames,[r[3:] for r in poses])
            track(layer,"rotate","amount","turn",frames,[-r[2] for r in poses])
            track(layer,"translate","origin","place",frames,[((r[0]+px)/64+d%4*GRID+MARGIN,-(r[1]+py)/64-d//4*GRID-MARGIN) for r in poses])
    ET.indent(canvas);ET.ElementTree(canvas).write(root/(name+".sif"),encoding="utf-8",xml_declaration=True)


def compile_header(root,header,joints,blade,elbows):
    def pair(pt):return "{%.4ff,%.4ff}"%tuple(pt)
    lines=['#pragma once','#include "ember_drake_motion.hpp"','namespace ember::obsidian_sentinel_motion {',
           'using ember_drake_motion::Clip; using ember_drake_motion::Key;',
           'inline constexpr int order[8][6]={'+','.join('{'+','.join(map(str,o))+'}' for o in ORDER)+'};',
           'inline constexpr float bladeGlow[8][2]={'+','.join(pair(pt) for pt in blade)+'};',
           'inline constexpr float elbows[8][2]={'+','.join(pair(pt) for pt in elbows)+'};',
           'inline constexpr float pivots[8][6][2]={']
    lines+=[' {'+','.join(pair(pt) for pt in row)+'},' for row in joints]
    lines.append('};')
    for name,(_,frames) in CLIPS.items():
        keys=read_keys(root/(name+'.sif'),frames,PARTS,lambda p,d:joints[d,p],GRID,MARGIN)
        if keys[0]!=keys[-1]:raise ValueError("Open motion: "+name)
        lines+=['inline constexpr unsigned char '+name+'Times[]={'+','.join(map(str,frames))+'};',
                'inline constexpr Key '+name+'Keys[][8][6]={']
        for frame in keys:
            lines+=[' {']+['  {'+','.join('{'+','.join(map(str,k[p]))+'}' for p in range(6))+'},' for k in frame]+[' },']
        lines+=['};',f'inline constexpr Clip {name}={{{name}Keys,{name}Times,{len(frames)}}};']
        if name in GESTURE:
            canvas=ET.parse(root/(name+'.sif'))
            bends=[]
            for view in canvas.getroot().findall("layer"):
                marks=view.findall(".//layer[@desc='elbow']/param/animated/waypoint")
                assert [p.get('time') for p in marks]==[f'{f}f' for f in frames]
                bends.append([-round(float(p.find('angle').get('value'))*128) for p in marks])
            lines+=['inline constexpr short '+name+'Elbow[][8]={',
                    *[' {'+','.join(map(str,values))+'},' for values in zip(*bends)],'};']
    header.write_text('\n'.join(lines)+'\n}\n')


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("cells",type=Path);parser.add_argument("root",type=Path);parser.add_argument("header",type=Path)
    parser.add_argument("--render",type=Path);parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    parts=args.root/"parts";parts.mkdir(exist_ok=True,parents=True)
    joints,toes,blade=landmarks(args.cells)
    for image in args.cells.glob("*.png"):shutil.copyfile(image,parts/image.name)
    elbows=elbow_landmarks(parts)
    separate_sleeves(parts,elbows,blade)
    for name in CLIPS:author(args.root,name,joints,toes,elbows)
    compile_header(args.root,args.header,joints,blade,elbows)
    if args.render:
        receipts={};args.render.mkdir(exist_ok=True,parents=True)
        for name,(_,frames) in CLIPS.items():
            receipts[name]=synfig_cli.render(args.root/(name+".sif"),output_root=args.render,threads=2,
                                             expected_frames=frames[-1]+1,require_motion=True)
        verify_views(receipts)
        (args.render/"synfig.json").write_text(json.dumps(receipts,indent=2))

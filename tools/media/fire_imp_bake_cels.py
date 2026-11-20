"""Bake coherent directional cels of the six articulated Fire Imp cutouts.

The canonical paint and its editable layer masks are inputs. Each intermediate
is constructed around anatomical landmarks; eyes, jaw and limb joints stay
attached to the same features between illustrations.
"""
import argparse
import json
import tempfile
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageOps
from scipy import ndimage as ndi
from scipy.ndimage import distance_transform_edt
from scipy.spatial import cKDTree

from fire_imp_paint import SOURCES, FEET_CENTER, KEY_TO_SOURCE, tail_curve
from fire_imp_half_views import SOURCE as EXTRA_SOURCE, export as export_extra
from caldera_turn_registration import optical, remap, swatch, xy

SCALE = 2
CELL = 160
STEPS = 12
# Correspondences on the source illustrations in the 160px canvas. Across the
# front we follow the anatomical sides. A far eye disappears behind the nasal
# bridge and the rear views follow its temple landmark instead.
FACE = {
 0: [(66,11),(75,9),(75,18),(93,19),(61,24),(117,32),(96,30),(114,34),(113,36),(119,35),(122,38),(117,44),(113,49),(90,44),(81,51),(82,57),(75,65),(83,72),(74,85),(67,96),(83,98)],
 1: [(67,10),(80,9),(73,24),(91,22),(56,22),(111,28),(86,42),(107,41),(94,47),(106,46),(104,52),(98,57),(97,62),(80,54),(69,53),(84,63),(66,65),(81,73),(73,85),(65,97),(83,100)],
 2: [(69,9),(90,9),(67,23),(89,23),(50,25),(110,25),(68,43),(91,43),(72,48),(87,48),(80,51),(80,59),(80,64),(65,54),(63,55),(96,56),(67,67),(88,71),(79,86),(69,99),(89,100)],
 7: [(80,10),(90,9),(84,20),(106,19),(78,28),(125,30),(112,32),(123,31),(128,35),(130,34),(131,39),(125,45),(124,47),(104,45),(91,54),(114,53),(94,68),(106,67),(94,85),(88,96),(103,100)],
 6: [(61,10),(83,9),(59,23),(89,22),(43,25),(105,25),(66,34),(85,34),(66,39),(86,39),(75,42),(75,47),(75,51),(62,46),(61,54),(91,55),(63,67),(85,69),(75,86),(65,100),(87,100)]
}
# Traced central fold of elbows on the same paintings, ordered anatomically.
ELBOW = np.float32([[[70,69],[97,82]],[[46,72],[101,79]],[[46,75],[109,73]],[[59,79],[114,72]],
 [[63,82],[90,69]],[[39,74],[88,69]],[[45,72],[102,72]],[[72,69],[121,74]]])


def face_points(direction, towards):
    k=KEY_TO_SOURCE.get(direction,direction)
    a=np.float32(FACE[k])
    if direction in KEY_TO_SOURCE or (direction,towards) in ((2,3),(6,5)):
        a[:,0]=159-a[:,0]
    return a


def hi_source(reference):
    views={}
    for d,box in SOURCES.items():
        im=np.array(reference.crop(box).convert("RGBA"))
        _, labels, stats, _ = cv2.connectedComponentsWithStats((im[...,3]>75).astype(np.uint8))
        mask=cv2.dilate(np.uint8(labels==1+stats[1:,cv2.CC_STAT_AREA].argmax()),np.ones((5,5),np.uint8),iterations=2)
        im[...,3]=np.uint8(np.where(mask,(im[...,3].astype(float)-6)*(255/244),0).clip(0,255))
        im=Image.fromarray(im);im=im.crop(im.getbbox())
        w=round(im.width*135/im.height)
        left=round(80-w*FEET_CENTER[d])
        buf=Image.new('RGBA',(CELL*SCALE,CELL*SCALE))
        buf.paste(im.resize((w*SCALE,135*SCALE),Image.Resampling.LANCZOS),(left*SCALE,9*SCALE))
        views[d]=buf
    for d,base in KEY_TO_SOURCE.items():views[d]=ImageOps.mirror(views[base])
    return views


def pigment(im,ref):
    a=np.asarray(im); full=np.asarray(ref,np.float32)
    ink=np.asarray(im,dtype=np.float32)
    raw=cv2.resize(full[...,:3],(CELL,CELL),interpolation=cv2.INTER_AREA)
    error=np.max(abs(raw-ink[...,:3]),axis=-1)
    keep=np.clip(1-(error-8)/22,0,1)*(a[...,3]>100)
    mask=np.uint8(a[...,3]>30)
    inv=distance_transform_edt(~mask.astype(bool),return_distances=False,return_indices=True)
    col=a[tuple(inv)][...,:3].astype(np.float32)
    sharp=cv2.resize(col,None,fx=SCALE,fy=SCALE,interpolation=cv2.INTER_CUBIC)
    alpha=cv2.resize(ink[...,3],None,fx=SCALE,fy=SCALE,interpolation=cv2.INTER_CUBIC).clip(0,255)
    w=cv2.resize(keep,None,fx=SCALE,fy=SCALE,interpolation=cv2.INTER_LINEAR)[...,None]*np.clip((full[...,3:]-140)/90,0,1)
    rgb=sharp+w*(full[...,:3]-sharp)
    return np.dstack((rgb,alpha)).clip(0,255).astype(np.float32)


def rig_all(marks):
    d={int(k):np.float32(v['rig']) for k,v in marks.items()}
    for k,base in KEY_TO_SOURCE.items():
        d[k]=d[base][[0,2,1,3,5,4,7,6]].copy()
        d[k][:,0]=160-d[k][:,0]
    return d


def points(p,d,other,low,rig):
    if p==3:return face_points(d,other)
    if p==0:return np.float32(tail_curve(d)[np.linspace(0,219,14).astype(int)])
    if p>=4:return np.float32([rig[d][p],ELBOW[d,p-4],rig[d][p+2]])
    ys,xs=np.where(low[...,3]>=120)
    coords=[rig[d][p]]
    for t in [.39,.62,.86,.97]:
        y=np.quantile(ys,t); xx=xs[abs(ys-y)<2.3]
        coords.append([np.median(xx),y])
    return np.float32(coords)


def rigid_field(grid,target,source):
    # Local weighted similarity: the two eyes, for example, move with the
    # bridge of the nose while preserving the strokes on the brow. No pixel
    # is traced through optical-flow matches from a horn into the chest.
    p=grid.reshape(-1,2)
    dist,idx=cKDTree(target).query(p,k=min(len(target),7))
    w=(2.1+dist**2)**-1.17
    w/=np.sum(w,axis=1,keepdims=True)
    a,b=target[idx],source[idx]
    ca=np.einsum('nk,nki->ni',w,a); cb=np.einsum('nk,nki->ni',w,b)
    a=a-ca[:,None,:];b=b-cb[:,None,:]
    dot=np.einsum('nk,nki,nki->n',w,a,b)
    cross=np.sum(w*(a[...,0]*b[...,1]-a[...,1]*b[...,0]),axis=1)
    size=np.maximum(.01,np.einsum('nk,nki,nki->n',w,a,a))
    mag=np.maximum(.01,np.hypot(dot,cross))
    zoom=np.clip(mag/size,.78,1.27)
    c=dot/mag*zoom;s=cross/mag*zoom
    q=p-ca
    mapped=np.stack((q[:,0]*c-q[:,1]*s,q[:,0]*s+q[:,1]*c),axis=1)+cb
    return np.float32(mapped.reshape(grid.shape))


def extent(a):
    y,x=np.where(a[...,3]>1)
    return (int(x.min()),int(y.min()),int(x.max()+1),int(y.max()+1))


def bake(a,b,pa,pb,t,p):
    x0,y0,x1,y1=extent(a);u0,v0,u1,v1=extent(b)
    left=max(0,min(x0,u0)-10);top=max(0,min(y0,v0)-10)
    right=min(CELL*SCALE,max(x1,u1)+10);bottom=min(CELL*SCALE,max(y1,v1)+10)
    y,x=np.mgrid[top:bottom,left:right].astype(np.float32)
    grid=np.stack(((x+.5)/SCALE-.5,(y+.5)/SCALE-.5),axis=-1)
    target=(1-t)*pa+t*pb
    ca=rigid_field(grid,target,pa);cb=rigid_field(grid,target,pb)
    def mapped(im,xy):
        return cv2.remap(im,(xy+.5)*SCALE-.5,None,cv2.INTER_CUBIC,borderMode=cv2.BORDER_REPLICATE).clip(0,255)
    aa=mapped(a,ca);bb=mapped(b,cb)
    def sdf(alpha):
        inside=alpha>=127.5
        field=np.where(inside,distance_transform_edt(inside)-.5,.5-distance_transform_edt(~inside)).astype(np.float32)
        soft=(alpha>6)&(alpha<249)
        field[soft]=alpha[soft]/255-.5
        return field
    da,db=sdf(aa[...,3]),sdf(bb[...,3])
    alpha=np.clip(.5+(1-t)*da+t*db,0,1)
    # Each already posed cel owns a single silhouette. Keep pigment from
    # outside the source cutout out of the fill, particularly at the jaw.
    color=registered_pigment(aa,bb,da,db,t)
    result=np.zeros((CELL*SCALE,CELL*SCALE,4),np.uint8)
    result[top:bottom,left:right]=np.rint(np.dstack((color.clip(0,255),alpha*255))).astype(np.uint8)
    result[result[...,3]==0,:3]=0
    return result
# Use semantically paired points on both originals of each short arc. The eye
# disappears at the outer contour when the shoulders turn away from camera.
FACE[0][8:10]=[(112.7,35.3),(118,35)]
FACE[1][8:10]=[(93.1,45.5),(107,45)]
FACE[2][8:10]=[(73,47),(88.7,47)]
FACE[7][8:10]=[(128,34.4),(131,35)]
FACE[6][6:13]=[(92,33),(96,32),(96.1,37),(98,38),(97.5,40),(95,43),(91,46)]
FACES={
0:[(68,11),(77,10),(78,20),(96,20),(64,28),(117,37),(103,37),(114,37),(110.5,40.6),(116.5,41),(118,44),(114,49),(111,54),(91,49),(79,55),(94,63),(70,69),(85,74),(79,85),(64,98),(85,99)],
1:[(68,11),(86,10),(71,23),(94,22),(56,25),(110,29),(86,38),(104,37),(93,43),(104,43),(100,47),(99,54),(97,60),(79,52),(61,55),(95,56),(66,66),(91,69),(78,85),(66,99),(88,100)],
7:[(73,12),(82,11),(82,21),(99,18),(72,29),(112,36),(108,30),(117,32),(114,36),(118,35),(119,37),(116,45),(114,50),(95,47),(76,55),(100,56),(75,67),(92,70),(82,85),(68,98),(91,100)],
6:[(63,12),(75,11),(70,22),(88,19),(51,27),(100,32),(95,31),(104,32),(102.8,35.8),(105,35),(106,39),(102,44),(99,49),(82,46),(62,53),(92,53),(67,67),(87,68),(77,85),(64,99),(90,99)]}
MIRROR={2:1,3:0,4:7,5:6}

def extend(im):
    pixels=np.array(im.convert("RGBA"),np.float32)
    matte=pixels[...,3]<55
    nearest=ndi.distance_transform_edt(matte,return_distances=False,return_indices=True)
    pixels[...,:3]=np.where(matte[...,None],pixels[tuple(nearest)][...,:3],pixels[...,:3])
    return pixels

def midparts(k,root,data):
    base=MIRROR.get(k,k)
    record=data[str(base)]
    mirror=base!=k
    order=(0,2,1,3,5,4) if mirror else range(6)
    images=[]
    landmarks=[]
    for p in order:
        im=Image.open(root/f'{base}-{p}-2x.png').convert('RGBA')
        if mirror:im=im.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
        images.append(extend(im))
        if p==0:
            tail=np.array(record["tail"])
            op=np.column_stack([np.interp(np.linspace(0,1,14),np.linspace(0,1,len(tail)),col) for col in tail.T])
        elif p==3:op=np.array(FACES[base],float)
        elif p>=4:
            op=np.float32([record["rig"][p],record["elbows"][p-4],record["rig"][p+2]])
        else:
            low=np.asarray(im.resize((160,160)))
            ys,xs=np.where(low[...,3]>=120)
            op=[np.float32(record["rig"][p])]
            for t in (.39,.62,.86,.97):
                y=np.quantile(ys,t);x=np.median(xs[abs(ys-y)<2.3])
                op.append([159-x if mirror else x,y])
            op=np.float32(op)
        op=np.array(op,np.float32)
        if mirror:op[:,0]=159-op[:,0]
        landmarks.append(op)
    return images,landmarks


def registered_pigment(a, b, da, db, t):
    near, far, dn, df = (a, b, da, db) if t <= .5 else (b, a, db, da)
    distance = min(t, 1 - t)
    # Move the diffuse pigment through the full interval. Only one drawing
    # supplies the high-frequency detail, so a seam or claw keeps one edge.
    weight_a = np.clip(da + 1, 0, 1)
    weight_b = np.clip(db + 1, 0, 1)
    def diffuse(rgb, coverage):
        blur = lambda x: cv2.GaussianBlur(x, (0, 0), 3.2 * SCALE)
        return blur(rgb * coverage[..., None]) / np.maximum(blur(coverage)[..., None], .01)
    first = diffuse(near[..., :3], weight_a if t <= .5 else weight_b)
    second = diffuse(far[..., :3], weight_b if t <= .5 else weight_a)
    correction = second-first
    valid = (np.abs(correction).max(axis=2) < 160) & (df > -3 * SCALE)
    color = near[..., :3] + distance * correction * valid[...,None]
    reveal = (dn < -.5) & (df > .5)
    color[reveal] = far[..., :3][reveal]
    return color


def pack(source,destination,header,steps,register_flow=True):
    scale=1.5
    entries=[]
    for index in range(8*steps):
        for part in range(6):
            im=Image.open(source/f'{index:02}-{part}.png').convert('RGBA').resize((240,240),Image.Resampling.LANCZOS)
            alpha=np.array(im.getchannel('A'))
            alpha[alpha<3]=0
            im.putalpha(Image.fromarray(alpha))
            box=im.getbbox()
            ys,xs=np.where(alpha)
            weights=alpha[ys,xs].astype(np.float64)
            center=(round(64*np.average(xs+.5,weights=weights)),round(64*np.average(ys+.5,weights=weights)))
            entries.append((index,part,im.crop(box),box[:2],center))
    bottom=0;rows=[];records={}
    page=Image.new('RGBA',(2048,2048))
    for d,p,im,origin,center in sorted(entries,key=lambda e:-e[2].height):
        w,h=im.width+4,im.height+4
        row=next((r for r in rows if r[2]>=h and r[1]+w<=2048),None)
        if row is None:
            row=[bottom,0,h];rows.append(row);bottom+=h
        if bottom>2048:raise ValueError(f'atlas over budget {bottom}')
        x,y=row[1]+2,row[0]+2
        records[d,p]=(x,y,*im.size,*origin,*center)
        page.paste(im,(x,y))
        row[1]+=w
    destination.parent.mkdir(exist_ok=True,parents=True)
    page.crop((0,0,2048,bottom)).save(destination,optimize=True)
    if register_flow:
        save_registration(page, records, steps, destination.with_name("cel-flow.png"))
    header.parent.mkdir(exist_ok=True,parents=True)
    text=['#pragma once','namespace ember::fire_imp_cels {',
          f'inline constexpr int Steps={steps}, Count=8*Steps;',
          'inline constexpr float RasterScale=1.5f;',
          'struct Cel {short x,y,w,h,ox,oy,cx64,cy64;};','inline constexpr Cel sheet[Count][6]={']
    text+=['  {'+','.join('{'+','.join(map(str,records[d,p]))+'}' for p in range(6))+'},' for d in range(8*steps)]
    header.write_text('\n'.join(text+['};','}'])+'\n')
    print(destination,bottom,flush=True)


def save_registration(page, records, steps, destination):
    # Register adjacent drawings only, at most a few degrees apart. The atlas
    # carries one signed motion vector per texel; the shader follows it in
    # opposite directions towards the two neighboring painted frames.
    grid = 16
    count = steps * 8
    result = np.full((grid * 6, grid * count, 3), 128, dtype=np.uint8)
    for part in range(6):
        guides = []
        for cel in range(count):
            x, y, w, h, *_ = records[cel, part]
            guides.append(swatch(page.crop((x, y, x + w, y + h)))[1])
        for cel, before in enumerate(guides):
            after = guides[(cel + 1) % count]
            forward = optical(before, after)
            backward = optical(after, before)
            u = forward.copy()
            v = backward.copy()
            for _ in range(3):
                u = remap(forward, xy - .5 * u)
                v = remap(backward, xy - .5 * v)
            delta = cv2.resize(.5 * (u - v), (grid, grid), interpolation=cv2.INTER_AREA)
            delta = np.clip(delta, -16, 16)
            pixels = np.rint(128 + 3 * delta).astype(np.uint8)
            result[part*grid:(part+1)*grid, cel*grid:(cel+1)*grid, :2] = pixels
    Image.fromarray(result).save(destination, optimize=True)

REFERENCE=Path("art/volcano/fire-imp/redesign/references/sunburst-turnaround.png")
BODY=Path("assets/volcano/fire-imp.png")
MARKERS=Path("art/volcano/fire-imp/paint/landmarks.json")


def bake_atlas(work, atlas, header, painted_only=False):
    extra = work/"half-views"
    export_extra(extra)
    dat=json.loads((EXTRA_SOURCE/"rig.json").read_text("utf-8"))
    art=hi_source(Image.open(REFERENCE).convert("RGBA"))
    sheet=Image.open(BODY).convert("RGBA")
    rig=rig_all(json.loads(MARKERS.read_text("utf-8")))
    originals={}
    low={}
    for d in range(8):
        for p in range(6):
            im=sheet.crop((d*160,p*160,(d+1)*160,(p+1)*160));low[d,p]=np.asarray(im)
            originals[d,p]=pigment(im,art[d])
    cv2.setNumThreads(2)
    steps = 2 if painted_only else STEPS
    for k in range(8):
        n=(k+1)%8
        keys,midpoints=midparts(k,extra,dat)
        end_a = [points(p,k,n,low[k,p],rig) for p in range(6)]
        end_b = [points(p,n,k,low[n,p],rig) for p in range(6)]
        for sub in range(steps):
            t=sub/steps
            idx=k*steps+sub
            for p in range(6):
                if sub==0:
                    full=originals[k,p]
                elif 2*sub==steps:
                    full=keys[p]
                else:
                    if t<.5:
                        start,stop=originals[k,p],keys[p]
                        pa,pb=end_a[p],midpoints[p]
                    else:
                        start,stop=keys[p],originals[n,p]
                        pa,pb=midpoints[p],end_b[p]
                    full=bake(start,stop,pa,pb,(t*2)%1,p)
                im=Image.fromarray(np.uint8(np.rint(full).clip(0,255)))
                im.save(work/f'{idx:02}-{p}.png')
    pack(work, atlas, header, steps, register_flow=not painted_only)

if __name__ == "__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("atlas", type=Path)
    cli.add_argument("header", type=Path)
    cli.add_argument("--painted-only", action="store_true")
    cli.add_argument("--no-audio", action="store_true", required=True)
    args = cli.parse_args()
    with tempfile.TemporaryDirectory(prefix="emberfall-imp-") as tmp:
        bake_atlas(Path(tmp), args.atlas, args.header, args.painted_only)

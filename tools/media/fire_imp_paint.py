"""Build the imp's articulated turn, keeping all five painted keys and six cutouts editable."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageOps
from scipy import ndimage as nd
from scipy.interpolate import RBFInterpolator
from scipy.interpolate import PchipInterpolator
from scipy.spatial import cKDTree

from ember_drake_turn import CELL, UV, SAMPLE, Paint, inbetween, sample
from slag_crawler_paint import cut, FILM
from caldera_corpse_paint import cell_paint


SOURCES = {0:(0,0,510,505),1:(510,0,990,506),2:(1010,0,1536,510),
           7:(0,512,488,1024),6:(504,512,990,1024)}
FEET_CENTER = {0:.57,1:.59,2:.55,7:.44,6:.52}
KEY_TO_SOURCE = {3:1,4:0,5:7}
LABELS = ('Ogon','Lewa noga','Prawa noga','Głowa i tułów','Lewa ręka','Ręka rzucająca')

# Keep the belt and the articulated shoulder roots. The tail barb and wrists
# visible beyond these contours belong exclusively to their own paintings.
TORSO = {
    0: ((48,4),(128,4),(130,51),(98,53),(94,66),(91,86),(94,102),(76,104),(57,104),(59,86),(65,72),(69,56),(53,43)),
    1: ((46,4),(120,4),(116,41),(104,52),(102,65),(90,68),(91,82),(99,101),(82,107),(62,108),(53,101),(56,86),(60,74),(63,59),(49,44)),
    2: ((44,4),(117,4),(117,43),(103,57),(100,68),(92,73),(95,86),(101,100),(93,110),(66,110),(55,103),(59,90),(64,76),(60,64),(54,52),(45,42)),
    6: ((36,4),(112,4),(110,40),(97,54),(97,62),(91,68),(88,78),(92,88),(98,103),(91,112),(62,111),(52,104),(58,87),(62,77),(59,67),(54,62),(52,51),(41,42)),
    7: ((69,4),(137,4),(139,45),(122,50),(119,62),(111,67),(108,80),(114,95),(107,112),(85,110),(74,101),(77,87),(82,74),(77,65),(80,54),(69,44)),
}
# Center lines traced from the rump to the tip of each barb. Matching
# locations along the curve prevents two separate tails during the rear turn.
TAIL = {
    0: ((67,90),(61,97),(53,103),(44,105),(36,103),(29,96),(26,88),(24,78),(25,69),(28,62),(34,58),(40,54),(50,50)),
    1: ((63,91),(56,97),(48,99),(39,98),(30,95),(24,89),(21,81),(20,72),(21,64),(24,58),(30,53),(38,49),(48,44)),
    2: ((66,91),(60,98),(52,101),(42,100),(32,99),(25,93),(22,86),(21,77),(22,68),(26,60),(31,55),(39,50),(47,46)),
    6: ((79,89),(87,97),(94,102),(103,106),(112,105),(120,100),(125,93),(128,82),(127,72),(124,64),(120,57),(114,51),(104,44)),
    7: ((90,85),(83,93),(75,100),(62,105),(52,105),(44,102),(39,97),(35,88),(34,79),(35,71),(39,64),(46,58),(58,52)),
}


def tail_curve(key):
    source=KEY_TO_SOURCE.get(key,key)
    pts=np.array(TAIL[source],dtype=float)
    if key in KEY_TO_SOURCE:pts[:,0]=CELL-1-pts[:,0]
    u=np.linspace(0,1,len(pts))
    return PchipInterpolator(u,pts,axis=0)(np.linspace(0,1,220))


def tangents(curve):
    axis=np.gradient(curve,axis=0)
    axis/=np.maximum(1e-5,np.linalg.norm(axis,axis=1,keepdims=True))
    return axis,np.column_stack((-axis[:,1],axis[:,0]))


def tail_between(a,b,k,n,t):
    c0,c1=tail_curve(k),tail_curve(n)
    curve=c0*(1-t)+c1*t
    yy,xx=np.mgrid[:CELL,:CELL]
    grid=np.stack((xx,yy),axis=-1)
    dist,i=cKDTree(curve).query(grid)
    along,normal=tangents(curve)
    offset=grid-curve[i]
    u=np.sum(offset*along[i],-1)
    v=np.sum(offset*normal[i],-1)
    def warp(paint,line):
        x,y=tangents(line)
        pt=line[i]+x[i]*u[...,None]+y[i]*v[...,None]
        return sample(paint.color,pt[...,0],pt[...,1]),sample(paint.distance,pt[...,0],pt[...,1])
    rgb0,a0=warp(a,c0);rgb1,a1=warp(b,c1)
    alpha=np.clip(.5+(1-t)*a0+t*a1,0,1)*(dist<15)
    color=(1-t)*rgb0+t*rgb1
    return Image.fromarray(np.uint8(np.clip(np.dstack((color,alpha*255))+.5,0,255)))

# Correspondences of the two horns, ears, pupils, nostrils, jaw, neck and
# belt in the painted keys. The frontal key shows the eye on the side of the
# approaching three-quarter view. A rear view contains no painted eye.
HEAD = {
    0:((66,11),(75,9),(73,18),(83,14),(97,25),(104,24),(61,25),(104,27),
       (113,35),(117,34),(121,37),(114,49),(86,46),(75,54),(86,75),(76,86)),
    1:((67,11),(80,9),(66,23),(83,17),(86,36),(99,35),(56,22),(111,27),
       (94,47),(106,46),(103,51),(98,62),(83,55),(63,57),(90,62),(73,84)),
    2:((69,9),(90,9),(63,19),(94,19),(73,40),(86,40),(50,25),(110,25),
       (74,49),(87,49),(80,53),(79,64),(80,60),(61,56),(96,57),(78,86)),
    7:((80,12),(90,9),(83,19),(98,16),(105,30),(115,28),(78,28),(117,27),
       (127,35),(130,35),(132,38),(125,47),(104,44),(87,54),(111,54),(91,85)),
    6:((63,9),(85,9),(55,18),(91,18),(64,36),(85,36),(43,25),(105,24),
       (66,40),(83,40),(74,42),(75,48),(75,46),(56,57),(93,57),(75,85)),
}


def landmarks(key,other):
    original=KEY_TO_SOURCE.get(key,key)
    points=np.array(HEAD[original],dtype=float)
    if key in KEY_TO_SOURCE or key==2 and other==3 or key==6 and other==5:
        points[:,0]=CELL-1-points[:,0]
    return np.vstack((points,((2,2),(158,2),(2,149),(158,149),(77,152))))


def torso_between(a,b,k,n,t):
    left,right=landmarks(k,n),landmarks(n,k)
    target=left*(1-t)+right*t
    yy,xx=np.mgrid[:CELL,:CELL]
    grid=np.stack((xx,yy),axis=-1).astype(float)
    def warp(paint,points):
        offset=RBFInterpolator(target,points-target,kernel='thin_plate_spline',smoothing=1.1)
        xy=grid+offset(grid.reshape(-1,2)).reshape(grid.shape)
        return sample(paint.color,xy[...,0],xy[...,1]),sample(paint.distance,xy[...,0],xy[...,1])
    c0,d0=warp(a,left);c1,d1=warp(b,right)
    alpha=np.clip((1-t)*d0+t*d1+.5,0,1)
    # A shallow seam replaces repeating two sets of teeth or ridged horns.
    progress=np.clip((t-.12-.12*np.clip((xx-target[8,0])/35,-1,1))/.76,0,1)
    progress=progress*progress*(3-2*progress)
    w0=(1-progress)*np.clip(d0+1,.025,1);w1=progress*np.clip(d1+1,.025,1)
    rgb=(c0*w0[...,None]+c1*w1[...,None])/(w0+w1)[...,None]
    im=Image.fromarray(np.uint8(np.rint(np.dstack((np.clip(rgb,0,255),alpha*255)))))
    sharp=im.convert('RGB').filter(ImageFilter.UnsharpMask(radius=.73,percent=90,threshold=3))
    sharp.putalpha(im.getchannel('A'))
    return sharp


def keys(source, folder):
    folder.mkdir(exist_ok=True,parents=True)
    with Image.open(source) as ref:
        for key,box in SOURCES.items():
            pixels=np.asarray(ref.crop(box).convert('RGBA')).copy()
            alpha=pixels[...,3]
            connected,count=nd.label(alpha>75)
            sizes=np.bincount(connected.ravel(),minlength=count+1)
            sizes[0]=0
            keep=nd.binary_dilation(connected==sizes.argmax(),iterations=4)
            alpha[:]=np.where(keep,np.clip((alpha.astype(float)-6)*(255/244),0,255),0)
            im=Image.fromarray(pixels)
            im=im.crop(im.getbbox())
            height=135
            width=round(im.width*height/im.height)
            im=im.resize((width,height),SAMPLE)
            tile=Image.new('RGBA',(CELL,CELL))
            tile.paste(im,(round(80-width*FEET_CENTER[key]),144-height))
            tile.save(folder/f'key-{key}.png')


def guides(folder):
    font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',12)
    for key in SOURCES:
        src=Image.open(folder/f'key-{key}.png')
        tile=Image.new('RGBA',src.size,(39,35,41))
        tile.alpha_composite(src)
        tile=tile.resize((480,480))
        draw=ImageDraw.Draw(tile,'RGBA')
        for a in range(0,160,10):
            draw.line((a*3,0,a*3,480),fill=(235,230,226,52))
            draw.line((0,a*3,480,a*3),fill=(235,230,226,52))
            draw.text((a*3,2),str(a),font=font,fill='white')
            draw.text((2,a*3),str(a),font=font,fill='white')
        tile.save(folder/f'guide-{key}.png')


def restore_flank(body, donor, key):
    """Paint the exposed ribs and belt from the matching front or rear illustration."""
    # Traced edges of the shoulder, ribs, waistband and leather skirt. At the
    # oblique views, the resting arm used to hide nearest-colour extensions.
    footprint = {
        0: ((58,75,88),(65,69,88),(73,64,85),(82,63,84),(88,61,87),(98,64,87),(103,66,84)),
        1: ((57,66,73),(66,61,71),(74,58,69),(82,56,67),(87,55,66),(99,56,68),(104,59,66)),
        7: ((55,83,97),(65,80,94),(74,80,93),(83,79,94),(90,80,98),(100,82,98),(107,85,95)),
    }[key]
    ys, edges_l, edges_r = np.array(footprint).T
    py, px = np.mgrid[:CELL, :CELL]
    left, right = np.interp(py, ys, edges_l), np.interp(py, ys, edges_r)
    u = (px-left)/(right-left)
    if key==7:
        xx = 61+u*20
        yy = np.interp(py, ys, (56,64,72,81,89,100,108))
    else:
        xx = 65+u*19
        yy = np.interp(py, ys, (61,68,75,82,87,98,105))
    rgba=np.array(body)
    src=np.asarray(donor,dtype=float)
    coords=[yy,xx]
    paint=np.stack([nd.map_coordinates(src[...,ch],coords,order=1,mode='nearest') for ch in range(3)],axis=-1)
    # The flank faces away from the key light. Keep its painted folds legible
    # while the change feathers into the original armour at the shoulder.
    width=np.minimum(px-left,right-px)
    blend=np.minimum(np.clip(width/2.3,0,1),np.clip(np.minimum(py-ys[0],ys[-1]-py)/3,0,1))
    blend*=nd.map_coordinates(src[...,3],coords,order=1,mode='nearest')/255
    blend=np.where(rgba[...,3]>0,blend,0)[...,None]
    rgba[...,:3]=np.uint8(np.rint(rgba[...,:3]*(1-blend)+paint*(.78 if key==0 else .83)*blend))
    return Image.fromarray(rgba)


def prepare(root,marks):
    result={}
    for k in SOURCES:
        row=cut(Image.open(root/'keys'/f'key-{k}.png'),marks[str(k)])
        mask=Image.new('L',(CELL*3,CELL*3))
        ImageDraw.Draw(mask).polygon([(x*3,y*3) for x,y in TORSO[k]],fill=255)
        body=row[3]
        alpha=np.asarray(body.getchannel('A'),float)*np.asarray(mask.resize(body.size,SAMPLE))/255
        body.putalpha(Image.fromarray(np.uint8(np.rint(alpha))))
        result[k]=row
    for k,parts in result.items():
        for p,part in enumerate(parts):
            if part is not None:continue
            m=marks[str(k)][str(p)]
            donor=result[m['foreign']][p]
            x,y,w,h=m['xywh']
            frag=donor.crop(donor.getbbox()).resize((w,h),SAMPLE)
            if m.get('mirror'):frag=ImageOps.mirror(frag)
            layer=Image.new('RGBA',(CELL,CELL))
            layer.paste(frag,(x,y))
            parts[p]=layer
    for k in (0,1,7):
        result[k][3]=restore_flank(result[k][3],result[6 if k==7 else 2][3],k)
    for k,original in KEY_TO_SOURCE.items():
        mirror=[ImageOps.mirror(part) for part in result[original]]
        result[k]=[mirror[p] for p in (0,2,1,3,5,4)]
    return [result[i] for i in range(8)]


def material_input(root,cuts):
    frames=[]
    for row in cuts+cuts[:1]:
        im=Image.new('RGB',(UV*3,UV*2))
        for p,paint in enumerate(row):
            im.paste(Image.fromarray(np.uint8(np.rint(np.clip(paint.texture,0,255)))),
                     (p%3*UV,p//3*UV))
        frames.append(im)
    path=root/'imp-turn-materials.png'
    frames[0].save(path,save_all=True,append_images=frames[1:],duration=125,loop=0,disposal=0,blend=0)
    workflow={
        '1':{'class_type':'LoadImage','inputs':{'image':path.name}},
        '2':{'class_type':'FrameInterpolationModelLoader','inputs':{'model_name':FILM}},
        '3':{'class_type':'FrameInterpolate','inputs':{'images':['1',0],'interp_model':['2',0],'multiplier':4}},
        '4':{'class_type':'SaveImage','inputs':{'images':['3',0],'filename_prefix':'emberfall-imp/turn'}}}
    (root/'film-workflow.json').write_text(json.dumps(workflow,indent=2))


def views(root,marks,film):
    original=prepare(root,marks)
    cuts=[[Paint(part) for part in row] for row in original]
    if not film:
        material_input(root,cuts)
        return
    frames=sorted(film.glob('turn*.png'))
    if len(frames)!=33:raise ValueError(f'Expected 33 FILM frames, got {len(frames)}')
    out=root/'cells'
    out.mkdir(exist_ok=True)
    for d in range(32):
        k=d//4;n=(k+1)%8;t=d%4/4
        pigments=Image.open(frames[d]).convert('RGB')
        for p in range(6):
            if not t:
                im=original[k][p]
            elif p==0:
                im=tail_between(cuts[k][p],cuts[n][p],k,n,t)
            elif p==3:
                im=torso_between(cuts[k][p],cuts[n][p],k,n,t)
            else:
                u,v=p%3*UV,p//3*UV
                rgb=np.asarray(pigments.crop((u,v,u+UV,v+UV)),dtype=float)
                im=inbetween(cuts[k][p],cuts[n][p],t,rgb)
            im.save(out/f'{d:02}-{p}.png')


def pack(cells,destination,header,body,relief):
    tiles=[[Image.open(cells/f'{d:02}-{p}.png').convert('RGBA') for p in range(6)] for d in range(32)]
    frames_by_part=[]
    for p in range(6):
        boxes=[row[p].getchannel('A').getbbox() for row in tiles]
        if not all(boxes):raise ValueError(f'Missing painted layer {p}')
        full=(min(b[0] for b in boxes),min(b[1] for b in boxes),
              max(b[2] for b in boxes),max(b[3] for b in boxes))
        # The tail folds edge-on behind the body. Follow its whole profile as
        # it curls. The larger anatomical cutouts keep half the common frame
        # so a horn tip or a hoof never rescales the rest of the silhouette.
        frames_by_part.append(boxes if p==0 else [tuple(round((x+y)/2) for x,y in zip(b,full)) for b in boxes])
    entries=[]
    for d,row in enumerate(tiles):
        for p,part in enumerate(row):
            bounds=frames_by_part[p][d]
            entries.append((d,p,part.crop(bounds),bounds[:2]))
    width=1024;bottom=0;shelves=[];record={}
    for d,p,tile,origin in sorted(entries,key=lambda e:-e[2].height):
        w,h=tile.width+4,tile.height+4
        shelf=next((s for s in shelves if s[2]>=h and s[1]+w<=width),None)
        if shelf is None:
            shelf=[bottom,0,h];shelves.append(shelf);bottom+=h
        record[d,p]=(shelf[1]+2,shelf[0]+2,*tile.size,*origin)
        shelf[1]+=w
    if bottom>2048:raise ValueError('Exceeded atlas budget')
    page=Image.new('RGBA',(width,bottom))
    for d,p,tile,origin in entries:page.paste(tile,record[d,p][:2])
    destination.parent.mkdir(exist_ok=True,parents=True)
    page.save(destination,optimize=True)
    lines=['#pragma once','#include "../volcano_turn_tile.hpp"','namespace fire_imp_turn_atlas {',
           'using Tile=volcano_turn_data::Tile;',
           'inline constexpr int Count=32, Parts=6, Cell=160, PageCount=1;',
           'inline constexpr int authoredPart(int,int p){return p;}','inline constexpr Tile tiles[Count][Parts]={']
    lines+=[' {'+','.join('{0,'+','.join(map(str,record[d,p]))+'}' for p in range(6))+'},' for d in range(32)]
    header.write_text('\n'.join(lines+['};','}'])+'\n')
    base=Image.new('RGBA',(8*CELL,6*CELL));material=Image.new('RGBA',(8*128,6*128))
    for k in range(8):
        for p,im in enumerate(tiles[4*k]):
            base.paste(im,(k*CELL,p*CELL))
            _,rest=cell_paint(im,2,p,k)
            material.paste(rest,(k*128,p*128))
    base.save(body,optimize=True);material.save(relief,optimize=True)
    from fire_imp_turn_maps import register
    register(destination,header,destination.parent,landmarks)


def rig(markers,header):
    rows={k:markers[str(k)]['rig'] for k in SOURCES}
    for k,src in KEY_TO_SOURCE.items():
        old=rows[src]
        rows[k]=[[CELL-x,y] for x,y in [old[p] for p in (0,2,1,3,5,4,7,6)]]
    fmt=lambda pair:'{'+','.join(f'{v*128/CELL:.3f}f' for v in pair)+'}'
    lines=['#pragma once','namespace ember::fire_imp_rig {','inline constexpr float pivots[8][6][2]={']
    lines+=[' {'+','.join(fmt(v) for v in rows[k][:6])+'},' for k in range(8)]
    lines+=['};']
    for name,point in (('guardPalm',6),('palm',7)):
        lines.append('inline constexpr float '+name+'[8][2]={'+','.join(fmt(rows[k][point]) for k in range(8))+'};')
    lines.append('}')
    header.write_text('\n'.join(lines)+'\n')


if __name__=='__main__':
    cli=argparse.ArgumentParser(description=__doc__)
    cli.add_argument('root',type=Path)
    cli.add_argument('--source',type=Path);cli.add_argument('--marks',type=Path);cli.add_argument('--film',type=Path)
    cli.add_argument('--rig',type=Path);cli.add_argument('--cells',type=Path);cli.add_argument('--atlas',type=Path)
    cli.add_argument('--header',type=Path);cli.add_argument('--body',type=Path);cli.add_argument('--relief',type=Path)
    cli.add_argument('--no-audio',action='store_true',required=True)
    a=cli.parse_args()
    if a.source:
        keys(a.source,a.root/'keys')
        guides(a.root/'keys')
    if a.marks:
        marks=json.loads(a.marks.read_text())
        if a.rig:rig(marks,a.rig)
        if not a.cells:views(a.root,marks,a.film)
    if a.cells:pack(a.cells,a.atlas,a.header,a.body,a.relief)

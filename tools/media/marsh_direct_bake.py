"""Render complete paintings at 512 angles. No motion interpolation in the pigment."""
import json
import math
import struct
import numpy as np
import gpu


def resolve(raw,h,w,sub):
    samples=raw.reshape(h,sub,w,sub,4)
    count=np.count_nonzero(samples[...,3],axis=(1,3))
    rgb=np.rint(samples[...,:3].sum(axis=(1,3))/np.maximum(1,count)[...,None]).astype('u1')
    alpha=np.rint(count*(255/sub**2)).astype('u1')
    # Populate the immediate transparent ring for bilinear sampling of straight RGBA.
    full=count>0
    accum=np.zeros_like(rgb,dtype='f4')
    n=np.zeros_like(count)
    for axis,step in ((0,-1),(0,1),(1,-1),(1,1)):
        occupied=np.roll(full,step,axis=axis)
        accum+=np.roll(rgb,step,axis=axis)*occupied[...,None]
        n+=occupied
    rim=~full&(n>0)
    rgb[rim]=np.rint(accum[rim]/n[rim,None]).astype('u1')
    return np.dstack((rgb,alpha))


def landmarks(rig,s,pose,dy,alpha):
    w,h,ax,ay=rig.SIZE[s]
    marks={name:[round(float(x),3),round(float(y+dy),3)] for name,(x,y) in pose.land.items()}
    marks["limbs"]=[[round(float(p[0]),4),round(float(p[1]+dy),4),
                     round(float(p[2]),4),round(float(radius),4)] for p,radius in pose.feet]
    marks["feet"]=[]
    for p,radius in sorted(pose.feet,key=lambda f:f[0][1],reverse=True)[:2]:
        x=round(float(p[0])+ax)
        start=max(0,x-round(radius))
        rows,cols=np.where(alpha[:,start:min(w,x+round(radius)+1)]>120)
        if len(rows):
            end=rows.max()
            marks["feet"].append([round(float(cols[rows>=end-1].mean()+start-ax),2),
                                  round(float(end+.5-ay),2),radius])
        else:
            marks["feet"].append([float(p[0]),float(p[1]+dy),radius])
    return marks


def export(painter,rig,s,poses,brushes,export_dir,shader,save_png,bearings=None):
    w,h,ax,ay=rig.SIZE[s]
    uv,_,tris=painter.strip_mesh(rig)
    dy=painter.ground_offset(rig,s,poses,shader,brushes,tris,uv)
    sub=6 if s in (0,3) else 4
    buffer=painter.target(w*sub,h*sub)
    folder=export_dir/'frames'/str(s)
    folder.mkdir(exist_ok=True,parents=True)
    marks=[None]*128
    stack=[]
    bell=[]
    full=bearings is None
    page=np.zeros((h*8,w*8,4),'u1')
    with buffer[0].bind():
        gpu.state.viewport_set(0,0,w*sub,h*sub)
        for k in range(512) if full else bearings:
            source,t=divmod(k,4)
            pose=rig.frame(s,k/4)
            base=float((1-t/4)*dy[source]+t/4*dy[(source+1)%128])
            raw=painter.render_frame(rig,poses,s,source,{source:base},shader,brushes,tris,uv,
                                     buffer,sub,0,fractional=pose)
            frame=resolve(raw,h,w,sub)
            alpha=frame[...,3]
            if (alpha[:2].any() or alpha[-2:].any() or alpha[:,:2].any() or alpha[:,-2:].any()):
                raise ValueError(f"Clipped drawing {s}/{k}")
            save_png(folder/f"{k:03}.png",frame)
            stack.append(painter.layer_stack(pose))
            if s==3:
                head=pose.land["head"]
                bell.append([round(head[0]+ax,4),round(head[1]+ay+base,4)])
            if t==0:
                marks[source]=landmarks(rig,s,pose,base,frame[...,3])
                v,u=divmod(source%64,8)
                page[v*h:(v+1)*h,u*w:(u+1)*w]=frame
                if full and source%64==63:save_png(export_dir/f"neutral-{s}-{source//64}.png",page)
    if s==3 and full:
        (export_dir/'bell-centers.json').write_text(json.dumps(
            {'space':'full native cell pixels, top left origin, x right, y down','centers':bell}),encoding='utf8')
    return marks,dy,stack


WALK_FRAMES=12
WALK_MARGIN=12


def export_walk(painter,rig,s,neutral,ground,brushes,output,shader,save_png):
    w,h,ax,ay=rig.SIZE[s]
    uv,_,tris=painter.strip_mesh(rig)
    margin=WALK_MARGIN
    w+=2*margin;h+=2*margin;ax+=margin;ay+=margin
    # These are small game cels; four samples per axis preserve contact edges.
    sub=4
    buffer=painter.target(w*sub,h*sub)
    folder=output/'walk'/str(s)
    folder.mkdir(exist_ok=True,parents=True)
    marks=[]
    with buffer[0].bind():
        gpu.state.viewport_set(0,0,w*sub,h*sub)
        for bearing in range(128):
            for step in range(WALK_FRAMES):
                pose=rig.frame(s,bearing,step/WALK_FRAMES)
                raw=painter.render_frame(rig,neutral,s,bearing,ground,shader,brushes,tris,uv,
                                         buffer,sub,margin,fractional=pose)
                frame=resolve(raw,h,w,sub)
                a=frame[...,3]
                if a[:2].any() or a[-2:].any() or a[:,:2].any() or a[:,-2:].any():
                    raise ValueError(f"Clipped walk cel {s}/{bearing}/{step}")
                save_png(folder/f'{bearing*WALK_FRAMES+step:04}.png',frame)
                # Sample actual paint where the two supporting limbs meet the bog.
                grounded=sorted(pose.feet,key=lambda foot:(
                    round(pose.iso*foot[0][2]-foot[0][1]/pose.sy,2),
                    -foot[0][1]))[:2]
                contacts=[]
                for (x,y,depth),radius in grounded:
                    gy=pose.iso*depth*pose.sy+ground[bearing]+ay
                    cx=x+ax
                    l=max(0,round(cx-radius));r=min(w,round(cx+radius)+1)
                    top=max(0,math.floor(gy-7));end=min(h,math.ceil(gy+8))
                    py,px=np.where(a[top:end,l:r]>120)
                    if len(py):
                        low=py.max()
                        contacts.extend([px[py>=low-1].mean()+l+.5-ax,top+low+.5-ay,radius])
                    else:
                        contacts.extend([x,y+ground[bearing],0.])
                if contacts[0]>contacts[3]:
                    contacts=contacts[3:]+contacts[:3]
                light=pose.land['core']
                marks.append([*contacts,light[0],light[1]+ground[bearing]])
    data=np.rint(np.asarray(marks)*16).astype('<i2')
    (output/f'walk-support-{s}.bin').write_bytes(
        struct.pack('<4sHH',b'MWS1',s,len(marks))+data.tobytes())
    # The editable shot shows a full turn while continuously cycling the gait.
    poses=[rig.frame(s,i*2,(i%WALK_FRAMES)/WALK_FRAMES) for i in range(64)]
    stacks=[painter.layer_stack(pose) for pose in poses]
    scene=painter.planar_master(rig,poses,s,brushes,output,ground[::2],stacks)
    scene.name=scene.name.replace('Obrót 64','Chód ze skrętem')
    scene.render.fps=24
    return scene

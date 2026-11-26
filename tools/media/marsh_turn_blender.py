"""Painted swamp actors, five planar views with editable action timelines.

Inside Blender call job = start(repo, repo / "art/creatures/marsh-turn", output),
bake_views(job, range(5)), then finish(job). A view index is N, NW, W, SW or S.
"""
import importlib.util
import json
import math
import time
from pathlib import Path

import bpy
import numpy as np


VIEWS = ("N", "NW", "W", "SW", "S")
CARDS = ("back", "backquarter", "side", None, "front")
HEIGHT = (77, 66, 99, 146)
GX, GY = 27, 31


def direction_pose(s, view, clip, t, u, v, width, height):
    """Projected, species-specific muscle motion in a painted XY plane."""
    base = np.column_stack(((u-.5)*width, (v-1)*height))
    x, y = base[:, 0].copy(), base[:, 1].copy()
    angle = view*math.pi/4
    side, depth = math.sin(angle), -math.cos(angle)
    frontness = abs(depth)
    head_u = .5-.28*side
    head_v = ((.27, .65, .31, .24)[s] + (.12, .17, .035, .035)[s]*depth)

    def patch(cx, cy, wx, wy):
        return np.exp(-((u-cx)/wx)**2-((v-cy)/wy)**2)

    def ease(x):
        x = np.clip(x, 0, 1)
        return x*x*(3-2*x)

    def pulse(a, peak, end):
        return ease((t-a)/(peak-a))*ease((end-t)/(end-peak))

    head = patch(head_u, head_v, .28, .30)
    chest = patch(.48, .43, .42, .39)
    limb = ease((v-.52)/.41)
    leg_a = limb*np.exp(-((u-(.30+.09*side))/.18)**2)
    leg_b = limb*np.exp(-((u-(.77+.06*side))/.17)**2)
    forearm = patch(.18, .65, .18, .32)
    fararm = patch(.82, .63, .17, .30)
    # Distance along the spine, including the axial (front/back) projections.
    spine = np.clip(.5+(u-.5)*side-(v-.5)*depth, 0, 1)
    wave = np.sin(math.tau*t-4*spine)-np.sin(-4*spine)
    swing = math.sin(math.tau*t)
    weight = (1-math.cos(math.tau*t))*.5
    unit = height/100
    dx, dy = np.zeros_like(u), np.zeros_like(u)

    if clip == 0:
        if s in (0, 3):
            step = (11 if s==0 else 10)*swing*(leg_a-leg_b)
            dx += side*step+2.2*frontness*swing*chest
            dy += -depth*.56*step-(7.5 if s==0 else 6)*(max(0,swing)*leg_a+max(0,-swing)*leg_b)
            dy += weight*(4 if s==0 else 2.3)*chest
            dx += (4.5 if s==3 else 2)*swing*(fararm-forearm)
            dy += swing*(2.5*head+2*(fararm-forearm))
        elif s==1:
            dx += side*7*wave+frontness*(4*wave*(.3+spine))
            dy += -depth*4.7*wave + 4.2*wave*np.sin(math.pi*spine)
            dx += (u-.5)*4*wave
        else:
            sweep=np.sin(math.tau*t-3.5*v)-np.sin(-3.5*v)
            dx += 2.2*swing*(1-v)+(4.7+1.9*frontness)*limb*sweep
            dy += 2.6*swing*(1-v)+2.4*limb*swing*np.sin(6*u)
    elif clip == 1:
        prep, snap, follow = pulse(0,.25,.48),pulse(.24,.50,.85),pulse(.58,.80,1)
        movement = -7*prep+15*snap-3*follow
        if s==0:
            reach=movement*(.70*chest+.95*head+.23*forearm)
            dx -= side*reach
            dy += depth*.56*reach+(8*prep-4*snap)*chest
        elif s==1:
            nose=(1-spine)**1.5
            reach=movement*1.3*nose
            dx += -side*reach+(u-.5)*(9*snap-3*prep)*nose
            dy += .7*depth*reach+(-8*prep+5*snap)*np.sin(math.pi*spine)
        elif s==2:
            reach=movement*(1-v)**1.1
            dx += -side*reach+(u-.5)*12*snap*chest
            dy += depth*.6*reach+(3*prep-5*snap)*head
            dx += 4.5*snap*limb*np.sin(6*v)
        else:
            bell=patch(.31+.17*frontness,.28,.27,.25)
            reach=movement*(.58*chest+1.25*forearm+.5*bell)
            dx += -side*reach+prep*4*fararm
            dy += .52*depth*reach+(-17*prep+11*snap)*forearm+(6*snap-3*prep)*bell
    elif clip == 2:
        rebound,settle=pulse(0,.23,.69),pulse(.43,.74,1)
        retreat=12*rebound-3.8*settle
        upper=(chest+.48*head if s in (0,3) else (1-spine)**1.4 if s==1 else 1-v)
        dx += (side+.36*frontness)*retreat*upper
        dy += (-.44*depth*retreat+3*rebound)*upper
        dx -= (u-.5)*3.6*rebound*chest
        dy += 2*settle*(forearm-fararm)
    else:
        knee,roll,limp=ease(t/.43),ease((t-.15)/.65),ease((t-.36)/.59)
        bounce=pulse(.64,.76,.98)
        # Relax the extremities individually. The substantial parts keep their
        # thickness as the creature rolls onto its shoulder or swollen flank.
        if s==0:
            dx += -7*knee*head+(14*leg_b-8*leg_a+4*fararm)*limp
            dy += 5*knee*chest+11*roll*head-11*limp*leg_b-2*bounce*head
            turn,projection=-.62,.94
        elif s==1:
            tail=ease((spine-.50)/.46)
            dx += limp*(4*(1-spine)**2-(18*side+9*frontness)*tail)
            dy += limp*(6*np.sin(math.pi*spine)-(14+7*side)*tail)
            dx += roll*(u-.5)*7*np.exp(-((spine-.48)/.27)**2)
            turn,projection=.24,.99
        elif s==2:
            dx += -12*knee*(1-v)+10*limp*limb*np.sin(4*v+u)
            dy += 8*knee*chest-15*limp*limb
            dx += (u-.5)*9*roll*chest
            turn,projection=-.97,.89
        else:
            dx += -6*knee*chest+limp*(10*forearm-7*fararm+7*leg_b)
            dy += 12*knee*chest+6*knee*head-7*bounce*head-10*limp*fararm
            turn,projection=-1.02,.88
        x += dx*unit; y += dy*unit
        center=-height*.50
        radians=turn*(1-.12*depth)*roll
        ca,sa=math.cos(radians),math.sin(radians)
        # Only slight foreshortening at rest. The trunk retains its rounded
        # cross section, and folded roots and legs still lift it off the mud.
        x,y=x*ca-(y-center)*sa,(x*sa+(y-center)*ca)*(1-(1-projection)*roll)+center
        return np.column_stack((x,-y,np.zeros_like(u)))
    return np.column_stack((x+dx*unit,-y-dy*unit,np.zeros_like(u)))


def start(root, paintings, output):
    root,paintings,out=map(Path,(root,paintings,output))
    out.mkdir(parents=True,exist_ok=True)
    spec=importlib.util.spec_from_file_location("emberfall_marsh_base",root/"tools/media/marsh_actor_blender.py")
    old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)
    old.HEIGHT=HEIGHT
    batch=bpy.data.scenes.new("Mokradła | arkusze kierunków")
    old.setup(batch,8*old.SW,8*old.SH)
    batch.cycles.samples=8
    batch.camera.location=(4*old.SW,-4*old.SH,20)
    return dict(root=root,paintings=paintings,out=out,base=old,batch=batch,scenes=[],timings=[])


def painting(job,s,view):
    old=job["base"]
    suffix=CARDS[view]
    full=job["paintings"]/f"{s}-{suffix}.png" if suffix else None
    if suffix:
        if not full.is_file():
            raise FileNotFoundError(full)
        image=bpy.data.images.load(str(full),check_existing=True)
        w,h=image.size
        pixels=np.empty(w*h*4,dtype=np.float32)
        image.pixels.foreach_get(pixels)
        ys,xs=np.nonzero(pixels.reshape(h,w,4)[::-1,:,3]>.08)
        box=(xs.min(),ys.min(),xs.max()+1,ys.max()+1)
    else:
        image=bpy.data.images.load(str(job["root"]/"assets/media/mokradla/atlas.png"),check_existing=True)
        x,y,r,b=old.BOX[s]
        box=(448*s+x,448+y,448*s+r,448+b)
    if not image.packed_file:image.pack()
    return image,box


def build_view(job,s,view):
    old=job["base"]
    image,(x0,y0,x1,y1)=painting(job,s,view)
    mat=old.material(image)
    title=old.NAMES[s]+" | "+VIEWS[view]
    scene=bpy.data.scenes.new(title)
    old.setup(scene,old.SW,old.SH)
    h=HEIGHT[s]*(1.15 if s==1 and view==0 else 1)
    # The same world scale is preserved across the painted turntable.
    min_ratio=.72 if s==1 and view==0 else (.72,.42,.37,.58)[s]
    ratio=np.clip((x1-x0)/(y1-y0),min_ratio,(1.8,2.05,1.02,1.37)[s])
    w=h*ratio
    u,v=(values.ravel() for values in np.meshgrid(np.linspace(0,1,GX+1),np.linspace(0,1,GY+1)))
    faces=[]
    for row in range(GY):
        for col in range(GX):
            a=row*(GX+1)+col
            faces.append((a,a+GX+1,a+GX+2,a+1))
    def pose(clip,phase):
        return direction_pose(s,view,clip,phase,u,v,w,h)
    mesh=bpy.data.meshes.new(title)
    mesh.from_pydata(pose(0,0).tolist(),[],faces)
    uv=mesh.uv_layers.new(name="Malowidło kierunkowe")
    refs=np.array([loop.vertex_index for loop in mesh.loops])
    uv.uv.foreach_set("vector",np.column_stack(((x0+u*(x1-x0))/image.size[0],1-(y0+v*(y1-y0))/image.size[1]))[refs].ravel())
    mesh.materials.append(mat)
    animal=bpy.data.objects.new(title,mesh);scene.collection.objects.link(animal)
    animal.shape_key_add(name="Spoczynek")
    scene.camera.location.y=h*.45
    poses=[]
    for clip,label in enumerate(old.CLIPS):
        start_frame=1+clip*70
        scene.timeline_markers.new(label,frame=start_frame)
        for frame in range(16):
            vertices=pose(clip,frame/(16 if clip==0 else 15)).astype(np.float32)
            poses.append(vertices)
            key=animal.shape_key_add(name=f"{label} {frame+1:02}")
            key.data.foreach_set("co",vertices.ravel())
            peak=start_frame+frame*3
            for tick,value in ((peak-3,0),(peak,1),(peak+3,0)):
                key.value=value;key.keyframe_insert(data_path="value",frame=tick)
    scene.frame_end=256
    scene.frame_set(1)
    tick=time.perf_counter()
    subdir=job["out"]/VIEWS[view]
    subdir.mkdir(exist_ok=True)
    old.bake(job["batch"],animal,poses,s,subdir)
    job["timings"].append(dict(species=s,view=view,seconds=round(time.perf_counter()-tick,3)))
    job["scenes"].append(scene)
    (job["out"]/"progress.json").write_text(json.dumps(job["timings"],indent=2))


def bake_views(job,indices):
    for view in indices:
        for species in range(4):
            build_view(job,species,view)


def finish(job):
    assert len(job["scenes"])==20
    assert {(entry["species"],entry["view"]) for entry in job["timings"]}=={(species,view) for species in range(4) for view in range(5)}
    path=job["out"]/"Mokradla-kierunki.blend"
    bpy.data.libraries.write(str(path),set(job["scenes"]),path_remap="RELATIVE_ALL",fake_user=True,compress=True)
    data={"version":bpy.app.version_string,"views":VIEWS,"frames":1280,"sheets":job["timings"],
          "render_seconds":round(sum(entry["seconds"] for entry in job["timings"]),3),"master":str(path)}
    (job["out"]/"render.json").write_text(json.dumps(data,indent=2,ensure_ascii=False),encoding="utf-8")
    return str(path)

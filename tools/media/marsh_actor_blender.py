"""Author flat painted swamp creatures and render their four actions in Blender.
Run inside Blender with REPO and OUTPUT set to absolute paths. The source PNG
and this generator reproduce the packed projects. Use marsh_actor_pack.py on
the rendered sheets to compute contact points and build the runtime atlases.
"""
import json
import math
import time
from pathlib import Path

import bpy
import numpy as np

NAMES = ("Ropucha torfowa", "Pijawka denna", "Błędnik trzcinowy", "Dzwonnik Topieli")
CLIPS = ("Chód", "Atak", "Trafienie", "Upadek")
HEIGHT = (99, 89, 130, 143)
BOX = ((25,137,420,436), (18,172,431,436), (88,23,360,436), (21,19,427,435))
GRID_X, GRID_Y = 28, 32
SW, SH = 240, 236
SAMPLES = 16


def ease(v):
    v = np.clip(v, 0, 1)
    return v*v*(3-2*v)


def bell(t, a, peak, last):
    return ease((t-a)/(peak-a)) * ease((last-t)/(last-peak))


def shape(s, clip, t, u, v, width, height):
    """u,v are normalised painted coordinates; v increases towards the mud."""
    x = (u-.5)*width
    y = (v-1)*height
    thorax = np.exp(-((u-.48)/.45)**2-((v-.38)/.46)**2)
    front = np.exp(-((u-.20)/.26)**2-((v-.31)/.30)**2)
    forefoot = np.exp(-((u-.40)/.20)**2)*ease((v-.53)/.34)
    rearfoot = np.exp(-((u-.89)/.18)**2)*ease((v-.54)/.35)
    farfoot = np.exp(-((u-.11)/.14)**2)*ease((v-.46)/.34)
    ends = ease((v-.39)/.61)
    head = np.exp(-((u-.20)/.22)**2-((v-.24)/.25)**2)
    claw = np.exp(-((u-.11)/.18)**2-((v-.68)/.32)**2)
    neararm = np.exp(-((u-.85)/.23)**2-((v-.60)/.28)**2)
    foot_l = np.exp(-((u-.35)/.16)**2)*ease((v-.62)/.29)
    foot_r = np.exp(-((u-.83)/.18)**2)*ease((v-.63)/.27)
    phase = math.tau*t
    swing, high = math.sin(phase), math.sin(phase/2)**2
    sweep = np.sin(phase-u*4.6)-np.sin(-u*4.6)
    if clip == 0:
        if s == 0:  # Alternating forefeet, crouching haunch and counter-moving head.
            x += swing*(11*forefoot-8*farfoot-10*rearfoot)+3*high*(thorax-1.3*front)
            y += -max(0,swing)*7*forefoot-max(0,-swing)*(6*rearfoot+5*farfoot)
            y += 3.5*high*thorax+1.8*swing*front
        elif s == 1:  # Peristaltic contraction travelling from sucker to tail.
            x += 6*sweep*(.40+.60*v)+5*high*(u-.5)
            y += 5*sweep*np.sin(np.pi*u)*(.8-.35*v)+3*high*(1-v)
        elif s == 2:  # Lantern stalk bows, trailing reed tips drag on the peat.
            ripple = np.sin(phase-3.4*v)-np.sin(-3.4*v)
            x += (3.4*(1-v)*swing+7*ends*ripple+2.8*ends*np.sin(phase)*np.sin(7*u))
            y += .035*y*swing+2.7*ends*swing*np.sin(5*u)
        else:  # Weighted heel-to-toe stride and a delayed iron bell.
            x += swing*(12*foot_l-12*foot_r-7*claw+6*neararm)+3.0*high*head
            y += -max(0,swing)*9*foot_l-max(0,-swing)*9*foot_r
            y += 2.8*high*thorax+3.4*swing*head+2.3*swing*(claw-neararm)
    elif clip == 1:
        prep, strike, settle = bell(t,0,.26,.48),bell(t,.24,.50,.84),bell(t,.55,.78,1)
        if s == 0:  # Haunch loads first, throat and mouth punch forward.
            x += (5*prep-15*strike+3*settle)*thorax+(3*prep-10*strike)*front+5*prep*rearfoot
            y += (10*prep-5.6*strike+2*settle)*thorax+(-4*prep+3*strike)*front
            x -= 7*strike*(forefoot+farfoot)
        elif s == 1:  # Tail anchored while the ringed mouth telescopes.
            nose = (1-u)**1.5
            x += nose*(12*prep-21*strike+3*settle)
            y += nose*(-12*prep+7*strike-2*settle)+np.sin(np.pi*u)*7*(strike-prep)
            x += (u-.1)*np.exp(-((u-.11)/.18)**2)*16*strike
        elif s == 2:  # Reed crown leans away, lamp bends towards its magical discharge.
            x += (10*prep-19*strike+4*settle)*(1-v)**1.1+4*strike*ends*np.sin(5*v)
            y += np.exp(-((v-.32)/.27)**2)*(4*prep-5*strike)
            x += (u-.45)*20*strike*np.exp(-((v-.41)/.38)**2)
        else:  # Counterweighted backswing, striking hand and hanging bell follow at different speeds.
            lift = bell(t,.02,.29,.49)
            x += (5*prep-9*strike)*thorax+(11*lift-20*strike+3*settle)*claw
            y += (-20*lift+12*strike-3*settle)*claw+(3*prep+7*strike)*head
            x += (-5*prep-10*strike+3*settle)*head+7*prep*neararm
            y += -6*prep*neararm+3.5*strike*thorax
    elif clip == 2:
        recoil,r = bell(t,0,.24,.70),bell(t,.46,.73,1)
        if s == 0:
            x += (11*recoil-3*r)*(thorax+.42*front)
            y += (-7*recoil+2*r)*front+5*recoil*thorax
        elif s == 1:
            x += (17*recoil-3*r)*(1-u)**1.6
            y += 9*recoil*np.sin(np.pi*u)-5*recoil*(1-u)
        elif s == 2:
            x += (15*recoil-5*r)*(1-v)-4*ends*recoil*np.sin(v*5)
            y += (5*recoil-2*r)*np.sin(np.pi*v)
        else:
            x += (12*recoil-3*r)*(1-v)+4*claw*r-4*neararm*recoil
            y += 6*recoil*thorax-4*recoil*claw+3*r*head
    else:
        kneel = ease(t/.48)
        fall = ease((t-.16)/(.64 if s==2 else .57))
        loose = ease((t-.34)/.63)
        bounce = bell(t,.65,.76,.96)
        if s == 0:
            # Chest rolls to one side, forefeet fold and hind toes unfurl.
            x += -9*kneel*thorax+(6*forefoot+10*rearfoot-3*farfoot)*loose
            y += 13*kneel*thorax+8*fall*front-(forefoot+rearfoot)*3*bounce
            angle, depth = -.48*fall,1-.22*fall
        elif s == 1:
            x += (1-u)**1.5*(14*kneel-6*loose)+4*np.sin(np.pi*u)*fall
            y += 13*kneel*np.sin(np.pi*u)-5*fall*(1-u)-4*bounce*np.sin(np.pi*u)
            # Draw the trailing sucker towards the belly with a late C-shaped curl.
            tail = ease((u-.62)/.36)
            x -= loose*tail*(17+7*v)
            y -= loose*tail*(21-8*v)
            y += 6*loose*np.sin(np.pi*u)**3
            angle,depth = .23*fall,1-.25*fall
        elif s == 2:
            x += -19*kneel*(1-v)+11*loose*ends*np.sin(6*v)-6*loose*ends
            y += 9*kneel*(1-v)-13*loose*ends**2
            angle,depth = -.90*fall,1-.40*fall
        else:
            x += -10*kneel*thorax+(8*claw-6*neararm+5*foot_r)*fall
            y += 22*kneel*thorax+8*kneel*head-7*bounce*head-7*loose*neararm
            angle,depth = -.98*fall,1-.41*fall
        # Roll around the trunk and foreshorten the body in the ground plane.
        cy=-height*.46
        ca,sa=math.cos(angle),math.sin(angle)
        x,y=x*ca-(y-cy)*sa,(x*sa+(y-cy)*ca)*depth+cy
    return np.column_stack((x,-y,np.zeros_like(x)))


def material(image):
    mat = bpy.data.materials.new("Malowidło | przezroczystość")
    mat.use_nodes=True
    nodes,links=mat.node_tree.nodes,mat.node_tree.links
    nodes.clear()
    tex=nodes.new("ShaderNodeTexImage");tex.image=image;tex.interpolation="Linear"
    em=nodes.new("ShaderNodeEmission");links.new(tex.outputs["Color"],em.inputs[0])
    mix=nodes.new("ShaderNodeMixShader")
    links.new(tex.outputs["Alpha"],mix.inputs[0])
    links.new(nodes.new("ShaderNodeBsdfTransparent").outputs[0],mix.inputs[1])
    links.new(em.outputs[0],mix.inputs[2])
    links.new(mix.outputs[0],nodes.new("ShaderNodeOutputMaterial").inputs["Surface"])
    return mat


def setup(scene,w,h):
    scene.render.engine="CYCLES"
    scene.cycles.device="CPU";scene.cycles.samples=12;scene.cycles.use_denoising=False
    scene.cycles.max_bounces=1;scene.cycles.transparent_max_bounces=2
    scene.cycles.use_adaptive_sampling=False;scene.cycles.seed=13
    scene.render.threads_mode="FIXED";scene.render.threads=4
    scene.render.resolution_x=w;scene.render.resolution_y=h;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format="PNG";scene.render.image_settings.color_mode="RGBA"
    scene.render.film_transparent=True;scene.view_settings.view_transform="Standard";scene.view_settings.look="None"
    scene.render.fps=60
    c=bpy.data.cameras.new("Rzut płaski");c.type="ORTHO";c.ortho_scale=max(w,h);c.clip_end=80
    camera=bpy.data.objects.new("Kamera",c);scene.collection.objects.link(camera);camera.location=(0,0,20)
    scene.camera=camera


def creature(species,mat):
    scene=bpy.data.scenes.new(NAMES[species]);setup(scene,SW,SH)
    x0,y0,x1,y1=BOX[species]
    h=HEIGHT[species];w=h*(x1-x0)/(y1-y0)
    u,v=(arr.ravel() for arr in np.meshgrid(np.linspace(0,1,GRID_X+1),np.linspace(0,1,GRID_Y+1)))
    faces=[]
    for row in range(GRID_Y):
        for col in range(GRID_X):
            a=row*(GRID_X+1)+col
            faces.append((a,a+GRID_X+1,a+GRID_X+2,a+1))
    mesh=bpy.data.meshes.new("Siatka odkształceń 2D")
    rest=shape(species,0,0,u,v,w,h)
    mesh.from_pydata(rest.tolist(),[],faces)
    uv=mesh.uv_layers.new(name="Malowidło")
    refs=np.array([l.vertex_index for l in mesh.loops])
    uv.uv.foreach_set("vector",np.column_stack(((448*species+x0+u*(x1-x0))/1792,1-(448+y0+v*(y1-y0))/896))[refs].ravel())
    mesh.materials.append(mat)
    ob=bpy.data.objects.new(NAMES[species],mesh);scene.collection.objects.link(ob)
    ob.shape_key_add(name="Spoczynek")
    scene.camera.location.y=h*.45
    poses=[]
    for clip,title in enumerate(CLIPS):
        at=1+clip*70;scene.timeline_markers.new(title,frame=at)
        count=16 if clip==0 else 15
        for k in range(16):
            verts=shape(species,clip,k/count,u,v,w,h).astype(np.float32)
            poses.append(verts)
            key=ob.shape_key_add(name=f"{title} {k+1:02}")
            key.data.foreach_set("co",verts.ravel())
            frame=at+k*3
            for tick,value in ((frame-3,0),(frame,1),(frame+3,0)):
                key.value=value;key.keyframe_insert(data_path="value",frame=tick)
    scene.frame_end=1+3*70+45
    scene.frame_set(1)
    return scene,ob,poses


def bake(scene,base,poses,species,out):
    mesh=base.data
    count=len(mesh.vertices)
    faces=[tuple(f.vertices) for f in mesh.polygons]
    uv=np.zeros(len(mesh.loops)*2,dtype=np.float32);mesh.uv_layers.active.uv.foreach_get("vector",uv)
    vertices=[];parts=[]
    for k,pose in enumerate(poses):
        offset=np.array([SW*(k%8+.5),-SH*(k//8+.5)-HEIGHT[species]*.43,0])
        vertices.extend((pose+offset).tolist())
        parts.extend(tuple(i+k*count for i in face) for face in faces)
    sheet=bpy.data.meshes.new("Klatki atlasu")
    sheet.from_pydata(vertices,[],parts)
    sheet.uv_layers.new(name="Malowidło").uv.foreach_set("vector",np.tile(uv,64))
    sheet.materials.append(mesh.materials[0])
    ob=bpy.data.objects.new("Klatki",sheet);scene.collection.objects.link(ob)
    path=out/f"render-{species}.png";scene.render.filepath=str(path)
    bpy.ops.render.render(scene=scene.name,write_still=True)
    bpy.data.objects.remove(ob,do_unlink=True);bpy.data.meshes.remove(sheet)


def run(root,output):
    root,out=Path(root),Path(output);out.mkdir(parents=True,exist_ok=True)
    image=bpy.data.images.load(str(root/"assets/media/mokradla/atlas.png"),check_existing=False);image.pack()
    mat=material(image)
    batch=bpy.data.scenes.new("Wypalanie");setup(batch,8*SW,8*SH)
    batch.camera.location=(4*SW,-4*SH,20)
    masters=[];stats=[]
    for s in range(4):
        sc,ob,poses=creature(s,mat);masters.append(sc)
        tick=time.perf_counter();bake(batch,ob,poses,s,out)
        stats.append(round(time.perf_counter()-tick,3))
        (out/"progress.json").write_text(json.dumps(stats))
    path=out/"Mokradla-bestie.blend"
    bpy.data.libraries.write(str(path),set(masters),path_remap="RELATIVE_ALL",fake_user=True,compress=True)
    (out/"render.json").write_text(json.dumps({"software":bpy.app.version_string,"seconds":stats,"total_render_seconds":sum(stats),"species":NAMES,"clips":CLIPS},ensure_ascii=False,indent=2),encoding="utf-8")
    return str(path)


if __name__=="__main__":
    print(run(REPO,OUTPUT))

"""Build editable gait/strike/recoil/death actions for the 2D swamp turntable.

In Blender execute save_actions(runtime_dir, destination). The resulting native
project contains packed directional paintings and an example from every quadrant
of each species. Select a scene and scrub its four labelled clips. The per-yaw
vertex tracks for the other bearings use the same keyed lattice.
"""
import hashlib
import json
import math
from pathlib import Path
import time
import bpy
import numpy as np

CELLS=((140,100,70,92),(164,96,82,89),(120,120,60,112),(204,164,102,159))
NAMES=('Ropucha torfowa','Pijawka denna','Błędnik trzcinowy','Dzwonnik Topieli')
CLIPS=('Chód','Atak','Trafienie','Upadek')
SIDE=13


def artwork(path, material_name):
    image=bpy.data.images.load(str(path),check_existing=False)
    image.pack()
    mat=bpy.data.materials.new(material_name)
    mat.use_nodes=True
    tree=mat.node_tree
    tree.nodes.clear()
    out=tree.nodes.new('ShaderNodeOutputMaterial')
    emission=tree.nodes.new('ShaderNodeEmission')
    transparent=tree.nodes.new('ShaderNodeBsdfTransparent')
    mix=tree.nodes.new('ShaderNodeMixShader')
    brush=tree.nodes.new('ShaderNodeTexImage')
    brush.image=image
    tree.links.new(brush.outputs['Color'],emission.inputs['Color'])
    tree.links.new(brush.outputs['Alpha'],mix.inputs[0])
    tree.links.new(transparent.outputs[0],mix.inputs[1])
    tree.links.new(emission.outputs[0],mix.inputs[2])
    tree.links.new(mix.outputs[0],out.inputs[0])
    return mat,image


def motion_page(path, expected):
    # PNG channels are little-endian integer pairs. Mark as data before reading:
    # even a correct sRGB transform would corrupt these signed coordinates.
    image=bpy.data.images.load(str(path),check_existing=False)
    image.colorspace_settings.name='Non-Color'
    image.alpha_mode='CHANNEL_PACKED'
    width,height=image.size
    floats=np.empty(width*height*4,'f4');image.pixels.foreach_get(floats)
    rgba=np.rint(floats.reshape(height,width,4)[::-1]*255).astype('u1')
    bpy.data.images.remove(image)
    decoded=rgba.view('<i2').reshape(64,SIDE,128,SIDE,2).transpose(2,0,1,3,4)
    if hashlib.sha256(np.ascontiguousarray(decoded).tobytes()).hexdigest()!=expected:
        raise ValueError(f'Blender decoded different geometry in {path}')
    return decoded/16.


def add_scene(s,b,mat,poses):
    w,h,ax,ay=CELLS[s]
    name=f'{NAMES[s]} | {b*360/128:.2f}°'
    scene=bpy.data.scenes.new(name)
    scene.render.engine='CYCLES'
    scene.cycles.samples=2
    scene.render.film_transparent=True
    scene.render.resolution_x=w*3;scene.render.resolution_y=h*3;scene.render.resolution_percentage=100
    scene.render.fps=60
    scene.view_settings.view_transform='Standard'
    x,y=np.meshgrid(np.linspace(0,w,SIDE)-ax,np.linspace(0,h,SIDE)-ay)
    neutral=np.stack((x,-y,np.zeros_like(x)),axis=-1).astype('f4')
    quads=[(i,i+SIDE,i+SIDE+1,i+1) for r in range(SIDE-1) for c in range(SIDE-1) for i in (r*SIDE+c,)]
    mesh=bpy.data.meshes.new(name)
    mesh.from_pydata(neutral.reshape(-1,3).tolist(),[],quads)
    mesh.materials.append(mat)
    uv=mesh.uv_layers.new(name='Malarstwo kierunkowe')
    uvx,uvy=np.meshgrid((b%8+np.linspace(0,1,SIDE))/8,1-((b%64)//8+np.linspace(0,1,SIDE))/8)
    indices=np.array([l.vertex_index for l in mesh.loops])
    uv.uv.foreach_set('vector',np.stack((uvx,uvy),axis=-1).reshape(-1,2)[indices].astype('f4').ravel())
    obj=bpy.data.objects.new(name,mesh)
    scene.collection.objects.link(obj)
    obj.shape_key_add(name='Spoczynek')
    for clip,title in enumerate(CLIPS):
        start=1+clip*72
        scene.timeline_markers.new(title,frame=start)
        for k in range(16):
            key=obj.shape_key_add(name=f'{title} {k+1:02}')
            xyz=neutral.copy()
            xyz[:,:,:2]+=poses[clip*16+k]*(1,-1)
            key.data.foreach_set('co',xyz.ravel())
            key.value=0;key.keyframe_insert(data_path='value',frame=start+4*k-4)
            key.value=1;key.keyframe_insert(data_path='value',frame=start+4*k)
            if clip<3 or k<15:
                key.value=0;key.keyframe_insert(data_path='value',frame=start+4*k+4)
    action=mesh.shape_keys.animation_data.action
    action.name=name+' | cztery akcje'
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for curve in bag.fcurves:
                    for point in curve.keyframe_points:point.interpolation='LINEAR'
    camdata=bpy.data.cameras.new(name)
    camera=bpy.data.objects.new(name+' | kamera',camdata)
    scene.collection.objects.link(camera)
    camdata.type='ORTHO';camdata.ortho_scale=max(w,h)*1.18
    camera.location=(0,h*.43,250)
    scene.camera=camera
    scene.frame_start=1;scene.frame_end=277
    scene.frame_set(1)
    return scene,obj


def save_actions(runtime_dir, destination, bearings=(8,40,72,104)):
    runtime_dir=Path(runtime_dir);destination=Path(destination)
    if destination.exists(): raise FileExistsError(destination)
    destination.parent.mkdir(parents=True,exist_ok=True)
    start=time.monotonic()
    scenes=[];objects=[];materials=[];images=[];verified=[]
    receipt=json.loads((runtime_dir/'motion-receipt.json').read_text(encoding='utf-8'))
    try:
        for s in range(4):
            poses=motion_page(runtime_dir/f'motion-{s}.png',receipt['species'][s]['decoded_geometry_sha256'])
            for page in range(2):
                if not any(b//64==page for b in bearings):continue
                mat,image=artwork(runtime_dir/f'neutral-{s}-{page}.png',NAMES[s]+f' {page}')
                images.append(image);materials.append(mat)
                for b in bearings:
                    if b//64!=page:continue
                    scene,obj=add_scene(s,b,mat,poses[b]);scenes.append(scene);objects.append(obj)
                    sample=np.empty(SIDE*SIDE*3,'f4')
                    basis=np.empty_like(sample)
                    keys=obj.data.shape_keys.key_blocks
                    keys[0].data.foreach_get('co',basis)
                    for i in range(64):
                        keys[i+1].data.foreach_get('co',sample)
                        measured=(sample-basis).reshape(SIDE,SIDE,3)[:,:,:2]*(1,-1)
                        if not np.allclose(measured,poses[b,i],atol=.00003):
                            raise ValueError(f'Blender shape differs: {s}/{b}/{i}')
                    verified.append([s,b,len(keys)-1])
        bpy.data.libraries.write(str(destination),set(scenes),fake_user=True,compress=True)
    finally:
        meshes=[obj.data for obj in objects]
        cameras=[scene.camera for scene in scenes]
        actions=[mesh.shape_keys.animation_data.action for mesh in meshes]
        for scene in scenes:bpy.data.scenes.remove(scene)
        for obj in objects:bpy.data.objects.remove(obj)
        for camera in cameras:
            data=camera.data;bpy.data.objects.remove(camera);bpy.data.cameras.remove(data)
        for mesh in meshes:bpy.data.meshes.remove(mesh)
        for action in actions:bpy.data.actions.remove(action)
        for mat in materials:bpy.data.materials.remove(mat)
        for image in images:bpy.data.images.remove(image)
    with bpy.data.libraries.load(str(destination),link=False) as (src,_):
        if len(src.scenes)!=len(verified):raise ValueError('Missing saved scenes')
    return {'file':str(destination),'seconds':round(time.monotonic()-start,3),
            'scenes':len(verified),'verified_keys':sum(item[2] for item in verified),
            'sha256':hashlib.sha256(destination.read_bytes()).hexdigest()}

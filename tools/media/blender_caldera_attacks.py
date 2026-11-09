"""Author and sample Blender animation curves for layered 2D Caldera attacks."""
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy


ROLES = ('slash', 'breath', 'eruption', 'cinder')
# Frame, width, height, rotation (degrees), opacity. Angles are in screen space.
EFFECTS = (
    [(1,.60,.70,-10,.92),(7,1.02,1.14,-2,1),(17,1.10,1.04,5,.78),(34,1.19,.83,9,.30),(61,1.27,.62,12,0)],
    [(1,.68,.69,-2,.83),(8,.99,1.06,1,1),(18,.94,.86,-1,.86),(23,1.04,1.12,1,1),
     (38,.94,.89,-1,.85),(43,1.04,1.11,1,1),(54,1.02,1.01,0,.94),(61,.95,.84,-1,.45)],
    [(1,.72,.28,-3,.94),(8,1.02,.94,1,1),(16,1.06,1.11,0,.94),(31,1.11,.91,-3,.56),(49,1.17,.71,-5,.18),(61,1.20,.58,-7,0)],
    [(1,1,.92,0,1),(15,1.05,1.02,-3,.96),(31,.96,1.10,2,.91),(47,1.03,.96,3,.97),(61,1,.92,0,1)],
)
# Frame, torso advance in sprite pixels, lift, body pitch, head/weapon drive.
POSES = (
    [(1,0,0,0,0),(17,-4,1,-3,-5),(29,-6,3,-4,-8),(32,8,-2,5,17),(37,12,-3,7,11),(49,4,-1,3,2),(61,0,0,0,0)],
    [(1,0,0,0,0),(13,-5,3,-4,-7),(22,-8,7,-6,-13),(25,8,2,5,10),(30,6,2,3,6),
     (34,8,1,4,9),(41,6,2,3,5),(43,9,1,5,10),(50,3,1,2,4),(61,0,0,0,0)],
    [(1,0,0,0,0),(16,-2,2,-2,-12),(36,-3,5,-4,-24),(42,2,4,2,15),(49,4,-1,5,19),(61,0,0,0,0)],
)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def plane(scene, image):
    mesh = bpy.data.meshes.new('Flat effect card')
    mesh.from_pydata([(-1,-1,0),(1,-1,0),(1,1,0),(-1,1,0)], [], [(0,1,2,3)])
    uv = mesh.uv_layers.new(name='Artwork UV')
    for index, point in enumerate(((0,0),(1,0),(1,1),(0,1))):
        uv.data[index].uv = point
    card = bpy.data.objects.new('Painted effect', mesh)
    scene.collection.objects.link(card)
    mat = bpy.data.materials.new('ComfyUI color • Krita transparency')
    mat.use_nodes = True
    tree = mat.node_tree
    tree.nodes.clear()
    tex = tree.nodes.new('ShaderNodeTexImage')
    tex.image = image
    tex.extension = 'CLIP'
    opacity = tree.nodes.new('ShaderNodeValue')
    opacity.label = 'Animated opacity'
    mask = tree.nodes.new('ShaderNodeMath')
    mask.operation = 'MULTIPLY'
    emit = tree.nodes.new('ShaderNodeEmission')
    transparent = tree.nodes.new('ShaderNodeBsdfTransparent')
    mix = tree.nodes.new('ShaderNodeMixShader')
    out = tree.nodes.new('ShaderNodeOutputMaterial')
    for source, target in [(tex.outputs['Color'],emit.inputs[0]), (tex.outputs['Alpha'],mask.inputs[0]),
                           (opacity.outputs[0],mask.inputs[1]), (mask.outputs[0],mix.inputs[0]),
                           (transparent.outputs[0],mix.inputs[1]), (emit.outputs[0],mix.inputs[2]),
                           (mix.outputs[0],out.inputs['Surface'])]:
        tree.links.new(source,target)
    card.data.materials.append(mat)
    return card, opacity.outputs[0]


def stage(role, image):
    scene = bpy.data.scenes.new(role.title())
    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 12
    scene.render.threads_mode = 'FIXED'
    scene.render.threads = 2
    scene.render.resolution_x = scene.render.resolution_y = 256
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    scene.render.film_transparent = True
    scene.view_settings.view_transform = 'Standard'
    scene.render.fps = 60
    scene.frame_start, scene.frame_end = 1, 61
    camera = bpy.data.objects.new('Orthographic preview', bpy.data.cameras.new('2D effects camera'))
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = 2.80
    camera.location.z = 5
    scene.collection.objects.link(camera)
    scene.camera = camera
    return scene, *plane(scene, image)


def rig_pose(scene, keys):
    pose = bpy.data.objects.new('Torso • advance and pitch', None)
    drive = bpy.data.objects.new('Head or weapon • follow through', None)
    scene.collection.objects.link(pose)
    scene.collection.objects.link(drive)
    for frame, advance, lift, pitch, rotation in keys:
        pose.location = (advance, lift, 0)
        pose.rotation_euler.z = math.radians(pitch)
        drive.rotation_euler.z = math.radians(rotation)
        pose.keyframe_insert('location', frame=frame)
        pose.keyframe_insert('rotation_euler', index=2, frame=frame)
        drive.keyframe_insert('rotation_euler', index=2, frame=frame)
    return pose, drive


def header(effects, poses):
    result = ['#pragma once', '#include <algorithm>', '',
              '// Baked at 60 Hz from the editable Blender effect and pose curves.',
              'namespace caldera_attack_motion {',
              'struct Effect {float width,height,angle,opacity;};',
              'struct Pose {float advance,lift,pitch,drive;};']
    for name, cls, table in [('Effects','Effect',effects),('Poses','Pose',poses)]:
        result.append(f'inline constexpr {cls} {name}[{len(table)}][61] = {{')
        for row in table:
            result.append(' {')
            result.extend('  {' + ','.join(f'{n:.6f}f' for n in key) + '},' for key in row)
            result.append(' },')
        result.append('};')
    result.extend('''inline Effect effect(int clip,float t){
 float frame=std::clamp(t,0.f,1.f)*60;int i=std::min(int(frame),59);float u=frame-i;
 const auto& a=Effects[clip][i];const auto& b=Effects[clip][i+1];
 return {a.width+(b.width-a.width)*u,a.height+(b.height-a.height)*u,
         a.angle+(b.angle-a.angle)*u,a.opacity+(b.opacity-a.opacity)*u};
}
inline Pose pose(int clip,float t){
 float frame=std::clamp(t,0.f,1.f)*60;int i=std::min(int(frame),59);float u=frame-i;
 const auto& a=Poses[clip][i];const auto& b=Poses[clip][i+1];
 return {a.advance+(b.advance-a.advance)*u,a.lift+(b.lift-a.lift)*u,
         a.pitch+(b.pitch-a.pitch)*u,a.drive+(b.drive-a.drive)*u};
}
}'''.splitlines())
    return '\n'.join(result)+'\n'


def produce(root, refined):
    destination = root / ('blender-delivery' if refined else 'blender-v2')
    painted = root / ('painted-refined' if refined else 'painted')
    destination.mkdir(exist_ok=False)
    effects, poses, scenes = [], [], []
    for index, role in enumerate(ROLES):
        image = bpy.data.images.load(str(painted / f'{role}.png'))
        image.pack()
        scene, card, opacity = stage(role, image)
        bpy.context.window.scene = scene
        scenes.append(scene)
        for frame, sx, sy, rotation, alpha in EFFECTS[index]:
            card.scale = (sx, sy, 1)
            card.rotation_euler.z = math.radians(-rotation)
            opacity.default_value = alpha
            card.keyframe_insert('scale', frame=frame)
            card.keyframe_insert('rotation_euler', index=2, frame=frame)
            opacity.keyframe_insert('default_value', frame=frame)
        actors = rig_pose(scene, POSES[index]) if index < len(POSES) else None
        effect_keys, pose_keys = [], []
        for frame in range(1,62):
            scene.frame_set(frame)
            effect_keys.append((card.scale.x,card.scale.y,-math.degrees(card.rotation_euler.z),opacity.default_value))
            if actors:
                pose, drive = actors
                pose_keys.append((pose.location.x,pose.location.y,math.degrees(pose.rotation_euler.z),math.degrees(drive.rotation_euler.z)))
        effects.append(effect_keys)
        if actors:
            poses.append(pose_keys)
    unique = [len(set(keys)) for keys in effects+poses]
    assert min(unique)>24, unique
    for first, last in zip(effects[3][0], effects[3][-1]):
        assert abs(first-last)<1e-5, 'Cinder pulse must loop smoothly'
    motion = destination / 'caldera_attack_motion.hpp'
    motion.write_text(header(effects, poses), encoding='ascii')
    bpy.context.window.scene = scenes[0]
    master = destination / 'caldera-attack-motion.blend'
    bpy.ops.wm.save_as_mainfile(filepath=str(master))
    renders = []
    for scene, role in zip(scenes, ROLES):
        bpy.context.window.scene = scene
        for frame in (8,26,48):
            scene.frame_set(frame)
            path = destination / f'{role}-{frame:02}.png'
            scene.render.filepath = str(path)
            bpy.ops.render.render(write_still=True, scene=scene.name)
            renders.append({'role':role,'frame':frame,'path':path.name,'sha256':sha(path)})
        assert len({r['sha256'] for r in renders if r['role']==role}) == 3, role
    receipt = {'status':'PASS','program':'Blender '+bpy.app.version_string,'master':str(master),
               'master_sha256':sha(master),'motion_sha256':sha(motion),'effect_tracks':4,'actor_tracks':3,
               'sample_hz':60,'renders':renders,'runtime_2d':True,'audio':False}
    (destination / 'receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')


if __name__ == '__main__':
    args = sys.argv[sys.argv.index('--')+1:]
    assert '--no-audio' in args[1:]
    produce(Path(args[0]).resolve(), '--refined' in args[1:])

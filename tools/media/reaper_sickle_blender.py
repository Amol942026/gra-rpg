"""Author the Reaper's flat sickle layers and export Blender motion curves."""

import json
from math import degrees, radians
from pathlib import Path
from time import perf_counter

import bpy
from mathutils import Vector
from mathutils.geometry import tessellate_polygon


SIZE = 96
GRIP = (48, 32)
CHANNELS = ("shoulder", "elbow", "wrist", "roll")

# Lift the hand in front of the chest, then cut down through the same arc.
# Frame 27 is impact and keeps the original position of the blade.
ATTACK = (
    (0, 0, 0, 0, 0),
    (5, -16, -5, 4, .04),
    (11, -54, -15, 9, .11),
    (17, -101, -22, 11, .19),
    (22, -135, -19, -4, .26),
    (24, -139, -21, -8, .32),
    (25, -136, -19, -9, .28),
    (27, -113, -12, 5, .08),
    (31, -78, -22, 22, .06),
    (37, -55, -20, 19, .15),
    (46, -24, -11, 9, .12),
    (55, -6, -3, 2, .03),
    (60, 0, 0, 0, 0),
)

WALK = (
    (0, 2, 5, -2, .10),
    (8, 8, 7, -4, .16),
    (16, 11, 5, -2, .19),
    (24, 6, 1, 2, .14),
    (32, -3, -3, 5, .08),
    (40, -10, -4, 4, .03),
    (48, -8, 0, 1, .04),
    (55, -3, 3, 0, .07),
    (60, 2, 5, -2, .10),
)


def linear(byte):
    value = byte / 255
    return value / 12.92 if value <= .04045 else ((value + .055) / 1.055) ** 2.4


def face(scene, name, color, origin, *segments):
    """Fill a flat cubic path with an unlit, hand-selected material color."""
    points = [Vector(origin)]
    for first, second, end in segments:
        start = points[-1]
        first, second, end = map(Vector, (first, second, end))
        for i in range(1, 13):
            t = i / 12
            u = 1 - t
            points.append(u**3 * start + 3*u*u*t * first + 3*u*t*t * second + t**3 * end)
    polygon = [Vector((x - GRIP[0], GRIP[1] - y, 0)) for x, y in points]
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(polygon, [], tessellate_polygon([polygon]))
    item = bpy.data.objects.new(name, mesh)
    item.location.z = .01 * len(scene.objects)
    scene.collection.objects.link(item)
    material = bpy.data.materials.new(name)
    material.diffuse_color = tuple(linear(value) for value in bytes.fromhex(color)) + (1,)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    nodes.clear()
    light = nodes.new("ShaderNodeEmission")
    light.inputs[0].default_value = material.diffuse_color
    output = nodes.new("ShaderNodeOutputMaterial")
    material.node_tree.links.new(light.outputs[0], output.inputs[0])
    mesh.materials.append(material)
    return item


def paint(scene):
    face(scene, "Sylwetka ostrza", "173430", (43, 41),
         ((31, 34), (13, 44), (9, 60)), ((4, 77), (18, 91), (35, 91)),
         ((40, 91), (45, 89), (49, 85)), ((36, 88), (24, 81), (23, 71)),
         ((22, 60), (31, 55), (43, 52)), ((47, 50), (47, 44), (43, 41)))
    face(scene, "Szkło – korpus", "416e64", (42, 43),
         ((30, 38), (15, 47), (12, 61)), ((8, 75), (19, 87), (33, 89)),
         ((38, 89), (43, 88), (47, 86)), ((33, 87), (23, 80), (21, 70)),
         ((20, 60), (31, 51), (43, 48)), ((45, 47), (43, 45), (42, 43)))
    face(scene, "Matowy grzbiet", "294c47", (41, 44),
         ((29, 41), (17, 50), (15, 63)), ((13, 74), (20, 84), (28, 87)),
         ((18, 78), (17, 65), (25, 55)), ((29, 50), (36, 48), (43, 47)))
    face(scene, "Szlif krawędzi", "9ac3aa", (42, 50),
         ((31, 53), (22, 60), (22, 68)), ((21, 80), (33, 88), (48, 85.6)),
         ((36, 90), (20, 83), (19, 69)), ((18, 61), (27, 51), (38, 48)))
    face(scene, "Odbicie na szlifie", "d3e0bf", (24, 59),
         ((20, 65), (20, 72), (24, 78)), ((21, 76), (19, 73), (19, 69)),
         ((19, 65), (21, 61), (24, 59)))
    face(scene, "Refleks spękanego szkła", "759c88", (15, 62),
         ((17, 52), (25, 48), (32, 45)), ((24, 49), (21, 53), (19, 58)),
         ((17, 58), (16, 61), (15, 62)))
    face(scene, "Rękojeść i trzpień", "33291d", (32, 50),
         ((30, 47), (32, 43), (35, 40)), ((45, 32), (54, 22), (63, 17)),
         ((67, 16), (70, 20), (68, 23)), ((62, 29), (51, 37), (42, 48)),
         ((39, 51), (36, 53), (32, 50)))
    face(scene, "Skórzane owinięcie", "79583c", (37, 44),
         ((45, 35), (53, 28), (62, 21)), ((64, 20), (66, 20), (66, 22)),
         ((59, 30), (50, 35), (42, 45)), ((40, 47), (38, 48), (37, 44)))
    for index, (x, y) in enumerate(((44, 36), (48, 32), (52, 28), (57, 24))):
        face(scene, f"Szew skóry {index + 1}", "493724", (x-2, y-1),
             ((x, y), (x+1, y+2), (x+2, y+3)),
             ((x+1, y+4), (x, y+4), (x-1, y+3)),
             ((x-2, y+2), (x-3, y), (x-2, y-1)))
    face(scene, "Brązowa tuleja", "a07c4c", (31, 46),
         ((33, 41), (36, 39), (39, 37)), ((43, 38), (45, 42), (45, 45)),
         ((42, 48), (37, 51), (34, 51)), ((32, 50), (31, 48), (31, 46)))
    face(scene, "Krawędź tulei", "d4b47a", (32, 45),
         ((33, 42), (36, 40), (39, 38)), ((40, 38), (41, 39), (41, 40)),
         ((37, 42), (36, 43), (34, 47)))
    face(scene, "Płaska głowica", "b99157", (60, 21),
         ((61, 19), (63, 17), (66, 17)), ((68, 17), (69, 19), (69, 21)),
         ((68, 23), (66, 24), (65, 25)), ((64, 23), (62, 22), (60, 21)))


def animate(scene, name, keys, blade, cyclic):
    shoulder = bpy.data.objects.new(name + " / bark", None)
    elbow = bpy.data.objects.new(name + " / łokieć", None)
    wrist = bpy.data.objects.new(name + " / nadgarstek", None)
    controls = (shoulder, elbow, wrist)
    for control in controls:
        scene.collection.objects.link(control)
    elbow.parent = shoulder
    elbow.location.y = -14
    wrist.parent = elbow
    wrist.location.y = -13
    wrist["roll"] = 0.
    wrist["clip"] = name
    for at, s, e, w, roll in keys:
        for control, angle in zip(controls, (s, e, w)):
            control.rotation_euler.z = -radians(angle)
            control.keyframe_insert(data_path="rotation_euler", index=2, frame=at+1, group=name)
        wrist["roll"] = float(roll)
        wrist.keyframe_insert(data_path='["roll"]', frame=at+1, group=name)
    curves = []
    for control in controls:
        animation = control.animation_data
        fcurves = animation.action.layers[0].strips[0].channelbag(animation.action_slot).fcurves
        curves.append(fcurves.find("rotation_euler", index=2))
        for fcurve in fcurves:
            for point in fcurve.keyframe_points:
                point.handle_left_type = point.handle_right_type = "AUTO_CLAMPED"
            if cyclic:
                fcurve.modifiers.new("CYCLES")
    roll = bpy.data.objects.new(name + " / obrót ostrza", None)
    scene.collection.objects.link(roll)
    roll.parent = wrist
    driver = roll.driver_add("scale", 0).driver
    variable = driver.variables.new()
    variable.name = "roll"
    variable.targets[0].id = wrist
    variable.targets[0].data_path = '["roll"]'
    driver.expression = "1 - .37 * roll"
    for source in blade:
        copy = source.copy()
        scene.collection.objects.link(copy)
        copy.parent = roll
    roll_curve = fcurves.find('["roll"]')
    # Evaluate the F-curves directly. This scene need not be active in Blender's window.
    samples = [[*(-degrees(curve.evaluate(frame)) for curve in curves), roll_curve.evaluate(frame)]
               for frame in range(1, 62)]
    if any(max(values) - min(values) < .06 for values in zip(*samples)):
        raise RuntimeError(f"Static motion export: {name}")
    if any(abs(a[3] - b[3]) > .35 for a, b in zip(samples, samples[1:])):
        raise RuntimeError(f"Discontinuous blade roll: {name}")
    scene.frame_set(1)
    return samples


def make_camera(scene):
    camera = bpy.data.cameras.new("Rzut ortograficzny")
    camera.type = "ORTHO"
    camera.ortho_scale = SIZE
    scene.camera = bpy.data.objects.new(camera.name, camera)
    scene.collection.objects.link(scene.camera)
    scene.camera.location = (SIZE/2 - GRIP[0], GRIP[1] - SIZE/2, 8)


def write_curves(output, motions):
    lines = ["#pragma once", "",
             "// Samples exported from the editable Blender sickle rig.",
             "namespace ember::reaper_sickle_motion {", ""]
    for clip, samples in motions.items():
        for channel, values in zip(CHANNELS, zip(*samples)):
            lines.append(f"inline constexpr float {clip}_{channel}[61]={{")
            for first in range(0, len(values), 8):
                row = ",".join(f"{value:.5f}f" for value in values[first:first+8])
                lines.append(" " + row + ",")
            lines.append("};")
        lines.append("")
    lines.extend(("}", ""))
    (output / "reaper_sickle_motion.hpp").write_text("\n".join(lines), encoding="utf-8")


def bake(output, samples_path):
    started = perf_counter()
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    art = bpy.data.scenes.new("Sierp / ilustracja 2D")
    art.view_settings.view_transform = "Standard"
    art.render.film_transparent = True
    art.render.image_settings.color_mode = "RGBA"
    art.render.resolution_x = art.render.resolution_y = SIZE * 3
    art.render.resolution_percentage = 100
    art.render.threads_mode = "FIXED"
    art.render.threads = 2
    paint(art)
    blade = list(art.objects)
    make_camera(art)
    motions = {}
    scenes = [art]
    for name, keys, cyclic in (("Chód", WALK, True), ("Cięcie", ATTACK, False)):
        scene = bpy.data.scenes.new("Sierp / " + name)
        scene.frame_start, scene.frame_end, scene.render.fps = 1, 61, 60
        motions["walk" if cyclic else "attack"] = animate(scene, name, keys, blade, cyclic)
        make_camera(scene)
        scene.camera.data.ortho_scale = 156
        scene.camera.location.y = -32
        scenes.append(scene)
    for scene in scenes:
        scene["kotwica_px"] = list(GRIP)
    master = output / "Sierp-Szklanego-Zniwiarza.blend"
    bpy.data.libraries.write(str(master), set(scenes), fake_user=True, compress=True)
    art.render.filepath = str(output / "sierp-rgba.png")
    bpy.ops.render.render(scene=art.name, write_still=True)
    write_curves(output, motions)
    Path(samples_path).write_text(json.dumps({
        "grip": GRIP, "channels": CHANNELS, "samples": motions,
        "master": str(master), "render_seconds": perf_counter()-started,
    }, indent=2), encoding="utf-8")

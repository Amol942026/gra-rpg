"""Animate the Glass Reaper's painted cards and sample their Blender F-curves."""

import json
from math import atan2, cos, pi, radians, remainder, sin
from pathlib import Path
from time import perf_counter

import bpy
from mathutils import Vector


SICKLE_RELEASE_FRAME = 23
LEG_SETTLE_FRAMES = 10
LEG_SETTLE_SECONDS = LEG_SETTLE_FRAMES * .9 / 104

# Frames 0 and 60 are the same point in the gait. In the attack, frame 27 is impact.
TRACKS = {
    "walk": (("drop", "shift", "pitch", "head", "free_arm", "free_elbow"), (
        (0, 1.3, 0, .8, .4, -7, -8),
        (8, 2.9, .54, 1.9, -.9, -17, -18),
        (16, 1.4, .34, 1.3, -1.3, -7, -11),
        (23, .62, -.16, .4, -.3, 6, -5),
        (30, 1.3, 0, .8, .4, 13, -7),
        (38, 2.9, -.54, 1.9, -.9, 18, -14),
        (46, 1.4, -.34, 1.3, -1.3, 8, -16),
        (53, .62, .16, .4, -.3, 1, -11),
        (60, 1.3, 0, .8, .4, -7, -8),
    )),
    "step": (("reach", "lift", "knee", "toe"), (
        (0, .30, 0, 5, -7), (6, .20, 0, 13, 0), (18, 0, 0, 7, 0),
        (30, -.20, 0, 10, 0), (36, -.30, 0, 24, 12), (40, -.29, .22, 37, 11),
        (45, -.12, .58, 46, 4), (50, .12, .53, 37, 0),
        (56, .31, .15, 15, -5), (60, .30, 0, 5, -7),
    )),
    "attack": (("drop", "shift", "pitch", "head", "free_arm", "free_elbow",
                "lead_reach", "lead_lift", "lead_toe", "rear_toe"), (
        (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
        (6, 1.2, -1.3, -2.5, 1, 4, -7, 0, 0, 0, 0),
        (11, 1.4, -2.8, -5, 2.5, 10, -11, 1.7, 1.9, 5, 0),
        (16, 1.1, -3.5, -8.5, 4.4, 17, -14, 5, 4.5, 1, 0),
        (20, 2.4, -1.55, -10.5, 5.8, 22, -16, 8.1, 2.4, -7, 0),
        (23, 3.4, .6, -9, 6.4, 22, -18, 9.3, .4, -3, 3),
        (25, 4.2, 3.9, 2, 2, 9, -21, 9.3, 0, 0, 8),
        (27, 5.5, 8.2, 16, -4, -21, -27, 9.3, 0, 0, 12),
        (32, 6.2, 8.6, 17, -3.5, -31, -20, 9.3, 0, 0, 10),
        (38, 4.8, 5.8, 11, 1, -24, -16, 9.3, 0, 0, 4),
        (42, 3.3, 2.7, 6, 2, -13, -13, 9.3, 0, 0, 0),
        (48, 1.8, .7, 2, 1, -1, -10, 6, 3.4, 7, 0),
        (54, .7, -.35, -.5, .3, 4, -5, 2, 2, 2, 0),
        (60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    )),
    "fall": (("blend", "roll", "hip_shift", "hip_drop", "chest_pitch", "chest_width",
              "chest_length", "head_tilt", "leg_top", "knee_top", "leg_low", "knee_low",
              "arm_top", "elbow_top", "arm_low", "elbow_low", "arm_spread", "ground", "back_arch",
              "left_arm_settle"), (
        (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
        (7, .33, 1.5, .45, 4, -2, .01, 0, -5, 8, -19, 5, -13, -4, 13, 3, 10, 5, 0, 0, 0),
        (14, .69, 6, 1.2, 8, -3, .04, -.015, -9, 16, -31, 11, -25, -9, 21, 8, 14, 15, 0, 2, 0),
        (20, .88, 16, 2.3, 10.2, 0, .07, -.03,
         -11, 22, -34, 15, -29, -13, 24, 10, 16, 23, 0, 5.5, 0),
        (SICKLE_RELEASE_FRAME, .92, 23, 3.1, 10.9, 1.5, .09, -.04,
         -12, 23, -34, 16, -31, -13, 26, 12, 15, 25, 0, 8.5, 0),
        (29, .97, 39, 4.8, 11.7, 4, .12, -.055, -13, 24, -33, 16, -33,
         -14, 28, 14, 14, 27, .24, 11.5, 0),
        (40, 1, 66, 7.3, 8.8, 4, .18, -.08, -17, 28, -39, 11, -29,
         -18, 34, 10, 8, 24, .72, 7.5, .15),
        (50, 1, 81, 8.3, 4.8, -2, .22, -.10, -21, 26, -36, 5, -22,
         -21, 30, 8, 3, 19, .96, 2.3, .71),
        (60, 1, 80, 8.4, 3.9, -4, .22, -.10, -19, 24, -33, 4, -20,
         -20, 28, 6, 0, 17, 1, 0, 1),
    )),
    "settle": (("leg_splay", "boot_turn"), (
        (0, 0, 0), (7, 1.35, 1.8), (22, 2.55, 4), (40, 3.85, 7.55), (60, 4.4, 9.5),
    )),
}

NAMES = ("Hełm", "Lewa noga", "Prawa noga", "Napierśnik i płaszcz", "Lewe ramię", "Prawe ramię")


def find_curves(obj):
    data = obj.animation_data
    return data.action.layers[0].strips[0].channelbag(data.action_slot).fcurves


def animate(scene, name, channels, keys, first=1, span=60):
    control = bpy.data.objects.new(name, None)
    scene.collection.objects.link(control)
    for frame, *values in keys:
        for channel, value in zip(channels, values):
            control[channel] = float(value)
            control.keyframe_insert(data_path=f'["{channel}"]', frame=first+frame*span/60, group=name)
    curves = [find_curves(control).find(f'["{channel}"]') for channel in channels]
    for channel, curve in zip(channels, curves):
        for point in curve.keyframe_points:
            point.handle_left_type = point.handle_right_type = "AUTO_CLAMPED"
            if name == "step" and channel == "reach" and point.co.x < 37:
                point.interpolation = "LINEAR"
            if name == "step" and channel == "reach" and point.co.x == 37:
                point.handle_right_type = "FREE"
                point.handle_right = (38.5, -.30-1.5/60)
            if name == "step" and channel == "reach" and point.co.x == 61:
                point.handle_left_type = "FREE"
                point.handle_left = (59.5, .30+1.5/60)
        if name in ("walk", "step"):
            curve.modifiers.new("CYCLES")
    values = [[curve.evaluate(first+frame*span/60) for curve in curves] for frame in range(61)]
    if name in ("walk", "step") and values[0] != values[-1]:
        raise ValueError(f"Open cycle: {name}")
    if name == "step" and any(abs(row[0]-(.30-frame/60)) > .00001 or row[1] != 0
                              for frame, row in enumerate(values[:37])):
        raise ValueError("Planted foot must follow the ground for the support phase")
    return control, values


def material(path):
    image = bpy.data.images.load(str(path), check_existing=True)
    image.pack()
    mat = bpy.data.materials.new(path.stem)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    nodes.clear()
    texture = nodes.new("ShaderNodeTexImage")
    texture.image = image
    emission = nodes.new("ShaderNodeEmission")
    transparent = nodes.new("ShaderNodeBsdfTransparent")
    mix = nodes.new("ShaderNodeMixShader")
    output = nodes.new("ShaderNodeOutputMaterial")
    for source, target in ((texture.outputs["Color"], emission.inputs[0]),
                           (texture.outputs["Alpha"], mix.inputs[0]),
                           (transparent.outputs[0], mix.inputs[1]),
                           (emission.outputs[0], mix.inputs[2]),
                           (mix.outputs[0], output.inputs[0])):
        mat.node_tree.links.new(source, target)
    return mat


def card(scene, name, pivot, depth, mat, rows=(0, 160), columns=(0, 160)):
    x, y = pivot
    mesh = bpy.data.meshes.new(name)
    vertices = [(u-x, y-row, 0) for row in rows for u in columns]
    faces = [(i, i+1, i-1, i-2) for i in range(2, len(vertices), 2)]
    mesh.from_pydata(vertices, [], faces)
    uv = mesh.uv_layers.new()
    for loop in mesh.loops:
        u, v, _ = vertices[loop.vertex_index]
        uv.uv[loop.index].vector = ((u+x)/160, 1+(v-y)/160)
    obj = bpy.data.objects.new(name, mesh)
    scene.collection.objects.link(obj)
    obj.location = (x-80, 144-y, depth)
    obj.data.materials.append(mat)
    return obj


def driver(obj, path, index, control, channel, base=0., factor=1.):
    target = obj.driver_add(path, index).driver
    variable = target.variables.new()
    variable.name = "curve"
    variable.type = "SINGLE_PROP"
    variable.targets[0].id = control
    variable.targets[0].data_path = f'["{channel}"]'
    target.expression = f"{base} + {factor} * curve"


def preview_attack_leg(item, control, lead, sole):
    item.shape_key_add(name="Poza bazowa", from_mix=False)
    handle_range = 16
    hips = (1, 1, .53, 0, 0)
    foot = (0, 0, .47, 1, 1)
    controls = [("shift", "miednica", hips, (1.25, 0)),
                ("drop", "ugięcie", hips, (0, -1.25))]
    if lead:
        controls += [("lead_reach", "wykrok", foot, (1.25, -.42)),
                     ("lead_lift", "uniesienie stopy", foot, (0, 1.25))]
    for channel, label, weights, offset in controls:
        shape = item.shape_key_add(name=label, from_mix=False)
        shape.slider_min = -1
        for index, vertex in enumerate(shape.data):
            weight = weights[index//2]*handle_range
            vertex.co.x += offset[0]*weight
            vertex.co.y += offset[1]*weight
        driver(item.data.shape_keys, shape.path_from_id("value"), -1, control, channel,
               factor=1/handle_range)
    shape = item.shape_key_add(name="Przetoczenie stopy", from_mix=False)
    shape.slider_min = -1
    angle = -radians(handle_range)
    for vertex in list(shape.data)[6:]:
        x, y = vertex.co.x-sole[0], vertex.co.y-sole[1]
        vertex.co.x = sole[0]+cos(angle)*x-sin(angle)*y
        vertex.co.y = sole[1]+sin(angle)*x+cos(angle)*y
    driver(item.data.shape_keys, shape.path_from_id("value"), -1, control,
           "lead_toe" if lead else "rear_toe", factor=1/handle_range)


def preview_fall_joint(item, control, channel, joint):
    item.shape_key_add(name="Poza bazowa", from_mix=False)
    shape = item.shape_key_add(name="Zgięcie stawu", from_mix=False)
    shape.slider_min = -1
    degrees = 45
    angle = -radians(degrees)
    for vertex in list(shape.data)[4:]:
        x, y = vertex.co.x-joint[0], vertex.co.y-joint[1]
        strength = min(1., max(0., -y/12.))
        c, s = cos(angle*strength), sin(angle*strength)
        vertex.co.x = joint[0]+c*x-s*y
        vertex.co.y = joint[1]+s*x+c*y
    driver(item.data.shape_keys, shape.path_from_id("value"), -1, control, channel,
           factor=1/degrees)


def preview_relaxed_boot(item, control, sole, outward):
    shape = item.shape_key_add(name="Rozchylenie stopy", from_mix=False)
    degrees = 12
    c, s = cos(radians(outward*degrees)), sin(radians(outward*degrees))
    for vertex in list(shape.data)[8:]:
        x, y = vertex.co.x-sole[0], vertex.co.y-sole[1]
        vertex.co.x = sole[0]+c*x-s*y
        vertex.co.y = sole[1]+s*x+c*y
    driver(item.data.shape_keys, shape.path_from_id("value"), -1, control,
           "boot_turn", factor=1/degrees)


def preview_sickle(scene, arm, pivot):
    blade = Path(__file__).resolve().parents[2]/"assets/desert/reaper-turn/sickle.png"
    grip = (48, 32)
    size = .39/.8
    mesh = bpy.data.meshes.new("Sierp z gry")
    corners = ((0, 96), (96, 96), (96, 0), (0, 0))
    mesh.from_pydata([((x-grip[0])*size, (grip[1]-y)*size, 0) for x, y in corners],
                     [], [(0, 1, 2, 3)])
    uv = mesh.uv_layers.new()
    for loop in mesh.loops:
        x, y = corners[loop.vertex_index]
        uv.data[loop.index].uv = (x/96, 1-y/96)
    weapon = bpy.data.objects.new("Sierp · niezależny tor upadku", mesh)
    scene.collection.objects.link(weapon)
    weapon.data.materials.append(material(blade))

    ground_angle = -radians(82)
    start = SICKLE_RELEASE_FRAME
    grips = []
    with bpy.context.temp_override(scene=scene, view_layer=scene.view_layers[0]):
        for frame in (*range(start+1), 60):
            scene.frame_set(frame+1)
            graph = bpy.context.evaluated_depsgraph_get()
            transform = arm.evaluated_get(graph).matrix_world
            grips.append((transform @ Vector((60-pivot[0], pivot[1]-87, .1)),
                          transform.to_euler().z-radians(8)))
        release, angle = grips[start]
        previous, previous_angle = grips[start-1]
        landing = grips[-1][0]+Vector((-7.5/.8, -8.5/.8, 0))
        dx, dy, _ = landing-release
        flight = 60-start
        vy = max(.70*min(dy, 0), min(2.5/.8, (release.y-previous.y)*flight))

        def drift(distance, velocity, t):
            limit = distance*1.8
            initial = max(min(0, limit), min(max(0, limit), velocity))
            return distance*t+(initial-distance)*t*(1-t)**2

        for frame in range(61):
            if frame<=start:
                position, rotation = grips[frame]
                flatten = 0
            else:
                t = (frame-start)/flight
                position = Vector((release.x+drift(dx, (release.x-previous.x)*flight, t),
                                   release.y+vy*t+(dy-vy)*t*t, 1))
                spin = remainder(ground_angle-angle, 2*pi)
                rotation = angle+drift(spin, remainder(angle-previous_angle, 2*pi)*flight, t)
                fade = max(0., (t-.45)/.55)
                flatten = fade*fade*(3-2*fade)
            weapon.location = (position.x, position.y, 1)
            weapon.rotation_euler.z = rotation
            weapon.scale.y = 1-.45*flatten
            for path in ("location", "rotation_euler", "scale"):
                weapon.keyframe_insert(data_path=path, frame=frame+1)


def preview(scene, paths, rig, control, clip, settle=None):
    root = bpy.data.objects.new("Kotwica Żniwiarza", None)
    scene.collection.objects.link(root)
    order = rig.get("order", (2, 4, 1, 3, 0, 5))
    feet = rig.get("feet")
    parts = []
    for p, (name, mat) in enumerate(zip(NAMES, paths)):
        pivot = rig["pivots"][p]
        rows = (0, 160)
        columns = (0, 160)
        joint = None
        if clip in ("walk", "attack") and feet and p in (1, 2):
            foot = feet[p-1]
            knee = pivot[1]+.47*(foot[1]-pivot[1])
            rows = (0, pivot[1], knee, foot[1], 160)
            if clip == "attack":
                columns = (max(0, min(pivot[0], foot[0])-20),
                           min(160, max(pivot[0], foot[0])+20))
        if clip == "fall" and ((feet and p in (1, 2)) or p in (4, 5)):
            tip = feet[p-1] if p<3 else ((103, 83) if p==4 else (60, 87))
            portion = .47 if p<3 else .63
            joint = (pivot[0]+portion*(tip[0]-pivot[0]), pivot[1]+portion*(tip[1]-pivot[1]))
            rows = (0, pivot[1], joint[1], tip[1], 160)
            if p<3:
                ankle = pivot[1]+.84*(tip[1]-pivot[1])
                rows = (0, pivot[1], joint[1], ankle, tip[1], 160)
            columns = (max(0, min(pivot[0], tip[0])-20),
                       min(160, max(pivot[0], tip[0])+20))
        item = card(scene, name, pivot, order.index(p)*.06, mat, rows, columns)
        item.parent = root
        parts.append(item)
        if joint:
            channel = ("knee_low", "knee_top", "elbow_low", "elbow_top")[(1, 2, 4, 5).index(p)]
            preview_fall_joint(item, control, channel,
                               (joint[0]-pivot[0], pivot[1]-joint[1]))
            if p<3:
                outward = 1 if tip[0]>=feet[2-p][0] else -1
                preview_relaxed_boot(item, settle,
                                     (tip[0]-pivot[0], pivot[1]-tip[1]), outward)
        if len(rows)>2 and clip == "attack":
            preview_attack_leg(item, control, lead=p == 1,
                               sole=(foot[0]-pivot[0], pivot[1]-foot[1]))
        if len(rows)>2 and clip == "walk":
            center = (feet[0][0]+feet[1][0])*.5
            item.shape_key_add(name="Poza bazowa")
            stance = item.shape_key_add(name="Węższy rozstaw")
            for index, weight in enumerate((0, 0, .47, 1, 1)):
                for vertex in stance.data[2*index:2*index+2]:
                    vertex.co.x += weight*(center-foot[0])
            driver(item.data.shape_keys, stance.path_from_id("value"), -1,
                   control, "stance_width", base=1, factor=-1)
    if clip == "fall":
        driver(root, "location", 0, control, "hip_shift")
        driver(root, "location", 1, control, "hip_drop", 37, -1)
        driver(root, "rotation_euler", 2, control, "roll", factor=-.017453293)
        driver(parts[3], "scale", 0, control, "chest_width", 1)
        driver(parts[3], "scale", 1, control, "chest_length", 1)
        for p in (0, 4, 5):
            parts[p].parent = parts[3]
            parts[p].location -= parts[3].location
        for p, channel in enumerate(("head_tilt", "leg_low", "leg_top", "chest_pitch", "arm_low", "arm_top")):
            driver(parts[p], "rotation_euler", 2, control, channel, factor=-.017453293)
            if p in (1, 2) and feet:
                swing = parts[p].animation_data.drivers.find("rotation_euler", index=2).driver
                splay = swing.variables.new()
                splay.name, splay.type = "splay", "SINGLE_PROP"
                splay.targets[0].id = settle
                splay.targets[0].data_path = '["leg_splay"]'
                outward = 1 if feet[p-1][0]>=feet[2-p][0] else -1
                swing.expression += f" + {radians(outward)} * splay"
            if p in (4, 5):
                value = parts[p].animation_data.drivers.find("rotation_euler", index=2).driver
                spread = value.variables.new()
                spread.name = "spread"
                spread.type = "SINGLE_PROP"
                spread.targets[0].id = control
                spread.targets[0].data_path = '["arm_spread"]'
                value.expression += f" + {radians(1 if p == 4 else -1)} * spread"
        pitch = parts[3].animation_data.drivers.find("rotation_euler", index=2).driver
        arch = pitch.variables.new()
        arch.name = "arch"
        arch.type = "SINGLE_PROP"
        arch.targets[0].id = control
        arch.targets[0].data_path = '["back_arch"]'
        pitch.expression += f" - {radians(1)} * arch"
        left = parts[4].animation_data.drivers.find("rotation_euler", index=2).driver
        for name, channel in (("settle", "left_arm_settle"), ("roll", "roll"),
                              ("pitch", "chest_pitch"), ("arch", "back_arch")):
            variable = left.variables.new()
            variable.name = name
            variable.type = "SINGLE_PROP"
            variable.targets[0].id = control
            variable.targets[0].data_path = f'["{channel}"]'
        x, y = rig["pivots"][4]
        ground_angle = atan2(-.48, -1)-atan2(y-83, 103-x)
        left.expression = (f"({left.expression})*(1-settle)"
                           f"+settle*({ground_angle}+{radians(1)}*(roll+pitch+arch))")
        preview_sickle(scene, parts[5], rig["pivots"][5])
    elif clip == "attack":
        hips = bpy.data.objects.new("Biodra i tułów", None)
        scene.collection.objects.link(hips)
        hips.parent = root
        hips.location = (*parts[3].location[:2], 0)
        for p in (0, 3, 4, 5):
            item = parts[p]
            item.parent = hips
            item.location -= hips.location
        driver(hips, "location", 0, control, "shift", hips.location.x, 1.25)
        driver(hips, "location", 1, control, "drop", hips.location.y, -1.25)
        driver(hips, "rotation_euler", 2, control, "pitch", factor=-.017453293)
        for p, channel in ((0, "head"), (4, "free_arm")):
            driver(parts[p], "rotation_euler", 2, control, channel, factor=-.017453293)
    else:
        driver(root, "location", 0, control, "shift")
        driver(parts[3], "location", 1, control, "drop", parts[3].location.y, -1)
        for p, channel in ((0, "head"), (3, "pitch"), (4, "free_arm")):
            driver(parts[p], "rotation_euler", 2, control, channel, factor=-.017453293)
    camera = bpy.data.cameras.new("Ortograficzna")
    camera.type, camera.ortho_scale = "ORTHO", 212 if clip == "fall" else 172
    scene.camera = bpy.data.objects.new(camera.name, camera)
    scene.collection.objects.link(scene.camera)
    scene.camera.location = (28, 74, 250) if clip == "fall" else (20, 62, 250)


def bake(paint, output, receipt):
    paint, output = Path(paint), Path(output)
    output.mkdir(parents=True, exist_ok=True)
    rig = json.loads((paint/"rig.json").read_text())
    paths = [material(paint/f"{i}.png") for i in range(6)]
    lines = ["#pragma once", "", "// F-curves sampled in Blender at 60 intervals.",
             "namespace ember::reaper_motion {"]
    names = {"walk": "Chód", "attack": "Cięcie", "fall": "Upadek"}
    scenes = []
    for clip in names:
        scene = bpy.data.scenes.new(names[clip])
        scenes.append(scene)
        scene.frame_start, scene.frame_end, scene.render.fps = 1, 61, 60
        channels, keys = TRACKS[clip]
        controller, values = animate(scene, clip, channels, keys)
        if clip == "walk":
            controller["stance_width"] = .70
            controller.id_properties_ui("stance_width").update(min=0., max=1.)
            lines.append(f'inline constexpr float StanceWidth={controller["stance_width"]:.5f}f;')
            lines.append("")
        leg_settle = None
        if clip == "fall":
            scene.frame_end += LEG_SETTLE_FRAMES
            leg_settle, settle_samples = animate(scene, "settle", *TRACKS["settle"],
                                                 first=61, span=LEG_SETTLE_FRAMES)
            lines.append(f"inline constexpr float LegSettleSeconds={LEG_SETTLE_SECONDS:.7f}f;")
            lines += cpp_track("LegSettle", TRACKS["settle"][0], settle_samples)
        preview(scene, paths, rig, controller, clip, settle=leg_settle)
        scene.render.resolution_x = scene.render.resolution_y = 256
        scene.render.resolution_percentage = 100
        scene.render.image_settings.color_mode = "RGBA"
        scene.render.film_transparent = True
        scene.view_settings.view_transform = "Standard"
        scene.render.threads_mode, scene.render.threads = "FIXED", 2
        scene.frame_set(scene.frame_end if clip == "fall" else 1)
        if clip == "walk":
            step, values_step = animate(scene, "step", *TRACKS["step"])
            step_right, _ = animate(scene, "step_right", TRACKS["step"][0],
                                    [(i, *values_step[(i+30)%60]) for i in range(61)])
            for p, track in ((1, step), (2, step_right)):
                item = next(obj for obj in scene.objects if obj.name.split(".")[0] == NAMES[p])
                driver(item, "rotation_euler", 2, track, "reach", factor=.61)
                driver(item, "location", 1, track, "lift", base=item.location.y, factor=4.2)
            lines += cpp_track("Step", *TRACKS["step"][:1], values_step)
        if clip == "fall":
            lines.append(f"inline constexpr float SickleRelease={SICKLE_RELEASE_FRAME}.f/60;")
        lines += cpp_track(clip.capitalize(), channels, values)
    master = output/"Szklany-Zniwiarz.blend"
    bpy.data.libraries.write(str(master), set(scenes), fake_user=True, compress=True)
    (output/"reaper_motion.hpp").write_text("\n".join(lines + ["}", ""]))
    started = perf_counter()
    scene.render.filepath = str(output/"poza-upadku.png")
    bpy.ops.render.render(scene=scene.name, write_still=True)
    Path(receipt).write_text(json.dumps({
        "master": str(master), "render_seconds": perf_counter()-started, "samples_per_track": 61,
    }), encoding="utf-8")


def cpp_track(name, channels, values):
    lines = [f"struct {name} {{float {','.join(channels)};}};",
             f"inline constexpr std::array<{name},61> {name.lower()}{{{{"]
    for row in values:
        lines.append(" {" + ",".join(f"{v:.5f}f" for v in row) + "},")
    return lines + ["}};",
                    f"inline {name} sample{name}(float phase){{",
                    " const float at=std::clamp(phase,0.f,1.f)*60;",
                    " const int i=std::min(59,int(at));",
                    f" const auto& a={name.lower()}[i];const auto& b={name.lower()}[i+1];",
                    " const float t=at-i;",
                    " return {" + ",\n         ".join(f"a.{c}+(b.{c}-a.{c})*t" for c in channels) + "};",
                    "}", ""]

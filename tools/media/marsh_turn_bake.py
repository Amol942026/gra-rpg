"""Bake the existing planar Blender shape keys on a fixed subpixel grid.

Call bake(master, output) in Blender with a GPU context. The sheets feed
marsh_turn_pack.py. All poses use identical paint, sample offsets and filtering.
"""
import json
import struct
import time
import zlib
from pathlib import Path

import bpy
import gpu
import numpy as np
from gpu_extras.batch import batch_for_shader

WIDTH, HEIGHT = 240, 236
SAMPLES = 4
REST_HEIGHTS = (77, 66, 99, 146)
SPECIES = ("Ropucha", "Pijawka", "Błędnik", "Dzwonnik")
VIEWS = ("N", "NW", "W", "SW", "S")


def paint_shader():
    vary = gpu.types.GPUStageInterfaceInfo("marsh_turn_uv")
    vary.smooth("VEC2", "uv")
    info = gpu.types.GPUShaderCreateInfo()
    info.vertex_in(0, "VEC2", "point")
    info.vertex_in(1, "VEC2", "paintUV")
    info.vertex_out(vary)
    info.sampler(0, "FLOAT_2D", "painting")
    info.fragment_out(0, "VEC4", "result")
    info.vertex_source("void main(){uv=paintUV; gl_Position=vec4(point,0.,1.);}")
    # The explicit texture contains the PNG's sRGB bytes normalized to floats.
    # Premultiply before reducing coverage so transparent fringes add no dark halo.
    info.fragment_source("""
        void main() {
            vec4 ink = textureLod(painting, uv, 0.);
            result = vec4(ink.rgb * ink.a, ink.a);
        }
    """)
    return gpu.shader.create_from_info(info)


def save_png(path, rgba):
    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data
                + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff))

    h, w, _ = rgba.shape
    rows = rgba.copy()
    rows[:, 1:] -= rgba[:, :-1]
    data = b"".join(b"\x01" + row.tobytes() for row in rows)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">2I5B", w, h, 8, 6, 0, 0, 0))
                     + chunk(b"IDAT", zlib.compress(data, 4)) + chunk(b"IEND", b""))


def render_pose(fb, shader, paint, xy, uv, triangles):
    # Geometry is in native cel pixels (origin top-left). Crop the GPU readback.
    left, top = np.maximum(0, np.floor(xy.min(axis=0)).astype(int) - 1)
    right, bottom = np.minimum([WIDTH, HEIGHT], np.ceil(xy.max(axis=0)).astype(int) + 1)
    clip = xy * np.float32([2 / WIDTH, -2 / HEIGHT]) + np.float32([-1, 1])
    batch = batch_for_shader(shader, "TRIS", {"point": clip, "paintUV": uv}, indices=triangles)
    fb.clear(color=(0., 0., 0., 0.))
    shader.bind()
    shader.uniform_sampler("painting", paint)
    batch.draw(shader)
    w, h = right-left, bottom-top
    sub = SAMPLES
    pixels = fb.read_color(int(left*sub), int((HEIGHT-bottom)*sub), int(w*sub), int(h*sub), 4, 0, "UBYTE")
    values = np.frombuffer(pixels, dtype=np.uint8).reshape(h*sub, w*sub, 4)[::-1]
    sums = values.reshape(h, sub, w, sub, 4).sum(axis=(1, 3), dtype=np.uint32)
    a = sums[:, :, 3:]
    result = np.empty((h, w, 4), dtype=np.uint8)
    result[:, :, :3] = np.minimum(255, (sums[:, :, :3]*255 + a//2) // np.maximum(1, a))
    result[:, :, 3] = ((a[:, :, 0] + sub*sub//2) // (sub*sub)).astype(np.uint8)
    return left, top, result


def render_view(scene, species, fb, shader, output):
    actor = next(obj for obj in scene.objects if obj.type == "MESH" and obj.data.shape_keys)
    mesh = actor.data
    mesh.calc_loop_triangles()
    triangles = [tuple(t.vertices) for t in mesh.loop_triangles]
    keys = mesh.shape_keys.key_blocks
    if len(keys) != 65:
        raise ValueError(f"{scene.name}: expected 64 poses plus the basis")
    node = next(n for n in mesh.materials[0].node_tree.nodes if n.type == "TEX_IMAGE")
    # Upload a single level explicitly. Some Radeon drivers crash when Blender
    # builds a full mip chain for a newly linked image inside an offscreen draw.
    w, h = node.image.size
    source = np.empty(w*h*4, np.float32)
    node.image.pixels.foreach_get(source)
    paint = gpu.types.GPUTexture((w, h), format="RGBA16F",
                                 data=gpu.types.Buffer("FLOAT", len(source), source))
    uv = np.empty((len(mesh.vertices), 2), dtype=np.float32)
    for loop, coord in zip(mesh.loops, mesh.uv_layers.active.uv):
        uv[loop.vertex_index] = coord.vector[:]
    xyz = np.empty((len(mesh.vertices), 3), dtype=np.float32)
    poses = []
    sheet = np.zeros((8*HEIGHT, 8*WIDTH, 4), dtype=np.uint8)
    origin = np.float32([WIDTH*.5, HEIGHT*.5 + REST_HEIGHTS[species]*.43])
    for i, key in enumerate(list(keys)[1:]):
        key.data.foreach_get("co", xyz.ravel())
        xy = xyz[:, :2] * np.float32([1, -1]) + origin
        poses.append(xy.reshape(32, 28, 2))
        x, y, rgba = render_pose(fb, shader, paint, xy, uv, triangles)
        x += (i % 8)*WIDTH
        y += (i // 8)*HEIGHT
        sheet[y:y+rgba.shape[0], x:x+rgba.shape[1]] = rgba
    output.parent.mkdir(parents=True, exist_ok=True)
    save_png(output, sheet)
    return np.stack(poses)


def bake(master, output, select=None):
    master, output = Path(master), Path(output)
    libs_before = set(bpy.data.libraries)
    old_blend, old_depth = gpu.state.blend_get(), gpu.state.depth_test_get()
    timings = []
    vertices = np.zeros((4, 5, 64, 32, 28, 2), dtype=np.float32)
    begin = time.perf_counter()
    with bpy.data.libraries.load(str(master), link=True) as (source, loaded):
        loaded.scenes = list(source.scenes)
    offscreen = gpu.types.GPUOffScreen(WIDTH*SAMPLES, HEIGHT*SAMPLES)
    try:
        shader = paint_shader()
        with offscreen.bind():
            gpu.state.blend_set("NONE")
            gpu.state.depth_test_set("NONE")
            fb = gpu.state.active_framebuffer_get()
            for scene in loaded.scenes:
                s = next(i for i, name in enumerate(SPECIES) if scene.name.startswith(name))
                v = VIEWS.index(scene.name.split(" | ")[1].split(".")[0])
                if select is not None and (s, v) not in select:
                    continue
                tick = time.perf_counter()
                vertices[s, v] = render_view(scene, s, fb, shader, output/VIEWS[v]/f"render-{s}.png")
                timings.append({"species": s, "view": v, "seconds": round(time.perf_counter()-tick, 3)})
    finally:
        offscreen.free()
        gpu.state.blend_set(old_blend)
        gpu.state.depth_test_set(old_depth)
        for lib in set(bpy.data.libraries) - libs_before:
            bpy.data.libraries.remove(lib)
    if select is None and {(s["species"], s["view"]) for s in timings} != {(s, v) for s in range(4) for v in range(5)}:
        raise RuntimeError("Incomplete turn")
    if select is None:
        np.savez_compressed(output/"pose-vertices.npz", xy=vertices)
    record = dict(version=bpy.app.version_string, master=str(master), views=VIEWS, frames=64*len(timings),
                  raster="fixed 4x4 subpixel coverage", sheets=timings, render_seconds=round(time.perf_counter()-begin, 3))
    (output/"render.json").write_text(json.dumps(record, ensure_ascii=False, indent=2), encoding="utf-8")
    return record["render_seconds"]

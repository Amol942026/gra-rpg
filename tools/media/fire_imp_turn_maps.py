"""Register the imp's edited turn guides to the original Krita paintings.

Each RGBA texel stores two backwards UV offsets. The renderer interpolates
coordinates in this map, then reads paint from one original direction. The
intermediate Comfy/Synfig views supply correspondence, never final pigment.
"""
from pathlib import Path
import re

import cv2
import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt
from scipy.interpolate import RBFInterpolator

from caldera_turn_registration import WORK, optical, remap, swatch, xy

GRID = 48


def donor_threshold(u, v, part):
    return 0.50 + 0.34 * (u - 0.5) + 0.27 * (v - 0.5) + 0.013 * (part - 2.5)


def contour(rgba):
    alpha = cv2.resize(rgba[..., 3], (WORK, WORK), interpolation=cv2.INTER_LINEAR) / 255.
    inside = alpha >= .5
    distance = np.where(inside, distance_transform_edt(inside) - .5,
                        .5 - distance_transform_edt(~inside))
    edge = (alpha > .01) & (alpha < .99)
    distance[edge] = alpha[edge] - .5
    return distance.astype(np.float32)


def limb_coordinates(shape, donor):
    # Each toe or claw stays at the end of its own limb even when a side view
    # points across the cell. Match the vertical span, then the painted runs
    # within each scanline before following individual scales optically.
    def spans(mask):
        filled = mask >= 0
        rows = np.flatnonzero(filled.any(axis=1))
        lo = np.argmax(filled[rows], axis=1)
        hi = WORK - 1 - np.argmax(filled[rows, ::-1], axis=1)
        return rows, lo, hi
    rows, left, right = spans(shape)
    keys, key_left, key_right = spans(donor)
    x, y = xy[..., 0], xy[..., 1]
    sy = keys[0] + (y - rows[0]) * (keys[-1]-keys[0]) / (rows[-1]-rows[0])
    a, b = np.interp(y, rows, left), np.interp(y, rows, right)
    c, d = np.interp(sy, keys, key_left), np.interp(sy, keys, key_right)
    progress = np.clip((x-a) / np.maximum(b-a, 1), 0., 1.)
    sx = c + (d-c)*progress + np.minimum(x-a, 0) + np.maximum(x-b, 0)
    return np.float32(np.stack((sx, sy), axis=-1))


def head_coordinates(points, target, target_tile, key_tile):
    # Use the painted correspondences of the pupils, horns, nostrils and jaw.
    # An optical match alone can stretch a bright eye across the cheek.
    def tile_space(tile, grid):
        _, _, w, h, ox, oy = tile
        return (grid + .5) * np.array((w, h)) / WORK + np.array((ox, oy)) - .5
    grid = tile_space(target_tile, xy)
    warp = RBFInterpolator(target, points-target, kernel="thin_plate_spline", smoothing=1.1)
    mapped = grid + warp(grid.reshape(-1, 2)).reshape(grid.shape)
    _, _, w, h, ox, oy = key_tile
    return np.float32((mapped - np.array((ox, oy)) + .5) * WORK / np.array((w, h)) - .5)


def registered_uv(guide, key_guide, shape, key_shape, part, initial=None):
    # A painterly optical field can follow a bright claw into the backdrop.
    # Fit its boundary normals to the traced cutout so wrists, tail and horns
    # stay continuous where the visible paint switches sides.
    if part in (1, 2, 4, 5):
        initial = limb_coordinates(shape, key_shape)
    if initial is not None:
        correction = optical(guide, remap(key_guide, initial))
        bound = 2.5 if part == 3 else 4
        where = remap(initial, xy + np.clip(correction, -bound, bound))
    else:
        where = xy + optical(guide, key_guide)
    dy, dx = np.gradient(cv2.GaussianBlur(key_shape, (0, 0), .8))
    grad = np.stack((dx, dy), axis=-1)
    for _ in range(8):
        reached = remap(key_shape, where)
        normal = remap(grad, where)
        magnitude = np.maximum(np.sum(normal*normal, axis=-1), .20)
        # Distance is meaningful at the outline, while interior features
        # should retain their optical correspondence and perspective.
        weight = np.clip(1 - (np.minimum(abs(reached), abs(shape)) - 2) / 10, 0, 1)
        delta = np.clip((shape-reached) * weight / magnitude, -3., 3.)
        where += delta[..., None] * normal
    return (where - xy) / WORK


def crisp_sample(rgba, u, v):
    """Narrow bilinear footprint on premultiplied paint, with transparent gutter."""
    h, w = rgba.shape[:2]
    valid = (u >= 0) & (u <= 1) & (v >= 0) & (v <= 1)
    x, y = u * w - 0.5, v * h - 0.5
    def sharpen(coord):
        f = np.clip((coord - np.floor(coord) - 0.13) / 0.74, 0, 1)
        return np.float32(np.floor(coord) + f * f * (3 - 2 * f))
    pm = rgba.astype(np.float32) / 255
    pm[..., :3] *= pm[..., 3:]
    sample = cv2.remap(pm, sharpen(x), sharpen(y), cv2.INTER_LINEAR)
    sample[..., :3] /= np.maximum(sample[..., 3:], 1 / 65536)
    sample[~valid] = 0
    return np.uint8(np.rint(np.clip(sample, 0, 1) * 255))


def register(source, header, output, landmarks):
    cv2.setNumThreads(2)
    tiles = [tuple(map(int, t)) for t in re.findall(
        r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}", Path(header).read_text()
    )]
    if len(tiles) != 192:
        raise ValueError("Expected the imp's 32 directions and six layers")
    atlas = Image.open(source).convert("RGBA")
    maps = np.full((6 * GRID, 8 * 5 * GRID, 4), 128, dtype=np.uint8)
    for part in range(6):
        cells, guides, shapes = [], [], []
        for x, y, w, h, _, _ in tiles[part::6]:
            cell = atlas.crop((x, y, x + w, y + h))
            cells.append(np.array(cell))
            guides.append(swatch(cell)[1])
            shapes.append(contour(cells[-1]))
        for base in range(8):
            first, last = 4 * base, (4 * base + 4) % 32
            for sub in range(5):
                view = (first + sub) % 32
                node = 5 * base + sub
                fields = []
                for key in (first, last):
                    if view == key:
                        field = np.zeros((GRID, GRID, 2), dtype=np.float32)
                    else:
                        initial = None
                        if part == 3:
                            left, right = landmarks(base, last//4), landmarks(last//4, base)
                            target = left + (right-left)*(sub/4)
                            initial = head_coordinates(left if key==first else right, target,
                                                       tiles[view*6+part], tiles[key*6+part])
                        flow = registered_uv(guides[view], guides[key], shapes[view], shapes[key], part, initial)
                        field = cv2.resize(flow, (GRID, GRID), interpolation=cv2.INTER_AREA)
                    fields.append(field)
                encoded = np.uint8(np.rint(128 + 255 * np.dstack(fields)).clip(0, 255))
                maps[part*GRID:(part+1)*GRID, node*GRID:(node+1)*GRID] = encoded
                if sub in (0, 4):
                    continue
                # Bake the same one-source rule for the shaderless renderer.
                x, y, w, h, _, _ = tiles[view * 6 + part]
                u, v = np.meshgrid((np.arange(w) + .5) / w, (np.arange(h) + .5) / h)
                offsets = (cv2.resize(encoded.astype(np.float32), (w, h)) - 128) / 255
                left = crisp_sample(cells[first], u + offsets[..., 0], v + offsets[..., 1])
                right = crisp_sample(cells[last], u + offsets[..., 2], v + offsets[..., 3])
                use_right = (sub / 4 >= donor_threshold(u, v, part))[..., None]
                atlas.paste(Image.fromarray(np.where(use_right, right, left)), (x, y))
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    atlas.save(output / "parts-0.png", optimize=True)
    Image.fromarray(maps).save(output / "turn-flow.png", optimize=True)

"""Register Krita cutouts and generate 32 directional paintings for the animated drake."""

import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter
from scipy import ndimage as nd
from scipy.interpolate import RBFInterpolator


CELL = 160
UV = 112
PARTS = (0, 1, 1, 2, 2, 3, 4, 5)
ORIGINS = ((91, 70), (89, 43), (47, 42), (7, 43), (5, 70))
SAMPLE = Image.Resampling.LANCZOS
VIEWS = 32


def authored(key, part):
    return 4 - part if key in (2, 3, 4, 5) and part in (0, 4) else part


def feet(image, facing):
    """Separate diagonal partners before warping so they keep their own footholds."""
    rgba = np.asarray(image)
    labels, count = nd.label(rgba[..., 3] > 70)
    sizes = np.bincount(labels.ravel(), minlength=count + 1)
    largest = np.argsort(sizes[1:])[-2:] + 1
    if len(largest) == 2 and sizes[largest[0]] >= 20:
        largest = sorted(largest, key=lambda i: np.where(labels == i)[1].mean())
        near = nd.distance_transform_edt(labels != largest[0]) < nd.distance_transform_edt(
            labels != largest[1]
        )
    elif facing in (1, 3):
        # The far rear ankle crosses the near front forearm in this drawing.
        # Follow the sketched occlusion edge before separating the two paws.
        y, x = np.indices(rgba.shape[:2])
        edge = np.interp(y, [115, 125, 135, 145, 153, 163, 176], [78, 85, 91, 98, 102, 102, 103])
        near = x < edge if facing == 1 else x < 191 - edge
    else:
        raise ValueError("Missing diagonal pair of paws")
    result = []
    for mask in (near, ~near):
        pixels = rgba.copy()
        pixels[..., 3] *= mask
        result.append(Image.fromarray(pixels))
    return result


def sample(plane, x, y):
    if plane.ndim == 3:
        return np.stack([sample(plane[..., i], x, y) for i in range(plane.shape[2])], axis=-1)
    return nd.map_coordinates(plane, [y, x], order=1, mode="nearest", prefilter=False)


class Paint:
    """Scanline correspondence in pigment-space, with the contour kept separately."""

    def __init__(self, image):
        self.image = image.resize((CELL, CELL), SAMPLE)
        pixels = np.asarray(self.image)
        alpha = pixels[..., 3] / 255.0
        solid = alpha > 0.45
        self.distance = (
            nd.distance_transform_edt(solid) - nd.distance_transform_edt(~solid) + np.where(solid, -0.5, 0.5)
        ).astype(np.float32)
        edges = (alpha > 0.03) & (alpha < 0.97)
        self.distance[edges] = alpha[edges] - 0.5
        near = nd.distance_transform_edt(alpha < 0.08, return_distances=False, return_indices=True)
        self.color = pixels[tuple(near)][..., :3].astype(np.float32)
        rows = np.where(solid.any(axis=1))[0]
        mass = alpha[rows].sum(axis=1)
        # Damped mass coordinates preserve the relative thickness of the skull,
        # horns, membranes and toes while allowing their extremes to rotate.
        v = np.linspace(0, 1, UV)
        yq = np.interp(v, (np.cumsum(mass) - mass * 0.5) / mass.sum(), rows)
        self.y = 0.48 * yq + 0.52 * (rows[0] + (rows[-1] - rows[0]) * v)
        xleft = np.array([np.where(solid[y])[0][0] for y in rows], float)
        xright = np.array([np.where(solid[y])[0][-1] for y in rows], float)
        # Expand the correspondence rectangle by several pixels to retain
        # antialiasing and the narrow membrane tips in its edge falloff.
        self.left = nd.gaussian_filter1d(np.interp(self.y, rows, xleft), 1.4) - 3
        self.right = nd.gaussian_filter1d(np.interp(self.y, rows, xright), 1.4) + 3
        self.left = 0.84 * self.left + 0.16 * (xleft.min() - 3)
        self.right = 0.84 * self.right + 0.16 * (xright.max() + 3)
        self.y += np.linspace(-2, 2, UV)
        u = np.linspace(0, 1, UV)[None, :]
        x = self.left[:, None] + u * (self.right - self.left)[:, None]
        self.texture = sample(self.color, x, np.broadcast_to(self.y[:, None], x.shape))


GRID_Y, GRID_X = np.mgrid[:CELL, :CELL].astype(float)
SAMPLES = np.arange(UV)


# Tail registration in the original 192 px Krita cells (base to barbed tip).
# The flank and the tail overlap under the hips, so their correspondences must
# be solved independently when the tail passes behind the body.
TAIL = {
    0: [
        (74, 134, 19),
        (62, 140, 14),
        (44, 149, 9),
        (25, 152, 7),
        (13, 148, 6),
        (12, 138, 4),
        (18, 129, 5),
        (26, 124, 7),
        (31, 118, 4),
    ],
    1: [
        (78, 120, 21),
        (59, 128, 16),
        (44, 142, 10),
        (25, 149, 7),
        (15, 143, 5),
        (15, 133, 4),
        (20, 123, 4),
        (30, 116, 7),
        (38, 108, 4),
    ],
    2: [
        (83, 112, 13),
        (70, 119, 11),
        (60, 128, 6),
        (54, 136, 5),
        (46, 134, 5),
        (44, 126, 4),
        (47, 118, 4),
        (53, 113, 6),
        (58, 105, 4),
    ],
    6: [
        (96, 112, 22),
        (95, 132, 15),
        (96, 153, 11),
        (98, 167, 8),
        (105, 175, 5),
        (116, 179, 4),
        (124, 176, 4),
        (129, 168, 7),
        (139, 161, 4),
    ],
    7: [
        (97, 123, 23),
        (77, 141, 16),
        (54, 162, 12),
        (35, 170, 8),
        (21, 169, 6),
        (13, 163, 5),
        (13, 153, 4),
        (21, 144, 6),
        (30, 135, 4),
    ],
}


def tail_points(key):
    source = TAIL[{3: 1, 4: 0, 5: 7}.get(key, key)]
    points = np.array(source, dtype=float)
    if key in (3, 4, 5):
        points[:, 0] = 191 - points[:, 0]
    return points * CELL / 192


def along_tail(points):
    """Nearest longitudinal and signed transverse coordinate on the tail."""
    delta = np.diff(points[:, :2], axis=0)
    length = np.linalg.norm(delta, axis=1)
    length = np.maximum(length, 0.1)
    tangent = delta / length[:, None]
    best = np.full_like(GRID_X, np.inf)
    segment = np.zeros_like(GRID_X, dtype=np.int32)
    longitudinal = np.zeros_like(GRID_X)
    across = np.zeros_like(GRID_X)
    for i, ((x, y, _), (tx, ty)) in enumerate(zip(points, tangent)):
        dx, dy = GRID_X - x, GRID_Y - y
        dist = dx * tx + dy * ty
        u = np.clip(dist / length[i], 0.0, 1.0)
        lateral = dy * tx - dx * ty
        squared = (dist - u * length[i]) ** 2 + lateral * lateral
        replace = squared < best
        segment[replace] = i
        longitudinal[replace] = u[replace]
        across[replace] = lateral[replace]
        best = np.minimum(best, squared)
    radius = points[segment, 2] * (1 - longitudinal) + points[segment + 1, 2] * longitudinal
    return segment, longitudinal, across / radius, np.sqrt(best), radius


class TailPaint:
    def __init__(self, full, key):
        self.full = full
        self.points = tail_points(key)
        i, u, _, dist, radius = along_tail(self.points)
        # Only the free length leaves the flank. A tapered overlap under the
        # hip makes the two pieces meet even in a foreshortened front view.
        erasure = np.clip(i + u - 0.65, 0, 1) * np.clip(radius + 3 - dist, 0, 1)
        rgba = np.array(full.image)
        rgba[..., 3] = np.uint8(np.rint(rgba[..., 3] * (1 - erasure)))
        self.flank = Paint(Image.fromarray(rgba))


def tail_between(a, b, t):
    points = (1 - t) * a.points + t * b.points
    # Opposite sides can project a curved segment in opposite directions.
    # Preserve a little breadth as the barb turns through a three-quarter pose.
    for i in range(len(points) - 1):
        da = a.points[i + 1, :2] - a.points[i, :2]
        db = b.points[i + 1, :2] - b.points[i, :2]
        angle0 = np.arctan2(da[1], da[0])
        angle1 = np.arctan2(db[1], db[0])
        delta = (angle1 - angle0 + np.pi) % (2 * np.pi) - np.pi
        turn = angle0 + t * delta
        length = (1 - t) * np.linalg.norm(da) + t * np.linalg.norm(db)
        delta_linear = (1 - t) * da + t * db
        points[i + 1, :2] = (
            points[i, :2] + 0.76 * delta_linear + 0.24 * length * np.array([np.cos(turn), np.sin(turn)])
        )
    segment, u, v, dist, radius = along_tail(points)

    def ref(tail):
        p0 = tail.points[segment]
        p1 = tail.points[segment + 1]
        c = p0 * (1 - u[..., None]) + p1 * u[..., None]
        dx, dy = (p1 - p0)[..., 0], (p1 - p0)[..., 1]
        normal = v * c[..., 2] / np.maximum(0.1, np.hypot(dx, dy))
        x = c[..., 0] - normal * dy
        y = c[..., 1] + normal * dx
        return sample(tail.full.color, x, y), sample(tail.full.distance, x, y)

    c0, d0 = ref(a)
    c1, d1 = ref(b)
    weight = (1 - t) * np.clip(d0 + 1, 0.02, 1)
    other = t * np.clip(d1 + 1, 0.02, 1)
    rgb = (c0 * weight[..., None] + c1 * other[..., None]) / (weight + other)[..., None]
    alpha = np.clip((1 - t) * d0 + t * d1 + 0.5, 0, 1) * np.clip(radius + 2 - dist, 0, 1)
    return Image.fromarray(np.uint8(np.rint(np.dstack((rgb, 255 * alpha)))))


# Neck base / nape / crest, visible horn tip, horn base, near eye, snout,
# opposite horn tip, base and eye. Attach highlights to the skull rather
# than interpolating scales according to their horizontal position.
SKULL = {
    0: [
        (139, 135),
        (123, 112),
        (135, 101),
        (136, 87),
        (155, 101),
        (167, 112),
        (177, 120),
        (143, 92),
        (158, 100),
        (169, 110),
    ],
    1: [
        (129, 118),
        (114, 88),
        (129, 74),
        (135, 59),
        (155, 85),
        (161, 99),
        (173, 116),
        (157, 62),
        (170, 85),
        (170, 97),
    ],
    2: [
        (95, 111),
        (82, 96),
        (94, 68),
        (87, 60),
        (86, 81),
        (88, 93),
        (95, 107),
        (103, 60),
        (105, 81),
        (101, 93),
    ],
    6: [
        (95, 81),
        (83, 72),
        (95, 50),
        (87, 55),
        (85, 64),
        (89, 65),
        (96, 52),
        (103, 55),
        (106, 64),
        (103, 65),
    ],
    7: [
        (156, 99),
        (145, 84),
        (154, 62),
        (148, 57),
        (150, 69),
        (162, 77),
        (172, 76),
        (163, 59),
        (168, 69),
        (168, 76),
    ],
}


def skull_points(key, other, ref):
    points = np.array(SKULL[{3: 1, 4: 0, 5: 7}.get(key, key)], float)
    if key in (3, 4, 5) or (key == 2 and other == 3) or (key == 6 and other == 5):
        points[:, 0] = 191 - points[:, 0]
    points *= CELL / 192
    # Jaw motion is below the eye line; keep the protruding lower jaw
    # inside the same deformation used for the unarticulated upper skull.
    left, up, right, down = ref.image.getchannel("A").getbbox()
    pad = 8
    points = np.vstack(
        (
            points,
            (
                (left - pad, up - pad),
                (right + pad, up - pad),
                (right + pad, down + pad),
                (left - pad, down + pad),
            ),
        )
    )
    return points


def skull_between(a, b, first, nextkey, t):
    p = skull_points(first, nextkey, a)
    q = skull_points(nextkey, first, b)
    target = p * (1 - t) + q * t
    domain = np.stack((GRID_X, GRID_Y), axis=-1)
    region = (
        (GRID_X >= target[-4:, 0].min())
        & (GRID_X <= target[-4:, 0].max())
        & (GRID_Y >= target[-4:, 1].min())
        & (GRID_Y <= target[-4:, 1].max())
    )

    def reference(points, ref):
        warp = RBFInterpolator(target, points - target, kernel="thin_plate_spline", smoothing=1.3)
        xy = domain.copy()
        xy[region] += warp(domain[region])
        return sample(ref.color, xy[..., 0], xy[..., 1]), sample(ref.distance, xy[..., 0], xy[..., 1])

    c0, d0 = reference(p, a)
    c1, d1 = reference(q, b)
    edge = (1 - t) * d0 + t * d1
    alpha = np.clip(edge + 0.5, 0, 1) * region
    # Keep a single paint line for eyes, horns and nostrils. The soft
    # material seam slides across the registered skull during the turn.
    lo, hi = target[:-4, 0].min(), target[:-4, 0].max()
    across = np.clip((GRID_X - lo) / max(1.0, hi - lo), 0, 1)
    material = np.clip((t - (0.095 + 0.38 * across)) / 0.43, 0, 1)
    material = material * material * (3 - 2 * material)
    w0 = (1 - material) * np.clip(d0 + 1, 0.015, 1)
    w1 = material * np.clip(d1 + 1, 0.015, 1)
    rgb = (c0 * w0[..., None] + c1 * w1[..., None]) / np.maximum((w0 + w1)[..., None], 0.001)
    im = Image.fromarray(np.uint8(np.rint(np.dstack((np.clip(rgb, 0, 255), alpha * 255)))))
    sharp = im.convert("RGB").filter(ImageFilter.UnsharpMask(radius=0.7, percent=70, threshold=4))
    sharp.putalpha(im.getchannel("A"))
    return sharp


def inbetween(a, b, t, film=None, *, retain_detail=False, one_paint=False, whole_paint=False):
    def lerp(left, right):
        return (1 - t) * left + t * right

    height = lerp(a.y, b.y)
    v = np.interp(GRID_Y[:, 0], height, SAMPLES)
    left = np.interp(v, SAMPLES, lerp(a.left, b.left))[:, None]
    width = np.interp(v, SAMPLES, lerp(a.right - a.left, b.right - b.left))[:, None]
    u = (GRID_X - left) / width

    def reference(ref):
        x = (
            np.interp(v, SAMPLES, ref.left)[:, None]
            + u * np.interp(v, SAMPLES, ref.right - ref.left)[:, None]
        )
        y = np.broadcast_to(np.interp(v, SAMPLES, ref.y)[:, None], u.shape)
        return x, y, sample(ref.distance, x, y)

    x0, y0, sd0 = reference(a)
    x1, y1, sd1 = reference(b)
    distance = lerp(sd0, sd1)
    alpha = np.clip(distance + 0.5, 0, 1)
    alpha[(GRID_Y < height[0]) | (GRID_Y > height[-1])] = 0
    if whole_paint:
        # Keep every mark on a part in the same painting and resample its
        # pigment once. The signed silhouette still follows both views.
        donor, x, y = (a, x0, y0) if t < .5 else (b, x1, y1)
        rgb = np.stack([nd.map_coordinates(donor.color[..., index], [y, x], order=3, mode="nearest")
                        for index in range(3)], axis=-1)
        return Image.fromarray(np.uint8(np.rint(np.dstack((np.clip(rgb, 0, 255), 255 * alpha)))))
    rgb0 = sample(a.color, x0, y0)
    rgb1 = sample(b.color, x1, y1)
    if one_paint:
        # The contour travels continuously. A moving seam reveals one set of
        # brush marks at a time, so fur, ears and straps never have two edges.
        seam = np.clip(.50 + .55 * (u - .5) + .22 * (v[:, None] / (UV - 1) - .5), .13, .87)
        second = t >= seam
        second = np.where((sd0 < -.25) & (sd1 >= -.25), True, second)
        second = np.where((sd1 < -.25) & (sd0 >= -.25), False, second)
        rgb = np.where(second[..., None], rgb1, rgb0)
        return Image.fromarray(np.uint8(np.rint(np.dstack((rgb, 255 * alpha)))))
    # A tip revealed in just one key picks up that key's paint. Dark matte
    # pixels from uncovered skin cannot bleed into the highlight.
    w0 = (1 - t) * np.clip(sd0 + 1, 0.03, 1)
    w1 = t * np.clip(sd1 + 1, 0.03, 1)
    color = (rgb0 * w0[..., None] + rgb1 * w1[..., None]) / (w0 + w1)[..., None]
    if film is not None:
        neural = sample(film, u * (UV - 1), np.broadcast_to(v[:, None], u.shape))
        weight = 0.72 * (4 * t * (1 - t)) * np.clip(np.minimum(sd0, sd1), 0, 1)
        color += (neural - color) * weight[..., None]
    if retain_detail:
        # The local model determines the broad paint. Preserve the defined
        # edges of the ram's fur and plates from an aligned nearby painting.
        limit = .52 + .33 * (u - .5) + .16 * (v[:, None] / (UV - 1) - .5)
        blend = np.clip((t - limit + .075) / .15, 0, 1)
        blend = np.where(sd0 < -.5, 1, np.where(sd1 < -.5, 0, blend))
        low = nd.gaussian_filter(color, (1.35, 1.35, 0))
        d0 = rgb0 - nd.gaussian_filter(rgb0, (1.35, 1.35, 0))
        d1 = rgb1 - nd.gaussian_filter(rgb1, (1.35, 1.35, 0))
        detailed = low + 1.12 * (d0 + blend[..., None] * (d1 - d0))
        w = (.57 + .30 * (4 * t * (1 - t))) * np.clip(distance + .2, 0, 1)
        color += (detailed - color) * w[..., None]
    return Image.fromarray(np.uint8(np.rint(np.dstack((np.clip(color, 0, 255), 255 * alpha)))))


def inputs(atlas, jaws):
    atlas = Image.open(atlas).convert("RGBA")
    jaws = Image.open(jaws).convert("RGBA")
    sprites = []
    opening = []
    for d in range(8):
        parts = []
        for p in range(6):
            row = authored(d, p)
            im = atlas.crop((192 * d, 192 * row, 192 * d + 192, 192 * row + 192))
            parts.extend(feet(im, d) if p in (1, 2) else [im])
        sprites.append([Paint(im) for im in parts])
        levels = []
        if d < 5:
            for g in range(8):
                full = Image.new("RGBA", (192, 192))
                tile = jaws.crop((d * 96, g * 96, (d + 1) * 96, (g + 1) * 96))
                full.paste(tile, ORIGINS[d])
                levels.append(Paint(full))
        else:
            levels = [sprites[-1][-1]] * 8
        opening.append(levels)
    return sprites, opening


def prepare(root, sprites):
    frames = []
    for parts in sprites + sprites[:1]:
        sheet = Image.new("RGB", (UV * 4, UV * 2))
        for i, part in enumerate(parts):
            sheet.paste(Image.fromarray(np.uint8(np.rint(part.texture))), ((i % 4) * UV, (i // 4) * UV))
        frames.append(sheet)
    name = "ember-drake-angular-materials.png"
    frames[0].save(root / name, save_all=True, append_images=frames[1:], duration=100, loop=0)
    flow = {
        "1": {"class_type": "LoadImage", "inputs": {"image": name}},
        "2": {
            "class_type": "FrameInterpolationModelLoader",
            "inputs": {"model_name": "film_net_fp16.safetensors"},
        },
        "3": {
            "class_type": "FrameInterpolate",
            "inputs": {"images": ["1", 0], "interp_model": ["2", 0], "multiplier": 4},
        },
        "4": {
            "class_type": "SaveImage",
            "inputs": {"images": ["3", 0], "filename_prefix": "ember-drake-angular-materials/turn"},
        },
    }
    (root / "film-workflow.json").write_text(json.dumps(flow, indent=2), encoding="utf-8")


def make_cells(root, sprites, opening, film, write_cells=True):
    colors = sorted(film.glob("turn_*.png")) if film else []
    if film and len(colors) != 33:
        raise ValueError(f"FILM returned {len(colors)} of 33 paintings")
    cells = root / "cells"
    if write_cells:
        cells.mkdir(exist_ok=True)
    views = []
    mouths = []
    tails = [TailPaint(parts[5], key) for key, parts in enumerate(sprites)]
    for i in range(VIEWS):
        k = i // 4
        nxt = (k + 1) % 8
        t = i % 4 / 4
        result = [Image.new("RGBA", (CELL, CELL)) for _ in range(6)]
        plate = Image.open(colors[i]).convert("RGB") if colors else None
        for n, part in enumerate(PARTS):
            uv = None
            if plate and t:
                u, v = (n % 4) * UV, (n // 4) * UV
                uv = np.array(plate.crop((u, v, u + UV, v + UV))).astype(np.float32)
            if t and n == 5:
                im = tail_between(tails[k], tails[nxt], t)
                im.alpha_composite(inbetween(tails[k].flank, tails[nxt].flank, t))
            elif t and n == 7:
                im = skull_between(sprites[k][n], sprites[nxt][n], k, nxt, t)
            else:
                im = inbetween(sprites[k][n], sprites[nxt][n], t, uv) if t else sprites[k][n].image
            result[part].alpha_composite(im)
        if write_cells:
            for p, im in enumerate(result):
                im.save(cells / f"{i:02}-{p}.png")
        views.append(result)
        levels = []
        for g in range(8):
            if any(d < 5 for d in (k, nxt)) and t:
                im = skull_between(opening[k][g], opening[nxt][g], k, nxt, t)
            else:
                im = opening[k][g].image if not t else result[5]
            levels.append(im)
        mouths.append(levels)
    return views, mouths


def use_edited_cells(views, mouths, directory):
    for i in range(VIEWS):
        for p in range(6):
            with Image.open(directory / f"{i:02}-{p}.png") as image:
                if image.mode != "RGBA" or image.size != (CELL, CELL):
                    raise ValueError(f"Expected an editable RGBA {CELL}px cell: view {i}, part {p}")
                old = np.asarray(views[i][p])
                new = np.asarray(image)
                if np.array_equal(old[..., 3], new[..., 3]) and np.array_equal(
                    old[new[..., 3] > 0], new[new[..., 3] > 0]
                ):
                    continue
                edited = image.copy()
            views[i][p] = edited
            if p == 5:
                mouths[i][0] = edited
                if 20 <= i <= 28:
                    mouths[i] = [edited] * 8


def pack(root, output, header, views, jaws):
    stores = []
    refs = {}
    identical = {}
    for tag, rows in (("body", views), ("mouth", jaws)):
        for i, row in enumerate(rows):
            for n, im in enumerate(row):
                box = im.getchannel("A").getbbox()
                if box is None:
                    raise ValueError(f"Empty tile {tag}/{i}/{n}")
                tile = im.crop(box)
                key = (tile.size, hashlib.sha256(tile.tobytes()).digest())
                if key not in identical:
                    identical[key] = len(stores)
                    stores.append(tile)
                refs[tag, i, n] = (identical[key], box[0], box[1])
    places = {}
    shelf = []
    extent = 0
    width = 2048
    for i in sorted(range(len(stores)), key=lambda i: -stores[i].height):
        im = stores[i]
        w, h = im.width + 4, im.height + 4
        candidates = [r for r in shelf if r[2] >= h and r[1] + w <= width]
        if candidates:
            row = min(candidates, key=lambda r: r[2] - h)
        else:
            row = [extent, 0, h]
            shelf.append(row)
            extent += h
        places[i] = (row[1] + 2, row[0] + 2)
        row[1] += w
    if extent > 2048:
        raise ValueError(f"Geometry exceeds atlas budget: {extent}")
    page = Image.new("RGBA", (width, extent))
    for idx, tile in enumerate(stores):
        page.paste(tile, places[idx])
    output.mkdir(parents=True, exist_ok=True)
    page.save(output / "parts-0.png", optimize=True)
    lines = [
        "#pragma once",
        "// Generated by tools/media/ember_drake_turn.py.",
        "namespace ember_drake_turn_atlas {",
        f"inline constexpr int Count={VIEWS},Parts=6,Cell={CELL},PageCount=1;",
        "struct Tile {short page,x,y,w,h,ox,oy;};",
        "inline constexpr int authoredPart(int key,int part){",
        " return key>=2 && key<=5 && (part==0 || part==4) ? 4-part : part;",
        "}",
    ]
    for tag, name, count in (("body", "tiles", 6), ("mouth", "mouth", 8)):
        lines.append(f"inline constexpr Tile {name}[Count][{count}]={{")
        for d in range(VIEWS):
            row = []
            for p in range(count):
                i, ox, oy = refs[tag, d, p]
                x, y = places[i]
                w, h = stores[i].size
                row.append("{" + ",".join(map(str, (0, x, y, w, h, ox, oy))) + "}")
            lines.append(" {" + ",".join(row) + "},")
        lines.append("};")
    lines.append("}")
    header.write_text("\n".join(lines) + "\n", encoding="utf-8")
    review = Image.new("RGB", (CELL * 8, CELL * 4), (43, 42, 47))
    for i, parts in enumerate(views):
        d = ((i + 2) // 4) % 8
        rawOrder = (0, 1, 2, 5, 3, 4) if d >= 5 else (0, 1, 2, 3, 4, 5)
        im = Image.new("RGBA", (CELL, CELL))
        for n in rawOrder:
            im.alpha_composite(parts[authored(d, n)])
        review.paste(im, ((i % 8) * CELL, (i // 8) * CELL), im)
    review.save(root / "turn-contact.png")
    print(json.dumps(dict(size=page.size, unique=len(stores), parts=VIEWS * 6, openings=VIEWS * 8)))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["prepare", "pack"])
    parser.add_argument("root", type=Path)
    parser.add_argument("--film", type=Path)
    parser.add_argument(
        "--edited-cells", type=Path, help="32 x 6 RGBA cells exported from the editable Krita master"
    )
    parser.add_argument("--output", type=Path, default=Path("assets/volcano/drake-turn"))
    parser.add_argument("--header", type=Path, default=Path("src/generated/ember_drake_turn_atlas.hpp"))
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    args.root.mkdir(exist_ok=True, parents=True)
    sprites, openings = inputs("assets/volcano/ember-drake.png", "assets/volcano/ember-drake-mouth.png")
    if args.action == "prepare":
        prepare(args.root, sprites)
    else:
        views, mouths = make_cells(args.root, sprites, openings, args.film, write_cells=not args.edited_cells)
        if args.edited_cells:
            use_edited_cells(views, mouths, args.edited_cells)
        pack(args.root, args.output, args.header, views, mouths)

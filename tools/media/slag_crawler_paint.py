"""Cut, register and pack the original 2-D paintings of the Caldera slag crawler."""

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageOps
from scipy import ndimage as nd
from scipy.interpolate import RBFInterpolator

from caldera_corpse_paint import cell_paint
from ember_drake_turn import Paint, UV, inbetween, sample


CELL = 160
PARTS = (
    "ogon",
    "lewa tylna",
    "prawa tylna",
    "pancerz i szczęki",
    "lewa przednia",
    "prawa przednia",
)
FILM = "film_net_fp16.safetensors"
SAMPLE = Image.Resampling.LANCZOS
# Previous footprint and vertical envelope in source pixels (192 px/cell).
# These are placement guides; the source illustrations supply every visible pixel.
BOUNDS = (
    (16, 87, 158, 167),
    (24, 87, 154, 172),
    (56, 87, 135, 179),
    (42, 87, 168, 172),
    (39, 87, 175, 167),
    (42, 87, 167, 172),
    (56, 87, 135, 179),
    (25, 87, 154, 172),
)

# Corresponding shell scale, eye, ivory and beak landmarks in painted keys.
# The axial front views have two eyes. Track the side visible in the adjacent
# three-quarter painting, so neither eye slides across the bridge of the nose.
LANDMARKS = {
    0: (
        (40, 99),
        (47, 87),
        (63, 78),
        (84, 73),
        (92, 96),
        (70, 120),
        (96, 121),
        (103, 102),
        (106, 108),
        (115, 114),
        (119, 104.5),
        (129, 107),
        (126, 117),
    ),
    1: (
        (39, 99),
        (48, 84),
        (61, 78),
        (81, 73),
        (76, 102),
        (63, 120),
        (92, 128),
        (86, 109),
        (88, 112),
        (99, 119),
        (103.8, 112),
        (121, 115),
        (115, 129),
    ),
    2: (
        (51, 103),
        (56, 92),
        (66, 81),
        (79.5, 73),
        (61, 102),
        (58, 118),
        (77, 133),
        (63, 115),
        (62, 120),
        (70, 126),
        (71, 117.5),
        (81, 122),
        (79.5, 133),
    ),
    7: (
        (39, 105),
        (48, 89),
        (67, 80),
        (87, 73),
        (94, 99),
        (62, 126),
        (100, 115),
        (111, 100),
        (118, 101),
        (120, 107),
        (123, 96),
        (127, 97),
        (123, 106),
    ),
    6: (
        (53, 103),
        (57, 90),
        (68, 82),
        (79, 73),
        (94, 99),
        (66, 124),
        (99, 118),
        (97, 104),
        (100, 107),
        (99, 111),
        (101, 101),
        (105, 103),
        (102, 110),
    ),
}


def shell_points(key, other):
    raw = np.array(LANDMARKS[{3: 1, 4: 0, 5: 7}.get(key, key)], dtype=float)
    if key in (3, 4, 5) or key == 2 and other == 3 or key == 6 and other == 5:
        raw[:, 0] = CELL - 1 - raw[:, 0]
    return np.vstack((raw, ((2, 54), (156, 54), (156, 154), (2, 154))))


def turn_shell(a, b, k, n, t):
    p, q = shell_points(k, n), shell_points(n, k)
    target = (1 - t) * p + t * q
    y, x = np.mgrid[64:151, 3:156]
    grid = np.stack((x, y), axis=-1).astype(float)

    def warped(ref, path):
        transform = RBFInterpolator(
            target, path - target, kernel="thin_plate_spline", smoothing=1.15
        )
        coords = grid + transform(grid.reshape(-1, 2)).reshape(grid.shape)
        return sample(ref.color, coords[..., 0], coords[..., 1]), sample(
            ref.distance, coords[..., 0], coords[..., 1]
        )

    left, ld = warped(a, p)
    right, rd = warped(b, q)
    alpha = np.clip(ld * (1 - t) + rd * t + 0.5, 0, 1)
    # Slide the material seam over the joint without doubling the eyes, plate
    # rims or ivory point. Only the geometry follows a weighted silhouette.
    side = np.clip((x - target[:, 0].min()) / np.ptp(target[:, 0]), 0, 1)
    weight = np.clip((t - (0.10 + 0.25 * side)) / 0.50, 0, 1)
    weight = weight * weight * (3 - 2 * weight)
    wl = (1 - weight) * np.clip(ld + 1, 0.02, 1)
    wr = weight * np.clip(rd + 1, 0.02, 1)
    rgb = (left * wl[..., None] + right * wr[..., None]) / (wl + wr)[..., None]
    tile = Image.new("RGBA", (CELL, CELL))
    im = Image.fromarray(
        np.uint8(np.rint(np.dstack((np.clip(rgb, 0, 255), alpha * 255))))
    )
    sharp = im.convert("RGB").filter(
        ImageFilter.UnsharpMask(radius=0.65, percent=65, threshold=4)
    )
    sharp.putalpha(im.getchannel("A"))
    tile.paste(sharp, (3, 64))
    return tile


def panels(source, root, layout):
    src = Image.open(source).convert("RGBA")
    root.mkdir(parents=True, exist_ok=True)
    for key, box in layout.items():
        image = src.crop(tuple(box))
        if image.getchannel("A").getextrema()[0] > 12:
            image = remove_matte(image)
        else:
            mask = np.asarray(image.getchannel("A"))
            image.putalpha(Image.fromarray(np.uint8(np.where(mask >= 12, mask, 0))))
        crop = image.getchannel("A").getbbox()
        if not crop:
            raise ValueError(f"Empty source panel {key}")
        left, top, right, bottom = [round(s * CELL / 192) for s in BOUNDS[int(key)]]
        tile = Image.new("RGBA", (CELL, CELL))
        tile.paste(
            image.crop(crop).resize((right - left, bottom - top), SAMPLE), (left, top)
        )
        tile.save(root / f"key-{key}.png")


def remove_matte(image):
    rgb = np.asarray(image, dtype=np.float32)[..., :3]
    border = np.concatenate((rgb[0], rgb[-1], rgb[:, 0], rgb[:, -1]))
    bg = np.median(border, axis=0)
    dist = np.linalg.norm(rgb - bg, axis=-1)
    # Keep the entire connected silhouette, including neutral edge highlights.
    strong = dist > 54
    components, count = nd.label(strong)
    sizes = np.bincount(components.ravel(), minlength=count + 1)
    sizes[0] = 0
    selected = components == sizes.argmax()
    soft = np.clip((dist - 12) / 42, 0, 1)
    near = nd.binary_dilation(selected, iterations=2)
    alpha = soft * near
    solid = nd.binary_fill_holes(selected)
    alpha[solid] = 1
    straight = np.clip(
        (rgb - bg * (1 - alpha[..., None])) / np.maximum(alpha[..., None], 0.04), 0, 255
    )
    return Image.fromarray(np.uint8(np.rint(np.dstack((straight, alpha * 255)))))


def prepare_film(root, cells):
    frames = []
    for parts in cells + cells[:1]:
        frame = Image.new("RGB", (UV * 3, UV * 2))
        for p, part in enumerate(parts):
            swatch = Image.fromarray(np.uint8(np.clip(np.rint(part.texture), 0, 255)))
            frame.paste(swatch, ((p % 3) * UV, (p // 3) * UV))
        frames.append(frame)
    source = root / "crawler-turn-materials.png"
    frames[0].save(
        source,
        save_all=True,
        append_images=frames[1:],
        duration=120,
        loop=0,
        disposal=0,
        blend=0,
    )
    workflow = {
        "1": {"class_type": "LoadImage", "inputs": {"image": source.name}},
        "2": {
            "class_type": "FrameInterpolationModelLoader",
            "inputs": {"model_name": FILM},
        },
        "3": {
            "class_type": "FrameInterpolate",
            "inputs": {"images": ["1", 0], "interp_model": ["2", 0], "multiplier": 4},
        },
        "4": {
            "class_type": "SaveImage",
            "inputs": {
                "images": ["3", 0],
                "filename_prefix": "emberfall-slag-crawler/turn",
            },
        },
    }
    (root / "film-workflow.json").write_text(json.dumps(workflow, indent=2) + "\n")


def feather_polygon(points):
    mask = Image.new("L", (CELL * 4, CELL * 4))
    ImageDraw.Draw(mask).polygon(
        [(round(x * 4), round(y * 4)) for x, y in points], fill=255
    )
    return np.asarray(mask.resize((CELL, CELL), SAMPLE), dtype=np.float32) / 255


def cut(source, markers):
    """Remove articulated paintings and extend the belly under the limb roots."""
    pixels = np.asarray(source).copy()
    complete = pixels[..., 3] / 255.0
    cutouts = [None] * 6
    body = complete.copy()
    for p in (0, 1, 2, 4, 5):
        spec = markers[str(p)]
        if "shape" not in spec:
            continue
        mask = feather_polygon(spec["shape"])
        leg = pixels.copy()
        leg[..., 3] = np.uint8(np.rint(255 * complete * mask))
        cutouts[p] = Image.fromarray(leg)
        body *= 1 - mask
    # The split follows natural seams and the feet lift no more than a few
    # pixels. Extend the paint under every joint so the pose never opens holes.
    keep = body > 0.50
    nearest = nd.distance_transform_edt(
        ~keep, return_distances=False, return_indices=True
    )
    grown = nd.maximum_filter(body, size=9) * complete
    # A small overlap carries the shoulder seam. Avoid creating duplicate toes.
    near = np.maximum(body, grown)
    pigment = pixels[tuple(nearest)][..., :3].copy()
    pigment[keep] = pixels[..., :3][keep]
    cutouts[3] = Image.fromarray(np.uint8(np.dstack((pigment, np.rint(255 * near)))))
    for p in (0, 1, 2, 4, 5):
        if cutouts[p] is not None:
            continue
        spec = markers[str(p)]
        if "foreign" in spec:
            continue
        donor = cutouts[spec["from"]]
        left, top, right, bottom = spec["box"]
        tile = donor.crop(donor.getbbox()).resize((right - left, bottom - top), SAMPLE)
        if spec.get("mirror"):
            tile = ImageOps.mirror(tile)
        tint = np.asarray(tile).copy()
        tint[..., :3] = np.uint8(np.rint(tint[..., :3] * spec.get("light", 0.62)))
        cutouts[p] = Image.new("RGBA", (CELL, CELL))
        cutouts[p].paste(Image.fromarray(tint), (left, top))
    return cutouts


def read_keys(folder, markers):
    keys = {}
    for key, row in markers.items():
        if key.isnumeric():
            with Image.open(folder / f"key-{key}.png") as source:
                keys[int(key)] = cut(source, row)
    for key, parts in keys.items():
        for p, part in enumerate(parts):
            if part is not None:
                continue
            spec = markers[str(key)][str(p)]
            donor = keys[spec["foreign"]][p]
            x, y, w, h = spec["xywh"]
            fragment = donor.crop(donor.getbbox()).resize((w, h), SAMPLE)
            if spec.get("mirror"):
                fragment = ImageOps.mirror(fragment)
            parts[p] = Image.new("RGBA", (CELL, CELL))
            parts[p].paste(fragment, (x, y))
    for key, source in ((3, 1), (4, 0), (5, 7)):
        mirrored = [ImageOps.mirror(part) for part in keys[source]]
        keys[key] = [mirrored[p] for p in (0, 2, 1, 3, 5, 4)]
    return [keys[i] for i in range(8)]


def raw_order(key):
    front_right = (0, 1, 4, 3, 2, 5)
    front_left = (0, 2, 5, 3, 1, 4)
    return (
        front_right,
        front_right,
        (0, 1, 2, 3, 4, 5),
        front_left,
        front_left,
        (5, 2, 4, 3, 0, 1),
        (4, 5, 3, 1, 2, 0),
        (4, 1, 5, 3, 0, 2),
    )[key]


def atlas(root, views, output, header, *, name="slag_crawler", ordering=raw_order, width=1024):
    count = len(views)
    shelves = []
    extent = 0
    entries = []
    for d, row in enumerate(views):
        for p, part in enumerate(row):
            box = part.getchannel("A").getbbox()
            if not box:
                raise ValueError(f"Empty art {d}/{p}")
            entries.append((part.crop(box), box[:2], d, p))
    page_xy = {}
    for im, _, d, p in sorted(entries, key=lambda e: -e[0].height):
        w, h = im.width + 4, im.height + 4
        fits = [s for s in shelves if s[2] >= h and s[1] + w <= width]
        if fits:
            shelf = min(fits, key=lambda s: s[2] - h)
        else:
            shelf = [extent, 0, h]
            shelves.append(shelf)
            extent += h
        page_xy[d, p] = (shelf[1] + 2, shelf[0] + 2)
        shelf[1] += w
    if extent > 2048:
        raise ValueError("Atlas exceeds 2048 px")
    page = Image.new("RGBA", (width, extent))
    lines = [
        "#pragma once",
        '#include "../volcano_turn_tile.hpp"',
        f"// Generated by tools/media/{name}_paint.py.",
        f"namespace {name}_turn_atlas {{",
        "using Tile=volcano_turn_data::Tile;",
        f"inline constexpr int Count={count}, Parts=6, Cell=160, PageCount=1;",
        "inline constexpr int authoredPart(int, int part){return part;}",
        "inline constexpr Tile tiles[Count][Parts]={",
    ]
    rows = [[] for _ in range(count)]
    for im, origin, d, p in entries:
        x, y = page_xy[d, p]
        page.paste(im, (x, y))
        ox, oy = origin
        rows[d].append(
            "{" + ",".join(map(str, (0, x, y, im.width, im.height, ox, oy))) + "}"
        )
    lines.extend(" {" + ",".join(row) + "}," for row in rows)
    lines += ["};", "}"]
    output.mkdir(parents=True, exist_ok=True)
    page.save(output / "parts-0.png", optimize=True)
    header.write_text("\n".join(lines) + "\n")
    contact = Image.new("RGBA", (CELL * 8, CELL * ((count + 7) // 8)), (43, 41, 46, 255))
    steps = count // 8
    for i, parts in enumerate(views):
        for p in ordering(((i + steps // 2) // steps) % 8):
            contact.alpha_composite(parts[p], ((i % 8) * CELL, (i // 8) * CELL))
    preview = "crawler-contact.png" if name == "slag_crawler" else f"{name}-contact.png"
    contact.convert("RGB").save(root / preview)
    return page.size


def base_assets(root, keys):
    live = Image.new("RGBA", (8 * CELL, 6 * CELL))
    cooled = Image.new("RGBA", (8 * 128, 6 * 128))
    for k, row in enumerate(keys):
        for p, piece in enumerate(row):
            live.paste(piece, (k * CELL, p * CELL))
            _, material = cell_paint(piece, 0, p, k)
            cooled.paste(material, (k * 128, p * 128))
    live.save(root / "slag-crawler.png")
    cooled.save(root / "slag-crawler-relief.png")


def bake(root, markers, film, output, header, edited=None):
    keys = read_keys(root / "keys", markers)
    cuts = [[Paint(part) for part in row] for row in keys]
    if film is None and edited is None:
        prepare_film(root, cuts)
    frames = sorted(film.glob("turn_*.png")) if film else []
    if film and len(frames) != 33:
        raise ValueError("Expected 33 FILM frames")
    cells = root / "cells"
    cells.mkdir(exist_ok=True)
    views = []
    for i in range(32):
        k = i // 4
        t = (i % 4) / 4
        pigments = Image.open(frames[i]).convert("RGB") if frames else None
        row = []
        for p in range(6):
            a, b = cuts[k][p], cuts[(k + 1) % 8][p]
            if edited:
                im = Image.open(edited / f"{i:02}-{p}.png").convert("RGBA")
                if not t:
                    # Krita may zero RGB below fully transparent pixels. Preserve
                    # the exact source cutting when the painted key is unchanged.
                    new = np.asarray(im)
                    original = np.asarray(keys[k][p])
                    if np.array_equal(new[..., 3], original[..., 3]) and np.array_equal(
                        new[new[..., 3] > 0], original[original[..., 3] > 0]
                    ):
                        im = keys[k][p]
            elif not t:
                im = keys[k][p]
            elif p == 3:
                im = turn_shell(a, b, k, (k + 1) % 8, t)
            else:
                x, y = (p % 3) * UV, (p // 3) * UV
                pigment = (
                    np.asarray(pigments.crop((x, y, x + UV, y + UV)), dtype=np.float32)
                    if pigments
                    else None
                )
                im = inbetween(a, b, t, pigment)
            im.save(cells / f"{i:02}-{p}.png")
            row.append(im)
        views.append(row)
    base_assets(root, views[::4])
    return atlas(root, views, output, header)


if __name__ == "__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("source", type=Path)
    cli.add_argument("markers", type=Path)
    cli.add_argument("root", type=Path)
    cli.add_argument("--film", type=Path)
    cli.add_argument(
        "--prepare-film",
        action="store_true",
        help="Write the registered APNG and ComfyUI workflow, then stop.",
    )
    cli.add_argument("--edited-cells", type=Path)
    cli.add_argument("--output", type=Path)
    cli.add_argument("--header", type=Path)
    cli.add_argument("--no-audio", action="store_true", required=True)
    args = cli.parse_args()
    if not args.prepare_film and (args.output is None or args.header is None):
        cli.error("Packing requires --output and --header")
    markers = json.loads(args.markers.read_text(encoding="utf8"))
    panels(args.source, args.root / "keys", markers["panels"])
    if args.prepare_film:
        cutouts = read_keys(args.root / "keys", markers)
        prepare_film(args.root, [[Paint(part) for part in row] for row in cutouts])
    else:
        print(
            bake(
                args.root,
                markers,
                args.film,
                args.output,
                args.header,
                args.edited_cells,
            )
        )

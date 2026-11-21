"""Bake opaque directional cutouts and a single layer order for each Caldera turn cel."""

import argparse
import hashlib
import json
import re
from pathlib import Path

import cv2
import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt

from caldera_turn_registration import WORK, optical, remap, restore, swatch, xy


VIEWS = 32
SUBSTEPS = 4
SIZE = VIEWS * SUBSTEPS
EXTENT = 2048
PAD = 2
ORDERS = {
    "drake": ((0, 1, 2, 3, 4, 5),) * 5 + ((0, 1, 2, 5, 3, 4),) * 3,
    "crawler": ((0, 1, 4, 3, 2, 5), (0, 1, 4, 3, 2, 5), (0, 1, 2, 3, 4, 5),
                (0, 2, 5, 3, 1, 4), (0, 2, 5, 3, 1, 4), (5, 2, 4, 3, 0, 1),
                (4, 5, 3, 1, 2, 0), (4, 1, 5, 3, 0, 2)),
}


def tile_rows(path, species):
    boxes = [tuple(map(int, values)) for values in re.findall(
        r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}", path.read_text(encoding="utf-8"))]
    count = 14 if species == "drake" else 6
    if len(boxes) != VIEWS * count:
        raise ValueError(f"Incorrect number of cutouts in {path}")
    return [boxes[p:192:6] for p in range(6)] + [boxes[192 + j::8] for j in range(count - 6)]


def paint_source(piece):
    alpha = np.asarray(piece)[..., 3]
    solid = alpha >= 128
    distance = distance_transform_edt(solid) - distance_transform_edt(~solid)
    distance -= .5 * np.sign(distance)
    distance[(0 < alpha) & (alpha < 255)] = alpha[(0 < alpha) & (alpha < 255)] / 255 - .5
    color, _ = swatch(piece)
    signed = cv2.resize(distance.astype(np.float32), (WORK, WORK), interpolation=cv2.INTER_LINEAR)
    return np.dstack((color, signed))


def warp(art, field, fraction):
    # Work from this painting's own flow. Reversing the other painting's flow
    # sends newly exposed claws and horns back across the foreground edge.
    source = xy - fraction * field
    for _ in range(3):
        source = xy - fraction * remap(field, source)
    return remap(art, source)


def bake(left, right, forward, backward, position, width, height):
    a = warp(left, forward, position)
    b = warp(right, backward, 1 - position)
    alpha = np.clip(.5 + (1 - position) * a[..., 3] + position * b[..., 3], 0, 1)

    # Carry the line work from one registered drawing. Only the underlying
    # tone changes continuously. A donor outside the painted region can supply
    # tone, but the new visible edge needs the other drawing's detail.
    tone_a = cv2.GaussianBlur(a[..., :3], (0, 0), 2.1)
    tone_b = cv2.GaussianBlur(b[..., :3], (0, 0), 2.1)
    use_b = (b[..., 3] > a[..., 3] + .85) if position < .5 else (b[..., 3] > a[..., 3] - .85)
    detail = np.where(use_b[..., None], b[..., :3] - tone_b, a[..., :3] - tone_a)
    rgb = (1 - position) * tone_a + position * tone_b + detail
    # Registration resamples paint at fractional texel positions. Restore a
    # little contrast at that scale before the cel is sampled again in the game.
    brush = rgb - cv2.GaussianBlur(rgb, (0, 0), .9)
    rgb = np.clip(rgb + np.clip(brush * .46, -14, 14), 0, 255)
    premultiplied = np.dstack((rgb * alpha[..., None], alpha))
    result = cv2.resize(premultiplied, (width, height), interpolation=cv2.INTER_AREA)
    values = result[..., 3:]
    np.divide(result[..., :3], values, out=result[..., :3], where=values > 0)
    result[..., 3] *= 255
    return Image.fromarray(np.rint(result).clip(0, 255).astype(np.uint8))


def bake_row(sheet, boxes, part, species):
    pieces = []
    for x, y, w, h, ox, oy in boxes:
        piece = sheet.crop((x, y, x + w, y + h))
        if species == "crawler" and part == 3:
            l, t, r, b = piece.getchannel("A").point(lambda a: 255 if a >= 128 else 0).getbbox()
            piece = piece.crop((max(0, l-1), max(0, t-1), min(w, r+1), min(h, b+1)))
        pieces.append(piece)
    guides = [(piece, *swatch(piece)) for piece in pieces]
    paintings = [restore(guides, i) if i % 4 and part < 6 else piece for i, piece in enumerate(pieces)]
    rgba = [paint_source(piece) for piece in paintings]
    cells = []
    for i, a in enumerate(paintings):
        j = (i + 1) % VIEWS
        b = paintings[j]
        forward = optical(guides[i][2], guides[j][2])
        backward = optical(guides[j][2], guides[i][2])
        cells.append(a)
        for step in range(1, SUBSTEPS):
            position = step / SUBSTEPS
            w = round(a.width + (b.width - a.width) * position)
            h = round(a.height + (b.height - a.height) * position)
            cells.append(bake(rgba[i], rgba[j], forward, backward, position, w, h))
    return cells


def choose_orders(species, rows, boxes):
    def order(key):
        indices = ORDERS[species][key]
        if species == "drake" and 2 <= key <= 5:
            return tuple(4 - p if p in (0, 4) else p for p in indices)
        return indices

    result = []
    per_key = 4 * SUBSTEPS
    for base in range(8):
        first, second = order(base), order((base + 1) % 8)
        if first == second:
            result.extend([first] * per_key)
            continue

        def cost(step):
            index = base * per_key + step
            view, sub = divmod(index, SUBSTEPS)
            t = sub / SUBSTEPS
            layers = []
            for row, tiles in zip(rows[:6], boxes[:6], strict=True):
                a, b = tiles[view], tiles[(view + 1) % VIEWS]
                x, y = (round(a[k] + t * (b[k] - a[k])) for k in (4, 5))
                layers.append((row[index], (x, y)))
            resolved = []
            for indices in (first, second):
                frame = Image.new("RGBA", (160, 160))
                for p in indices:
                    frame.alpha_composite(*layers[p])
                resolved.append(np.asarray(frame, dtype=np.float32))
            a, b = resolved
            change = np.abs(a[..., :3] - b[..., :3]) * np.minimum(a[..., 3:], b[..., 3:]) / 255
            return float(change.sum()) + 90 * abs(step - per_key / 2)

        # Only switch the whole draw order, where changing overlap is least visible.
        swap = min(range(per_key // 4, 3 * per_key // 4 + 1), key=cost)
        result.extend([first] * swap + [second] * (per_key - swap))
    return result


def pack(rows, output):
    frames = [(frame, part, image) for part, row in enumerate(rows) for frame, image in enumerate(row)]
    records = [[None] * len(rows) for _ in range(SIZE)]
    pages = []
    atlas = Image.new("RGBA", (EXTENT, EXTENT))
    left = top = height = used_width = 0

    def save(used_height):
        path = output / f"turn-cels-{len(pages)}.png"
        atlas.crop((0, 0, used_width, used_height)).save(path, compress_level=5)
        pages.append({"path": path.name, "dimensions": [used_width, used_height],
                      "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})

    for frame, part, image in sorted(frames, key=lambda cell: -cell[2].height):
        w, h = image.width + 2 * PAD, image.height + 2 * PAD
        if left + w > EXTENT:
            top += height
            left = height = 0
        if top + h > EXTENT:
            save(top)
            atlas = Image.new("RGBA", (EXTENT, EXTENT))
            left = top = height = used_width = 0
        x, y = left + PAD, top + PAD
        atlas.paste(image, (x, y))
        records[frame][part] = (len(pages), x, y, image.width, image.height)
        height = max(height, h)
        left += w
        used_width = max(left, used_width)
    save(top + height)
    if not 1 <= len(pages) <= 2:
        raise ValueError("Too many texture pages for the Caldera renderer")
    return records, pages


def main(species, sheet, metadata, output):
    cv2.setNumThreads(2)
    output.mkdir(parents=True, exist_ok=False)
    with Image.open(sheet) as image:
        atlas = image.convert("RGBA")
    boxes = tile_rows(metadata, species)
    frames = [bake_row(atlas, row, part, species) for part, row in enumerate(boxes)]
    cells, pages = pack(frames, output)
    depth = choose_orders(species, frames, boxes)
    lines = ["#pragma once", "// Generated by tools/media/caldera_direction_cels.py.",
             '#include "../volcano_turn_tile.hpp"', f"namespace {species}_direction_cels {{",
             f"inline constexpr int Count={SIZE}, PageCount={len(pages)};",
             f"inline constexpr volcano_turn_data::Tile tiles[Count][{len(boxes)}]={{"]
    for row in cells:
        lines.append(" {" + ",".join("{" + ",".join(map(str, (*cell, 0, 0))) + "}" for cell in row) + "},")
    lines += ["};", "inline constexpr int order[Count][6]={"]
    for row in depth:
        lines.append(" {" + ",".join(map(str, row)) + "},")
    lines += ["};", "}"]
    (output / f"{species}_direction_cels.hpp").write_text("\n".join(lines) + "\n", encoding="utf-8")
    receipt = {"species": species, "source": str(sheet), "sha256": hashlib.sha256(sheet.read_bytes()).hexdigest(),
               "directions": SIZE, "parts_per_direction": len(boxes), "pages": pages,
               "gpu_rgba8_bytes": sum(p["dimensions"][0] * p["dimensions"][1] * 4 for p in pages)}
    (output / "report.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("species", choices=tuple(ORDERS))
    parser.add_argument("sheet", type=Path)
    parser.add_argument("metadata", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    main(args.species, args.sheet, args.metadata, args.output)

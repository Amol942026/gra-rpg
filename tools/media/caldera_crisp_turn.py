"""Pack registered 2D paint and signed contours for Caldera beasts."""

import argparse
from pathlib import Path
import re

import cv2
import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt

from caldera_turn_registration import GRID, WORK, optical, remap, restore, swatch, xy


def contours(piece, sharpen=0.0):
    pixels = np.asarray(piece)
    alpha = pixels[..., 3]
    solid = alpha >= 128
    # Signed distance measured at texel centers. Preserve the authored AA
    # values in the edge band and extend nearby pigment outside the silhouette.
    distance = distance_transform_edt(solid) - distance_transform_edt(~solid)
    distance -= np.sign(distance) * .5
    partial = (alpha > 0) & (alpha < 255)
    distance[partial] = alpha[partial] / 255 - .5
    donor = distance_transform_edt(alpha < 96, return_distances=False, return_indices=True)
    rgb = pixels[tuple(donor)][..., :3]
    if sharpen:
        # Recover a little contrast on the painted fur and edges of plates.
        # The extended pigment keeps the filter away from transparent black.
        paint = rgb.astype(np.float32)
        high = np.clip(paint - cv2.GaussianBlur(paint, (0, 0), .85), -24, 24)
        rgb = np.rint(paint + sharpen * high).clip(0, 255).astype(np.uint8)
    out = np.empty_like(pixels)
    out[..., :3] = rgb
    out[..., 3] = np.rint(128 + 12 * distance).clip(0, 255).astype(np.uint8)
    return Image.fromarray(out)


def registered_flow(a, b):
    forward, backward = optical(a, b), optical(b, a)
    u, v = forward, backward
    for _ in range(3):
        u = remap(forward, xy - .5*u)
        v = remap(backward, xy - .5*v)
    vector = cv2.resize(.5*(u-v), (GRID, GRID), interpolation=cv2.INTER_AREA)
    return np.rint(128 + vector*(384 / WORK)).clip(0, 255).astype(np.uint8)


def pack(source, header, output, sharpen=0.0):
    tiles = [tuple(map(int, values)) for values in re.findall(
        r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}", header.read_text(encoding="utf-8"))]
    if len(tiles) not in (32 * 6, 32 * 14):
        raise ValueError(f"Unexpected atlas metadata {len(tiles)} in {header}")
    ordinary = [tiles[part:192:6] for part in range(6)]
    jaws = [tiles[192 + level::8] for level in range(8)] if len(tiles) > 192 else []
    with Image.open(source) as image:
        atlas = image.convert("RGBA")
    crisp = Image.new("RGBA", atlas.size, (0, 0, 0, 0))
    flow = np.full(((6 + len(jaws))*GRID, 32*GRID, 3), 128, dtype=np.uint8)
    cv2.setNumThreads(2)
    for layer, row in enumerate(ordinary + jaws):
        slices = [atlas.crop((x, y, x+w, y+h)) for x,y,w,h,_,_ in row]
        guides = [(piece, *swatch(piece)) for piece in slices]
        for i, ((x,y,*_), piece) in enumerate(zip(row, slices, strict=True)):
            # Recover the texture of the hand-painted key inside the softer
            # intermediates. The original alpha remains the source of the edge.
            if layer < 6 and i % 4:
                piece = Image.blend(piece, restore(guides, i), .78)
            crisp.paste(contours(piece, sharpen), (x,y))
            block = registered_flow(guides[i][2], guides[(i+1)%32][2])
            flow[layer*GRID:(layer+1)*GRID, i*GRID:(i+1)*GRID, :2] = block
    output.mkdir(parents=True, exist_ok=True)
    crisp.save(output/"turn-ink.png", optimize=True)
    Image.fromarray(flow).save(output/"turn-flow.png", optimize=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("header", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--sharpen", type=float, default=0.0)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    if not 0.0 <= args.sharpen <= 1.0:
        parser.error("--sharpen must be between 0 and 1")
    pack(args.source, args.header, args.output, args.sharpen)

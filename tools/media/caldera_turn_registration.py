# /// script
# requires-python = ">=3.10"
# dependencies = ["numpy", "scipy", "pillow", "opencv-python-headless==5.1.0.95"]
# ///
"""Register the paint of adjacent turn views in the same local coordinates as the GPU rig.

Run after packing the 32 edited Krita views. Canonical paintings and all alpha masks
stay exactly as exported. The extra RG map gives the motion at the halfway angle.
"""

from pathlib import Path
import argparse
import re
import cv2
import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt


GRID = 32
WORK = 128
xy = np.stack(np.meshgrid(np.arange(WORK), np.arange(WORK)), axis=-1).astype(np.float32)


def remap(image, where):
    return cv2.remap(
        image,
        where.astype(np.float32),
        None,
        cv2.INTER_LINEAR,
        borderMode=cv2.BORDER_REPLICATE,
    )


def swatch(image):
    rgba = np.asarray(image)
    # Extend pigment into the transparent area before warping. Dark matte would
    # otherwise contaminate the visor and both thin edges of the sword.
    indices = distance_transform_edt(
        rgba[..., 3] < 56, return_distances=False, return_indices=True
    )
    rgb = cv2.resize(
        rgba[tuple(indices)][..., :3], (WORK, WORK), interpolation=cv2.INTER_CUBIC
    ).astype(np.float32)
    a = cv2.resize(rgba[..., 3], (WORK, WORK), interpolation=cv2.INTER_LINEAR) / 255.0
    luma = cv2.cvtColor(rgb, cv2.COLOR_RGB2GRAY)
    guide = ((68 + 1.28 * luma) * a).clip(0, 255).astype(np.uint8)
    return rgb, guide


def optical(a, b):
    flow = cv2.DISOpticalFlow_create(cv2.DISOPTICAL_FLOW_PRESET_FAST)
    flow.setFinestScale(0)
    flow.setPatchSize(10)
    flow.setPatchStride(4)
    flow.setVariationalRefinementIterations(4)
    return flow.calc(a, b, None)


def restore(tiles, index):
    # Recover pen and brush detail from the neighbouring original directions.
    # FILM and the hand-painted visor still determine local colour and shape.
    base, sub = divmod(index, 4)
    piece, target, guide = tiles[index]
    left = tiles[4 * base]
    right = tiles[(4 * base + 4) % 32]
    fraction = sub / 4
    registered = []
    for _, original, key_guide in (left, right):
        inverse = optical(guide, key_guide)
        registered.append(remap(original, xy + inverse))
    # A changing highlight should advance as a single mark. Keep the crisp
    # nearer facet for the higher spatial frequencies, blend the broader tone.
    weight = np.clip((fraction - 0.36) / 0.28, 0.0, 1.0)
    source = (1 - weight) * registered[0] + weight * registered[1]
    delta = source - cv2.GaussianBlur(source, (0, 0), 2.4)
    original_detail = target - cv2.GaussianBlur(target, (0, 0), 2.4)
    a = np.asarray(piece)[..., 3]
    canvas = np.asarray(piece).copy()
    # Work at the painted resolution to preserve existing high-contrast eyes
    # and outlines. Recover only missing local contrast from the aligned key.
    strength = np.clip(
        1.04 - np.abs(original_detail).mean(axis=-1, keepdims=True) / 38.0, 0.28, 0.95
    )
    detail = (delta * 1.25 - original_detail * 0.55) * strength
    detail = cv2.resize(detail, piece.size, interpolation=cv2.INTER_AREA)
    canvas[..., :3] = (
        np.clip(canvas[..., :3].astype(float) + detail, 0, 255).round().astype(np.uint8)
    )
    canvas[a == 0, :3] = 0
    return Image.fromarray(canvas)


def register(source, header, output):
    data = re.findall(
        r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}", Path(header).read_text()
    )
    if len(data) != 32 * 6:
        raise ValueError(f"Unexpected number of tiles: {len(data)}")
    cv2.setNumThreads(2)
    atlas = Image.open(source).convert("RGBA")
    motion = np.full((6 * GRID, 32 * GRID, 3), 128, dtype=np.uint8)
    for part in range(6):
        row = []
        for view in range(32):
            x, y, w, h, *_ = map(int, data[view * 6 + part])
            image = atlas.crop((x, y, x + w, y + h))
            row.append((image, *swatch(image)))
        for view, (img, rgb, guide) in enumerate(row):
            if view % 4:
                x, y, *_ = map(int, data[view * 6 + part])
                atlas.paste(restore(row, view), (x, y))
            next_guide = row[(view + 1) % 32][2]
            ab = optical(guide, next_guide)
            ba = optical(next_guide, guide)
            # Invert the two Lagrangian half-steps at the common midpoint.
            # Domain, range and units are identical to the interpolated tile UV.
            f, b = ab, ba
            for _ in range(3):
                f = remap(ab, xy - 0.5 * f)
                b = remap(ba, xy - 0.5 * b)
            flow = cv2.resize(0.5 * (f - b), (GRID, GRID), interpolation=cv2.INTER_AREA)
            enc = np.rint(128 + flow * (384.0 / WORK)).clip(0, 255).astype(np.uint8)
            motion[
                part * GRID : (part + 1) * GRID, view * GRID : (view + 1) * GRID, :2
            ] = enc
    output = Path(output)
    output.mkdir(exist_ok=True, parents=True)
    atlas.save(output / "parts-0.png", optimize=True)
    Image.fromarray(motion).save(output / "turn-flow.png", optimize=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("header", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    register(args.source, args.header, args.output)

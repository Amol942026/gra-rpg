"""Register the six painted pieces of the wraith across its full turn."""
from pathlib import Path
import re
import cv2
import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt
from caldera_turn_registration import GRID, WORK, optical, remap, swatch, xy


def register(folder, header):
    cv2.setNumThreads(2)
    source = Image.open(folder / "parts-0.png").convert("RGBA")
    source.load()
    paint = source.copy()
    edges = Image.new("L", source.size, 0)
    entries = re.findall(r"\{0,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\}", header.read_text())
    if len(entries) != 32 * 6:
        raise ValueError("Expected 32 painted views of six robe pieces")
    motion = np.full((6 * GRID, 32 * GRID, 2), 128, np.uint8)
    for part in range(6):
        guides = []
        for values in entries[part::6]:
            x, y, w, h, _, _ = map(int, values)
            tile = source.crop((x, y, x + w, y + h))
            guides.append(swatch(tile)[1])
            # Two transparent guard texels make filtering at a cuff independent
            # of the colour and distance of its neighbour on the packed page.
            rgba = np.pad(np.asarray(tile), ((2, 2), (2, 2), (0, 0)))
            alpha = rgba[..., 3]
            near = distance_transform_edt(alpha < 24, return_distances=False, return_indices=True)
            rgba[..., :3] = np.where(alpha[..., None] < 24, rgba[tuple(near)][..., :3], rgba[..., :3])
            paint.paste(Image.fromarray(rgba), (x - 2, y - 2))
            inside = alpha > 127
            sd = np.where(inside, distance_transform_edt(inside) - .5,
                          .5 - distance_transform_edt(~inside))
            antialias = (alpha > 0) & (alpha < 255)
            sd[antialias] = alpha[antialias] / 255. - .5
            edges.paste(Image.fromarray(np.uint8(np.clip(np.rint(128 + 8 * sd), 0, 255))), (x - 2, y - 2))
        for k, a in enumerate(guides):
            b = guides[(k + 1) % len(guides)]
            ab, ba = optical(a, b), optical(b, a)
            f, r = ab, ba
            for _ in range(4):
                f, r = remap(ab, xy - .5 * f), remap(ba, xy - .5 * r)
            flow = cv2.resize(.5 * (f - r), (GRID, GRID), interpolation=cv2.INTER_AREA)
            motion[part*GRID:(part+1)*GRID, k*GRID:(k+1)*GRID] = np.uint8(
                np.clip(np.rint(128 + flow * (384. / WORK)), 0, 255))
    rgb = np.zeros((*motion.shape[:2], 3), np.uint8)
    rgb[..., :2] = motion
    Image.fromarray(rgb).save(folder / "registration.png", optimize=True)
    edges.save(folder / "edges.png", optimize=True)
    paint.save(folder / "parts-0.png", optimize=True)


if __name__ == "__main__":
    import argparse
    args = argparse.ArgumentParser(description=__doc__)
    args.add_argument("folder", type=Path)
    args.add_argument("header", type=Path)
    args.add_argument("--no-audio", required=True, action="store_true")
    cfg = args.parse_args()
    register(cfg.folder, cfg.header)

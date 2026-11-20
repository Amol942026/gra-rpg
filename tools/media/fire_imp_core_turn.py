"""Eksport wspólnych ujęć głowy i tułowia chochlika z malowanych warstw."""

import argparse
import math
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage


WIDTH, HEIGHT = 84, 116
COLS, ROWS = 24, 15
SCALE = 1.5
ANCHOR = (42, 101)
ROW, COL = np.indices((HEIGHT, WIDTH))
X, Y = (COL + .5 - ANCHOR[0]) / SCALE, (ROW + .5 - ANCHOR[1]) / SCALE


def pixels(path):
    with Image.open(path) as source:
        rgba = np.asarray(source.convert("RGBA"), dtype=np.float32) / 255
    rgba[..., :3] *= rgba[..., 3:]
    return rgba


def sample(layer, x, y):
    coords = (y - .5, x - .5)
    return np.stack([ndimage.map_coordinates(layer[..., channel], coords, order=1,
                                           mode="constant", prefilter=False)
                     for channel in range(4)], axis=-1)


def paint(layer, u, v):
    return sample(layer, 1 + u * 158, 1 + v * 158)


def core(degree, back, front, head):
    turn = math.radians(degree)
    x, y = math.cos(turn), math.sin(turn) / .52
    length = math.hypot(x, y)
    x, y = x / length, y / length
    yaw = math.atan2(x, y)
    width = 26.4 + 5.0 * abs(y)
    shift = x * 1.25 * np.exp(-((Y + 17) / 10) ** 2)
    lateral = (X - shift) / width
    body_v = .5 + (Y + 14.4) / 39.8
    torso = paint(back, .5 + lateral, body_v)
    chest_u = .5 + (np.arcsin(np.clip(lateral * 1.97, -1, 1)) - yaw) / 2.38
    chest = paint(front, chest_u, body_v)
    skin = torso[..., 3:]
    weight = chest[..., 3:] * np.clip((skin - .22) * 1.8, 0, 1)
    chest_rgb = chest[..., :3] / np.maximum(chest[..., 3:], 1.e-5)
    torso[..., :3] = torso[..., :3] * (1 - weight) * .96 + chest_rgb * skin * weight

    skull_x, skull_y = 2.0 * x, 1.04 * y - 37
    skull = sample(head, 44 + (X - skull_x) * 2, 58 + (Y - skull_y) * 2)
    combined = skull + torso * (1 - skull[..., 3:])
    alpha = combined[..., 3]
    labels, _ = ndimage.label(alpha >= .39, np.ones((3, 3)))
    islands = np.bincount(labels.ravel())[1:]
    if np.count_nonzero(islands >= 3) != 1:
        raise ValueError(f"{degree}°: głowa i tułów mają rozdzielone piksele {islands.tolist()}")
    if np.max(alpha[[0, -1], :]) > .005 or np.max(alpha[:, [0, -1]]) > .005:
        raise ValueError(f"{degree}°: ujęcie przylega do brzegu atlasu")
    combined[..., :3] /= np.maximum(alpha[..., None], 1.e-5)
    return Image.fromarray(np.rint(np.clip(combined, 0, 1) * 255).astype(np.uint8))


def bake(body_path, head_path, out, review):
    body, heads = pixels(body_path), pixels(head_path)
    assert body.shape == (640, 480, 4) and heads.shape == (18 * 88, 20 * 88, 4)
    back, front = body[480:640, 160:320], body[160:320, :160]
    atlas = Image.new("RGBA", (COLS * WIDTH, ROWS * HEIGHT))
    sheet = Image.new("RGBA", (8 * WIDTH, HEIGHT), (57, 54, 51, 255))
    for degree in range(COLS * ROWS):
        x, y = (degree % 20) * 88, (degree // 20) * 88
        tile = core(degree, back, front, heads[y:y + 88, x:x + 88])
        atlas.paste(tile, (degree % COLS * WIDTH, degree // COLS * HEIGHT))
        if degree % 45 == 0:
            sheet.alpha_composite(tile, (degree // 45 * WIDTH, 0))
    atlas.save(out)
    if review:
        sheet.save(review)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("body", type=Path)
    parser.add_argument("head", type=Path)
    parser.add_argument("out", type=Path)
    parser.add_argument("--review", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    bake(args.body, args.head, args.out, args.review)

"""Składa pełne, dwuwymiarowe ujęcia głowy chochlika z warstw Krity."""

import argparse
import math
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage


CELL = 88
COLS, ROWS = 20, 18
ANCHOR_X, ANCHOR_Y = 44, 58
ROWS_2D, COLS_2D = np.indices((CELL, CELL))
X = (COLS_2D + .5 - ANCHOR_X) * .5
Y = (ROWS_2D + .5 - ANCHOR_Y) * .5


def sample(part, u, v):
    coords = (v * 158 + .5, u * 158 + .5)
    return np.stack([ndimage.map_coordinates(part[:, :, n], coords, order=1,
                                           mode="constant", prefilter=False)
                     for n in range(part.shape[2])], axis=-1)


def over(front, back):
    return front + back * (1 - front[:, :, 3:])


def sprite(part, x, y, width, height, angle, pivot, mirror=False):
    dx, dy = X - x, Y - y
    c, s = math.cos(angle), math.sin(angle)
    u = (dx * c + dy * s) / ((-1 if mirror else 1) * width) + pivot[0]
    v = (-dx * s + dy * c) / height + pivot[1]
    return sample(part, u, v)


def face_layers(face, feature_matte):
    """Split the facial painting into texture, lighting and feature coverage."""
    alpha = face[..., 3]
    smooth_alpha = ndimage.gaussian_filter(alpha, 7)
    smooth_color = np.stack(
        [ndimage.gaussian_filter(face[..., channel], 7) for channel in range(3)],
        axis=-1,
    ) / np.maximum(smooth_alpha[..., None], 1.e-5)
    # The old cutout included its own forehead, cheeks and chin with a hard
    # outer edge. Only the inner features carry their own broad shading.
    distance = ndimage.distance_transform_edt(alpha > .65)
    inside = np.clip((distance - 3) / 16, 0, 1)
    inside = inside**2 * (3 - 2 * inside)
    features = ndimage.gaussian_filter(feature_matte, 7)
    return np.dstack((smooth_color * alpha[..., None], inside * alpha, features * alpha))


def head(degrees, skull, horn, face, lighting):
    angle = math.radians(degrees)
    x, y = math.cos(angle), math.sin(angle) / .52
    length = math.hypot(x, y)
    x, y = x / length, y / length
    yaw = math.atan2(x, y)

    # Shape the front of the skull by moving the rows around the cheek and
    # jaw in 2D. The facial painting shares that exact contour.
    snout = x * 2.9 * np.exp(-((Y - 3.2) / 4.4) ** 2)
    across = X - snout
    skin = sample(skull, .5 + across / 24.6, .5 + (Y - .1) / 24.2)
    arc = np.arcsin(np.clip(across / 12.5, -1, 1)) - yaw
    u, v = .5 + arc / 2.38, .5 + (Y - .4) / 20.4
    painted = sample(face, u, v)
    tones = sample(lighting, u, v)
    face_alpha = np.maximum(painted[..., 3:], 1.e-5)
    skull_alpha = skin[..., 3:]
    scalp = skin[..., :3] / np.maximum(skull_alpha, 1.e-5)
    broad = tones[..., :3] / face_alpha
    texture = (painted[..., :3] - tones[..., :3]) / face_alpha
    feature_weight = np.clip(tones[..., 4:] / face_alpha, 0, 1)
    # Eyes, nose and mouth retain their painted modelling. The surrounding
    # scales inherit the colour of the skull, with a little facial texture.
    pigment = scalp * (1 - feature_weight) + broad * feature_weight + texture
    weight = tones[..., 3:4] * (.32 + .68 * feature_weight)
    weight *= np.clip((skull_alpha - .3) * 1.65, 0, 1)
    skin[..., :3] = skin[..., :3] * (1 - weight) + np.clip(pigment, 0, 1) * skull_alpha * weight

    horns = np.zeros_like(skin)
    for side in (-1, 1):
        attachment_x = -side * 6.2 * y - .4 * x
        attachment_y = -5.45 + 2.65 * side * x
        turn = -.24 * x + .23 * side * y
        layer = sprite(horn, attachment_x, attachment_y, 12.1, 21.1,
                       turn, (.76, .83), mirror=side == -1)
        layer[:, :, :3] *= .93
        horns = over(layer, horns)
    return over(skin, horns)


def tile(image):
    alpha = image[:, :, 3]
    labels, count = ndimage.label(alpha >= .36, np.ones((3, 3)))
    islands = np.bincount(labels.ravel())[1:]
    if not count or np.count_nonzero(islands >= 2) != 1:
        raise ValueError(f"Odstający fragment głowy: {islands.tolist()}")
    if np.max(alpha[[0, -1], :]) > .005 or np.max(alpha[:, [0, -1]]) > .005:
        raise ValueError("Głowa wychodzi poza komórkę atlasu.")
    result = image.copy()
    result[:, :, :3] /= np.maximum(alpha[:, :, None], 1.e-5)
    return Image.fromarray(np.rint(np.clip(result, 0, 1) * 255).astype(np.uint8))


def bake(path, destination, review, feature_matte):
    with Image.open(path) as source:
        paint = np.asarray(source.convert("RGBA"), dtype=np.float32) / 255
    assert paint.shape == (640, 480, 4), path
    paint[:, :, :3] *= paint[:, :, 3:]
    skull, horn, face = (paint[:160, start:start + 160] for start in (0, 160, 320))
    with Image.open(feature_matte) as source:
        if source.size != (160, 160) or "A" not in source.getbands():
            raise ValueError("Maska rysów musi być plikiem RGBA 160 × 160 z Krity.")
        matte = np.asarray(source.getchannel("A"), dtype=np.float32) / 255
    lighting = face_layers(face, matte)
    atlas = Image.new("RGBA", (CELL * COLS, CELL * ROWS))
    sheet = Image.new("RGBA", (CELL * 8, CELL * 2), (65, 61, 58, 255))
    for degrees in range(COLS * ROWS):
        try:
            view = tile(head(degrees, skull, horn, face, lighting))
        except ValueError as error:
            raise ValueError(f"{degrees}°: {error}") from error
        atlas.paste(view, (degrees % COLS * CELL, degrees // COLS * CELL))
        if degrees % 22 == 0 and degrees < 352:
            i = degrees // 22
            sheet.alpha_composite(view, (i % 8 * CELL, i // 8 * CELL))
    atlas.save(destination)
    if review:
        sheet.save(review)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--review", type=Path)
    parser.add_argument("--feature-matte", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    bake(args.source, args.destination, args.review, args.feature_matte)

"""Separate the four additional hand-registered fire imp paintings at rig and atlas resolutions."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage


SOURCE = Path(__file__).resolve().parents[2] / "art/volcano/fire-imp/paint/inbetweens"


def polygon(points, size):
    mask = Image.new("L", (size * 4, size * 4))
    ImageDraw.Draw(mask).polygon([(round(x * size / 40), round(y * size / 40)) for x, y in points], fill=255)
    return np.asarray(mask.resize((size, size), Image.Resampling.LANCZOS), dtype=float) / 255


def tile_for(source, projection, factor):
    # The measured heel line is shared by the unscaled combat rig and the HD cel.
    crop = np.array(source.crop(projection["source"]))
    labels, _ = ndimage.label(crop[:, :, 3] > 80)
    areas = np.bincount(labels.ravel())
    areas[0] = 0
    crop[:, :, 3] *= ndimage.binary_dilation(labels == areas.argmax(), iterations=5)
    left, top, right, bottom = [v * factor for v in projection["dest"]]
    tile = Image.new("RGBA", (160 * factor, 160 * factor))
    tile.paste(Image.fromarray(crop).resize((right-left, bottom-top), Image.Resampling.LANCZOS), (left, top))
    return tile


def bridge_tail(rgb, mask, hidden, curve, factor):
    # Carry a cross-section along the tail behind the near wrist. On emerging
    # next to the moving hand the barb and root still belong to a single tail.
    points = np.asarray(curve) * factor
    arc = np.r_[0, np.cumsum(np.linalg.norm(np.diff(points, axis=0), axis=1))]
    travel = np.arange(0, arc[-1], .45 * factor)
    path = np.stack([np.interp(travel, arc, coords) for coords in points.T], axis=-1)
    tangent = np.gradient(path, axis=0)
    tangent /= np.maximum(np.linalg.norm(tangent, axis=1, keepdims=True), .001)
    normals = np.stack((-tangent[:, 1], tangent[:, 0]), axis=-1)
    middle = np.rint(path).astype(int)
    clear = mask[middle[:, 1], middle[:, 0]] > .8
    visible = np.flatnonzero(clear)
    if not len(visible):
        return
    for j in np.flatnonzero(~clear):
        if travel[j] > arc[-1] * .77:
            continue
        source = visible[np.argmin(abs(visible - j))]
        radius = (2.4 - travel[j] / arc[-1]) * factor
        for offset in np.arange(-radius, radius + .25, .5):
            x, y = np.rint(path[j] + offset * normals[j]).astype(int)
            sx, sy = np.rint(path[source] + offset * normals[source]).astype(int)
            if hidden[y, x] and mask[sy, sx] > .5:
                rgb[y, x] = rgb[sy, sx]
                mask[y, x] = max(mask[y, x], min(1., radius-abs(offset)))


def split_view(source, markers, output, sector, factor):
    tile = tile_for(source, markers["projection"], factor)
    tile.save(output / f"mid-{sector}-{factor}x.png")
    size = tile.width
    rgba = np.asarray(tile)
    alpha = rgba[:, :, 3] / 255.
    head = polygon(markers["head"], size)
    torso = np.maximum(head, polygon(markers["chest"], size))
    owner = np.full(alpha.shape, 3, dtype=np.uint8)
    outlined = np.zeros(alpha.shape, dtype=bool)
    order = markers["order"]
    for part in order:
        if part != 3:
            cut = polygon(markers[str(part)], size) > .5
            owner[cut] = part
            outlined |= cut
    # Assign painted pixels at the very tips of claws and ears to the nearest
    # traced region. The torso cannot acquire loose fingers or tail fragments.
    traced = (alpha > .22) & (outlined | (torso > .45))
    nearest = ndimage.distance_transform_edt(~traced, return_distances=False, return_indices=True)
    owner = owner[tuple(nearest)]
    for depth, part in enumerate(order):
        rgb = rgba[:, :, :3].copy()
        coverage = (owner == part).astype(float)
        occluded = np.isin(owner, order[depth + 1:]) & (rgba[:, :, 3] == 255)
        if part == 3:
            interior = (owner == part) & (alpha > .45)
            distance, nearest = ndimage.distance_transform_edt(~interior, return_indices=True)
            overlap = np.clip(1 - (distance - .5*factor)/(2.1*factor), 0, 1) * torso
            rgb[occluded] = rgb[tuple(nearest[:, occluded])]
            coverage = np.maximum(coverage, overlap * occluded)
        if part == 0:
            bridge_tail(rgb, coverage, occluded, markers["tail"], factor)
        data = np.dstack((rgb, np.rint(alpha * coverage * 255).astype(np.uint8)))
        data[data[:, :, 3] == 0, :3] = 0
        Image.fromarray(data).save(output / f"{sector}-{part}-{factor}x.png")
    if factor == 2:
        Image.fromarray(np.rint(head * alpha * 255).astype(np.uint8)).save(output / f"{sector}-head-2x.png")


def export(output, source_folder=SOURCE):
    markers = json.loads((source_folder / "rig.json").read_text("utf-8"))
    output.mkdir(parents=True, exist_ok=True)
    with Image.open(source_folder / "sunburst.png") as painted:
        for sector, record in markers.items():
            for factor in (1, 2):
                split_view(painted, record, output, int(sector), factor)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    export(parser.parse_args().output)

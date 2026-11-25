"""Bake adjacent-direction motion from the already rendered Blender walking poses."""

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image


GRID_WIDTH, GRID_HEIGHT = 26, 30
CELL_WIDTH, CELL_HEIGHT = 104, 120
FACINGS, WALK = 48, 24


def sample_grid():
    y, x = np.indices((GRID_HEIGHT, GRID_WIDTH), dtype=np.float32)
    return np.column_stack(((x.ravel() + .5) * (CELL_WIDTH / GRID_WIDTH),
                            (y.ravel() + .5) * (CELL_HEIGHT / GRID_HEIGHT)))


def surface_grid(mesh, faces, points):
    """Register samples to the visible triangles, using Blender's layer depth."""
    projected = mesh[faces]
    xy = projected[:, :, :2]
    lo, hi = xy.min(axis=1), xy.max(axis=1)
    within = (points[None, :, 0] >= lo[:, None, 0]) & (points[None, :, 0] <= hi[:, None, 0])
    within &= (points[None, :, 1] >= lo[:, None, 1]) & (points[None, :, 1] <= hi[:, None, 1])
    tri, sample = np.nonzero(within)
    a = xy[tri, 0]
    u, v = xy[tri, 1] - a, xy[tri, 2] - a
    d = points[sample] - a
    det = u[:, 0] * v[:, 1] - v[:, 0] * u[:, 1]
    good = np.abs(det) > 1e-6
    b = (d[:, 0] * v[:, 1] - v[:, 0] * d[:, 1]) / np.where(good, det, 1)
    c = (u[:, 0] * d[:, 1] - d[:, 0] * u[:, 1]) / np.where(good, det, 1)
    good &= (b >= 0) & (c >= 0) & (b + c <= 1)
    sample, tri = sample[good], tri[good]
    weights = np.column_stack((1 - b[good] - c[good], b[good], c[good]))
    depth = (projected[tri, :, 2] * weights).sum(axis=1)
    order = np.lexsort((-depth, sample))
    covered, first = np.unique(sample[order], return_index=True)
    winners = order[first]
    if not len(covered):
        raise ValueError("Empty visible mesh")

    # Extend the nearest visible movement outside the silhouette. Samples in
    # that band can enter or leave the shape during interpolation.
    distances = ((points[:, None] - points[covered][None, :]) ** 2).sum(axis=2)
    nearest = distances.argmin(axis=1)
    return faces[tri[winners]], weights[winners], nearest


def flow(source, target, registration):
    vertices, weights, nearest = registration
    delta = target[vertices, :2] - source[vertices, :2]
    return (delta * weights[:, :, None]).sum(axis=1)[nearest]


def bake(geometry, output):
    data = np.load(geometry)
    poses, faces = data["poses"], data["faces"]
    if poses.shape[:2] != (FACINGS, WALK) or faces.shape[1:] != (3,):
        raise ValueError("Unexpected layout of the Blender walk poses")
    grid = sample_grid()
    fields = np.empty((WALK * GRID_HEIGHT, FACINGS * GRID_WIDTH, 4), dtype=np.uint8)
    maximum = 0.0
    for phase in range(WALK):
        walk = poses[:, phase]
        visible = [surface_grid(pose, faces, grid) for pose in walk]
        for facing in range(FACINGS):
            following = (facing + 1) % FACINGS
            forward = flow(walk[facing], walk[following], visible[facing])
            backward = flow(walk[following], walk[facing], visible[following])
            motion = np.column_stack((forward, backward))
            maximum = max(maximum, float(np.abs(motion).max()))
            packed = np.clip(np.rint(128 + motion * 8), 0, 255).astype(np.uint8)
            x, y = facing * GRID_WIDTH, phase * GRID_HEIGHT
            fields[y:y + GRID_HEIGHT, x:x + GRID_WIDTH] = packed.reshape(GRID_HEIGHT, GRID_WIDTH, 4)
    if maximum > 15.875:
        raise ValueError(f"Directional flow exceeds the encoded range: {maximum:.2f} px")
    Image.fromarray(fields).save(output)
    return {"pairs": FACINGS * WALK, "grid": [GRID_WIDTH, GRID_HEIGHT], "max_displacement_px": maximum}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("geometry", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    print(json.dumps(bake(args.geometry, args.output)))

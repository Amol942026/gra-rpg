"""Bake marsh turns by following paint between adjacent, authored 2D views."""
from argparse import ArgumentParser
import hashlib
import json
from pathlib import Path
import sys
import time

import numpy as np
from PIL import Image
from scipy import ndimage as ndi

from marsh_rebuild_paint import complete_paint, cycle, extra_keys, keys, register
from marsh_surface_paint import painted_bell, sample
from marsh_layered_turn import KeeperTurn, ToadTurn


def correspondence(paintings, comfy, weights):
    sys.path.insert(0, str(comfy))
    import torch
    from torch.nn import functional as F
    from safetensors.torch import load_file
    from comfy_extras.frame_interpolation_models.film_net import FILMNet, flow_pyramid_synthesis

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    dtype = torch.float16 if device.type == "cuda" else torch.float32
    model = FILMNet()
    model.load_state_dict(load_file(str(weights)))
    model.eval().to(device=device, dtype=dtype)
    if device.type == "cpu":
        torch.set_num_threads(4)

    count, height, width, _ = paintings.shape
    # Keep a clear matte around the contours and pad the feature pyramid.
    xpad = (-width - 24) % 64 + 24
    ypad = (-height - 24) % 64 + 24
    x0, y0 = xpad // 2, ypad // 2
    model._build_warp_grids(height + ypad, width + xpad, device)
    pixels = torch.as_tensor(paintings, device=device, dtype=dtype).movedim(-1, 1) / 255
    alpha = pixels[:, 3:4]
    matte = pixels[:, :3] * alpha + (1 - alpha) * .072
    matte = F.pad(matte, (x0, xpad-x0, y0, ypad-y0), value=.072)

    fields = []
    with torch.inference_mode():
        for start in range(0, count, 4):
            step = list(range(start, min(start + 4, count)))
            _, a = model.extract_features(matte[step])
            _, b = model.extract_features(matte[[(i + 1) % count for i in step]])
            forward = flow_pyramid_synthesis(model.predict_flow(a, b, model.warp))[0]
            backward = flow_pyramid_synthesis(model.predict_flow(b, a, model.warp))[0]
            both = torch.stack((forward, backward), dim=1)
            fields.append(both[..., y0:y0+height, x0:x0+width].float().cpu().numpy())
    fields = np.concatenate(fields)
    if not np.isfinite(fields).all():
        raise ValueError("Non-finite directional displacement")
    return fields


def prepare_pair(first, second, fields):
    outlines = tuple(register(image)[2].astype("f4") for image in (first, second))
    # FILM returns x, y. SciPy expects row, column.
    movement = ndi.gaussian_filter(fields[:, ::-1], (0, 0, .85, .85))
    return outlines, movement


def silhouette_at(pair, grid, t, target_area, target_center):
    outlines, movement = pair
    warped = []
    for edge, field, weight in zip(outlines, movement, (t, 1 - t)):
        location = grid - weight * field
        location = grid - weight * sample(field, location)
        warped.append(sample(edge[None], location)[0])
    distance = (1 - t) * warped[0] + t * warped[1]
    # Use a single contour with the expected projected volume.
    low, high = -8., 8.
    for _ in range(14):
        level = (low + high) / 2
        if np.clip(.5 + distance - level, 0., 1.).sum() > target_area:
            low = level
        else:
            high = level
    distance -= (low + high) / 2
    alpha = np.clip(.5 + distance, 0., 1.)
    center = np.sum(alpha * grid[1]) / alpha.sum()
    return ndi.shift(distance, (0, target_center - center),
                     order=1, mode="constant", cval=-32., prefilter=False)


def keeper_coverage(paintings, headings, fields, grid, area, centers):
    pairs = [prepare_pair(image, paintings[(i+1) % len(headings)], fields[i])
             for i, image in enumerate(paintings)]
    outlines = []
    for direction in range(512):
        yaw = direction * 360 / 512
        index = np.searchsorted(headings, yaw, side="right") - 1
        t = (yaw - headings[index]) / 15
        outlines.append(silhouette_at(pairs[index], grid, t, area(yaw), centers(yaw)))
    boundary = np.stack(outlines)
    depth = np.clip((.76 - grid[0] / grid.shape[1]) / .22, 0., 1.)
    depth = depth*depth*(3 - 2*depth)
    smooth = ndi.gaussian_filter1d(boundary, 5., axis=0, mode="wrap")
    return np.clip(.5 + boundary + depth*(smooth-boundary), 0., 1.)


def bake(original, additional, species, layout, comfy, weights):
    base = keys(original, species)
    grid = np.indices(base[0].shape[:2], dtype="f4")
    frame_path = layout / "frames" / str(species)
    frame_path.mkdir(parents=True)
    rig = ToadTurn(base, grid) if species == 0 else KeeperTurn(base, grid)
    strip = np.clip(np.rint(rig.surface[0].transpose(1, 2, 0)), 0., 255.).astype("u1")
    Image.fromarray(strip).save(layout / f"material-{species}.png")
    if species == 0:
        for direction in range(512):
            frame = rig.frame(direction * 360/512)
            frame[..., 3] = complete_paint(frame)
            Image.fromarray(frame).save(frame_path / f"{direction:03}.png", compress_level=2)
        return {"angles": 512, "painted_parts": "one trunk, four feet"}

    painted = dict(zip(range(0, 360, 45), base))
    painted.update(extra_keys(additional, base, species))
    headings = np.array(sorted(painted))
    paintings = np.stack([painted[heading] for heading in headings])
    alpha = np.array(base)[..., 3] / 255
    area = alpha.sum(axis=(1, 2))
    centers = (alpha * grid[1]).sum(axis=(1, 2)) / area
    fields = correspondence(paintings, comfy, weights)
    np.savez_compressed(layout / "film-3.npz", headings=headings, vectors=fields.astype("f2"))
    bell = painted_bell(base[2])[:, 38:80, 84:120].transpose(1, 2, 0)
    bell[..., 3] *= 255.
    Image.fromarray(np.clip(np.rint(bell), 0, 255).astype("u1")).save(layout / "keeper-bell.png")
    for direction, coverage in enumerate(keeper_coverage(
            paintings, headings, fields, grid, cycle(area), cycle(centers))):
        frame = rig.frame(direction * 360/512, coverage)
        frame[..., 3] = complete_paint(frame)
        Image.fromarray(frame).save(frame_path / f"{direction:03}.png", compress_level=2)
    return {"angles": 512, "painted_parts": "one cloak, two sleeves, one bell"}


if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--toad", type=Path, required=True)
    parser.add_argument("--toad-extra", type=Path, help="Accepted for older export commands")
    parser.add_argument("--keeper", type=Path, required=True)
    parser.add_argument("--keeper-extra", type=Path, required=True)
    parser.add_argument("--comfy", type=Path, required=True)
    parser.add_argument("--weights", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    start = time.perf_counter()
    results = {}
    for species, original, additional in ((0, args.toad, args.toad_extra), (3, args.keeper, args.keeper_extra)):
        results[species] = bake(original, additional, species, args.output, args.comfy, args.weights)
        print(f"Species {species}: 512 assembled poses", flush=True)
    report = {"source": "ComfyUI FILM", "species": results, "seconds": time.perf_counter() - start,
              "model_sha256": hashlib.sha256(args.weights.read_bytes()).hexdigest()}
    (args.output / "flow-receipt.json").write_text(json.dumps(report, indent=2), encoding="utf-8")

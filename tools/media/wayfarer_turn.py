"""Prepare painted angular keys and pack the local ComfyUI FILM head turn."""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageOps

from wanderer_paints import write_ora


CELL = 128
WORK_CELL = 256
MODEL = "film_net_fp16.safetensors"


def tile(sheet, index):
    return sheet.crop((index * CELL, 0, (index + 1) * CELL, CELL))


def paint(folder):
    source = Image.open(folder / "sunburst-inbetweens.png").convert("RGBA")
    layers = []
    for i in range(4):
        head = source.crop((i * source.width // 4, 0, (i + 1) * source.width // 4, source.height))
        alpha = head.getchannel("A").point(lambda a: a if a >= 10 else 0)
        head.putalpha(alpha)
        head = head.crop(alpha.getbbox())
        head.thumbnail((124, 120), Image.Resampling.LANCZOS)
        layer = Image.new("RGBA", (4 * CELL, CELL))
        layer.paste(head, (i * CELL + (CELL - head.width) // 2, (CELL - head.height) // 2))
        layers.append((f"Obrót w prawo {22.5 + 45 * i:g}°", layer))
    write_ora(folder / "turn-keys.ora", layers, "Wędrowiec · cztery ujęcia pośrednie")


def keys(original, painted):
    base = Image.open(original).convert("RGBA")
    new = Image.open(painted).convert("RGBA")
    half = [tile(new if i % 2 else base, i // 2) for i in range(9)]
    return half + [ImageOps.mirror(v) for v in reversed(half[1:-1])]


def prepare(folder, views):
    frames = []
    for head in views + views[:1]:
        head = head.resize((WORK_CELL, WORK_CELL), Image.Resampling.LANCZOS)
        # Two synchronized exposures allow reconstruction of anti-aliased RGBA
        # without carrying a green or blue matte into hair highlights.
        frame = Image.new("RGB", (2 * WORK_CELL, WORK_CELL))
        frame.paste("white", (WORK_CELL, 0, 2 * WORK_CELL, WORK_CELL))
        for x in (0, WORK_CELL):
            frame.paste(head, (x, 0), head.getchannel("A"))
        frames.append(frame)
    filename = "wayfarer-turn-dual-matte.png"
    frames[0].save(folder / filename, save_all=True, append_images=frames[1:], duration=100, loop=0)
    graph = {
        "1": {"class_type": "LoadImage", "inputs": {"image": filename}},
        "2": {"class_type": "FrameInterpolationModelLoader", "inputs": {"model_name": MODEL}},
        "3": {"class_type": "FrameInterpolate", "inputs": {"images": ["1", 0], "interp_model": ["2", 0], "multiplier": 4}},
        "4": {"class_type": "SaveImage", "inputs": {"images": ["3", 0], "filename_prefix": "wayfarer-turn-20261118/turn"}}
    }
    (folder / "film-workflow.json").write_text(json.dumps(graph, indent=2), encoding="utf-8")


def reconstruct(path):
    rgb = np.asarray(Image.open(path).convert("RGB"), dtype=np.float32) / 255
    black, white = np.split(rgb, 2, axis=1)
    alpha = np.clip(1 - np.median(white - black, axis=2), 0, 1)
    # A two-exposure estimate is noisy where the optical flows disagree on a
    # hair tip. Divide the black exposure only where there is visible coverage.
    alpha[alpha < .025] = 0
    color = np.clip(black / np.maximum(alpha[..., None], .08), 0, 1)
    data = np.dstack((color, alpha))
    head = Image.fromarray(np.rint(data * 255).astype(np.uint8))
    return head.resize((CELL, CELL), Image.Resampling.LANCZOS)


def pack(folder, views, rendered):
    paths = sorted(rendered.glob("turn_*.png"))
    if len(paths) != 65:
        raise ValueError(f"Expected 65 frames including the closing key, found {len(paths)}")
    atlas = Image.new("RGBA", (8 * CELL, 8 * CELL))
    for i in range(64):
        head = views[i // 4] if i % 4 == 0 else reconstruct(paths[i])
        atlas.paste(head, (CELL * (i % 8), CELL * (i // 8)))
    atlas.save(folder / "hero-head-turn.png", optimize=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("paint", "prepare", "pack"))
    parser.add_argument("folder", type=Path)
    parser.add_argument("--original", type=Path, default=Path("assets/sprites/hero-heads.png"))
    parser.add_argument("--rendered", type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    if args.action == "paint":
        paint(args.folder)
    else:
        keyframes = keys(args.original, args.folder / "turn-keys.png")
        if args.action == "prepare":
            prepare(args.folder, keyframes)
        else:
            pack(args.folder, keyframes, args.rendered)

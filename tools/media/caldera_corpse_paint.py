"""Author corpse materials on the existing directional Caldera cutouts."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import random
import xml.etree.ElementTree as ET
import zipfile

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


NAMES = (
    "slag-crawler", "lava-salamander", "fire-imp", "ash-wraith",
    "obsidian-sentinel", "ember-drake", "varkhor",
)
# Cool pigments and broad tonal shifts for each cooled surface. A death
# repaint is anchored to the painted cutout, including the palette of each limb.
TONES = (
    ((37, 51, 55), (169, 172, 153), (228, 179, 110), .18, .42),
    ((31, 55, 52), (169, 169, 121), (216, 140, 77), .14, .35),
    ((71, 38, 33), (190, 124, 84), (237, 150, 84), .15, .46),
    ((29, 38, 58), (144, 158, 173), (172, 179, 161), .18, .05),
    ((25, 40, 48), (161, 171, 170), (201, 161, 101), .22, .15),
    ((66, 44, 34), (198, 143, 99), (222, 153, 76), .12, .31),
    ((34, 37, 42), (172, 163, 140), (242, 118, 64), .22, .37),
)


def png(image):
    stream = io.BytesIO()
    image.save(stream, format="PNG")
    return stream.getvalue()


def height_field(mask):
    solid = mask.point(lambda a: 255 if a >= 110 else 0)
    distances = np.zeros((mask.height, mask.width), dtype=np.float32)
    for step in range(1, 19):
        solid = solid.filter(ImageFilter.MinFilter(3))
        core = np.asarray(solid, dtype=np.float32) / 255
        if not core.any():
            break
        distances += core
    radius = max(5, float(distances.max()))
    relief = Image.fromarray(np.uint8(np.sqrt(distances / radius) * 255), "L")
    return relief.filter(ImageFilter.GaussianBlur(.7))


def engrave(draw, bounds, species, part, seed):
    left, top, right, bottom = bounds
    width, height = right-left, bottom-top
    scale = max(width, height) / 112
    rng = random.Random(seed)
    dark, light, accent, _, _ = TONES[species]

    def line(points, color, strength, weight=1):
        vertices = [(left+x*width, top+y*height) for x, y in points]
        draw.line(vertices, fill=(*color, strength), width=max(1, round(scale*weight)), joint="curve")

    def seam(points, strength, weight=1):
        line([(x, y-.012) for x, y in points], light, strength, weight)
        line(points, dark, min(210, strength+47), weight*1.25)

    if species in (0, 6) and part in (0, 3, 4, 5):
        # Branching fractures and small chipped planes, with pale exposed stone.
        for index in range(4 if part == 3 else 2):
            x = (.25 if species == 6 else .18) + index * .17 + rng.uniform(-.04, .055)
            y = rng.uniform(.24, .61)
            side = -1 if index % 2 else 1
            points = [(x, y), (x+.09*side, y+.084), (x+.034*side, y+.15),
                      (x+.12*side, y+.25)]
            seam(points, 135, 1.65 if part == 3 else 1.05)
            seam([points[1], (x+.17*side, y+.1), (x+.2*side, y+.06)], 96)
            if species == 6 or index % 2 == 0:
                line(points[1:3], accent, 119, 1)
    elif species == 3 and part in (0, 3, 4, 5):
        # Long threads follow the cloth rather than crossing the empty hood.
        for index in range(6 if part in (0, 3) else 3):
            x = .2 + index * (.115 if part in (0, 3) else .23)
            y = rng.uniform(.42, .58)
            bend = rng.uniform(-.09, .1)
            points = [(x, y), (x+bend*.35, y+.12), (x+bend, y+.27), (x+bend*.4, .88)]
            seam(points, 92, 1.05)
        # A few short lit fibers at the frayed hem.
        for index in range(5):
            x = rng.uniform(.2, .83)
            y = rng.uniform(.72, .88)
            line([(x, y), (x+.016, y+.055)], accent, 93)
    elif species == 4:
        if part in (1, 2, 3, 4):
            # Chamfered plates, battered reinforcement and gold stud heads.
            for x, y in ((.34, .52), (.57, .62), (.41, .77)):
                points = [(x-.09, y-.08), (x-.04, y-.045), (x-.05, y+.045), (x+.075, y+.11)]
                seam(points, 162, 1.25)
                line([(x+.043, y+.014), (x+.064, y+.016)], accent, 175, 1.8)
        else:
            for index in range(4):
                x, y = rng.uniform(.25, .73), rng.uniform(.32, .76)
                line([(x-.035, y-.045), (x+.045, y+.025)], light, 134)
    elif species == 5 and part == 0:
        # Creases spread along both wings toward their respective tips.
        for side in (-1, 1):
            for index in range(3):
                x = .47 + .06*(side>0) + side*index*.029
                seam([(x, .82), (.5+side*.22, .59-index*.054),
                      (.5+side*(.32+.033*index), .23+index*.16)], 108, 1.4)
    elif species in (1, 2, 5) and part == 3:
        # Tiny directional scale rims and deep, interrupted marks between ribs.
        for row in range(4):
            y = (.46 if species == 2 else .42) + row*.106
            for col in range(3 if species == 2 else 7):
                x = .21 + (col+row%2*.39)*(.197 if species == 2 else .09)
                x += rng.uniform(-.02,.024)
                seam([(x-.03, y-.028), (x, y), (x+.028, y-.023)],
                     rng.randint(98, 142), 1.3)
    else:
        for index in range(5):
            x, y = rng.uniform(.32, .68), rng.uniform(.3, .76)
            line([(x-.04, y), (x+.054, y+.023)], light, 94, 1.4)


def cell_paint(cell, species, part, direction):
    mask = cell.getchannel("A")
    encoded_size = 160 if species == 6 else 128
    bounds = mask.getbbox()
    if not bounds:
        return cell, Image.new("RGBA", (encoded_size, encoded_size), (128, 128, 128, 0))
    relief = height_field(mask)
    depth = np.asarray(relief, dtype=np.float32) / 255
    original = np.asarray(cell, dtype=np.float32)[..., :3]
    softened = np.asarray(cell.convert("RGB").filter(ImageFilter.GaussianBlur(1.9)), dtype=np.float32)
    value = original @ np.array([.24, .61, .15], dtype=np.float32)
    dark, light, accent, shade, dim = TONES[species]
    y, x = np.mgrid[:cell.height, :cell.width]
    width = bounds[2]-bounds[0]
    height = bounds[3]-bounds[1]
    crosslight = np.clip(.76-(y-bounds[1])/height*.49 - (x-bounds[0])/width*.15, .05, 1)
    dense = np.clip((value - 35) / 132, 0, 1)
    pigment = np.asarray(dark) + (np.asarray(light)-dark) * dense[..., None]
    color = original + (pigment-original) * shade + (original-softened) * .28
    color *= (.86 + .22*crosslight + .13*depth)[..., None]
    coal = np.clip((original[..., 0]-original[..., 2]-38)/95, 0, 1)
    glow = coal * np.clip((value-69)/90, 0, 1) * dim
    color += (np.asarray(accent, dtype=np.float32)*.75 - color) * glow[..., None]
    paint = Image.fromarray(np.uint8(np.clip(color, 0, 255)), "RGB").convert("RGBA")
    detail = Image.new("RGBA", cell.size)
    engrave(ImageDraw.Draw(detail), bounds, species, part, (species+1)*739+part*41+direction*19)
    paint = Image.alpha_composite(paint, detail)
    paint.putalpha(mask)

    # Pack signed RGB paint adjustment and the convex height in one lookup.
    # The data texture never supplies visibility or changes the live sprite.
    delta = (np.asarray(paint, dtype=np.float32)[..., :3] - original)
    delta *= np.clip((np.asarray(mask, dtype=np.float32)-12)/130, 0, 1)[..., None]
    change = Image.fromarray(np.uint8(np.clip(128 + delta, 0, 255)), "RGB")
    change = change.resize((encoded_size, encoded_size), Image.Resampling.LANCZOS)
    change.putalpha(relief.resize(change.size, Image.Resampling.LANCZOS))
    return paint, change


def editable_master(path, source, paint):
    image = ET.Element("image", {"w": str(source.width), "h": str(source.height),
                                 "name": path.stem, "version": "0.0.3"})
    stack = ET.SubElement(image, "stack")
    ET.SubElement(stack, "layer", {"name": "Spękania, łuski i malowanie ciała",
                                  "src": "data/coat.png", "opacity": "1.0", "visibility": "visible"})
    ET.SubElement(stack, "layer", {"name": "Oryginalna wycinanka ComfyUI i Krita",
                                  "src": "data/source.png", "opacity": "1.0", "visibility": "hidden"})
    with zipfile.ZipFile(path, "x") as archive:
        archive.writestr("mimetype", "image/openraster")
        archive.writestr("stack.xml", ET.tostring(image, encoding="utf-8"))
        archive.writestr("data/coat.png", png(paint))
        archive.writestr("data/source.png", png(source))
        archive.writestr("mergedimage.png", png(paint))


def build(asset_root, authoring_root):
    authoring_root.mkdir(parents=True, exist_ok=False)
    out = authoring_root/"materials"
    out.mkdir()
    receipts = []
    for species, name in enumerate(NAMES):
        source_path = asset_root/(name+".png")
        source = Image.open(source_path).convert("RGBA")
        cell = source.width//8
        encoded_size = 160 if species == 6 else 128
        assert source.size == (8*cell, 6*cell)
        paint = Image.new("RGBA", source.size)
        texture = Image.new("RGBA", (8*encoded_size, 6*encoded_size))
        for part in range(6):
            for direction in range(8):
                box = (direction*cell, part*cell, (direction+1)*cell, (part+1)*cell)
                part_paint, part_material = cell_paint(source.crop(box), species, part, direction)
                paint.paste(part_paint, box[:2])
                texture.paste(part_material, (direction*encoded_size, part*encoded_size))
        assert paint.getchannel("A").tobytes() == source.getchannel("A").tobytes()
        editable_master(authoring_root/(name+".ora"), source, paint)
        material_path = out/(name+".png")
        texture.save(material_path)
        assert Image.open(material_path).tobytes() == texture.tobytes()
        sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        receipts.append({"species": name, "source_sha256": sha(source_path),
                         "packed_sha256": sha(material_path), "rgba8": texture.width*texture.height*4,
                         "cells": 48, "cell_size": encoded_size, "source_alpha_identical": True})
    (authoring_root/"materials-receipt.json").write_text(
        json.dumps(receipts, indent=2), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--assets", type=Path, default=Path("assets/volcano"))
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    build(args.assets, args.root)

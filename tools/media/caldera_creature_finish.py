"""Transfer approved texture swatches to the six articulated Caldera cutouts."""

import argparse
import hashlib
import io
import json
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET
import zipfile

import numpy as np
from PIL import Image, ImageFilter, ImageOps

from caldera_corpse_paint import NAMES, cell_paint

SPECIES = ("slag-crawler", "fire-imp", "ember-drake")
PARTS = ("Ogon i skrzydła", "Daleka tylna noga", "Bliska tylna noga",
         "Tułów, głowa i pancerz", "Daleka przednia kończyna", "Bliska przednia kończyna")
LUMA = np.array([.24, .60, .16], dtype=np.float32)


def png(image):
    target = io.BytesIO()
    image.save(target, format="PNG")
    return target.getvalue()


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def swatches(root, name):
    first = Image.open(root/(name+"-imagegen.png")).convert("RGB").resize((512, 512), Image.Resampling.LANCZOS)
    local = Image.open(root/(name+"-comfy.png")).convert("RGB")
    source = Image.blend(first, local, .34)
    return [source.crop((x+12, y+12, x+244, y+244)) for y in (0, 256) for x in (0, 256)]


def textures(source, shape, mirror):
    sampled = []
    for swatch in source:
        paint = swatch.resize(shape, Image.Resampling.LANCZOS)
        if mirror:
            paint = ImageOps.mirror(paint)
        color = np.asarray(paint, dtype=np.float32)
        broad = np.asarray(paint.filter(ImageFilter.GaussianBlur(4)), dtype=np.float32)
        sampled.append((color-broad)*1.08+(broad-color.mean((0, 1)))*.51)
    return sampled


def finish_cell(piece, swatch, name, part, direction):
    alpha_image = piece.getchannel("A")
    alpha = np.asarray(alpha_image, dtype=np.float32)/255
    bounds = alpha_image.point(lambda value: 255 if value > 80 else 0).getbbox()
    if not bounds:
        return piece, Image.new("RGBA", piece.size)
    left, top, right, bottom = bounds
    width, height = right-left, bottom-top
    original = np.array(piece, dtype=np.float32)[..., :3]
    pixels = original[top:bottom, left:right]
    trim = alpha[top:bottom, left:right]
    details = textures(swatch, (width, height), direction >= 4)
    heat = np.clip((pixels[..., 0]-pixels[..., 2]-24)/86, 0, 1)
    value = pixels @ LUMA
    y = np.linspace(0, 1, height, dtype=np.float32)[:, None]
    if name == "slag-crawler":
        material = details[0] if part == 3 else details[2] if part == 0 else details[3]
        burial = np.clip((y-.50)*2.8, 0, .66)*np.clip(1-value/170, 0, 1)
        material = material*(1-burial[..., None])+details[1]*burial[..., None]
        pigment = (.93, 1.00, 1.035)
    elif name == "fire-imp":
        horn = np.clip((.34-y)*5, 0, 1)*heat if part == 3 else 0*heat
        material = details[0]*(1-horn[..., None])+details[1]*horn[..., None]
        leather = np.clip((y-.58)*2, 0, .53)*(1-heat)
        material = material*(1-leather[..., None])+details[2]*leather[..., None]
        brass = heat*np.clip((y-.69)*3.4, 0, .62) if part == 3 else 0*heat
        material += details[3]*brass[..., None]
        pigment = (1.04, .98, .99)
    else:
        material = details[1] if part == 0 else details[0] if part == 3 else details[2]
        scutes = np.clip(heat*.62+(1-y)*.24, 0, .7)
        material = material*(1-scutes[..., None])+details[3]*scutes[..., None]
        pigment = (1.04, 1.00, 1.015)
    interior = np.asarray(alpha_image.filter(ImageFilter.MinFilter(3)), dtype=np.float32)[top:bottom, left:right]/255
    soft = np.asarray(piece.convert("RGB").filter(ImageFilter.GaussianBlur(1.05)),
                      dtype=np.float32)[top:bottom, left:right]
    emissive = heat*np.clip((value-135)/46, 0, 1)
    dent = (material @ LUMA)
    detailing = material*.35+dent[..., None]*.39
    strength = (.80 + .10*(part in (0, 3))) * interior*(1-emissive*.82)
    paint = pixels+((pixels-soft)*.63+detailing+(pixels*np.array(pigment)-pixels))*strength[..., None]
    original[top:bottom, left:right] = np.clip(paint, 0, 255)
    result = Image.fromarray(original.astype(np.uint8), "RGB").convert("RGBA")
    result.putalpha(alpha_image)

    glints = np.zeros((height, width, 4), dtype=np.uint8)
    glints[..., :3] = (149, 172, 179) if name == "slag-crawler" else (227, 168, 104)
    contrast = np.clip((dent-7)*3.2, 0, 112)*trim*interior
    glints[..., 3] = (contrast*(1-emissive*.88)).astype(np.uint8)
    marks = Image.new("RGBA", piece.size)
    marks.paste(Image.fromarray(glints), (left, top))
    return result, marks


def prepare(root, assets):
    backup = root/"backup"
    (backup/"corpses").mkdir(parents=True, exist_ok=False)
    staging = root/"staging"
    staging.mkdir()
    for name in SPECIES:
        source = assets/(name+".png")
        shutil.copy2(source, backup/source.name)
        shutil.copy2(assets/"corpses"/source.name, backup/"corpses"/source.name)
        original = Image.open(source).convert("RGBA")
        tile = original.width//8
        assert original.size == (8*tile, 6*tile)
        swatch = swatches(root, name)
        paint = Image.new("RGBA", original.size)
        highlights = Image.new("RGBA", original.size)
        for row in range(6):
            for col in range(8):
                rect = (col*tile, row*tile, (col+1)*tile, (row+1)*tile)
                surface, marks = finish_cell(original.crop(rect), swatch, name, row, col)
                paint.paste(surface, rect[:2])
                highlights.paste(marks, rect[:2])
        image = ET.Element("image", w=str(original.width), h=str(original.height), name=name, version="0.0.3")
        stack = ET.SubElement(image, "stack")
        with zipfile.ZipFile(staging/(name+".ora"), "x", zipfile.ZIP_DEFLATED) as archive:
            archive.writestr("mimetype", "image/openraster")
            def layer(label, key, pixels, y=0, visible=True):
                ET.SubElement(stack, "layer", name=label, src=key, opacity="1.0", x="0", y=str(y),
                              visibility="visible" if visible else "hidden", **{"composite-op":"svg:src-over"})
                archive.writestr(key, png(pixels))
            layer("Refleksy i wyszczerbienia", "data/highlights.png", highlights)
            for row, label in enumerate(PARTS):
                layer(label, f"data/part-{row}.png",
                      paint.crop((0,row*tile,paint.width,(row+1)*tile)), row*tile)
            layer("Oryginalna wycinanka", "data/original.png", original, visible=False)
            archive.writestr("stack.xml", ET.tostring(image, encoding="utf-8"))
            archive.writestr("mergedimage.png", png(Image.alpha_composite(paint,highlights)))


def pack(root):
    out = root/"runtime"
    (out/"corpses").mkdir(parents=True, exist_ok=False)
    records = []
    for name in SPECIES:
        master = root/(name+".kra")
        original = Image.open(root/"backup"/(name+".png")).convert("RGBA")
        with zipfile.ZipFile(master) as doc:
            paint = Image.open(io.BytesIO(doc.read("mergedimage.png"))).convert("RGBA")
        assert paint.size == original.size
        paint.putalpha(original.getchannel("A"))
        paint.save(out/(name+".png"))
        species = NAMES.index(name)
        tile = paint.width//8
        material = Image.new("RGBA", (1024,768))
        for row in range(6):
            for col in range(8):
                rect = (col*tile,row*tile,(col+1)*tile,(row+1)*tile)
                _, guide = cell_paint(paint.crop(rect), species, row, col)
                material.paste(guide,(col*128,row*128))
        corpse = out/"corpses"/(name+".png")
        material.save(corpse)
        live = out/(name+".png")
        assert paint.getchannel("A").tobytes() == original.getchannel("A").tobytes()
        records.append(dict(species=name, master_sha256=sha(master), live_sha256=sha(live),
                            corpse_sha256=sha(corpse), alpha_preserved=True,
                            imagegen_sha256=sha(root/(name+"-imagegen.png")),
                            comfy_sha256=sha(root/(name+"-comfy.png"))))
    (root/"art-receipt.json").write_text(json.dumps(records,indent=2),encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("phase", choices=("prepare", "pack"))
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--assets", type=Path, default=Path("assets/volcano"))
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    prepare(args.root,args.assets) if args.phase == "prepare" else pack(args.root)

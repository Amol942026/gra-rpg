"""Przygotowanie osobnych warstw Krity z dwóch ilustracji kierunkowych Sunburst."""
import argparse
from io import BytesIO
from pathlib import Path
from zipfile import ZipFile
import xml.etree.ElementTree as ET

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage


CELL = 160
LABELS = {
    "imp": ("Czaszka", "Róg", "Pysk", "Tułów i pas", "Ramię", "Przedramię i szpony",
            "Udo", "Goleń i racica", "Łuska barku", "Ogon", "Płyty grzbietowe", "Grot ogona"),
    "wraith": ("Kaptur", "Otwarcie kaptura", "Pelerynka", "Przednia poła", "Tren", "Boczna poła",
               "Górny rękaw", "Dolny rękaw", "Brosza", "Strzęp", "Złożona szata", "Wstęga"),
}


def cutout(cell, species, tile):
    pixels = np.array(cell)
    alpha = pixels[:, :, 3]
    labels, count = ndimage.label(alpha >= 116)
    assert count, (species, tile)
    areas = np.bincount(labels.ravel())
    largest = 1 + int(np.argmax(areas[1:]))
    # Remove matte debris while preserving isolated thread tips near the piece.
    solid = labels == largest
    near = ndimage.binary_dilation(solid, iterations=4)
    alpha[~near] = 0
    pixels[:, :, 3] = np.clip((alpha.astype(np.float32)-28)*255/227, 0, 255).astype(np.uint8)
    rgba = Image.fromarray(pixels)
    assert rgba.getbbox()
    rgba = rgba.crop(rgba.getbbox()).resize((CELL-4, CELL-4), Image.Resampling.LANCZOS)
    if species == "wraith" and tile == 1:
        # The gap between the embroidered edges is a hollow, dark opening.
        cavity = Image.new("RGBA", rgba.size)
        ImageDraw.Draw(cavity).polygon([(79, 10), (45, 37), (21, 79), (37, 113),
                                       (75, 144), (102, 119), (127, 79), (111, 48)], fill=(16, 17, 21, 255))
        cavity.alpha_composite(rgba)
        rgba = cavity
    result = Image.new("RGBA", (CELL, CELL))
    result.paste(rgba, (2, 2))
    return result


def image_bytes(image):
    data = BytesIO()
    image.save(data, format="PNG")
    return data.getvalue()


def prepare(source, output, species, imp_back):
    image = Image.open(source).convert("RGBA")
    assert image.getextrema()[3] == (0, 255), source
    size = (3*CELL, 4*CELL)
    root = ET.Element("image", w=str(size[0]), h=str(size[1]), name=species, version="0.0.3")
    stack = ET.SubElement(root, "stack")
    output.mkdir(parents=True, exist_ok=True)
    with ZipFile(output / f"{species}.ora", "x") as ora:
        ora.writestr("mimetype", "image/openraster")
        combined = Image.new("RGBA", size)
        for index, name in enumerate(LABELS[species]):
            x, y = index % 3, index // 3
            region = (round(x*image.width/3), round(y*image.height/4),
                      round((x+1)*image.width/3), round((y+1)*image.height/4))
            cell = Image.open(imp_back).convert("RGBA") if species=="imp" and index==10 else image.crop(region)
            sprite = cutout(cell, species, index)
            path = f"data/{index:02d}.png"
            ora.writestr(path, image_bytes(sprite))
            ET.SubElement(stack, "layer", name=name, src=path, x=str(x*CELL), y=str(y*CELL),
                          visibility="visible", opacity="1.0", **{"composite-op": "svg:src-over"})
            combined.paste(sprite, (x*CELL, y*CELL))
        ora.writestr("stack.xml", ET.tostring(root, encoding="utf-8"))
        ora.writestr("mergedimage.png", image_bytes(combined))
        combined.thumbnail((128, 128))
        ora.writestr("Thumbnails/thumbnail.png", image_bytes(combined))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--species", choices=LABELS, required=True)
    parser.add_argument("--imp-back", type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    if args.species=="imp" and not args.imp_back:
        parser.error("Ognisty chochlik wymaga osobno malowanych płyt grzbietowych.")
    prepare(args.source, args.output, args.species, args.imp_back)

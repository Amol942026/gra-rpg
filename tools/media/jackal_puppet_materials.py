"""Lay out swatches from the jackal's original painting for a layered Krita master."""

import argparse
import io
from pathlib import Path
import zipfile
from xml.etree import ElementTree as ET

import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt


SWATCHES = (
    ("Grzbiet i bok", (175, 218, 350, 297)),
    ("Fioletowa tkanina", (388, 181, 451, 230)),
    ("Pysk i kark", (432, 120, 477, 157)),
    ("Sierść nóg", (384, 321, 416, 401)),
    ("Sierść ogona", (53, 320, 113, 369)),
    ("Wnętrze ucha", (457, 50, 477, 106)),
    ("Łapy", (388, 453, 411, 482)),
    ("Pazury", (478, 141, 505, 157)),
)


def build(source, target):
    with Image.open(source) as original:
        profile = original.convert("RGBA").crop((0, 0, original.width // 2, original.height // 2))
    bounds = profile.getchannel("A").point(lambda alpha: 255 if alpha > 110 else 0).getbbox()
    if not bounds:
        raise ValueError("The source lacks an opaque profile.")
    pixels = np.asarray(profile.crop(bounds))
    # A few sample corners include transparency. Extend the closest painted
    # color into these corners so nothing from the background enters the cloth or fur.
    near = distance_transform_edt(pixels[..., 3] < 160, return_distances=False, return_indices=True)
    paint = Image.fromarray(pixels[tuple(near)][..., :3])
    sheet = Image.new("RGB", (1024, 512))
    root = ET.Element("image", w="1024", h="512", name="Szakal wydmowy", version="0.0.3")
    stack = ET.SubElement(root, "stack")
    layers = []
    for i, (name, region) in enumerate(SWATCHES):
        swatch = paint.crop(region).resize((256, 256), Image.Resampling.LANCZOS)
        x, y = i % 4 * 256, i // 4 * 256
        layers.append((swatch, name, x, y))
        sheet.paste(swatch, (x, y))
    white = Image.new("RGB", (8, 8), "white")
    layers.append((white, "Biel do detali", 0, 0))
    sheet.paste(white, (0, 0))
    with zipfile.ZipFile(target, "x", compression=zipfile.ZIP_DEFLATED) as package:
        package.writestr("mimetype", "image/openraster", compress_type=zipfile.ZIP_STORED)
        for i, (layer, label, x, y) in enumerate(reversed(layers)):
            path = f"data/{i}.png"
            ET.SubElement(stack, "layer", name=label, src=path, x=str(x), y=str(y),
                          visibility="visible", opacity="1.0", **{"composite-op": "svg:src-over"})
            data = io.BytesIO()
            layer.save(data, format="PNG")
            package.writestr(path, data.getvalue())
        merged = io.BytesIO()
        sheet.save(merged, format="PNG")
        package.writestr("mergedimage.png", merged.getvalue())
        package.writestr("stack.xml", ET.tostring(root))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    build(Path("art/desert/new-enemies/jackal-source.png"), args.output)


if __name__ == "__main__":
    main()

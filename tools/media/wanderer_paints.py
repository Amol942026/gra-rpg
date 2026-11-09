"""Prepare ComfyUI materials and finish Emberfall's 2D atlas in native Krita."""
import argparse
import hashlib
import io
import json
import math
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageOps

from krita_mcp_client import KritaMCP, start_krita_if_needed


CELL = 128
SIZE = (4 * CELL, 3 * CELL)


def png(image):
    stream = io.BytesIO()
    image.save(stream, format="PNG")
    return stream.getvalue()


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def origin(index):
    return index % 4 * CELL, index // 4 * CELL


def place(patch, index):
    image = Image.new("RGBA", SIZE)
    image.paste(patch, origin(index))
    return image


def material(source, box, dark, light):
    patch = source.crop(box).resize((CELL, CELL), Image.Resampling.LANCZOS)
    gray = ImageOps.autocontrast(patch.convert("L"), cutoff=3)
    return ImageOps.colorize(gray, dark, light).convert("RGBA")


def facial_features(flux):
    patch = Image.new("RGBA", (CELL, CELL))
    # Keep the sculpted skin under the paint and blend around individual features.
    for src, dst, opacity in [
        ((236, 68, 255, 81), (24, 36, 61, 63), 222),
        ((255, 68, 276, 81), (64, 36, 103, 63), 222),
        ((248, 77, 262, 93), (49, 53, 79, 83), 152),
        ((243, 91, 270, 102), (35, 79, 92, 103), 188),
    ]:
        x, y, right, bottom = dst
        width, height = right-x, bottom-y
        color = flux.crop(src).resize((width, height), Image.Resampling.LANCZOS).convert("RGBA")
        color = Image.blend(color, Image.new("RGBA", color.size, (202, 158, 121, 255)), .21)
        alpha = Image.new("L", (width, height))
        ImageDraw.Draw(alpha).ellipse((4, 4, width-5, height-5), fill=opacity)
        color.putalpha(alpha.filter(ImageFilter.GaussianBlur(3)))
        patch.alpha_composite(color, (x, y))
    return patch


def atlas_layers(flux, sdxl):
    front_hair = material(flux, (222, 31, 286, 72), "#302421", "#735543")
    back_hair = material(sdxl, (372, 306, 425, 351), "#322721", "#6c5041")
    return [
        ("01 · FLUX — oczy, nos i usta", place(facial_features(flux), 0)),
        ("02 · FLUX — ciemne pasma z przodu", place(front_hair, 1)),
        ("03 · SDXL — pasma z tyłu", place(back_hair, 2)),
        ("04 · FLUX — splot tkaniny kaftana", place(material(flux, (229, 206, 249, 243), "#a4a4a4", "#ffffff"), 3)),
        ("05 · FLUX — skóra karwaszy", place(material(flux, (171, 248, 192, 279), "#653d30", "#b8845d"), 4)),
        ("06 · FLUX — skóra i wytarcia butów", place(material(flux, (266, 392, 301, 459), "#a4a4a4", "#ffffff"), 5)),
        ("07 · FLUX — miedziane naramienniki", place(material(flux, (307, 135, 330, 161), "#704027", "#d28c53"), 6)),
        ("08 · FLUX — fałdy płaszcza", place(material(flux, (216, 106, 234, 144), "#989898", "#fafafa"), 7)),
        ("09 · FLUX — poły kaftana", place(material(flux, (206, 275, 225, 323), "#a6a6a6", "#ffffff"), 8)),
    ]


def write_ora(path, layers, title="Emberfall — wędrowiec / malowane materiały"):
    width, height = layers[0][1].size
    xml = ET.Element("image", w=str(width), h=str(height), version="0.0.3", name=title)
    stack = ET.SubElement(xml, "stack")
    merged = Image.new("RGBA", (width, height))
    with zipfile.ZipFile(path, "x", zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("mimetype", "image/openraster", compress_type=zipfile.ZIP_STORED)
        for index, (label, image) in enumerate(layers):
            source = f"data/{index}.png"
            ET.SubElement(stack, "layer", {"name": label, "src": source, "x": "0", "y": "0",
                                            "opacity": "1.0", "composite-op": "svg:src-over",
                                            "visibility": "visible"})
            archive.writestr(source, png(image))
            merged.alpha_composite(image)
        archive.writestr("mergedimage.png", png(merged))
        archive.writestr("stack.xml", ET.tostring(xml, encoding="utf-8", xml_declaration=True))


def finish_in_krita(root, source):
    master = root / "wanderer-paints.kra"
    atlas = root / "hero-paints.png"
    start_krita_if_needed()
    with KritaMCP(root / "krita-log") as client:
        doc = client.call("krita_document", action="open", path=str(source))["affected_document_id"]
        state = client.call("krita_state", document_id=doc)
        if state["runtime_compatibility"]["status"] != "compatible" or len(state["layers"]) != 9:
            raise RuntimeError("The editable nine-layer atlas did not open correctly in Krita")

        def layer(label):
            client.call("krita_layers", action="create", document_id=doc, name=label)
            return client.call("krita_state", document_id=doc)["active_layer_id"]

        def mark(layer_id, index, color, width, points, shape="path"):
            x, y = origin(index)
            client.call("krita_draw", document_id=doc, layer_id=layer_id, shape=shape,
                        points=[{"x": x+a, "y": y+b} for a, b in points], foreground=color,
                        brush_size=width, fill_style="ForegroundColor" if shape == "polygon" else "None",
                        stroke_style="None" if shape == "polygon" else "ForegroundColor")

        face = layer("10 · Krita — powieki i tęczówki")
        for x in (42, 84):
            mark(face, 0, "#735849", 1.7, [(x-7, 52), (x, 49), (x+7, 51)])
            mark(face, 0, "#cab69c", 1.8, [(x-5, 52), (x+5, 52)])
            mark(face, 0, "#54665b", 2.6, [(x, 50.8), (x, 53)])
        hair = layer("11 · Krita — gładkie, ciemnobrązowe pasma")
        for offset in range(0, 37, 9):
            mark(hair, 1, "#644937", 1.6,
                 [(18, 44+offset), (39, 29+offset), (65, 21+offset), (99, 22+offset)])
            mark(hair, 2, "#5b4335", 1.5,
                 [(24+offset, 18), (28+offset, 39), (32+offset, 69), (29+offset, 104)])

        seams = layer("12 · Krita — szwy, lamówki i sygnatura płaszcza")
        for index in (3, 7, 8):
            for x in (15, 112):
                mark(seams, index, "#d6d6d6", 1.5, [(x, 6), (x+2, 51), (x, 121)])
            mark(seams, index, "#f7f7f7", 2, [(8, 114), (38, 119), (89, 119), (120, 114)])
        for index, base in [(4, "#d4a071"), (5, "#d3d3d3"), (6, "#e1ad6f")]:
            mark(seams, index, base, 2, [(12, 14), (18, 74), (14, 112), (106, 112)])
            mark(seams, index, base, 2, [(108, 16), (105, 70), (109, 107)])
            for x in (24, 42, 60, 78, 96):
                mark(seams, index, base, 1.5, [(x-3, 104), (x+3, 106)])
        mark(seams, 7, "#f1f1f1", 2.5, [(64, 44), (77, 59), (64, 74), (51, 59), (64, 44)])
        for angle in range(0, 360, 45):
            dx, dy = math.cos(math.radians(angle)), math.sin(math.radians(angle))
            mark(seams, 7, "#e5e5e5", 1.8, [(64+17*dx, 59+17*dy), (64+23*dx, 59+23*dy)])

        saved = client.call("krita_save_export", document_id=doc, kra_path=str(master),
                            png_path=str(atlas), overwrite=False)
        client.call("krita_preview", document_id=doc, max_width=1024, max_height=768)
        state = client.call("krita_state", document_id=doc)
        client.call("krita_document", action="close", document_id=doc, discard_changes=False)
    with Image.open(atlas) as image, zipfile.ZipFile(master) as archive:
        image.load()
        merged = Image.open(archive.open("mergedimage.png")).convert("RGBA")
        if image.mode != "RGBA" or image.size != SIZE or image.tobytes() != merged.tobytes():
            raise ValueError("The native Krita master and PNG export do not match")
        if archive.testzip() is not None or len(state["layers"]) != 12:
            raise ValueError("Incomplete layered Krita master")
    return {"master": str(master), "atlas": str(atlas), "atlas_sha256": digest(atlas),
            "master_sha256": digest(master), "layers": state["layers"], "export": saved}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--flux", type=Path, required=True)
    parser.add_argument("--sdxl", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    ora = args.root / "materials-input.ora"
    with Image.open(args.flux) as flux, Image.open(args.sdxl) as sdxl:
        if flux.size != (512, 512) or sdxl.size != (512, 512):
            raise ValueError("Source crop coordinates require the two 512 px concept images")
        write_ora(ora, atlas_layers(flux, sdxl))
    result = finish_in_krita(args.root, ora)
    result["inputs"] = [{"model": model, "source": str(path), "sha256": digest(path)}
                        for model, path in (("FLUX.1 Schnell", args.flux), ("SDXL 1.0", args.sdxl))]
    (args.root / "paint-receipt.json").write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"Saved and verified {len(result['layers'])} native Krita layers.")


if __name__ == "__main__":
    main()

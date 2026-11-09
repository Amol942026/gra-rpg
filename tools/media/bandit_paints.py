"""Finish the forest bandit's two atlas cells in the native Krita bridge."""
import argparse
import json
from pathlib import Path
from zipfile import ZipFile

from PIL import Image

from krita_mcp_client import KritaMCP, start_krita_if_needed
from wanderer_paints import CELL, digest, material, origin, write_ora


def box(cell):
    x, y = origin(cell)
    return x, y, x+CELL, y+CELL


def finish(root, original, concept):
    root.mkdir(parents=True, exist_ok=False)
    with Image.open(original) as source:
        atlas = source.convert("RGBA")
    if atlas.size not in ((512, 512),(512, 640)):
        raise ValueError("Unexpected forest creature atlas size")
    legacy = atlas.copy()
    for cell in (6, 15):
        legacy.paste((0, 0, 0, 0), box(cell))
    layers = [("Materiały pozostałych stworzeń", legacy)]
    with Image.open(concept) as flux:
        for cell, label, crop, dark, light in [
            (6, "FLUX · wytarta skóra kurty", (210, 157, 249, 211), "#513524", "#a8784d"),
            (15, "FLUX · włókna czerwonego kaptura", (236, 34, 270, 55), "#502b28", "#9c4c3b"),
        ]:
            layer = Image.new("RGBA", atlas.size)
            layer.paste(material(flux, crop, dark, light), origin(cell))
            layers.insert(0, (label, layer))
    ora = root / "bandit-input.ora"
    write_ora(ora, layers, title="Emberfall — leśny rozbójnik")

    master = root / "forest-bandit.kra"
    exported = root / "krita-export.png"
    start_krita_if_needed()
    with KritaMCP(root / "krita-log") as client:
        doc = client.call("krita_document", action="open", path=str(ora))["affected_document_id"]
        state = client.call("krita_state", document_id=doc)
        if state["runtime_compatibility"]["status"] != "compatible" or len(state["layers"]) != 3:
            raise RuntimeError("Krita did not open the layered source")

        def paint(cell, name):
            client.call("krita_layers", action="create", document_id=doc, name=name)
            layer = client.call("krita_state", document_id=doc)["active_layer_id"]
            x, y = origin(cell)

            def stroke(points, color, width):
                client.call("krita_draw", document_id=doc, layer_id=layer, shape="path",
                            points=[{"x": x+u, "y": y+v} for u, v in points],
                            foreground=color, brush_size=width, fill_style="None",
                            stroke_style="ForegroundColor")
            return stroke

        stroke = paint(6, "Krita · obszycie kurty i zadrapania")
        for x in (19, 105):
            stroke([(x, 8), (x+4, 51), (x, 117)], "#593827", 2.1)
            for y in range(15, 112, 12):
                stroke([(x+3, y), (x+7, y+3)], "#c49a72", 1.5)
        stroke([(27, 100), (42, 105), (66, 107), (103, 102)], "#9b714d", 2)
        stroke([(54, 39), (67, 36), (80, 39)], "#af8a65", 1.4)

        stroke = paint(15, "Krita · szew kaptura i załamania tkaniny")
        stroke([(63, 8), (60, 41), (63, 76), (62, 119)], "#5d322d", 2.8)
        for y in range(12, 118, 11):
            stroke([(64, y), (69, y+2)], "#b76d58", 1.6)
        stroke([(15, 36), (27, 62), (39, 74)], "#713b33", 2.5)
        stroke([(88, 91), (105, 74), (111, 52)], "#b16653", 2.1)

        client.call("krita_save_export", document_id=doc, kra_path=str(master),
                    png_path=str(exported), overwrite=False)
        client.call("krita_preview", document_id=doc, max_width=768, max_height=768)
        state = client.call("krita_state", document_id=doc)
        client.call("krita_document", action="close", document_id=doc, discard_changes=False)

    with Image.open(exported) as source, ZipFile(master) as archive:
        painted = source.convert("RGBA")
        merged = Image.open(archive.open("mergedimage.png")).convert("RGBA")
        if painted.tobytes() != merged.tobytes() or len(state["layers"]) != 5:
            raise ValueError("The native master and exported layers disagree")
        for cell in (6, 15):
            atlas.paste(painted.crop(box(cell)), origin(cell))
    result = root / "creature-paints.png"
    atlas.save(result)
    receipt = {"status": "PASS", "master": str(master), "native_layers": state["layers"],
               "changed_cells": [6, 15], "atlas_sha256": digest(result),
               "original_sha256": digest(original), "concept": str(concept),
               "concept_sha256": digest(concept)}
    (root / "paint-receipt.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2), encoding="utf-8")
    print("Krita: five layers, two revised bandit materials.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--original", type=Path, required=True)
    parser.add_argument("--concept", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    finish(args.root, args.original, args.concept)

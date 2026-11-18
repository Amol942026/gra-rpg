"""Assemble the articulated crawler turn in Krita and round-trip the saved layers."""

import argparse
from pathlib import Path
from zipfile import ZipFile

from PIL import Image

from krita_mcp_client import KritaMCP, start_krita_if_needed
from wanderer_paints import write_ora
from slag_crawler_paint import PARTS, raw_order, shell_points


DEPTH = sorted({(rank, p) for k in range(8) for rank, p in enumerate(raw_order(k))})
LABELS = tuple(f"{rank + 1} · {PARTS[p]}" for rank, p in DEPTH)
SIZE = (1280, 640)


def layout():
    for view in range(32):
        key = ((view + 2) // 4) % 8
        for rank, part in enumerate(raw_order(key)):
            yield (
                view,
                part,
                DEPTH.index((rank, part)),
                ((view % 8) * 160, (view // 8) * 160),
            )


def assemble(cells, folder):
    folder.mkdir(exist_ok=False, parents=True)
    layers = [Image.new("RGBA", SIZE) for _ in LABELS]
    for view, part, layer, pos in layout():
        with Image.open(cells / f"{view:02}-{part}.png") as image:
            if image.size != (160, 160) or not image.getbbox():
                raise ValueError("Invalid cutout")
            layers[layer].paste(image, pos)
    ora = folder / "pelzacz.ora"
    write_ora(ora, list(reversed(list(zip(LABELS, layers)))), "Żużlowy Pełzacz · obrót")
    start_krita_if_needed()
    kra = folder / "pelzacz.kra"
    with KritaMCP(folder / "mcp-authoring") as c:
        doc = c.call("krita_document", action="open", path=str(ora))[
            "affected_document_id"
        ]
        state = c.call("krita_state", document_id=doc)
        if state["runtime_compatibility"]["status"] != "compatible" or len(
            state["layers"]
        ) != len(LABELS):
            raise ValueError("Invalid Krita master")
        ids = {layer["name"]: layer["id"] for layer in state["layers"]}
        # Paint the tiny amber catchlight at its anatomical registration point.
        # The original five paintings already carry their own lens highlights.
        for view, part, index, (x, y) in layout():
            if part != 3 or view % 4 == 0 or view > 15:
                continue
            a = view // 4
            b = (a + 1) % 8
            t = view % 4 / 4
            eye = (1 - t) * shell_points(a, b)[10] + t * shell_points(b, a)[10]
            c.call(
                "krita_draw",
                shape="ellipse",
                document_id=doc,
                layer_id=ids[LABELS[index]],
                x=x + float(eye[0]) - 0.20,
                y=y + float(eye[1]) - 0.45,
                width=1.02,
                height=0.8,
                foreground="#f8a940",
                fill_style="ForegroundColor",
                stroke_style="None",
            )
        c.call(
            "krita_save_export",
            document_id=doc,
            kra_path=str(kra),
            png_path=str(folder / "pelzacz.png"),
            overwrite=False,
        )
        c.call("krita_preview", document_id=doc, max_width=800, max_height=400)
    with ZipFile(kra) as z:
        assert z.testzip() is None and "maindoc.xml" in z.namelist()
    return kra


def export(kra, folder):
    folder.mkdir(exist_ok=False, parents=True)
    start_krita_if_needed()
    layers = []
    with KritaMCP(folder / "mcp-export") as c:
        doc = c.call("krita_document", action="open", path=str(kra))[
            "affected_document_id"
        ]
        state = c.call("krita_state", document_id=doc)
        ids = {layer["name"]: layer["id"] for layer in state["layers"]}
        for i, name in enumerate(LABELS):
            path = folder / f"layer-{i}.png"
            try:
                c.call(
                    "krita_asset",
                    action="export_layer",
                    document_id=doc,
                    layer_id=ids[name],
                    output_path=str(path),
                    crop="canvas",
                    overwrite=False,
                    preview_max_width=160,
                    preview_max_height=80,
                )
            except RuntimeError as e:
                if "BRIDGE_UNAVAILABLE" not in str(e) or not path.is_file():
                    raise
            with Image.open(path) as layer:
                if layer.size != SIZE or not layer.getbbox():
                    raise ValueError(f"Missing paint on {name}")
                layers.append(layer.convert("RGBA"))
    cells = folder / "cells"
    cells.mkdir()
    for view, part, layer, (x, y) in layout():
        tile = layers[layer].crop((x, y, x + 160, y + 160))
        if not tile.getbbox():
            raise ValueError(f"Empty native cutout {view}/{part}")
        tile.save(cells / f"{view:02}-{part}.png")
    return cells


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("assemble", "export"))
    parser.add_argument("input", type=Path)
    parser.add_argument("folder", type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    a = parser.parse_args()
    print(
        (assemble if a.action == "assemble" else export)(
            a.input.resolve(), a.folder.resolve()
        )
    )

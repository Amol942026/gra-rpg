"""Assemble the cleaned angular cutouts into an editable Krita turnaround."""
import argparse
from pathlib import Path
from zipfile import ZipFile

from PIL import Image, ImageChops

from krita_mcp_client import KritaMCP, start_krita_if_needed
from wanderer_paints import write_ora


LABELS = ("Skrzydło dalsze", "Łapy A", "Łapy B", "Głowa za pancerzem",
          "Pancerz i ogon", "Skrzydło bliższe", "Głowa przed pancerzem")
SIZE = (1280, 640)


def cutout_layout():
    for view in range(32):
        compass = ((view+2)//4) % 8
        at = (view % 8 * 160, view // 8 * 160)
        # Physical wings swap their foreground/background roles across the left flank.
        far, near = (5, 0) if 2 <= compass <= 5 else (0, 5)
        for part, layer in enumerate((far, 1, 2, 4, near, 3 if compass >= 5 else 6)):
            yield view, part, layer, at


def make_master(cells, output):
    # In the rear views the neck passes behind the breastplate. Splitting the
    # head between two paint layers preserves occlusion without erasing pixels.
    layers = [Image.new("RGBA", SIZE) for _ in LABELS]
    for view, part, index, at in cutout_layout():
        with Image.open(cells / f"{view:02}-{part}.png") as tile:
            if tile.size != (160, 160) or tile.mode != "RGBA" or not tile.getbbox():
                raise ValueError(f"Invalid directional cutout {view}/{part}")
            layers[index].paste(tile, at)
    output.mkdir(parents=True, exist_ok=False)
    ora = output / "drake-turn.ora"
    master = output / "drake-turn.kra"
    export = output / "drake-turn.png"
    # OpenRaster stack order starts with the topmost paint layer.
    write_ora(ora, list(reversed(list(zip(LABELS, layers)))), "Żarowy Drak · obrót")
    start_krita_if_needed()
    with KritaMCP(output / "mcp") as client:
        doc = client.call("krita_document", action="open", path=str(ora))["affected_document_id"]
        state = client.call("krita_state", document_id=doc)
        if state["runtime_compatibility"]["status"] != "compatible" or len(state["layers"]) != len(LABELS):
            raise RuntimeError("Krita did not open the complete layered turnaround")
        client.call("krita_save_export", document_id=doc, kra_path=str(master),
                    png_path=str(export), overwrite=False)
        client.call("krita_preview", document_id=doc, max_width=960, max_height=480)
    with ZipFile(master) as native:
        if native.testzip() is not None or "maindoc.xml" not in native.namelist():
            raise ValueError("Invalid native Krita file")
    composite = Image.new("RGBA", SIZE)
    for layer in layers:
        composite.alpha_composite(layer)
    # Compare the actual native export on a matte so invisible RGB is ignored.
    matte = Image.new("RGBA", SIZE, (42, 42, 42, 255))
    expected = Image.alpha_composite(matte, composite).convert("RGB")
    with Image.open(export) as saved:
        actual = Image.alpha_composite(matte, saved.convert("RGBA")).convert("RGB")
    hist = ImageChops.difference(expected, actual).histogram()
    large = sum(sum(hist[channel * 256 + 14:(channel + 1) * 256]) for channel in range(3))
    if large > SIZE[0] * SIZE[1] * .004:
        raise ValueError("Native layer ordering or compositing differs from the cutouts")
    return master


def export_cells(master, output):
    output.mkdir(parents=True, exist_ok=False)
    start_krita_if_needed()
    layers = []
    with KritaMCP(output / "mcp") as client:
        doc = client.call("krita_document", action="open", path=str(master))["affected_document_id"]
        state = client.call("krita_state", document_id=doc)
        ids = {layer["name"]: layer["id"] for layer in state["layers"]}
        for index, name in enumerate(LABELS):
            path = output / f"layer-{index}.png"
            try:
                client.call("krita_asset", document_id=doc, action="export_layer", layer_id=ids[name],
                            output_path=str(path), crop="canvas", overwrite=False,
                            preview_max_width=160, preview_max_height=80)
            except RuntimeError as error:
                # Some bridge versions disconnect while returning the preview of
                # an already saved layer. Verify that export instead of repeating it.
                if "BRIDGE_UNAVAILABLE" not in str(error) or not path.is_file():
                    raise
            with Image.open(path) as image:
                if image.size != SIZE or image.mode != "RGBA" or not image.getbbox():
                    raise ValueError(f"Invalid native layer dimensions for {name}")
                layers.append(image.convert("RGBA"))
    cells = output / "cells"
    cells.mkdir()
    for view, part, index, (x, y) in cutout_layout():
        tile = layers[index].crop((x, y, x+160, y+160))
        if not tile.getbbox():
            raise ValueError(f"Missing native cutout {view}/{part}")
        tile.save(cells / f"{view:02}-{part}.png")
    return cells


if __name__ == "__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("action", choices=("assemble", "export"))
    cli.add_argument("source", type=Path)
    cli.add_argument("output", type=Path)
    cli.add_argument("--no-audio", action="store_true", required=True)
    args = cli.parse_args()
    operation = make_master if args.action == "assemble" else export_cells
    print(operation(args.source.resolve(), args.output.resolve()))

"""Paint a resting-only finish over the existing directional Cinderhorn masters."""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter

from krita_mcp_client import KritaMCP, start_krita_if_needed


# Contours of the existing paint at its original 887-pixel width. Small
# fragments follow the bevel or growth direction of the material they touch.
MARKS = {
    "front": (
        ("#b8a798", 7, [(148,318),(163,311),(186,304),(203,291)]),
        ("#a0a8ad", 8, [(127,358),(144,350),(159,342),(170,340)]),
        ("#c1b19f", 6, [(325,349),(352,354),(368,352),(384,345)]),
        ("#aaa6a4", 7, [(397,371),(417,385),(431,391),(442,391)]),
        ("#9b9490", 5, [(433,446),(445,455),(449,466),(463,470)]),
        ("#826751", 5, [(517,277),(505,290),(496,307),(485,313)]),
        ("#ac9275", 6, [(565,356),(554,376),(552,394),(557,410)]),
        ("#ad9074", 6, [(620,416),(614,431),(614,446),(606,466)]),
        ("#ba8f66", 6, [(376,77),(403,73),(425,79),(444,90)]),
        ("#c59871", 7, [(488,117),(500,130),(508,146),(519,149)]),
        ("#bb8f67", 7, [(609,207),(620,222),(625,237),(637,239)]),
        ("#c89e78", 6, [(648,254),(661,269),(672,272)]),
        ("#d67746", 9, [(435,396),(452,409),(467,415),(488,437)]),
        ("#ed9b64", 4, [(451,408),(462,412),(465,420),(477,424)]),
        ("#cb754a", 6, [(147,410),(162,405),(180,392),(193,388)]),
        ("#e39c68", 4, [(177,394),(185,389),(191,386)]),
    ),
    "back": (
        ("#9d9c9b", 7, [(255,348),(270,338),(288,335),(302,329)]),
        ("#ada8a3", 7, [(301,373),(324,359),(343,351),(357,350)]),
        ("#b2a7a0", 7, [(300,459),(313,446),(326,441),(344,427)]),
        ("#b4aaa0", 6, [(321,553),(341,556),(351,548),(365,541)]),
        ("#a4a0a0", 7, [(529,308),(542,305),(553,293),(575,287)]),
        ("#a19a94", 6, [(627,377),(642,385),(651,393),(662,394)]),
        ("#ad9277", 5, [(453,245),(465,258),(483,268),(496,268)]),
        ("#b49a85", 6, [(588,204),(572,215),(567,228),(560,239)]),
        ("#b89b85", 5, [(665,167),(671,184),(668,202),(660,212)]),
        ("#c6a07b", 7, [(454,151),(470,137),(489,126),(505,122)]),
        ("#ce9c73", 6, [(523,111),(541,103),(558,102),(570,95)]),
        ("#c09b77", 6, [(629,54),(641,55),(655,63),(666,68)]),
        ("#d78657", 8, [(292,424),(310,410),(330,407),(351,395),(367,393)]),
        ("#f0ae77", 4, [(319,409),(328,407),(345,401),(353,396)]),
        ("#c78052", 6, [(649,307),(674,315),(684,323),(701,329)]),
        ("#f0a56c", 4, [(675,317),(682,324),(693,327)]),
    ),
}


def finish_surface(source: Image.Image) -> Image.Image:
    rgb = np.array(source.convert("RGB"), dtype=np.float32)
    broad = np.array(source.convert("RGB").filter(ImageFilter.GaussianBlur(9)), dtype=np.float32)
    fine = np.array(source.convert("RGB").filter(ImageFilter.GaussianBlur(2)), dtype=np.float32)
    tone = rgb @ np.array((.25, .62, .13), dtype=np.float32)
    y = np.arange(source.height)[:, None]
    upper = np.clip((660-y)/450, 0, 1)
    # Lift the upper rock faces and leave the shaded undercoat cool.
    contrast = (.15 + .18*upper)[..., None]
    chroma = np.clip((rgb[:,:,0]-rgb[:,:,2]-6)/37, 0, 1)
    cool = np.clip((rgb[:,:,2]-rgb[:,:,0]+12)/37, 0, 1)
    sheen = np.clip((tone-64)/83, 0, 1) * upper
    rgb += (rgb-broad) * contrast + (rgb-fine)*.20
    rgb += (sheen[..., None] * np.array((24,18,13))
            + (sheen*cool)[..., None] * np.array((-7,0,10))
            + (sheen*chroma)[..., None] * np.array((8,0,-8)))
    result = Image.fromarray(np.clip(rgb+.5,0,255).astype("uint8")).convert("RGBA")
    result.putalpha(source.getchannel("A"))
    return result


def paint(source: Path, output: Path) -> None:
    start_krita_if_needed()
    with KritaMCP(output/"krita") as krita:
        for view in MARKS:
            pixels = Image.open(source/f"{view}.png").convert("RGBA")
            finish = output/f"{view}-swiatlo.png"
            finish_surface(pixels).save(finish)
            document = krita.call("krita_document", action="open", path=str(finish))["affected_document_id"]
            state = krita.call("krita_state", document_id=document)
            if state["runtime_compatibility"]["status"] != "compatible":
                raise RuntimeError(state["runtime_compatibility"])
            krita.call("krita_layers", action="rename", document_id=document,
                       layer_id=state["active_layer_id"], name="Płyty, sierść i rogi · światło boczne")
            labels = ("Wykruszone krawędzie i słoje rogów", "Żar wewnątrz szczelin")
            accents = []
            for label in labels:
                layer = krita.call("krita_layers", action="create", document_id=document,
                                   name=label)["active_layer_id"]
                accents.append(layer)
            for index, (color, width, points) in enumerate(MARKS[view]):
                valid = [{"x": x, "y": y} for x,y in points if pixels.getpixel((x,y))[3] > 235]
                if len(valid) >= 2:
                    krita.call("krita_draw", document_id=document, layer_id=accents[int(index>=12)],
                               shape="path", points=valid, foreground=color, brush_size=width,
                               fill_style="None", stroke_style="ForegroundColor")
            for layer, opacity in zip(accents, (.68, .80)):
                krita.call("krita_layers", action="set_properties", document_id=document,
                           layer_id=layer, opacity=opacity)
            saved = output/f"{view}-malowanie.png"
            krita.call("krita_save_export", document_id=document, kra_path=str(output/f"{view}.kra"),
                       png_path=str(saved), overwrite=False)
            krita.call("krita_preview", document_id=document, max_width=450, max_height=450)
            painted = Image.open(saved).convert("RGBA")
            # Material changes never alter any edge, stored ground contact or pivot.
            painted.putalpha(pixels.getchannel("A"))
            painted.save(output/f"{view}.png")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    options = parser.parse_args()
    options.output.mkdir(parents=True, exist_ok=False)
    paint(options.source, options.output)


if __name__ == "__main__":
    main()

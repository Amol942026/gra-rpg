"""Refinish the original directional Reaper cards without changing their cutouts."""

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image
from scipy.ndimage import gaussian_filter

from desert_enemy_paint import CELL, add_accents, authored, bake, ora
from krita_mcp_client import KritaMCP, start_krita_if_needed


def smooth(edge, width, values):
    t = np.clip((values-edge)/width, 0, 1)
    return t*t*(3-2*t)


def material_fields(materials):
    w, h = materials.size
    fields = []
    for i in range(4):
        x, y = i % 2 * (w//2), i // 2 * (h//2)
        crop = materials.crop((x+12, y+12, x+w//2-12, y+h//2-12))
        pixels = np.array(crop.convert("L").resize((CELL, CELL), Image.Resampling.LANCZOS), dtype=np.float32)
        fields.append((pixels-gaussian_filter(pixels, 6)) + .32*(pixels-pixels.mean()))
    return fields


def refinish(part, fields, view):
    old = np.asarray(part)
    rgb = old[..., :3].astype(np.float32)
    r, g, b = np.moveaxis(rgb, 2, 0)
    value = .24*r+.63*g+.13*b
    glass = smooth(1, 9, g-r)*smooth(-4, 12, g-b)
    bronze = (1-glass)*smooth(7, 28, r-b)
    cloth = (1-glass-bronze)*smooth(0, 12, b-g)
    leather = 1-glass-bronze-cloth
    material = (glass, bronze, cloth, leather)
    solid = old[..., 3]/255.
    weights = gaussian_filter(solid, 1.45)
    local_light = gaussian_filter(value*solid, 1.45)/np.maximum(weights, .001)
    bevel = np.clip(value-local_light, -38, 40)
    lit = smooth(25, 75, value)
    color = np.array(rgb)
    offset = (view % 4)*19
    for mask, texture, tint, depth in zip(material, fields,
            ((-6, 20, 15), (21, 10, -1), (0, 4, 19), (4, 2, 1)), (1.48, 1.10, 1.20, .68)):
        texture = np.roll(texture, offset, axis=1)
        color += mask[..., None]*(lit[..., None]*np.array(tint)+
                                  (bevel*.57+texture*depth)[..., None])
    color += (4+glass*6)[..., None]*smooth(70, 39, value)[..., None]
    output = old.copy()
    output[..., :3] = np.round(color).clip(0, 255)
    return Image.fromarray(output)


def paint_accents(keys, work, sprite_source):
    document = work/"zniwiarz-warstwy-materialu.ora"
    ora(keys, "reaper", document)
    start_krita_if_needed()
    with KritaMCP(work/"krita") as client:
        doc = client.call("krita_document", action="open", path=str(document))["affected_document_id"]
        client.call("krita_state", document_id=doc)
        layer = client.call("krita_layers", action="create", document_id=doc,
                            name="Fazy szkła i ślady zużycia")["active_layer_id"]
        for view, (parts, _) in enumerate(keys):
            for slot in (0, 3):
                pixels = np.asarray(parts[slot]).astype(float)
                rgb = pixels[..., :3]
                light = rgb.mean(axis=2)
                score = light - gaussian_filter(light, 2)
                inner = (pixels[..., 3] > 248) & (rgb[..., 1]-rgb[..., 0] > 4)
                if slot == 3:
                    inner[69:] = False
                inner[:, :3] = inner[:, -3:] = False
                inner &= np.roll(inner, -1, axis=1) & np.roll(inner, -2, axis=1)
                score[~inner] = -1000
                for _ in range(3):
                    y, x = map(int, np.unravel_index(score.argmax(), score.shape))
                    if score[y, x] < 1:
                        break
                    client.call("krita_draw", document_id=doc, layer_id=layer, shape="path",
                                points=[{"x": xx + view%4*CELL, "y": y + view//4*CELL}
                                        for xx in (x, x+2)],
                                foreground="#a2c9b1", stroke_style="ForegroundColor",
                                fill_style="None", brush_size=1.3)
                    score[max(0, y-6):y+7, max(0, x-6):x+7] = -1000
        client.call("krita_asset", document_id=doc, action="export_layer", layer_id=layer,
                    output_path=str(work/"akcenty.png"), crop="canvas", overwrite=False)
        client.call("krita_save_export", document_id=doc, kra_path=str(sprite_source),
                    png_path=str(sprite_source.with_suffix(".png")), overwrite=False)
        client.call("krita_preview", document_id=doc, max_width=512, max_height=256)
    with Image.open(work/"akcenty.png") as accents:
        add_accents(keys, accents.convert("RGBA"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--material", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    work = args.work.resolve()
    sheet = Image.open(args.material).convert("RGB")
    sheet = sheet.resize((768, 768), Image.Resampling.LANCZOS)
    sheet.save(work/"materialy-reaper.png")
    fields = material_fields(sheet)
    old = authored(Path("art/desert/new-enemies/reaper-source.png"), "reaper")
    keys = [([refinish(part, fields, view) for part in parts], points)
            for view, (parts, points) in enumerate(old)]
    master = Path("C:/AI2D/sprites")/(work.name+".kra")
    paint_accents(keys, work, master)
    for (before, _), (after, _) in zip(old, keys):
        assert all(a.getchannel("A").tobytes() == b.getchannel("A").tobytes()
                   for a, b in zip(before, after))
    parts, pivots = keys[1]
    cards = work/"cards"
    cards.mkdir(exist_ok=True)
    for index, part in enumerate(parts):
        part.save(cards/f"{index}.png")
    (cards/"rig.json").write_text(json.dumps({"pivots": pivots[:6], "joints": pivots[6:]}))
    out = work/"atlas"
    out.mkdir(exist_ok=True)
    bake(keys, "reaper", work, out, work/"desert_reaper_turn_atlas.hpp")
    print(json.dumps({"master": str(master), "preserved_cutouts": 48, "atlas": str(out)}))


if __name__ == "__main__":
    main()

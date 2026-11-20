"""Prepare six layered paintings of the ram and export its 32 directional views."""

import argparse
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageOps

from ember_drake_turn import Paint, UV, inbetween
from krita_mcp_client import KritaMCP, start_krita_if_needed
from slag_crawler_paint import atlas
from wanderer_paints import write_ora

CELL = 160
KEYS = (0, 1, 2, 6, 7)
PARTS = ("Tylna lewa", "Tylna prawa", "Głowa", "Tułów", "Przednia lewa", "Przednia prawa")

# The cuts include the upper leg below the plates and wool at the neck.
# A far leg hidden in the painting is built from the other leg of that pair.
CUTS = {
    0: {
        1: [(31,85),(58,85),(58,111),(44,122),(47,146),(19,146),(22,107)],
        2: [(59,29),(127,29),(144,70),(126,95),(113,108),(99,100),(97,84),(86,76),(67,71)],
        3: [(16,56),(119,55),(122,97),(110,111),(93,117),(70,121),(51,118),(35,112),(15,100)],
        5: [(74,84),(105,83),(111,103),(105,120),(110,147),(82,147),(79,115)],
    },
    1: {
        0: [(61,82),(82,86),(86,101),(77,114),(78,136),(58,136),(57,110)],
        1: [(41,85),(61,79),(62,105),(53,118),(51,141),(30,141),(31,111)],
        2: [(60,30),(130,30),(141,69),(125,94),(112,106),(98,103),(93,89),(86,80),(70,69)],
        3: [(33,58),(130,54),(132,91),(123,103),(106,115),(78,118),(54,111),(36,106)],
        4: [(111,79),(123,78),(128,111),(128,122),(134,142),(110,142),(108,115)],
        5: [(73,86),(98,84),(102,106),(95,122),(100,147),(78,147),(71,113)],
    },
    2: {
        0: [(81,93),(93,93),(94,118),(91,136),(80,136),(81,116)],
        1: [(65,93),(79,92),(78,116),(79,136),(66,136),(62,119)],
        2: [(45,31),(114,31),(114,74),(105,90),(92,107),(81,113),(67,108),(53,96),(47,75)],
        3: [(42,65),(113,65),(116,98),(105,112),(91,123),(68,123),(47,112),(41,94)],
        4: [(87,88),(106,87),(108,110),(105,127),(112,146),(91,146),(87,126)],
        5: [(55,88),(74,90),(73,119),(68,131),(73,146),(52,146),(49,128),(52,112)],
    },
    6: {
        0: [(49,86),(75,85),(77,104),(70,125),(69,146),(48,146),(49,119)],
        1: [(87,86),(108,86),(108,115),(106,126),(113,146),(90,146),(89,125)],
        2: [(47,30),(113,30),(113,62),(100,70),(62,71),(48,61)],
        3: [(40,47),(117,46),(116,92),(106,105),(82,108),(53,106),(42,95)],
        4: [(62,91),(77,93),(77,124),(75,135),(59,135),(58,117)],
        5: [(82,93),(96,94),(99,116),(96,135),(82,135)],
    },
    7: {
        0: [(44,85),(61,84),(65,106),(54,117),(52,133),(34,133),(33,111)],
        1: [(57,89),(80,85),(83,103),(74,120),(77,147),(58,147),(53,118)],
        2: [(56,28),(122,31),(129,56),(121,73),(106,77),(92,67),(76,66),(65,62)],
        3: [(37,57),(120,44),(134,78),(112,98),(92,107),(69,116),(46,110),(37,95)],
        4: [(73,78),(91,80),(91,103),(88,121),(74,121),(72,103)],
        5: [(93,78),(112,73),(112,98),(107,112),(116,129),(97,129),(90,104)],
    },
}

MIRRORED = (1, 0, 2, 3, 5, 4)
ORDER = (
    (0, 4, 3, 1, 5, 2), (0, 4, 3, 1, 5, 2), (0, 1, 3, 4, 5, 2),
    (1, 5, 3, 0, 4, 2), (1, 5, 3, 0, 4, 2), (5, 4, 2, 3, 1, 0),
    (4, 5, 2, 3, 0, 1), (4, 5, 2, 3, 0, 1),
)


def normalize(source):
    source = source.crop(source.getchannel("A").point(lambda a: 255 if a > 55 else 0).getbbox())
    scale = min(116 / source.width, 111 / source.height)
    source = source.resize((round(scale * source.width), round(scale * source.height)), Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", (CELL, CELL))
    canvas.paste(source, (80 - source.width // 2, 145 - source.height))
    return canvas


def clipped(image, points):
    mask = Image.new("L", image.size)
    ImageDraw.Draw(mask).polygon(points, fill=255)
    mask = np.asarray(mask.filter(ImageFilter.GaussianBlur(.95)), dtype=np.float32) / 255
    pixels = np.array(image)
    pixels[..., 3] = np.rint(pixels[..., 3] * mask).astype(np.uint8)
    return Image.fromarray(pixels)


def far_leg(near, offset):
    leg = near.crop(near.getbbox())
    leg = leg.resize((round(leg.width * .78), round(leg.height * .86)), Image.Resampling.LANCZOS)
    pixels = np.array(leg)
    pixels[..., :3] = np.rint(pixels[..., :3] * .72).astype(np.uint8)
    result = Image.new("RGBA", (CELL, CELL))
    result.paste(Image.fromarray(pixels), offset)
    return result


def make_master(source, old, root, master):
    image = Image.open(source).convert("RGBA")
    x, y = image.width // 2, image.height // 2
    front, rear, side, quarter = [
        image.crop(bounds) for bounds in ((0, 0, x, y), (x, 0, 2*x, y), (0, y, x, 2*y), (x, y, 2*x, 2*y))
    ]
    rear_quarter = Image.open(old).convert("RGBA").crop((150, 2, 294, 146))
    slices = [normalize(frame) for frame in (side, quarter, front, rear, rear_quarter)]
    layers = [Image.new("RGBA", (len(KEYS) * CELL, len(PARTS) * CELL)) for _ in PARTS]
    for c, (d, frame) in enumerate(zip(KEYS, slices, strict=True)):
        pieces = {i: clipped(frame, points) for i, points in CUTS[d].items()}
        if d == 0:
            pieces[0] = far_leg(pieces[1], (43, 83))
            pieces[4] = far_leg(pieces[5], (108, 85))
        for i, part in sorted(pieces.items()):
            layers[i].paste(part, (c * CELL, i * CELL))

    ora = root / "rogacz.ora"
    write_ora(ora, list(zip(PARTS, layers, strict=True)), "Rogacz Żaru — klucze obrotu")
    start_krita_if_needed()
    with KritaMCP(root / "krita") as client:
        doc = client.call("krita_document", action="open", path=str(ora))["affected_document_id"]
        state = client.call("krita_state", document_id=doc)
        assert len(state["layers"]) == len(PARTS)
        client.call("krita_preview", document_id=doc, max_width=480, max_height=576)
        client.call("krita_save_export", document_id=doc, kra_path=str(master),
                    png_path=str(master.with_suffix(".png")), overwrite=False)


def read_paintings(source):
    sheet = Image.open(source).convert("RGBA")
    assert sheet.size == (len(KEYS) * CELL, len(PARTS) * CELL)
    keys = {
        d: [sheet.crop((c * CELL, p * CELL, (c + 1) * CELL, (p + 1) * CELL)) for p in range(6)]
        for c, d in enumerate(KEYS)
    }
    for d, original in ((3, 1), (4, 0), (5, 7)):
        keys[d] = [ImageOps.mirror(keys[original][p]) for p in MIRRORED]
    return keys, {d: [Paint(layer) for layer in frames] for d, frames in keys.items()}


def prepare_film(source, destination):
    _, paintings = read_paintings(source)
    frames = []
    for d in (*range(8), 0):
        sheet = Image.new("RGB", (UV * 3, UV * 2))
        for p, tile in enumerate(paintings[d]):
            sheet.paste(Image.fromarray(np.uint8(np.rint(tile.texture))), (p % 3 * UV, p // 3 * UV))
        frames.append(sheet)
    frames[0].save(destination, save_all=True, append_images=frames[1:], duration=120, loop=0)


def bake(source, root, output, header, film):
    keys, refs = read_paintings(source)
    frames = sorted(film.glob("view_*.png")) if film else []
    if film and len(frames) != 33:
        raise ValueError("Expected 33 ComfyUI FILM frames")
    views = []
    for angle in range(32):
        d, step = divmod(angle, 4)
        t = step / 4
        if not step:
            views.append(keys[d])
            continue
        pigments = Image.open(frames[angle]).convert("RGB") if frames else None
        parts = []
        for p, (a, b) in enumerate(zip(refs[d], refs[(d + 1) % 8], strict=True)):
            x, y = p % 3 * UV, p // 3 * UV
            material = np.asarray(pigments.crop((x, y, x + UV, y + UV)), dtype=np.float32) if pigments else None
            parts.append(inbetween(a, b, t, material, retain_detail=True))
        views.append(parts)
    atlas(root, views, output, header, name="cinderhorn", ordering=ORDER.__getitem__)


if __name__ == "__main__":
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument("source", type=Path)
    cli.add_argument("work", type=Path)
    cli.add_argument("--old", type=Path)
    action = cli.add_mutually_exclusive_group()
    action.add_argument("--master", type=Path)
    cli.add_argument("--output", type=Path)
    cli.add_argument("--header", type=Path)
    action.add_argument("--prepare-film", type=Path)
    cli.add_argument("--film", type=Path)
    cli.add_argument("--no-audio", action="store_true", required=True)
    a = cli.parse_args()
    if a.master and not a.old:
        cli.error("--master requires --old with the painted rear quarter")
    if not (a.master or a.prepare_film or (a.output and a.header)):
        cli.error("atlas export requires --output and --header")
    a.work.mkdir(parents=True, exist_ok=True)
    if a.master:
        make_master(a.source, a.old, a.work, a.master)
    elif a.prepare_film:
        prepare_film(a.source, a.prepare_film)
    else:
        bake(a.source, a.work, a.output, a.header, a.film)

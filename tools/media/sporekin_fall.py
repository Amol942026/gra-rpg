"""Compose the fallen Zarodnik kniei from its painted Krita cutouts."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import time
from xml.etree import ElementTree as ET
from zipfile import ZipFile

from PIL import Image, ImageDraw

from hero_synfig import group, image, rotate, value
from regional_enemy_visuals import ANCHOR, PAD, SIZE, STRIDE, animated_layer, digest, split_parts
import synfig_cli


FRAMES = 8


def units(point):
    return ((point[0] - 72) / 36, (72 - point[1]) / 36)


def ease(t):
    return t * t * (3 - 2 * t)


def transform(canvas, name, origin, scale):
    layer = animated_layer(canvas, "stretch", "amount", "vector", scale)
    layer.set("desc", name)
    value(layer, "center", "vector", units(origin))


def move(canvas, offsets):
    animated_layer(canvas, "translate", "origin", "vector", [(x / 36, -y / 36) for x, y in offsets])


def parts(work, source):
    art = Image.open(source).convert("RGBA")
    x0, y0, x1, y1 = art.getchannel("A").point(lambda a: 255 if a >= 55 else 0).getbbox()
    scale = min(94 / (x1 - x0), 73 / (y1 - y0))

    def fit(x, y):
        return ANCHOR[0] + (x - (x0 + x1) / 2) * scale, ANCHOR[1] + (y - y1) * scale

    cuts = {}
    for name, piece, joint in split_parts("sporekin", art):
        bounds = piece.getchannel("A").getbbox()
        piece.crop(bounds).save(work / "parts" / f"{name}.png")
        cuts[name] = (bounds, joint)
    return fit, cuts


def author(project, fit, cuts, side, rear):
    canvas = ET.Element("canvas", version="1.2", width=str(SIZE * 2), height=str(SIZE * 2), fps="60",
                        **{"view-box": "-2 2 2 -2", "antialias": "3", "begin-time": "0f",
                           "end-time": f"{FRAMES - 1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = f"Emberfall / Zarodnik kniei / {'lewy' if side < 0 else 'prawy'} bok"
    ET.SubElement(canvas, "desc").text = "Sześć malowanych warstw Krity, wspólne kołysanie i niezależne stawy."
    rig = group(canvas, "Kapelusz, korpus i kończyny")
    timing = [i / (FRAMES - 1) for i in range(FRAMES)]
    near = "left" if (side < 0) != rear else "right"
    far = "right" if near == "left" else "left"
    order = (f"{far}-leg", f"{far}-arm", f"{near}-leg", "body", f"{near}-arm", "cap")
    for part in order:
        box, joint = cuts[part]
        drawing = group(rig, part)
        pivot = fit(*joint)
        image(drawing, part, (*units(fit(*box[:2])), *units(fit(*box[2:]))))
        lifting = [ease(t) for t in timing]
        if part == "cap":
            # The stem bends first. The broad cap tilts back a little and its
            # gill rim is visible above the ground.
            angles = [side * (12 if rear else 17) * t for t in lifting]
            scale = [(1 + .035 * t, 1 - .13 * t) for t in lifting]
            offsets = [(-side * 2.4 * t, -2.8 * t) for t in lifting]
        elif part == "body":
            angles = [-side * 4 * t for t in lifting]
            scale = [(1 + .075 * t, 1 - .05 * t) for t in lifting]
            offsets = [(0, -1.2 * t) for t in lifting]
        else:
            foreground = part.startswith(near)
            leg = part.endswith("leg")
            fold = (-19 if leg else -31) if foreground else (29 if leg else 44)
            angles = [side * fold * t for t in lifting]
            scale = [(1 - (.08 if leg else .14) * t, 1 - (.08 if foreground else .27) * t) for t in lifting]
            offsets = [(side * (2.2 if foreground else -1.2) * t, (-3.5 if leg else 2.7) * t) for t in lifting]
        transform(drawing, "Skrót przy zetknięciu", pivot, scale)
        rotate(drawing, part, units(pivot), angles)
        move(drawing, offsets)

    core = (72, 92)
    tilt = -side * (74 if rear else 80)
    rotate(rig, "Upadek na bok", units(core), [tilt * t for t in timing])
    transform(rig, "Skrót perspektywiczny", core, [(1 + .025 * t, 1 - .23 * t) for t in timing])
    move(rig, [(side * 2.6 * t, 6.5 * t) for t in timing])
    ET.indent(canvas)
    ET.ElementTree(canvas).write(project, encoding="utf-8", xml_declaration=True)


def bake(source, animation, run):
    animation.mkdir(parents=True, exist_ok=False)
    run.mkdir(parents=True, exist_ok=False)
    begin = time.perf_counter()
    receipt = {"source": [], "atlases": [], "anchor": ANCHOR}
    for view in ("front", "back"):
        png = source / f"{view}.png"
        native = source / f"{view}.kra"
        with ZipFile(native) as document:
            if document.testzip() or "maindoc.xml" not in document.namelist():
                raise ValueError(f"Invalid Krita master: {native}")
        receipt["source"].append({"path": str(native), "sha256": digest(native), "png_sha256": digest(png)})
        work = animation / view
        (work / "parts").mkdir(parents=True)
        fit, cuts = parts(work, png)
        atlas = Image.new("RGBA", (FRAMES * STRIDE, STRIDE * 2))
        preview = Image.new("RGBA", atlas.size, (47, 52, 40, 255))
        labels = ImageDraw.Draw(preview)
        passes = []
        for row, side in enumerate((1, -1)):
            project = work / f"bok-{side:+d}.sif"
            author(project, fit, cuts, side, view == "back")
            completed = synfig_cli.render(project, output_root=work / "renders", threads=2,
                                          expected_frames=FRAMES, require_motion=True)
            bounds = []
            for index, path in enumerate(sorted(Path(completed["OutputDirectory"]).glob("frame*.png"))):
                rgba = Image.open(path).convert("RGBA").resize((SIZE, SIZE), Image.Resampling.LANCZOS)
                alpha = rgba.getchannel("A").point(lambda a: 255 if a >= 75 else 0)
                box = alpha.getbbox()
                if not box or min(box[:2]) < 3 or max(box[2:]) > SIZE - 3:
                    raise ValueError(f"Pose touches atlas margin: {view}/{side}/{index}: {box}")
                # Hold the authored ground plane through the whole roll. The
                # body width/height still comes entirely from the separate parts.
                body = Image.new("RGBA", (SIZE, SIZE))
                body.paste(rgba, (0, ANCHOR[1] + 1 - box[3]))
                box = body.getchannel("A").point(lambda a: 255 if a >= 75 else 0).getbbox()
                dest = (index * STRIDE + PAD, row * STRIDE + PAD)
                atlas.paste(body, dest)
                preview.alpha_composite(body, dest)
                labels.text((dest[0] + 9, dest[1] + 6), f"{side:+d} / {index + 1}", fill=(208, 188, 145))
                bounds.append(box)
            start, end = bounds[0], bounds[-1]
            if end[3] != ANCHOR[1] + 1 or end[2] - end[0] < start[2] - start[0] or end[3] - end[1] < 39:
                raise ValueError(f"Missing grounded volume: {view}/{side}: {start} -> {end}")
            passes.append({"bounds": bounds, **{key: completed[key] for key in
                           ("Project", "ProjectSha256", "ActualFrames", "UniquePixelFrames", "ElapsedSeconds")}})
        output = run / f"sporekin-fall-{view}.png"
        atlas.save(output)
        preview.save(run / f"sporekin-{view}-preview.png")
        receipt["atlases"].append({"file": str(output), "sha256": digest(output),
                                   "size": atlas.size, "synfig": passes})
    receipt["render_export_seconds"] = time.perf_counter() - begin
    receipt["status"] = "PASS"
    (run / "report.json").write_text(json.dumps(receipt, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--animation", type=Path, required=True)
    parser.add_argument("--run", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    bake(args.source, args.animation, args.run)

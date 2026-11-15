"""Animate the painted Dune Eye rig and reuse its final frame as the corpse."""
from __future__ import annotations

import argparse
import json
import math
import shutil
import time
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

import synfig_cli


CELL = 144
STRIDE = 148
FRAMES = 16
COLUMNS = 8
SOURCE = Path("C:/AI2D/animations/region-enemies-20261111/dune-eye")
RUNTIME = Path("assets/sprites/region-enemies/dune-eye-fall.png")


def ease(t: float, start: float, end: float) -> float:
    x = min(1.0, max(0.0, (t - start) / (end - start)))
    return x * x * (3 - 2 * x)


def animated(layer: ET.Element, param: str, kind: str, values: list) -> None:
    parameter = layer.find(f"param[@name='{param}']")
    if parameter is None:
        parameter = ET.SubElement(layer, "param", name=param)
    previous = parameter.find("animated")
    if previous is not None:
        parameter.remove(previous)
    curve = ET.SubElement(parameter, "animated", type=kind)
    for name, value in enumerate(values):
        key = ET.SubElement(curve, "waypoint", time=f"{name}f", before="linear", after="linear")
        sample = ET.SubElement(key, kind)
        if kind == "angle":
            sample.set("value", f"{value:.7f}")
        else:
            ET.SubElement(sample, "x").text = f"{value[0]:.7f}"
            ET.SubElement(sample, "y").text = f"{value[1]:.7f}"


def author(path: Path, flank: int) -> None:
    canvas = ET.parse(SOURCE / "fall.sif").getroot()
    canvas.set("fps", "60")
    canvas.set("end-time", f"{FRAMES - 1}f")
    canvas.find("name").text = "Emberfall / Oko Wydm / utrata lewitacji"
    layers = canvas.find("layer/param[@name='canvas']/canvas")
    for layer in list(layers):
        if layer.attrib.get("type") != "PasteCanvas":
            layers.remove(layer)
    # The keystone drops just in front of the lens. Smaller pieces land
    # independently, leaving the painted lens exposed against the sand.
    ground = [
        (0, 115, -21), (17, 103, -45), (-24, 112, 27), (26, 117, -39),
        (18, 126, -12), (-8, 124, 28), (-31, 124, 36), (2, 132, -29),
    ]
    for index, layer in enumerate(layers):
        part = layer.find("param[@name='canvas']/canvas")
        rotation = part.find("layer[@type='rotate']")
        pivot = rotation.find("param[@name='origin']/vector")
        ox = float(pivot.findtext("x"))
        oy = float(pivot.findtext("y"))
        move = part.find("layer[@type='translate']")
        width = ET.Element("layer", type="stretch", active="true", version="0.1")
        center = ET.SubElement(ET.SubElement(width, "param", name="center"), "vector")
        ET.SubElement(center, "x").text = str(ox)
        ET.SubElement(center, "y").text = str(oy)
        part.insert(list(part).index(rotation), width)
        sx, sy, angle = ground[index]
        horizontal, vertical, rotation_values, volume = [], [], [], []
        for frame in range(FRAMES):
            t = frame / (FRAMES - 1)
            lag = (index % 3) * .035
            fall = ease(t, .07 + lag, .90 - lag)
            tilt = ease(t, .02 + lag, .84)
            flare = math.sin(fall * math.pi)
            bounced = math.sin(ease(t, .57 - lag, .89) * math.pi) * ease(t, .58 - lag, .70)
            x = ((sx / 36 * flank - ox) * fall) + flare * ox * .20
            y = ((72 - sy) / 36 - oy) * fall + (flare * 2.1 + bounced * (2 if index else 1)) / 36
            horizontal.append(x)
            vertical.append(y)
            rotation_values.append(angle * tilt * flank + 7 * flare * flank)
            volume.append((1 + .03 * fall, 1 - (.32 if index == 0 else .10) * fall))
        animated(move, "origin", "vector", list(zip(horizontal, vertical)))
        animated(rotation, "amount", "angle", rotation_values)
        animated(width, "amount", "vector", volume)
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run", required=True)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    source = Path("C:/AI2D/animations") / args.run
    run = Path("C:/AI2D/pipeline/runs") / args.run
    source.mkdir()
    run.mkdir()
    shutil.copytree(SOURCE / "parts", source / "parts")
    sheet = Image.new("RGBA", (COLUMNS * STRIDE, 4 * STRIDE))
    gallery = Image.new("RGBA", (CELL * 8, CELL * 2), (63, 51, 37, 255))
    start = time.perf_counter()
    reports = []
    for row, flank in enumerate((1, -1)):
        project = source / f"oko-wydm-{row + 1}.sif"
        author(project, flank)
        result = synfig_cli.render(project, output_root=source / "renders", threads=2,
                                   expected_frames=FRAMES, require_motion=True)
        reports.append(result)
        files = sorted(Path(result["OutputDirectory"]).glob("frame*.png"))
        for frame, file in enumerate(files):
            paint = Image.open(file).convert("RGBA")
            if paint.size != (CELL, CELL):
                raise ValueError(f"{file}: invalid cell size {paint.size}")
            bounds = paint.getchannel("A").getbbox()
            if not bounds or min(bounds[:2]) < 1 or max(bounds[2:]) > CELL - 1:
                raise ValueError(f"{file}: content exceeds cell: {bounds}")
            sheet.paste(paint, ((frame % COLUMNS) * STRIDE + 2,
                                (row * 2 + frame // COLUMNS) * STRIDE + 2))
            if frame in (0, 3, 5, 7, 9, 11, 13, 15):
                column = (0, 3, 5, 7, 9, 11, 13, 15).index(frame)
                gallery.alpha_composite(paint, (column * CELL, row * CELL))
    output = run / RUNTIME.name
    sheet.save(output)
    gallery.convert("RGB").save(run / "timeline.jpg", quality=94)
    if RUNTIME.is_file():
        backup = Path("art/pipeline/backups") / args.run
        backup.mkdir(parents=True, exist_ok=False)
        shutil.copy2(RUNTIME, backup / RUNTIME.name)
    staging = RUNTIME.with_suffix(".tmp")
    shutil.copy2(output, staging)
    staging.replace(RUNTIME)
    (run / "report.json").write_text(json.dumps({
        "synfig": reports, "render_s": time.perf_counter() - start,
        "atlas": str(output), "sha256": synfig_cli.sha(output), "frames": FRAMES * 2,
    }, ensure_ascii=False, indent=2), encoding="utf-8")
    print(output)


if __name__ == "__main__":
    main()

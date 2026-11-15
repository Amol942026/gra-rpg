"""Author the hero's anchored recoil as editable Synfig controls."""

import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

import synfig_cli
from wolf_motion_synfig import control


FPS = 60
FRAMES = 20
TRACKS = (
    ("Odchylenie tułowia", .95, (.86, .61, .32),
     ((0, 0), (1, 9.5), (3, 21), (6, 26.5), (8, 16), (11, 3.8), (14, 1), (18, .18), (20, 0))),
    ("Ułożenie głowy", 3.2, (.63, .76, .85),
     ((0, 0), (2, -.45), (4, -4.8), (7, -6.4), (10, -2.3), (13, -.7), (17, -.12), (20, 0))),
    ("Ugięcie kolan", 7.5, (.83, .49, .39),
     ((0, 0), (2, .32), (4, 1.45), (8, 3.9), (10, 3.46), (13, 1.44), (17, .32), (20, 0))),
)


def samples(keys):
    """Shape-preserving cubic transitions retain smooth velocity between marks."""
    widths = [b[0] - a[0] for a, b in zip(keys, keys[1:])]
    rates = [(b[1] - a[1]) / width for a, b, width in zip(keys, keys[1:], widths)]
    tangents = [0.0]
    for w0, w1, r0, r1 in zip(widths, widths[1:], rates, rates[1:]):
        a, b = w0 + 2 * w1, 2 * w0 + w1
        tangents.append((a + b) / (a / r0 + b / r1) if r0 * r1 > 0 else 0.0)
    tangents.append(0.0)
    for (start, v0), (end, v1), width, t0, t1 in zip(keys, keys[1:], widths, tangents, tangents[1:]):
        for frame in range(start, end):
            u = (frame - start) / width
            u2, u3 = u * u, u * u * u
            yield ((2 * u3 - 3 * u2 + 1) * v0 + (u3 - 2 * u2 + u) * width * t0
                   + (-2 * u3 + 3 * u2) * v1 + (u3 - u2) * width * t1)
    yield keys[-1][1]


def author(path):
    canvas = ET.Element("canvas", version="1.2", width="240", height="196", fps=str(FPS), **{
        "view-box": "-34 42 34 -13.5", "antialias": "2", "begin-time": "0f",
        "end-time": f"{FRAMES}f", "bgcolor": "0 0 0 0",
    })
    ET.SubElement(canvas, "name").text = "Emberfall / Wędrowiec / Reakcja na trafienie"
    ET.SubElement(canvas, "desc").text = (
        "Krzywe odchylenia tułowia, ułożenia głowy i ugięcia kolan przy zakotwiczonych stopach."
    )
    for index, (label, scale, color, marks) in enumerate(TRACKS):
        row = 31 - index * 17
        control(canvas, label, [(scale * value, row) for value in samples(marks)], 2.5, color)
    ET.indent(canvas)
    with path.open("xb") as stream:
        ET.ElementTree(canvas).write(stream, encoding="utf-8", xml_declaration=True)


def export_header(source, path):
    layers = ET.parse(source).getroot().findall("layer")
    tracks = []
    for layer, (label, scale, _, _) in zip(layers, TRACKS, strict=True):
        waypoints = layer.findall("param[@name='origin']/animated/waypoint")
        if layer.get("desc") != label or [point.get("time") for point in waypoints] != [f"{frame}f" for frame in range(FRAMES + 1)]:
            raise ValueError(f"Invalid Synfig track for '{label}'")
        tracks.append([float(point.findtext("vector/x")) / scale for point in waypoints])
    rows = list(zip(*tracks, strict=True))
    if any(rows[0]) or any(rows[-1]):
        raise ValueError("The Synfig reaction must begin and end at the base pose.")
    lines = ["#pragma once", "// Generated from native Synfig waypoints by tools/media/hero_hit_reaction_synfig.py.",
             "namespace ember::hero_hit_reaction::data {", "struct Key {float lean,head,drop;};",
             f"inline constexpr int Frames={FRAMES};", f"inline constexpr float Duration=Frames/{FPS}.0f;",
             "inline constexpr Key samples[]={"]
    for row in rows:
        lines.append(" {" + ", ".join(f"{value:.6f}f" for value in row) + "},")
    lines.extend(("};", "}"))
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    args.root.mkdir(parents=True)
    source = args.root / "hero-hit-reaction.sif"
    author(source)
    receipt = synfig_cli.render(source, output_root=args.root / "renders", threads=2,
                                expected_frames=FRAMES + 1, require_motion=True)
    export_header(source, args.header)
    receipt.update(DurationSeconds=FRAMES / FPS, ControlSamples=FRAMES + 1,
                   HeaderSha256=synfig_cli.sha(args.header))
    (args.root / "report.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

"""Build and export the Obsydianowy Wojownik fragment-motion controls."""
import argparse
import json
from pathlib import Path
import xml.etree.ElementTree as ET

from wolf_motion_synfig import control
import synfig_cli


# 22 frames until the armor fragments have all reached the ground. X is
# outward/downward travel, Y is vertical lift, with a short second impact.
WAYPOINTS = (
    (0, 0.00, 0.00, 0.00, 0.00),
    (3, 0.08, 0.42, 0.10, 0.25),
    (7, 0.27, 1.00, 0.39, 0.48),
    (12, 0.62, 0.64, 0.74, 0.75),
    (16, 0.92, 0.00, 0.95, 1.00),
    (18, 0.97, 0.11, 0.975, 0.96),
    (20, 1.00, 0.00, 1.00, 1.00),
    (22, 1.00, 0.00, 1.00, 1.00),
)


def curves():
    for a, b in zip(WAYPOINTS, WAYPOINTS[1:]):
        for frame in range(a[0], b[0]):
            t = (frame-a[0])/(b[0]-a[0])
            yield tuple(x+(y-x)*t for x, y in zip(a[1:], b[1:]))
    yield WAYPOINTS[-1][1:]


def author(path):
    rows = list(curves())
    canvas = ET.Element("canvas", version="1.2", width="420", height="160", fps="52",
                        **{"view-box": "0 45 118 0", "antialias": "2", "begin-time": "0f",
                           "end-time": f"{len(rows)-1}f", "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / Rozpad obsydianowego pancerza"
    control(canvas, "Przesunięcie i odskok odłamka",
            [(8+100*travel, 12+23*lift) for travel, lift, _, _ in rows], 2.4, (.55,.77,.83))
    control(canvas, "Obrót i położenie na podłożu",
            [(8+100*spin, 5+24*flatten) for _, _, spin, flatten in rows], 1.8, (.78,.63,.45))
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    return len(rows)


def export(path, header):
    lanes = [layer.findall("param[@name='origin']/animated/waypoint")
             for layer in ET.parse(path).findall("layer")]
    rows = []
    for move, turn in zip(*lanes, strict=True):
        values = [(float(node.findtext("vector/"+axis))-origin)/size
                  for node, axis, origin, size in (
                      (move,"x",8,100), (move,"y",12,23), (turn,"x",8,100), (turn,"y",5,24))]
        rows.append(" {"+",".join(f"{max(0,min(1,v)):.5f}f" for v in values)+"},")
    header.write_text("\n".join((
        "#pragma once", "// Waypoints exported by tools/media/sentinel_shatter_synfig.py.",
        "namespace sentinel_death_art::motion {", "struct Key {float travel,lift,spin,flatten;};",
        "inline constexpr Key frames[]={", *rows, "};", "}", "",
    )), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--header", required=True, type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    path = args.root/"rozpad-obsydianowego-pancerza.sif"
    frames = author(path)
    receipt = synfig_cli.render(path, output_root=args.root/"renders", threads=2,
                                expected_frames=frames, require_motion=True)
    export(path, args.header)
    (args.root/"render-receipt.json").write_text(json.dumps(receipt,indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

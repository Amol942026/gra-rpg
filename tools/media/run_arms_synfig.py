"""Author the running-arm controls in Synfig and export the sampled cycle."""
import argparse
import xml.etree.ElementTree as ET
from pathlib import Path

import hero_synfig

FRAMES = 48
CHANNELS = ("swing", "flex", "tuck", "shoulder")
PREVIEW_SCALE = .06
KEYS = (
    [-33, -28, -5, 42, 76, 50, 8, -20, -33],
    [48, 46, 49, 61, 54, 69, 63, 54, 48],
    # Negative tuck angles open the elbows away from the torso.
    [-9, -10, -12, -13, -14, -13, -12, -10, -9],
    [-.75, -.7, -.25, 2.25, 5.45, 3, .65, -.35, -.75],
)


def sample_track(keys):
    # Periodic monotone Hermite tangents carry velocity through intermediate
    # waypoints. The motion eases only where the direction actually changes.
    count = len(keys) - 1
    differences = [keys[i + 1] - keys[i] for i in range(count)]
    tangents = [2 * a * b / (a + b) if a * b > 0 else 0
                for a, b in zip(differences[-1:] + differences[:-1], differences)]
    samples = []
    for frame in range(FRAMES + 1):
        position = frame * count / FRAMES
        segment = min(int(position), count - 1)
        t = position - segment
        a, b = keys[segment:segment + 2]
        m, n = tangents[segment], tangents[(segment + 1) % count]
        samples.append((1 - t) ** 2 * ((1 + 2 * t) * a + t * m)
                       + t ** 2 * ((3 - 2 * t) * b + (t - 1) * n))
    return samples


def author(path):
    tracks = [sample_track(keys) for keys in KEYS]
    canvas = ET.Element("canvas", version="1.2", width="320", height="256",
                        **{"view-box": "-2.5 2 2.5 -2", "fps": "60",
                           "begin-time": "0f", "end-time": f"{FRAMES-1}f",
                           "bgcolor": "0 0 0 0"})
    ET.SubElement(canvas, "name").text = "Emberfall / bieg / prowadzenie łokci"
    ET.SubElement(canvas, "desc").text = (
        "Wymach, zgięcie, odchylenie łokcia na zewnątrz i wysunięcie barku. Dwa widoki wycinanki 2D. "
        "Druga ręka w grze jest przesunięta o pół cyklu.")
    side = hero_synfig.group(canvas, "Wymach i zgięcie z boku")
    hero_synfig.image(side, "body", (-1.9, 1.1, -.6, -.6))
    arm = hero_synfig.group(side, "runtime:shoulder")
    origin = side.find("layer[@desc='runtime:shoulder']/param[@name='origin']")
    origin.clear()
    origin.set("name", "origin")
    path_keys = ET.SubElement(origin, "animated", type="vector")
    for frame, value in enumerate(tracks[3]):
        waypoint = ET.SubElement(path_keys, "waypoint", time=f"{frame}f",
                                 before="linear", after="linear")
        vector = ET.SubElement(waypoint, "vector")
        ET.SubElement(vector, "x").text = f"{value * PREVIEW_SCALE:.7f}"
        ET.SubElement(vector, "y").text = "0"
    hero_synfig.image(arm, "arm", (-1.42, .87, -1.02, -.03))
    forearm = hero_synfig.group(arm, "Przedramię")
    hero_synfig.image(forearm, "arm", (-1.43, .07, -1.00, -.86))
    hero_synfig.rotate(forearm, "flex", (-1.22, .02), tracks[1])
    hero_synfig.rotate(arm, "swing", (-1.22, .83), tracks[0])
    front = hero_synfig.group(canvas, "Prowadzenie łokcia z przodu")
    hero_synfig.image(front, "body", (.55, 1.1, 1.84, -.6))
    near_arm = hero_synfig.group(front, "Łokieć odchylony na zewnątrz")
    hero_synfig.image(near_arm, "arm", (1.65, .92, 2.06, -.48))
    hero_synfig.rotate(near_arm, "tuck", (1.83, .86), [-v for v in tracks[2]])
    for layer in canvas.iter("layer"):
        filename = layer.find("param[@name='filename']/string")
        if filename is not None:
            filename.text = "motion/" + filename.text
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)


def export(project, header):
    root = ET.parse(project).getroot()
    tracks = {}
    for layer in root.iter("layer"):
        label = layer.get("desc", "")
        if not label.startswith("runtime:"):
            continue
        name = label.removeprefix("runtime:")
        if name == "shoulder":
            frames = layer.findall("param[@name='origin']/animated/waypoint")
            samples = {int(w.get("time")[:-1]): float(w.findtext("vector/x")) / PREVIEW_SCALE
                       for w in frames}
        else:
            frames = layer.findall("param[@name='amount']/animated/waypoint")
            sign = -1 if name == "tuck" else 1
            samples = {int(w.get("time")[:-1]): sign * float(w.find("angle").get("value"))
                       for w in frames}
        assert samples.keys() == set(range(FRAMES + 1))
        tracks[name] = [samples[i] for i in range(FRAMES + 1)]
        assert tracks[name][0] == tracks[name][-1]
    assert tracks.keys() == set(CHANNELS)
    rows = [", ".join(f"{v:.7f}f" for v in row) for row in zip(*(tracks[c] for c in CHANNELS))]
    header.write_text(
        '#pragma once\n// Exported from art/hero/run-arms.sif by tools/media/run_arms_synfig.py.\n'
        'namespace ember::hero_motion::run_synfig {\n'
        'struct Key {float swing,flex,tuck,shoulder;};\n'
        'inline constexpr Key keys[]={\n' + '\n'.join(f'    {{{row}}},' for row in rows)
        + '\n};\n}\n', encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--author", action="store_true")
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    if args.author:
        author(args.project)
    export(args.project, args.header)

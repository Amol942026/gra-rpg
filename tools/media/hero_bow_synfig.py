"""Export the synchronized front and side wrist guides of the bow animation."""
import argparse
import math
from pathlib import Path
import xml.etree.ElementTree as ET


def export(source, target):
    canvas = ET.parse(source)
    tracks = {}
    names = {"Grip", "Nock", "Loose", "LoadHand", "LoadGrip", "ArrowNock", "ArrowTip"}
    guide_names = {stem + view for stem in names for view in ("Front", "Side")}
    for layer in canvas.iter("layer"):
        name = layer.get("desc", "")
        if name not in guide_names:
            continue
        tracks[name] = {
            int(w.get("time")[:-1]): (float(w.findtext("vector/x")), float(w.findtext("vector/y")))
            for w in layer.findall("param[@name='origin']/animated/waypoint")
        }

    def guide(name, f):
        x, z = tracks[name + "Front"][f]
        forward, side_z = tracks[name + "Side"][f]
        if z != side_z:
            raise ValueError(f"Front and side height differ at {name} {f}")
        return x, forward - (56 if "LoadHandFront" in tracks else 32), z

    def xyz(v):
        return "{" + ",".join(f"{n:.4f}f" for n in v) + "}"

    if "LoadHandFront" in tracks:
        lines = ["#pragma once", "// Exported from art/hero/traveler-performance/bow-nock.sif.",
                 "namespace ember::hero_bow::data {",
                 "struct LoadKey {hero_motion::Vec grip,hand,axis;float gripOffset;};",
                 "inline constexpr LoadKey prepare[]={"]
        frames = sorted(tracks["LoadHandFront"])
        if frames != list(range(len(frames))):
            raise ValueError("Expected a sampled wrist pose on every Synfig frame.")
        for frame in frames:
            grip, hand, rear, tip = (guide(name, frame) for name in ("LoadGrip", "LoadHand", "ArrowNock", "ArrowTip"))
            axis = tuple((t - r) / 36 for t, r in zip(tip, rear))
            if abs(math.hypot(*axis) - 1) > .001:
                raise ValueError(f"Arrow length changes on frame {frame}.")
            offset = min(11, max(0, sum((h - r) * a for h, r, a in zip(hand, rear, axis))))
            lines.append(f" {{{xyz(grip)},{xyz(hand)},{xyz(axis)},{offset:.4f}f}},")
        target.write_text("\n".join(lines + ["};", "}", ""]), encoding="utf-8")
        return

    lines = ["#pragma once", "// Exported from art/hero/traveler-performance/bow-arms.sif.",
             "namespace ember::hero_bow::data {",
             "struct DrawKey {float t;hero_motion::Vec grip,nock;};",
             "struct LooseKey {float t;hero_motion::Vec delta;};",
             "inline constexpr DrawKey draw[]={"]
    draw = sorted(tracks["GripFront"])
    for f in draw:
        lines.append(f" {{{f / draw[-1]:.8f}f,{xyz(guide('Grip', f))},{xyz(guide('Nock', f))}}},")
    lines.extend(["};", "inline constexpr LooseKey loose[]={"])
    loose = sorted(tracks["LooseFront"])
    anchor = guide("Loose", loose[0])
    for f in loose:
        delta = [v - a for v, a in zip(guide("Loose", f), anchor)]
        t = (f - loose[0]) / (loose[-1] - loose[0])
        lines.append(f" {{{t:.8f}f,{xyz(delta)}}},")
    target.write_text("\n".join(lines + ["};", "}", ""]), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("target", type=Path)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    export(args.source, args.target)

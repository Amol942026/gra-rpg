"""Bake Varkhor's ash textures and export the disintegration curves from Synfig."""

from pathlib import Path
import argparse
import hashlib
import json
import random
import xml.etree.ElementTree as ET

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

from wolf_motion_synfig import control
import synfig_cli


FRAMES = 45
AIR_FRAMES = 60
TRACKS = (
    ("Rozpad", ((0, 0), (.03, 0), (.10, .075), (.25, .23), (.45, .44),
                (.63, .66), (.79, .87), (.89, 1), (1, 1))),
    ("Osunięcie", ((0, 0), (.04, 0), (.16, .018), (.31, .09), (.45, .24),
                   (.62, .48), (.76, .70), (.90, 1), (1, 1))),
    ("Obłok", ((0, 0), (.04, 0), (.11, .10), (.23, .38), (.44, .89),
               (.54, 1), (.62, .88), (.72, .62), (.81, .25), (.90, 0), (1, 0))),
    ("Osad", ((0, 0), (.18, 0), (.31, .12), (.46, .49), (.61, .87),
              (.72, .96), (.88, 1), (1, 1))),
    ("Rozpad tułowia", ((0, 0), (.09, 0), (.20, .12), (.30, .38),
                        (.39, .68), (.48, .94), (.54, 1), (1, 1))),
)
AIR_TRACKS = (
    ("Krycie opadającego pyłu", ((0, 0), (5, 0), (12, .45), (22, .98), (28, 1),
                                (38, .78), (48, .49), (55, .19), (60, 0))),
    ("Rozchodzenie pyłu", ((0, 0), (5, 0), (18, .61), (29, .95), (40, 1), (60, 1))),
    ("Opadanie pyłu", ((0, 0), (5, 0), (13, .12), (21, .33),
                       (28, .58), (37, .88), (44, 1), (60, 1))),
)


def tangents(keys):
    # Shape-preserving tangents keep movement continuous through the timing
    # marks and come to rest along the held portions of a track.
    widths = [x1-x0 for (x0, _), (x1, _) in zip(keys, keys[1:])]
    speeds = [(b[1]-a[1])/width for a, b, width in zip(keys, keys[1:], widths)]
    slopes = [0.]
    for h0, h1, v0, v1 in zip(widths, widths[1:], speeds, speeds[1:]):
        w0, w1 = h0+2*h1, 2*h0+h1
        slopes.append((w0+w1)/(w0/v0+w1/v1) if v0*v1 > 0 else 0.)
    return slopes + [0.]


def sample(keys, slopes, phase):
    for (x0, y0), (x1, y1), m0, m1 in zip(keys, keys[1:], slopes, slopes[1:]):
        if phase <= x1:
            width = x1-x0
            p = (phase-x0)/width
            p2, p3 = p*p, p*p*p
            return ((2*p3-3*p2+1)*y0 + (p3-2*p2+p)*width*m0
                    + (-2*p3+3*p2)*y1 + (p3-p2)*width*m1)
    return keys[-1][1]


def noise(width, height, density, rng):
    sample = Image.fromarray(rng.integers(0, 256, (density, density), dtype=np.uint8))
    return np.asarray(sample.resize((width, height), Image.Resampling.BICUBIC), dtype=np.float64)/255-.5


# A rising curl, a low drift, a splayed edge, and a dense rolling core.
# Each lobe is positioned and stretched separately to break up the silhouette.
BILLOWS = (
    ((15, 43, 8, 6, .54), (25, 38, 11, 8, .72), (28, 29, 8, 11, .73),
     (35, 19, 9, 8, .79), (44, 18, 6, 6, .51), (42, 29, 8, 9, .65),
     (35, 40, 10, 7, .52)),
    ((11, 38, 8, 5, .43), (20, 34, 12, 6, .69), (29, 33, 10, 10, .75),
     (38, 34, 11, 8, .70), (48, 29, 9, 5, .48), (54, 36, 5, 4, .36),
     (44, 41, 8, 4, .42)),
    ((17, 22, 7, 5, .45), (24, 27, 8, 9, .67), (31, 37, 10, 11, .75),
     (40, 32, 9, 7, .58), (48, 25, 8, 6, .48), (50, 41, 8, 6, .40),
     (38, 46, 10, 6, .52)),
    ((16, 35, 8, 9, .59), (24, 24, 10, 9, .65), (37, 20, 10, 8, .68),
     (47, 27, 8, 8, .66), (44, 39, 10, 8, .69), (32, 45, 10, 6, .55),
     (28, 35, 12, 11, .75)),
)


def ash_billows(rng):
    y, x = np.mgrid[:64, :64]
    padding = np.minimum.reduce((x, y, 63-x, 63-y))
    for lobes, opacity_scale in zip(BILLOWS, (.78, .98, .87, .66)):
        eddies = noise(64, 64, 7, rng)
        turbulence = noise(64, 64, 13, rng)
        grit = noise(64, 64, 26, rng)
        warp_x = x + 6 * eddies + 2 * grit
        warp_y = y + 5 * turbulence - 2 * grit
        transmittance = np.ones(x.shape)
        for px, py, rx, ry, opacity in lobes:
            radius = ((warp_x-px)/rx)**2 + ((warp_y-py)/ry)**2
            transmittance *= 1 - opacity*np.exp(-.85*radius**1.25)
        density = 1 - transmittance
        density *= 1 + .28*eddies + .21*turbulence + .17*grit
        density = np.clip(density, 0, 1)
        alpha = np.clip((density-.033)/.85, 0, 1)
        alpha = alpha**1.16 * opacity_scale * np.clip((padding-1)/4, 0, 1)
        slope_y, slope_x = np.gradient(density)
        rim = np.clip((slope_x + .7*slope_y)*220, -26, 24)
        tone = np.clip(146 + 63*density + rim + 34*eddies + 21*turbulence
                       + 20*grit + (31-y)*.38, 108, 236)
        pixels = np.dstack((tone, tone*.961, tone*.938, alpha*255)).astype(np.uint8)
        yield Image.fromarray(pixels, "RGBA").filter(ImageFilter.GaussianBlur(.27))


def ash_atlas(path):
    rng = np.random.default_rng(4112)
    y, x = np.mgrid[:128, :256]
    xx, yy = x-127, y-86
    broad = noise(256, 128, 12, rng)
    fines = noise(256, 128, 51, rng)
    grain = rng.random((128, 256))-.5
    surface = np.zeros_like(xx, dtype=np.float64)
    for px, py, spread_x, spread_y, peak in (
        (-18, -10, 37, 16, 1), (28, -4, 35, 16, .83),
        (-57, 5, 26, 9, .45), (64, 7, 27, 9, .38), (0, 10, 48, 10, .34),
    ):
        surface += peak*np.exp(-.5*(((xx-px)/spread_x)**2+((yy-py)/spread_y)**2))
    contour = surface + broad*.38 + fines*.11
    edge = np.clip((contour-(.22 + grain*.09))/.12, 0, 1)
    edge *= np.clip(np.minimum.reduce((x, y, 255-x, 127-y))/10, 0, 1)
    slope_y, slope_x = np.gradient(surface*.8 + broad*.17 + fines*.07)
    crest = np.clip((surface-.15)/1.24, 0, 1)
    shade = 42 + 46*crest - 1300*slope_y - 830*slope_x + 28*broad + 27*fines + 30*grain
    shade = np.clip(shade, 21, 142)
    body = np.dstack((shade*1.03+6, shade*.97+2, shade*.93+4, 255*edge)).clip(0,255).astype(np.uint8)
    pile = Image.fromarray(body, "RGBA")
    grit = Image.new("RGBA", pile.size)
    paint = ImageDraw.Draw(grit)
    scatter = random.Random(4112)
    for _ in range(1300):
        px, py = scatter.randrange(256), scatter.randrange(128)
        if edge[py, px] < .83:
            continue
        value = int(shade[py, px])+scatter.randint(-21, 32)
        size = scatter.choices((1, 2, 3), (7, 4, 1))[0]
        tint = (value+11, value+5, value+2, scatter.randint(80, 200))
        paint.ellipse((px, py, px+size, py+max(1, size//2)), fill=tint)
    # A handful of cold ochre sparks distinguish the ash from normal terrain.
    for px, py in ((86, 79), (122, 75), (135, 89), (162, 92), (173, 71), (187, 83)):
        paint.point((px-1, py-1), (204, 137, 78, 187))
        paint.ellipse((px, py, px+3, py+1), (119, 51, 26, 147))
    pile = Image.alpha_composite(pile, grit)
    atlas = Image.new("RGBA", (256, 192))
    atlas.paste(pile, (0, 0))
    for i, puff in enumerate(ash_billows(rng)):
        atlas.paste(puff, (i*64, 128))
    atlas.save(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--no-audio", required=True, action="store_true")
    args = parser.parse_args()
    args.root.mkdir(parents=True)
    path = args.root/"varkhor-ash.sif"
    atlas = args.root/"varkhor-ash.png"
    ash_atlas(atlas)
    canvas = ET.Element("canvas", version="1.2", width="384", height="388", fps="60", **{
        "view-box": "-72 55 72 -90", "antialias": "2", "begin-time": "0f", "end-time": f"{AIR_FRAMES-1}f",
        "bgcolor": "0 0 0 0",
    })
    ET.SubElement(canvas, "name").text = "Emberfall / Varkhor / Rozpad w pył"
    colors = ((.95, .36, .18), (.61, .42, .30), (.72, .65, .59), (.55, .54, .48), (.93, .64, .26),
              (.82, .77, .69), (.40, .63, .84), (.47, .75, .67))
    for index, (name, keys) in enumerate(TRACKS + AIR_TRACKS):
        frames = FRAMES if index < len(TRACKS) else AIR_FRAMES
        if index >= len(TRACKS):
            keys = tuple((frame/frames, value) for frame, value in keys)
        slopes = tangents(keys)
        values = [(-60+sample(keys, slopes, frame/frames)*120, 45-index*17)
                  for frame in range(frames+1)]
        control(canvas, name, values, 3.3, colors[index])
    ET.indent(canvas)
    ET.ElementTree(canvas).write(path, encoding="utf-8", xml_declaration=True)
    layers = ET.parse(path).getroot().findall("layer")
    tracks = [[float(w.findtext("vector/x"))/120+.5
               for w in layer.findall("param[@name='origin']/animated/waypoint")] for layer in layers]
    body_tracks, air_tracks = tracks[:len(TRACKS)], tracks[len(TRACKS):]
    assert all(len(track) == FRAMES+1 for track in body_tracks)
    assert all(len(track) == AIR_FRAMES+1 for track in air_tracks)
    receipt = synfig_cli.render(path,output_root=args.root/"renders",threads=2,
                                expected_frames=AIR_FRAMES,require_motion=True)
    lines = ["#pragma once", "// Motion samples exported from the saved Synfig source.",
             "namespace varkhor_ash_motion {",
             "struct Key {float dissolve,sink,cloud,pile,torso;};",
             "struct AirKey {float opacity,spread,settle;};",
             f"inline constexpr int Frames={FRAMES},AirFrames={AIR_FRAMES};",
             "inline constexpr int FramesPerSecond=60;"]
    for key, name, channels in (("Key", "samples", body_tracks), ("AirKey", "airSamples", air_tracks)):
        lines.append(f"inline constexpr {key} {name}[]={{")
        for frame in zip(*channels):
            lines.append(" {"+", ".join(f"{value:.6f}f" for value in frame)+"},")
        lines.append("};")
    lines.append("}")
    args.header.write_text("\n".join(lines)+"\n", encoding="utf-8")
    receipt["texture_sha256"] = hashlib.sha256(atlas.read_bytes()).hexdigest()
    (args.root/"report.json").write_text(json.dumps(receipt,ensure_ascii=False,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main()

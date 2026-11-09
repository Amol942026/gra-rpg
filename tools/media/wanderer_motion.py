"""Author and render the wanderer's new cloth follow-through in Synfig."""
import argparse
import hashlib
import json
import shutil
from pathlib import Path

from PIL import Image, ImageDraw

import hero_synfig
import synfig_cli


CLOTH = {
    "idle": ([0, .45, 1.1, 1.65, 1, 0, -.75, -.65, 0],
             [0, .18, .5, .75, .38, 0, -.34, -.25, 0]),
    "walk": ([-1.5, 0, 3.3, 5, 2.1, -.2, -3.2, -4.5, -1.5],
             [.25, 1.5, 3.2, 2.3, .65, 1.5, 3.2, 2.3, .25]),
    "run": ([-2, -.4, 4.6, 7.9, 3.4, 0, -4.6, -7.9, -2],
            [1.5, 2.5, 5.5, 7, 4.3, 3, 5.8, 7.1, 1.5]),
    "slash": ([0, 0, -1.5, -4.5, -5.2, .8, 5.7, 3.5, 0],
              [0, 0, -.3, -1.3, 1.9, 5.7, 3.3, 1, 0]),
    "heavy": ([0, -.8, -2.9, -6.7, -5.8, 2.4, 8.1, 4.1, 0],
              [0, -.4, -1.4, -2.9, 2.5, 8.4, 5.3, 1.2, 0]),
    "roll": ([0, -1.2, -4.6, -6.7, -5.9, -1.8, 2.9, 1.9, 0],
             [0, -1.8, -5.2, -8.2, -7.7, -3.3, 2.2, 2.8, 0]),
}


def cutout(source, polygon):
    mask = Image.new("L", source.size)
    ImageDraw.Draw(mask).polygon(polygon, fill=255)
    part = source.copy()
    part.putalpha(mask)
    return part.crop(mask.getbbox())


def preview_parts(source, atlas, sword, directory):
    directory.mkdir(parents=True)
    with Image.open(source) as image:
        model = image.convert("RGBA")
    silhouettes = {
        "face": [(231, 67), (245, 56), (272, 61), (282, 76), (277, 94), (266, 107),
                 (251, 109), (239, 97), (232, 83)],
        "hair_front": [(219, 69), (222, 53), (233, 41), (249, 31), (266, 31), (279, 42),
                       (288, 56), (286, 75), (277, 69), (271, 62), (253, 67), (239, 71), (231, 78)],
        "scarf": [(217, 93), (234, 101), (249, 127), (259, 133), (282, 100), (298, 113),
                  (291, 142), (258, 173), (228, 154), (210, 127)],
        "body": [(205, 125), (231, 126), (255, 148), (278, 126), (306, 135), (298, 246),
                 (315, 322), (276, 343), (253, 324), (224, 340), (192, 319), (216, 247)],
        "arm": [(184, 121), (201, 130), (221, 153), (215, 194), (199, 247), (193, 278),
                (196, 304), (188, 317), (173, 308), (169, 284), (171, 238), (166, 190)],
        "leg": [(205, 302), (241, 311), (248, 351), (239, 384), (238, 419), (232, 459),
                (218, 480), (202, 475), (198, 459), (203, 432), (198, 392), (197, 355)],
    }
    for label, points in silhouettes.items():
        cutout(model, points).save(directory / f"{label}.png")
    with Image.open(atlas) as image:
        cloak = image.crop((384, 128, 512, 256)).convert("RGBA")
        cloak = Image.merge("RGBA", tuple(channel.point(lambda x, scale=s: int(x*scale))
                                          for channel, s in zip(cloak.split(), (.26, .51, .53, 1))))
    cutout(cloak, [(35, 3), (91, 3), (103, 38), (123, 110), (104, 121), (85, 115),
                  (67, 123), (44, 114), (26, 119), (7, 112), (25, 43)]).save(directory / "cape.png")
    shutil.copy2(sword, directory / "sword.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--flux", type=Path, required=True)
    parser.add_argument("--atlas", type=Path, required=True)
    parser.add_argument("--synfig", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True, exist_ok=False)
    repo = Path(__file__).resolve().parents[2]
    parts = args.root / "cutouts"
    preview_parts(args.flux, args.atlas, repo / "art/hero/motion/parts/sword.png", parts)
    clips = {name: (frames, loop, [*keys[:4], *CLOTH[name]])
             for name, (frames, loop, keys) in hero_synfig.CLIPS.items()}
    projects = hero_synfig.author(args.root / "projects", parts, clips)
    header = args.root / "hero_secondary_data.hpp"
    receipts = hero_synfig.compile_tracks(projects, header)
    synfig_cli.SYNFIG = args.synfig
    for project, receipt in zip(projects, receipts):
        result = synfig_cli.render(project, output_root=args.root / "renders", threads=2,
                                   expected_frames=clips[project.stem][0], require_motion=True)
        receipt["render"] = result
        print(f"Rendered {project.stem}: {clips[project.stem][0]} frames", flush=True)
    report = {"status": "PASS", "clips": receipts, "header_sha256": hashlib.sha256(header.read_bytes()).hexdigest(),
              "frames": sum(clip[0] for clip in clips.values()), "audio_played": False}
    (args.root / "motion-receipt.json").write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()

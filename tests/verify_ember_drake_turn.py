"""Compare hidden in-game renders of Caldera creatures before and after a turn redesign."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
from statistics import mean

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageSequence


def summarize(folder):
    entries = {}
    with (folder / "turn-samples.csv").open(encoding="utf-8", newline="") as file:
        for row in csv.DictReader(file):
            entries.setdefault(row["pose"], []).append(row)
    result = {}
    for pose, rows in entries.items():
        assert len(rows) == 361
        values = [float(row["pixel_delta"]) for row in rows[1:]]
        result[pose] = {
            "changed_degree_steps": sum(int(row["changed"]) for row in rows[1:]),
            "max_delta_per_visible_pixel": round(max(values), 4),
            "metric": "absolute mask alpha difference" if pose == "shadow" else "mean absolute RGB difference",
            "worst_from_degree": int(rows[values.index(max(values))]["degree"]),
            "mean_visible_pixels": round(mean(int(row["visible_pixels"]) for row in rows[:-1]), 2),
            "width_east": int(rows[0]["width"]),
            "height_east": int(rows[0]["height"]),
        }
    return result


def save_preview(directory, output, minimum_visible=3000):
    files = sorted((directory / "turn-walk").glob("*.png"))
    assert len(files) == 66
    images = []
    bounds = []
    for filename in files:
        with Image.open(filename) as image:
            images.append(image.convert("RGB"))
        array = np.asarray(images[-1])
        yy, xx = np.where(np.any(np.abs(array.astype(int)-array[0, 0]) >= 4, axis=2))
        assert xx.size > minimum_visible
        bounds.append((xx.min(), yy.min(), xx.max()+1, yy.max()+1))
    bounds = np.array(bounds)
    left, top = bounds.min(axis=0)[:2] - 3
    right, bottom = bounds.max(axis=0)[2:] + 3
    w, h = images[0].size
    assert 0 <= left < right <= w and 0 <= top < bottom <= h
    images = [frame.crop((left, top, right, bottom)) for frame in images]
    # Share one palette across frames so fine scale highlights keep their hue.
    thumbnails = Image.new("RGB", (images[0].width*6, images[0].height*3))
    for i, frame in enumerate(images[::4]):
        thumbnails.paste(frame, (i % 6 * frame.width, i // 6 * frame.height))
    palette = thumbnails.quantize(colors=256)
    indexed = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in images]
    durations = [10 * (round((i+1)*100/30)-round(i*100/30)) for i in range(len(indexed))]
    indexed[0].save(output, save_all=True, append_images=indexed[1:], duration=durations,
                    loop=0, disposal=1, optimize=False)
    with Image.open(output) as gif:
        assert gif.n_frames == len(indexed)
        assert gif.size == images[0].size
        decoded = np.stack([np.asarray(frame.convert("RGB")) for frame in ImageSequence.Iterator(gif)]).astype(np.int16)
    differences = np.abs(decoded - np.roll(decoded, 1, axis=0)).mean(axis=(1,2,3))
    assert min(differences) > 0 and differences[0] <= max(differences[1:])*1.1
    return {"frames": len(indexed), "duration_ms": sum(durations), "size": list(images[0].size),
            "loop_seam_vs_mean_frame_change": round(float(differences[0]/differences[1:].mean()), 3)}


def crawler_keys(baseline, current):
    result = {}
    for index, direction in enumerate(("E", "SE", "S", "SW", "W", "NW", "N", "NE")):
        images = []
        for directory in (baseline, current):
            with Image.open(directory / f"orientation-{index}.png") as image:
                images.append(np.asarray(image.convert("RGBA"), dtype=np.int16))
        old, new = images
        bounds = []
        for pixels in images:
            yy, xx = np.nonzero(pixels[..., 3] > 50)
            assert xx.size and xx.min() > 0 and yy.min() > 0
            assert xx.max() + 1 < pixels.shape[1] and yy.max() + 1 < pixels.shape[0]
            bounds.append([int(xx.min()), int(yy.min()), int(np.ptp(xx)+1), int(np.ptp(yy)+1)])
        visible = (old[..., 3] > 50) | (new[..., 3] > 50)
        result[direction] = {"old_xywh": bounds[0], "new_xywh": bounds[1],
                             "difference_over_union_rgb": round(float(np.abs(old[..., :3] - new[..., :3])[visible].mean()), 3)}
    return result


def detached_silhouette(folder):
    maximum, worst = 0., 0
    for path in sorted(folder.glob("turn-idle-*.png")):
        with Image.open(path) as image:
            alpha = image.getchannel("A")
        # Coverage may fall below 50% on a one-pixel claw. Bridge just one pixel
        # before looking for truly disconnected chunks of the creature.
        filled = alpha.point(lambda a: 255 if a >= 100 else 0).filter(ImageFilter.MaxFilter(3))
        counts = []
        while filled.getbbox():
            y, x = np.argwhere(np.asarray(filled) > 0)[0]
            pixels = filled.histogram()[255]
            ImageDraw.floodfill(filled, (int(x), int(y)), 0)
            counts.append(pixels - filled.histogram()[255])
        assert counts
        lost = 1 - max(counts)/sum(counts)
        if lost > maximum:
            maximum, worst = lost, int(path.stem.rsplit("-",1)[1])
    return {"largest_detached_fraction": round(maximum, 5), "worst_angle": worst}


def check_unchanged_drake(current, reference, output):
    from tempfile import TemporaryDirectory
    regression = current / "drake-regression"
    data = json.loads((regression/"turn-summary.json").read_text(encoding="utf-8"))
    with TemporaryDirectory(dir=output) as folder:
        fresh = Path(folder) / "drake.gif"
        save_preview(regression, fresh)
        with Image.open(fresh) as after, Image.open(reference) as before:
            shape_matches = before.size == after.size and before.n_frames == after.n_frames
            maximum = 255
            if shape_matches:
                maximum = 0
                for i in range(after.n_frames):
                    before.seek(i)
                    after.seek(i)
                    a = np.asarray(after.convert("RGB"), dtype=np.int16)
                    b = np.asarray(before.convert("RGB"), dtype=np.int16)
                    maximum = max(maximum, int(np.abs(a-b).max()))
    return {"checks": data["checks"], "failures": data["failures"], "same_dimensions_and_frames": shape_matches,
            "max_channel_difference": maximum}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("baseline", type=Path)
    parser.add_argument("current", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--creature", choices=("drake", "crawler"), default="drake")
    parser.add_argument("--drake-reference", type=Path, help="Compare the shared renderer with an earlier in-game drake GIF")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    before, after = summarize(args.baseline), summarize(args.current)
    checks = {}
    for pose in before:
        checks[f"{pose}_has_more_angles"] = after[pose]["changed_degree_steps"] >= 3*before[pose]["changed_degree_steps"]
        checks[f"{pose}_smaller_worst_jump"] = (after[pose]["max_delta_per_visible_pixel"] <
                                                before[pose]["max_delta_per_visible_pixel"]*.85)
    crawler = args.creature == "crawler"
    keys = crawler_keys(args.baseline, args.current) if crawler else {}
    if crawler:
        checks["all_eight_original_views_replaced"] = all(v["difference_over_union_rgb"] > 10 for v in keys.values())
        continuity = detached_silhouette(args.current)
        checks["connected_idle_silhouettes"] = continuity["largest_detached_fraction"] < .05
        for axis, name in ((2, "width"), (3, "height")):
            ratios = [v["new_xywh"][axis] / v["old_xywh"][axis] for v in keys.values()]
            # Art can change the contours, but it must keep the original screen scale.
            checks[f"preserved_average_{name}"] = .94 <= mean(ratios) <= 1.06
            checks[f"preserved_eight_{name}s"] = all(.86 <= ratio <= 1.14 for ratio in ratios)
    else:
        for dimension in ("width_east", "height_east"):
            ratio = after["idle"][dimension] / before["idle"][dimension]
            checks[f"larger_{dimension}"] = 1.10 < ratio < 1.21
    captures = {key:json.loads((directory / "turn-summary.json").read_text(encoding="utf-8"))
                for key, directory in (("before", args.baseline), ("after", args.current))}
    checks["runtime_probes_pass"] = all(item["failures"] == 0 for item in captures.values())
    checks["old_art_behaviors_pass"] = json.loads((args.current / "drake-summary.json").read_text())["failures"] == 0
    preview_file = "zuzlowy-pelzacz-obrot.gif" if crawler else "zarowy-drak-obrot.gif"
    preview = save_preview(args.current, args.output/preview_file,
                           int(after["walk"]["mean_visible_pixels"]*.4) if crawler else 3000)
    atlas_folder = args.current / "assets" / "volcano" / f"{args.creature}-turn"
    atlas_hashes = {sheet.name: hashlib.sha256(sheet.read_bytes()).hexdigest()
                    for sheet in sorted(atlas_folder.glob("parts-*.png"))}
    assert atlas_hashes
    report = {"before": before, "after": after, "checks": checks, "renderer": captures,
              "runtime_atlas_sha256": atlas_hashes, "preview": preview}
    if crawler:
        report["eight_canonical_views"] = keys
        report["silhouette_continuity"] = continuity
    if args.drake_reference:
        proof = check_unchanged_drake(args.current, args.drake_reference, args.output)
        report["drake_regression"] = proof
        checks["drake_regression"] = proof["failures"] == 0 and proof["max_channel_difference"] == 0
    (args.output/"turn-report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(report, indent=2))
    if not all(checks.values()):
        raise SystemExit(1)


if __name__ == "__main__":
    main()

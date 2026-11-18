"""Review native captures from hero_gait_turn_qa and make a compact game preview."""

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image


def cells(path, width, height):
    pixels = np.asarray(Image.open(path).convert("RGBA"))
    rows, cols = pixels.shape[0] // height, pixels.shape[1] // width
    return (pixels.reshape(rows, height, cols, width, 4)
            .transpose(0, 2, 1, 3, 4).reshape(-1, height, width, 4))


def composite(pixels):
    alpha = pixels[..., 3:4].astype(np.float32) / 255
    return pixels[..., :3] * alpha + 48 * (1 - alpha)


def difference(pixels, other):
    return np.abs(pixels - other).mean(axis=(1, 2, 3))


def turns(folder):
    turns = {}
    for outfit in ("face", "bare", "helmet", "hood", "closed"):
        cw, ccw = [composite(cells(folder / f"turn-{outfit}-{side}.png", 76, 76))
                   for side in ("cw", "ccw")]
        step = difference(cw, np.roll(cw, 1, axis=0))
        turns[outfit] = {
            "visible_steps_over_0.03": int((step > .03).sum()),
            "changed_steps": int(np.count_nonzero(step)),
            "peak_step_difference": float(step.max()),
            "median_step_difference": float(np.median(step)),
            "wrap_last_first_difference": float(step[0]),
            "largest_step_at_degrees": int(step.argmax()),
            "cw_ccw_max_frame_difference": float(difference(cw, ccw[(-np.arange(360)) % 360]).max()),
        }
    return turns


def rig(folder):
    data = (np.loadtxt(folder / "joints.csv", delimiter=",", skiprows=1)
            .reshape(2, 121, 17, 6)[..., 3:])
    soles = (np.loadtxt(folder / "soles.csv", delimiter=",", skiprows=1)
             .reshape(2, 121, 4)[..., 2:])
    result = {}
    for run, name in enumerate(("walk", "run")):
        points = data[run]
        rigid = []
        for upper, rest_length in ((4, 13), (5, 13), (7, 13), (8, 13),
                                   (10, 14.5), (11, 14.5), (13, 14.5), (14, 14.5)):
            length = np.linalg.norm(points[:, upper] - points[:, upper + 1], axis=1)
            rigid.append(np.abs(length - rest_length).max())
        support = []
        for side in range(2):
            u = (np.arange(120) / 120 + .5 * side) % 1
            contact = u < (.42 if run else .60)
            support.extend(soles[run, :120, side][contact])
        result[name] = {
            "rigid_segment_max_error": float(max(rigid)),
            "grip_max_error": float(np.linalg.norm(points[:, 9] - points[:, 16], axis=1).max()),
            "cycle_seam_max_error": float(np.linalg.norm(points[0] - points[-1], axis=1).max()),
            "stance_sole_max_error": float(np.max(np.abs(support))),
            "lowest_sole": float(soles[run].min()),
            "raised_feet_frames_out_of_120": int((soles[run, :120].min(axis=1) > .05).sum()),
            "pelvis_sway_xyz": np.ptp(points[:, 1], axis=0).tolist(),
            "left_wrist_forward_swing": float(np.ptp(points[:, 6, 1])),
            "right_wrist_forward_swing": float(np.ptp(points[:, 9, 1])),
        }
    return result


def gait_sheets(folder):
    result = {}
    for name in ("walk", "run"):
        for direction in range(8):
            sheet = f"{name}-{direction}"
            frames = cells(folder / f"{sheet}.png", 168, 196)
            mask = frames[..., 3] > 30
            border = (mask[:, 0].any(axis=1) | mask[:, -1].any(axis=1)
                      | mask[:, :, 0].any(axis=1) | mask[:, :, -1].any(axis=1))
            rgb = composite(frames)
            result[sheet] = {
                "cropped_frames": int(border.sum()),
                "empty_frames": int((~mask.any(axis=(1, 2))).sum()),
                "identical_adjacent_frames": int((difference(rgb, np.roll(rgb, 1, axis=0)) == 0).sum()),
            }
    return result


def inspect(folder):
    modes = [composite(cells(folder / f"renderer-{i}.png", 76, 76)) for i in range(3)]
    return {
        "turns": turns(folder),
        "gaits": rig(folder),
        "directional_gait_sheets": gait_sheets(folder),
        "native_paths_vs_fast_max_frame_difference": [
            float(difference(modes[0], alt).max()) for alt in modes[1:]
        ],
        "render_export_seconds": float((folder / "render-seconds.txt").read_text()),
    }


def make_preview(folder, output):
    rgba = cells(folder / "moving-turn.png", 168, 196)[::2]
    mask = np.any(rgba[..., 3] > 15, axis=0)
    y, x = np.nonzero(mask)
    box = (max(0, x.min()-3), max(0, y.min()-3), min(168, x.max()+4), min(196, y.max()+4))
    frames = []
    for tile in rgba:
        sprite = Image.fromarray(tile).crop(box)
        bg = Image.new("RGB", sprite.size, (38, 46, 43))
        bg.paste(sprite, mask=sprite.getchannel("A"))
        bg.thumbnail((138, 145), Image.Resampling.LANCZOS)
        frames.append(bg)
    # One shared palette keeps the painted colours stable between frames.
    strip = Image.new("RGB", (frames[0].width * 10, frames[0].height * 6))
    for i, frame in enumerate(frames):
        strip.paste(frame, (i % 10 * frame.width, i // 10 * frame.height))
    palette = strip.quantize(colors=248, method=Image.Quantize.MEDIANCUT)
    indexed = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in frames]
    indexed[0].save(output, save_all=True, append_images=indexed[1:],
                    duration=[30, 30, 40] * 20, loop=0, optimize=False)
    with Image.open(output) as saved:
        assert saved.n_frames == len(indexed)
        return {"width": saved.width, "height": saved.height, "frames": saved.n_frames,
                "seconds": 2, "bytes": output.stat().st_size}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run", type=Path, help="Run directory with native captures in old/ and new/")
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    old, new = (inspect(args.run / stage) for stage in ("old", "new"))
    old_face, face = old["turns"]["face"], new["turns"]["face"]
    checks = {
        "continuous_face": face["visible_steps_over_0.03"] >= 300,
        "reduced_largest_face_jump": face["peak_step_difference"] < old_face["peak_step_difference"] * .5,
        "signed_turn_symmetry": all(v["cw_ccw_max_frame_difference"] < .08 for v in new["turns"].values()),
        "framebuffer_path_parity": max(new["native_paths_vs_fast_max_frame_difference"]) < .1,
        "uncropped_directional_gaits": all(not any(v.values()) for v in new["directional_gait_sheets"].values()),
        "fixed_lengths_and_grip": all(max(v["grip_max_error"], v["rigid_segment_max_error"],
                                           v["cycle_seam_max_error"]) < .002 for v in new["gaits"].values()),
        "grounded_stance": all(v["stance_sole_max_error"] < .06 and v["lowest_sole"] > -.06
                                for v in new["gaits"].values()),
        "walk_support_and_running_flight": (new["gaits"]["walk"]["raised_feet_frames_out_of_120"] == 0
                                            and new["gaits"]["run"]["raised_feet_frames_out_of_120"] > 0),
    }
    preview = make_preview(args.run / "new", args.run / "ruch-bohatera.gif")
    result = {"before": old, "after": new, "checks": checks, "preview": preview}
    (args.run / "gait-turn-qa.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({"checks": checks, "before_face": old_face, "face": face, "preview": preview}))
    return 0 if all(checks.values()) else 1


if __name__ == "__main__":
    raise SystemExit(main())

"""Read native Synfig contact track, actual projected feet and renderer images."""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from PIL import Image, ImageDraw


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--qa", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    parser.add_argument("--preview", action="store_true")
    args = parser.parse_args()
    project = Path(__file__).resolve().parents[2]
    checks = []

    def check(ok, text):
        checks.append({"pass": bool(ok), "check": text})

    source = project / "art/creatures/motion/humanoid.sif"
    receipt = json.loads((source.parent / "motion-receipt.json").read_text(encoding="utf-8"))
    expected = next(clip for clip in receipt["clips"] if clip["family"] == "humanoid")
    check(hashlib.sha256(source.read_bytes()).hexdigest() == expected["sha256"], "original authored Synfig contact curve preserved")
    track = ET.parse(source).find(".//layer[@desc='runtime:foot']/param/animated")
    points = [(float(w.findtext("vector/x")), float(w.findtext("vector/y"))) for w in track]
    check(len(points) == 61 and points[0] == points[-1], "native contact curve loops without a discontinuity")
    support = expected["contact_fraction"]
    check(support == .64 and all(abs(points[i][0]-(1-2*i/60/support)) < 1e-6 and points[i][1] == 0 for i in range(39)), "linear grounded part occupies 64 percent of the native cycle")
    header = project / "src/generated/creature_motion_data.hpp"
    check(hashlib.sha256(header.read_bytes()).hexdigest() == receipt["header_sha256"], "runtime still samples the exact existing Synfig export")
    summary = json.loads((args.qa / "summary.json").read_text(encoding="utf-8"))
    check(summary["status"] == "PASS" and summary["failures"] == 0, "actual player renderer and simulation tests passed")
    check(summary["audio_opened"] is False, "no audio device opened")
    rows = list(csv.DictReader((args.qa / "feet.csv").open(encoding="utf-8")))
    check(len(rows) == 720, "both feet tracked through all 360 movement frames")
    previous = {}
    slip, contacts = 0, 0
    for row in rows:
        key = row["sequence"], row["side"]
        phase, x, y = (float(row[k]) for k in ("phase", "foot_x", "foot_y"))
        if key in previous and key[0] in ("chase", "patrol"):
            p, xx, yy = previous[key]
            if phase >= p and phase < support-.02 and p < support-.02:
                delta = math.hypot(x-xx, y-yy)
                slip = max(slip, delta)
                contacts += 1
                check(delta < .015, f"planted support at {key[0]} frame {row['frame']} foot {key[1]}")
        previous[key] = phase, x, y
    check(contacts > 200 and slip < .015, "independent foot displacement agrees with ground contact")
    count = 0
    for folder, expected_count in (("poses", 192), ("chase", 120), ("patrol", 120), ("stop-start", 120)):
        paths = list((args.qa / folder).glob("*.png"))
        check(len(paths) == expected_count, folder + ": complete native sequence")
        hashes = set()
        for path in paths:
            with Image.open(path) as im:
                im.load()
                box = im.getchannel("A").getbbox()
                check(im.size == (900, 600) and box is not None and box[0] > 2 and box[1] > 2 and box[2] < 898 and box[3] < 598 and box[3]-box[1] > 200, path.name + ": full detailed actor at the correct render scale")
                hashes.add(hashlib.sha256(im.tobytes()).hexdigest())
            count += 1
        check(len(hashes) > expected_count*.7, folder + ": animation contains changing native poses")
    check(count == 552, "192 directional poses plus 360 real movement frames verified")
    result = {"status": "PASS" if all(c["pass"] for c in checks) else "FAIL", "checks": len(checks), "failures": [c for c in checks if not c["pass"]], "verified_frames": count, "contact_samples": contacts, "max_slip_px": slip, "source_sha256": expected["sha256"], "audio_opened": False}
    if args.preview:
        frames = []
        for i in range(120):
            image = Image.new("RGBA", (900, 600), (18, 33, 37, 255))
            draw = ImageDraw.Draw(image)
            for x in range(0, 900, 50):
                draw.line((x, 0, x, 599), fill=(43, 62, 62, 255))
            for y in range(10, 600, 50):
                draw.line((0, y, 899, y), fill=(43, 62, 62, 255))
            with Image.open(args.qa / "chase" / f"{i}.png") as pose:
                image.alpha_composite(pose)
            frames.append(image.convert("RGB"))
        frames[0].save(args.qa / "chase-preview.webp", save_all=True, append_images=frames[1:], duration=[17,17,16]*40, loop=0, lossless=True, method=4)
    (args.qa / "verification.json").write_text(json.dumps(result, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    print(json.dumps(result, ensure_ascii=True))
    raise SystemExit(0 if result["status"] == "PASS" else 2)


if __name__ == "__main__":
    main()

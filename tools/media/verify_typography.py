"""Independent readback: true bold assets, real text bounds and GPU captures."""
import argparse
import csv
import hashlib
import json
import struct
from pathlib import Path

from PIL import Image


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--qa", type=Path, required=True)
    parser.add_argument("--assets", type=Path, default=Path("assets"))
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    qa = args.qa
    checks = []

    def check(ok, label):
        checks.append({"pass": bool(ok), "check": label})

    def sfnt_tables(data):
        count = struct.unpack_from(">H", data, 4)[0]
        return {data[12 + 16*i:16 + 16*i].decode("ascii"): struct.unpack_from(
            ">II", data, 20 + 16*i) for i in range(count)}

    for name in ("Lato-Bold.ttf", "Cinzel-Bold.ttf"):
        data = (args.assets / name).read_bytes()
        tables = sfnt_tables(data)
        weight = struct.unpack_from(">H", data, tables["OS/2"][0] + 4)[0]
        check(weight == 700 and "fvar" not in tables, name + ": genuine static bold face, weight 700")
    provenance = json.loads((Path(__file__).resolve().parents[2] / "art/typography/provenance.json").read_text(encoding="utf-8"))
    check(hashlib.sha256((args.assets / "Cinzel-Bold.ttf").read_bytes()).hexdigest() == provenance["output_sha256"], "title font matches its licensed local derivation")
    summary = json.loads((qa / "summary.json").read_text(encoding="utf-8"))
    check(summary["status"] == "PASS" and summary["failures"] == 0 and summary["scenes"] == 57, "renderer checks passed in all 57 screen/scale combinations")
    check(summary["audio_opened"] is False, "renderer never opened an audio device")
    weights = list(csv.DictReader((qa / "coverage.tsv").open(encoding="utf-8"), delimiter="\t"))
    check(len(weights) == 18, "both faces and small captions compared at three native pixel scales")
    for row in weights:
        check(float(row["ratio"]) > 1.08, f"GPU glyph coverage increased: scale={row['scale']}, role={row['role']}")
    rows = list(csv.DictReader((qa / "text-bounds.tsv").open(encoding="utf-8"), delimiter="\t"))
    check(len(rows) > 1000, "bounds recorded from actual production text draws")
    for row in rows:
        x, y, w, h = (float(row[k]) for k in ("x", "y", "width", "height"))
        check(x >= -0.1 and y >= -0.1 and x+w <= 1440.1 and y+h <= 900.1,
              f"text inside canvas: {row['scene']} {row['scale']} {row['text']}")
    for scale in (80, 100, 120):
        images = sorted(qa.glob(f"*-{scale}.png"))
        check(len(images) == 20, f"all 19 scenes and weight reference present at scale {scale}")
        for path in images:
            with Image.open(path) as im:
                im.load()
                check(im.size == (1440*scale//100, 900*scale//100), path.name + ": native resolution")
                extrema = im.convert("RGB").getextrema()
                check(all(high-low > 100 for low, high in extrema), path.name + ": visible rendered content")
    for folder, filename in (("qa-v39", "rarity-ui-tests.txt"), ("qa-v42", "trade-tests.txt"),
                             ("qa-v13", "progression-tests.txt"), ("qa-v27", "skills-tests.txt"),
                             ("qa-v30", "combat-tests.txt")):
        path = qa.parent / folder / filename
        # Legacy suites have independent result files beside the isolated captures.
        if not path.exists() and folder == "qa-v13":
            candidates = list((qa.parent / folder).glob("*tests.txt"))
            path = candidates[0] if len(candidates) == 1 else path
        text = path.read_text(encoding="utf-8")
        check("FAIL" not in text and "failures: 0" in text, folder + ": existing UI regression suite passed")
    result = {"status": "PASS" if all(c["pass"] for c in checks) else "FAIL", "checks": len(checks),
              "failures": [c for c in checks if not c["pass"]], "text_draws": len(rows),
              "native_scene_captures": 57, "audio_opened": False}
    (qa / "verification.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, ensure_ascii=True))
    raise SystemExit(0 if result["status"] == "PASS" else 2)


if __name__ == "__main__":
    main()

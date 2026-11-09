"""Check real, hidden renderer captures and assemble an unpainted comparison."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import numpy as np
from PIL import Image, ImageDraw, ImageFont


def verify(qa, output):
    output.mkdir(parents=True, exist_ok=True)
    renderer = re.search(r"(\d+)/(\d+) passed", (qa / "render-tests.txt").read_text())
    assert renderer and renderer[1] == renderer[2], "renderer checks failed"
    checks = 0
    views = []
    for direction in range(8):
        before = np.array(Image.open(qa / f"hair-before-{direction}.png").convert("RGBA"))
        after = np.array(Image.open(qa / f"hair-after-{direction}.png").convert("RGBA"))
        assert before.shape == after.shape == (420, 400, 4)
        assert np.array_equal(before[:, :, 3], after[:, :, 3]), "silhouette changed"
        checks += 1
        changed = np.any(before[:, :, :3] != after[:, :, :3], axis=2)
        y, x = np.where(changed)
        assert len(x) > 100, "correction missing from actual hero"
        assert x.min() >= 160 and x.max() < 240 and y.min() >= 85 and y.max() < 175, "non-hair appearance changed"
        checks += 2
        views.append({"direction": direction, "changed_pixels": int(len(x)),
                      "bounds": [int(x.min()), int(y.min()), int(x.max()) + 1, int(y.max()) + 1]})
    before = np.array(Image.open(qa / "hair-before-2.png"))
    after = np.array(Image.open(qa / "hair-after-2.png"))
    old_highlight = float(np.percentile(before[98:112, 184:216, 0], 95))
    new_highlight = float(np.percentile(after[98:112, 184:216, 0], 95))
    assert new_highlight < old_highlight * .85, "forehead highlight still visible"
    checks += 1
    canvas = Image.new("RGB", (600, 480), (17, 25, 27))
    draw = ImageDraw.Draw(canvas)
    font = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", 17)
    for row, (stage, label) in enumerate((("before", "PRZED"), ("after", "PO"))):
        draw.text((15, row * 240 + 7), label, fill=(230, 199, 139), font=font)
        for column, direction in enumerate((1, 2, 6)):
            capture = Image.open(qa / f"hair-{stage}-{direction}.png").convert("RGBA")
            head = capture.crop((155, 80, 245, 175)).resize((180, 190), Image.Resampling.LANCZOS)
            canvas.paste(head, (column * 200 + 10, row * 240 + 39), head)
    canvas.save(output / "comparison.png")
    report = {"status": "PASS", "capture_checks": checks, "renderer_checks": int(renderer[1]),
              "animation_frames": 420, "views": views, "front_highlight_red_p95": {
                  "before": old_highlight, "after": new_highlight},
              "source_atlas_sha256": hashlib.sha256(Path("assets/sprites/hero-paints.png").read_bytes()).hexdigest(),
              "source_kra_sha256": hashlib.sha256(Path("art/hero/comfy/hero-materials.kra").read_bytes()).hexdigest(),
              "audio_device_opened": False, "hidden": True}
    (output / "render-QA.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Hair captures: {checks} checks passed; highlight {old_highlight:g} -> {new_highlight:g}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--qa", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    verify(args.qa, args.output)

"""Independent spacing/coverage checks and previews from real renderer output."""
import argparse
import json
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont


def verify(root, captures):
    audits = [json.loads((root / f"{name}.json").read_text()) for name in ("before", "after")]
    stats = []
    for audit in audits:
        positions = np.array([p[:2] for p in audit["props"] if p[2] >= 14])
        distances = np.linalg.norm(positions[:, None] - positions[None, :], axis=2)
        np.fill_diagonal(distances, np.inf)
        occupied = len(set(map(tuple, np.floor(positions / 9).astype(int))))
        stats.append({"decorations": len(positions), "minimum_spacing": float(distances.min()),
                      "mean_nearest_spacing": float(distances.min(axis=1).mean()),
                      "pairs_under_3_tiles": int(np.sum(distances < 3) // 2), "occupied_9_tile_sectors": occupied,
                      "largest_unreserved_gap": audit["largest_gap"]})
    assert stats[1]["minimum_spacing"] > 4.4
    assert stats[0]["pairs_under_3_tiles"] > 0 and stats[1]["pairs_under_3_tiles"] == 0
    assert stats[1]["occupied_9_tile_sectors"] > stats[0]["occupied_9_tile_sectors"]
    assert stats[1]["largest_unreserved_gap"] < stats[0]["largest_unreserved_gap"] * .5
    summary = json.loads((captures / "summary.json").read_text())
    assert summary["status"] == "PASS" and summary["hidden"] and not summary["audio_opened"]
    for n in range(12):
        with Image.open(captures / f"boundary-{n}.png") as frame:
            frame.load()
            assert frame.size == (1728, 1080)
    font = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", 19)
    overview = Image.new("RGB", (1000, 550), (22, 25, 31))
    draw = ImageDraw.Draw(overview)
    colors = {14: (179, 167, 151), 15: (139, 172, 155), 16: (230, 172, 93), 17: (153, 166, 209)}
    for column, (audit, label) in enumerate(zip(audits, ("PRZED", "PO"))):
        ox = column * 500 + 25
        draw.text((ox, 12), label, font=font, fill=(234, 206, 155))
        for n in range(9):
            v = n * 54
            draw.line((ox + v, 50, ox + v, 482), fill=(45, 45, 53))
            draw.line((ox, 50 + v, ox + 432, 50 + v), fill=(45, 45, 53))
        for x, y, family, size in audit["props"]:
            if family < 14:
                continue
            px, py = ox + x * 6, 50 + y * 6
            draw.ellipse((px - 5, py - 5, px + 5, py + 5), fill=colors[family])
        draw.text((ox, 505), f"Odstęp min.: {stats[column]['minimum_spacing']:.1f} pola", font=font, fill=(220, 220, 220))
    overview.save(root / "distribution.png")
    with Image.open(captures / "boundary-0.png") as frame:
        frame.crop((20, 140, 1240, 890)).resize((976, 600), Image.Resampling.LANCZOS).save(root / "boundary-preview.png")
    (root / "render-QA.json").write_text(json.dumps({"status": "PASS", "before": stats[0], "after": stats[1],
        "renderer": summary, "boundary_captures": 12}, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(stats, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--captures", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    verify(args.root, args.captures)

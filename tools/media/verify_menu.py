"""Decode native menu QA PNGs independently; no audio or external services."""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageChops


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--qa", type=Path, required=True)
    parser.add_argument("--startup", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    expected = {"main-80", "main-100", "main-120", "load-empty", "options",
                "credits", "exit-startup", "pause-prologue", "save-prologue",
                "pause-forest", "pause-desert", "load-populated", "pause-80",
                "pause-120", "options-80", "options-120", "pause-tower",
                "pause-death", "exit-ingame"}
    paths = sorted(args.qa.glob("*.png"))
    if {p.stem for p in paths} != expected:
        raise ValueError("Incomplete/unexpected menu image set")
    if args.startup:
        paths.append(args.startup)
    images = []
    checks = 1
    for path in paths:
        with Image.open(path) as picture:
            picture.load()
            if path != args.startup:
                factor = 0.8 if path.stem.endswith("-80") else 1.2 if path.stem.endswith("-120") else 1
                if picture.size != (round(1440 * factor), round(900 * factor)):
                    raise ValueError(f"Wrong dimensions: {path}")
                checks += 1
            elif picture.width < 1000 or picture.height < 600:
                raise ValueError("Startup presentation too small")
            rgba = picture.convert("RGBA")
            if rgba.getchannel("A").getextrema() != (255, 255):
                raise ValueError(f"Nonopaque final presentation: {path}")
            rgb = rgba.convert("RGB")
            r, g, b = rgb.split()
            bright = ImageChops.lighter(ImageChops.lighter(r, g), b).histogram()
            if sum(bright[51:]) < picture.width * picture.height // 500:
                raise ValueError(f"Empty/dark render: {path}")
            if len(rgb.resize((360, 225)).getcolors(360 * 225) or []) < 100:
                raise ValueError(f"Insufficient rendered detail: {path}")
            checks += 3
            images.append({"file": str(path.resolve()), "width": picture.width,
                           "height": picture.height,
                           "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})
    result = {"status": "PASS", "checks": checks, "images": images,
              "audio_device_opened": False}
    (args.qa / "image-verification.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(f"PASS: {checks} independent image checks, {len(images)} decoded PNGs")


if __name__ == "__main__":
    main()

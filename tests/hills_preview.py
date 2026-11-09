"""Assemble actual hidden-renderer captures; never edits runtime art."""
from pathlib import Path
import argparse
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser()
parser.add_argument("captures", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
font = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", 19)
small = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", 14)
realms = [("forest", "Wielka Knieja"), ("desert", "Pustynia"), ("volcano", "Kaldera Popiołu")]
frames = []
for index in range(60):
    preview = Image.new("RGB", (1260, 322), (17, 28, 30))
    draw = ImageDraw.Draw(preview)
    for column, (realm, title) in enumerate(realms):
        with Image.open(args.captures / realm / f"frame-{index:03d}.png") as image:
            preview.paste(image.convert("RGB").resize((420, 264), Image.Resampling.LANCZOS), (column * 420, 38))
        draw.text((column * 420 + 14, 5), title, font=font, fill=(235, 214, 164))
    draw.text((14, 303), "Rzeczywiste klatki gry: wejście na wzniesienie i zejście po drugiej stronie", font=small, fill=(182, 197, 193))
    frames.append(preview)
frames[0].save(args.output / "walkable-hills.gif", save_all=True, append_images=frames[1:], duration=120, loop=0, optimize=False)
sheet = Image.new("RGB", (1260, 322 * 3), (17, 28, 30))
for row, index in enumerate((0, 28, 59)):
    sheet.paste(frames[index], (0, row * 322))
sheet.save(args.output / "ascent-crest-descent.png")
print(f"Saved 60 captured frames: {args.output}")

"""QA contact sheet from captured game pixels; no runtime art changes."""
from pathlib import Path
import argparse
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser()
parser.add_argument("captures", type=Path)
parser.add_argument("output", type=Path)
args = parser.parse_args()
font = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", 23)
sheet = Image.new("RGB", (1200, 828), (17, 28, 30))
draw = ImageDraw.Draw(sheet)
for column, (filename, title) in enumerate((("forest-hill-7.png", "Las — pagórki i grzbiety"), ("desert-hill-6.png", "Pustynia — wydmy"), ("volcano-hill-11.png", "Kaldera — wzniesienia"))):
    draw.text((column * 400 + 12, 8), title, font=font, fill=(235, 214, 164))
    with Image.open(args.captures / filename) as image:
        shot = image.crop((125, 160, 925, 800)).resize((400, 320), Image.Resampling.LANCZOS)
        sheet.paste(shot, (column * 400, 46))
draw.text((12, 380), "Kaldera — ciągła nawierzchnia, miękkie brzegi i połączone rozwidlenia", font=font, fill=(235, 214, 164))
for column, filename in enumerate(("volcano-junction-0.png", "volcano-junction-1.png")):
    with Image.open(args.captures / filename) as image:
        shot = image.crop((0, 155, 1050, 855)).resize((600, 400), Image.Resampling.LANCZOS)
        sheet.paste(shot, (column * 600, 428))
sheet.save(args.output)

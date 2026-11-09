"""Contact sheet of actual hidden gameplay captures; does not author game art."""
from pathlib import Path
import argparse
from PIL import Image, ImageDraw, ImageFont

p = argparse.ArgumentParser()
p.add_argument('capture_root', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
font = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 21)
sheet = Image.new('RGB', (1200, 808), '#141d22')
draw = ImageDraw.Draw(sheet)
for i, (species, label) in enumerate([
    (10, 'Kobra piaskowa — Jadowity wachlarz'),
    (9, 'Skarabeusz miedziany — Miedziana fala'),
    (16, 'Obsydianowy wojownik — Rozłupujący cios'),
    (17, 'Żarowy drak — Żarowy oddech'),
]):
    x, y = i % 2 * 600, i // 2 * 404
    frame = Image.open(a.capture_root / f'{species}-0-warning.png')
    sheet.paste(frame.crop((365, 195, 965, 555)), (x, y + 42))
    draw.text((x + 12, y + 10), label, font=font, fill='#efdbad')
a.output.parent.mkdir(parents=True, exist_ok=True)
sheet.save(a.output)

"""Pack the painted 2D turnarounds and garment studies into editable Krita inputs."""
import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageOps

from wanderer_paints import place, write_ora


CELL = 128
LANCZOS = Image.Resampling.LANCZOS


def heads(path):
    source = Image.open(path).convert("RGBA")
    views = []
    for index in range(5):
        tile = source.crop((source.width * index // 5, 0, source.width * (index + 1) // 5, source.height))
        alpha = tile.getchannel("A").point(lambda value: 0 if value < 10 else value)
        tile.putalpha(alpha)
        views.append(tile.crop(alpha.getbbox()))
    # Match the crown-to-chin distance. The renderer packs and restores the full UV cell.
    height = min(120, int(min(124 * v.height / v.width for v in views)))
    layers = []
    labels = ("Przód", "Trzy czwarte", "Profil", "Trzy czwarte od tyłu", "Tył")
    for index, (label, view) in enumerate(zip(labels, views)):
        view = view.resize((round(view.width * height / view.height), height), LANCZOS)
        row = Image.new("RGBA", (5 * CELL, CELL))
        row.paste(view, (index * CELL + (CELL - view.width) // 2, (CELL - height) // 2))
        layers.append((label, row))
    return layers, views[0]


def face_material(front):
    # Place only features on the skin under helmets; cheeks and skin silhouette
    # come from the shaped head, so no painted face border can appear in profile.
    width, height = front.size
    result = Image.new("RGBA", (CELL, CELL))
    for box, dest in [
        ((.23,.53,.46,.65), (30,38,61,62)), ((.55,.53,.79,.65), (67,38,98,62)),
        ((.43,.64,.61,.79), (51,57,78,80)), ((.36,.79,.71,.91), (40,79,90,101)),
    ]:
        region = tuple(round(n*s) for n,s in zip(box,(width,height,width,height)))
        x,y,right,bottom = dest
        patch = front.crop(region).resize((right-x,bottom-y), LANCZOS)
        mask = Image.new("L", patch.size)
        ImageDraw.Draw(mask).ellipse((3,3,patch.width-4,patch.height-4),fill=255)
        patch.putalpha(mask.filter(ImageFilter.GaussianBlur(2)))
        result.alpha_composite(patch, (x,y))
    return result


def tone(image, dark, light):
    lum = ImageOps.autocontrast(image.convert("L"), cutoff=(1, 1))
    return ImageOps.colorize(lum, dark, light)


def garments(sunburst, materials, face):
    sheet = Image.open(sunburst).convert("RGB")
    def study(col, row):
        return sheet.crop((col * sheet.width // 3 + 3, row * sheet.height // 2 + 3,
                           (col + 1) * sheet.width // 3 - 3, (row + 1) * sheet.height // 2 - 3)).resize((CELL, CELL), LANCZOS)

    def comfy(name):
        safe = {"bracer": (.38,.10,.96,.75), "hair": (.38,.02,.98,.96),
                "boot": (.06,.13,.93,.90), "copper": (.11,.06,.95,.66),
                "trousers": (.06,.06,.98,.90), "sleeves": (.04,.05,.83,.96),
                "skirts": (.04,.06,.67,.94)}
        image = Image.open(materials / (name + ".png")).convert("RGB")
        x0,y0,x1,y1 = safe.get(name, (.06,.06,.94,.94))
        region = (round(x0*image.width),round(y0*image.height),round(x1*image.width),round(y1*image.height))
        return image.crop(region).resize((CELL, CELL), LANCZOS)

    def layered(col, row, name, dark, light, weight):
        garment = tone(study(col, row), dark, light)
        material = tone(comfy(name), dark, light)
        return Image.blend(garment, material, weight)

    layers = [
        ("Twarz pod hełmem", face_material(face)),
        ("Grzywka", tone(comfy("hair"), "#29211c", "#685040")),
        ("Włosy przy karku", tone(ImageOps.mirror(comfy("hair")), "#29201c", "#634b39")),
        ("Pikowany kaftan", layered(0, 0, "tunic", "#6d6d6d", "#ffffff", .22)),
        ("Karwasze", layered(2, 0, "bracer", "#4f3026", "#cd9a71", .32)),
        ("Podróżne buty", tone(comfy("boot"), "#707070", "#fafafa")),
        ("Kuta miedź", layered(0, 1, "copper", "#563628", "#e6a269", .24)),
        ("Peleryna z różą wiatrów", layered(1, 0, "mantle", "#727272", "#fcfcfc", .17)),
        ("Rozcięte poły", layered(1, 1, "skirts", "#737373", "#f4f4f4", .26)),
        ("Bryczesy", tone(comfy("trousers"), "#777777", "#f7f7f7")),
        ("Rękawy", tone(comfy("sleeves"), "#797979", "#fffaf0")),
        ("Kaletka", layered(2, 1, "pouch", "#4f372a", "#bd9062", .25)),
    ]
    return [(name, place(image.convert("RGBA"), index)) for index, (name, image) in enumerate(layers)]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("--materials", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    layers, face = heads(args.source / "sunburst-heads.png")
    if args.materials:
        write_ora(args.source / "garments.ora", garments(args.source / "sunburst-garments.png", args.materials, face),
                  "Wędrowiec · warstwy materiałów")
    else:
        write_ora(args.source / "heads.ora", layers, "Wędrowiec · obrót głowy")

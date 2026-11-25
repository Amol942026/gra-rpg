"""Retouch the ComfyUI cobra material in Krita for its on-screen size."""
import argparse
import json
from pathlib import Path

from PIL import Image, ImageFilter, ImageOps

from krita_mcp_client import KritaMCP, start_krita_if_needed
from polish_volcano_art import Paint
from volcano_river_detail import retain_finish


def paint_skin(root: Path, source_atlas: Path):
    root.mkdir(parents=True)
    atlas = Image.open(source_atlas).convert('RGBA')
    old = atlas.crop((256, 256, 384, 384))
    # Retain the mineral/copper variation of the existing ComfyUI material.
    # Reduce its contrast before painting recognizable scales over it.
    tone = ImageOps.colorize(old.convert('L').filter(ImageFilter.GaussianBlur(3)),
                            '#75472b', '#976737')
    base = root / 'tonal-base.png'
    tone.save(base)
    start_krita_if_needed()
    with KritaMCP(root / 'krita', timeout=60) as client:
        document = client.call('krita_document', action='open', path=str(base))['affected_document_id']
        state = client.call('krita_state', document_id=document)
        assert state['runtime_compatibility']['status'] == 'compatible'
        client.call('krita_layers', action='rename', document_id=document,
                    layer_id=state['active_layer_id'], name='ComfyUI / tony miedzi')
        brush = Paint.__new__(Paint)
        brush.c, brush.doc, brush.lid = client, document, state['active_layer_id']
        brush.width = brush.height = 128
        brush.ox = brush.oy = 0
        brush.s = 1
        brush.layers = ['ComfyUI / tony miedzi']
        brush.layer('Łuski grzbietu i boków', .84)
        shades = ('#b58148', '#bd894e', '#ad7541', '#cb9659', '#a36e3d')
        for row in range(-1, 10):
            y = 8 + row * 14
            for column in range(6):
                x = 17 + column * 19 + (row % 2) * 8
                shade = shades[(row * 3 + column * 2) % len(shades)]
                brush.poly(shade, [(x - 8, y - 3), (x - 4, y - 6), (x + 4, y - 6),
                                   (x + 8, y - 3), (x + 7, y + 4), (x, y + 8), (x - 7, y + 4)])
        brush.layer('Ciemne siodła', .55)
        for y in (26, 65, 101):
            brush.poly('#644332', [(15, y - 7), (40, y - 1), (65, y + 5), (89, y), (113, y - 9),
                                   (114, y - 3), (92, y + 9), (65, y + 15), (40, y + 8), (14, y)])
        brush.layer('Tarczki brzuszne')
        for side in (1, -1):
            def points(coords):
                return [(x if side > 0 else 127 - x, y) for x, y in coords]
            brush.poly('#66412c', points([(0, 0), (15, 0), (14, 127), (0, 127)]))
            for y in range(0, 128, 10):
                brush.poly('#d1a66b', points([(0, y), (8, y), (13, y + 2), (12, y + 8), (0, y + 8)]))
            brush.line('#e3b779', points([(1, 0), (1, 127)]), 2)
        result = retain_finish(brush, root, 'kobra-luski')
    paint = Image.open(root / 'kobra-luski.png').convert('RGBA')
    assert paint.size == (128, 128) and paint.getchannel('A').getextrema() == (255, 255)
    # Replace only the cobra's cell in the shared sprite atlas.
    atlas.paste(paint, (256, 256))
    atlas.save(root / 'creature-paints.png')
    (root / 'receipt.json').write_text(json.dumps(result, indent=2), encoding='utf8')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('source_atlas', type=Path)
    args = parser.parse_args()
    paint_skin(args.directory.resolve(), args.source_atlas.resolve())

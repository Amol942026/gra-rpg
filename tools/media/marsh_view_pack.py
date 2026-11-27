"""Pack single-exposure 512-angle Blender renders for Emberfall's marsh actors."""
from argparse import ArgumentParser
from pathlib import Path
import hashlib
import json
import struct

import numpy as np
from PIL import Image


SIZES = ((140, 100), (164, 96), (120, 120), (204, 164))
WALK_MARGIN = 12
FACINGS = 512
LIMIT = 2048
GUTTER = 2


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pack_species(source, target, species, walk=False):
    width, height = SIZES[species]
    if walk:
        width+=2*WALK_MARGIN
        height+=2*WALK_MARGIN
    count=128*12 if walk else FACINGS
    name_prefix='walk' if walk else 'views'
    tiles = []
    sources = hashlib.sha256()
    # Native Blender action projects and the 128-key motion lattice use
    # every fourth complete exposure in a regular 8x8 UV layout.
    keys = Image.new('RGBA', (8*width, 8*height))
    for index in range(count):
        path = source / str(species) / (f'{index:04}.png' if walk else f'{index:03}.png')
        with Image.open(path) as image:
            if image.size != (width, height) or image.mode != 'RGBA':
                raise ValueError(f'Invalid canvas or channels: {path}')
            alpha = image.getchannel('A')
            box = alpha.getbbox()
            if box is None:
                raise ValueError(f'Empty facing: {path}')
            x, y, x1, y1 = box
            if min(x, y, width-x1, height-y1) < GUTTER:
                raise ValueError(f'Missing transparent border: {path}')
            x, y = x-GUTTER, y-GUTTER
            pixels = np.array(image.crop((x, y, x1+GUTTER, y1+GUTTER)))
        # Keep the transparent fringe from Blender verbatim. Its pigment also
        # participates in straight-alpha bilinear sampling next to the outline.
        tiles.append((index, x, y, pixels))
        if not walk and index%4 == 0:
            key = index//4
            keys.paste(Image.fromarray(pixels), (key%8*width+x, key%64//8*height+y))
            if key%64 == 63:
                keys.save(target/f'neutral-{species}-{key//64}.png', compress_level=6)
                keys = Image.new('RGBA', keys.size)
        sources.update(bytes.fromhex(digest(path)))
    tiles.sort(key=lambda row: (-row[3].shape[0], -row[3].shape[1], row[0]))
    page = np.zeros((LIMIT, LIMIT, 4), dtype=np.uint8)
    rects = [None]*count
    pages = []
    left = top = shelf_height = used_width = 0

    def save_page():
        image = Image.fromarray(page[:top+shelf_height, :used_width])
        name = target / f'{name_prefix}-{species}-{len(pages)}.png'
        image.save(name, compress_level=6)
        with Image.open(name) as saved:
            if not np.array_equal(np.asarray(saved), np.asarray(image)):
                raise ValueError(f'Atlas pixels changed at export: {name}')
        pages.append({'file': name.name, 'size': image.size, 'sha256': digest(name)})

    for index, x, y, pixels in tiles:
        h, w = pixels.shape[:2]
        if left+w > LIMIT:
            if top+shelf_height+h > LIMIT:
                save_page()
                page.fill(0)
                top = used_width = 0
            else:
                top += shelf_height
            left = shelf_height = 0
        page[top:top+h, left:left+w] = pixels
        rects[index] = (len(pages), left, top, w, h, x, y)
        left += w
        shelf_height = max(shelf_height, h)
        used_width = max(used_width, left)
    save_page()
    header = struct.pack('<4s4H', b'MVI1', species, count, len(pages), 0)
    index_file = target / f'{name_prefix}-{species}.bin'
    index_file.write_bytes(header + b''.join(struct.pack('<7H', *r) for r in rects))
    return {'species': species, 'frames': count, 'sources_sha256': sources.hexdigest(),
            'pages': pages, 'index_sha256': digest(index_file)}


if __name__ == '__main__':
    parser = ArgumentParser(description=__doc__)
    parser.add_argument('--frames', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--walk', type=Path, help='Blender walking cels and support metadata under this export directory')
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    results = [pack_species(args.frames, args.out, s) for s in range(len(SIZES))]
    (args.out/'views-receipt.json').write_text(json.dumps(results, indent=2), encoding='utf-8')
    if args.walk:
        results = [pack_species(args.walk/'walk', args.out, s, walk=True) for s in range(len(SIZES))]
        for s in range(len(SIZES)):
            name=f'walk-support-{s}.bin'
            path=args.out/name
            path.write_bytes((args.walk/name).read_bytes())
            results[s]['support_sha256']=digest(path)
        (args.out/'walk-receipt.json').write_text(json.dumps(results,indent=2),encoding='utf-8')

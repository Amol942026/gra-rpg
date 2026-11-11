"""Repaint existing tower sprites with local ComfyUI and native Krita.

The game render supplies the exact alpha, perspective, and placement. The
generated material pass and painted details are saved as separate KRA layers.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
import urllib.request
from xml.etree.ElementTree import Element, SubElement, tostring
from zipfile import ZipFile, ZIP_DEFLATED

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

from krita_mcp_client import KritaMCP

RUN = Path('C:/AI2D/pipeline/runs/tower-paint-20261111')
MASTER = Path('C:/AI2D/sprites/emberfall-tower-20261111')
COMFY = Path('C:/Users/Amol/AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI')
MODEL = 'DreamShaper_8_pruned.safetensors'
PREFIX = 'emberfall-tower-20261111'
PILOT = (5, 22, 23)
DESCRIPTIONS = [
    'tall fluted limestone tower column, carved capital and layered plinth',
    'round walnut reading table with an open parchment book, closed books and brass candle holder',
    'carved limestone pedestal with beveled octagonal foot and pale stone top',
    'medieval arched limestone fireplace with hand cut voussoirs, oak logs and small bottles on its mantel',
    'carved grey limestone basin on a pedestal with shallow turquoise water',
    'tall walnut bookshelf filled with five rows of weathered leather-bound books, brass corner fittings and carved wood',
    'carved walnut lectern with an open illuminated parchment manuscript and red bookmark',
    'dark wrought iron alchemy cauldron on tripod feet, riveted bronze rim and emerald liquid',
    'walnut herb planter with damp dark soil and antique brass corner fittings',
    'faceted standing slate rune stone on a limestone pedestal, incised stone and worn rim',
    'angled antique brass telescope on a wooden tripod, blue glass lens and engraved brass rings',
    'astronomical armillary sphere with three fine engraved brass rings on a carved stone pedestal',
    'ancient stone relic altar with engraved bronze rim and pale limestone faces',
    'round walnut alchemy worktable with glass potion bottles, stone mortar and open notes',
    'arched gothic limestone window with leaded blue diamond panes and carved stone sill',
    'small blue crystal scrying orb, layered glass reflections and pale turquoise caustics',
    'small floating faceted violet quartz crystal, translucent blue and lavender glints',
    'small floating faceted warm amber relic crystal, gold reflections and fine facets',
    'green medicinal sprig with broad mint leaves and visible veins',
    'sage green alchemical herb with small lavender flowers',
    'olive green medicinal herb with golden buds and long leaves',
    'small amber candle flame, soft orange edge and warm pale core',
    'walnut medieval shop counter, shelves, amber and teal glass bottles, parchment, cloth and brass fittings',
    'isometric ascending limestone stairs with individual stone treads, carved balusters and worn handrail',
    'isometric descending limestone stairwell, carved railings, individual steps and dark recessed shaft',
]


def write_json(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def item_name(index):
    return f'{index:02}.png'


def prepare():
    MASTER.mkdir(exist_ok=False)
    for folder in ('inputs', 'workflows', 'native', 'delivery', 'raw', 'logs', 'ora'):
        (MASTER / folder).mkdir()
    staging = COMFY / 'input' / PREFIX
    staging.mkdir(exist_ok=False)
    manifest = []
    graphs = {'pilot': {}, 'furniture': {}, 'decorations': {}}
    for graph in graphs.values():
        graph['1'] = {'class_type': 'CheckpointLoaderSimple', 'inputs': {'ckpt_name': MODEL}}
    for i, subject in enumerate(DESCRIPTIONS):
        source_path = RUN / 'baseline' / item_name(i)
        source = Image.open(source_path).convert('RGBA')
        box = source.getchannel('A').getbbox()
        crop = source.crop(box)
        scale = 448 / max(crop.size)
        size = tuple(max(1, round(n * scale)) for n in crop.size)
        canvas = tuple(max(192, ((n + 32 + 63) // 64) * 64) for n in size)
        offset = tuple((n - m) // 2 for n, m in zip(canvas, size))
        backdrop = Image.new('RGBA', canvas, (28, 34, 40, 255))
        backdrop.alpha_composite(crop.resize(size, Image.Resampling.LANCZOS), offset)
        backdrop.convert('RGB').save(MASTER / 'inputs' / item_name(i))
        backdrop.convert('RGB').save(staging / item_name(i))
        group = 'pilot' if i in PILOT else 'furniture' if i < 15 else 'decorations'
        graph = graphs[group]
        load, positive, negative, encode, sample, decode, save = map(str, range(10+i*10, 17+i*10))
        graph[load] = {'class_type': 'LoadImage', 'inputs': {'image': f'{PREFIX}/{item_name(i)}'}}
        prompt = (
            f'An isolated {subject}. Hand painted realistic isometric fantasy RPG prop, '
            'preserve the supplied object and camera exactly, fine craftsmanship, rich surface detail, '
            'subtle weathering, warm antique gold highlights and cool blue shadows, '
            'detailed material grain, crisp sculpted edges, dark slate background, no cast shadow'
        )
        avoid = 'text, letters, watermark, scenery, room, wall behind the object, extra objects, plastic, cartoon, blurry, perspective change, cut off, bright background'
        for key, value in ((positive, prompt), (negative, avoid)):
            graph[key] = {'class_type': 'CLIPTextEncode', 'inputs': {'clip': ['1', 1], 'text': value}}
        graph[encode] = {'class_type': 'VAEEncode', 'inputs': {'pixels': [load, 0], 'vae': ['1', 2]}}
        denoise = .38 if i < 15 or i == 22 else .34
        if i >= 23:
            denoise = .32
        graph[sample] = {'class_type': 'KSampler', 'inputs': {
            'model': ['1', 0], 'positive': [positive, 0], 'negative': [negative, 0],
            'latent_image': [encode, 0], 'seed': 7112600+i, 'steps': 18, 'cfg': 6.0,
            'sampler_name': 'dpmpp_2m', 'scheduler': 'karras', 'denoise': denoise,
        }}
        graph[decode] = {'class_type': 'VAEDecode', 'inputs': {'samples': [sample, 0], 'vae': ['1', 2]}}
        graph[save] = {'class_type': 'SaveImage', 'inputs': {'images': [decode, 0], 'filename_prefix': f'{PREFIX}/{i:02}'}}
        manifest.append({'id': i, 'source': str(source_path), 'source_sha256': digest(source_path),
                         'size': source.size, 'crop': box, 'scaled': size, 'offset': offset,
                         'group': group, 'node': save, 'subject': subject, 'denoise': denoise})
    for group, graph in graphs.items():
        write_json(MASTER / 'workflows' / (group + '.json'), graph)
    write_json(MASTER / 'inputs.json', manifest)
    print('Prepared 25 original 2D shapes and three local workflows', flush=True)


def png_bytes(image):
    buffer = io.BytesIO()
    image.save(buffer, format='PNG')
    return buffer.getvalue()


def write_ora(path, base, generated):
    document = Element('image', {'w': str(base.width), 'h': str(base.height), 'name': 'Emberfall – Wieża Eldrina', 'version': '0.0.3'})
    stack = SubElement(document, 'stack')
    with ZipFile(path, 'x', ZIP_DEFLATED) as archive:
        archive.writestr('mimetype', 'image/openraster', compress_type=0)
        for index, (name, image) in enumerate((('ComfyUI – malowanie materiału', generated), ('Oryginalny obrys i światło', base))):
            member = f'data/layer{index}.png'
            archive.writestr(member, png_bytes(image))
            SubElement(stack, 'layer', {'name': name, 'src': member, 'x': '0', 'y': '0', 'opacity': '1.0', 'visibility': 'visible', 'composite-op': 'svg:src-over'})
        archive.writestr('stack.xml', tostring(document, encoding='utf-8'))
        archive.writestr('mergedimage.png', png_bytes(Image.alpha_composite(base, generated)))


def collect(group, prompt_id):
    manifest = json.loads((MASTER / 'inputs.json').read_text(encoding='utf-8'))
    with urllib.request.urlopen(f'http://127.0.0.1:8188/history/{prompt_id}', timeout=15) as response:
        history = json.load(response)[prompt_id]
    if history['status']['status_str'] != 'success':
        raise RuntimeError(history['status'])
    write_json(MASTER / 'logs' / (group + '-history.json'), history)
    for record in manifest:
        if record['group'] != group:
            continue
        name = item_name(record['id'])
        image = history['outputs'][record['node']]['images'][0]
        source = COMFY / 'output' / image['subfolder'] / image['filename']
        raw = MASTER / 'raw' / name
        with raw.open('xb') as stream:
            stream.write(source.read_bytes())
        base = Image.open(record['source']).convert('RGBA')
        left, top, right, bottom = record['crop']
        x, y = record['offset']
        w, h = record['scaled']
        paint = Image.open(raw).convert('RGBA').crop((x,y,x+w,y+h)).resize((right-left,bottom-top),Image.Resampling.LANCZOS)
        mask = base.getchannel('A').crop(record['crop']).filter(ImageFilter.MinFilter(5)).filter(ImageFilter.GaussianBlur(1.2))
        paint.putalpha(mask.point(lambda a: round(a*.80)))
        layer = Image.new('RGBA', base.size)
        layer.paste(paint, (left, top))
        write_ora(MASTER / 'ora' / f'{record["id"]:02}.ora', base, layer)
    print(f'Collected and masked {group} in editable layers', flush=True)


def decorate(client, doc, layer, base, index):
    # Add short material highlights inside the real outline. Place them along
    # bright surfaces in each vertical band, away from gaps between thin parts.
    rgba = np.asarray(base)
    alpha = np.asarray(base.getchannel('A').filter(ImageFilter.MinFilter(7)))
    left, top, right, bottom = base.getchannel('A').getbbox()
    for j, fraction in enumerate((.22, .44, .63, .80)):
        y = round(top + (bottom-top)*fraction)
        low = max(2, y-6)
        high = min(base.height-3, y+7)
        if high <= low:
            continue
        valid = alpha[low:high] >= 245
        if not np.any(valid):
            continue
        rgb = rgba[low:high,:,:3].astype(float)
        luma = rgb.mean(axis=2)
        center = left+(right-left)*(.28 if j%2==0 else .68)
        score = luma - np.abs(np.arange(base.width)-center)*1.1
        py, px = np.unravel_index(np.argmax(np.where(valid,score,-1e9)), score.shape)
        py += low
        count = 8 if index < 15 or index >= 22 else 4
        # Stop a mark before the existing edge, preserving the small cutouts.
        points = [{'x':int(px),'y':int(py)}]
        for step in range(1,count):
            qx, qy = int(px+step), int(py-step*.48)
            if qx>=base.width or qy<0 or alpha[qy,qx]<245:
                break
            points.append({'x':qx,'y':qy})
        if len(points)<3:
            continue
        color = tuple(min(255, int(v*.78+57)) for v in rgba[py,px,:3])
        client.call('krita_draw',shape='path',document_id=doc,layer_id=layer,
                    points=points,foreground='#%02x%02x%02x'%color,fill_style='None',
                    stroke_style='ForegroundColor',brush_size=1.4 if index<15 or index>=22 else 1)


def finish(indices):
    records = json.loads((MASTER / 'inputs.json').read_text(encoding='utf-8'))
    with KritaMCP(MASTER / 'logs' / ('krita-'+str(indices[0]))) as client:
        for i in indices:
            record = records[i]
            name = item_name(i)
            base = Image.open(record['source']).convert('RGBA')
            state = client.call('krita_document', action='open', path=str(MASTER/'ora'/f'{i:02}.ora'))
            doc = state['affected_document_id']
            state = client.call('krita_state', document_id=doc)
            if state['runtime_compatibility']['status'] != 'compatible':
                raise RuntimeError(state['runtime_compatibility'])
            added = client.call('krita_layers', action='create',document_id=doc,name='Krita – rysunek krawędzi i refleksy')
            layer = added['active_layer_id']
            decorate(client,doc,layer,base,i)
            client.call('krita_preview',document_id=doc,max_width=512,max_height=512)
            native = MASTER/'native'/f'{i:02}.kra'
            export = MASTER/'native'/name
            client.call('krita_save_export',document_id=doc,kra_path=str(native),png_path=str(export),overwrite=False)
            client.call('krita_document',action='close',document_id=doc,discard_changes=False)
            rgba = Image.open(export).convert('RGBA')
            assert rgba.size==base.size and native.stat().st_size>1000
            rgba.putalpha(base.getchannel('A'))
            result = MASTER/'delivery'/name
            with result.open('xb') as stream:
                rgba.save(stream,format='PNG')
            print(f'Krita saved {name}, exact alpha retained',flush=True)


def contact():
    font = ImageFont.truetype('C:/Windows/Fonts/arial.ttf', 14)
    for page in range(3):
        ids = range(page*10,min((page+1)*10,25))
        sheet = Image.new('RGBA',(1250,270*((len(ids)+4)//5)),(27,34,45,255))
        draw = ImageDraw.Draw(sheet)
        for tile,i in enumerate(ids):
            source = Image.open(RUN/'baseline'/item_name(i)).convert('RGBA')
            result = Image.open(MASTER/'delivery'/item_name(i)).convert('RGBA')
            box = source.getchannel('A').getbbox()
            x,y = tile%5*250,tile//5*270
            for col,image in enumerate((source,result)):
                sprite = image.crop(box)
                sprite.thumbnail((113,220),Image.Resampling.LANCZOS)
                sheet.alpha_composite(sprite,(x+col*122+(122-sprite.width)//2,y+229-sprite.height))
                draw.text((x+col*122+12,y+232),'Przed' if col==0 else 'Po',font=font,fill='#e0cfad')
            draw.text((x+12,y+251),f'{i:02} {DESCRIPTIONS[i].split(",")[0][:28]}',font=font,fill='#e0cfad')
        sheet.convert('RGB').save(RUN/f'comparison-{page+1}.png')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--no-audio', action='store_true', required=True)
    parser.add_argument('action', choices=['prepare','collect','finish','contact'])
    parser.add_argument('parameters', nargs='*')
    args = parser.parse_args()
    if args.action == 'prepare': prepare()
    elif args.action == 'collect': collect(*args.parameters)
    elif args.action == 'finish': finish(list(map(int,args.parameters)))
    else: contact()

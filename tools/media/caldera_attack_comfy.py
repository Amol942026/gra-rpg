"""Prepare one local img2img graph and preserve the Krita alpha in its output."""
import argparse
import json
import shutil
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter


ROLES = {
    'slash': 'a crescent shockwave of violet magic, razor bright lavender cutting edge, broken obsidian splinters, dark purple energy ribbons',
    'breath': 'a long narrow tongue of dragon fire flowing from left to right, luminous gold core, turbulent orange flame curls, glowing embers and dark red smoke',
    'eruption': 'an upward plume of molten lava erupting from a shallow oval crater, fractured basalt rim, white hot center, long flame tongues and suspended red volcanic rocks',
    'cinder': 'one cracked obsidian meteor with molten orange fissures, white hot leading edge on the right and a wispy fire trail tapering to the left',
}
MODEL = 'DreamShaper_8_pruned.safetensors'


def prepare(root, comfy):
    source = Image.open(root / 'krita/caldera-attack-paints.png').convert('RGBA')
    assert source.size == (2048, 512)
    folder = 'emberfall-caldera-attacks-20261109'
    inputs = comfy / 'input' / folder
    inputs.mkdir(exist_ok=False)
    (root / 'inputs').mkdir(exist_ok=False)
    (root / 'comfy').mkdir(exist_ok=False)
    graph = {'1': {'class_type': 'CheckpointLoaderSimple', 'inputs': {'ckpt_name': MODEL}}}
    for index, (role, description) in enumerate(ROLES.items()):
        tile = source.crop((index*512, 0, (index+1)*512, 512))
        tile.save(root / 'inputs' / f'{role}.png')
        background = Image.new('RGBA', tile.size, (17, 14, 23, 255))
        background.alpha_composite(tile)
        background.convert('RGB').save(inputs / f'{role}.png')
        shutil.copy2(inputs / f'{role}.png', root / 'comfy' / f'input-{role}.png')
        start = 10 + index*10
        node = lambda n: str(start+n)
        prompt = ('isolated painted fantasy action RPG spell effect texture, ' + description +
                  ', high contrast, exceptionally fine material details, incandescent gradients, rich hand painted shading, '
                  'angular volcanic shards and very fine sparks, keep the reference exact silhouette and direction, dark plain background')
        negative = ('characters, face, dragon body, people, hands, weapons, icon border, user interface, landscape, '
                    'lettering, watermark, duplicate effect, obscured silhouette, flat vector, oversaturated white blob')
        graph.update({
            node(0): {'class_type': 'LoadImage', 'inputs': {'image': f'{folder}/{role}.png'}},
            node(1): {'class_type': 'CLIPTextEncode', 'inputs': {'clip': ['1', 1], 'text': prompt}},
            node(2): {'class_type': 'CLIPTextEncode', 'inputs': {'clip': ['1', 1], 'text': negative}},
            node(3): {'class_type': 'VAEEncode', 'inputs': {'pixels': [node(0), 0], 'vae': ['1', 2]}},
            node(4): {'class_type': 'KSampler', 'inputs': {'model': ['1', 0], 'positive': [node(1), 0],
                      'negative': [node(2), 0], 'latent_image': [node(3), 0], 'seed': 20261109021+index,
                      'steps': 19, 'cfg': 6.2, 'sampler_name': 'dpmpp_2m', 'scheduler': 'karras', 'denoise': .43}},
            node(5): {'class_type': 'VAEDecode', 'inputs': {'samples': [node(4), 0], 'vae': ['1', 2]}},
            node(6): {'class_type': 'SaveImage', 'inputs': {'images': [node(5), 0], 'filename_prefix': f'{folder}/{role}'}},
        })
    path = root / 'comfy/attack-materials.json'
    path.write_text(json.dumps(graph, indent=2), encoding='utf-8')
    print(path)


def deliver(root, refined):
    output = root / ('painted-refined' if refined else 'painted')
    output.mkdir(exist_ok=False)
    atlas = Image.new('RGBA', (2048, 512))
    for index, role in enumerate(ROLES):
        source_folder = root / 'comfy' / ('outputs-refined' if refined else 'outputs')
        prefix = f'refined-{role}' if refined else role
        candidates = list(source_folder.rglob(f'{prefix}_*.png'))
        assert len(candidates) == 1, (role, candidates)
        original = Image.open(root / 'inputs' / f'{role}.png').convert('RGBA')
        generated = Image.open(candidates[0]).convert('RGB')
        assert generated.size == original.size
        alpha = original.getchannel('A')
        # Keep painted edge glow and sparks. Let diffusion refine the opaque interior.
        weight = np.asarray(alpha.filter(ImageFilter.MinFilter(5)), dtype=float) / 255 * (.89 if refined else .74)
        rgba = np.array(original)
        rgb = np.asarray(generated, dtype=float)
        rgba[:, :, :3] = np.round(rgba[:, :, :3]*(1-weight[:, :, None])+rgb*weight[:, :, None]).astype('uint8')
        result = Image.fromarray(rgba)
        assert result.getchannel('A').tobytes() == alpha.tobytes()
        result.save(output / f'{role}.png')
        atlas.paste(result, (index*512, 0))
    atlas.save(root / ('caldera-attack-fx-refined.png' if refined else 'caldera-attack-fx.png'), optimize=True)
    print('Four ComfyUI textures delivered with native Krita alpha')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('action', choices=['prepare', 'deliver'])
    parser.add_argument('root', type=Path)
    parser.add_argument('--comfy', type=Path)
    parser.add_argument('--refined', action='store_true')
    parser.add_argument('--no-audio', required=True, action='store_true')
    args = parser.parse_args()
    if args.action == 'prepare':
        prepare(args.root, args.comfy)
    else:
        deliver(args.root, args.refined)

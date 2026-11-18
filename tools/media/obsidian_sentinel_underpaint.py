"""Complete the concealed breastplate and belt before rigging the painted sword arm."""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

from obsidian_sentinel_paint import PARTS
from ember_drake_synfig import NAMES


def backing(parts):
    outlines={
        3:((61,39),(70,39),(69,45),(77,50),(77.5,57),(75,64),(75,70),(73,74),
           (53.5,74),(53.5,69),(55,62),(54,56),(53.5,51),(61,46)),
        0:((57,66),(74,65),(78,73),(83,82),(71,86),(62,85),(55,82),(46,80),(52,71)),
    }
    materials={}
    for part,outline in outlines.items():
        plate=Image.open(parts/f'S-{PARTS[part]}.png').convert('RGBA')
        mask=Image.new('L',plate.size)
        ImageDraw.Draw(mask).polygon([(x*plate.width/128,y*plate.height/128) for x,y in outline],fill=255)
        pixels=np.array(plate)
        pixels[:,:,3]=np.minimum(pixels[:,:,3],np.asarray(mask))
        materials[part]=Image.fromarray(pixels)
    return materials


def underpaint(tile,view,part,material):
    gap=min(view,32-view,abs(view-16))
    if part not in material or gap>=4:
        return tile
    key,sub=divmod(view,4)
    def blend(values):
        return values[key]+(values[(key+1)%8]-values[key])*sub/4
    hip=blend((65,64,64,64,62,65,64,64))
    lean=blend((4,2.5,0,-2,-3,-2.5,0,2.5))
    width=blend((.78,.93,1,.93,.76,.93,1,.93))
    east=min(view,32-view)<4
    if not east:
        width=-width
    source=material[part]
    def project(x,y):
        sy=(y*128/tile.height-73-(1 if east else -1))/1.07+73
        sx=65+(x*128/tile.width-hip-lean*(73-sy)/34)/width
        return np.array([sx,sy])*source.width/128
    origin=project(0,0)
    dx,dy=project(1,0)-origin,project(0,1)-origin
    affine=dx[0],dy[0],origin[0],dx[1],dy[1],origin[1]
    painted=np.array(source.transform(tile.size,Image.Transform.AFFINE,affine,Image.Resampling.BICUBIC))
    painted[:,:,:3]=np.rint(painted[:,:,:3]*(.86,.85,.85)).astype('uint8')
    painted[:,:,3]=np.rint(painted[:,:,3]*(1-gap/4))
    output=Image.fromarray(painted)
    output.alpha_composite(tile)
    return output


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('parts',type=Path)
    parser.add_argument('cells',type=Path)
    parser.add_argument('output',type=Path)
    parser.add_argument('--no-audio',action='store_true',required=True)
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=False)
    (args.output/'parts').mkdir()
    material=backing(args.parts)
    for view in range(32):
        for part in range(6):
            name=f'{view:02}-{part}.png'
            tile=Image.open(args.cells/name).convert('RGBA')
            underpaint(tile,view,part,material).save(args.output/name)
            if view%4==0:
                name=f'{NAMES[view//4]}-{PARTS[part]}.png'
                tile=Image.open(args.parts/name).convert('RGBA')
                underpaint(tile,view,part,material).save(args.output/'parts'/name)

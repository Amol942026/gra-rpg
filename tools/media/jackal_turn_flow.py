"""Bake FILM correspondence fields and silhouette distances from the Synfig cels."""

import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import time

import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt, gaussian_filter


WIDTH, HEIGHT = 172, 150
WALK, ATTACK, FALL = 28, 17, 14
FRAMES = WALK+1+ATTACK+2*FALL
PAINTINGS = (0, 1, 2, 6, 7, 8, 9, 10, 11)
FACINGS = ((0,0),(8,0),(1,0),(9,0),(2,0),(9,1),(1,1),(8,1),
           (0,1),(11,1),(7,1),(10,1),(6,0),(10,0),(7,0),(11,0))
GRID = (40, 36)
BLOCK = (10, 8)
ATLAS = ((GRID[0]+2)*BLOCK[0]*4, (GRID[1]+2)*BLOCK[1]*4)
PART_CELL = (60, 152)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def cel(page, frame):
    x,y = 1+frame%9*174, 1+frame//9*152
    return page[y:y+HEIGHT, x:x+WIDTH]


def facing_key(index, pose):
    painting, mirror = FACINGS[index%len(FACINGS)]
    if mirror and pose<WALK:
        pose = (pose+WALK//2)%WALK
    elif mirror and pose>=WALK+1+ATTACK:
        base = WALK+1+ATTACK
        pose = base+(pose-base+FALL)%(2*FALL)
    return painting, mirror, pose


def facing_cel(pages, index, pose):
    painting, mirror, pose = facing_key(index,pose)
    result = cel(pages[painting],pose)
    return result[:, ::-1] if mirror else result


def signed_coverage(alpha):
    mask = alpha>=128
    signed = np.where(mask,distance_transform_edt(mask)-.5,.5-distance_transform_edt(~mask))
    edge = (alpha>0)&(alpha<255)&(np.abs(signed)<=.5)
    signed[edge] = alpha[edge]/255.-.5
    return np.uint8(np.clip(np.rint(128+signed*8),0,255))


def write_distances(pages, destination):
    for name, page in pages.items():
        output = np.zeros(page.shape[:2],dtype=np.uint8)
        for frame in range(FRAMES):
            cel(output,frame)[:] = signed_coverage(cel(page,frame)[...,3])
        Image.fromarray(output).save(destination/f"distance-{name}.png")


def field_rect(pair, pose):
    return (((pair%4)*BLOCK[0]+pose%BLOCK[0])*(GRID[0]+2),
            ((pair//4)*BLOCK[1]+pose//BLOCK[0])*(GRID[1]+2))


def write_rig(root, asset_root, part_root, output):
    import jackal_cels as rig
    import jackal_intermediate_views as turn_views
    from jackal_skin_bake import POSES, prepare

    source = Image.open(root/"jackal-repaint-source.png")
    turn = Image.open(root/"jackal-intermediate-krita.png")
    painted = Image.open(asset_root/"painted-views.png")
    keys = {}
    # Use the very same marked joints and frame poses as the Synfig exporter.
    for view in PAINTINGS:
        if view in turn_views.POSES:
            sprite,bones = turn_views.register(turn,source,painted,view)
        else:
            _,bones,_,_ = prepare(source,POSES[view],view)
            x,y = view%4*320,view//4*288
            sprite = painted.crop((x,y,x+320,y+288)).convert("RGBA")
        raw=np.asarray((POSES | turn_views.POSES)[view][1],dtype=float)
        scale=(bones[0,4]-bones[0,0])/(raw[0,4]-raw[0,0])
        offset=bones[0,0]-raw[0,0]*scale
        yy,xx=np.indices((sprite.height,sprite.width),dtype=float)/2
        border=rig.PchipInterpolator(*zip(*(rig.BELLY | turn_views.BELLY)[view]))
        edge=border((xx-offset[0])/scale[0])*scale[1]+offset[1]
        opacity=np.asarray(sprite.getchannel("A"))
        contour=np.column_stack(np.where(np.clip((edge-yy)/1.4+.63,0,1)[::2,::2]*opacity[::2,::2]>96))[:,::-1]
        for frame in range(FRAMES):
            _,roll,pose=rig.pose(bones,contour,view,frame)
            joints=(np.asarray(pose)-[rig.LEFT,rig.TOP])*[WIDTH/rig.WIDTH,HEIGHT/rig.HEIGHT]
            poses=[]
            for slot,limb in enumerate(joints):
                kind,phase,_=rig.action(frame)
                pitch=(np.degrees(roll) if kind=="fall" else
                       -rig.step(phase,slot)[2]*12*np.cos(rig.ANGLES[view]) if kind=="walk" else 0.)
                poses.extend((*limb.ravel(),pitch))
            keys[view,frame]=poses

    atlas=Image.new("RGBA",(8*PART_CELL[0],PART_CELL[1]))
    descriptors=[]
    for group,slot in enumerate((0,2)):
        for part in range(4):
            source_name=f"leg-0-{slot}-{part}.png" if part<3 else f"paw-0-{slot}.png"
            with Image.open(part_root/source_name) as source_image:
                image=source_image.convert("RGBA")
            if image.width>PART_CELL[0]-2 or image.height>PART_CELL[1]-2:
                raise ValueError(f"Przygotowana część jest za duża: {source_name}")
            x=(group*4+part)*PART_CELL[0]+1
            atlas.paste(image,(x,1))
            if part<3:
                widths=((6.65,4.7,3.08),(3.15,2.53,2.58),(2.48,2.78,2.40)) if slot==0 else (
                        (5.05,4.48,3.05),(3.05,2.48,2.54),(2.55,2.79,2.40))
                half=max(widths[part])+1.1
                lead,trail=((3.15,2.15),(2.4,1.55),(1.50,.88))[part]
            else:
                half,height=(3.9,3.5) if slot==0 else (3.7,3.2)
                lead,trail=height*1.25,height*.14
            descriptors.append((x,1,image.width,image.height,half*WIDTH/rig.WIDTH,
                                lead*HEIGHT/rig.HEIGHT,trail*HEIGHT/rig.HEIGHT))
    atlas.save(output/"turn-limbs.png")
    with (output/"turn-rig.bin").open("wb") as stream:
        stream.write(b"JTR1")
        for part in descriptors:
            stream.write(struct.pack("<7f",*part))
        for view in PAINTINGS:
            for frame in range(FRAMES):
                stream.write(struct.pack("<36f",*keys[view,frame]))


def bake(pages, comfy, weights, device, batch_size):
    sys.path.insert(0,str(comfy))
    import torch
    import torch.nn.functional as F
    from safetensors.torch import load_file
    from comfy_extras.frame_interpolation_models.film_net import FILMNet, flow_pyramid_synthesis

    if device=="cpu":
        torch.set_num_threads(4)
    dtype = torch.float16 if device=="cuda" else torch.float32
    model = FILMNet()
    model.load_state_dict(load_file(str(weights)))
    model = model.eval().to(device=device, dtype=dtype)
    torch.backends.cudnn.allow_tf32 = True
    pad = (10,10,5,5)  # preserve the original cel coordinate origin at the crop
    model._build_warp_grids(HEIGHT+pad[2]+pad[3], WIDTH+pad[0]+pad[1], torch.device(device))

    def batch_input(cels):
        rgba = torch.from_numpy(np.stack(cels)).to(device=device,dtype=dtype).movedim(-1,1)/255
        alpha = rgba[:,3:4]
        # A neutral dark matte gives the optical model a clean contour signal.
        matte = rgba[:,:3]*alpha + (1-alpha)*.072
        return F.pad(matte,pad,value=.072)

    def correspondence(a,b):
        prediction = flow_pyramid_synthesis(model.predict_flow(a,b,model.warp))[0].float()
        return prediction[...,pad[2]:pad[2]+HEIGHT,pad[0]:pad[0]+WIDTH]

    out = np.full((ATLAS[1],ATLAS[0],4),128,dtype=np.uint8)
    pairs = [(i,j) for i in range(len(FACINGS)) for j in range(FRAMES)]
    with torch.inference_mode():
        for offset in range(0,len(pairs),batch_size):
            items = pairs[offset:offset+batch_size]
            a = batch_input([facing_cel(pages,i,j) for i,j in items])
            b = batch_input([facing_cel(pages,i+1,j) for i,j in items])
            _, a_keys = model.extract_features(a)
            _, b_keys = model.extract_features(b)
            both = torch.cat((correspondence(a_keys,b_keys),correspondence(b_keys,a_keys)),dim=1)
            if not torch.isfinite(both).all():
                raise ValueError(f"Invalid FILM motion at pair {items[0]}")
            vectors = F.interpolate(both,size=GRID[::-1],mode="bilinear",align_corners=False).movedim(1,-1).cpu().numpy()
            for (direction,pose),field in zip(items,vectors):
                x,y = field_rect(direction,pose)
                field = gaussian_filter(field,sigma=(.42,.42,0),mode="nearest")
                values = np.uint8(np.clip(np.rint(128+field*2),0,255))
                out[y:y+GRID[1]+2,x:x+GRID[0]+2] = np.pad(values,((1,1),(1,1),(0,0)),mode="edge")
            if offset%96==0:
                print(f"FILM: {offset+len(items)}/{len(pairs)} par",flush=True)
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--comfy", type=Path, required=True)
    parser.add_argument("--weights", type=Path, required=True)
    parser.add_argument("--assets", type=Path, default=Path("assets/desert/jackal-turn"))
    parser.add_argument("--sources", type=Path, default=Path("art/desert/new-enemies"))
    parser.add_argument("--body-render", type=Path, required=True)
    parser.add_argument("--device", choices=("cuda","cpu"), default="cuda")
    parser.add_argument("--batch-size", type=int, default=6)
    parser.add_argument("--no-audio", action="store_true",required=True)
    args = parser.parse_args()
    args.root.mkdir(parents=True)
    body_art = args.body_render/"atlases"
    pages = {name:np.asarray(Image.open(body_art/f"body-{name}.png").convert("RGBA")) for name in PAINTINGS}
    started = time.monotonic()
    field = bake(pages,args.comfy,args.weights,args.device,args.batch_size)
    Image.fromarray(field).save(args.root/"turn-fields.png")
    seconds = time.monotonic()-started
    write_distances(pages,args.root)
    for item in sorted(body_art.glob("body-*.png")):
        (args.root/item.name).write_bytes(item.read_bytes())
    write_rig(args.sources,args.assets,args.body_render/"parts",args.root)
    report = {"model":"ComfyUI FILM / bidirectional displacement", "weights_sha256":sha(args.weights),
              "direction_pairs":len(FACINGS),"poses":FRAMES,"flow_seconds":round(seconds,3),
              "generation_seconds":round(time.monotonic()-started,3),
              "inputs":{p.name:sha(p) for p in sorted(body_art.glob("body-*.png"))},
              "data":{p.name:sha(p) for p in sorted(args.root.iterdir()) if p.is_file()}}
    (args.root/"receipt.json").write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8")


if __name__=="__main__":
    main()

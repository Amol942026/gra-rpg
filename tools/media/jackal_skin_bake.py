"""Prepare the jackal's painted views and a continuous 2D skin for each view."""

import argparse
import io
import math
import struct
from pathlib import Path
from xml.etree import ElementTree as ET
from zipfile import ZipFile, ZIP_DEFLATED, ZIP_STORED

import numpy as np
from PIL import Image
from scipy.ndimage import binary_dilation, label


WIDTH, HEIGHT, TEXELS, STEP = 160, 144, 2, 4
ORIGIN = (80, 119)
VIEW_ANGLES = (0, math.atan(.5), math.pi/2, math.pi-math.atan(.5), math.pi,
               math.pi+math.atan(.5), 3*math.pi/2, 2*math.pi-math.atan(.5))
TAILS = {
    0: ((143,198),(48,365)),
    1: ((646,164),(588,324)),
    7: ((183,687),(139,901)),
    6: ((761,723),(762,930)),
}
# Hip, stifle, hock, base of the toes, toe tip on the hind legs. The
# corresponding forelimb landmarks are scapula, shoulder, elbow, carpus, toe.
# Both pairs are ordered right, left in the animal's frame of reference.
POSES = {
    0: ((0, 0, 550, 484), (
        ((169,202),(151,279),(96,352),(93,414),(108,437)),
        ((190,204),(193,277),(142,358),(157,400),(179,421)),
        ((333,172),(366,218),(349,284),(353,392),(375,429)),
        ((317,187),(344,226),(336,285),(339,383),(366,415)),
    ), ((371,182),(472,97),(523,128))),
    1: ((555, 0, 974, 484), (
        ((669,164),(653,244),(619,312),(622,381),(634,401)),
        ((702,167),(717,251),(680,330),(697,371),(723,385)),
        ((774,161),(811,217),(795,284),(798,415),(810,444)),
        ((831,177),(879,232),(864,285),(865,390),(885,420)),
    ), ((824,203),(891,104),(935,162))),
    2: ((1110, 0, 1404, 484), (
        ((1196,211),(1191,269),(1189,323),(1175,393),(1175,416)),
        ((1270,211),(1267,261),(1266,321),(1272,381),(1286,410)),
        ((1220,177),(1231,231),(1233,302),(1227,418),(1224,449)),
        ((1323,176),(1320,231),(1322,298),(1330,414),(1341,449)),
    ), ((1276,218),(1282,92),(1282,159))),
    7: ((73, 503, 488, 1009), (
        ((189,713),(157,799),(118,877),(119,932),(125,957)),
        ((224,709),(224,807),(184,886),(192,943),(216,969)),
        ((333,676),(367,750),(362,807),(368,895),(389,921)),
        ((290,693),(320,749),(300,808),(292,884),(308,911)),
    ), ((359,638),(418,594),(454,623))),
    6: ((653, 503, 875, 1009), (
        ((807,728),(808,808),(813,897),(817,945),(823,976)),
        ((719,729),(714,808),(709,895),(710,945),(704,975)),
        ((806,671),(818,730),(824,811),(826,897),(827,922)),
        ((718,671),(707,730),(704,811),(697,893),(704,923)),
    ), ((759,629),(761,559),(762,548))),
}


def smoothstep(edge0, edge1, xs):
    t = np.clip((xs-edge0)/(edge1-edge0), 0, 1)
    return t*t*(3-2*t)


def prepare(image, spec, direction):
    cut, legs, face = spec
    image = image.crop(cut)
    rgba = np.array(image)
    alpha = rgba[..., 3]
    components, _ = label(alpha >= 80, structure=np.ones((3, 3), dtype=bool))
    areas = np.bincount(components.ravel())
    areas[0] = 0
    keep = binary_dilation(components == areas.argmax(), iterations=2)
    rgba[..., 3] = np.where(keep, np.minimum(255, alpha.astype(np.uint16)*255//239), 0)
    animal = Image.fromarray(rgba)
    bounds = animal.getchannel("A").point(lambda a: 255 if a >= 110 else 0).getbbox()
    legs = np.asarray(legs, dtype=np.float32) - cut[:2]
    face = np.asarray(face, dtype=np.float32) - cut[:2]
    feet = legs[:, -1].mean(axis=0)
    height = 88.0 + 1.8 * abs(math.sin(VIEW_ANGLES[direction]))
    scale = height/(bounds[3]-bounds[1])
    # Marker for the middle of the stance, not the far tips of the nose and tail.
    place = np.array(ORIGIN)-feet*scale
    offset = tuple(-place/scale)
    transform = (1/(scale*TEXELS),0,offset[0],0,1/(scale*TEXELS),offset[1])
    painted = animal.transform((WIDTH*TEXELS, HEIGHT*TEXELS), Image.Transform.AFFINE,
                               transform, Image.Resampling.BICUBIC)
    tail = TAILS.get(direction)
    tail = (np.asarray(tail,dtype=np.float32)-cut[:2])*scale+place if tail else None
    return painted, legs*scale+place, face*scale+place, tail


def direction_views(image):
    views = {d:prepare(image, spec, d) for d,spec in POSES.items()}
    for dest, source in ((3,1),(4,0),(5,7)):
        sprite, legs, face, tail = views[source]
        legs = legs[[1,0,3,2]].copy()
        legs[...,0] = WIDTH-legs[...,0]
        face = face.copy()
        face[:,0] = WIDTH-face[:,0]
        if tail is not None:
            tail = tail.copy()
            tail[:,0] = WIDTH-tail[:,0]
        views[dest] = sprite.transpose(Image.Transpose.FLIP_LEFT_RIGHT),legs,face,tail
    return [views[i] for i in range(8)]


def weights(points, legs, face, tail):
    distances, positions = [], []
    for chain in legs:
        lengths, offsets = [], []
        for start, end in zip(chain, chain[1:]):
            line = end-start
            t = np.clip((points-start) @ line / (line @ line), 0, 1)
            distances_to_edge = np.linalg.norm(points-start-t[:,None]*line, axis=1)
            lengths.append(distances_to_edge)
            offsets.append(t)
        distances.append(np.min(lengths, axis=0))
        closest = np.argmin(lengths, axis=0)
        nearest_t = np.asarray(offsets)[closest,np.arange(len(points))]
        positions.append(closest + nearest_t)
    distances, positions = np.array(distances),np.array(positions)
    # Keep the edge of a paw moving with its center. Distance mainly selects
    # the contributing leg. The influence eases out at the socket and into
    # the transparent border.
    nearest = distances.min(axis=0)
    support = np.exp(-(distances**2-nearest**2)/13)
    support /= support.sum(axis=0)
    support *= (1-smoothstep(7,16,nearest))*smoothstep(.14,1.08,positions)
    bone_weights = []
    for along, w in zip(positions, support):
        for center in (.5,1.5,2.5,3.5):
            factor = np.clip(1-np.abs(np.clip(along,.5,3.5)-center),0,1)
            bone_weights.append(w * factor)
    head_axis = face[1]-face[0]
    head = smoothstep(.05,.88,(points-face[0]) @ head_axis / (head_axis @ head_axis))
    head *= 1-smoothstep(18,29,np.linalg.norm(points-face[1],axis=1))
    wag = np.zeros(len(points))
    if tail is not None:
        tangent = tail[1]-tail[0]
        t = np.clip((points-tail[0]) @ tangent / (tangent @ tangent),0,1)
        distance = np.linalg.norm(points-tail[0]-t[:,None]*tangent,axis=1)
        wag = smoothstep(.16,1,t)*(1-smoothstep(4.5,11.5,distance))
        wag *= 1-np.clip(np.sum(bone_weights,axis=0)*.95,0,1)
    w = np.column_stack(bone_weights)
    w *= (1-head[:,None])*(1-wag[:,None]*.45)
    return np.uint8(np.rint(np.column_stack((w,head,wag))*255))


def topology(sprite):
    low = sprite.getchannel("A").resize((WIDTH,HEIGHT), Image.Resampling.LANCZOS)
    alpha = binary_dilation(np.asarray(low)>11)
    active = {}
    indices = []
    ly,lx = np.indices((STEP,STEP))
    for y in range(0,HEIGHT,STEP):
        for x in range(0,WIDTH,STEP):
            area = alpha[y:y+STEP,x:x+STEP]
            if (area & (lx>=ly)).any():
                indices.extend(active.setdefault(p,len(active)) for p in ((x,y),(x+STEP,y+STEP),(x+STEP,y)))
            if (area & (ly>=lx)).any():
                indices.extend(active.setdefault(p,len(active)) for p in ((x,y),(x,y+STEP),(x+STEP,y+STEP)))
    return np.asarray(list(active),dtype=np.float32),indices


def write_skin(views, target):
    with target.open("wb") as out:
        out.write(b"JSK3")
        for sprite, legs, face, tail in views:
            points, indices = topology(sprite)
            influences = weights(points,legs,face,tail)
            out.write(struct.pack("<HH",len(points),len(indices)))
            for point in np.concatenate((legs.reshape(-1,2),face)):
                out.write(struct.pack("<ff",*point))
            for (x,y),w in zip(points,influences):
                out.write(struct.pack("<BB18B",int(x),int(y),*w))
            out.write(struct.pack(f"<{len(indices)}H",*indices))


def atlas(views, master):
    size = (4*WIDTH*TEXELS,2*HEIGHT*TEXELS)
    sheet = Image.new("RGBA",size)
    root = ET.Element("image",w=str(size[0]),h=str(size[1]),name="Szakal wydmowy",version="0.0.3")
    stack = ET.SubElement(root,"stack")
    with ZipFile(master,"x",compression=ZIP_DEFLATED) as archive:
        archive.writestr("mimetype","image/openraster",compress_type=ZIP_STORED)
        for i,(sprite,_,_,_) in enumerate(views):
            x,y = (i%4)*WIDTH*TEXELS, (i//4)*HEIGHT*TEXELS
            path = f"data/{i}.png"
            ET.SubElement(stack,"layer",name=("Wschód","Południowy wschód","Południe","Południowy zachód",
                                             "Zachód","Północny zachód","Północ","Północny wschód")[i],
                          src=path,x=str(x),y=str(y),visibility="visible",opacity="1.0",
                          **{"composite-op":"svg:src-over"})
            data = io.BytesIO()
            sprite.save(data,format="PNG")
            archive.writestr(path,data.getvalue())
            sheet.paste(sprite,(x,y))
        data = io.BytesIO()
        sheet.save(data,format="PNG")
        archive.writestr("mergedimage.png",data.getvalue())
        archive.writestr("stack.xml",ET.tostring(root))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source",type=Path)
    parser.add_argument("work",type=Path)
    parser.add_argument("--rig-only",action="store_true")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    args.work.mkdir(parents=True,exist_ok=True)
    with Image.open(args.source) as image:
        views = direction_views(image.convert("RGBA"))
    if not args.rig_only:
        atlas(views,args.work/"szakal-skora.ora")
    write_skin(views,args.work/"painted-skin.bin")


if __name__ == "__main__":
    main()

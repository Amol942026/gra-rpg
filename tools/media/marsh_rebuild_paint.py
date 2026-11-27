"""Register the complete painted views used in the new toad and keeper rigs."""
from argparse import ArgumentParser
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage as ndi
from scipy.interpolate import PchipInterpolator


METRICS = {
    0: (140, 100, 70, 92, (70, 70, 62, 69, 69)),
    3: (204, 164, 102, 159, (139, 140, 139, 140, 140)),
}
EXTRA_YAW = (15, 30, 60, 75, 105, 120, 150, 165)


def complete_paint(image):
    """Keep the main body and close subpixel gaps along the painted outline."""
    label, count = ndi.label(image[..., 3] > 21, np.ones((3, 3)))
    if not count:
        raise ValueError("Empty painted silhouette")
    areas = np.bincount(label.ravel())
    areas[0] = 0
    solid = label == areas.argmax()
    return np.where(ndi.binary_dilation(solid, iterations=1), image[..., 3], 0)


def keys(path, species):
    width, height, ax, ay, heights = METRICS[species]
    result = []
    atlas = Image.open(path).convert("RGBA")
    for pose in range(5):
        x, y = pose % 3 * 512, pose // 3 * 512
        cell = np.array(atlas.crop((x, y, x + 512, y + 512)))
        cell[..., 3] = complete_paint(cell)
        body = Image.fromarray(cell)
        body = body.crop(body.getchannel("A").getbbox())
        h = heights[pose]
        w = round(body.width * h / body.height)
        if width-w < 14:
            raise ValueError("Directional painting overflows the authored cell")
        frame = Image.new("RGBA", (width, height))
        body = body.resize((w, h), Image.Resampling.LANCZOS)
        # Slight anatomical offset in profile, centred on the weight-bearing hips.
        dx = ((0, -1, -2, 1, 0) if species == 0 else (0, -1, -2, 2, 0))[pose]
        frame.paste(body, (ax - w//2+dx, ay-h))
        result.append(np.array(frame))
    front, diagonal, profile, rear, back = result
    return [profile[:, ::-1], diagonal[:, ::-1], front, diagonal, profile, rear, back, rear[:, ::-1]]


def register(frame):
    opacity = frame[..., 3]
    mask = opacity > 120
    (_, edge_index) = ndi.distance_transform_edt(~mask, return_indices=True)
    pigment = frame[tuple(edge_index)][..., :3].astype(np.float32)
    inside = ndi.distance_transform_edt(mask)-.5
    outside = ndi.distance_transform_edt(~mask)-.5
    sdf = np.where(mask,inside,-outside)
    limits = [(row, columns.min(), columns.max()) for row, solid in enumerate(mask)
              if (columns := np.flatnonzero(solid)).size]
    rows, left, right = np.asarray(limits).T
    dy = np.arange(len(mask))
    center = ndi.gaussian_filter1d(np.interp(dy, rows, (left+right)/2), 1.65)
    span = ndi.gaussian_filter1d(np.interp(dy, rows, np.maximum(7, (right-left)/2)), 1.65)
    return center-span, center+span, sdf, pigment


def smooth(t):
    x=np.clip(t,0.,1.)
    return x*x*(3-2*x)


def cycle(values,headings=None):
    # Adjacent angular intervals share their tangent, including 359 -> 0.
    if headings is None:
        headings=np.arange(8,dtype=float)*45.
    knots=np.concatenate((headings[-2:]-360.,headings,headings[:3]+360.))
    periodic=np.concatenate((values[-2:],values,values[:3]))
    return PchipInterpolator(knots,periodic,axis=0)


def extra_keys(path, base, species):
    """Place the new complete paintings on the existing projected footprint."""
    width, height, anchor_x, anchor_y, _ = METRICS[species]
    sheet = Image.open(path).convert("RGBA")
    if sheet.size != (2048, 1024):
        raise ValueError("Expected eight painted 512 × 512 direction cells")
    extents = []
    for frame in base:
        alpha = frame[..., 3]
        left, top, right, bottom = Image.fromarray(alpha).getbbox()
        extents.append((right - left, bottom - top, alpha.sum() / 255.))
    footprint = cycle(np.asarray(extents))
    for index, yaw in enumerate(EXTRA_YAW):
        left, top = index % 4 * 512, index // 4 * 512
        pixels = np.array(sheet.crop((left, top, left + 512, top + 512)))
        pixels[..., 3] = complete_paint(pixels)
        body = Image.fromarray(pixels)
        body = body.crop(body.getchannel("A").getbbox())
        angle = 90 + yaw
        projected_width, projected_height, projected_area = footprint(angle)
        area = np.asarray(body.getchannel("A")).sum() / 255.
        # Shape comes from the new artwork. The width and height reflect the
        # same creature and its original ground projection across headings.
        fill_ratio = area / (body.width * body.height)
        fitted_width = np.clip(
            projected_area / (projected_height * fill_ratio),
            projected_width * .94, projected_width * 1.06,
        )
        body = body.resize((round(fitted_width), round(projected_height)), Image.Resampling.LANCZOS)
        frame = Image.new("RGBA", (width, height))
        frame.paste(body, (anchor_x - body.width // 2, anchor_y - body.height))
        painted = np.asarray(frame)
        yield angle, painted
        yield (90 - yaw) % 360, painted[:, ::-1]


def inbetween(a,b,shape,anchors,t,dense):
    """Build one outline on the registered anatomy and pass paint through it."""
    h,w=a[0][2].shape
    y,x=np.indices((h,w),dtype=np.float32)
    lo,hi,pivot=shape
    rel=(x-pivot[:,None])/np.where(x<pivot[:,None],(pivot-lo)[:,None],(hi-pivot)[:,None])
    def sample(key):
        registration,local=key
        sdf,pigment=registration[2:]
        ys=np.interp(y[:,0],anchors,local[:5])
        # The silhouette limits and internal eye/bell station use the same
        # vertical remap, so changing heading cannot drag the face ahead.
        ly,ry,mid=key_shapes(registration,local,h)
        lower,upper,center=(np.interp(ys,np.arange(h),c)[:,None] for c in (ly,ry,mid))
        xs=center+rel*np.where(rel<0,center-lower,upper-center)
        at=(np.broadcast_to(ys[:,None],(h,w)),xs)
        def channel(image):
            return ndi.map_coordinates(image, at, order=1, mode="nearest", prefilter=False)
        return channel(sdf),np.stack([channel(pigment[...,c]) for c in range(3)],axis=2)
    da,ca=sample(a)
    db,cb=sample(b)
    turn=smooth(t)
    sdf=da+(db-da)*turn
    # Reveal paint across the surface over several bearings. The same outline
    # is rasterized once, with no hard, whole-body pigment switch at mid-turn.
    bias=np.clip((db-da)/8,-.25,.25)*4*t*(1-t)
    mix=smooth((t-.23+.12*np.clip(rel,-1,1)+bias)/.54)[...,None]
    if dense:
        # Distribute each appearance change throughout the full 15-degree span.
        mix = (.75*t + .25*turn + .16*bias)[...,None]
    return sdf,ca+(cb-ca)*mix


def key_shapes(registration,local,height):
    lo,hi=registration[:2]
    y=np.arange(height)
    top,point,base=local[[1,2,3]]
    center=(lo+hi)*.5
    pull=np.exp(-((y-point)/((base-top)*.21))**2)
    return lo,hi,center+(hi-lo)*local[5]*pull


def render(sdf,rgb,area):
    # Clip at a smoothly varying projected mass rather than allowing the two
    # differently painted silhouettes to temporarily shrink between bearings.
    lo,hi=-2.5,2.5
    for _ in range(13):
        level=(lo+hi)/2
        if np.clip(.5+sdf-level,0,1).sum()>area:lo=level
        else:hi=level
    opacity=np.clip(.5+sdf-(lo+hi)/2,0,1)*255
    out=np.rint(np.dstack((rgb,opacity))).clip(0,255).astype("u1")
    out[..., 3] = complete_paint(out)
    return out


def stabilize_pigment(frames,shapes,anchors,shifts,directory,dense):
    """Stabilize fine paint in coordinates attached to the turning anatomy."""
    count=len(frames)
    height,width=frames[0].shape[:2]
    y,x=np.indices((height,width),dtype="f4")
    rows=np.arange(height)
    # Separate fine bark and skin texture from the large painted features.
    separation=1.35 if dense else .96
    detail=np.empty((count,height,width,3),dtype="f4")
    for i,frame in enumerate(frames):
        color=frame[...,:3].astype("f4")
        detail[i]=color-ndi.gaussian_filter(color,(separation,separation,0))
    steps=range(-48,49,8) if dense else range(-24,25,8)
    weights=np.exp(-.5*(np.asarray(steps)/(21 if dense else 11))**2).astype("f4")
    weights/=weights.sum()

    for i,frame in enumerate(frames):
        left,right,center=shapes[i]
        xx=x-shifts[i]
        signed=(xx-center[:,None])/np.where(xx<center[:,None],(center-left)[:,None],(right-center)[:,None])
        paint=np.zeros((height,width,3),dtype="f4")
        for delta,weight in zip(steps,weights):
            j=(i+delta)%count
            if not delta:
                paint+=detail[i]*weight
                continue
            sy=np.interp(y[:,0],anchors[i],anchors[j])
            lo,hi,mid=(np.interp(sy,rows,v)[:,None] for v in shapes[j])
            sx=mid+shifts[j]+signed*np.where(signed<0,mid-lo,hi-mid)
            coordinates=(np.broadcast_to(sy[:,None],(height,width)),sx)
            for channel in range(3):
                paint[...,channel]+=weight*ndi.map_coordinates(
                    detail[j,...,channel],coordinates,order=1,mode="nearest",prefilter=False)
        rgba=frame.copy()
        # Keep the large features and coverage. Filter only the small detail
        # which belongs to the same part of the body in adjoining bearings.
        rgb=frame[...,:3].astype("f4")+.86*(paint-detail[i])
        rgba[...,:3]=np.rint(rgb).clip(0,255).astype("u1")
        Image.fromarray(rgba).save(directory/f"{i:03}.png",compress_level=2)


def build(name, path, directory, extra=None):
    directory.mkdir(parents=True, exist_ok=True)
    base = keys(path,name)
    painted=dict(zip(range(0,360,45),base))
    if extra is not None:
        painted.update(extra_keys(extra,base,name))
    headings=np.array(sorted(painted),dtype=float)
    views=[painted[int(angle)] for angle in headings]
    tracks = list(map(register, views))
    eye_keys={0:(.31,.44,.50,.44,.31,.31,.32,.31),
              3:(.29,.30,.29,.30,.29,.28,.28,.28)}[name]
    # The bell or eye line is inside the common silhouette. On a rear view the
    # station follows the covered attachment point.
    side_keys=(.29,.17,0.,-.17,-.29,-.23,0.,.23) if name==0 else (.31,.15,0.,-.15,-.31,-.25,0.,.25)
    eyepoint=cycle(np.asarray(eye_keys))(headings)
    lateral=cycle(np.asarray(side_keys))(headings)
    locals=[]
    area=[]
    shape=[]
    centers=[]
    for image,track,eye,side in zip(views,tracks,eyepoint,lateral):
        mask=image[...,3]>120
        yy=np.flatnonzero(mask.any(axis=1))
        h=image.shape[0]
        top,base=yy[0],yy[-1]
        point=top+(base-top)*eye
        local=np.array((0,top,point,base,h-1,side))
        locals.append(local)
        alpha=image[...,3].astype("f4")
        area.append(alpha.sum()/255)
        centers.append(np.sum(alpha*np.arange(image.shape[1]))/alpha.sum())
        shape.append(key_shapes(track,local,h))
    silhouettes=cycle(np.asarray(shape),headings)
    angles=cycle(np.asarray(locals)[:,:5],headings)
    # The authored in-between views inherit the original smooth progression
    # of projected size and balance, including the eastern wrap.
    cardinal=np.asarray(headings%45==0.)
    masses=cycle(np.asarray(area)[cardinal])
    balance=cycle(np.asarray(centers)[cardinal])
    frames=[]
    shapes=[]
    anchors=[]
    shifts=[]
    for angle in range(512):
        heading=angle*360./512
        bearing=int(np.searchsorted(headings,heading,side="right")-1)
        successor=(bearing+1)%len(headings)
        end=headings[successor] if successor else 360.
        fraction=(heading-headings[bearing])/(end-headings[bearing])
        shape=silhouettes(heading)
        stations=angles(heading)
        sdf,rgb=inbetween((tracks[bearing],locals[bearing]),(tracks[successor],locals[successor]),
                           shape,stations,fraction,extra is not None)
        pixels=render(sdf,rgb,masses(heading))
        alpha=pixels[...,3].astype("f4")
        actual=np.sum(alpha*np.arange(alpha.shape[1]))/alpha.sum()
        # Preserve the point of balance while the shoulders change projection.
        shift=balance(heading)-actual
        pixels=ndi.shift(pixels,(0,shift,0),order=1,mode="nearest",prefilter=False)
        if min(Image.fromarray(pixels[..., 3]).getbbox()[:2]) < 3:
            raise ValueError("Directional paint is too close to an atlas edge")
        frames.append(pixels)
        shapes.append(shape)
        anchors.append(stations)
        shifts.append(shift)
    stabilize_pigment(frames,shapes,anchors,shifts,directory,extra is not None)
    return headings.tolist()


if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--toad", type=Path, required=True)
    parser.add_argument("--keeper", type=Path, required=True)
    parser.add_argument("--toad-extra", type=Path)
    parser.add_argument("--keeper-extra", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    for species,path,extra in ((0,args.toad,args.toad_extra),(3,args.keeper,args.keeper_extra)):
        headings=build(species,path,args.output/"frames"/str(species),extra)
        print(f"{species}: {len(headings)} complete directional keys:",headings)

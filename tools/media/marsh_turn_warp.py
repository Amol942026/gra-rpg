"""Register the five painted 2D swamp views across the twenty Blender timelines.

Each output texel records a motion vector in the halfway pose, plus the time at
which the rotating surface reveals the next painting. Directions are already
screen-oriented. All landmarks animate with one material location on each rig.
"""
import argparse
import hashlib
import json
import os
import re
import time
from pathlib import Path

os.environ["OPENBLAS_NUM_THREADS"] = "1"
import numpy as np
from PIL import Image, ImageFilter

from scipy.ndimage import distance_transform_edt, gaussian_filter

GRID = 32
PITCH = .75
HEADINGS = ((2,True),(3,True),(4,False),(3,False),
            (2,False),(1,False),(0,False),(1,True))

# Coordinates are percentages of the neutral painting's UV rectangle. Each
# column follows the same anatomical feature over the five painted angles.
# Pairs on the axial views exchange near/far features on the other half-turn.
TOAD = (
    # snout, two eyes, throat, nape, back, two wrists, two fore toes, hind hocks, hind toes
    ((50,9),(41,7),(60,7),(50,28),(50,30),(50,58),(14,45),(86,45),(4,49),(96,49),(25,69),(76,69),(12,91),(88,91)),
    ((9,17),(15,12),(34,9),(18,33),(44,25),(66,45),(17,59),(87,55),(5,76),(96,75),(44,77),(80,63),(43,95),(88,76)),
    ((3,39),(16,26),(26,10),(19,54),(40,26),(61,30),(29,72),(19,55),(16,92),(7,73),(79,66),(88,58),(69,87),(92,79)),
    ((4,25),(23,19),(30,9),(22,45),(46,25),(63,32),(28,74),(14,58),(26,94),(5,75),(75,65),(89,62),(64,81),(94,81)),
    ((50,40),(35,33),(65,33),(50,58),(50,14),(50,17),(23,68),(77,68),(22,93),(78,93),(13,56),(87,56),(5,76),(95,76)),
)
TOAD_PAIRS = ((1,2),(6,7),(8,9),(10,11),(12,13))

LIGHT = (
    # bead, top/bottom and two edges of pod, stem hook, two upper leaves,
    # dangling root and a fold on each side.
    ((40,34),(40,21),(40,45),(26,35),(54,34),(43,4),(8,35),(88,36),(44,96),(33,72),(62,65)),
    ((25,35),(25,23),(26,46),(13,35),(40,35),(31,4),(5,37),(89,32),(34,96),(24,71),(56,67)),
    ((20,39),(22,27),(21,50),(8,40),(38,40),(28,5),(4,42),(86,35),(48,96),(31,75),(68,65)),
    ((24,33),(25,22),(25,45),(8,34),(39,35),(30,5),(2,33),(79,30),(33,96),(25,73),(62,64)),
    ((44,36),(43,24),(43,48),(28,37),(59,37),(45,5),(4,43),(91,36),(39,96),(29,71),(64,68)),
)
LIGHT_PAIRS = ((3,4),(6,7),(9,10))

KEEPER = (
    # bell crown, mouth rim (left, middle, right), crown of shoulders,
    # shoulders, elbows, hands, hips, knees, feet.
    ((46,11),(33,28),(47,31),(59,30),(51,9),(28,25),(70,23),(13,48),(85,46),(9,78),(92,78),(41,53),(62,55),(31,72),(70,75),(26,94),(77,94)),
    ((28,12),(9,24),(19,29),(36,33),(52,10),(27,24),(68,25),(16,49),(78,50),(9,78),(66,81),(39,52),(70,60),(31,75),(80,78),(28,92),(85,96)),
    ((27,12),(2,34),(16,37),(34,39),(53,10),(30,24),(65,24),(36,48),(64,49),(32,75),(50,82),(38,55),(72,56),(34,70),(78,74),(23,94),(81,95)),
    ((28,12),(6,32),(22,35),(41,36),(54,10),(30,25),(72,23),(12,50),(85,48),(8,83),(70,83),(38,54),(71,57),(31,73),(80,74),(28,94),(82,94)),
    ((33,13),(20,38),(34,37),(49,38),(52,9),(23,27),(73,26),(13,48),(88,45),(10,78),(89,77),(34,53),(69,54),(30,74),(73,73),(28,94),(76,94)),
)
KEEPER_PAIRS = ((1,3),(5,6),(7,8),(9,10),(11,12),(13,14),(15,16))

# Mouth-to-tail spine, followed by the local half-width in the neutral cell.
# Section normals rotate with the spine instead of matching the rows of an
# upright sucker to the rows of a horizontal, coiled animal.
LEECH = (
    ((86,13,4),(70,21,10),(53,36,16),(46,55,19),(48,73,15),(48,90,4)),
    ((6,19,5),(19,23,9),(39,33,13),(68,47,14),(86,74,10),(45,92,2)),
    ((7,76,7),(20,67,12),(39,46,14),(62,35,14),(84,43,10),(78,81,2)),
    ((10,76,8),(20,63,13),(37,40,15),(57,30,14),(80,44,11),(65,78,2)),
    ((26,84,9),(26,65,13),(34,40,12),(52,30,10),(75,38,7),(69,63,2)),
)


def rig_sample(grid, uv):
    rows, cols = grid.shape[-3:-1]
    loc = np.clip(uv,0,1)*[cols-1,rows-1]
    x,y = np.minimum(loc.astype(int), [cols-2,rows-2]).T
    tx,ty = (loc-np.column_stack((x,y))).T[...,None]
    return ((1-tx)*(1-ty)*grid[...,y,x,:]+tx*(1-ty)*grid[...,y,x+1,:]
            +(1-tx)*ty*grid[...,y+1,x,:]+tx*ty*grid[...,y+1,x+1,:])


def marks(species, view, axial_swap, rest_size):
    if species==1:
        spine=np.array(LEECH[view],dtype=float)
        uv=spine[:,:2]/100
        direction=np.gradient(uv*rest_size,axis=0)
        normal=np.column_stack((direction[:,1],-direction[:,0]))
        normal/=np.linalg.norm(normal,axis=1)[:,None]
        offset=normal*spine[:,2,None]/rest_size
        if axial_swap: offset=-offset
        return np.clip(np.vstack((uv,uv-offset,uv+offset)),0,1)
    table,pairs={0:(TOAD,TOAD_PAIRS),2:(LIGHT,LIGHT_PAIRS),3:(KEEPER,KEEPER_PAIRS)}[species]
    coords=np.array(table[view],dtype=float)/100
    if axial_swap:
        for a,b in pairs: coords[[a,b]]=coords[[b,a]]
    return coords


def oriented(grid, uv, mirrored, width):
    coords=rig_sample(grid,uv)
    if mirrored: coords[...,0]=width-coords[...,0]
    return coords


def fit(points, vectors, quality, query, scale):
    """Smooth RBF displacement around an affine body rotation; no frame noise."""
    center=points.mean(axis=0)
    xy=(points-center)/scale
    target=(query-center)/scale
    poly=np.column_stack((np.ones(len(points)),xy))
    qpoly=np.column_stack((np.ones(len(target)),target))
    def kernel(x,y):
        squared=((x[:,None]-y[None])**2).sum(axis=2)
        return np.exp(-squared*.5)
    system=kernel(xy,xy)
    system.flat[::len(points)+1]+=.12/quality
    full=np.block([[system, poly],[poly.T,-.002*np.eye(3)]])
    weights=np.linalg.solve(full,np.vstack((vectors,np.zeros((3,2)))))
    return kernel(target,xy)@weights[:-3]+qpoly@weights[-3:]


def pair_projection(a,b,weight,at,scale,from_first=True):
    mid=(a+b)*.5
    velocity=b-a
    q=at.copy()
    sign=.5 if from_first else -.5
    for _ in range(4):
        q=at+sign*fit(mid,velocity,weight,q,scale)
    return 2*q-at


def edge_samples(image):
    alpha=image[...,3].astype(float)/255
    blurred=np.asarray(Image.fromarray((alpha*255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(1.4)),float)
    dy,dx=np.gradient(blurred)
    gradient=np.hypot(dx,dy)
    ys,xs=np.where((blurred>73)&(blurred<180)&(gradient>17))
    sel=(xs+2*ys)%3==0
    ys,xs=ys[sel],xs[sel]
    return np.column_stack((xs+.5,ys+.5)),np.column_stack((dx[ys,xs],dy[ys,xs]))/gradient[ys,xs,None]


def contour_pairs(image_a,image_b,a,b,w,scale):
    """A few mutually plausible boundary matches guide toes and fine occlusion edges."""
    p,normal=edge_samples(image_a)
    candidates,n2=edge_samples(image_b)
    projected=pair_projection(a,b,w,p,scale)
    delta=projected[:,None]-candidates[None]
    ds=(delta*delta).sum(axis=2)
    cost=ds+20*(1-np.clip(normal@n2.T,-1,1))**2
    k=cost.argmin(axis=1)
    q=candidates[k]
    back=pair_projection(a,b,w,q,scale,False)
    err=cost[np.arange(len(k)),k]
    good=(err<34)&(np.linalg.norm(back-p,axis=1)<6)
    # Prioritise matched features whose correction matters, while spacing them
    # to avoid high-frequency wrinkling from nearby, incompatible fingertips.
    ids=np.flatnonzero(good)
    ids=ids[np.argsort(-ds[ids,k[ids]])]
    selected=[]
    for i in ids:
        if all(np.linalg.norm(p[i]-p[j])>9 for j in selected):
            selected.append(i)
            if len(selected)>=32: break
    return p[selected],q[selected]


def mass_landmarks(image):
    """Equal fractions of visible body mass correspond even when a limb vanishes."""
    a=image[...,3]/255
    yy,xx=np.indices(a.shape,dtype=float)
    rows=np.cumsum(a.sum(axis=1))
    heights=np.interp(np.linspace(.035,.965,8)*rows[-1],rows,np.arange(len(rows)))+.5
    out=[]
    for y in heights:
        weights=np.exp(-.5*((yy+.5-y)/3.)**2)*a
        cols=np.cumsum(weights.sum(axis=0))
        for x in np.interp(np.linspace(.035,.965,5)*cols[-1],cols,np.arange(len(cols)))+.5:
            out.append((x,y))
    return np.asarray(out)


def rest_uv(rig,points,mirror,width):
    points=points.copy()
    if mirror: points[:,0]=width-points[:,0]
    return np.clip((points-rig[0,0])/(rig[-1,-1]-rig[0,0]),0,1)


def tile_grid(width,height):
    x,y=np.meshgrid(np.linspace(0,width,GRID),np.linspace(0,height,GRID))
    return np.column_stack((x.ravel(),y.ravel()))


def surface_samples(image):
    alpha=image[...,3].astype(np.float32)/255
    mask=alpha>.45
    distance=np.where(mask,distance_transform_edt(mask)-.5,.5-distance_transform_edt(~mask))
    light=image[...,:3]@np.array([.24,.65,.11],dtype=np.float32)
    blurred_alpha=gaussian_filter(alpha,3)
    light=gaussian_filter(alpha*light,3)/np.maximum(.001,blurred_alpha)
    return np.dstack((distance,light,blurred_alpha))


def sample_image(image,edge_xy,width,flipped):
    xy=edge_xy.copy()
    if flipped: xy[:,0]=width-xy[:,0]
    height=image.shape[0]
    xy=np.clip(xy-.5,[0,0],[width-1,height-1])
    index=np.minimum(xy.astype(int),[width-2,height-2])
    x,y=index.T;tx,ty=(xy-index).T[...,None]
    return ((1-tx)*(1-ty)*image[y,x]+tx*(1-ty)*image[y,x+1]
            +(1-tx)*ty*image[y+1,x]+tx*ty*image[y+1,x+1])


def bound_deformation(flow, width, height, focus):
    """Project the field onto smooth, invertible halfway deformations.

    Hidden legs can swap depth ordering between two paintings. Stretching a
    texture until those two identities cross would create a fold. Clip the
    Jacobian, integrate the bounded gradients, then leave that disocclusion to
    the reveal matte. Even beyond the body, queries must remain invertible.
    """
    flow=flow.reshape(GRID,GRID,2)
    dx=width/(GRID-1)
    dy=height/(GRID-1)
    # Mirror extension gives a zero normal derivative at the outer lattice.
    f=np.concatenate((flow,flow[:,::-1]),axis=1)
    f=np.concatenate((f,f[::-1]),axis=0)
    ny,nx=f.shape[:2]
    fx=(np.exp(2j*np.pi*np.fft.fftfreq(nx))-1)[None,:]/dx
    fy=(np.exp(2j*np.pi*np.fft.fftfreq(ny))-1)[:,None]/dy
    poisson=abs(fx)**2+abs(fy)**2
    poisson[0,0]=1
    for _ in range(2):
        du=(np.roll(f,-1,axis=1)-f)/dx
        dv=(np.roll(f,-1,axis=0)-f)/dy
        # Spectral norm of a 2x2 matrix, vectorised over the lattice.
        a=(du*du).sum(axis=2)
        c=(dv*dv).sum(axis=2)
        b=(du*dv).sum(axis=2)
        norm=np.sqrt((a+c+np.sqrt((a-c)**2+4*b*b))*.5)
        factor=np.minimum(1.,1.1/np.maximum(.001,norm))[...,None]
        if factor.min()==1:
            break
        du*=factor;dv*=factor
        freq=(fx.conj()[...,None]*np.fft.fft2(du,axes=(0,1))
              +fy.conj()[...,None]*np.fft.fft2(dv,axes=(0,1)))/poisson[...,None]
        freq[0,0]=f.mean(axis=(0,1))*(nx*ny)
        f=np.fft.ifft2(freq,axes=(0,1)).real
    result=f[:GRID,:GRID]
    # A bilinear tile's largest derivative is bounded by the largest at
    # its four vertices. Reserve room for the RG8 vector quantisation.
    du=np.diff(result,axis=1)/dx
    dv=np.diff(result,axis=0)/dy
    peak=0.
    for u in (du[:-1],du[1:]):
        for v in (dv[:,:-1],dv[:,1:]):
            a=(u*u).sum(axis=2);c=(v*v).sum(axis=2);b=(u*v).sum(axis=2)
            peak=max(peak,np.sqrt((a+c+np.sqrt((a-c)**2+4*b*b))*.5).max())
    result=result*min(1.,1.22/max(.001,peak))
    # Keep the eye, sucker, wick or bell centered on its anatomical path.
    col,row=np.rint(focus/[width,height]*(GRID-1)).astype(int).clip(0,GRID-1)
    result+=flow[row,col]-result[row,col]
    return result.reshape(-1,2)


def semantic_warp(g,images,surfaces,species,sector,width,height,feet,cores,grid):
    view_a,flip_a=HEADINGS[sector]
    view_b,flip_b=HEADINGS[(sector+1)%8]
    half_turn=flip_a or flip_b
    def material(view,flip):
        rig=g[view,0]
        return marks(species,view,half_turn and view in (0,4),rig[-1,-1]-rig[0,0])
    uv_a=material(view_a,flip_a);uv_b=material(view_b,flip_b)
    a=oriented(g[view_a,0],uv_a,flip_a,width);b=oriented(g[view_b,0],uv_b,flip_b,width)
    weights=np.ones(len(a))
    neutral_a=images[view_a][:height,:width]
    neutral_b=images[view_b][:height,:width]
    if flip_a: neutral_a=neutral_a[:,::-1]
    if flip_b: neutral_b=neutral_b[:,::-1]
    mass_a,mass_b=mass_landmarks(neutral_a),mass_landmarks(neutral_b)
    pa,pb=contour_pairs(neutral_a,neutral_b,a,b,weights,height*.22)
    pa=np.vstack((pa,mass_a));pb=np.vstack((pb,mass_b))
    uv_a=np.vstack((uv_a,rest_uv(g[view_a,0],pa,flip_a,width)))
    uv_b=np.vstack((uv_b,rest_uv(g[view_b,0],pb,flip_b,width)))
    weights=np.r_[weights,np.full(len(pa)-len(mass_a),.40),np.full(len(mass_a),1.1)]
    if species==1 and sector in (1,5,6):
        # The dorsal and frontal paintings curl the tail behind the sucker.
        # The visible overlap switches sides; only match the exposed mass here.
        weights[:len(a)]*=.16
        weights[-len(mass_a):]*=2.5
    all_a=oriented(g[view_a],uv_a,flip_a,width)
    all_b=oriented(g[view_b],uv_b,flip_b,width)
    def pose_points(view,flip,pose):
        foot=feet[view,pose].copy()
        if flip: foot[:,0]=width-foot[:,0]
        foot=foot[np.argsort(foot[:,0])]
        core=cores[view,pose].copy()
        if flip: core[0]=width-core[0]
        return foot,core
    tiles=[];errors=[]
    for pose,(a,b) in enumerate(zip(all_a,all_b)):
        fa,ca=pose_points(view_a,flip_a,pose)
        fb,cb=pose_points(view_b,flip_b,pose)
        valid=(fa[:,2]>0)&(fb[:,2]>0)
        ap=np.vstack((a,fa[valid,:2]));bp=np.vstack((b,fb[valid,:2]))
        weight=np.r_[weights,np.full(int(valid.sum()),1.8)]
        if species==2:
            ap=np.vstack((ap,ca));bp=np.vstack((bp,cb));weight=np.r_[weight,12.]
        mid=(ap+bp)*.5
        flow=fit(mid,bp-ap,weight,grid,height*.22)
        # Extend a close-to-rigid rotation through empty space; displacement at
        # the cell's distant corners can otherwise exceed byte range.
        focus_id={0:1,1:0,2:len(mid)-1,3:2}[species]
        flow=np.clip(bound_deformation(flow,width,height,mid[focus_id]),-94.5,94.5)
        sa=sample_image(surfaces[view_a][pose],grid-.5*flow,width,flip_a)
        sb=sample_image(surfaces[view_b][pose],grid+.5*flow,width,flip_b)
        threshold=surface_reveal(grid,mid,species,sector,height,sa[:,0],sb[:,0])
        delta=np.clip(sb[:,1]-sa[:,1],-32.,32.)*np.clip(np.minimum(sa[:,2],sb[:,2])*2,0,1)
        data=np.column_stack((np.rint(flow/PITCH)+128,np.rint(threshold*255),np.rint(delta)+128))
        tiles.append(data.astype(np.uint8).reshape(GRID,GRID,4))
        projected=fit(mid,bp-ap,weight,mid[:len(a)-len(pa)],height*.22)
        errors.extend(np.linalg.norm((bp-ap)[:len(projected)]-projected,axis=1).tolist())
    return tiles,errors


def surface_reveal(grid,mid,species,sector,height,dist_a,dist_b):
    # A single broad, slowly moving reveal line follows body space, not screen
    # space or a random per-frame mask. Diagonal curvature helps hide view
    # changes along a turning flank; anatomically paired points move together.
    base=mid[:{0:14,1:6,2:11,3:17}[species]]
    center=np.median(base,axis=0)
    extent=np.maximum(np.ptp(base,axis=0),[height*.30,height*.28])
    uv=(grid-center)/extent
    lateral=1 if sector in (0,1,6,7) else -1
    signed=lateral*uv[:,0]+.24*uv[:,1]-.12*np.cos(2.8*uv[:,1])
    threshold=.5+.27*np.tanh(2.1*signed)
    # Protect the carved bell mouth and the painted face as one feature.
    # Elsewhere the reveal can travel more broadly across bark and skin.
    facial=base[:(4 if species in (0,3) else 3)]
    center=facial.mean(axis=0)
    radius=np.maximum(np.ptp(facial,axis=0)*.64,[height*.10,height*.075])
    radius*=1.35 if species==3 else 1.0
    rr=np.linalg.norm((grid-center)/radius,axis=1)
    plateau=np.clip((1.8-rr)/.9,0,1)
    key=threshold[np.argmin(np.linalg.norm(grid-center,axis=1))]
    threshold+=(key-threshold)*plateau*.94
    # At unmatched toes and tails the seam follows the traveling silhouette.
    # A fixed vertical wipe through empty space can amputate an entire limb.
    sign_change=dist_a*dist_b<0
    crossing=np.divide(dist_a,dist_a-dist_b,out=threshold.copy(),where=sign_change)
    strength=np.clip(np.abs(dist_a-dist_b)/3.,0,1)*sign_change
    return np.clip(threshold+(crossing-threshold)*strength,.04,.96)


def check_vectors(tiles,w,h):
    """Check actual quantized, bilinear sampling cells, including their corners."""
    flow=(np.asarray(tiles)[...,:2].astype(float)-128)*PITCH
    du=np.diff(flow,axis=2)*(GRID-1)/w
    dv=np.diff(flow,axis=1)*(GRID-1)/h
    peak=0.; lowest=1.
    for u in (du[:,:-1],du[:,1:]):
        for v in (dv[:,:,:-1],dv[:,:,1:]):
            a=(u*u).sum(axis=3);b=(u*v).sum(axis=3);c=(v*v).sum(axis=3)
            norm=np.sqrt((a+c+np.sqrt((a-c)**2+4*b*b))*.5)
            peak=max(peak,np.max(norm))
            trace=u[...,0]+v[...,1];det=u[...,0]*v[...,1]-u[...,1]*v[...,0]
            extreme=np.clip(-trace/(2*np.maximum(det,.000001)),-.5,.5)
            for r in (-.5,.5,extreme):
                lowest=min(lowest,np.min(1+r*trace+r*r*det))
    if peak>=1.95 or lowest<=0: raise ValueError(f"Non-invertible vector tile: {peak}, {lowest}")
    return {"largest_velocity_gradient":round(float(peak),4),"min_jacobian":round(float(lowest),4)}


def header_array(text,name,shape,width):
    value=text.split(name+"[",1)[1].split("=",1)[1].split(";",1)[0]
    return np.array([float(n) for n in re.findall(r"(-?\d+\.\d+)f",value)]).reshape(*shape,width)


def export(root,geometry,out):
    started=time.perf_counter()
    book=root/"assets/media/mokradla-ruch"
    receipt=json.loads((book/"turn-import-receipt.json").read_text(encoding="utf8"))
    header=(root/"src/generated/marsh_turn_frames.hpp").read_text()
    feet=header_array(header,"Supports",(4,5,64,2),3)
    cores=header_array(header,"LightCore",(5,64),2)
    rig=np.load(geometry)["xy"]
    assert rig.shape[:3]==(4,5,64) and np.isfinite(rig).all()
    out.mkdir(parents=True,exist_ok=True)
    files=[]; stats=[]
    for s,(w,h,ax,ay,_) in enumerate(receipt["cell_dimensions"]):
        pages=[]; surfaces=[]
        for view in range(5):
            with Image.open(book/f"turn-{s}-{view}.png") as im:
                full=np.array(im.convert("RGBA"));pages.append(full[:h,:w])
                surfaces.append([surface_samples(full[(i//8)*h:(i//8+1)*h,(i%8)*w:(i%8+1)*w])
                                 for i in range(64)])
        sheet=np.empty((GRID*8,GRID*64,4),dtype=np.uint8)
        grid=tile_grid(w,h)
        for k in range(8):
            packed,error=semantic_warp(rig[s],pages,surfaces,s,k,w,h,feet[s]+[ax,ay,0],cores+[ax,ay],grid)
            sheet[k*GRID:(k+1)*GRID]=np.concatenate(packed,axis=1)
            stats.append({"species":s,"sector":k,"p90_fit_residual_before_constraints_px":round(float(np.percentile(error,90)),3),
                          **check_vectors(packed,w,h)})
        path=out/f"turn-warp-{s}.png"
        Image.fromarray(sheet).save(path)
        with Image.open(path) as im:
            assert np.array_equal(im,sheet)
        files.append({"file":path.name,"size":path.stat().st_size,"sha256":hashlib.sha256(path.read_bytes()).hexdigest()})
        print(f"Registered species {s} in {time.perf_counter()-started:.1f} s",flush=True)
    record={"grid":GRID,"pose_columns":64,"clockwise_sectors":8,"pitch_pixels":PITCH,
            "coordinates":"Cell-edge coordinates, screen oriented, (0,0) at top left. F locates the endpoints from midpoint M: A=M-F/2, B=M+F/2.",
            "red_green":"F=(RG*255-128)*0.75","blue":"continuous surface replacement threshold",
            "alpha":"low-frequency change in luminance from A to B, byte minus 128",
            "source_geometry_sha256":hashlib.sha256(geometry.read_bytes()).hexdigest(),
            "source_atlases":receipt["files"],"seconds":round(time.perf_counter()-started,3),"registrations":stats,"files":files}
    (out/"turn-warp-receipt.json").write_text(json.dumps(record,indent=2),encoding="utf8")


if __name__=="__main__":
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--root",type=Path,required=True)
    p.add_argument("--geometry",type=Path,required=True)
    p.add_argument("--output",type=Path,required=True)
    args=p.parse_args()
    export(args.root,args.geometry,args.output)

"""Bake the planar swamp puppet. One pigment painting per bearing, four keyed actions.

Inputs are 128 unique neutral silhouettes, their points of contact, head and core
from the Blender turnaround. The result contains only CPU vertex displacements
and support sockets. There is no resampling of paint between action poses.
"""
from __future__ import annotations
import argparse
import contextlib
import importlib.util
import hashlib
import json
import math
import time
from pathlib import Path
import numpy as np
from PIL import Image
from scipy import ndimage, sparse
from scipy.sparse.linalg import splu

CELLS = ((140, 100, 70, 92), (164, 96, 82, 89), (120, 120, 60, 112), (204, 164, 102, 159))
REST = (77, 66, 98, 145)
FACES, FRAMES, SIDE = 128, 16, 13
CLIPS = ('Chód', 'Atak', 'Trafienie', 'Upadek')


def smooth(x):
    x = np.clip(x, 0., 1.)
    return x*x*(3.-2.*x)


def pulse(time, first, peak, last):
    return smooth((time-first)/(peak-first))*smooth((last-time)/(last-peak))


def painted_cells(directory, species):
    width, height, _, _ = CELLS[species]
    cells = []
    for page in range(2):
        with Image.open(directory/f'neutral-{species}-{page}.png') as source:
            rgba = np.asarray(source.convert('RGBA'))
        if rgba.shape[:2] != (height*8, width*8):
            raise ValueError(f'Bad dimensions ({species}, {page}): {rgba.shape}')
        cells.extend(rgba[y*height:(y+1)*height, x*width:(x+1)*width].copy()
                     for y in range(8) for x in range(8))
    return np.array(cells)


def velocity(paints, bearing, xy, size):
    """Small-displacement, color-and-coverage Lucas-Kanade fallback for custom sheets."""
    def signals(k):
        a = paints[k % FACES].astype(np.float32)/255
        a[..., :3] *= a[..., 3:]
        return a
    current = signals(bearing)
    temporal = (signals(bearing+1) - signals(bearing-1))*.5
    vertical, horizontal = np.gradient(current, axis=(0, 1))
    def moment(a, b):
        return ndimage.gaussian_filter(np.sum(a*b, axis=-1), 4.4)
    xx, xy2, yy = moment(horizontal, horizontal), moment(horizontal, vertical), moment(vertical, vertical)
    xt, yt = moment(horizontal, temporal), moment(vertical, temporal)
    xx += .000025
    yy += .000025
    det = xx*yy - xy2*xy2
    vx, vy = (xy2*yt-yy*xt)/det, (xy2*xt-xx*yt)/det
    return np.clip(sample_field(np.stack((vx, vy), axis=-1), xy, size), -5, 5)


def sample_field(field, xy, size):
    w, h, ax, ay = size
    sx = (xy[:, 0]+ax)*field.shape[1]/w-.5
    sy = (xy[:, 1]+ay)*field.shape[0]/h-.5
    return np.column_stack([ndimage.map_coordinates(field[..., i], (sy, sx), order=1, mode='nearest')
                            for i in range(2)])


def neutral_marks(paint, size, named, angle, species):
    _, _, ax, ay = size
    mask = paint[..., 3] > 127
    yy, xx = np.nonzero(mask)
    if len(xx) < 80:
        raise ValueError('Empty neutral painting')
    depth = np.sin(angle)
    bottom = yy.max()+1
    x_center = .5*float(xx.min()+xx.max()+1)-ax
    default_body = (x_center, .5*float(yy.min()+yy.max()+1)-ay)
    body = np.asarray(named.get('body', default_body), dtype=float)[:2]
    default_head = body + (np.cos(angle)*(.26 if species==1 else .15)*(xx.max()-xx.min()),
                           (.4*depth-.08 if species==1 else -.25+.16*depth)*(bottom-yy.min()))
    head = np.asarray(named.get('head', default_head), dtype=float)[:2]
    core = np.asarray(named.get('core', head), dtype=float)[:2]
    hull = mask & ~ndimage.binary_erosion(mask)
    cy, cx = np.nonzero(hull)
    perimeter = np.column_stack((cx+.5-ax, cy+.5-ay))
    # Include the two bottom cell corners so interpolated edge contacts remain
    # grounded to subpixel precision rather than the centre of the final texel.
    pm = perimeter.copy()
    pm[:, 1] += .42
    return body, head, core, pm


def lookup(coords, size):
    w, h, ax, ay = size
    grid = np.clip((coords + (ax, ay))*((SIDE-1)/w, (SIDE-1)/h), 0, SIDE-1-.000001)
    ix, iy = grid.astype(int).T
    fx, fy = (grid-grid.astype(int)).T
    origin = iy*SIDE+ix
    # The renderer's triangles use the top-left to bottom-right diagonal.
    return np.array((origin, origin+1, origin+SIDE, origin+SIDE+1)), np.array((
        1-np.maximum(fx, fy), np.maximum(0., fx-fy), np.maximum(0., fy-fx), np.minimum(fx, fy)))


def mapped(points, weights, delta):
    ids, values = weights
    return points + np.einsum('ij,ijk->jk', values, delta[ids])


class PaintRigidity:
    """Keep local rotations from the keyed gesture, resist visible material strain."""
    def __init__(self, grid, species, body, head, feet):
        rows = np.arange(SIDE*SIDE).reshape(SIDE, SIDE)
        a, b, c, d = (corners.ravel() for corners in
                      (rows[:-1, :-1], rows[1:, :-1], rows[1:, 1:], rows[:-1, 1:]))
        # The same diagonal is used in lookup() and in the game.
        self.triangles = np.concatenate((np.stack((a, b, c), axis=1),
                                         np.stack((a, c, d), axis=1)))
        tile = grid[self.triangles]
        edge = tile[:, 1:] - tile[:, :1]
        self.inverse = np.linalg.inv(edge.transpose(0, 2, 1))
        self.edges = tile[:, (1, 2, 0)] - tile
        centers = tile.mean(axis=1)
        def plate(center, radii):
            return np.exp(-.5*np.sum(((centers-center)/radii)**2, axis=1))
        body_radius = ((30, 19), (38, 18), (18, 22), (38, 39))[species]
        head_radius = ((22, 14), (16, 13), (19, 18), (26, 26))[species]
        weights = .46 + 9*plate(body, body_radius) + 24*plate(head, head_radius)
        next_corner = self.triangles[:, (1, 2, 0)]
        ends = np.stack((self.triangles, next_corner), axis=-1).reshape(-1, 2)
        e = np.arange(len(ends))
        incidence = sparse.coo_matrix((np.tile((-1., 1.), len(ends)),
            (np.repeat(e, 2), ends.ravel())), shape=(len(e), len(grid))).tocsr()
        self.force = incidence.T.multiply(np.repeat(weights, 3)).tocsr()
        # Give the painted foot contacts and face the same timing as the
        # authored pose. Stiffness applies between controls, not to their travel.
        def pin(point, radii):
            return np.exp(-.5*np.sum(((grid-point)/radii)**2, axis=1))
        foot_radius = ((9, 6), (8, 6), (6, 7), (12, 10))[species]
        foot_pins = np.max([pin(foot[:2], foot_radius) for foot in feet], axis=0)
        self.tether = 1 + 22*foot_pins + 26*pin(head, (9, 9))
        self.solve = splu((sparse.diags(self.tether)+self.force@incidence).tocsc()).solve

    def apply(self, delta):
        points = delta[self.triangles]
        stretch = (points[:, 1:] - points[:, :1]).transpose(0, 2, 1) @ self.inverse
        # Polar rotation of each triangle. The rest edge lengths stay exact in
        # these targets, even when the original motion asks the ink to stretch.
        c = 2 + stretch[:, 0, 0] + stretch[:, 1, 1]
        s = stretch[:, 1, 0] - stretch[:, 0, 1]
        norm = np.maximum(np.hypot(c, s), 1e-8)
        c, s = (c/norm-1)[:, None], (s/norm)[:, None]
        x, y = self.edges[..., 0], self.edges[..., 1]
        rotated = np.stack((c*x-s*y, s*x+c*y), axis=-1).reshape(-1, 2)
        return self.solve(self.tether[:, None]*delta + self.force @ rotated)


class LeechSpine:
    """An articulated travelling bend. Consecutive painted rings keep their spacing."""
    def __init__(self, centers, grid):
        if len(centers) < 3:
            raise ValueError('Incomplete chain of painted leech rings')
        self.origin = centers[0]
        self.edge = np.diff(centers, axis=0)
        self.middle = .5*(centers[:-1] + centers[1:])
        self.relative = grid[:, None, :] - self.middle
        # Feed adjacent overlapping ring transforms into the same sheet mesh.
        distance = np.sum(self.relative**2, axis=2)
        weight = np.exp(-(distance-distance.min(axis=1)[:,None])/(2*9.**2))
        self.weight = weight/weight.sum(axis=1)[:,None]
        self.wave = np.linspace(0., -3.8, len(self.edge))
        self.center = centers[len(centers)//2]

    def pose(self, phase):
        angle = .15*(np.sin(math.tau*phase+self.wave)-np.sin(self.wave))
        c, s = np.cos(angle), np.sin(angle)
        def rotate(x, y):
            return np.stack((c*x-s*y, s*x+c*y), axis=-1)
        edge = rotate(*self.edge.T)
        points = np.vstack((self.origin, self.origin+np.cumsum(edge, axis=0)))
        anchor = self.center-points[len(points)//2]
        shifts = .5*(points[:-1]+points[1:])+anchor-self.middle
        x, y = self.relative[..., 0], self.relative[..., 1]
        offsets = rotate(x, y)-self.relative+shifts
        return np.einsum('ns,nsi->ni', self.weight, offsets)


def unfold(grid, verts, fit, cell_area):
    """Preserve the rotation and scale while preventing a folded UV triangle."""
    def weakest(points):
        p=points.reshape(SIDE,SIDE,2)
        a,b,c,d=p[:-1,:-1],p[1:,:-1],p[1:,1:],p[:-1,1:]
        def area(v,w):
            return v[...,1]*w[...,0]-v[...,0]*w[...,1]
        return min(np.min(area(b-a,c-a)),np.min(area(c-a,d-a)))
    if weakest(grid+verts)>=cell_area*.15:
        return
    affine=fit[0]@(fit[1]@verts)
    bend=verts-affine
    low,high=0.,1.
    for _ in range(10):
        t=(low+high)*.5
        if weakest(grid+affine+t*bend)>=cell_area*.15:low=t
        else:high=t
    verts[:]=affine+low*bend


def rig_setup(species, bearing, grid, body, head, named, outline):
    angle = bearing*math.tau/FACES
    height = REST[species]
    fwd = np.array((math.cos(angle), .53*math.sin(angle)))
    side = np.array((-math.sin(angle), .53*math.cos(angle)))
    x, y = grid.T
    min_y = np.min(outline[:, 1])
    v = np.clip((y-min_y)/height, 0, 1.15)
    relative = (grid-body)/height
    axial = np.clip(.5+(relative @ fwd)/(.92*np.dot(fwd, fwd)), -.30, 1.3)
    def patch(target, sx, sy):
        return np.exp(-((x-target[0])/(height*sx))**2-((y-target[1])/(height*sy))**2)
    chest = patch(body, .46, .43)
    face = patch(head, .26, .23)
    limb = smooth((v-.48)/.46)
    # The two lowest feet trade places as we turn. They locate the contact
    # shadow, but aren't joint identities. Keep the rig's original limb order.
    foot = np.asarray(named['limbs'], dtype=float)
    if species in (0,3):
        z=foot[:,2]
        foreground=.48+.52*smooth((z-z.min())/max(1.,float(np.ptp(z))))
        region=[patch(point[:2],.17,.31)*limb*opacity for point,opacity in zip(foot,foreground)]
        if species==0:
            left_leg=np.maximum(region[0],region[3])
            right_leg=np.maximum(region[2],region[1])
        else:
            left_leg,right_leg=region
    else:
        left_leg=right_leg=np.zeros_like(x)
    width = np.ptp(outline[:, 0])
    left_arm = patch(body+(-.34*width, .13*height), .19, .30)
    right_arm = patch(body+(.34*width, .13*height), .19, .30)
    return dict(h=height, f=fwd, s=side, v=v, axial=axial, ch=chest, head=face, limb=limb,
                left=left_leg, right=right_leg, arm0=left_arm, arm1=right_arm,
                body=body, grid=grid, rel=relative, axis=angle)


def motion(species, clip, phase, a):
    """Screen-plane performance; displacements scale with the measured body height."""
    x, y = a['grid'].T
    h, f, side = a['h'], a['f'], a['s']
    v, spine, chest, face, lower = a['v'], a['axial'], a['ch'], a['head'], a['limb']
    l, r, arm0, arm1 = a['left'], a['right'], a['arm0'], a['arm1']
    rel, body = a['rel'], a['body']
    zero = np.zeros_like(x)
    dx, dy = zero.copy(), zero.copy()
    tau = math.tau*phase
    swing, load = math.sin(tau), .5-.5*math.cos(tau)
    if clip==0:
        if species in (0,3):
            # Opposing swing/stance and a double support when the feet pass.
            step = (.13 if species==0 else .105)*swing*(l-r)
            lift = (.065 if species==0 else .062)*(max(0,swing)*l + max(0,-swing)*r)
            shoulder = (.047 if species==0 else .082)*swing*(arm1-arm0)
            dx += f[0]*(step+shoulder) + .015*load*side[0]*chest
            dy += f[1]*(step+shoulder) - lift + (.036 if species==0 else .023)*load*chest
            dy += .019*swing*face + .021*swing*(arm0-arm1)
        elif species==1:
            return a['rings'].pose(phase)
        else:
            trail = np.sin(tau-3.8*v)-np.sin(-3.8*v)
            dx += .024*swing*(1-v) + .073*trail*lower
            dy += .024*swing*(1-v) + .014*trail*lower*np.sin(rel[:,0]*8)
    elif clip==1:
        prepare, strike, settle = pulse(phase,0,.26,.48), pulse(phase,.24,.50,.83), pulse(phase,.55,.80,1)
        shift = -.075*prepare+.169*strike-.035*settle
        if species==0:
            forward = shift*(.70*chest+1.0*face+.15*(arm0+arm1))
            dx += f[0]*forward
            dy += f[1]*forward + (.065*prepare-.047*strike)*chest
        elif species==1:
            nose = .24*chest+.88*smooth(spine)+.65*face
            dx += f[0]*1.25*shift*nose
            dy += f[1]*1.25*shift*nose + (-.076*prepare+.035*strike)*chest
        elif species==2:
            thrust = shift*(.75*face+.55*chest)
            dx += f[0]*thrust + .042*strike*lower*np.sin(6*v)
            dy += f[1]*thrust - .047*strike*face
        else:
            bell = face+.48*chest
            forward = shift*(.69*chest+.73*arm0+.57*arm1)
            dx += f[0]*forward + .034*side[0]*prepare*arm1
            dy += f[1]*forward + (-.16*prepare+.115*strike)*arm0+(.054*strike-.044*prepare)*bell
    elif clip==2:
        recoil, settle = pulse(phase,0,.23,.64), pulse(phase,.42,.72,1)
        upper = chest+.45*face if species in (0,3) else .42*chest+.82*face
        shove = (-.115*recoil+.038*settle)*upper
        dx += f[0]*shove + .026*side[0]*recoil*upper
        dy += f[1]*shove + .034*recoil*upper + .019*settle*(arm0-arm1)
    else:
        kneel, roll, limp = smooth(phase/.39),smooth((phase-.13)/.68),smooth((phase-.32)/.66)
        bounce = pulse(phase,.63,.77,.98)
        if species==0:
            # The hind legs fold under the flank as the chest settles. The
            # unbraced forelimbs splay out and the snout lands past the shoulder.
            dx += limp*(.13*r-.07*l+.075*(arm1-arm0)+.08*f[0]*face)
            back=1-smooth((v-.05)/.36)
            dy += .115*kneel*chest+.265*roll*face-.19*limp*lower-.105*limp*r-.022*bounce*face-.06*roll*back
        elif species==1:
            tail = 1-smooth((spine-.18)/.46)
            # The tip hooks back toward the belly while the sucker relaxes
            # onto the sediment. Keep a substantial arch at mid-body.
            mouth=smooth((spine-.53)/.39)
            dx += limp*((.40*f[0]+.23*side[0])*tail-.12*f[0]*mouth+.045*rel[:,0]*chest)
            dy += limp*((.22*f[1]-.26)*tail+.11*mouth-.11*chest)
        elif species==2:
            dx += -.11*kneel*(1-v)+.08*limp*lower*np.sin(4*v+rel[:,0])
            dy += .070*kneel*chest-.165*limp*lower
        else:
            dx += -.044*kneel*chest+limp*(.095*arm0-.083*arm1+.076*r)
            dy += .13*kneel*chest+.041*kneel*face-.067*bounce*face-.095*limp*arm1
        angle = (-1.00,.52,-1.02,-1.05)[species]*(1-.1*math.sin(a['axis']))*roll
        flatten = 1-(.075,.035,.10,.14)[species]*roll
        if species==0:
            # Pitch the head toward the mud in BOTH profiles. When facing
            # the camera (or away), expose the side of the belly instead.
            c,d=math.cos(a['axis']),math.sin(a['axis'])
            angle=(.36*c-.48*d*d)*roll
            flatten=1-.12*roll
        px, py = x+dx*h-body[0], y+dy*h-body[1]
        ca, sa = math.cos(angle), math.sin(angle)
        return np.stack((px*ca-py*sa+body[0]-x,(px*sa+py*ca)*flatten+body[1]-y),axis=1)
    return np.stack((dx*h,dy*h),axis=1)


def grounded(verts, outline, contour_lookup, core, core_lookup, neutral_bottom, sole, grid, fit, area):
    unfold(grid,verts,fit,area)
    moved = mapped(outline, contour_lookup, verts)
    bottom = np.max(moved[:,1])
    if sole is None:
        offset = neutral_bottom-bottom
        verts[:, 1] += offset
        moved[:, 1] += offset
    else:
        # Set the support without moving the torso when a different toe
        # becomes the lowest point. The sole has full weight at terrain level.
        verts[:,1] += (neutral_bottom-bottom)*sole
        unfold(grid,verts,fit,area)
        moved=mapped(outline,contour_lookup,verts)
    supports = []
    planted = np.min(moved[:, 0])+.5*np.ptp(moved[:, 0])
    for part in (moved[moved[:,0]<=planted], moved[moved[:,0]>planted]):
        low = np.max(part[:,1])
        belt = part[part[:,1]>=low-1.8]
        px = np.median(belt[:,0])
        radius = np.clip(np.ptp(belt[:,0])*.30,2,15)
        supports.append([px,low,radius])
    c = mapped(core[None,:],core_lookup,verts).ravel()
    return np.concatenate((np.asarray(supports).ravel(),c))


def occlusion_pages(source, dense, out, species):
    """For each quarter of a turn interval identify which painting owns the ink."""
    w,h,_,_=CELLS[species]
    path=source/f'visibility-{species}.npz'
    if not path.is_file():return []
    with np.load(path) as data:
        owners=data['owners']
    if owners.dtype!=np.uint8 or owners.shape!=(FACES,4,h*2,w*2):
        raise ValueError(f'Unexpected supersampled layer masks in {path}')
    y,x=np.indices((h*2,w*2),dtype=np.float32)
    flow_coords=((y+.5)/4-.5,(x+.5)/4-.5)
    def flow(b):
        return [ndimage.map_coordinates((lane.astype(np.float32)-128)/16,flow_coords,
                                        order=1,mode='nearest') for lane in dense[b].transpose(2,0,1)]
    def owner_at(b,velocity,t):
        ix=np.clip(np.rint(x+2*t*velocity[0]).astype(int),0,2*w-1)
        iy=np.clip(np.rint(y+2*t*velocity[1]).astype(int),0,2*h-1)
        return owners[b,0,iy,ix]
    previous=flow(0)
    outputs=[]
    page=np.empty((8*h,8*w,3),dtype=np.uint8)
    for b in range(FACES):
        n=(b+1)%FACES
        following=flow(n)
        cy,cx=divmod(b%64,8)
        for k in (1,2,3):
            t=k/4
            first=owner_at(b,previous,-t)
            second=owner_at(n,following,1-t)
            visible=owners[b,k]
            change=(first!=second)&(first!=0)&(second!=0)
            weight=np.full(visible.shape,t*t*(3-2*t),dtype=np.float32)
            weight[change&(first==visible)]=0
            weight[change&(second==visible)]=1
            # Antialias the occlusion boundary within a texel, not over a limb.
            weight=weight.reshape(h,2,w,2).mean(axis=(1,3))
            page[cy*h:(cy+1)*h,cx*w:(cx+1)*w,k-1]=np.rint(255*weight).astype('u1')
        previous=following
        if b%64==63:
            filename=out/f'cover-{species}-{b//64}.png'
            Image.fromarray(page).save(filename,compress_level=7)
            outputs.append(filename)
    return outputs


def compile_species(input_dir, source_dir, out, species, marks):
    paints = painted_cells(input_dir, species)
    w,h,ax,ay=size=CELLS[species]
    x,y=np.meshgrid(np.linspace(-ax,w-ax,SIDE),np.linspace(-ay,h-ay,SIDE))
    grid=np.stack((x,y),axis=-1).reshape(-1,2)
    affine=np.column_stack((grid,np.ones(len(grid))))
    fit=affine,np.linalg.pinv(affine)
    cell_area=w*h/(SIDE-1)**2
    poses=np.zeros((FACES,4*FRAMES,SIDE*SIDE,2),dtype='<i2')
    tangents=np.empty((FACES,SIDE*SIDE,2),dtype='<i2')
    dense=np.empty((FACES,h//2,w//2,2),dtype='u1')
    sockets=np.empty((FACES,4*FRAMES,8),dtype='<i2')
    path=source_dir/f'yaw-{species}.npz'
    if not path.exists():
        path=source_dir/f'yaw-raw-{species}.npz'
    with np.load(path) if path.exists() else contextlib.nullcontext() as data:
        flow=np.array(data['flow']) if data is not None else None
        ink=np.array(data['ink']) if data is not None and 'ink' in data else None
    if flow is not None and (flow.shape[0]!=FACES or flow.shape[-1]!=2 or not np.isfinite(flow).all()):
        raise ValueError(f'Invalid bearing velocities in {path}')
    for b in range(FACES):
        name=dict(marks[b])
        if 'limbs' not in name:
            rig=compass.frame(species,b)
            dy=name['body'][1]-rig.land['body'][1]
            name['limbs']=[[q[0],q[1]+dy,q[2],radius] for q,radius in rig.feet]
        angle=b*math.tau/FACES
        body,head,core,outline=neutral_marks(paints[b],size,name,angle,species)
        hull_lookup=lookup(outline,size)
        core_lookup=lookup(core[None,:],size)
        head_lookup=lookup(head[None,:],size)
        # Every projected bearing stands on the same terrain height. Retain the
        # subpixel contact of the original painting when sampling action deltas.
        neutral_bottom=np.max(outline[:,1])
        sole=smooth((grid[:,1]-(neutral_bottom-.42*REST[species]))/(.38*REST[species]))
        setup=rig_setup(species,b,grid,body,head,name,outline)
        rigid=PaintRigidity(grid,species,body,head,np.asarray(name['limbs']))
        if species == 1:
            centers=np.array([name[key] for key in sorted(name) if key.startswith('ring-')])
            if not len(centers):
                # Earlier editions recorded only head, belly and contacts.
                base=compass.frame(species,b)
                dy=body[1]-base.land['body'][1]
                centers=np.array([.5*(p.xy[0,0]+p.xy[-1,-1])+(0,dy)
                                  for p in base.parts if p.paint in ('leech-ring','leech-ring-dark')])
            setup['rings']=LeechSpine(centers,grid)
        if flow is None:
            tangent=velocity(paints,b,grid,size)
            field=ndimage.zoom(tangent.reshape(SIDE,SIDE,2),(h/SIDE,w/SIDE,1),order=1)
        elif flow.shape[1:3]==(SIDE,SIDE):
            tangent=flow[b].reshape(-1,2)
            field=ndimage.zoom(tangent.reshape(SIDE,SIDE,2),(h/SIDE,w/SIDE,1),order=1)
        else:
            field=flow[b].astype('f4')
            if ink is not None:
                # Continue the nearest painted tangent into the transparent
                # margin. This keeps narrow projected roots attached to the mesh.
                iy, ix=ndimage.distance_transform_edt(~ink[b],return_distances=False,return_indices=True)
                field=field[iy,ix]
            tangent=sample_field(field,grid,size)
        tangents[b]=np.rint(tangent*64).astype('<i2')
        coarse=field.reshape(h//2,2,w//2,2,2).mean(axis=(1,3))
        dense[b]=np.rint(128+16*coarse).clip(0,255).astype('u1')
        for clip in range(4):
            for k in range(FRAMES):
                phase=k/(FRAMES if clip==0 else FRAMES-1)
                delta=motion(species,clip,phase,setup)
                if clip != 3:
                    keyed=delta
                    delta=rigid.apply(keyed)
                    if clip in (1,2):
                        # The original contact and recoil key the face in world
                        # space; let the actor carry that reach as a whole.
                        delta+=mapped(head[None,:],head_lookup,keyed-delta)[0]-head
                # A creeping annelid rests along its continuous belly. Driving
                # a narrow toe band into the ground buckles its front ring.
                contact= None if clip==3 or (species==1 and clip==0) else sole
                points=grounded(delta,outline,hull_lookup,core,core_lookup,neutral_bottom,contact,grid,fit,cell_area)
                i=clip*FRAMES+k
                poses[b,i]=np.rint(delta*16).astype('<i2')
                sockets[b,i]=np.rint(points*16).astype('<i2')
    if not all(np.array_equal(poses[:,0],poses[:,clip*FRAMES]) for clip in range(1,4)):
        raise ValueError('Action entry pose diverges from idle')
    packed=poses.reshape(FACES,4*FRAMES,SIDE,SIDE,2).transpose(1,2,0,3,4)
    rgba=np.ascontiguousarray(packed).view('u1').reshape(4*FRAMES*SIDE,FACES*SIDE,4)
    target=out/f'motion-{species}.png'
    Image.fromarray(rgba, 'RGBA').save(target,compress_level=7)
    with Image.open(target) as saved:
        if not np.array_equal(np.asarray(saved),rgba): raise ValueError('Motion PNG changed')
    support=out/f'support-{species}.bin'
    support.write_bytes(b'MRP2'+bytes((species,SIDE,FACES,4*FRAMES))+tangents.tobytes()+sockets.tobytes())
    outputs=[target,support]
    for page in range(2):
        first=np.arange(page*64,(page+1)*64)
        pair=np.concatenate((dense[first],dense[(first+1)%FACES]),axis=3)
        atlas=pair.reshape(8,8,h//2,w//2,4).transpose(0,2,1,3,4).reshape(h*4,w*4,4)
        path_out=out/f'flow-{species}-{page}.png'
        Image.fromarray(atlas,'RGBA').save(path_out,compress_level=7)
        outputs.append(path_out)
    sources=[input_dir/f'neutral-{species}-{page}.png' for page in range(2)]+[source_dir/'landmarks.json']
    if flow is not None:sources.append(path)
    masks=occlusion_pages(source_dir,dense,out,species)
    if masks:
        outputs.extend(masks)
        sources.append(source_dir/f'visibility-{species}.npz')
    return {'species':species,'tangent':'parts' if flow is not None else 'local optical velocity',
            'decoded_geometry_sha256':hashlib.sha256(np.ascontiguousarray(poses).tobytes()).hexdigest(),
            'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
            'range_px':[float(poses.min()/16),float(poses.max()/16)],
            'files':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in outputs}}


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--input',required=True,type=Path)
    parser.add_argument('--source',required=True,type=Path)
    parser.add_argument('--out',required=True,type=Path)
    parser.add_argument('--species',nargs='+',type=int,default=list(range(4)))
    opt=parser.parse_args()
    marks=json.loads((opt.source/'landmarks.json').read_text(encoding='utf-8-sig'))
    opt.out.mkdir(exist_ok=True,parents=True)
    start=time.monotonic()
    summary=[]
    for species in opt.species:
        row=marks[species] if isinstance(marks,list) else marks[str(species)]
        summary.append(compile_species(opt.input,opt.source,opt.out,species,row))
        print('Completed species',species,flush=True)
    report={'facings':FACES,'phase_keys_per_action':FRAMES,'mesh_vertices':SIDE*SIDE,
            'species':summary,'seconds':round(time.monotonic()-start,3)}
    (opt.out/'motion-receipt.json').write_text(json.dumps(report,indent=2),encoding='utf8')


if __name__=='__main__':
    spec=importlib.util.spec_from_file_location('marsh_compass_source',Path(__file__).with_name('marsh_compass_rig.py'))
    compass=importlib.util.module_from_spec(spec);spec.loader.exec_module(compass)
    main()

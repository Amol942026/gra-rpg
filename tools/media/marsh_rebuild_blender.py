"""Animate the new marsh paintings as connected planar meshes through Blender MCP."""
import importlib.util
import math
from pathlib import Path
import struct
import sys
import time

import bpy
import gpu
from gpu_extras.batch import batch_for_shader
import numpy as np


CELLS = {0: (140, 100, 70, 92), 3: (204, 164, 102, 159)}
SIDE = 13
STEPS = 12
MARGIN = 12


def module(repo, name):
    spec = importlib.util.spec_from_file_location(name, repo/"tools/media"/f"{name}.py")
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


def smooth(x):
    x = np.clip(x, 0, 1)
    return x*x*(3-2*x)


def cross(p, q):
    return p[...,0]*q[...,1]-p[...,1]*q[...,0]


def support_path(phase,travel,lift):
    """Backwards support, followed by a swing with matched contact speed."""
    p=phase%1.
    stance=.60
    if p<stance:
        return travel*(.5-p/stance),0.
    t=(p-stance)/(1-stance)
    slope=-(1-stance)/stance
    u=t*t*(3-2*t)
    tangent=t*(1-t)*(1-2*t)
    return travel*(u-.5+slope*tangent),lift*math.sin(math.pi*t)**2


def silhouette(image):
    a = np.empty(image.size[0]*image.size[1]*4, "f4")
    image.pixels.foreach_get(a)
    return a.reshape(image.size[1],image.size[0],4)


class Actor:
    def __init__(self, species, heading, rgba):
        self.s = species
        self.w,self.h,self.ax,self.ay = CELLS[species]
        self.a = heading*math.tau/128
        self.y,self.x = np.mgrid[0:self.h:complex(0,SIDE),0:self.w:complex(0,SIDE)]
        self.mesh = np.stack((self.x,self.y),axis=-1)
        yy,xx = np.where(rgba[::-1,:,3]>.39)
        self.outline = np.column_stack((xx,yy)).astype("f4")
        self.lower = self.outline[yy>=np.percentile(yy,82)]
        self.left,self.top = int(xx.min()),int(yy.min())
        self.width,self.height = int(np.ptp(xx)+1),int(np.ptp(yy)+1)
        self.support = []
        # Locate the two actual painted contacts instead of lifting a generic
        # left/right quarter of an independent torso and separate limbs.
        for lo,hi in ((.03,.51),(.51,.98)):
            region = self.lower[(self.lower[:,0]>=self.left+lo*self.width)&
                                (self.lower[:,0]< self.left+hi*self.width)]
            if len(region)==0:
                region=self.lower
            depth=region[:,1].max()
            sole=region[region[:,1]>=depth-2]
            self.support.append((float(np.median(sole[:,0])),float(depth),self.width*(.095 if species==0 else .106)))

        # The deepest painted pixel changes identity when a toe or root
        # appears around the side. Estimate the contact from its whole sole.
        alpha = rgba[::-1, :, 3]
        body = alpha.sum(axis=1)
        bottom = np.searchsorted(np.cumsum(body), .985 * body.sum())
        y, x = np.indices(alpha.shape)
        near_floor = np.exp(-.5 * ((y - bottom) / (4.2 if species == 0 else 6.0))**2)
        middle = self.left + self.width / 2
        right = smooth(.5 + (x - middle) / (self.width * .23))
        self.gait_support = []
        for half in (1 - right, right):
            weight = alpha * near_floor * half
            weight /= weight.sum()
            self.gait_support.append(((weight*x).sum(), (weight*y).sum(), self.support[0][2]))
        self.gait_shape = (self.left, self.top, self.width, self.height)

    def patch(self,x,y,rx,ry):
        return np.exp(-((self.x-x)/rx)**2-((self.y-y)/ry)**2)

    def marks(self,delta,walk=False):
        support = self.gait_support if walk else self.support
        _, top, width, height = self.gait_shape if walk else (self.left,self.top,self.width,self.height)
        feet=[]
        for x,y,r in support:
            dx,dy=self.at(delta,np.array([[x,y]]))[0]
            feet.extend((x+dx-self.ax,y+dy-self.ay,r))
        fx=self.ax+.26*width*math.cos(self.a)
        fy=top+.36*height
        dx,dy=self.at(delta,np.array([[fx,fy]]))[0]
        return [*feet,fx+dx-self.ax,fy+dy-self.ay]

    def at(self,delta,points):
        px=np.clip(points[:,0]*(SIDE-1)/self.w,0,SIDE-1.001)
        py=np.clip(points[:,1]*(SIDE-1)/self.h,0,SIDE-1.001)
        ix,iy=px.astype(int),py.astype(int)
        tx,ty=(px-ix)[:,None],(py-iy)[:,None]
        return ((1-tx)*(1-ty)*delta[iy,ix]+tx*(1-ty)*delta[iy,ix+1]+
                (1-tx)*ty*delta[iy+1,ix]+tx*ty*delta[iy+1,ix+1])

    def keeper_arm(self, side, turn, bend, weight):
        """Rotate both links around their shared joints before skinning the cuff."""
        _, top, width, height = self.gait_shape
        shoulder = np.array((self.ax + side*.31*width, top + .28*height))
        elbow = np.array((self.ax + side*.43*width, top + .48*height))

        def rotation(angle):
            c, s = math.cos(angle), math.sin(angle)
            return np.array(((c, s), (-s, c)))

        upper_rotation = rotation(turn)
        posed_elbow = shoulder + (elbow - shoulder) @ upper_rotation
        upper = shoulder + (self.mesh - shoulder) @ upper_rotation
        forearm = posed_elbow + (self.mesh - elbow) @ rotation(turn + bend)
        joint = smooth((self.y - elbow[1] + 5.) / 10.)[..., None]
        return (upper + (forearm - upper)*joint - self.mesh)*weight[..., None]

    def pose(self,clip,t):
        frog=self.s==0
        _, top, width, height = self.gait_shape if clip==0 else (self.left,self.top,self.width,self.height)
        support = self.gait_support if clip==0 else self.support
        c,d=math.cos(self.a),math.sin(self.a)
        trunk=self.patch(self.ax,top+height*(.50 if frog else .42),width*.59,height*.36)
        head=self.patch(self.ax+width*c*(.31 if frog else .12),
                        top+height*(.30+.16*d if frog else .24),width*.29,height*.29)
        foot=[self.patch(x,y,width*.19,height*(.29 if frog else .26)) for x,y,_ in support]
        row = self.y/self.h
        limb = smooth((row-(.71 if frog else .62))/(.18 if frog else .24))
        side = (self.x-self.ax)/self.w
        sleeve = smooth((row-.30)/.17)*(1-smooth((row-.75)/.16))
        arms = [smooth((direction*side-.145)/.15)*sleeve for direction in (-1,1)]
        # The trunk is a rigid translating island. Only the cuff and the
        # lower legs use skin weights. Motion cannot leak into the chest.
        rigid = 1-limb
        def pulse(start,peak,end):
            return smooth((t-start)/(peak-start))*smooth((end-t)/(end-peak))
        dx=np.zeros_like(self.x);dy=np.zeros_like(dx)
        posed_arms = np.zeros_like(self.mesh)
        if clip==0:
            travel=21. if frog else 27.
            lift=4.5 if frog else 7.3
            left= support_path(t,travel,lift)
            right=support_path(t+.5,travel,lift)
            stride=limb*(left[0]*foot[0]+right[0]*foot[1])
            clearance=limb*(left[1]*foot[0]+right[1]*foot[1])
            swing=math.sin(t*math.tau)
            dx+=c*stride
            if not frog:
                for side,weight in zip((-1,1),arms):
                    step = side*swing
                    turn = (.16 + .07*abs(c))*step
                    bend = .19*step - .09*step*step
                    posed_arms += self.keeper_arm(side, turn, bend, weight)
            dy+=.46*d*stride-clearance-(1.35 if frog else 1.2)*swing*swing*rigid
        elif clip==1:
            wind=pulse(0,.30,.52)
            impact=pulse(.29,.55,.81)
            away=-7*wind+(14 if frog else 12)*impact
            reach=away*rigid
            arm=arms[0]*(.5-.5*c)+arms[1]*(.5+.5*c)
            dx+=c*reach
            dy+=.46*d*reach+rigid*(6*wind-3*impact)
            if frog:
                dx+=c*arm*(-2.8*wind+5.6*impact)
            else:
                for side,weight in zip((-1,1),arms):
                    near = .72 + .28*side*c
                    turn = side*near*(.32*wind + .46*impact)
                    bend = side*near*(.42*wind - .23*impact)
                    posed_arms += self.keeper_arm(side, turn, bend, weight)
        elif clip==2:
            back=pulse(0,.21,.68)
            settle=pulse(.41,.74,1)
            p=-8.5*back+2.3*settle
            dx+=c*p*rigid
            dy+=(.46*d*p+1.65*back)*rigid
        else:
            knee=smooth(t/.38)
            drop=smooth((t-.20)/.69)
            dy+=knee*(5*trunk+2.6*head)-4.4*drop*foot[1]
            dx+=drop*(3*foot[1]-4.2*foot[0])
        local=np.stack((dx,dy),axis=-1)+posed_arms
        # Limit only the proposed local deformation. One wound mesh then
        # supplies all pixels, with no inverted faces or second body layer.
        area=self.w*self.h/(SIDE-1)**2
        def face_area(scale):
            p=self.mesh+local*scale
            a,b,q,e=p[:-1,:-1],p[:-1,1:],p[1:,1:],p[1:,:-1]
            return min(cross(b-a,q-a).min(),cross(q-a,e-a).min())
        if face_area(1.)<area*.36:
            lo,hi=0.,1.
            for _ in range(12):
                mid=(lo+hi)*.5
                if face_area(mid)<area*.36:hi=mid
                else:lo=mid
            local*=lo
        if clip==3:
            sign=1 if c>=0 else -1
            # The low toad settles chest-first. It cannot fall through the
            # same upright-to-side arc as the standing keeper.
            angle=(.36*c+.17*(1-abs(c)) if frog else -1.29*sign)*drop
            pivot=np.array([self.ax,self.top+height*.64])
            rotation=np.array(((math.cos(angle),math.sin(angle)),(-math.sin(angle),math.cos(angle))))
            collapsed=(self.mesh+local-pivot)@rotation
            if frog:
                collapsed[...,1]*=1-.28*drop
            local=collapsed+pivot-self.mesh
            floor=(self.outline+self.at(local,self.outline))[:,1].max()
            local[...,1]+=(self.ay-.5-floor)*smooth(t/.28)
        return local


def steady_support(actors):
    """Remove pixel-level jumps in the foot sockets as the silhouette turns."""
    originals=np.asarray([a.support for a,_ in actors])
    target=np.zeros_like(originals)
    total=0.
    for shift in range(-5,6):
        weight=math.exp(-.5*(shift/2.1)**2)
        target+=weight*np.roll(originals,shift,axis=0)
        total+=weight
    for (actor,_),support in zip(actors,target/total):
        actor.support=support


def steady_gait(actors):
    """Fit support and upper-body landmarks on the same closed yaw curve."""
    feet = np.array([actor.gait_support for actor, _ in actors])
    body = np.array([actor.gait_shape for actor, _ in actors])
    # A periodic bending penalty leaves broad turns and stance differences
    # intact. The wrap between the last heading and east shares the curve.
    def fit(points, cutoff):
        bands = np.arange(len(points)//2 + 1)
        response = 1 / (1 + (bands / cutoff)**4)
        response = response.reshape((-1,) + (1,) * (points.ndim - 1))
        return np.fft.irfft(np.fft.rfft(points, axis=0) * response, n=len(points), axis=0)

    for (actor, _), support, shape in zip(actors, fit(feet, 7.), fit(body, 8.)):
        actor.gait_support = support
        actor.gait_shape = shape


def bake_walk(painter, direct, shader, buffer, texture, actor, output, index, delta):
    w,h=actor.w,actor.h
    width,height=w+2*MARGIN,h+2*MARGIN
    sub=3
    p=actor.mesh+delta+MARGIN
    positions=np.column_stack((p[...,0].ravel()*2/width-1,1-p[...,1].ravel()*2/height,np.zeros(SIDE**2))).astype("f4")
    uv=np.column_stack((actor.x.ravel()/w,1-actor.y.ravel()/h)).astype("f4")
    tris=[(a,a+1,a+SIDE+1) if t==0 else (a,a+SIDE+1,a+SIDE)
          for row in range(SIDE-1) for col in range(SIDE-1) for t in (0,1) for a in (row*SIDE+col,)]
    fb=buffer[0]
    with fb.bind():
        gpu.state.viewport_set(0,0,width*sub,height*sub)
        fb.clear(color=(0,0,0,0),depth=1.)
        shader.bind()
        shader.uniform_sampler("paint",texture)
        shader.uniform_float("light",1)
        shader.uniform_float("partId",1)
        shader.uniform_float("clipLine",(0.,0.,1.))
        shader.uniform_float("wrapPaint",(0.,0.,0.))
        batch_for_shader(shader,"TRIS",{"point":positions,"brush":uv,"delta":np.zeros_like(uv)},indices=tris).draw(shader)
        raw=fb.read_color(0,0,width*sub,height*sub,4,0,"UBYTE")
        raw=np.frombuffer(raw,"u1").reshape(height*sub,width*sub,4)[::-1]
        frame=direct.resolve(raw,height,width,sub)
    if frame[:2,:,3].any() or frame[-2:,:,3].any() or frame[:,:2,3].any() or frame[:,-2:,3].any():
        raise ValueError(f"Clipped walking silhouette {actor.s}/{index}")
    painter(output/f"{index:04}.png",frame)


def run(repo, output, project, motion_only=False):
    repo,output,project=map(Path,(repo,output,project))
    start=time.perf_counter()
    render=module(repo,"marsh_compass_blender")
    direct=module(repo,"marsh_direct_bake")
    save=module(repo,"marsh_turn_bake").save_png
    author=module(repo,"marsh_motion_blender")
    shader=render.program()
    created=[]
    results={}
    depth=gpu.state.depth_test_get()
    blend=gpu.state.blend_get()
    viewport=gpu.state.viewport_get()
    try:
        gpu.state.blend_set("NONE")
        gpu.state.depth_test_set("NONE")
        for s in CELLS:
            w,h=CELLS[s][:2]
            since=time.perf_counter()
            raw=output/"frames"/str(s)
            walk=output/"walk"/str(s)
            walk.mkdir(exist_ok=True,parents=True)
            target=render.target((w+2*MARGIN)*3,(h+2*MARGIN)*3)
            palette=np.zeros((h*8,w*8,4),'u1')
            poses=np.zeros((128,64,SIDE,SIDE,2),dtype='<i2')
            marks=[]
            walk_marks=[]
            mats={}
            actors=[]
            for b in range(128):
                p=raw/f"{b*4:03}.png"
                img=bpy.data.images.load(str(p),check_existing=False)
                img.colorspace_settings.name="Non-Color"
                image=silhouette(img)
                bpy.data.images.remove(img)
                actors.append((Actor(s,b,image),image))
            steady_support(actors)
            steady_gait(actors)
            for b,(a,image) in enumerate(actors):
                if not motion_only:
                    texture=gpu.types.GPUTexture((w,h),format="RGBA16F",
                                                 data=gpu.types.Buffer("FLOAT",image.size,image.ravel()))
                    for i in range(STEPS):
                        delta=a.pose(0,i/STEPS)
                        walk_marks.append(a.marks(delta,walk=True))
                        bake_walk(save,direct,shader,target,texture,a,walk,b*STEPS+i,delta)
                for clip in range(4):
                    for frame in range(16):
                        delta=a.pose(clip,frame/(16 if clip==0 else 15))
                        marks.append(a.marks(delta,walk=clip==0))
                        poses[b,16*clip+frame]=np.rint(delta*16).astype('<i2')
                torso = poses[b,:48,3:7,5:8].astype(np.int32)
                if np.any(torso != torso[:, :1, :1]):
                    raise ValueError(f"Torso stretched at heading {b} on actor {s}")
                row,col=divmod(b%64,8)
                palette[row*h:(row+1)*h,col*w:(col+1)*w]=(image[::-1]*255+.5).astype("u1")
                if b%64==63:
                    path=output/f"neutral-{s}-{b//64}.png"
                    save(path,palette)
                    mats[b//64]=author.artwork(path,f"Rysunki {s} | {b//64}")[0]
            rg=poses.transpose(1,2,0,3,4).copy().view("u1").reshape(64*SIDE,128*SIDE,4)
            save(output/f"motion-{s}.png",rg)
            (output/f"support-{s}.bin").write_bytes(b"MRP2"+bytes((s,SIDE,128,64))+
                bytes(128*SIDE**2*4)+np.rint(np.array(marks)*16).astype("<i2").tobytes())
            if not motion_only:
                (output/f"walk-support-{s}.bin").write_bytes(struct.pack("<4sHH",b"MWS1",s,len(walk_marks))+
                    np.rint(np.array(walk_marks)*16).astype("<i2").tobytes())
            for b in (8,24,40,56,72,88,104,120):
                scene,_=author.add_scene(s,b,mats[b//64],poses[b].astype("f4")/16.)
                created.append(scene)
            results[s]=round(time.perf_counter()-since,3)
        bpy.data.libraries.write(str(project),set(created),compress=True,fake_user=True)
        return {"render_seconds":results,"total_seconds":round(time.perf_counter()-start,3),
                "walk_cels":0 if motion_only else len(results)*128*STEPS,
                "views":len(results)*512,"action_poses":len(results)*128*64}
    finally:
        gpu.state.depth_test_set(depth)
        gpu.state.blend_set(blend)
        gpu.state.viewport_set(*viewport)

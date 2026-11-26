"""Screen-space cutout rig for the Marsh. Axes in local game pixels; y is down.

The joints trace ellipses on the XY drawing. A separate ink-order coordinate
allows the nearer foot, plate or branch to cover the farther one. All editable
Blender meshes are flat XY artwork.
"""
import math
from dataclasses import dataclass
import numpy as np


SIZE = ((140,100,70,92),(164,96,82,89),(120,120,60,112),(204,164,102,159))
GRID = 11
V,U = np.mgrid[0:1:complex(0,GRID),0:1:complex(0,GRID)]


@dataclass
class Piece:
    label: str
    paint: str
    xy: np.ndarray
    order: float
    shade: float
    clip: tuple = (0.,0.,1.)
    under: float | None = None


class Drawing:
    def __init__(self,s,i):
        self.s = s
        self.a = i*math.tau/128
        self.c,self.d = math.cos(self.a),math.sin(self.a)
        self.iso = (.44,.43,.42,.44)[s]
        # Calibrate the cast once. Azimuth only changes the joint positions,
        # the orientation of the attached drawings and their overlap.
        self.sx,self.sy=((1.24,1.025),(1.51,1.23),(1.24,1.),(1.40,.995))[s]
        self.parts = []
        self.land = {}
        self.feet = []

    def joint(self,f,l,height):
        depth = f*self.d+l*self.c
        return np.array([f*self.c-l*self.d, self.iso*depth-height, depth],dtype=float)

    def card(self,label,paint,p,w,h,angle=0.,flip=1.,shade=1.,sway=0.,
             layer=0.,taper=0.):
        x=((U-.5)*w*flip*(1+taper*(V-.5))+sway*np.sin(np.pi*V))*self.sx
        y=(V-.5)*h*self.sy
        ca,sa=math.cos(angle),math.sin(angle)
        xy=np.stack((p[0]*self.sx+ca*x-sa*y,p[1]*self.sy+sa*x+ca*y),axis=-1)
        self.parts.append(Piece(label,paint,xy,layer,shade))
        return self.parts[-1]

    def link(self,label,paint,p,q,width,flip=1.,bend=0.,shade=1.,layer=0.,pad=1.):
        delta=q-p
        # Recover the fixed lengths of the joint offsets on the ground and
        # vertically. Overlapping ink at the joints takes up foreshortening.
        height=self.iso*delta[2]-delta[1]
        span=math.sqrt(delta[0]**2+delta[2]**2+height**2)
        angle=math.atan2(delta[1]*self.sy,delta[0]*self.sx)-math.pi/2
        return self.card(label,paint,(p+q)/2,width,span*pad,angle,
                         flip=flip,sway=bend,layer=layer,shade=shade)

    def around(self,piece,back,front,view,outward):
        """Slide the visible rim in front of its neighbour at a changing joint.

        A second, uncut impression at the back always supplies the part's
        exterior silhouette. Both impressions carry the same pigment and ID.
        Positive view is towards the viewer. The sweep starts on the side
        facing away from the body centre, in the actual texture orientation.
        """
        piece.under,piece.order=back,front
        if view<=-.65 or view>=.65:
            piece.clip=(0.,0.,1. if view>0. else -1.)
            return
        direction=np.asarray(outward,dtype=float)*[self.sx,self.sy]
        if np.linalg.norm(direction)<.01:
            direction=np.array([0.,1.])
        xy=piece.xy
        dx=(xy[0,-1]-xy[0,0])@direction
        dy=(xy[0,0]-xy[-1,0])@direction
        width=abs(dx)+abs(dy)
        t=(view+.65)/1.3
        t=t*t*(3.-2.*t)
        piece.clip=(dx/width,dy/width,t-(max(dx,0.)+max(dy,0.))/width)

    def mark(self,key,p):
        self.land[key]=p[:2].tolist()


def toad(r):
    """Hunched mossy back, hinged throat, two independently orbiting eyes and four feet."""
    c,d=r.c,r.d
    # A single skin painting wraps the entire torso. Independent patches of
    # the same hide used to slide over the spine when the frog turned.
    body=r.joint(-2,0,32)
    r.card("01 Korowaty grzbiet","toad-back",body,74.,56.,shade=1.02)
    chin=r.joint(13.+11.*d*d,0,34.)
    p=r.card("03 Pysk i podgardle","toad-chin",chin,41.,36.,angle=.035*c,shade=1.02)
    r.around(p,-5.,12.,1.38*d-.25,chin[:2]-body[:2])
    r.mark("head",r.joint(26,0,39))
    r.mark("body",body)
    for side in (-1,1):
        q=str(side)
        cheek=r.joint(23,11.5*side,36.5)
        p=r.card("03 Policzek i oko "+q,"toad-cheek",cheek,37.,31.,angle=-.075*c,
                 flip=-side,shade=1.00)
        r.around(p,-7.+.1*side,13.+.1*side,1.12*side*c-.48-.25*abs(d),cheek[:2]-body[:2])
        hip=r.joint(-21,22*side,18)
        knee=r.joint(-26,23*side,14)
        hind=r.joint(-22,33*side,0)
        p=r.card("05 Udo "+q,"toad-hip",hip,30.,24.,angle=.16*c+.14*side*d,flip=side)
        hip_view=-.60*d+.85*side*c
        r.around(p,-10.+.1*side,6.+.1*side,hip_view,hip[:2]-body[:2])
        p=r.link("06 Tylna stopa "+q,"toad-hindfoot",knee,hind,16.,flip=side,
                 bend=1.8*side,shade=.96,pad=1.19)
        r.around(p,-11.+.1*side,5.+.1*side,hip_view,hind[:2]-body[:2])
        arm=r.joint(17,20*side,33)
        front=r.joint(28,24*side,0)
        p=r.link("07 Przednia łapa "+q,"toad-arm",arm,front,17.,
                 flip=side,bend=2.8*side,pad=1.05)
        r.around(p,-13.+.1*side,8.+.1*side,.52*d+.85*side*c,front[:2]-body[:2])
        r.feet.extend([(front,6.),(hind,5.)])


def leech(r):
    """Eighteen interlocking chitin annuli follow a single curved 2D spine."""
    centers=[]
    # Place the fine terminal plates closer together where the spine curls
    # back and narrows, so ink continues all the way to the tip in profile.
    knots=[1.-(1.-i/18)**1.30 for i in range(19)]
    n=len(knots)
    for t in knots:
        f=35-121*t+68*t**3.8
        l=-5+4.2*math.sin(t*math.pi) + (31*t**2.5)
        z=14+13*math.sin(math.pi*t)**.88-9*t*t
        p=r.joint(f,l,z)
        r.mark(f"ring-{len(centers):02d}",p)
        centers.append(p)
    r.mark("head",centers[0]);r.mark("body",centers[n//2])
    for i,p in enumerate(centers):
        t=knots[i]
        # Within one band the painted transverse arc is independent of the
        # next. The taper and underlap keep the tail and mouth continuous.
        thick=(15.4+4.7*math.sin(math.pi*t))*(1-.85*t**2.5)
        tangent=centers[min(i+1,n-1)]-centers[max(0,i-1)]
        side=tangent[0]/math.hypot(tangent[0],tangent[2])
        # Individual bands keep their ridges aligned along the back. The
        # spine supplies the turn; each round plate only banks a few degrees
        # as its neighbours and the nearer edge cover the rest of the band.
        angle=math.radians(44.+7.*side)
        pigments=("leech-ring","leech-ring","leech-ring-dark","leech-ring")
        # The top row closes head to tail when viewed from the mouth.
        # A nearer bend exposes the opposite lip of each rigid scale.
        depth=tangent[2]/math.hypot(tangent[0],tangent[2])
        far,near=-2.*i,8.+2.*i
        plate=r.card(f"01 Segment {i:02d}",pigments[i%4],p,thick*1.84,6.8+thick*.94,angle,
                     shade=.91+.065*math.sin(t*math.pi)+.025*math.cos(i*.72),taper=-.09)
        r.around(plate,far,near,depth,tangent[:2])
        if i%2==0 and i<n-2:
            f=34-121*t+68*t**3.8
            l=-5+4.2*math.sin(t*math.pi)+31*t**2.5
            z=14+13*math.sin(math.pi*t)**.88-9*t*t
            root=r.joint(f,l,z+thick*.56)
            # Leave filtering room over the tallest ridge in frontal views.
            tip=r.joint(f-4.8,l+1.0,z+thick*1.20-.9)
            spike=r.link(f"02 Kolec {i:02d}","leech-spine",tip,root,max(2.2,thick*.42),
                         flip=-1,pad=1.03)
            r.around(spike,far-.25,near-.25,depth,tangent[:2])
    mouth=r.joint(39.5,-5,14.7)
    # The rim and the first dorsal scale occlude the tooth rosette from behind.
    mouth_ink=r.card("03 Rozeta zębów","leech-mouth",mouth,18.,24.,angle=.10*r.c)
    r.around(mouth_ink,-45.,50.,r.d+.30,mouth[:2]-centers[1][:2])
    r.feet=[(r.joint(30,-4,1),7.),(r.joint(-16,7,1),6.)]


def wisp(r):
    """The reel-like lantern, its two sigils, and separate curling fronds."""
    c,d=r.c,r.d
    lamp=r.joint(2,0,64)
    r.mark("body",lamp); r.mark("core",lamp); r.mark("head",lamp)
    r.card("01 Latarnia","wisp-lamp",lamp,39.,40.,angle=-.04*c,layer=24.,shade=1.04)
    sprig=r.joint(-2,0,78)
    r.card("02 Korona i ogonek","wisp-crown",sprig,44.,43.,angle=-.10*c,layer=15.)
    for side in (-1,1):
        seed=r.joint(10,7*side,63)
        eye=r.card("03 Znamię "+str(side),"wisp-eye",seed,7.,9.,angle=.11*c,flip=side)
        r.around(eye,12.+.1*side,25.+.1*side,.82*d+.58*side*c+.20,seed[:2]-lamp[:2])
    for i in range(11):
        a=i*math.tau/11+.24
        ux,uy=math.cos(a),math.sin(a)
        f,l=13*ux,11*uy
        height=65+6*math.sin(i*2.4)
        root=r.joint(f,l,height)
        if i%3==0:
            tip=r.joint(f+20*ux-9,l+17*uy,height-19-7*(i%2))
            delta=tip-root
            span=math.sqrt((20*ux-9)**2+(17*uy)**2+(19+7*(i%2))**2)
            p=r.card(f"04 Liść lotny {i}","wisp-fan",(root+tip)/2,span*.80+4.,
                   10+2*math.cos(i),angle=math.atan2(delta[1]*r.sy,delta[0]*r.sx),
                   shade=.90+.09*math.sin(i*1.7))
        else:
            tip=r.joint(f-11-4*math.sin(i),l+8*uy,7+(i*9)%21)
            paint="wisp-tail" if i%2 else "wisp-leaf"
            p=r.link(f"05 Wić {i}",paint,root,tip,9+2*math.sin(i+2),
                   flip=1 if i%2 else -1,bend=3*ux,shade=.95+.10*math.sin(i*1.3),pad=1.03)
        r.around(p,-12.+i*.4,8.+i*.4,ux*d+uy*c,tip[:2]-lamp[:2])
    tip=r.joint(-10,1,0)
    r.link("06 Długa wić","wisp-tail",r.joint(-4,2,52),tip,13.,bend=1.,layer=-14.)
    r.feet=[(tip,4.),(r.joint(-8,4,3),3.)]


def keeper(r):
    """The bronze bell moves around the mantle; hands, shoulders, feet and chain occlude independently."""
    c,d=r.c,r.d
    body=r.joint(-2,0,90)
    r.mark("body",body)
    pelvis=r.joint(-3,0,64)
    r.card("00 Karpa","keeper-skirt",pelvis,57.,61.,angle=-.03*c,
           layer=-1.,shade=.89,taper=-.27)
    r.card("01 Płaszcz korzeni","keeper-mantle",body,71.,81.,angle=-.05*c)
    crown=r.joint(-3,0,119)
    r.card("01 Korona mchów","keeper-crown",crown,57.,26.,angle=.05*c,layer=1.)
    for side in (-1,1):
        lab=str(side)
        hip=r.joint(-2+4*side,15.5*side,64)
        foot=r.joint(4+13*side,23*side,0)
        leg_view=side*.84*c+(.12+side*.3)*d
        p=r.link("02 Korzeń nośny "+lab,"keeper-shin",hip,foot,27,
                 flip=side,bend=3.9*side,shade=.93,pad=1.04)
        r.around(p,-18.+side*.1,-7.+side*.1,leg_view,foot[:2]-pelvis[:2])
        r.feet.append((foot,9.))
        toe=r.joint(8+13*side,23*side,3)
        p=r.card("02 Stopa "+lab,"keeper-foot",toe,26.,12.,
                 angle=.055*c+.04*side*d,flip=side)
        r.around(p,-17.+side*.1,-6.+side*.1,leg_view,foot[:2]-pelvis[:2])
        shoulder=r.joint(1+3*side,23*side,112)
        elbow=r.joint(4+6*side,34*side,76)
        hand=r.joint(14+10*side,39*side,31+2*side)
        # From behind, the mantle drapes across the shoulders and upper arms.
        arm_view=side*.96*c+.19*d+.70*min(0.,d)
        direction=elbow[:2]-body[:2]
        p=r.link("03 Ramię "+lab,"keeper-upper",shoulder,elbow,20.,flip=side,
                 bend=.8*side,shade=.94,pad=1.23)
        r.around(p,-13.+side*.1,4.+side*.1,arm_view,direction)
        p=r.link("04 Dłoń "+lab,"keeper-hand",elbow,hand,20.,flip=side,
                 bend=-2.2*side,shade=.96,pad=1.12)
        r.around(p,-14.+side*.1,3.+side*.1,arm_view,direction)
        pauldron=r.joint(-2.5,22*side,111)
        p=r.card("05 Naramię "+lab,"keeper-shield",pauldron,32.,37.,angle=-.23*side*d+.07*c,flip=side)
        r.around(p,-12.+side*.1,7.+side*.1,arm_view,direction)
    chain=r.joint(-11,-5,67)
    p=r.card("06 Łańcuch","keeper-chain",chain,19,90,angle=.13*c)
    r.around(p,-16.,2.,-.88*d-.4*c,chain[:2]-body[:2])
    # The keeper carries the bell on a wooden crook. At each transition
    # between front and rear the whole bell is outside the painted torso.
    bell=r.joint(54.,-10.,86.)
    near=bell[2]>=0.
    socket=r.joint(12.,-7.,119.)
    hook=r.joint(54.,-10.,117.)
    collar=socket*.27+hook*.73
    for label,a,b,width,layer in (
        ("Drzewce",socket,collar,7.,-20.),
        ("Oplot",collar,hook,6.,17. if near else -23.),
        ("Rzemień",hook,r.joint(54.,-10.,103.5),3.4,17. if near else -23.),
    ):
        delta=(b[:2]-a[:2])*[r.sx,r.sy]
        r.card("07 Jarzmo | "+label,"keeper-yoke",(a+b)*.5,width,
               math.hypot(*delta)/r.sy+1.,angle=math.atan2(delta[1],delta[0])-math.pi/2,layer=layer)
    r.card("07 Dzwon","keeper-bell",bell,32.,43.,angle=-.035*c,
           layer=18. if near else -24.,shade=1.09)
    r.mark("head",bell)


def frame(s,i):
    r=Drawing(s,i)
    (toad,leech,wisp,keeper)[s](r)
    for key,(x,y) in r.land.items():
        r.land[key]=[x*r.sx,y*r.sy]
    r.feet=[(p*np.array([r.sx,r.sy,1.]),radius*r.sx) for p,radius in r.feet]
    if "core" not in r.land:
        r.land["core"]=r.land["head"]
    return r

"""Author and bake the jackal as flat painted meshes in Blender.

Run through execute_blender_code in an empty Blender session. Set JOB to the
absolute path of a JSON file containing paint, output and optionally preview.
"""

import json
import math
from pathlib import Path
import time

import bpy


WIDTH, HEIGHT, PAD = 104, 120, 2
ANCHOR_X, ANCHOR_Y = 52, 98
FACINGS, WALK, ATTACK, FALL = 48, 24, 15, 12
FRAMES = WALK + 1 + ATTACK + 2 * FALL
COLS, ROWS = 16, 16
PER_PAGE = COLS * ROWS // FRAMES
GROUND_SQUASH = .52
WALK_DISTANCE = 54
TORSO_HALF_LENGTH, TORSO_HALF_WIDTH = 29.8, 11.5
TORSO_DEPTH = .35
# Denser sampling near the rounded ends keeps the chest and rump tangent to
# the contour instead of forming a point at the first and last cross-section.
TORSO_SAMPLES = tuple(.5-.5*math.cos(math.pi*i/32) for i in range(33))
HEAD_SAMPLES = tuple(.5-.5*math.cos(math.pi*i/24) for i in range(25))


def lerp(a, b, t):
    return a + (b - a) * t


def smooth(a, b, x):
    t = min(1, max(0, (x - a) / (b - a)))
    return t * t * (3 - 2 * t)


def pulse(phase, start, peak, hold, end):
    return smooth(start,peak,phase)*(1-smooth(hold,end,phase))


def step_phase(phase, fore, side):
    return (phase+(.5 if side > 0 else 0)+(.75 if fore else 0))%1


def foot_loading(phase, fore, side):
    return pulse(step_phase(phase,fore,side),0,.087,.125,.30)


def angle_distance(a, b):
    return math.remainder(a - b, math.tau)


def fur_coordinate(u, pose, length, width):
    # In the flat drawing, reveal the visible part of one longitudinal
    # painting. Projecting its coordinates keeps the fur attached to the
    # same shoulder and haunch without dissolving into another painting.
    screen = 2*u-1
    along = length*pose.c
    around = width*pose.s
    visible = math.sqrt(max(0, 1-screen*screen))
    return .5+.5*(screen*along+visible*around)/math.hypot(along,around)


class PaintedMesh:
    def __init__(self):
        self.vertices = []
        self.faces = []
        self.paint = []
        self.material_slots = []

    def strip(self, spans, row, depth, pose, light=1, face_weight=None, bend=None,
              positions=None, hip_surface=None):
        start = len(self.vertices)
        single_paint = row in (0, 1)
        divisions = 3 if bend is not None else 1
        stride = divisions+1
        for i, (a, b) in enumerate(spans):
            u = i/(len(spans)-1) if positions is None else positions[i]
            for j in range(stride):
                v = j/divisions
                x,y = (lerp(a[0],b[0],v),lerp(a[1],b[1],v))
                if bend is not None:
                    middle = 4*v*(1-v)
                    x,y = x+(bend[0]+.94*(2*u-1))*middle,y+bend[1]*middle
                def uv(col, along):
                    return ((col+.025+.95*along)/3, 1-(row+.028+.944*v)/8)
                tone = light*lerp(1,.91 if row == 0 else .96,v)
                z = depth
                if single_paint:
                    ratio = (TORSO_HALF_LENGTH,TORSO_HALF_WIDTH) if row == 0 else (1,.76)
                    fixed = uv(0,fur_coordinate(u,pose,*ratio))
                    volume = lerp(.21,1,pose.roll) if row == 0 else .13
                    self.paint.append((fixed, fixed, (0, tone, volume, 1)))
                else:
                    side_u = 1-u if pose.c < 0 and row <= 3 else u
                    blend = smooth(.25,.96,abs(pose.s)) if face_weight is None else face_weight
                    volume = .17 if row >= 4 else 0
                    secondary = uv(1 if pose.s >= 0 else 2,u)
                    if hip_surface is not None:
                        z,secondary,blend,tone,volume = hip_surface(x,y,u,secondary,tone,volume)
                    self.paint.append((uv(0,side_u),secondary,(blend,tone,volume,1)))
                self.vertices.append((x,-y,z))
        for i in range(start,len(self.vertices)-stride,stride):
            for j in range(divisions):
                k = i+j
                self.faces.extend(((k,k+1,k+stride),(k+stride,k+1,k+stride+1)))
                self.material_slots.extend((int(single_paint),)*2)

    def ribbon(self, knots, row, depth, pose, light=1, face_weight=None, hip_surface=None):
        spans = []
        for i, (x, y, radius) in enumerate(knots):
            ax, ay, _ = knots[max(0, i-1)]
            bx, by, _ = knots[min(i+1, len(knots)-1)]
            length = max(.01, math.hypot(bx-ax, by-ay))
            dx, dy = (by-ay)*radius/length, (ax-bx)*radius/length
            spans.append(((x+dx,y+dy),(x-dx,y-dy),i/(len(knots)-1)))
        # Close the free end with a small round cap in the plane of the art.
        cap_x,cap_y = (bx-ax)/length*radius*.72,(by-ay)/length*radius*.72
        for i in range(1,5):
            t = math.pi*i/8
            cx,cy = x+cap_x*math.sin(t),y+cap_y*math.sin(t)
            sx,sy = dx*math.cos(t),dy*math.cos(t)
            spans.append(((cx+sx,cy+sy),(cx-sx,cy-sy),1))
        # Round both edges around the joints. Carry the paint coordinate with
        # the edge so the detail stays attached while the limb bends.
        def between(a,b,t):
            return (tuple(lerp(x,y,t) for x,y in zip(a[0],b[0])),
                    tuple(lerp(x,y,t) for x,y in zip(a[1],b[1])),
                    lerp(a[2],b[2],t))
        for _ in range(2):
            spans = [spans[0],*(between(a,b,t) for a,b in zip(spans,spans[1:]) for t in (.25,.75)),spans[-1]]
        self.strip([(a,b) for a,b,_ in spans],row,depth,pose,light,face_weight,
                   positions=[u for _,_,u in spans],hip_surface=hip_surface)

    def add_to(self, other, dx, dy):
        offset = len(other.vertices)
        other.vertices.extend((x+dx,y+dy,z) for x,y,z in self.vertices)
        other.faces.extend(tuple(i+offset for i in f) for f in self.faces)
        other.paint.extend(self.paint)
        other.material_slots.extend(self.material_slots)


class Pose:
    def __init__(self, yaw, kind, phase, side=1):
        self.c = math.cos(yaw)
        self.s = math.sin(yaw)
        self.phase = phase
        self.side = side
        self.walking = kind == "walk"
        self.biting = kind == "bite"
        bite, fall = self.biting, kind == "fall"
        self.drop = smooth(.035,.90,phase) if fall else 0
        self.fore_fold = smooth(.015,.42,phase) if fall else 0
        self.hind_fold = smooth(.13,.62,phase) if fall else 0
        self.roll = smooth(.11,.94,phase) if fall else 0
        self.land = smooth(.50,1,phase) if fall else 0
        self.load = pulse(phase,.02,.29,.30,.43) if bite else 0
        self.release = pulse(phase,.29,.45,.49,.88) if bite else 0
        self.brace = pulse(phase,.48,.66,.66,1) if bite else 0
        self.crouch = self.load+.23*self.brace
        self.drive = 5.4*self.release-3.8*self.load
        self.air = 3.7*pulse(phase,.32,.426,.45,.63) if bite else 0
        self.jaw = pulse(phase,.30,.402,.416,.472) if bite else 0
        head_lift = pulse(phase,.25,.365,.385,.49) if bite else 0
        head_snap = pulse(phase,.34,.448,.49,.78) if bite else 0
        head_recoil = pulse(phase,.47,.60,.64,.91) if bite else 0
        self.head_reach = 6.7*head_snap-4.8*self.load-2.4*head_recoil+2.4*self.land
        self.head_pitch = .29*self.load-.23*head_lift+.24*head_snap-.16*head_recoil
        self.neck_arc = (3.6*head_snap-2.8*self.load,4.2*self.load-4.9*head_lift-2.2*head_snap)
        self.step_angle = math.tau*phase
        weight = 2*self.step_angle
        self.hip = .92*math.sin(weight-.36)+.14*math.sin(weight*2-.9) if self.walking else 0
        self.shoulder = 1.14*math.sin(weight+2.20)+.12*math.sin(weight*2+.6) if self.walking else 0
        # Keep the supporting toe at ground height while the body settles
        # onto each paw. The shoulders take their load after the hips.
        self.hip_load = 0
        if self.walking:
            self.hip_load = sum(foot_loading(phase,False,side) for side in (-1,1))
            shoulder_load = sum(foot_loading(phase,True,side) for side in (-1,1))
            self.hip += 2.95*self.hip_load
            self.shoulder += 2.25*shoulder_load
        self.head_float = .65*self.shoulder+.48*math.sin(weight+1.32) if self.walking else 0
        self.hip += 2.2*self.load-.75*self.release+1.4*self.hind_fold*(1-self.drop)
        self.shoulder += 3.65*self.load-1.65*self.release+2.1*self.brace
        self.shoulder += 4.4*self.fore_fold*(1-self.drop)
        self.head_float += 4.1*self.load-4.6*head_lift+2.1*head_snap+2.6*self.brace
        self.head_float += 9.5*self.land

    def ground(self, along, across):
        return (along*self.c-across*self.s, (along*self.s+across*self.c)*GROUND_SQUASH)

    def point(self, along, across, height, extra=0):
        x, y = self.ground(along+self.drive, across)
        # All geometry is drawn in screen space. Layer depth only controls overlap.
        flatten_yaw = lerp(1, .45, min(1, height/50))
        roll = self.roll*self.side
        return (x+5*roll, y*flatten_yaw-height*(1-.66*self.drop) +
                extra+4*self.crouch-self.air+roll*along*.07-1.2*self.drop)

    def sockets(self, fore, side):
        along = 17.8+2.2*self.release if fore else -18.5
        across = side*(5.7 if fore else 6.0)
        high = 37.3 if fore else 38.8
        return along, across, high


def coordinates(sample, count):
    return sample/(count-1)


def weighted_joint(start, wrist, upper, lower, sign):
    dx, dy = wrist[0]-start[0], wrist[1]-start[1]
    length = max(.01, math.hypot(dx, dy))
    distance = min(upper+lower-.05, max(abs(upper-lower)+.05, length))
    ux, uy = dx/length, dy/length
    along = (upper*upper - lower*lower + distance*distance)/(2*distance)
    outward = math.sqrt(max(0, upper*upper-along*along))*sign
    return (start[0]+ux*along-uy*outward, start[1]+uy*along+ux*outward), (
        start[0]+ux*distance, start[1]+uy*distance)


def step(phase, fore, side):
    phase = step_phase(phase,fore,side)
    stance = .64 if fore else .62
    amplitude = WALK_DISTANCE*stance*.5
    if phase < stance:
        return amplitude-WALK_DISTANCE*phase, 0
    flight = (phase-stance)/(1-stance)
    # Match the planted foot's speed at both ends of the swing.
    tangent = -WALK_DISTANCE*(1-stance)
    stride = (lerp(-amplitude,amplitude,smooth(0,1,flight))
              + tangent*flight*(1-flight)*(1-2*flight))
    lift = math.sin(math.pi*flight)**2 * lerp(1.13,.87,flight)*(7.3 if fore else 6.8)
    return stride,lift


def torso_slice(torso, x, inset):
    left,right = torso[0][0][0],torso[-1][0][0]
    u = min(1-inset,max(inset,(x-left)/(right-left)))
    i = next(i for i,right_edge in enumerate(TORSO_SAMPLES[1:]) if right_edge >= u)
    t = (u-TORSO_SAMPLES[i])/(TORSO_SAMPLES[i+1]-TORSO_SAMPLES[i])
    top = lerp(torso[i][0][1],torso[i+1][0][1],t)
    bottom = lerp(torso[i][1][1],torso[i+1][1][1],t)
    return lerp(left,right,u),top,bottom


def torso_socket(torso, point):
    x,top,bottom = torso_slice(torso,point[0],.12)
    return x,min(bottom-4,max(top+3,point[1]))


def leg_socket(torso, p, fore, along, across, high, transfer):
    joint = torso_socket(torso,p.point(along,across,high,transfer))
    if fore:
        return joint
    # Draw the rear thigh from inside the haunch at all headings. This
    # gives the moving strip room to turn under the fur at the belly.
    projected = p.point(along,across*.83,high+3.2,transfer)
    x,top,bottom = torso_slice(torso,projected[0],.21)
    return (lerp(x,joint[0],p.roll),lerp(lerp(top,bottom,.43),joint[1],p.roll))


def hind_hip_surface(torso, pose, side, depth):
    left,right = torso[0][0][0],torso[-1][0][0]
    near = smooth(-.22,.48,-.86*pose.s+.64*side*pose.c)
    upright = 1-pose.roll
    # The visible thigh emerges from the same fur on the flank. Use that
    # paint at the join, then follow the leg's UVs below the upper thigh.
    def paint_at(x,y,u,secondary,tone,volume):
        if u >= .53 or upright <= 0:
            return depth,secondary,0,tone,volume
        screen_x,top,bottom = torso_slice(torso,x,.024)
        along = (screen_x-left)/(right-left)
        flank_v = min(1,max(0,(y-top)/max(.01,bottom-top)))
        flank_u = fur_coordinate(along,pose,TORSO_HALF_LENGTH,TORSO_HALF_WIDTH)
        ink = ((.025+.95*flank_u)/3,1-(.028+.944*flank_v)/8)
        blend = (1-smooth(.235,.53,u))*(1-smooth(2.25,11.5,y-bottom))*upright
        rim = smooth(.02,.16,u)*(1-smooth(.35,.52,u))
        z = lerp(depth,TORSO_DEPTH+.030,near*upright*rim)
        form = .47*(.972-.944*flank_v)**.55+.64
        flank_light = lerp(1,.91,flank_v)*lerp(1,form,lerp(.21,1,pose.roll))
        return z,ink,blend,lerp(tone,flank_light,blend),volume*(1-blend)
    return paint_at


def bite_paw(phase, fore, side):
    # Each pair has a lead and a follow-through. The rear toes push while
    # the front wrists fold, uncoil, and take the weight one at a time.
    phase -= .047 if side < 0 else 0
    if fore:
        retract = pulse(phase,.07,.23,.265,.41)
        extend = pulse(phase,.27,.445,.53,.88)
        air = pulse(phase,.13,.325,.37,.575)
        return_step = pulse(phase,.63,.74,.79,.94)
        return (-6.3*retract+12.7*extend, 11.5*air+3.0*return_step,
                3.6*retract+1.7*air, .92*retract-.32*extend)
    gather = pulse(phase,.025,.20,.265,.38)
    push = pulse(phase,.18,.36,.40,.575)
    reset = pulse(phase,.405,.59,.66,.92)
    air = pulse(phase,.385,.53,.58,.76)
    return (4.7*gather-10.0*push+6.7*reset, 6.3*air,
            3.55*push+.75*gather, .68*push-.30*reset)


def part_leg(out, p, fore, side, torso):
    along, across, high = p.sockets(fore, side)
    transfer = p.shoulder if fore else p.hip
    stride, lift = step(p.phase, fore, side) if p.walking else (0,0)
    ankle_lift, pitch = 0,0
    if p.walking:
        ankle_lift = (1.15 if fore else 1.55)*foot_loading(p.phase,fore,side)
    if p.biting:
        stride,lift,ankle_lift,pitch = bite_paw(p.phase,fore,side)
    fold = p.fore_fold if fore else p.hind_fold
    foot_along = along+stride+(1 if fore else -3)
    foot_along = lerp(foot_along,along+(-9 if fore else 10),fold)
    lift += fold*(6+3*side*p.side)
    foot_across = across+side*(1.2+min(2.1,.17*lift) if p.biting else 1.2)+p.side*p.roll*5
    actual_height = high-transfer-4*p.crouch+p.air
    flex, wrist = weighted_joint((along,actual_height),(foot_along-2.2,4.3+lift+ankle_lift),
                                 16.6 if fore else 19,22.8 if fore else 22.5,
                                 -1 if fore else 1)
    joint = leg_socket(torso,p,fore,along,across,high,transfer)
    mid = p.point(flex[0],lerp(across,foot_across,.45),flex[1],p.air)
    ankle = p.point(wrist[0],foot_across,wrist[1],p.air)
    toe_x, toe_y = p.ground(wrist[0]+2.8+p.drive,foot_across)
    toe = (toe_x+5*p.side*p.roll,toe_y-lift-1.2+fold*(4+1.8*side*p.side))
    # As the flank lands, the bent elbows and the hocks lie alongside it.
    # Give the two pairs different reach so they remain legible in the corpse.
    near = side*p.side
    lay = fold*p.roll
    mid_reach = -3.4 if fore else 6.1
    toe_reach = (11.4 if fore else 3.2)+2.6*near
    elbow = p.ground(mid_reach,side*(2.1+near))
    laid_foot = p.ground(toe_reach,side*(5.2+2.0*near))
    rest_y = joint[1]+9.4+2.2*near
    mid = (lerp(mid[0],joint[0]+elbow[0],lay),lerp(mid[1],rest_y+elbow[1]-.8,lay))
    toe = (lerp(toe[0],joint[0]+laid_foot[0],lay),lerp(toe[1],rest_y+laid_foot[1]+1.6,lay))
    ankle = (lerp(ankle[0],toe[0]-p.c*3.1,lay),lerp(ankle[1],toe[1]-2.3,lay))
    cap = (toe[0]+1.8*p.c*math.cos(pitch),toe[1]+.8*p.s+1.8*math.sin(pitch))
    upper_blend = .33 if fore else lerp(.44,.33,p.roll)
    knots = [
        (*joint,4.55 if fore else lerp(4.85,5.35,p.roll)),
        (lerp(joint[0],mid[0],upper_blend),lerp(joint[1],mid[1],upper_blend),
         5.25 if fore else lerp(6.85,6.65,p.roll)),
        (*(lerp(joint[0],mid[0],.84),lerp(joint[1],mid[1],.84)),3.48),
        (*mid,3.02),
        (*(lerp(mid[0],ankle[0],.36),lerp(mid[1],ankle[1],.36)),2.63),
        (*ankle,1.91), (*toe,2.55), (*cap,.85),
    ]
    near = (1 if fore else -1)*p.s*.70+side*p.c*.81
    depth = .15+near*.1
    hip_surface = None if fore else hind_hip_surface(torso,p,side,depth)
    out.ribbon(knots,6 if fore else 7,depth,p,.86+.12*near,0,hip_surface)


def torso_spans(p):
    centre = p.point(-1,0,45, (p.hip+p.shoulder)*.5)
    half_width = math.hypot(TORSO_HALF_LENGTH*p.c,TORSO_HALF_WIDTH*p.s)
    spans = []
    profile_weight = p.c*p.c
    rear_view = smooth(.22,.84,-p.s)
    for sample in TORSO_SAMPLES:
        u = 2*sample-1
        end_profile = math.sqrt(max(0,1-u*u))
        along = u if p.c >= 0 else -u
        hip = math.exp(-((along+.49)/.33)**2)
        chest = math.exp(-((along-.51)/.37)**2)
        waist = math.exp(-((along+.13)/.38)**2)
        upper = end_profile*(10.65+profile_weight*(3.4*hip+4.2*chest-1.8*waist))
        lower = end_profile*(10.6+profile_weight*((2.35+.85*p.hip_load)*hip+4.55*chest-3.2*waist))
        # The pelvis fills the shallow space where the two thighs emerge.
        lower += end_profile*(2.45*hip*profile_weight+3.45*rear_view)*(1-p.roll)
        transfer = (p.shoulder-p.hip)*u*p.c*.65
        tilt = u*(1.8*p.c + p.side*p.drop*2.3)
        x = centre[0]+u*half_width
        y = centre[1]+transfer+tilt
        # The rib cage and haunch retain their depth on the ground.
        upper += 1.55*end_profile*p.roll
        lower += 1.8*end_profile*p.roll
        spans.append(((x,y-upper),(x,y+lower)))
    return spans


def part_tail(out, p, torso):
    root = p.point(-23.6,0,46,p.hip)
    socket = torso_socket(torso,root)
    knots = []
    radii = (4.5,4.7,5.05,5.1,4.7,3.95,3.0,1.85,.9,.14)
    for i,radius in enumerate(radii):
        t = coordinates(i,len(radii))
        # The root follows the hip. The wave reaches the tip later.
        sway = 4.4*math.sin(p.step_angle-.8-1.2*t) if p.walking else 0
        across = t*t*(2.5+sway+2.6*p.release+p.side*5.8*p.roll)
        along = -23.6-18.8*(1-(1-t)**1.46)+1.7*t**4
        height = 46-25.2*t**1.38+3*p.release*t*t
        x,y = p.point(along,across,height,p.hip)
        seat = 1-smooth(0,.48,t)
        knots.append((x+(socket[0]-root[0])*seat,y+(socket[1]-root[1])*seat,
                      radius*(1-.13*p.drop)))
    # The rump covers the root. The rear view exposes the hanging part in
    # front of the hind legs, while the front view keeps it behind them.
    depth = lerp(.065,.287,smooth(.08,.94,-p.s))
    out.ribbon(knots,5,depth,p,.94,0)


def part_head(out, p):
    hx,hy = p.point(24+p.head_reach,0,61,p.head_float)
    bx,by = p.point(15,0,44,p.shoulder)
    # One continuous taper overlaps the chest and the base of the skull.
    # The source has the collar across u and the fur down v.
    spans = []
    for i in range(9):
        u = 2*i/8-1
        curved = 1-math.sqrt(max(0,1-u*u))
        spans.append(((hx+6.05*u,hy-2+3.6*curved),
                      (bx+9.1*u,by+3-3.2*curved)))
    # Keep the collar behind the chest at every heading.
    arc_x,arc_y = p.neck_arc
    out.strip(spans,1,.31+.035*p.s,p,bend=(arc_x*p.c,arc_y+arc_x*p.s*.22-.82))
    skull_start = len(out.vertices)
    for side in (-1,1):
        root_x = hx-p.s*4.2*side-p.c*1.05
        root_y = hy-5.4+1.35*p.c*side
        fold = p.roll*(1+.22*side*p.side)
        ear = [(root_x,root_y+1,3.05),
               (root_x-p.c*1.0-p.s*1.5*side+2.4*p.side*fold,root_y-4+1.75*fold,2.76),
               (root_x-p.c*1.2-p.s*2.0*side+4.5*p.side*fold,root_y-7.7+4.2*fold,1.48),
               (root_x-p.c*1.65-p.s*2.4*side+6.3*p.side*fold,root_y-10.45+6.4*fold,.43)]
        out.ribbon(ear,4,.51+.02*side*p.c,p,.98+.055*side*p.c)
    width = math.hypot(9.0*p.c,8.15*p.s)
    head = []
    for sample in HEAD_SAMPLES:
        u = 2*sample-1
        curve = math.sqrt(max(0,1-u*u))
        cheek = 1-.11*u*p.c
        head.append(((hx+width*u,hy-7.1*curve),
                     (hx+width*u+p.c*curve,hy+6.95*curve*cheek)))
    out.strip(head,2,.57,p,positions=HEAD_SAMPLES)
    # At the back the walnut coloured head paint covers the snout. Fade the
    # muzzle length to zero before passing behind the cheek line.
    visibility = 1-smooth(-.60,-.92,p.s)
    sx = hx+4.9*p.c
    sy = hy+2.1
    length = 8.6*abs(p.c)*visibility+.01
    direction = 1 if p.c >= 0 else -1
    nose_x = sx+direction*length
    nose_y = sy+2.1*visibility
    radius = visibility*(1-smooth(.22,.86,p.s))
    # A tiny transparent-width shape when the muzzle is entirely occluded.
    mouth = [(sx,sy,3.2*radius+.01),
             (lerp(sx,nose_x,.55),lerp(sy,nose_y,.55),2.48*radius+.01),
             (nose_x,nose_y,1.42*radius+.01)]
    opening = p.jaw*visibility
    spread = 2.45*opening*(.55+.45*p.s*p.s)
    lower_x = lerp(sx,nose_x,.82)
    lower_y = nose_y+6.25*opening
    # One dark interior joins the lips, and the lower jaw hinges under the snout.
    out.strip((((sx,sy),(sx,sy)),
               ((lower_x-spread,nose_y-.55*opening),(lower_x-spread,lower_y)),
               ((lower_x+spread,nose_y),(lower_x+spread,lower_y))),3,.597,p,.21,0)
    lower_jaw = [(sx,sy,opening*.8),
                 (lerp(sx,lower_x,.51),lerp(sy,lower_y,.69),opening*1.65),
                 (lower_x,lower_y,opening*1.05)]
    out.ribbon(lower_jaw,3,.605,p,.75,smooth(.25,.99,p.s))
    out.ribbon(mouth,3,.61,p,1,smooth(.25,.99,p.s))
    tip = -p.head_pitch*p.c+(-.43*p.c-.61*p.side*abs(p.s))*p.land
    if tip:
        c,s = math.cos(tip),math.sin(tip)
        for i in range(skull_start,len(out.vertices)):
            x,y,z = out.vertices[i]
            dx,dy = x-hx,y+hy
            out.vertices[i] = (hx+c*dx-s*dy,-hy+s*dx+c*dy,z)


def drawing(pose):
    out = PaintedMesh()
    torso = torso_spans(pose)
    part_tail(out,pose,torso)
    for fore in (False,True):
        for side in (-1,1):
            part_leg(out,pose,fore,side,torso)
    body_start = len(out.vertices)
    out.strip(torso,0,TORSO_DEPTH,pose,positions=TORSO_SAMPLES)
    body_end = len(out.vertices)
    part_head(out,pose)
    if pose.drop:
        # The spine follows the roll as a single drawing. At the ends of a
        # turn the rump, shoulders and head settle toward the chosen flank.
        angle = .29*pose.side*pose.roll*(.38+.62*abs(pose.s))
        c,s = math.cos(angle),math.sin(angle)
        pivot_y = 12
        out.vertices = [(x*c-(y-pivot_y)*s,x*s+(y-pivot_y)*c+pivot_y,z)
                        for x,y,z in out.vertices]
        # Finish on the first grounded pose. The same sample is used as loot.
        foot = min(y for _,y,_ in out.vertices)
        flank = min(y for _,y,_ in out.vertices[body_start:body_end])
        contact = lerp(foot,flank,pose.land)
        shift = -contact*pose.land
        out.vertices = [(x,y+shift,z) for x,y,z in out.vertices]
    return out


def action(frame):
    if frame < WALK:
        return "walk", frame/WALK, 1
    if frame == WALK:
        return "idle", 0, 1
    if frame <= WALK+ATTACK:
        return "bite", (frame-WALK-1)/(ATTACK-1), 1
    f = frame-WALK-ATTACK-1
    return "fall", (f%FALL)/(FALL-1), -1 if f < FALL else 1


def material(image, name, directional):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    tree = mat.node_tree
    nodes, links = tree.nodes, tree.links
    nodes.clear()
    def texture(name):
        uv = nodes.new("ShaderNodeUVMap")
        uv.uv_map = name
        tex = nodes.new("ShaderNodeTexImage")
        tex.image = image
        tex.extension = "EXTEND"
        links.new(uv.outputs["UV"],tex.inputs["Vector"])
        return tex.outputs["Color"],uv.outputs["UV"]
    attr = nodes.new("ShaderNodeVertexColor")
    attr.layer_name = "Krycie malowidła"
    data = nodes.new("ShaderNodeSeparateColor")
    links.new(attr.outputs["Color"],data.inputs["Color"])
    paint,uv = texture("Profil")
    lighting = data.outputs["Green"]
    if directional:
        tint = nodes.new("ShaderNodeMixRGB")
        links.new(data.outputs["Red"],tint.inputs[0])
        links.new(paint,tint.inputs[1])
        links.new(texture("Widok końcowy")[0],tint.inputs[2])
        paint = tint.outputs[0]
    # Cross-section lighting is baked from the same painted surface.
    def scalar(operation, *values):
        node = nodes.new("ShaderNodeMath")
        node.operation = operation
        for i,value in enumerate(values):
            if isinstance(value,(float,int)):
                node.inputs[i].default_value = value
            else:
                links.new(value,node.inputs[i])
        return node.outputs[0]
    coordinates = nodes.new("ShaderNodeSeparateXYZ")
    links.new(uv,coordinates.inputs[0])
    across = scalar("FRACT",scalar("MULTIPLY",coordinates.outputs["Y"],8))
    flank_light = scalar("MULTIPLY_ADD",scalar("POWER",across,.55),.47,.64)
    form_light = scalar("MULTIPLY_ADD",scalar("SUBTRACT",flank_light,1),data.outputs["Blue"],1)
    lighting = scalar("MULTIPLY",lighting,form_light)
    shade = nodes.new("ShaderNodeMixRGB")
    shade.blend_type = "MULTIPLY"
    shade.inputs[0].default_value = 1
    links.new(paint,shade.inputs[1])
    links.new(lighting,shade.inputs[2])
    emission = nodes.new("ShaderNodeEmission")
    links.new(shade.outputs[0],emission.inputs["Color"])
    result = nodes.new("ShaderNodeOutputMaterial")
    links.new(emission.outputs[0],result.inputs["Surface"])
    return mat


def create_object(scene, data, mat, name):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(data.vertices, [], data.faces)
    for material in mat:
        mesh.materials.append(material)
    mesh.polygons.foreach_set("material_index",data.material_slots)
    a = mesh.uv_layers.new(name="Profil")
    b = mesh.uv_layers.new(name="Widok końcowy")
    color = mesh.color_attributes.new(name="Krycie malowidła",type="FLOAT_COLOR",domain="POINT")
    uv0,uv1 = [],[]
    for loop in mesh.loops:
        s,t,_ = data.paint[loop.vertex_index]
        uv0.extend(s)
        uv1.extend(t)
    a.uv.foreach_set("vector",uv0)
    b.uv.foreach_set("vector",uv1)
    color.data.foreach_set("color",[v for _,_,c in data.paint for v in c])
    obj = bpy.data.objects.new(name,mesh)
    scene.collection.objects.link(obj)
    return obj


def setup(scene,w,h,samples=8):
    scene.render.engine = "CYCLES"
    scene.cycles.device = "CPU"
    scene.cycles.samples = samples
    scene.cycles.use_denoising = False
    scene.cycles.max_bounces = 1
    scene.cycles.use_adaptive_sampling = False
    scene.render.threads_mode = "FIXED"
    scene.render.threads = 4
    scene.render.resolution_x = w
    scene.render.resolution_y = h
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = True
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.look = "None"
    scene.render.fps = 60
    camera = bpy.data.cameras.new(scene.name)
    camera.type = "ORTHO"
    camera.ortho_scale = max(w,h)
    camera.clip_end = 60
    obj = bpy.data.objects.new("Kamera ortograficzna",camera)
    scene.collection.objects.link(obj)
    obj.location = (w*.5,-h*.5,20)
    scene.camera = obj


def turn_preview(mat):
    scene = bpy.data.scenes.new("Skręt w chodzie")
    setup(scene,WIDTH,HEIGHT,12)
    ticks = 3*FACINGS
    scene.frame_end = ticks
    for frame in range(1,ticks+1):
        direction = ((frame-1+1)//3)%FACINGS
        step = round((frame-1)/37*WALK)%WALK
        pose = drawing(Pose(direction*math.tau/FACINGS,"walk",step/WALK))
        data = PaintedMesh()
        pose.add_to(data,ANCHOR_X,-ANCHOR_Y)
        obj = create_object(scene,data,mat,f"Klatka {frame:03}")
        for visible_at in (frame-1,frame,frame+1):
            obj.hide_render = obj.hide_viewport = visible_at!=frame
            obj.keyframe_insert("hide_render",frame=visible_at)
            obj.keyframe_insert("hide_viewport",frame=visible_at)
    bpy.context.window.scene = scene
    scene.frame_set(1)
    for area in bpy.context.screen.areas:
        if area.type == "VIEW_3D":
            area.spaces.active.region_3d.view_perspective = "CAMERA"
    return scene


def master(output, mat):
    # Native shape keys put the shared 2D outline and all joints on the timeline.
    # A compact second and third view make the front and rear proportions easy to compare.
    for index, degrees in enumerate((0,90,270)):
        name = ("Bok","Przód","Tył")[index]
        scene = bpy.data.scenes.new(name)
        setup(scene,WIDTH,HEIGHT,12)
        yaw = math.radians(degrees)
        take = []
        for kind,count,seconds in (("walk",WALK,.58),("idle",1,.10),
                                   ("bite",ATTACK,.88),("fall",FALL,.31)):
            for k in range(count):
                t = k/(count if kind == "walk" else max(1,count-1))
                duration = round(60*seconds*(k+1)/count)-round(60*seconds*k/count)
                take.append((drawing(Pose(yaw,kind,t)),max(1,duration)))
        rest = PaintedMesh()
        take[0][0].add_to(rest,ANCHOR_X,-ANCHOR_Y)
        obj = create_object(scene,rest,mat,"Szakal wydmowy | wspólna siatka 2D")
        obj.shape_key_add(name="Poza bazowa")
        frame = 1
        preference = bpy.context.preferences.edit
        interpolation = preference.keyframe_new_interpolation_type
        preference.keyframe_new_interpolation_type = "LINEAR"
        for i,(pose,duration) in enumerate(take):
            key = obj.shape_key_add(name=f"Poza {i:02}")
            key.data.foreach_set("co",[a for x,y,z in pose.vertices
                                        for a in (x+ANCHOR_X,y-ANCHOR_Y,z)])
            previous = take[i-1][1] if i else 1
            tail_value = int(i==len(take)-1)
            for at,value in ((frame-previous,0),(frame,1),(frame+duration,tail_value)):
                key.value = value
                key.keyframe_insert("value",frame=at)
            frame += duration
        preference.keyframe_new_interpolation_type = interpolation
        scene.frame_start = 1
        scene.frame_end = frame-1
        scene.frame_set(7)
        for label,tick in (("Chód",1),("Ugryzienie",sum(d for _,d in take[:WALK+1])+1),
                           ("Upadek",sum(d for _,d in take[:WALK+1+ATTACK])+1)):
            scene.timeline_markers.new(label,frame=tick)
    turn_preview(mat)
    source = bpy.data.texts.new("Generator klatek szakala")
    source.write(Path(__file__).read_text(encoding="utf-8"))
    source.use_fake_user = True
    bpy.ops.wm.save_as_mainfile(filepath=str(output/"Szakal-wydmowy.blend"),compress=True)


def sheets(output,mat,preview):
    images = output/"atlasy"
    images.mkdir(exist_ok=True)
    scene = bpy.data.scenes.new("Eksport atlasów 2D")
    pitch_x,pitch_y = WIDTH+PAD,HEIGHT+PAD
    setup(scene,COLS*pitch_x,ROWS*pitch_y,8)
    measurements = []
    pages = list(range(FACINGS//PER_PAGE)) if not preview else list(preview)
    started = time.perf_counter()
    for page in pages:
        batch = PaintedMesh()
        for direction in range(PER_PAGE):
            facing = page*PER_PAGE+direction
            for frame in range(FRAMES):
                i = direction*FRAMES+frame
                kind, phase, side = action(frame)
                pose = drawing(Pose(facing*math.tau/FACINGS,kind,phase,side))
                min_x = min(x for x,_,_ in pose.vertices)
                max_x = max(x for x,_,_ in pose.vertices)
                min_y = min(y for _,y,_ in pose.vertices)
                max_y = max(y for _,y,_ in pose.vertices)
                if not (-ANCHOR_X+1<min_x<=max_x<WIDTH-ANCHOR_X-1 and
                        ANCHOR_Y-HEIGHT+1<min_y<=max_y<ANCHOR_Y-1):
                    raise ValueError(f"Poza poza komórką {facing=} {frame=} {(min_x,max_x,min_y,max_y)}")
                dx = (i%COLS)*pitch_x+1+ANCHOR_X
                dy = -(i//COLS)*pitch_y-1-ANCHOR_Y
                pose.add_to(batch,dx,dy)
        obj = create_object(scene,batch,mat,"Wyeksportowane klatki")
        path = images/f"jackal-{page:02}.png"
        scene.render.filepath = str(path)
        at = time.perf_counter()
        bpy.ops.render.render(write_still=True,scene=scene.name)
        measurements.append({"file":path.name,"seconds":round(time.perf_counter()-at,3)})
        (output/"render-progress.json").write_text(json.dumps(measurements),encoding="utf-8")
        mesh = obj.data
        bpy.data.objects.remove(obj,do_unlink=True)
        bpy.data.meshes.remove(mesh)
    return {"seconds":round(time.perf_counter()-started,3),"sheets":measurements,
            "facings":FACINGS,"walk":WALK,"attack":ATTACK,"fall":FALL,
            "columns":COLS,"rows":ROWS,"per_page":PER_PAGE,"size":[WIDTH,HEIGHT],
            "anchor":[ANCHOR_X,ANCHOR_Y],"pad":PAD}


def main(job):
    started = time.perf_counter()
    config = json.loads(Path(job).read_text(encoding="utf-8-sig"))
    output = Path(config["output"])
    output.mkdir(exist_ok=True)
    image = bpy.data.images.load(str(Path(config["paint"])),check_existing=False)
    image.pack()
    mat = (material(image,"Sierść szakala | głowa i kończyny",directional=True),
           material(image,"Sierść szakala | jeden malunek tułowia i karku",directional=False))
    if not config.get("preview"):
        master(output,mat)
    result = sheets(output,mat,config.get("preview",False))
    result["blender"] = bpy.app.version_string
    result["total_seconds"] = round(time.perf_counter()-started,3)
    (output/"render.json").write_text(json.dumps(result,indent=2),encoding="utf-8")


if __name__ == "__main__":
    main(JOB)

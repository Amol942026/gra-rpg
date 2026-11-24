"""Measure the rendered jackal poses and register their scale across a turn."""

import argparse
import json
from pathlib import Path
import struct

import numpy as np
from PIL import Image
from scipy.ndimage import gaussian_filter1d, map_coordinates

from jackal_turn_flow import FACINGS, FRAMES, GRID, HEIGHT, PAINTINGS, WALK, WIDTH, facing_cel, facing_key, field_rect


SAMPLES = 4 * len(FACINGS)
GROUND = 130
LOWER_JOINT = 98
CHEST_TOP = 38
TORSO_ROWS = np.arange(69,94,dtype=np.float32)+.5


def ease(t):
    t = np.clip(t,0,1)
    return t*t*(3-2*t)


def image(path):
    with Image.open(path) as source:
        return np.asarray(source, dtype=np.float32)


def bounds(mask):
    y,x = np.nonzero(mask)
    return np.array((x.min(), y.min(), x.max()+1, y.max()+1), dtype=float)


class Sprite:
    def __init__(self, assets):
        self.body = {view: image(assets/f"distance-{view}.png") for view in PAINTINGS}
        self.fields = image(assets/"turn-fields.png")
        self.ink = image(assets/"turn-limbs.png")[...,3]
        binary = (assets/"turn-rig.bin").read_bytes()
        if binary[:4] != b"JTR1":
            raise ValueError("Nieznany format stawów szakala")
        self.parts = np.frombuffer(binary,count=8*7,offset=4,dtype="<f4").reshape(8,7)
        self.joints = np.frombuffer(binary,offset=4+self.parts.nbytes,dtype="<f4").reshape(len(PAINTINGS),FRAMES,4,9)
        self.y,self.x = np.mgrid[:HEIGHT,:WIDTH].astype(np.float32)+.5
        self.field_uv = (self.y*GRID[1]/HEIGHT-.5,self.x*GRID[0]/WIDTH-.5)

    def distance(self, direction, phase, t, x_coords, y_coords, field_uv):
        x,y = field_rect(direction,phase)
        grid = (self.fields[y+1:y+GRID[1]+1,x+1:x+GRID[0]+1]-128)*.5
        fx,fy,bx,by = (map_coordinates(grid[...,i],field_uv,order=1,mode="nearest") for i in range(4))
        a,b = ((facing_cel(self.body,i,phase)-128)*.125 for i in (direction,direction+1))
        ma = map_coordinates(a,(y_coords-.5+by*t,x_coords-.5+bx*t),order=1,mode="constant",cval=-16)
        mb = map_coordinates(b,(y_coords-.5+fy*(1-t),x_coords-.5+fx*(1-t)),order=1,mode="constant",cval=-16)
        return ma*(1-t)+mb*t

    def contour(self, direction, phase, t):
        return self.distance(direction,phase,t,self.x,self.y,self.field_uv)>=0

    def torso_width(self, sample, phase, scale, local):
        direction, substep = divmod(sample,4)
        width,upper,lower,offset = scale
        left,right,along,back,belly,across = local
        knee = GROUND+(LOWER_JOINT-GROUND)*upper+offset
        fitted_y = LOWER_JOINT+(TORSO_ROWS-knee)/np.where(TORSO_ROWS<knee,upper,lower)
        shift = (belly-back)*(1-across)*.5
        source_y = np.interp(fitted_y,[0,CHEST_TOP,back+shift,belly-shift,LOWER_JOINT,HEIGHT],
                             [0,CHEST_TOP,back,belly,LOWER_JOINT,HEIGHT])
        x,y = np.meshgrid(self.x[0],source_y)
        uv = (y*GRID[1]/HEIGHT-.5,x*GRID[0]/WIDTH-.5)
        coverage = np.clip(self.distance(direction,phase,substep/4,x,y,uv)+.5,0,1)
        # Integrate covered width through the horizontal piecewise map.
        inside = np.clip(np.minimum(self.x[0]+.5,right)-np.maximum(self.x[0]-.5,left),0,1)
        return np.mean(coverage @ (1+(along-1)*inside))*width

    def leg_frame(self, direction, phase):
        name,mirror,frame = facing_key(direction,phase)
        joints = self.joints[PAINTINGS.index(name),frame]
        if not mirror:
            return joints
        joints = joints[[1,0,3,2]].copy()
        joints[:,0:8:2] = WIDTH-joints[:,0:8:2]
        joints[:,-1] *= -1
        return joints

    def torso_landmarks(self, sample, phase):
        direction, substep = divmod(sample, 4)
        t = substep/4
        roots = (1-t)*self.leg_frame(direction,phase)[:,:2]+t*self.leg_frame(direction+1,phase)[:,:2]
        hip, shoulder = roots.reshape(2,2,2).mean(axis=1)
        mask = self.contour(direction,phase,t)
        # Sample the ribcage between the joints, away from the tip of the
        # muzzle, raised neck and tail. End-on overlaps are faded out below.
        columns = np.round(hip[0]+(shoulder[0]-hip[0])*np.linspace(.22,.56,9)).astype(int)
        slices = [np.flatnonzero(mask[:,x]) for x in columns]
        back, belly = np.median([(s[0],s[-1]+1) for s in slices if len(s)],axis=0)
        center = (hip[0]+shoulder[0])*.5
        half = max(abs(shoulder[0]-hip[0])*.5,5.)
        return (center-half,center+half,max(back,CHEST_TOP+4),min(belly,LOWER_JOINT-4))

    def add_limbs(self, mask, direction, phase, t):
        pose = self.leg_frame(direction,phase)*(1-t)+self.leg_frame(direction+1,phase)*t
        for slot,leg in enumerate(pose):
            joints = leg[:8].reshape(4,2)
            for part,start in enumerate(joints):
                tx,ty,w,h,half,lead,trail = self.parts[4*(slot>=2)+part]
                if part<3:
                    line = joints[part+1]-start
                    span = np.linalg.norm(line)
                    axis = line/span if span>.01 else np.array((0,1))
                else:
                    theta = np.deg2rad(leg[-1])
                    span = 0
                    axis = np.array((np.sin(theta),np.cos(theta)))
                side = np.array((axis[1],-axis[0]))
                base = start-lead*axis-half*side
                top = base+(2*half)*side
                along = (span+lead+trail)*axis
                corners = np.array((base,top,base+along,top+along))
                l,u = np.maximum(np.floor(corners.min(axis=0)).astype(int),0)
                r,b = np.minimum(np.ceil(corners.max(axis=0)).astype(int),(WIDTH,HEIGHT))
                area = np.s_[u:b,l:r]
                px,py = self.x[area]-start[0],self.y[area]-start[1]
                local_x = (px*side[0]+py*side[1]+half)/(2*half)
                local_y = (px*axis[0]+py*axis[1]+lead)/(span+lead+trail)
                alpha = map_coordinates(self.ink,(ty+local_y*h-.5,tx+local_x*w-.5),order=1,mode="constant")
                mask[area] |= (local_x>=0)&(local_x<1)&(local_y>=0)&(local_y<1)&(alpha>=128)

    def measure(self, sample, phase):
        direction,substep = divmod(sample,4)
        t = substep/4
        torso = self.contour(direction,phase,t)
        l,head,r,_ = bounds(torso)
        # The midpoint of the four roots follows the ribcage without following
        # the paws or an individual shoulder turning toward the camera.
        root = (1-t)*self.leg_frame(direction,phase)[:,1]+t*self.leg_frame(direction+1,phase)[:,1]
        areas = (int(torso[:LOWER_JOINT].sum()),int(torso[LOWER_JOINT:].sum()))
        self.add_limbs(torso,direction,phase,t)
        _,_,_,bottom = bounds(torso)
        return r-l,head,bottom,float(root.mean()),*areas


def calibration(samples):
    width,top,bottom,body,upper_area,lower_area = samples.transpose(2,0,1)
    neutral_top = np.median(top,axis=0)
    neutral_body = np.median(body,axis=0)
    height = round(float(np.median(bottom-top)))
    target_bottom = neutral_top+height
    upper = (neutral_body-neutral_top)/(body-top)
    offset = neutral_top-GROUND-(top-GROUND)*upper
    mapped_joint = GROUND+(LOWER_JOINT-GROUND)*upper+offset
    lower = (target_bottom-mapped_joint)/(bottom-LOWER_JOINT)

    # Fit one projected size from the side, front and rear references. Using
    # both the span and the painted area catches bulky diagonal poses which
    # can have the right outer bounds. Keep the established slim rear view.
    quarter = SAMPLES//4
    angle = np.arange(SAMPLES)*2*np.pi/SAMPLES
    side, end = np.abs(np.cos(angle)), np.sin(angle)
    area = upper_area*upper+lower_area*lower

    cardinal_gains = np.array((.97,1.02,.97,.90))
    def projection(values, span):
        median = np.median(values,axis=1)
        cardinal = median[::quarter]*cardinal_gains
        sides = (cardinal[0]+cardinal[2])*.5
        ends = np.where(end>=0,cardinal[1],cardinal[3])
        if span:
            return np.hypot(sides*side,ends*end),median
        return ends+(sides-ends)*side**1.4,median

    expected_width, measured_width = projection(width,span=True)
    expected_area, measured_area = projection(area,span=False)
    area_width = expected_area*measured_width/measured_area
    # Smooth the departure from the projected span. Smoothing the absolute
    # span would broaden the narrow end-on views using their neighbors.
    profile = gaussian_filter1d((area_width/expected_width)**.65,1.4,mode="wrap")
    # Keep the gait's breadth variation, smoothing out the isolated angles
    # where a tail or shoulder extends past the rest of the painted contour.
    stride_width = gaussian_filter1d(width/measured_width[:,None],2.,axis=0,mode="wrap")
    horizontal = (expected_width*profile)[:,None]*stride_width/width
    adjustment = np.stack((horizontal,upper,lower,offset),-1)
    if (np.any(adjustment[...,:3]<.6) or np.any(adjustment[...,:3]>1.55)
            or np.any(np.abs(offset)>16) or not np.isfinite(adjustment).all()):
        raise ValueError("Nieprawidłowe proporcje kalibracji szakala")
    return adjustment,height


def torso_calibration(landmarks, scale):
    left,right,back,belly = landmarks.transpose(2,0,1)
    horizontal,vertical = scale[...,0],scale[...,1]
    length = (right-left)*horizontal
    depth = (belly-back)*vertical
    cardinal = [0,SAMPLES//2]
    profile_length = float(np.median(length[cardinal]))
    profile_depth = float(np.median(depth[cardinal]))
    yaw = np.arange(SAMPLES)[:,None]*2*np.pi/SAMPLES
    side = np.abs(np.cos(yaw))
    # Orthographic foreshortening of the same shoulder-to-hip distance.
    # The rear leg roots overlap in the projection. Their separation loses
    # authority over the outline as they turn away from the profile.
    along = np.clip(profile_length*side/length,.66,1.50)
    rear = np.sin(yaw)<0
    expansion = np.where(rear,ease((side-.57)/.26),1.)
    change = np.minimum(along-1,0)+np.maximum(along-1,0)*expansion
    along = 1+change*ease((side-.17)/.22)
    # The camera sees a little more of the back at three-quarter angles.
    target_depth = profile_depth+2.5*np.abs(np.sin(yaw))
    across = np.clip(target_depth/depth,.68,1.39)
    across = 1+gaussian_filter1d((across-1)*ease((side-.40)/.25),.65,axis=0,mode="wrap")
    back = gaussian_filter1d(back,.65,axis=0,mode="wrap")
    belly = gaussian_filter1d(belly,.65,axis=0,mode="wrap")
    result = np.stack((left,right,along,back,belly,across),axis=-1)
    if not np.isfinite(result).all() or (result[...,4]-result[...,3]<5).any():
        raise ValueError("Nieprawidłowe punkty odniesienia tułowia")
    return result,profile_length,profile_depth


def register_turn_width(sprite, scale, torso):
    # Fit the painted outline after local registration. A single projected
    # width between the side and end views keeps the turn free of rebounds.
    half,quarter = SAMPLES//2,SAMPLES//4
    measured = np.array([[sprite.torso_width(k,p,scale[k,p],torso[k,p])
                          for p in range(WALK+1)] for k in range(SAMPLES)])
    angle = np.arange(half)[:,None]*2*np.pi/SAMPLES
    profile_width = (measured[0]+measured[half])*.5
    edge = ease(np.sin(angle)/.49)
    for start,end_gain in ((0,1.),(half,.90)):
        end_width = measured[start+quarter]*end_gain
        target = end_width+(profile_width-end_width)*np.abs(np.cos(angle))**1.72
        span = slice(start,start+half)
        scale[span,:,0] *= 1+edge*(target/measured[span]-1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assets",type=Path,default=Path("assets/desert/jackal-turn"))
    parser.add_argument("--output",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    sprite=Sprite(args.assets)
    measures = np.array([[sprite.measure(angle,phase) for phase in range(WALK+1)] for angle in range(SAMPLES)])
    adjustment,height = calibration(measures)
    landmarks = np.array([[sprite.torso_landmarks(angle,phase) for phase in range(WALK+1)] for angle in range(SAMPLES)])
    torso,profile_length,profile_depth = torso_calibration(landmarks,adjustment)
    register_turn_width(sprite,adjustment,torso)
    if (adjustment[...,0]<.6).any() or (adjustment[...,0]>1.55).any() or not np.isfinite(adjustment).all():
        raise ValueError("Nieprawidłowa szerokość tylnej sylwetki szakala")
    args.output.mkdir(parents=True,exist_ok=False)
    (args.output/"turn-size.bin").write_bytes(struct.pack("<4sHH",b"JSC2",SAMPLES,WALK+1)+adjustment.astype("<f4").tobytes())
    (args.output/"turn-torso.bin").write_bytes(struct.pack("<4sHH",b"JTF1",SAMPLES,WALK+1)+torso.astype("<f4").tobytes())
    top,bottom = measures[...,1],measures[...,2]
    notes = {"headings":SAMPLES,"poses":WALK+1,"unadjusted_height":[int((bottom-top).min()),int((bottom-top).max())],
             "height_at_calibration_keys":height,"minimum_scales":adjustment[...,:3].min(axis=(0,1)).tolist(),
             "maximum_scales":adjustment[...,:3].max(axis=(0,1)).tolist(),
             "vertical_offset_atlas_px":[float(adjustment[...,3].min()),float(adjustment[...,3].max())],
             "profile_length_atlas_px":profile_length,"profile_depth_atlas_px":profile_depth}
    (args.output/"calibration.json").write_text(json.dumps(notes,indent=2)+"\n",encoding="utf-8")


if __name__=="__main__":
    main()

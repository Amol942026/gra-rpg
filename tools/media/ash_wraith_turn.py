"""Develop a 32-view Ash Wraith from six independently registered cloth paintings."""
import argparse
from pathlib import Path
import numpy as np
from PIL import Image
from scipy import ndimage as nd
import cv2
from ash_wraith_paint import SIZE, ORDER
from obsidian_sentinel_turn import film
from caldera_turn_registration import optical, remap

SWATCH=96
Y,X=np.mgrid[:SIZE,:SIZE].astype(np.float32)
FRONT=(115,111,96,80,76,91,96,100)


class Cloth:
    def __init__(self,image,p,d):
        self.im=image
        rgba=np.asarray(image)
        alpha=rgba[...,3]/255
        solid=alpha>.3
        near=nd.distance_transform_edt(~solid,return_distances=False,return_indices=True)
        self.color=rgba[tuple(near)][...,:3].astype(np.float32)
        self.color[solid]=rgba[...,:3][solid]
        self.alpha=alpha.astype(np.float32)
        rows=np.where(solid.any(axis=1))[0]
        fy=np.linspace(rows[0]-.8,rows[-1]+.8,SIZE)
        # The changing size of the sleeve does not change the local weave.
        self.top,self.bottom=fy[0],fy[-1]
        self.left=nd.gaussian_filter1d(np.interp(fy,rows,[np.flatnonzero(solid[y])[0] for y in rows]),2.1)-1.5
        self.right=nd.gaussian_filter1d(np.interp(fy,rows,[np.flatnonzero(solid[y])[-1] for y in rows]),2.1)+1.5
        center=(self.left+self.right)*.5
        if p==3:
            faceweight=np.clip((72-fy)/19,0,1)
            center=center*(1-faceweight)+FRONT[d]*faceweight
        self.center=np.clip(center,self.left+(self.right-self.left)*.13,self.right-(self.right-self.left)*.13)
        xx=np.broadcast_to(np.linspace(0,1,SWATCH),(SWATCH,SWATCH))
        v=np.broadcast_to(np.linspace(0,1,SWATCH)[:,None],xx.shape)
        sx,sy=self.xy(xx,v)
        self.swatch=np.stack([nd.map_coordinates(channel,[sy,sx],order=1,mode="nearest") for channel in self.color.transpose(2,0,1)],axis=-1)

    def guides(self,v):
        return tuple(np.interp(v,np.linspace(0,1,SIZE),line) for line in (self.left,self.center,self.right))

    def xy(self,u,v):
        left,center,right=self.guides(v)
        xx=np.where(u<.5,left+(center-left)*(2*u),center+(right-center)*(2*u-1))
        return xx,self.top+v*(self.bottom-self.top)


# silhouette left, a fold/face registration mark, and right for each height;
# the same cloth coordinates span the train, panels and mantle. A split at a
# torn edge therefore cannot drift away from its neighbour during the turn.
HEIGHTS=(18,33,46,59,74,91,113,135,155,170)
CONTOURS={
  "E": ((85,88,94),(82,113,125),(74,110,119),(68,99,113),(65,93,111),(71,90,111),(72,90,115),(65,89,113),(76,92,109),(83,94,102)),
  "SE":((94,96,102),(89,109,117),(85,108,119),(71,105,124),(62,101,135),(77,99,125),(75,99,125),(67,97,125),(77,96,119),(86,96,112)),
  "S": ((90,96,102),(83,96,109),(79,96,113),(66,96,126),(56,96,136),(72,96,120),(70,96,122),(65,96,127),(74,96,118),(83,96,110)),
  "NE":((89,92,98),(83,101,118),(81,101,116),(68,96,119),(61,90,124),(65,87,117),(65,87,115),(61,88,113),(70,91,111),(79,92,103)),
  "N": ((90,96,102),(83,96,109),(79,96,113),(65,96,127),(58,96,134),(71,96,121),(70,96,122),(65,96,127),(74,96,118),(83,96,110)),
}


def body_guides(key):
    from ash_wraith_paint import SOURCE,MIRRORED
    points=np.array(CONTOURS[SOURCE[key]],dtype=float)
    if key in MIRRORED:points=191-points[:,::-1]
    return np.stack([np.interp(Y,HEIGHTS,guide) for guide in points.T])


def sleeve_guides(key,p):
    from ash_wraith_paint import SOURCE,MIRRORED,LANDMARKS
    idx=1-(p-4) if key in MIRRORED else p-4
    hinges,cuffs=LANDMARKS[SOURCE[key]]
    start=np.array(hinges[4+idx],float);end=np.array(cuffs[idx],float)
    if key in MIRRORED:start[0]=191-start[0];end[0]=191-end[0]
    # A facing side cuff is seen on edge. Ease its cross-sectional radius
    # between the broad frontal ellipse and the thin side opening.
    span=8 if key in (0,4) and ((key==0 and p==4) or (key==4 and p==5)) else 12
    return start,end,float(span)


def project_material(src,old,t,comfy,k,p):
    if p < 4:
        orig=body_guides(old)
        target=(1-t)*body_guides(k)+t*body_guides((k+1)%8)
        left,center,right=target
        sx=np.where(X<center,orig[1]+(X-center)*(orig[1]-orig[0])/(center-left),
                    orig[1]+(X-center)*(orig[2]-orig[1])/(right-center))
        sy=Y
    else:
        a_sh,a_tip,a_span=sleeve_guides(k,p)
        b_sh,b_tip,b_span=sleeve_guides((k+1)%8,p)
        origin,tip,radius=sleeve_guides(old,p)
        shoulder=(1-t)*a_sh+t*b_sh
        end=(1-t)*a_tip+t*b_tip
        delta=end-shoulder
        # Work in the local material basis running from shoulder to cuff;
        # its perpendicular transforms the width independently of the length.
        l2=delta@delta
        along=((X-shoulder[0])*delta[0]+(Y-shoulder[1])*delta[1])/l2
        across=(-(X-shoulder[0])*delta[1]+(Y-shoulder[1])*delta[0])/np.sqrt(l2)
        leg=tip-origin
        perp=np.array([-leg[1],leg[0]])/np.linalg.norm(leg)
        across*=radius/((1-t)*a_span+t*b_span)
        sx=origin[0]+along*leg[0]+across*perp[0]
        sy=origin[1]+along*leg[1]+across*perp[1]
    def sample(a,order=1):
        return nd.map_coordinates(a,[sy,sx],order=order,mode="constant",cval=0)
    alpha=np.clip(sample(src.alpha),0,1)
    color=np.stack([sample(c,3) for c in src.color.transpose(2,0,1)],axis=-1)
    # The model supplies the rotating indirect colour only; thread, contour
    # and opacity are transported once from a single, fully painted view.
    v=(sy-src.top)/(src.bottom-src.top)
    left,center,right=src.guides(v)
    u=np.where(sx<center,.5*(sx-left)/(center-left),.5+.5*(sx-center)/(right-center))
    adjustment=nd.gaussian_filter(comfy-src.swatch,sigma=(4,4,0)).clip(-40,40)
    uv=[np.clip(v,0,1)*(SWATCH-1),np.clip(u,0,1)*(SWATCH-1)]
    for ch in range(3):color[...,ch]+=.20*nd.map_coordinates(adjustment[...,ch],uv,order=1,mode="nearest")
    return color.astype(np.float32),alpha,np.stack((sx,sy),axis=-1).astype(np.float32)


def cloth_distance(alpha):
    inside=alpha>.5
    distance=np.where(inside,nd.distance_transform_edt(inside)-.5,
                      .5-nd.distance_transform_edt(~inside))
    return np.where((alpha>0)&(alpha<1),alpha-.5,distance).astype(np.float32)


def correspond(ca,aa,cb,ab,t):
    # A mostly concealed side panel may reveal its full length as it turns.
    # Match the visible hems and edges before tracing the fine surface flow.
    def bounds(alpha):
        ink=alpha>.24
        rows=np.flatnonzero(ink.any(axis=1))
        left=np.argmax(ink[rows],axis=1)-1
        right=SIZE-np.argmax(ink[rows,::-1],axis=1)
        return rows,left,right
    ar,al,ah=bounds(aa);br,bl,bh=bounds(ab)
    top=(1-t)*ar[0]+t*br[0];bottom=(1-t)*ar[-1]+t*br[-1]
    v=(Y-top)/(bottom-top)
    ya=ar[0]+v*(ar[-1]-ar[0]);yb=br[0]+v*(br[-1]-br[0])
    la,ra=np.interp(ya,ar,al),np.interp(ya,ar,ah)
    lb,rb=np.interp(yb,br,bl),np.interp(yb,br,bh)
    left=(1-t)*la+t*lb;right=(1-t)*ra+t*rb
    u=(X-left)/(right-left)
    ma=np.stack((la+u*(ra-la),ya),axis=-1).astype(np.float32)
    mb=np.stack((lb+u*(rb-lb),yb),axis=-1).astype(np.float32)
    return remap(ca,ma),remap(aa,ma),remap(cb,mb),remap(ab,mb),ma,mb


def between(a,b,t,comfy,k,p):
    # First project BOTH paintings onto the actual intermediate shoulder, cuff
    # or mantle. Then follow the small residual motion of the woven pattern.
    # The full quarter turn now carries material from both directions.
    ca,aa,projection_a=project_material(a,k,t,comfy,k,p)
    cb,ab,projection_b=project_material(b,(k+1)%8,t,comfy,k,p)
    ca,aa,cb,ab,shape_a,shape_b=correspond(ca,aa,cb,ab,t)
    def guide(rgb,alpha):
        return np.uint8(np.clip((64+1.3*cv2.cvtColor(rgb,cv2.COLOR_RGB2GRAY))*alpha,0,255))
    left,right=guide(ca,aa),guide(cb,ab)
    ab_flow,ba_flow=optical(left,right),optical(right,left)
    coords=np.stack((X,Y),axis=-1)
    f,r=ab_flow,ba_flow
    for _ in range(3):
        f=remap(ab_flow,coords-.5*f)
        r=remap(ba_flow,coords-.5*r)
    step=.5*(f-r)
    local=remap(step,coords+(.5-t)*step)
    qa,qb=coords-t*local,coords+(1-t)*local
    sa,sb=remap(cloth_distance(aa),qa),remap(cloth_distance(ab),qb)
    alpha=np.clip(.5+(1-t)*sa+t*sb,0,1)
    wa=(1-t)*np.clip(.65+sa,.035,1)
    wb=t*np.clip(.65+sb,.035,1)
    def surface(source,projection,shape,flow,lit):
        # Compose the inverse maps, then read the actual brush stroke only once.
        # Repeated filtering of pigment erases the fine gold thread on the hood.
        at=remap(projection,remap(shape,flow))
        pigment=cv2.remap(source.color,at,None,cv2.INTER_CUBIC,borderMode=cv2.BORDER_REPLICATE)
        low=cv2.GaussianBlur(pigment,(0,0),1.6)
        high=pigment-low
        low+=cv2.GaussianBlur(remap(lit,flow)-pigment,(0,0),3.7)
        contrast=cv2.GaussianBlur(abs(high).mean(axis=-1),(0,0),1.5)
        return low,high,contrast
    la,ha,ea=surface(a,projection_a,shape_a,qa,ca)
    lb,hb,eb=surface(b,projection_b,shape_b,qb,cb)
    low=(wa[...,None]*la+wb[...,None]*lb)/(wa+wb)[...,None]
    # When two brush marks enter the same small area, carry more of the
    # better-defined one. Keep the broad lighting and exposure continuous.
    da,db=wa*(1+ea)**2,wb*(1+eb)**2
    high=(da[...,None]*ha+db[...,None]*hb)/(da+db)[...,None]
    paint=low+high*(1+.34*4*t*(1-t))
    rgba=np.uint8(np.clip(np.rint(np.dstack((paint,255*alpha))),0,255))
    rgba[rgba[...,3]==0,:3]=0
    return Image.fromarray(rgba)

def read_keys(work):
    return [[Cloth(Image.open(work/"keys"/f"{d}-{p}.png").convert("RGBA"),p,d) for p in range(6)] for d in range(8)]


def run_film(work,keys,host):
    frames=[]
    for row in keys+keys[:1]:
        plate=Image.new("RGB",(3*SWATCH,2*SWATCH))
        for p,cloth in enumerate(row):
            swatch=Image.fromarray(np.rint(cloth.swatch).clip(0,255).astype(np.uint8))
            plate.paste(swatch,(p%3*SWATCH,p//3*SWATCH))
        frames.append(plate)
    apng=work/"pigment-cloth.png"
    frames[0].save(apng,save_all=True,append_images=frames[1:],duration=100,loop=0,format="PNG")
    film(work,host,apng,asset_id="ash-wraith")


def material(im):
    # The height describes convex folds rather than a solid cylindrical limb:
    # a cuff is still hollow and the flat folded panel keeps its narrow ribs.
    a=np.asarray(im)
    alpha=a[...,3].astype(np.float32)/255
    color=a[...,:3].astype(np.float32)
    near=nd.distance_transform_edt(alpha<.25,return_distances=False,return_indices=True)
    pigment=color[tuple(near)]
    luma=pigment@np.array([.26,.6,.14])
    broad=nd.gaussian_filter(luma,2.0)
    distance=nd.distance_transform_edt(alpha>.3)
    puff=np.clip(distance/10,0,1)**.68
    relief=(.14+.39*puff+.30*np.clip((luma-15)/100,0,1)+.14*np.clip((luma-broad+13)/25,0,1))*alpha
    coals=np.clip((pigment[...,0]-pigment[...,2]-15)/85,0,1)
    adjustment=(-2-.075*pigment+(broad-luma)[...,None]*.08 + np.array([-8,-2,3])*coals[...,None])*alpha[...,None]
    r=np.empty_like(a)
    r[...,:3]=np.rint(np.clip(128+adjustment,0,255))
    r[...,3]=np.rint(np.clip(relief,0,1)*255)
    return Image.fromarray(r)


def pack(work,keys,out,header):
    cv2.setNumThreads(2)
    keysheet=Image.new("RGBA",(8*SIZE,6*SIZE))
    # Existing corpse meshes address eight columns of 128-pixel material.
    canonical_relief=Image.new("RGBA",(8*128,6*128),(128,128,128,0))
    preview=Image.new("RGBA",(SIZE*8,SIZE*4))
    entries=[]
    for view in range(32):
        d,sub=divmod(view,4)
        guide=Image.open(work/"film"/f"{view:02}.png").convert("RGB")
        parts=[]
        for p,cloth in enumerate(keys[d]):
            x,y=p%3*SWATCH,p//3*SWATCH
            g=np.asarray(guide.crop((x,y,x+SWATCH,y+SWATCH)),dtype=np.float32)
            im=between(cloth,keys[(d+1)%8][p],sub/4,g,d,p) if sub else cloth.im
            relief=material(im)
            if not sub:
                keysheet.paste(im,(d*SIZE,p*SIZE))
                canonical_relief.paste(relief.resize((128,128),Image.Resampling.LANCZOS),(d*128,p*128))
            box=im.getchannel("A").getbbox()
            if not box:raise ValueError(f"Empty cloth at {view} part {p}")
            entries.append((view,p,box,im.crop(box),relief.crop(box)))
            parts.append(im)
        frame=Image.new("RGBA",(SIZE,SIZE))
        for p in ORDER[((view+2)//4)%8]:frame.alpha_composite(parts[p])
        preview.paste(frame,(view%8*SIZE,view//8*SIZE))
    page_width=1536;shelves=[];height=0;positions={}
    for view,p,box,im,rel in sorted(entries,key=lambda e:-e[3].height):
        w,h=im.width+4,im.height+4
        choices=[row for row in shelves if row[1]>=h and row[2]+w<=page_width]
        if choices:shelf=min(choices,key=lambda r:r[1])
        else:shelf=[height,h,0];shelves.append(shelf);height+=h
        positions[view,p]=shelf[2]+2,shelf[0]+2
        shelf[2]+=w
    assert height<=2048
    paint=Image.new("RGBA",(page_width,height))
    folded=Image.new("RGBA",paint.size,(128,128,128,0))
    rows=[[] for _ in range(32)]
    for view,p,box,im,rel in entries:
        x,y=positions[view,p]; paint.paste(im,(x,y));folded.paste(rel,(x,y))
        rows[view].append((0,x,y,im.width,im.height,*box[:2]))
    folder=out/"wraith-turn";folder.mkdir(exist_ok=True)
    paint.save(folder/"parts-0.png",optimize=True)
    folded.save(folder/"material-0.png",optimize=True)
    keysheet.save(out/"ash-wraith.png",optimize=True)
    canonical_relief.save(out/"corpses"/"ash-wraith.png",optimize=True)
    header.write_text('#pragma once\n#include "../volcano_turn_tile.hpp"\nnamespace ash_wraith_turn_atlas {\n'
        'using Tile=volcano_turn_data::Tile;\ninline constexpr int Count=32, Parts=6, Cell=192, PageCount=1;\n'
        'inline constexpr int authoredPart(int,int p){return p;}\ninline constexpr Tile tiles[Count][Parts]={\n'
        +",\n".join(" {"+",".join("{"+",".join(map(str,e))+"}" for e in row)+"}" for row in rows)+"\n};\n}\n")
    preview.save(work/"turn-preview.png")
    from ash_wraith_registration import register
    register(folder,header)
    from ash_wraith_surface import build
    build(folder,header,folder)


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("work",type=Path)
    parser.add_argument("--comfy")
    parser.add_argument("--pack",action="store_true")
    parser.add_argument("--no-audio",required=True,action="store_true")
    args=parser.parse_args();keys=read_keys(args.work)
    if args.comfy:run_film(args.work,keys,args.comfy)
    if args.pack:pack(args.work,keys,Path("assets/volcano"),Path("src/generated/ash_wraith_turn_atlas.hpp"))



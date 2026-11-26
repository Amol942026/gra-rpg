"""Pack the five Blender views tightly, keeping one ground pivot per species."""
import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter

from marsh_actor_pack import footprint


VIEWS=("N","NW","W","SW","S")
SW,SH=240,236
CELS=64


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def painted_sheet(path):
    with Image.open(path) as src:
        assert src.size==(SW*8,SH*8)
        return src.convert("RGBA")


def crop(sheet,i):
    x,y=i%8*SW,i//8*SH
    return sheet.crop((x,y,x+SW,y+SH))


def bounds(im):
    a=np.asarray(im.getchannel("A"))
    yy,xx=np.nonzero(a>32)
    if not yy.size:
        raise ValueError("Empty render cell")
    return int(xx.min()),int(yy.min()),int(xx.max()),int(yy.max()),int(np.nonzero(a>=92)[0].max())


def edge_colors(im):
    """Extend the edge pigment by one texel for straight-alpha bilinear sampling."""
    rgba=np.array(im)
    empty=rgba[:,:,3]==0
    rgba[empty,:3]=0
    # A bilinear tap can reach an adjacent transparent texel, including on a diagonal.
    edges=np.asarray(im.getchannel("A").filter(ImageFilter.MaxFilter(3)))>0
    y,x=np.where(empty&edges)
    alpha=rgba[:,:,3].astype(np.int32)
    weights=np.zeros(len(x),dtype=np.int32)
    colors=np.zeros((len(x),3),dtype=np.int32)
    for dy,dx in ((-1,-1),(-1,0),(-1,1),(0,-1),(0,1),(1,-1),(1,0),(1,1)):
        yy=np.clip(y+dy,0,im.height-1);xx=np.clip(x+dx,0,im.width-1)
        a=alpha[yy,xx]
        colors+=a[:,None]*rgba[yy,xx,:3]
        weights+=a
    rgba[y,x,:3]=(colors+weights[:,None]//2)//weights[:,None]
    return Image.fromarray(rgba)


def frog_eyelids(rgb,phase,view):
    # Eye centers registered on the visible face at frames 8, 11 and 15 of
    # the roll. The outer lid samples the same animal's hide above the eye.
    marks={
        2:[((22.5,47.7),(25.0,55.0),(25.7,55.4))],
        3:[((39,43.4),(39,47.4),(39,47.4))],
        4:[((76.0,41.2),(76,38.0),(76.2,37.7)),
           ((52.4,49.7),(53.0,50.8),(53.5,51.2))]
    }
    if view not in marks or phase<8/15:
        return
    shut=np.clip((phase-8/15)/.30,0,1)
    shut=shut*shut*(3-2*shut)
    yy,xx=np.indices(rgb.shape[:2])
    above=rgb[np.maximum(yy-4,0),xx].copy()
    for points in marks[view]:
        cx,cy=(np.interp(phase*15,[8,11,15],vv) for vv in zip(*points))
        dx=xx-cx
        slope=(-.55,-.44,-.68)[view-2]
        dy=(yy-cy)-slope*dx
        area=np.clip(1.4-((dx/4.0)**2+(dy/1.85)**2),0,1)*shut
        crease=np.exp(-((dy-.8)/.65)**2)
        surface=above*(.87-.37*crease[:,:,None])
        rgb[:]=rgb*(1-area[:,:,None])+surface*area[:,:,None]


def corpse_relief(im,phase,species,view,core):
    """Paint in the thick upper-facing flank and the occlusion under the rolled mass."""
    amount=phase*phase*(3-2*phase)
    rgba=np.array(im)
    mask=rgba[:,:,3]>50
    top=np.argmax(mask,axis=0)
    bottom=len(mask)-1-np.argmax(mask[::-1],axis=0)
    y=np.arange(len(mask))[:,None]
    across=np.clip((y-top)/np.maximum(1,bottom-top),0,1)
    highlight=np.exp(-((across-.28)/.25)**2)
    under=np.maximum(0,(across-.55)/.45)**1.4
    rgb=rgba[:,:,:3].astype(float)
    factor=1+amount*(.11*highlight-.29*under-.055)
    if species==2:
        # Wick and luminous tissue lose light from the center out, leaving
        # the bark rim and reflected top light visible on the extinguished pod.
        yy,xx=np.indices(mask.shape)
        radius=np.exp(-.5*(((xx-core[0])/10.5)**2+((yy-core[1])/7.5)**2))
        factor*=(1-.13*amount)*(1-.76*radius*amount)
    rgb=rgb*factor[:,:,None]+amount*highlight[:,:,None]*np.array([3.2,2.3,1.2])
    if species==0:
        frog_eyelids(rgb,phase,view)
    rgba[:,:,:3]=np.clip(rgb,0,255).astype(np.uint8)
    rgba[rgba[:,:,3]==0]=0
    return Image.fromarray(rgba)


def supports(im,ax,ay,species):
    a=np.asarray(im.getchannel("A"))
    _,xs=np.nonzero(a>=64)
    first,last=int(xs.min()),int(xs.max())+1
    middle=int(first+(last-first)*(.54 if species==3 else .50))
    result=[]
    for lo,hi in [(first,middle),(middle,last)]:
        # Legacy helper measures against (88,159); translate into this species' tighter cell.
        x,y,rad=footprint(a,lo,hi)
        x+=88-ax;y+=159-ay
        if species==2 and y < -9:rad=0
        result.append([x,y,rad])
    assert max(q[1] for q in result if q[2])>=-1.5
    return result


def lantern(im,ax,ay):
    rgba=np.asarray(im,dtype=float)
    r,g,b,a=rgba.transpose(2,0,1)
    yy,xx=np.indices(a.shape)
    # Yellow-green core wins over pale silver edge lights in the rear painting.
    bright=(.30*r+.70*g-.46*b+.20*(g-r))*(a/255)
    py,px=np.unravel_index(bright.argmax(),bright.shape)
    weight=np.maximum(0,bright-bright[py,px]+14)**2
    weight*=(xx-px)**2+(yy-py)**2<=15**2
    cx,cy=np.average(xx,weights=weight),np.average(yy,weights=weight)
    closest=np.where(weight>0,(xx-cx)**2+(yy-cy)**2,np.inf)
    y,x=np.unravel_index(closest.argmin(),a.shape)
    return [x+.5-ax,y+.5-ay]


def cpp_rows(values,dimension):
    if dimension==1:
        return "{"+",".join(f"{float(x):.2f}f" for x in values)+"}"
    sep=",\n" if dimension>=3 else ","
    return "{"+sep.join(cpp_rows(row,dimension-1) for row in values)+"}"


def export(raw,root,report,layout_from=None):
    pack=root/"assets/media/mokradla-ruch"
    pack.mkdir(exist_ok=True,parents=True)
    layout=json.loads(layout_from.read_text(encoding="utf-8"))["cell_dimensions"] if layout_from else None
    geometry=None
    geometry_path=raw/"pose-vertices.npz"
    if geometry_path.is_file():
        with np.load(geometry_path) as archive:
            geometry=archive["xy"]
    shapes=[];all_feet=[];cores=[];files=[];receipts=[];proof=[]
    for s in range(4):
        sheets=[painted_sheet(raw/d/f"render-{s}.png") for d in VIEWS]
        bb=[[bounds(crop(sheet,i)) for i in range(CELS)] for sheet in sheets]
        flat=sum(bb,[])
        if any(left<2 or top<2 or right>=SW-2 or bottom>=SH-2 for left,top,right,bottom,_ in flat):
            raise ValueError(f"Raw Blender frame clipped for species {s}")
        half=max(max(SW//2-left,right-SW//2+1) for left,_,right,_,_ in flat)+3
        above=max(ground-top for _,top,_,_,ground in flat)+4
        below=max(bottom-ground for _,_,_,bottom,ground in flat)+4
        width=int(np.ceil(2*half/4))*4;height=int(np.ceil((above+below)/4))*4
        ax,ay=width//2,above
        assert max(width,height)<=254
        rest=bb[3][0][4]-bb[3][0][1]+1
        # Preserve physical scale and the anchor used by the art-directed eyelids on a rebake.
        # The per-cel padding checks below still catch any clipped artwork.
        if layout is not None:
            width,height,ax,ay,rest=layout[s]
        shapes.append((width,height,ax,ay,rest))
        feet=[];speciesproof=[]
        for view,sheet in enumerate(sheets):
            atlas=Image.new("RGBA",(8*width,8*height))
            view_feet=[];view_cores=[];margin=4096;cells=[]
            for i,(_,_,_,_,ground) in enumerate(bb[view]):
                offx=SW//2-ax;offy=ground-ay
                if geometry is not None:
                    geometry[s,view,i]-=(offx,offy)
                cel=crop(sheet,i).crop((offx,offy,offx+width,offy+height))
                core=lantern(cel,0,0) if s==2 else None
                if i>=48:
                    cel=corpse_relief(cel,(i-48)/15,s,view,core)
                cel=edge_colors(cel)
                left,top,right,bottom,_=bounds(cel)
                margin=min(margin,left,top,width-right-1,height-bottom-1)
                view_feet.append(supports(cel,ax,ay,s))
                if s==2:view_cores.append([core[0]-ax,core[1]-ay])
                atlas.paste(cel,(i%8*width,i//8*height))
                if i in (0,63):
                    cells.append({"height":bottom-top+1,"width":right-left+1,
                                  "solid":int((np.asarray(cel.getchannel("A"))>=92).sum())})
            assert margin>=2,(s,view,margin)
            feet.append(np.array(view_feet).reshape((4,16,2,3)))
            if s==2:cores.append(np.array(view_cores).reshape((4,16,2)))
            path=pack/f"turn-{s}-{view}.png"
            atlas.save(path,compress_level=6)
            with Image.open(path) as verify:
                assert atlas.tobytes()==verify.tobytes()
            files.append({"file":path.name,"sha256":sha(path),"bytes":path.stat().st_size})
            receipts.append({"species":s,"view":view,"margin":margin,"rest_and_corpse":cells})
            speciesproof.append((atlas.copy(),cells))
        all_feet.append(feet)
        proof.append(speciesproof)
    header=["#pragma once",'#include "marsh_actor_frames.hpp"',
            "// Generated from the Blender directional sheets by marsh_turn_pack.py.",
            "namespace marsh_turn_frames {"]
    for k,name in enumerate(("CellW","CellH","AnchorX","AnchorY","RestHeight")):
        header.append("inline constexpr int "+name+"[4]={"+",".join(str(v[k]) for v in shapes)+"};")
    header.append("inline constexpr marsh_actor_frames::Foot Supports[4][5][4][16][2]="+cpp_rows(all_feet,6)+";")
    header.append("inline constexpr marsh_actor_frames::Core LightCore[5][4][16]="+cpp_rows(cores,4)+";")
    header.extend(["}",""])
    (root/"src/generated/marsh_turn_frames.hpp").write_text("\n".join(header),encoding="utf-8")
    render=json.loads((raw/"render.json").read_text(encoding="utf-8"))
    paintings=[root/"assets/media/mokradla/atlas.png",*sorted((root/"art/creatures/marsh-turn").glob("*.png"))]
    record=dict(render=render,authorship="Original Emberfall painted swamp creatures, directional paintings and local Blender shape-key animation",
                source_files=[{"file":p.relative_to(root).as_posix(),"sha256":sha(p)} for p in paintings],
                cell_dimensions=shapes,views=VIEWS,final_frames=1280,files=files,tests=receipts,
                texture_bytes=sum(v[0]*v[1]*4*64*5 for v in shapes))
    (pack/"turn-import-receipt.json").write_text(json.dumps(record,ensure_ascii=False,indent=2),encoding="utf-8")
    report.mkdir(parents=True,exist_ok=True)
    if geometry is not None:
        np.savez_compressed(report/"packed-geometry.npz",xy=geometry)
    (report/"turn-export.json").write_text(json.dumps(record,ensure_ascii=False,indent=2),encoding="utf-8")
    for action,index in (("turn",0),("corpses",63)):
        board=Image.new("RGB",(5*204,4*180),(63,68,51))
        for s,pages in enumerate(proof):
            w,h,ax,ay,_=shapes[s]
            for view,(atlas,_) in enumerate(pages):
                x=index%8*w;y=index//8*h
                cell=atlas.crop((x,y,x+w,y+h))
                board.paste(cell,(view*204+102-ax,s*180+165-ay),cell)
        board.save(report/(action+"-sheet.png"))
    print("PASS",shapes,record["texture_bytes"])


if __name__=="__main__":
    parser=argparse.ArgumentParser()
    for option in ("source","root","report"):
        parser.add_argument("--"+option,type=Path,required=True)
    parser.add_argument("--layout-from",type=Path,help="Previous import receipt to retain the same cell geometry")
    args=parser.parse_args();export(args.source,args.root,args.report,args.layout_from)

"""Retouch warm visor reflections on the editable 32-view armature in Krita."""
import argparse
from pathlib import Path
from zipfile import ZipFile

import numpy as np
from PIL import Image, ImageFilter

from krita_mcp_client import KritaMCP,start_krita_if_needed
from obsidian_sentinel_paint import PARTS
from obsidian_sentinel_turn import ORDER
from wanderer_paints import write_ora

SIZE=(1280,640)
DEPTH=sorted({(rank,p) for order in ORDER for rank,p in enumerate(order)})
LAYERS={entry:f"{entry[0]+1} · {PARTS[entry[1]]}" for entry in DEPTH}
# Outside corners and nose bridge, traced on the 160-pixel plate. The
# profile shows only the forward end of the slit. Back plates have no eyes.
VISOR=np.array([((94.1,49.8),(97.1,51.8),(98.4,52.0)),((85.7,51.6),(91.4,54.8),(94,54)),
                ((78.3,50.7),(83.1,53.3),(87.1,50.5)),((79,53.9),(82.4,52.9),(87.3,50.2)),
                ((60.6,51.7),(61.9,51.3),(65.6,48.8))])


def finish_plate(tile,view):
    """Recover the narrow slate rim lights on material around the neck."""
    a=np.asarray(tile).copy()
    rgb=a[:,:,:3].astype(float)
    blur=np.asarray(tile.convert("RGB").filter(ImageFilter.GaussianBlur(1.1)),dtype=float)
    y,x=np.indices(a.shape[:2])
    key,sub=divmod(view,4)
    necks=np.array(((90,56),(85,62),(81,59),(80,60),(70,58),(71,54),(80,57),(83,53),(90,56)))
    cx,cy=necks[key]+sub/4*(necks[key+1]-necks[key])
    area=np.exp(-((x-cx)/16)**2-((y-cy)/9)**2)
    edge=np.clip((rgb-blur).mean(axis=2)/23,0,1)
    # Keep the lower values of the glass and lift only lit bevels.
    a[:,:,:3]=np.uint8(np.clip(rgb+(rgb-blur)*(.48 if sub else .18)+
                              (area*edge)[:,:,None]*np.array((36,37,41)),0,255))
    return Image.fromarray(a)


def slit(view):
    if view<=16:
        k,s=divmod(view,4)
        return (VISOR[k]+(VISOR[min(k+1,4)]-VISOR[k])*s/4),1.
    if view in (17,31):
        return (VISOR[4] if view==17 else VISOR[0]),.48
    return None


def produce(cells,root):
    root.mkdir(parents=True,exist_ok=False)
    sheets={entry:Image.new("RGBA",SIZE) for entry in DEPTH}
    visor=[]
    locations={}
    for view in range(32):
        pos=(view%8*160,view//8*160)
        for rank,p in enumerate(ORDER[((view+2)//4)%8]):
            tile=Image.open(cells/f"{view:02}-{p}.png").convert("RGBA")
            if tile.size!=(160,160) or not tile.getbbox():raise ValueError((view,p))
            if p==3:tile=finish_plate(tile,view)
            entry=(rank,p);sheets[entry].paste(tile,pos)
            locations[view,p]=entry,pos
            if p==3 and slit(view):
                points,strength=slit(view)
                # Sample a nearby opaque pixel if the turn foreshortens the helmet.
                pixels=np.asarray(tile)
                for dx in (0,1,-1,2,-2):
                    cx,cy=np.rint(points[1]+(dx,0)).astype(int)
                    if pixels[cy,cx,3]>220:break
                path=points+(pos[0]+dx,pos[1])
                if pixels[cy,cx,3]>220:visor.append((entry,path,strength))
    ora=root/"wojownik.ora"
    write_ora(ora,[(LAYERS[k],sheets[k]) for k in reversed(DEPTH)],"Obsydianowy wojownik")
    start_krita_if_needed()
    with KritaMCP(root/"krita") as client:
        doc=client.call("krita_document",action="open",path=str(ora))["affected_document_id"]
        state=client.call("krita_state",document_id=doc)
        ids={i["name"]:i["id"] for i in state["layers"]}
        if state["runtime_compatibility"]["status"]!="compatible" or len(ids)!=len(DEPTH):raise ValueError(state)
        for key,path,strength in visor:
            left,nose,right=path
            def stroke(poly,color):
                client.call("krita_draw",shape="polygon",points=[{"x":float(x),"y":float(y)} for x,y in poly],
                            document_id=doc,layer_id=ids[LAYERS[key]],foreground=color,
                            fill_style="ForegroundColor",stroke_style="None")
            stroke([left-(0,.55*strength),nose-(0,.45*strength),right-(0,.55*strength),
                    right+(0,.38*strength),nose+(0,1.25*strength),left+(0,.4*strength)],"#f29b39")
            stroke([nose-(.75,.20),nose+(.8,-.30),nose+(.4,1.05*strength)],"#ffe092")
        client.call("krita_save_export",document_id=doc,kra_path=str(root/"wojownik.kra"),
                    png_path=str(root/"wojownik.png"),overwrite=False)
        client.call("krita_preview",document_id=doc,max_width=800,max_height=400)
        # Only the visor strata change. Retain exact source pixels of the
        # other parts, avoiding a second color-management round trip.
        for index,key in enumerate(sorted({row[0] for row in visor})):
            path=root/f"visor-{index}.png"
            client.call("krita_asset",action="export_layer",document_id=doc,layer_id=ids[LAYERS[key]],
                        output_path=str(path),crop="canvas",overwrite=False,preview_max_width=160,preview_max_height=80)
            sheet=Image.open(path).convert("RGBA")
            if sheet.size!=SIZE or not sheet.getbbox():raise ValueError("Missing visor layer")
            sheets[key]=sheet
    with ZipFile(root/"wojownik.kra") as z:
        assert z.testzip() is None and "maindoc.xml" in z.namelist()
    out=root/"cells";out.mkdir()
    for (view,p),(key,(x,y)) in locations.items():
        sheets[key].crop((x,y,x+160,y+160)).save(out/f"{view:02}-{p}.png")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("cells",type=Path)
    parser.add_argument("folder",type=Path);parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args();produce(args.cells.resolve(),args.folder.resolve())

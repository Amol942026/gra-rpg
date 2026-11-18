"""Prepare the six cloth cutouts and their stable registration for the Ash Wraith."""
import argparse
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
from scipy import ndimage as nd
from krita_mcp_client import KritaMCP, start_krita_if_needed
from wanderer_paints import write_ora

SIZE = 192
DIRECTIONS = ("E", "SE", "S", "SW", "W", "NW", "N", "NE")
MODELS = {"E": (0, 0), "SE": (1, 0), "S": (2, 0), "NE": (0, 1), "N": (1, 1)}
SOURCE = ("E", "SE", "S", "SE", "E", "NE", "N", "NE")
NAMES = ("Tren", "Lewa poła", "Prawa poła", "Kaptur i peleryna", "Lewy rękaw", "Prawy rękaw")
MIRRORED = {3, 4, 5}
SWAP = (0, 2, 1, 3, 5, 4)
# Curves separate the openings of the sleeves from the bodice. Coordinates in
# the traced drawing are deliberately kept at the same scale as the atlas.
SLEEVES = {
    "E": ((), ((124,70),(103,65),(93,66),(86,74),(84,87),(83,101),(88,112),(97,126),(104,131),(132,115),(137,90))),
    "SE": (((125,68),(119,72),(119,82),(121,93),(124,108),(128,116),(133,121),(160,110),(163,70)),
           ((72,71),(84,70),(85,77),(82,86),(77,95),(74,105),(66,116),(59,122),(42,122),(37,93),(56,75))),
    "S": (((123,69),(114,73),(114,80),(119,87),(123,100),(126,111),(132,120),(157,124),(165,80)),
          ((68,69),(78,73),(78,80),(73,87),(69,100),(66,111),(60,120),(35,124),(27,80))),
    "NE": (((65,72),(73,76),(74,83),(70,96),(67,106),(63,113),(56,118),(41,118),(39,79)),
           ((113,68),(103,71),(99,82),(100,89),(108,102),(115,111),(123,121),(145,122),(157,74))),
    "N": (((71,68),(79,75),(76,83),(68,92),(64,103),(61,114),(54,123),(28,123),(30,82)),
          ((121,68),(113,75),(116,83),(124,92),(128,103),(131,114),(138,123),(164,123),(162,82))),
}
# The point of each mantle overlaps the waist. This traced seam avoids a
# horizontal cut across the chevrons when the lower cloth folds.
MANTLE = {
    "E": ((62,72),(75,92),(88,78),(100,68),(112,72),(128,83)),
    "SE": ((42,73),(66,73),(78,68),(95,76),(104,69),(105,102),(110,112),(117,103),(118,70),(128,73),(140,65),(153,71)),
    "S": ((32,68),(58,70),(77,76),(89,71),(89,103),(96,113),(103,103),(103,71),(115,76),(135,70),(158,68)),
    "NE": ((42,72),(58,76),(75,80),(81,89),(89,74),(101,77),(111,69),(123,68),(146,74)),
    "N": ((30,69),(51,68),(66,79),(77,75),(96,98),(115,75),(126,79),(141,68),(162,69)),
}
# waist at the back and the two front panels, collar, two shoulder hinges;
# and the centers of the two cuff openings (anatomical left, right).
LANDMARKS = {
    "E":  (((89,84),(92,84),(87,83),(94,62),(103,72),(99,71)), ((114,92),(116,99))),
    "SE": (((94,86),(103,85),(89,85),(105,62),(125,73),(76,73)), ((138,98),(55,103))),
    "S":  (((96,88),(106,85),(86,85),(96,62),(124,73),(68,73)), ((144,99),(48,99))),
    "NE": (((92,87),(86,85),(101,86),(96,60),(67,75),(110,73)), ((58,99),(131,99))),
    "N":  (((96,88),(86,85),(106,85),(96,61),(69,74),(123,74)), ((48,100),(144,100))),
}
ORDER = ((4,0,1,2,5,3),(0,1,4,2,5,3),(0,1,2,4,5,3),(0,2,5,1,4,3),
         (5,0,2,1,4,3),(0,2,5,1,4,3),(0,1,2,4,5,3),(0,1,4,2,5,3))


def write_rig(path):
    pivots, palms = [], []
    for k, source in enumerate(SOURCE):
        hinges, cuffs = LANDMARKS[source]
        if k in MIRRORED:
            hinges = [(191-hinges[i][0],hinges[i][1]) for i in SWAP]
            cuffs = [(191-x,y) for x,y in reversed(cuffs)]
        pivots.append(hinges)
        palms.append(cuffs)
    def cpp(rows):
        return "{\n" + ",\n".join(" {"+",".join("{"+",".join(f"{v*2/3:.3f}f" for v in xy)+"}" for xy in row)+"}" for row in rows) + "\n}"
    path.write_text("#pragma once\nnamespace ash_wraith_rig {\n"
        + "inline constexpr float pivots[8][6][2]="+cpp(pivots)+";\n"
        + "inline constexpr float palms[8][2][2]="+cpp(palms)+";\n"
        + "inline constexpr int order[8][6]={\n"
        + ",\n".join(" {"+",".join(map(str,row))+"}" for row in ORDER)+"\n};\n}\n")


def polygon(points):
    mask=Image.new("L",(SIZE,SIZE))
    if points:
        ImageDraw.Draw(mask).polygon(points,fill=255)
    return np.asarray(mask)>0


def isolate(master,col,row):
    w,h=master.width//3,master.height//2
    im=master.crop((col*w,row*h,(col+1)*w,(row+1)*h))
    a=np.asarray(im).copy()
    labels,_=nd.label(a[...,3]>42)
    mass=np.bincount(labels.ravel());mass[0]=0
    a[~nd.binary_dilation(labels==mass.argmax(),iterations=3)]=0
    a[...,3]=np.rint(np.clip((a[...,3].astype(float)-16)/235,0,1)*255)
    im=Image.fromarray(a);im=im.crop(im.getbbox())
    im=im.resize((round(im.width/im.height*152),152),Image.Resampling.LANCZOS)
    original_alpha=im.getchannel("A")
    # Crisp embroidery at native game size, preserving thin openwork at the hem.
    im=im.convert("RGB").filter(ImageFilter.UnsharpMask(1.05,82,3)).convert("RGBA")
    im.putalpha(original_alpha)
    cell=Image.new("RGBA",(SIZE,SIZE));cell.paste(im,((SIZE-im.width)//2,18))
    return cell


def cut(key,d,lining):
    rgba=np.asarray(key)
    visible=rgba[...,3]>7
    h,w=np.indices((SIZE,SIZE))
    left,right=[polygon(points)&visible for points in SLEEVES[d]]
    curve=np.asarray(MANTLE[d]); seam=np.interp(w,curve[:,0],curve[:,1])
    # Assign the thin outlines of both sleeve openings to the sleeve itself.
    # At the diagonal these dark flaps have almost the same value as the robe.
    body_edges={"E":(69,110),"SE":(75,120),"S":(74,118),"NE":(71,114),"N":(72,120)}[d]
    shoulder=(h>seam)&(h<122)
    outer=shoulder & visible & ((w<body_edges[0])|(w>body_edges[1]))
    dl=nd.distance_transform_edt(~left) if left.any() else np.full(left.shape,999.)
    dr=nd.distance_transform_edt(~right)
    left|=outer&(dl<dr)&(dl<=13)
    right|=outer&(dr<=dl)&(dr<=13)
    cloth=visible & ~(left|right)
    upper=cloth&(h<=seam)
    low=cloth&~upper
    center={"S":96,"SE":106,"E":94,"NE":88,"N":96}[d]
    half=7+np.clip((h-75)*.09,0,10)
    # At the back anatomical left appears screen-left.
    a,b=(w>center+half,w<center-half) if d in ("E","SE","S") else (w<center-half,w>center+half)
    masks=[low&~(a|b),low&a,low&b,upper,left,right]
    # Keep the low cuff tips with their sleeve while it swings. A traced
    # polygon can otherwise clip a tiny disconnected piece into the skirt.
    def connected(mask):
        labels,count=nd.label(mask,structure=np.ones((3,3)))
        if count<2:return mask
        mass=np.bincount(labels.ravel());mass[0]=0
        return labels==mass.argmax()
    masks=[connected(mask) for mask in masks]
    missing=visible&~np.logical_or.reduce(masks)
    distance=[nd.distance_transform_edt(~m) if m.any() else np.full(m.shape,999.) for m in masks]
    owner=np.argmin(distance,axis=0)
    masks=[m|(missing&(owner==p)) for p,m in enumerate(masks)]
    layers=[]
    for p,mask in enumerate(masks):
        if not mask.any():
            # The far cuff lies completely behind the side view silhouette.
            # Its painted lining appears before the hidden sleeve swings out.
            assert p==4 and d=="E"
            near_pixels=rgba.copy();near_pixels[~right]=0
            far=Image.fromarray(near_pixels).resize((round(SIZE*.74),round(SIZE*.81)),Image.Resampling.LANCZOS)
            tile=Image.new("RGBA",(SIZE,SIZE));tile.paste(far,(25,12))
            ar=np.asarray(tile).copy(); ar[...,3]*=visible;ar[...,:3]=np.uint8(ar[...,:3]*.77)
            layers.append(Image.fromarray(ar));continue
        # Preserve the painted outer contour. Extend the covered fabric inwards
        # at each joint so that motion exposes dyed fabric instead of a slit.
        near=nd.distance_transform_edt(~mask,return_distances=False,return_indices=True)
        depth=ORDER[DIRECTIONS.index(d)]
        higher=np.zeros(mask.shape,dtype=bool)
        for front in depth[depth.index(p)+1:]:
            higher |= masks[front]
        inside=mask|(nd.binary_dilation(mask,iterations=6)&higher)
        # Reflect brush strokes under the adjoining panel before continuing
        # them with a real painted swatch of the inner robe.
        reflected=(np.clip(2*near[0]-h,0,SIZE-1),np.clip(2*near[1]-w,0,SIZE-1))
        core=mask[reflected]
        pix=rgba[tuple(near)].copy()
        pix[core]=rgba[reflected][core]
        projected=LANDMARKS[d][0][0][0]
        radius=np.interp(h,(70,80,101,128,149,160),(9,13,17,19,11,0))
        inner=(h>=71)&(h<160)&(abs(w-projected)<radius)&visible
        if p==0:
            inside|=inner
        exposed=inside&~mask&~core
        sample_y=np.rint(np.interp(h,(65,162),(113,147))).astype(int)
        sample_x=np.rint(94+(w-projected)*.57+2*np.sin(h/13)).clip(75,115).astype(int)
        pigment=lining[sample_y,sample_x,:3].astype(float)
        pix[exposed,:3]=np.clip(pigment[exposed]*.86,0,255).astype(np.uint8)
        inside=connected(inside)
        pix[...,3]=np.where(inside,rgba[...,3],0)
        pix[~inside,:3]=0
        if p<=2:
            # Only the very lowest frayed fibres transmit a little background.
            opacity=np.clip(1-(h-139)*.004, .86,1)
            pix[...,3]=np.rint(pix[...,3]*opacity)
        layers.append(Image.fromarray(pix))
    return layers


def prep(ref,root):
    src=Image.open(ref).convert("RGBA")
    lining=np.asarray(isolate(src,*MODELS["S"]))
    cuts={d:cut(isolate(src,*pos),d,lining) for d,pos in MODELS.items()}
    cells=root/"keys";cells.mkdir(parents=True,exist_ok=True)
    for k,d in enumerate(DIRECTIONS):
        pieces=cuts[SOURCE[k]]
        if k in MIRRORED:
            pieces=[pieces[i].transpose(Image.Transpose.FLIP_LEFT_RIGHT) for i in SWAP]
        for p,im in enumerate(pieces): im.save(cells/f"{k}-{p}.png")
    write_rig(Path("src/generated/ash_wraith_rig.hpp"))


def krita(root):
    keys=root/"keys"
    layers=[]
    for p,name in enumerate(NAMES):
        strip=Image.new("RGBA",(SIZE*8,SIZE))
        for k in range(8):
            strip.paste(Image.open(keys/f"{k}-{p}.png"),(SIZE*k,0))
        layers.append((name,strip))
    ora=root/"plaszcz.ora"
    write_ora(ora,list(reversed(layers)), "Popielny Upiór — haft i podszewka")
    start_krita_if_needed()
    with KritaMCP(root/"krita-log") as client:
        doc=client.call("krita_document",action="open",path=str(ora))["affected_document_id"]
        state=client.call("krita_state",document_id=doc)
        ids={row["name"]:row["id"] for row in state["layers"]}
        if state["runtime_compatibility"]["status"]!="compatible" or set(ids)!=set(NAMES):
            raise RuntimeError("Nie udało się odczytać pełnego projektu szaty w Kricie")
        # Against the black face and slate mantle the stamped brooch should
        # have one legible copper glint, anchored to its own facet.
        for k in (1,2,3):
            im=np.asarray(layers[3][1].crop((SIZE*k,0,SIZE*(k+1),SIZE)))
            yy,xx=np.indices(im.shape[:2])
            cx={1:105,2:96,3:86}[k]
            crop=(abs(xx-cx)<8)&(abs(yy-61)<6)&(im[...,3]>180)
            score=im[...,0].astype(float)-im[...,2]+im[...,0]*.19
            yy,xx=np.unravel_index(np.where(crop,score,-1000).argmax(),crop.shape)
            x,y=SIZE*k+int(xx),int(yy)
            client.call("krita_draw",document_id=doc,layer_id=ids[NAMES[3]],shape="polygon",
                points=[{"x":x-1.20,"y":y+.25},{"x":x+.35,"y":y-.55},{"x":x+.70,"y":y+.62},{"x":x-.2,"y":y+1.05}],
                foreground="#d39d64",fill_style="ForegroundColor",stroke_style="None")
        client.call("krita_save_export",document_id=doc,kra_path=str(root/"plaszcz.kra"),png_path=str(root/"plaszcz.png"),overwrite=False)
        client.call("krita_preview",document_id=doc,max_width=900,max_height=130)
        output=root/"kaptur.png"
        client.call("krita_asset",action="export_layer",document_id=doc,layer_id=ids[NAMES[3]],output_path=str(output),crop="canvas",overwrite=False)
        out=Image.open(output).convert("RGBA")
        if out.size!=(SIZE*8,SIZE):raise ValueError(out.size)
        for k in range(8):out.crop((SIZE*k,0,SIZE*(k+1),SIZE)).save(keys/f"{k}-3.png")


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root",type=Path)
    parser.add_argument("--source",type=Path)
    parser.add_argument("--krita",action="store_true")
    parser.add_argument("--no-audio",required=True,action="store_true")
    args=parser.parse_args()
    if args.source: prep(args.source,args.root)
    if args.krita: krita(args.root)






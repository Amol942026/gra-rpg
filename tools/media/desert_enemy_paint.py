"""Assemble the painted jackal and reaper, with editable OpenRaster layers and directional atlases."""

import argparse
import io
from pathlib import Path
import shutil
import zipfile
from xml.etree import ElementTree as ET

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageOps
from scipy.ndimage import binary_dilation, distance_transform_edt, label

from ember_drake_turn import Paint, inbetween
from slag_crawler_paint import atlas


CELL = 160
# Head, left leg, right leg, body, left arm/foreleg, right arm/foreleg.
JACKAL = (
    (((88,73),(89,32),(134,32),(134,68),(109,78)), (101,67),
     ((52,87),(70,91),(68,130),(48,129),(49,115)), (56,91),
     ((44,87),(63,87),(56,110),(55,131),(41,132),(39,113)), (53,91),
     ((93,88),(104,89),(115,130),(96,130),(93,115)), (98,90),
     ((88,87),(100,85),(102,130),(92,130),(89,105)), (96,87)),
    (((88,70),(92,35),(121,35),(120,75),(100,81)), (98,71),
     ((66,84),(81,84),(83,119),(71,119),(65,104)), (72,88),
     ((55,81),(73,82),(67,101),(66,124),(52,123),(52,106)), (62,84),
     ((92,88),(113,87),(113,132),(97,132),(96,112)), (103,89),
     ((86,87),(97,87),(103,134),(89,134),(87,105)), (93,89)),
    (((60,34),(98,34),(98,72),(87,83),(71,83),(61,70)), (80,71),
     ((63,94),(73,94),(74,137),(63,137)), (69,102),
     ((89,94),(98,94),(99,137),(88,137)), (91,102),
     ((68,92),(79,94),(81,146),(69,146)), (74,96),
     ((81,94),(91,92),(92,144),(80,146)), (86,96)),
    (((83,67),(92,33),(125,32),(126,70),(113,82),(98,78)), (104,69),
     ((63,99),(76,96),(68,122),(67,140),(54,143),(54,119)), (65,101),
     ((67,94),(82,94),(75,117),(81,147),(67,147),(65,124)), (72,98),
     ((91,93),(100,94),(101,133),(89,133)), (96,96),
     ((100,92),(114,91),(111,135),(99,135)), (106,95)),
)
REAPER = (
    (((63,16),(91,16),(91,40),(84,45),(69,39)), (77,37),
     ((71,84),(91,84),(97,132),(74,132),(72,108)), (77,84),
     ((67,83),(78,82),(82,105),(85,132),(65,131)), (73,84),
     ((79,45),(90,47),(90,53),(117,69),(115,77),(100,76),(87,66),(78,62)), (83,51),
     ((70,44),(82,43),(84,72),(87,78),(95,95),(95,109),(82,107),(77,87),(68,80),(66,56)), (75,47)),
    (((69,16),(96,16),(96,37),(91,46),(78,43)), (84,40),
     ((88,87),(98,84),(112,126),(91,128)), (91,85),
     ((69,85),(81,87),(76,109),(76,133),(61,133),(59,116)), (74,87),
     ((88,46),(96,43),(105,56),(115,74),(107,80),(98,67),(88,56)), (93,47),
     ((69,40),(81,43),(74,57),(70,73),(64,81),(65,107),(48,109),(48,80),(55,68),(64,49)), (73,47)),
    (((70,16),(94,16),(94,37),(85,47),(75,44),(70,35)), (83,42),
     ((57,84),(76,87),(72,114),(75,146),(55,146),(57,112)), (70,86),
     ((88,87),(101,86),(111,147),(91,147),(91,120)), (93,86),
     ((68,42),(75,46),(68,56),(65,71),(58,83),(53,82),(55,69),(61,55)), (66,45),
     ((92,44),(97,42),(110,55),(119,78),(111,94),(102,82),(104,76),(96,62)), (98,47)),
    (((59,16),(86,16),(88,36),(81,44),(64,42)), (73,38),
     ((58,83),(73,88),(65,116),(64,139),(48,138),(49,118)), (64,87),
     ((79,85),(93,83),(94,110),(104,147),(80,147),(78,113)), (87,87),
     ((56,43),(67,42),(67,55),(59,72),(50,82),(47,77),(49,62)), (60,48),
     ((78,43),(88,40),(98,58),(101,77),(120,83),(122,111),(107,111),(105,94),(93,89),(85,72),(78,60)), (85,47)),
)
# In the frontal view the sickle lies to screen left and needs to follow the hand.
REAPER_FRONT_RIGHT = (((68,42),(75,46),(68,56),(65,71),(58,83),(52,84),(53,75),(59,62),(61,55)), (66,45))
# Remove all painted steel and the exposed handle from the reference before
# splitting the body. The engine attaches one rigid sickle to the clean palm.
REAPER_OLD_SICKLE = (
    ((81,81),(96,81),(96,105),(81,105),(82,96),(83,90),(84,86)),
    ((55,77),(62,78),(62,84),(58,88),(56,94),(57,100),(63,107),(52,110),
     (46,100),(46,91),(49,85)),
    ((52,81),(59,81),(59,87),(53,91),(51,99),(55,108),(60,114),(50,115),
     (43,109),(41,100),(44,90)),
    ((95,79),(121,79),(121,112),(98,112),(99,99),(95,90),(92,85)),
)
REAPER_PROFILE_HANDLE=((65,74),(70,74),(77,77),(84,79),(84,83),(77,84),(70,80),(65,79))
# The trunk remains complete when a shoulder rolls away from its neutral
# position. These areas follow the cuirass and coat, inside the painted outline.
REAPER_TRUNK=(
    ((72,38),(83,38),(86,58),(84,92),(78,108),(66,115),(67,86),(69,64)),
    ((76,38),(90,38),(95,57),(94,82),(92,110),(83,119),(74,111),(73,93),(70,73)),
    ((72,42),(92,42),(96,63),(96,98),(89,123),(78,123),(75,106),(71,92),(73,62)),
    ((63,40),(84,41),(90,61),(87,91),(85,121),(71,125),(64,103),(61,83)),
)
JACKAL_NECK=(
    ((91,67),(107,65),(110,74),(104,82),(95,87),(82,82),(87,73)),
    ((90,67),(110,68),(111,81),(105,92),(93,94),(87,84)),
    ((69,72),(89,70),(94,79),(91,96),(72,96),(65,83)),
    ((99,66),(116,65),(116,79),(108,88),(94,89),(87,80)),
)
# An elbow and a palm for each anatomical arm, in the same coordinates as the masks.
REAPER_JOINTS = (
    ((85,57),(94,65),(74,62),(72,75)),
    ((98,58),(105,70),(63,63),(58,76)),
    ((102,62),(111,77),(60,66),(54,81)),
    ((55,63),(49,75),(91,60),(96,78)),
)
ORDERS = {
    "jackal": ((1,4,3,2,5,0),(1,4,3,2,5,0),(1,2,3,4,5,0),(2,5,3,1,4,0),
               (2,5,3,1,4,0),(5,4,0,2,3,1),(4,5,0,3,1,2),(4,5,0,1,3,2)),
    "reaper": ((1,4,2,3,0,5),(2,4,1,3,0,5),(1,2,3,0,4,5),(1,5,2,3,0,4),
               (2,5,1,3,0,4),(4,1,3,2,0,5),(4,1,2,3,0,5),(4,1,3,2,0,5)),
}


def remove_specks(im, limit):
    pixels = np.array(im)
    labels, _ = label(pixels[..., 3] > 55)
    area = np.bincount(labels.ravel())
    area[0] = limit
    pixels[area[labels] < limit] = 0
    return Image.fromarray(pixels)


def one_surface(im):
    """Trim disconnected paint while keeping two pixels of antialiasing."""
    pixels=np.array(im)
    labels,count=label(pixels[...,3]>=85,structure=np.ones((3,3),dtype=bool))
    if count<=1:
        return im
    area=np.bincount(labels.ravel())
    area[0]=0
    keep=binary_dilation(labels==np.argmax(area),iterations=2)
    pixels[~keep]=0
    return Image.fromarray(pixels)


def feather_polygons(size,*polygons):
    solid=Image.new("L",size)
    draw=ImageDraw.Draw(solid)
    for polygon in polygons:
        draw.polygon(polygon,fill=255)
    return np.asarray(solid.filter(ImageFilter.GaussianBlur(.7)),dtype=np.float32)/255


def key_panels(path, who):
    im = Image.open(path).convert("RGBA")
    w, h = im.size
    for x, y in ((0, 0), (w//2, 0), (0, h//2), (w//2, h//2)):
        panel = im.crop((x, y, x+w//2, y+h//2))
        panel = panel.crop(panel.getchannel("A").point(lambda a: 255 if a > 90 else 0).getbbox())
        factor = min((106 if who == "jackal" else 125)/panel.height, 143/panel.width)
        panel = panel.resize(tuple(round(i*factor) for i in panel.size), Image.Resampling.LANCZOS)
        frame = Image.new("RGBA", (CELL, CELL))
        frame.paste(panel, (80-int(panel.width*.51), 144-panel.height))
        yield remove_specks(frame, 7)


def split(frame, who, view):
    annotations = (JACKAL if who == "jackal" else REAPER)[view]
    shape = list(zip(annotations[::2], annotations[1::2]))
    if who == "reaper":
        if view == 2:
            shape[-2:] = [shape[-1], REAPER_FRONT_RIGHT]
        mask = Image.new("L",frame.size,255)
        ImageDraw.Draw(mask).polygon(REAPER_OLD_SICKLE[view],fill=0)
        clean = np.minimum(np.asarray(frame.getchannel("A")),np.asarray(mask.filter(ImageFilter.GaussianBlur(.36))))
        frame = frame.copy()
        frame.putalpha(Image.fromarray(clean))
        if view==0:
            # The old bronze haft covered part of the bodice in profile.
            # Reconstruct those pixels from the adjacent fabric.
            area=Image.new("L",frame.size)
            ImageDraw.Draw(area).polygon(REAPER_PROFILE_HANDLE,fill=255)
            pixels=np.array(frame)
            missing=(np.asarray(area)>0)&(pixels[...,3]>32)
            donor=~(missing|(pixels[...,3]<190))
            yy,xx=distance_transform_edt(~donor,return_distances=False,return_indices=True)
            pixels[missing,:3]=pixels[yy[missing],xx[missing],:3]
            frame=Image.fromarray(pixels)
    masks = []
    body = np.asarray(frame.getchannel("A"), dtype=np.float32)/255
    unused = np.ones_like(body)
    for index,(polygon,_) in enumerate(shape):
        if who=="jackal" and index==0:
            # The base of the neck is shared by the head and shoulders.
            support=[JACKAL_NECK[view]]
        else:
            support=[]
        mask=feather_polygons(frame.size,polygon,*support)
        masks.append(mask)
        unused *= 1-mask
    # Overlapping feathered seams keep the cut painted edges attached as joints swing.
    adjusted = [np.minimum(1, mask*1.38) for mask in masks]
    core=REAPER_TRUNK[view] if who=="reaper" else JACKAL_NECK[view]
    trunk=np.maximum(unused*1.68,feather_polygons(frame.size,core)*1.4)
    adjusted.insert(3,np.minimum(1,trunk))
    pieces = []
    for alpha in adjusted:
        part = frame.copy()
        part.putalpha(Image.fromarray(np.uint8(body*alpha*255+.5)))
        pieces.append(one_surface(part))
    ground = max(p.getchannel("A").point(lambda a: 255 if a > 95 else 0).getbbox()[3]
                 for p in pieces if p.getchannel("A").getbbox())
    off = 144-ground
    aligned = []
    for part in pieces:
        cell = Image.new("RGBA", frame.size)
        cell.paste(part, (0, off))
        aligned.append(cell)
    p = [point for _, point in shape]
    p.insert(3, (77 if who == "jackal" else 81, 89 if who == "jackal" else 74))
    if who=="reaper":
        p.extend(REAPER_JOINTS[view])
    return aligned, [(x, y+off) for x, y in p]


def mirror(parts, pivots, who):
    order = (0, 2, 1, 3, 5, 4)
    result = [ImageOps.mirror(parts[p]) for p in order]
    point_order = order if who=="jackal" else (*order,8,9,6,7)
    points = [(160-pivots[p][0], pivots[p][1]) for p in point_order]
    return result, points


def reproject_pair(parts, who):
    left, lp = mirror(*parts[7], who)
    right, rp = parts[7]
    rear = [inbetween(Paint(a), Paint(b), .5, retain_detail=True) for a, b in zip(left, right)]
    return rear, [(float(x+u)/2, float(y+v)/2) for (x, y), (u, v) in zip(lp, rp)]


def authored(source, who):
    keys = {k: split(frame, who, i) for i, (k, frame) in enumerate(zip((0, 1, 2, 7), key_panels(source, who)))}
    for k, donor in ((3, 1), (4, 0), (5, 7)):
        keys[k] = mirror(*keys[donor], who)
    keys[6] = reproject_pair(keys, who)
    return [keys[k] for k in range(8)]


def ora(keys, who, path):
    sheet = Image.new("RGBA", (CELL*4, CELL*2))
    root = ET.Element("image", w=str(sheet.width), h=str(sheet.height), name=who, version="0.0.3")
    layers = ET.SubElement(root, "stack")
    names = ("Głowa", "Tylna lewa", "Tylna prawa", "Tułów", "Przednia lewa", "Przednia prawa") if who == "jackal" else (
        "Hełm", "Lewa noga", "Prawa noga", "Napierśnik i płaszcz", "Lewe ramię", "Prawe ramię i dłoń")
    with zipfile.ZipFile(path, "w") as out:
        out.writestr("mimetype", "image/openraster")
        for k, (parts, _) in enumerate(keys):
            xy = (k%4*CELL, k//4*CELL)
            group = ET.SubElement(layers, "stack", name=f"Kierunek {k+1}", isolation="isolate")
            for p in ORDERS[who][k]:
                sheet.alpha_composite(parts[p], xy)
            for p in reversed(ORDERS[who][k]):
                src = f"data/{k}-{p}.png"
                ET.SubElement(group, "layer", name=names[p], src=src, x=str(xy[0]), y=str(xy[1]),
                              visibility="visible", opacity="1.0", **{"composite-op":"svg:src-over"})
                b = io.BytesIO()
                parts[p].save(b, format="PNG")
                out.writestr(src, b.getvalue())
        b = io.BytesIO()
        sheet.save(b, format="PNG")
        out.writestr("mergedimage.png", b.getvalue())
        out.writestr("stack.xml", ET.tostring(root))


def finish_in_krita(keys, who, work, accent_path):
    from krita_mcp_client import KritaMCP, start_krita_if_needed

    start_krita_if_needed()
    native = Path("C:/AI2D/sprites")/f"emberfall-{who}-20261123.kra"
    native_accent = work/"painted-accent.png"
    if native.is_file() and native.with_suffix(".png").is_file() and native_accent.is_file():
        with zipfile.ZipFile(native) as archive:
            if archive.testzip() is not None:
                raise ValueError(f"Invalid Krita master: {native}")
        with Image.open(native_accent) as marks:
            marks.verify()
        shutil.copyfile(native_accent,accent_path)
        return
    document = work/f"{who}-finish.ora"
    shutil.copyfile(work/f"{who}.ora",document)
    with KritaMCP(work/"krita-finish") as client:
        doc = client.call("krita_document",action="open",path=str(document))["affected_document_id"]
        state = client.call("krita_state",document_id=doc)
        if state["runtime_compatibility"]["status"]!="compatible":
            raise RuntimeError("Krita runtime is incompatible")
        layer = client.call("krita_layers",action="create",document_id=doc,
                            name="Światło pustyni na oczach i metalu")["active_layer_id"]
        for k, (parts, points) in enumerate(keys):
            colors = ["#ffe3ae","#d4ab72"] if who == "jackal" else ["#d4f3cf","#81cdb5"]
            for p, color in zip((0,3), colors):
                if p==0 and k in (5,6,7):
                    continue
                original = np.asarray(parts[p])
                cx, cy = (int(i) for i in points[p])
                # Find a short, already lit edge inside the opaque paint near
                # the brow or breastplate, then trace it in the native master.
                candidates = []
                for y in range(max(2,cy-17),min(cy-3,157)):
                    for x in range(max(2,cx-15),min(cx+15,157)):
                        if np.min(original[y-1:y+2,x-1:x+3,3])<220:
                            continue
                        color_pixels = original[y,x:x+3,:3].astype(float)
                        h = np.min(np.mean(color_pixels,axis=1))
                        candidates.append((h-abs(x-cx)*.7-abs(y-cy+10)*.65,x,y))
                if not candidates:
                    continue
                _, x, y = max(candidates)
                dx,dy=k%4*CELL,k//4*CELL
                client.call("krita_draw",document_id=doc,layer_id=layer,shape="path",
                            points=[{"x":x+dx,"y":y+dy},{"x":x+2+dx,"y":y+dy}],
                            foreground=color,stroke_style="ForegroundColor",fill_style="None",brush_size=1.3)
        client.call("krita_asset",document_id=doc,action="export_layer",layer_id=layer,
                    output_path=str(native_accent.resolve()),crop="canvas",overwrite=False)
        client.call("krita_save_export",document_id=doc,kra_path=str(native),
                    png_path=str(native.with_suffix(".png")),overwrite=False)
        client.call("krita_preview",document_id=doc,max_width=480,max_height=256)
    shutil.copyfile(native_accent,accent_path)


def add_accents(keys, overlay):
    for k, (parts, _) in enumerate(keys):
        x,y=k%4*CELL,k//4*CELL
        marks = np.array(overlay.crop((x,y,x+CELL,y+CELL)))
        # Composite each stroke only on the corresponding body surface.
        applied = np.zeros((CELL,CELL), dtype=bool)
        for p in (0,3):
            old = np.array(parts[p])
            a = old[...,3]
            blend = .68*marks[...,3:4].astype(float)/255
            blend *= ((a>210)&~applied)[...,None]
            old[..., :3] = np.uint8(old[..., :3]*(1-blend)+marks[..., :3]*blend)
            applied |= blend[...,0]>0
            parts[p] = Image.fromarray(old)


def film_inputs(keys, folder):
    folder.mkdir(parents=True, exist_ok=True)
    frames = []
    for parts, _ in keys+keys[:1]:
        canvas = Image.new("RGB", (3*CELL, 2*CELL), (42, 36, 38))
        for i, part in enumerate(parts):
            canvas.paste(part, (i%3*CELL, i//3*CELL), part.getchannel("A"))
        frames.append(canvas)
    frames[0].save(folder/"flow.png", save_all=True, append_images=frames[1:], duration=100)


def film_finish(part, colors, index):
    x, y = index%3*CELL, index//3*CELL
    ai = np.array(colors.crop((x,y,x+CELL,y+CELL)), dtype=np.float32)
    pixels = np.array(part)
    rgb = pixels[..., :3].astype(np.float32)
    # Retain the authored silhouette and the sharp material texture. Limit
    # FILM influence to gentle color correction inside an already opaque part.
    delta = np.clip(ai-rgb, -18, 18)
    rgb += .22 * delta * np.clip((pixels[..., 3:4]-np.float32(165))/90, 0, 1)
    pixels[..., :3] = np.uint8(np.clip(rgb, 0, 255))
    return Image.fromarray(pixels)


def bake(keys, who, work, output, header, film):
    painted = [[Paint(part) for part in parts] for parts, _ in keys]
    views = []
    for k in range(8):
        for n in range(4):
            parts = keys[k][0] if n == 0 else [
                inbetween(a, b, n/4, retain_detail=True)
                for a, b in zip(painted[k], painted[(k+1)%8])]
            model = film/f"view_{k*4+n:03}.png"
            if n and model.exists():
                with Image.open(model) as colors:
                    parts = [film_finish(p, colors, i) for i, p in enumerate(parts)]
            views.append([one_surface(p) for p in parts])
    name = ("dune_jackal" if who == "jackal" else "glass_reaper")
    atlas(work, views, output, header, name=name, ordering=lambda k:ORDERS[who][k])
    contents = header.read_text().replace(f"{name}_paint.py", "desert_enemy_paint.py").rstrip()
    lines = [contents[:-1], "inline constexpr short pivots[8][6][2]={"]
    for _, points in keys:
        lines.append(" {"+",".join("{%d,%d}" % (x,y) for x,y in points[:6])+"},")
    lines.append("};")
    if who=="reaper":
        lines.append("inline constexpr short joints[8][4][2]={")
        for _, points in keys:
            lines.append(" {"+",".join("{%d,%d}" % (x,y) for x,y in points[6:])+"},")
        lines.append("};")
    lines.append("}")
    header.write_text("\n".join(lines)+"\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--prepare", action="store_true")
    parser.add_argument("--krita", action="store_true")
    parser.add_argument("--species",choices=tuple(ORDERS))
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    root = Path("art/desert/new-enemies")
    for who in ORDERS:
        if args.species and who!=args.species:
            continue
        work = args.work/who
        work.mkdir(parents=True, exist_ok=True)
        keys = authored(root/f"{who}-source.png",who)
        if args.prepare:
            ora(keys, who, work/f"{who}.ora")
            film_inputs(keys, work)
        else:
            accents = root/f"{who}-krita.png"
            if args.krita:
                finish_in_krita(keys,who,work,accents)
            if accents.is_file():
                with Image.open(accents) as overlay:
                    add_accents(keys, overlay.convert("RGBA"))
            bake(keys,who,work,Path("assets/desert")/f"{who}-turn",
                 Path("src/generated")/f"desert_{who}_turn_atlas.hpp",work/"film-output")


if __name__ == "__main__":
    main()

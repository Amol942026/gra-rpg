"""Paint the forest creatures' five material tiles in native Krita."""
import argparse
import json
from pathlib import Path
from zipfile import ZipFile

from PIL import Image

from bandit_paints import box
from krita_mcp_client import KritaMCP, start_krita_if_needed
from wanderer_paints import digest, material, origin, write_ora


MATERIALS = {
    0: ("slime", "Szlam · refleksy żelu", (123, 136, 384, 363), "#254634", "#a7d86d", 210),
    1: ("wolf", "Wilk · ciemna okrywa", (187, 157, 316, 229), "#414643", "#acb2a8", 191),
    2: ("skeleton", "Szkielet · spękana kość", (228, 48, 271, 72), "#615c43", "#e5d2a0", 178),
    7: ("treant", "Drzewiec · żłobienia kory", (224, 267, 279, 346), "#3e3022", "#a38b60", 218),
    13: ("wolf", "Wilk · jasne futro pyska", (381, 116, 416, 151), "#515b55", "#d8d7be", 162),
    16: ("treant", "Drzewiec · listowie korony", (331, 53, 383, 97), "#253f2a", "#789451", 220),
}


def prepare(atlas, concepts):
    original = atlas.copy()
    for cell in MATERIALS:
        original.paste((0, 0, 0, 0), box(cell))
    layers = [("Materiały pozostałych przeciwników", original)]
    for cell, (species, label, crop, dark, light, alpha) in MATERIALS.items():
        with Image.open(concepts/(species+".png")) as concept:
            patch = material(concept, crop, dark, light)
        edge = Image.new("L", patch.size)
        edge.putdata([round(alpha*min(1, min(x,y,127-x,127-y)/9))
                      for y in range(128) for x in range(128)])
        patch.putalpha(edge)
        layer = Image.new("RGBA", atlas.size)
        layer.paste(patch, origin(cell))
        layers.insert(0, ("FLUX · "+label, layer))
    return layers


def detail(client, doc, cell, label):
    client.call("krita_layers", action="create", document_id=doc, name="Krita · "+label)
    layer = client.call("krita_state", document_id=doc)["active_layer_id"]
    x, y = origin(cell)
    style = {"document_id":doc, "layer_id":layer, "fill_style":"None",
             "stroke_style":"ForegroundColor"}

    def line(points, color, width):
        client.call("krita_draw", shape="path", foreground=color, brush_size=width,
                    points=[{"x":x+u,"y":y+v} for u,v in points], **style)

    def ring(u, v, w, h, color):
        client.call("krita_draw", shape="ellipse", x=x+u, y=y+v, width=w, height=h,
                    foreground=color, brush_size=1.4, **style)

    if cell == 0:
        for u,v,w,h in [(22,48,13,15),(82,52,10,11),(71,93,16,13),(45,77,7,8),(103,85,8,9)]:
            ring(u,v,w,h,"#79ad60")
            line([(u+2,v+4),(u+4,v+2),(u+6,v+2)],"#c8e593",1.3)
        line([(23,41),(28,32),(37,27),(46,26)],"#c5df8a",2.4)
        line([(86,109),(98,106),(110,99)],"#76b86f",2)
    elif cell in (1,13):
        for i in range(10):
            u=15+(i*29)%88
            v=11+(i*19)%90
            line([(u,v),(u-3,v+6),(u-6,v+12)],
                 ("#888d80" if i%2 else "#58655c") if cell==1 else ("#b9baa2" if i%2 else "#7b8b7e"),1.5)
    elif cell == 2:
        line([(47,12),(54,23),(51,29),(61,39),(58,46)],"#817758",1.8)
        line([(54,23),(63,21),(68,24)],"#a19671",1.35)
        line([(97,66),(90,74),(92,83),(85,93)],"#827754",1.5)
        for u,v in [(22,54),(29,61),(80,98),(94,105),(33,81),(59,108)]:
            line([(u,v),(u+2,v+2)],"#a69873",1.65)
    elif cell == 7:
        for i in range(6):
            u=19+i*16
            line([(u,9+i%3*7),(u-3,30),(u+4,51),(u-4,87),(u+2,116)],"#51412c",2.2)
            line([(u+3,31),(u+7,47),(u+2,62)],"#a69268",1.3)
        for u,v in [(33,71),(42,77),(35,84),(76,46),(84,52),(92,47),(95,56)]:
            line([(u,v),(u+4,v+2),(u+8,v+1)],"#7b9055",2.5)
    else:
        for i in range(15):
            u=13+(i*27)%99
            v=13+(i*37)%96
            line([(u-4,v+2),(u,v),(u+3,v-4)],"#779a55" if i%3 else "#a3aa5c",1.45)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--original", type=Path, required=True)
    parser.add_argument("--concepts", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    root = args.root
    root.mkdir(parents=True, exist_ok=False)
    with Image.open(args.original) as source:
        if source.size != (512, 512):
            raise ValueError("Unexpected source creature atlas dimensions")
        atlas = Image.new("RGBA",(512,640))
        atlas.paste(source.convert("RGBA"),(0,0))
    ora = root/"materials.ora"
    write_ora(ora, prepare(atlas, args.concepts), title="Emberfall — materiały leśnego bestiariusza")
    master, native = root/"forest-creatures.kra", root/"krita-export.png"
    start_krita_if_needed()
    with KritaMCP(root/"krita-log") as client:
        doc = client.call("krita_document", action="open", path=str(ora))["affected_document_id"]
        state = client.call("krita_state", document_id=doc)
        if state["runtime_compatibility"]["status"]!="compatible" or len(state["layers"])!=7:
            raise RuntimeError("Layered inputs did not load")
        for cell, (_, label, *_) in MATERIALS.items():
            detail(client,doc,cell,label)
        client.call("krita_save_export", document_id=doc, kra_path=str(master), png_path=str(native), overwrite=False)
        client.call("krita_preview", document_id=doc, max_width=768, max_height=768)
        state = client.call("krita_state", document_id=doc)
        client.call("krita_document", action="close", document_id=doc, discard_changes=False)
    with Image.open(native) as source, ZipFile(master) as archive:
        painted = source.convert("RGBA")
        merged = Image.open(archive.open("mergedimage.png")).convert("RGBA")
        if painted.tobytes()!=merged.tobytes() or len(state["layers"])!=13:
            raise ValueError("Krita master and native export do not match")
        for cell in MATERIALS:
            atlas.paste(painted.crop(box(cell)), origin(cell))
    result = root/"creature-paints.png"
    atlas.save(result)
    report = {"status":"PASS", "layers":state["layers"], "changed_cells":sorted(MATERIALS),
              "master":str(master), "atlas_sha256":digest(result), "original_sha256":digest(args.original)}
    (root/"paint-receipt.json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    print("Krita: thirteen layers and six verified forest materials.")


if __name__ == "__main__":
    main()

"""Author a three-state transparent treasure atlas with native Krita MCP tools."""
from pathlib import Path
import argparse
import hashlib
import json
import shutil
import zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed


def create(name, logs):
    root = Path(r"C:\AI2D\sprites")
    kra, png = root / (name + ".kra"), root / (name + ".png")
    if kra.exists() or png.exists():
        raise FileExistsError("Use a fresh master name; originals are preserved.")
    start_krita_if_needed()
    with KritaMCP(logs) as c:
        doc = c.call("krita_document", action="create", width=768, height=256,
                     name=name)["affected_document_id"]
        state = c.call("krita_state", document_id=doc)
        assert state["runtime_compatibility"]["status"] == "compatible"
        c.call("krita_edit", action="select_all", document_id=doc)
        c.call("krita_edit", action="clear", document_id=doc, layer_id=state["active_layer_id"])
        c.call("krita_edit", action="deselect", document_id=doc)
        c.call("krita_layers", action="rename", document_id=doc,
               layer_id=state["active_layer_id"], name="Transparent canvas")
        layers = []
        for frame, label in enumerate(("Closed", "Open treasure", "Open empty")):
            offset = frame * 256

            def layer(label, opacity=1):
                c.call("krita_layers", action="create", document_id=doc, name=f"{frame} {label}")
                identifier = c.call("krita_state", document_id=doc)["active_layer_id"]
                if opacity != 1:
                    c.call("krita_layers", action="set_properties", document_id=doc,
                           layer_id=identifier, opacity=opacity)
                layers.append(identifier)
                return identifier

            def poly(identifier, color, points):
                c.call("krita_draw", shape="polygon", document_id=doc, layer_id=identifier,
                       points=[{"x":x+offset,"y":y} for x,y in points], foreground=color,
                       fill_style="ForegroundColor", stroke_style="None", brush_size=1)

            def path(identifier, color, points, size=2):
                c.call("krita_draw", shape="path", document_id=doc, layer_id=identifier,
                       points=[{"x":x+offset,"y":y} for x,y in points], foreground=color,
                       fill_style="None", stroke_style="ForegroundColor", brush_size=size)

            def ellipse(identifier, color, x,y,w,h):
                c.call("krita_draw", shape="ellipse", document_id=doc, layer_id=identifier,
                       x=x+offset,y=y,width=w,height=h,foreground=color,
                       fill_style="ForegroundColor",stroke_style="None")

            shadow = layer(label + " contact shadow", .25)
            ellipse(shadow,"#182b2d",35,205,198,35)
            if frame:
                lid = layer(label + " raised lid")
                poly(lid,"#233438",[(112,104),(91,26),(99,22),(204,70),(226,150),(216,156)])
                poly(lid,"#765032",[(119,103),(99,32),(196,77),(216,144)])
                poly(lid,"#b38b4c",[(119,103),(111,57),(123,62),(134,111)])
                poly(lid,"#b38b4c",[(196,77),(181,70),(202,138),(216,144)])
                poly(lid,"#b88242",[(110,43),(189,80),(205,130),(129,95)])
                path(lid,"#543d2e",[(119,57),(194,92)],2)
                path(lid,"#543d2e",[(123,73),(199,108)],2)
                path(lid,"#ddb974",[(101,31),(197,76),(217,143)],3)
                path(lid,"#f2d297",[(103,32),(194,75)],1)

            body = layer(label + " wooden box")
            poly(body,"#243136",[(39,136),(117,98),(222,145),(222,187),(141,229),(39,180)])
            poly(body,"#765035",[(44,141),(141,186),(141,223),(44,176)])
            poly(body,"#513b2d",[(141,186),(217,149),(217,184),(141,223)])
            poly(body,"#a87841",[(44,136),(118,101),(217,147),(141,186)])
            if frame:
                poly(body,"#2c302b",[(54,136),(119,107),(207,147),(141,179)])
                poly(body,"#493e2e",[(65,138),(119,114),(197,148),(141,174)])
                path(body,"#6b5033",[(82,138),(141,165),(181,146)],2)
                path(body,"#e0b66d",[(45,138),(141,183),(216,147)],3)

            grain = layer(label + " oak grain")
            for y in (152,164,176):
                path(grain,"#493b2e",[(47,y),(137,y+42)],1.5)
                path(grain,"#a07547",[(48,y+2),(134,y+42)],1)
            for y in (192,203,214):
                path(grain,"#352e28",[(145,y),(212,y-33)],1.5)
            path(grain,"#c39150",[(52,150),(67,157),(72,157),(82,162)],1)
            path(grain,"#4f3a2b",[(91,186),(104,190),(113,197)],1)
            ellipse(grain,"#513d2d",74,174,8,4)
            path(grain,"#b28852",[(73,174),(79,173),(85,178)],1)

            brass = layer(label + " aged brass fittings")
            for x in (52,119):
                poly(brass,"#554a31",[(x,145+(x-44)*.46),(x+10,150+(x-44)*.46),
                                       (x+10,188+(x-44)*.46),(x,183+(x-44)*.46)])
                poly(brass,"#b3975a",[(x+2,146+(x-44)*.46),(x+8,149+(x-44)*.46),
                                       (x+8,185+(x-44)*.46),(x+2,182+(x-44)*.46)])
                path(brass,"#ead194",[(x+2,148+(x-44)*.46),(x+2,179+(x-44)*.46)],1)
                for y in (154,177):
                    ellipse(brass,"#ecd39a",x+3,y+(x-44)*.46,3,3)
            path(brass,"#c3a56a",[(45,176),(141,223),(216,185)],3)
            path(brass,"#e5c38a",[(46,175),(140,221)],1)
            path(brass,"#b29b69",[(213,153),(213,180)],3)
            ellipse(brass,"#c6ad74",182,184,9,10)
            ellipse(brass,"#39382d",184,186,5,6)

            if not frame:
                lid = layer("Closed curved oak lid")
                poly(lid,"#253336",[(36,134),(39,119),(51,105),(117,74),(218,121),(225,136),(219,150),(141,190),(43,146)])
                poly(lid,"#9d703d",[(42,132),(47,117),(59,106),(117,80),(213,124),(219,135),(214,145),(141,183)])
                poly(lid,"#be9656",[(48,116),(117,83),(212,126),(207,135),(141,168)])
                poly(lid,"#d3ad68",[(58,108),(117,81),(211,125),(204,127),(119,89)])
                poly(lid,"#835b34",[(42,132),(141,178),(215,140),(213,148),(141,185),(44,142)])
                for a,b in (((52,112),(148,156)),((44,124),(141,169)),((91,94),(187,138))):
                    path(lid,"#6d512f",[a,b],1.5)
                for shift in (0,65):
                    poly(lid,"#6e5c38",[(53+shift,140+shift*.46),(49+shift,128+shift*.46),(53+shift,115+shift*.46),(63+shift,104+shift*.46),(72+shift,100+shift*.46),(81+shift,104+shift*.46),(66+shift,112+shift*.46),(61+shift,123+shift*.46),(61+shift,133+shift*.46),(64+shift,145+shift*.46)])
                    path(lid,"#d1b370",[(58+shift,141+shift*.46),(55+shift,128+shift*.46),(59+shift,116+shift*.46),(69+shift,106+shift*.46),(76+shift,104+shift*.46)],4)
                    path(lid,"#f0d899",[(57+shift,128+shift*.46),(62+shift,116+shift*.46),(71+shift,109+shift*.46)],1)
                lock = layer("Closed clasp and keyhole")
                poly(lock,"#3c3b2d",[(91,163),(106,170),(106,191),(91,184)])
                poly(lock,"#d4b675",[(93,165),(104,170),(104,188),(93,183)])
                ellipse(lock,"#384040",96,174,5,6)
                path(lock,"#384040",[(98,179),(98,183)],2)
                path(lock,"#fae4b6",[(94,167),(94,180)],1)
            elif frame == 1:
                treasure = layer("Open coins and teal gem")
                for x,y in ((87,140),(105,147),(124,153),(142,160),(158,153),(171,146),(133,141),(117,136),(151,143)):
                    ellipse(treasure,"#86663a",x-8,y-3,17,9)
                    ellipse(treasure,"#d8ae53",x-8,y-5,17,8)
                    path(treasure,"#f7df9a",[(x-5,y-3),(x+2,y-2)],1.5)
                poly(treasure,"#245b65",[(164,125),(176,130),(176,142),(163,151),(153,140),(156,130)])
                poly(treasure,"#7bd3bf",[(164,127),(171,132),(163,146),(157,138)])
                path(treasure,"#c6f3d9",[(159,132),(164,129),(168,132)],1.5)

        c.call("krita_save_export", document_id=doc, kra_path=str(kra), png_path=str(png), overwrite=False)
        c.call("krita_preview", document_id=doc, max_width=1024, max_height=384)
        final_state = c.call("krita_state", document_id=doc)
        c.call("krita_document", action="close", document_id=doc, discard_changes=False)
    from PIL import Image
    with Image.open(png) as im:
        im.load()
        assert im.mode == "RGBA" and im.size == (768,256)
        assert im.getchannel("A").getextrema() == (0,255)
        for i in range(3):
            cell = im.crop((i*256,0,(i+1)*256,256))
            assert cell.getpixel((0,0))[3] == 0
            assert cell.getchannel("A").getbbox()
    with zipfile.ZipFile(kra) as archive:
        assert archive.testzip() is None and "maindoc.xml" in archive.namelist()
    destination = Path(__file__).resolve().parents[2] / "assets/sprites/treasure-chests.png"
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        raise FileExistsError("Back up/version the previous runtime atlas before replacement.")
    shutil.copy2(png,destination)
    provenance = {"authoring":"Krita MCP native drawing; original 2D artwork, no external inputs",
                  "master":str(kra),"source_png":str(png),"sha256":hashlib.sha256(png.read_bytes()).hexdigest(),
                  "size":[768,256],"cell":[256,256],"anchor":[128,226],"scale":0.42,
                  "states":["closed","open_treasure","open_empty"],"layers":len(layers),
                  "compatibility":final_state["runtime_compatibility"],"gpu_bytes":768*256*4}
    destination.with_suffix(".json").write_text(json.dumps(provenance,indent=2),encoding="utf-8")
    print(json.dumps({"status":"PASS","atlas":str(destination),"kra":str(kra),"layers":len(layers)},indent=2))


if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--name",required=True)
    parser.add_argument("--log-directory",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    create(args.name,args.log_directory)

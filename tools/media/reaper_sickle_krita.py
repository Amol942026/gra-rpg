"""Paint the Glass Reaper's separate, palm-anchored weapon in Krita."""

import argparse
from pathlib import Path
import shutil
import zipfile

from PIL import Image

from krita_mcp_client import KritaMCP, start_krita_if_needed


def paint(root):
    start_krita_if_needed()
    master = Path("C:/AI2D/sprites/sierp-szklanego-zniwiarza-rgba-20261123.kra")
    png = master.with_suffix(".png")
    with KritaMCP(root/"krita-sierp-rgba") as client:
        doc = client.call("krita_document", action="create", width=96, height=96,
                          name="Emberfall / Sierp Szklanego żniwiarza")["affected_document_id"]
        initial=client.call("krita_state",document_id=doc)
        for background in initial["layers"]:
            client.call("krita_layers",document_id=doc,action="set_properties",
                        layer_id=background["id"],visible=False)

        def layer(name):
            return client.call("krita_layers",action="create",document_id=doc,name=name)["active_layer_id"]

        def shape(layer_id, color, coords, size=1):
            filled = len(coords)>2 and size==0
            client.call("krita_draw",document_id=doc,layer_id=layer_id,
                        shape="polygon" if filled else "path",
                        points=[{"x":x,"y":y} for x,y in coords],
                        foreground=color,fill_style="ForegroundColor" if filled else "None",
                        stroke_style="None" if filled else "ForegroundColor",brush_size=max(1,size))

        blade = layer("Szklane ostrze")
        shape(blade,"#183834",[(44,34),(32,37),(21,45),(15,57),(15,67),(20,77),(29,85),
              (41,89),(34,81),(28,72),(27,62),(31,53),(37,46),(48,42)],0)
        shape(blade,"#45776b",[(44,35),(33,39),(23,47),(18,58),(18,66),(23,77),(34,85),
              (28,75),(23,67),(23,59),(28,50),(36,44),(46,40)],0)
        shape(blade,"#8fbc9c",[(42,38),(32,41),(25,46),(20,54),(18,64),(22,74),(30,81)],2)
        shape(blade,"#27574e",[(41,41),(34,45),(28,51),(24,59),(25,68),(31,77)],2)
        shape(blade,"#b7d2af",[(19,56),(18,64),(21,72),(26,78)],1.1)
        metal = layer("Brązowa oprawa i rękojeść")
        shape(metal,"#472c1b",[(39,35),(56,22),(63,23),(65,28),(48,42),(40,43)],0)
        shape(metal,"#a9804c",[(40,34),(57,22),(62,22),(63,27),(47,40),(43,41)],0)
        shape(metal,"#dcc18a",[(43,35),(58,24),(61,24)],1.6)
        shape(metal,"#33251d",[(47,31),(50,35)],2.4)
        shape(metal,"#33251d",[(53,27),(56,31)],2.4)
        shape(metal,"#e1c68c",[(39,35),(44,40),(47,39)],1.6)
        client.call("krita_save_export",document_id=doc,kra_path=str(master),
                    png_path=str(png),overwrite=False)
        client.call("krita_preview",document_id=doc,max_width=192,max_height=192)
    with zipfile.ZipFile(master) as archive:
        if archive.testzip() is not None:
            raise ValueError("Krita master is damaged")
    with Image.open(png) as image:
        if image.mode!="RGBA" or image.size!=(96,96):
            raise ValueError("Sickle PNG needs a visible RGBA weapon")
        alpha=image.getchannel("A")
        bounds=alpha.getbbox()
        if not bounds or alpha.getextrema()!=(0,255) or min(*bounds[:2],96-bounds[2],96-bounds[3])<3:
            raise ValueError("The weapon and its transparent border must both be present")
    shutil.copyfile(png,Path("assets/desert/reaper-turn/sickle.png"))


if __name__ == "__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    paint(args.root)

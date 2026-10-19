"""Create an editable, layered fantasy prop using native Krita MCP drawing."""
from pathlib import Path
import json, zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed


def create_sprite(name, log_directory):
    root = Path(r"C:\AI2D\sprites")
    kra, png = root/(name+".kra"), root/(name+".png")
    if kra.exists() or png.exists():
        raise FileExistsError("Use a fresh sprite name.")
    start_krita_if_needed()
    with KritaMCP(log_directory) as c:
        created = c.call("krita_document", action="create", width=256, height=256, name=name)
        doc = created["affected_document_id"]
        state = c.call("krita_state", document_id=doc)
        assert state["runtime_compatibility"]["status"] == "compatible"
        c.call("krita_edit", action="select_all", document_id=doc)
        c.call("krita_edit", action="clear", document_id=doc, layer_id=state["active_layer_id"])
        c.call("krita_edit", action="deselect", document_id=doc)
        c.call("krita_layers", action="rename", document_id=doc, layer_id=state["active_layer_id"], name="00 Transparent canvas")
        layers = []
        def layer(name, opacity=1):
            c.call("krita_layers", action="create", document_id=doc, name=name)
            result = c.call("krita_state", document_id=doc)
            identifier = result["active_layer_id"]
            if opacity != 1:
                c.call("krita_layers", action="set_properties", document_id=doc, layer_id=identifier, opacity=opacity)
            layers.append({"id":identifier,"name":name})
            return identifier
        def polygon(identifier, color, coordinates):
            c.call("krita_draw", shape="polygon", document_id=doc, layer_id=identifier,
                   points=[{"x":x,"y":y} for x,y in coordinates], foreground=color,
                   fill_style="ForegroundColor", stroke_style="None", brush_size=1)
        def ellipse(identifier, color, x,y,w,h):
            c.call("krita_draw", shape="ellipse", document_id=doc, layer_id=identifier,
                   x=x,y=y,width=w,height=h,foreground=color,fill_style="ForegroundColor",stroke_style="None")
        def path(identifier,color,coordinates,size=2):
            c.call("krita_draw",shape="path",document_id=doc,layer_id=identifier,
                   points=[{"x":x,"y":y} for x,y in coordinates],foreground=color,
                   stroke_style="ForegroundColor",fill_style="None",brush_size=size)
        ground=layer("01 Contact shadow",.32)
        ellipse(ground,"#162830",65,217,126,18)
        base=layer("02 Carved sandstone pedestal")
        polygon(base,"#3c4149",[(65,205),(94,187),(162,187),(191,205),(173,225),(83,225)])
        polygon(base,"#806957",[(65,202),(92,181),(164,181),(191,202),(166,215),(90,215)])
        polygon(base,"#d1ac73",[(73,200),(96,185),(159,185),(181,200),(160,207),(96,207)])
        path(base,"#e7cd97",[(83,199),(103,190),(155,190),(169,198)],2)
        body=layer("03 Obsidian crystal silhouette")
        polygon(body,"#172d39",[(128,26),(166,67),(177,145),(147,182),(126,198),(87,163),(79,93),(105,52)])
        facet=layer("04 Teal crystal facets")
        polygon(facet,"#216678",[(128,32),(158,70),(146,117),(127,184),(117,102)])
        polygon(facet,"#348e9b",[(128,32),(117,102),(85,94),(107,57)])
        polygon(facet,"#65c6bd",[(117,102),(127,184),(91,159),(85,94)])
        polygon(facet,"#164957",[(158,70),(171,143),(146,177),(127,184),(146,117)])
        polygon(facet,"#8ce1d0",[(128,32),(117,102),(123,84),(134,42)])
        band=layer("05 Gold binding and rune")
        polygon(band,"#b7864d",[(86,139),(122,160),(169,124),(171,135),(124,174),(88,151)])
        path(band,"#f4d39b",[(91,142),(123,163),(168,129)],2.5)
        path(band,"#f8e1ad",[(123,93),(137,107),(126,119),(111,107),(123,93),(123,134)],3)
        shine=layer("06 Facet highlights")
        path(shine,"#c1f5e3",[(109,59),(89,96),(95,144)],2)
        path(shine,"#81cfc7",[(135,43),(157,72),(166,112)],1.5)
        ellipse(shine,"#fff0cc",121,72,4,4)
        ellipse(shine,"#d9f9e6",147,91,3,3)
        c.call("krita_save_export", document_id=doc, kra_path=str(kra), png_path=str(png), overwrite=False)
        c.call("krita_preview", document_id=doc,max_width=512,max_height=512)
        # Exercise real layer export, needed for later character cutout rigs.
        export=c.call("krita_asset",document_id=doc,action="export_layer",layer_id=body,
                      output_path=str(root/(name+"_silhouette.png")),crop="canvas",overwrite=False)
        after=c.call("krita_state",document_id=doc)
        c.call("krita_document",action="close",document_id=doc,discard_changes=False)
    from PIL import Image
    with Image.open(png) as image:
        image.load()
        assert image.size==(256,256) and image.mode=="RGBA"
        alpha=image.getchannel("A")
        assert alpha.getextrema()==(0,255) and alpha.getbbox()
    with zipfile.ZipFile(kra) as archive:
        assert archive.testzip() is None and "maindoc.xml" in archive.namelist()
    result={"status":"PASS","kra":str(kra),"png":str(png),"layers":layers,
            "document_id":doc,"runtime_compatibility":after["runtime_compatibility"],
            "layer_export":export,"created_in":"Krita MCP native draw tools"}
    Path(log_directory,"sprite-report.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
    return result


if __name__=="__main__":
    import argparse
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--name",required=True)
    p.add_argument("--log-directory",type=Path,required=True)
    a=p.parse_args()
    print(json.dumps(create_sprite(a.name,a.log_directory),indent=2))

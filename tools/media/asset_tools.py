"""Reusable 2D asset operations: Synfig cutouts, lighting bake, atlas and loop QA."""
from __future__ import annotations
import hashlib, json, math, shutil, xml.etree.ElementTree as ET
from pathlib import Path
import numpy as np
from PIL import Image


def sha(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def write_json(path, value):
    path=Path(path)
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open("x",encoding="utf-8") as stream:
        json.dump(value,stream,indent=2,ensure_ascii=False)


def bake_lighting(diffuse, maps_manifest, output, direction=(-.45,-.65,.75)):
    """Bake map lighting once; no runtime shader or additional draw calls needed."""
    manifest=json.loads(Path(maps_manifest).read_text(encoding="utf-8-sig"))
    assert manifest["Status"]=="PASS"
    assert sha(diffuse)==manifest["SourceSha256"]
    image=Image.open(diffuse).convert("RGBA")
    rgba=np.asarray(image).astype(np.float32)/255
    maps={x["Type"]:np.asarray(Image.open(x["Path"]).convert("RGB"),dtype=np.float32)/255
          for x in manifest["Maps"]}
    assert all(a.shape[:2]==rgba.shape[:2] for a in maps.values())
    normal=maps["normal"]*2-1
    normal/=np.maximum(np.linalg.norm(normal,axis=2,keepdims=True),.0001)
    light=np.array(direction,dtype=np.float32);light/=np.linalg.norm(light)
    lambert=np.maximum((normal*light).sum(axis=2),0)
    ao=maps["occlusion"][:,:,0]
    lighting=(.64+.40*lambert)*(.82+.18*ao)
    specular=maps["specular"][:,:,0]*np.maximum(normal[:,:,2],0)**24*.045
    linear=rgba[:,:,:3]**2.2
    linear=np.clip(linear*lighting[:,:,None]+specular[:,:,None],0,1)
    result=np.dstack((linear**(1/2.2),rgba[:,:,3]))
    output=Path(output)
    if output.exists():raise FileExistsError(output)
    Image.fromarray(np.round(result*255).astype(np.uint8)).save(output)
    return {"path":str(output),"sha256":sha(output),"size":list(image.size),
            "source_sha256":sha(diffuse),"maps":str(maps_manifest),"runtime_material_shader":False}


def make_synfig_cutout(sprite, project, *, pixels=128, fps=60, frames=60):
    """Native editable import layer with periodic waypoints and unchanged silhouette."""
    project=Path(project)
    if project.exists():raise FileExistsError(project)
    project.parent.mkdir(parents=True,exist_ok=True)
    linked=project.with_name(project.stem+"_source.png")
    if linked.exists():raise FileExistsError(linked)
    shutil.copy2(sprite,linked)
    root=ET.Element("canvas",version="1.2",width=str(pixels),height=str(pixels),
        xres="2834.645669",yres="2834.645669",**{"view-box":"-1.5 1.5 1.5 -1.5",
        "antialias":"2","fps":str(fps),"begin-time":"0f","end-time":f"{frames-1}f","bgcolor":"0 0 0 0"})
    ET.SubElement(root,"name").text="Emberfall rune crystal — periodic cutout"
    ET.SubElement(root,"desc").text="Krita sprite, 2D interpolation, 60 Hz, cyclic virtual endpoint; no 3D."
    layer=ET.SubElement(root,"layer",type="import",active="true",version="0.1",desc="Krita master sprite")
    def value(name,kind,value):
        param=ET.SubElement(layer,"param",name=name)
        el=ET.SubElement(param,kind)
        if kind=="string":el.text=value
        else:el.set("value",str(value))
    value("z_depth","real",0);value("amount","real",1);value("blend_method","integer",0)
    value("filename","string",linked.name)
    value("interpolation","integer",1);value("gamma_adjust","real",1);value("time_offset","time","0s")
    for name,corner in [("tl",(-1.15,1.15)),("br",(1.15,-1.15))]:
        animated=ET.SubElement(ET.SubElement(layer,"param",name=name),"animated",type="vector")
        for frame in range(-1,frames+2):
            t=2*math.pi*frame/frames
            x,y=corner[0]+.10*math.sin(t),corner[1]+.09*math.cos(t)
            point=ET.SubElement(animated,"waypoint",time=f"{frame}f",before="linear",after="linear")
            vector=ET.SubElement(point,"vector")
            ET.SubElement(vector,"x").text=f"{x:.9f}"
            ET.SubElement(vector,"y").text=f"{y:.9f}"
    ET.indent(root)
    with project.open("xb") as stream:
        stream.write(ET.tostring(root,encoding="utf-8",xml_declaration=True))
    return {"project":str(project),"linked_sprite":str(linked),"fps":fps,"frames":frames,"pixels":pixels}


def pack_atlas(frames, output, *, fps=60, pivot=(.5,.9), max_size=2048, padding=2):
    output=Path(output)
    if output.exists():raise FileExistsError(output)
    frames=[Path(p) for p in frames]
    if not frames:raise ValueError("No animation frames.")
    images=[Image.open(p).convert("RGBA") for p in frames]
    w,h=images[0].size
    if any(i.size!=(w,h) for i in images):raise ValueError("Frame dimensions differ.")
    cell_w,cell_h=w+2*padding,h+2*padding
    columns=min(len(images),max_size//cell_w,max(1,math.ceil(math.sqrt(len(images)))))
    rows=math.ceil(len(images)/columns)
    # raylib's desktop backends support NPOT textures. Avoid rounding a small
    # clip up to a 2048x2048 allocation when 1056x1056 contains every frame.
    aw=columns*cell_w;ah=rows*cell_h
    if max(aw,ah)>max_size:raise ValueError("Atlas exceeds texture budget; split the clip.")
    atlas=Image.new("RGBA",(aw,ah))
    records=[];pixels=[];bboxes=[]
    for index,(path,image) in enumerate(zip(frames,images)):
        x=(index%columns)*cell_w+padding;y=(index//columns)*cell_h+padding
        atlas.paste(image,(x,y))
        alpha=image.getchannel("A");bbox=alpha.getbbox()
        if not bbox or bbox[0]<=0 or bbox[1]<=0 or bbox[2]>=w or bbox[3]>=h:
            raise ValueError("Empty frame or content touches canvas boundary: "+str(path))
        bboxes.append(bbox);pixels.append(np.asarray(image,dtype=np.float32))
        records.append({"index":index,"rect":[x,y,w,h],"duration_seconds":1/fps,"sha256":sha(path)})
    atlas.save(output,optimize=True)
    differences=[float(np.abs(a-b).mean()) for a,b in zip(pixels,pixels[1:])]
    seam=float(np.abs(pixels[-1]-pixels[0]).mean())
    if max(differences)==0 or seam>max(differences)*1.3+.01:
        raise ValueError("No motion or visible discontinuity at loop boundary.")
    report={"status":"PASS","texture":output.name,"width":aw,"height":ah,
            "frame_width":w,"frame_height":h,"columns":columns,"fps":fps,"frame_count":len(images),"loop":True,
            "pivot_normalized":list(pivot),"frames":records,"padding":padding,
            "gpu_bytes_rgba8":aw*ah*4,"max_internal_mean_pixel_step":max(differences),
            "loop_mean_pixel_step":seam,"unique_frames":len({hashlib.sha256(p.tobytes()).digest() for p in pixels}),
            "alpha_bounds":bboxes,"sha256":sha(output)}
    write_json(output.with_suffix(".json"),report)
    # A visual inspection artifact, outside the runtime atlas contract.
    preview=output.with_name(output.stem+"_preview.gif")
    images[0].save(preview,save_all=True,append_images=images[2::2],duration=round(2000/fps),loop=0,disposal=2)
    return report

"""Build and bake the four 128-bearing XY cutout turntables in Blender.

From a local Blender with a GPU context call run(repo, output, export).
The active user's file stays open and is not saved or replaced.
"""
import importlib.util
import json
import math
import sys
import time
from pathlib import Path

import bpy
import gpu
import numpy as np
from gpu_extras.batch import batch_for_shader


def load_script(repo, name):
    spec=importlib.util.spec_from_file_location(name,repo/"tools/media"/(name+".py"))
    module=importlib.util.module_from_spec(spec)
    sys.modules[name]=module
    spec.loader.exec_module(module)
    return module


def paint_coverage(tree,alpha):
    # Match the bake's fragment discard before applying the separate mask
    # for the visible lip. Two impressions of one cutout must not build a halo.
    below=tree.nodes.new("ShaderNodeMath");below.operation="LESS_THAN"
    below.label="Próg pigmentu";below.inputs[1].default_value=.25
    keep=tree.nodes.new("ShaderNodeMath");keep.operation="SUBTRACT"
    keep.inputs[0].default_value=1.
    tree.links.new(alpha,below.inputs[0]);tree.links.new(below.outputs[0],keep.inputs[1])
    return keep.outputs[0]


def paint_material(img):
    material=bpy.data.materials.new("Malowanie | "+img.name)
    material.use_nodes=True
    tree=material.node_tree
    tree.nodes.clear()
    tex=tree.nodes.new("ShaderNodeTexImage"); tex.image=img
    alpha=tree.nodes.new("ShaderNodeBsdfTransparent")
    ink=tree.nodes.new("ShaderNodeEmission")
    mix=tree.nodes.new("ShaderNodeMixShader")
    out=tree.nodes.new("ShaderNodeOutputMaterial")
    for a,b in ((tex.outputs["Color"],ink.inputs["Color"]),(paint_coverage(tree,tex.outputs["Alpha"]),mix.inputs[0]),
                (alpha.outputs[0],mix.inputs[1]),(ink.outputs[0],mix.inputs[2]),(mix.outputs[0],out.inputs["Surface"])):
        tree.links.new(a,b)
    material.diffuse_color=(.55,.52,.36,1.)
    return material


def clipped_material(base):
    mat=base.copy()
    nodes=mat.node_tree.nodes
    wire=mat.node_tree.links.new
    uv=nodes.new("ShaderNodeTexCoord")
    point=nodes.new("ShaderNodeVectorMath");point.operation="ADD"
    point.inputs[1].default_value=(0.,0.,1.)
    wire(uv.outputs["UV"],point.inputs[0])
    plane=nodes.new("ShaderNodeVectorMath");plane.operation="DOT_PRODUCT"
    wire(point.outputs["Vector"],plane.inputs[0])
    cut=nodes.new("ShaderNodeMath");cut.operation="LESS_THAN"
    wire(plane.outputs["Value"],cut.inputs[0]);cut.inputs[1].default_value=0.
    keep=nodes.new("ShaderNodeMath");keep.operation="SUBTRACT";keep.inputs[0].default_value=1.
    wire(cut.outputs[0],keep.inputs[1])
    opacity=nodes.new("ShaderNodeMath");opacity.operation="MULTIPLY"
    mix=next(n for n in nodes if n.type=="MIX_SHADER")
    coverage=mix.inputs[0].links[0].from_socket
    wire(coverage,opacity.inputs[0]);wire(keep.outputs[0],opacity.inputs[1])
    wire(opacity.outputs[0],mix.inputs[0])
    return mat,plane.inputs[1]


def program():
    vary=gpu.types.GPUStageInterfaceInfo("marsh_compass_pigment")
    vary.smooth("VEC2","uv");vary.smooth("VEC2","vel")
    info=gpu.types.GPUShaderCreateInfo()
    info.vertex_in(0,"VEC3","point");info.vertex_in(1,"VEC2","brush");info.vertex_in(2,"VEC2","delta")
    info.vertex_out(vary);info.sampler(0,"FLOAT_2D","paint");info.push_constant("FLOAT","light")
    info.push_constant("FLOAT","partId")
    info.push_constant("VEC3","clipLine")
    info.fragment_out(0,"VEC4","outPaint");info.fragment_out(1,"VEC4","outMotion")
    info.vertex_source("void main(){uv=brush;vel=delta;gl_Position=vec4(point,1.);}")
    info.fragment_source("""
        void main(){
            if(dot(vec3(uv,1.),clipLine)<0.)discard;
            vec4 pigment=textureLod(paint,uv,0.);
            if(pigment.a<.25)discard;
            outPaint=vec4(clamp(pigment.rgb*light,0.,1.),1.);
            outMotion=vec4(clamp(.5+vel/16.,0.,1.),partId/255.,1.);
        }
    """)
    return gpu.shader.create_from_info(info)


def strip_mesh(rig):
    n=rig.GRID
    uv=np.column_stack((rig.U.ravel(),1.-rig.V.ravel())).astype("f")
    quads=[(v*n+u,v*n+u+1,(v+1)*n+u+1,(v+1)*n+u) for v in range(n-1) for u in range(n-1)]
    tris=[tri for a,b,c,d in quads for tri in ((a,b,c),(a,c,d))]
    return uv,quads,tris


def verify_rigid_turn(poses):
    """Neutral cutouts may orbit and overlap, but their painted features stay rigid."""
    def local_paint(piece):
        xy=piece.xy-piece.xy[0,0]
        edge=xy[0,-1]
        span=np.linalg.norm(edge)
        if not np.isfinite(span) or span<.001:
            raise ValueError(f"Collapsed paint in {piece.label}")
        u=edge/span
        return np.stack((xy@u,xy@(-u[1],u[0])),axis=-1)

    references=[(p.label,p.paint,local_paint(p)) for p in poses[0].parts]
    for bearing,pose in enumerate(poses):
        if len(pose.parts)!=len(references):
            raise ValueError(f"Missing part at bearing {bearing}")
        for piece,(label,paint,coordinates) in zip(pose.parts,references):
            if paint==piece.paint=="keeper-yoke":
                # The fixed end points project closer together in the front
                # view. Only length may change; the beam has constant width.
                if piece.label!=label or not np.allclose(local_paint(piece)[0],coordinates[0],atol=.02):
                    raise ValueError("Width of the projected crook changed")
                continue
            if ((piece.label,piece.paint)!=(label,paint)
                    or not np.allclose(local_paint(piece),coordinates,rtol=0.,atol=.02)):
                raise ValueError(f"Distorted or swapped pigment: {label}, bearing {bearing}")


def layer_stack(pose):
    # Resolve authored ties once, identically for the offscreen bake and the
    # editable Blender scene. Distinct planes also avoid coplanar depth fights.
    planes=[]
    for j,p in enumerate(pose.parts):
        planes.append((p.order,j,1))
        if p.under is not None:
            if not p.under<p.order:raise ValueError(f"Invalid lower layer: {p.label}")
            planes.append((p.under,j,0))
    ranks={(j,upper):depth for depth,(_,j,upper) in enumerate(sorted(planes))}
    return [(ranks[j,1],ranks.get((j,0)),tuple(p.clip)) for j,p in enumerate(pose.parts)]


def interpolation(action,mode):
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for curve in bag.fcurves:
                    for key in curve.keyframe_points:key.interpolation=mode


def planar_master(rig,poses,s,paints,output,ground,stack):
    w,h,ax,ay=rig.SIZE[s]
    uv,quads,tris=strip_mesh(rig)
    count=len(poses)
    scene=bpy.data.scenes.new(("Ropucha","Pijawka","Błędnik","Dzwonnik")[s]+f" | Obrót {count}")
    scene.frame_start=1;scene.frame_end=count;scene.render.fps=count//4
    scene.render.resolution_x=w*3;scene.render.resolution_y=h*3;scene.render.resolution_percentage=100
    scene.render.film_transparent=True
    camera=bpy.data.cameras.new(scene.name);cam=bpy.data.objects.new(scene.name+" | kamera",camera)
    scene.collection.objects.link(cam);scene.camera=cam;cam.location=(w*.5-ax,ay-h*.5,400)
    camera.type="ORTHO";camera.ortho_scale=max(w,h);camera.clip_end=1000
    for j,part in enumerate(poses[0].parts):
        name=scene.name.split(" |")[0]+" | "+part.label
        mesh=bpy.data.meshes.new(name)
        xyz=np.zeros((uv.shape[0],3),dtype="f")
        xyz[:,:2]=(part.xy.reshape(-1,2)+[0.,ground[0]])*[1,-1]
        mesh.from_pydata(xyz.tolist(),[],quads);mesh.update()
        layer=mesh.uv_layers.new(name="Pigment")
        for face in mesh.polygons:
            for k in face.loop_indices:
                layer.uv[k].vector=uv[mesh.loops[k].vertex_index]
        obj=bpy.data.objects.new(name,mesh);scene.collection.objects.link(obj)
        base=paints[part.paint][2]
        mesh.materials.append(base)
        dual=any(row[j][1] is not None for row in stack)
        clipped=any(row[j][2]!=(0.,0.,1.) for row in stack)
        if clipped:
            material,mask=clipped_material(base)
            obj.material_slots[0].link="OBJECT";obj.material_slots[0].material=material
        if dual:
            under=bpy.data.objects.new(name+" | pod spodem",mesh);scene.collection.objects.link(under)
        for step,row in enumerate(stack):
            front,back,line=row[j];frame=1+step*count/len(stack)
            obj.location.z=front*.015;obj.keyframe_insert(data_path="location",index=2,frame=frame)
            if dual:
                under.location.z=(front if back is None else back)*.015
                under.keyframe_insert(data_path="location",index=2,frame=frame)
                if step==0 or (back is None)!=(stack[step-1][j][1] is None):
                    under.hide_render=under.hide_viewport=back is None
                    under.keyframe_insert(data_path="hide_render",frame=frame)
                    under.keyframe_insert(data_path="hide_viewport",frame=frame)
            if clipped:
                mask.default_value=line;mask.keyframe_insert(data_path="default_value",frame=frame)
        interpolation(obj.animation_data.action,"CONSTANT")
        if dual:interpolation(under.animation_data.action,"CONSTANT")
        if clipped:interpolation(material.node_tree.animation_data.action,"LINEAR")
        obj["pędzel"]=part.paint
        obj.shape_key_add(name="Rysunek")
        mesh.shape_keys.use_relative=False
        # The absolute blocks and keyed ink-order are editable independently.
        for i in range(count):
            p=poses[i].parts[j]
            block=obj.shape_key_add(name=f"{i:03d} | {i*360/count:.4f}°",from_mix=False)
            xyz[:,:2]=(p.xy.reshape(-1,2)+[0.,ground[i]])*[1,-1]
            block.data.foreach_set("co",xyz.ravel());block.interpolation="KEY_LINEAR"
        keys=mesh.shape_keys
        keys.eval_time=keys.key_blocks[1].frame;keys.keyframe_insert(data_path="eval_time",frame=1)
        keys.eval_time=keys.key_blocks[-1].frame;keys.keyframe_insert(data_path="eval_time",frame=count)
        interpolation(keys.animation_data.action,"LINEAR")
    scene.frame_set(1)
    return scene


def render_frame(rig, poses,s,i,dy,shader,brushes,triangles,uv,buffer,sub,margin,
                 read_motion=False, fractional=None, owners_only=False):
    w,h,ax,ay=rig.SIZE[s]
    width,height=w+2*margin,h+2*margin
    framebuffer=buffer[0]
    framebuffer.clear(color=(0.,0.,0.,0.),depth=1.)
    current=poses[i] if fractional is None else fractional
    stack=layer_stack(current)
    shader.bind()
    for j,part in enumerate(current.parts):
        pos=part.xy.reshape(-1,2)
        xy=(pos+[ax+margin,ay+margin+dy[i]])*[2/width,-2/height]+[-1.,1.]
        point=np.empty((len(pos),3),dtype="f")
        point[:,:2]=xy
        if read_motion and fractional is None:
            vel=(poses[(i+1)%128].parts[j].xy-poses[(i-1)%128].parts[j].xy)*.5
            vel=vel.reshape(-1,2).copy()
            vel[:,1]+=(dy[(i+1)%128]-dy[(i-1)%128])*.5
        else:
            vel=np.zeros_like(pos)
        shader.uniform_sampler("paint",brushes[part.paint][1]);shader.uniform_float("light",part.shade)
        shader.uniform_float("partId",float(j+1))
        front,back,line=stack[j]
        # Linear clipping attains its extrema at the corners of the UV card.
        lo=line[2]+min(0.,line[0])+min(0.,line[1])
        hi=line[2]+max(0.,line[0])+max(0.,line[1])
        layers=[]
        if back is not None and lo<0.:layers.append((back,(0.,0.,1.)))
        if hi>=0.:layers.append((front,line))
        for order,line in layers:
            point[:,2]=-float(order)/180.
            shader.uniform_float("clipLine",line)
            batch=batch_for_shader(shader,"TRIS",{"point":point,"brush":uv,"delta":vel.astype("f")},indices=triangles)
            batch.draw(shader)
    def read(slot):
        data=framebuffer.read_color(0,0,width*sub,height*sub,4,slot,"UBYTE")
        return np.frombuffer(data,np.uint8).reshape(height*sub,width*sub,4)[::-1].copy()
    if owners_only:return read(1)[:,:,2]
    return (read(0),read(1)) if read_motion else read(0)


def target(w,h):
    colors=[gpu.types.GPUTexture((w,h),format="RGBA8") for _ in range(2)]
    depth=gpu.types.GPUTexture((w,h),format="DEPTH24_STENCIL8")
    return gpu.types.GPUFrameBuffer(depth_slot=depth,color_slots=colors),colors,depth


def visibility_samples(rig, poses, s, dy, shader, brushes, uv, triangles, output, save_png):
    # Record the actually visible surface in Blender between paintings. At an
    # occlusion edge the two registered drawings can contain different parts.
    w,h,_,_=rig.SIZE[s]
    sub=2
    buffer=target(w*sub,h*sub)
    owners=np.empty((128,4,h*sub,w*sub),dtype=np.uint8)
    stack=[]
    references=output/"visibility-reference"
    references.mkdir(exist_ok=True)
    with buffer[0].bind():
        gpu.state.viewport_set(0,0,w*sub,h*sub)
        for i in range(128):
            for k in range(4):
                t=k/4
                current=rig.frame(s,i+t) if k else poses[i]
                stack.append(layer_stack(current))
                # Register the reference subframe to the same terrain anchor.
                offset={i:(1-t)*dy[i]+t*dy[(i+1)%128]}
                args=(rig,poses,s,i,offset,shader,brushes,triangles,uv,buffer,sub,0)
                if k==2 and i%8==0:
                    rgba,motion=render_frame(*args,read_motion=True,fractional=current)
                    save_png(references/f"{s}-{i:03d}-half.png",rgba)
                    owners[i,k]=motion[:,:,2]
                else:
                    owners[i,k]=render_frame(*args,fractional=current,owners_only=True)
            if i%8==0:
                t=.375
                offset={i:(1-t)*dy[i]+t*dy[(i+1)%128]}
                rgba=render_frame(rig,poses,s,i,offset,shader,brushes,triangles,uv,buffer,
                                  sub,0,fractional=rig.frame(s,i+t))
                save_png(references/f"{s}-{i:03d}-three-eighths.png",rgba)
    np.savez_compressed(output/f"visibility-{s}.npz",owners=owners)
    return stack


def ground_offset(rig,s,poses,shader,brushes,triangles,uv):
    w,h,ax,ay=rig.SIZE[s]
    sub,margin=2,24
    buffer=target((w+2*margin)*sub,(h+2*margin)*sub)
    offsets=np.zeros(128,"f")
    with buffer[0].bind():
        gpu.state.viewport_set(0,0,(w+2*margin)*sub,(h+2*margin)*sub)
        for i in range(128):
            raw=render_frame(rig,poses,s,i,np.zeros(128),shader,brushes,triangles,uv,buffer,sub,margin)
            rows=np.nonzero(raw[:,:,3].max(axis=1)>150)[0]
            if not len(rows):raise RuntimeError(f"Empty painted silhouette {s}/{i}")
            offsets[i]=(ay+margin)-(rows[-1]+1)/sub
    # Smooth the support over less than one toe width to avoid raster phase
    # noise entering the neutral animation. Both ends use the same neighbours.
    kernel=np.exp(-.5*(np.arange(-5,6)/1.6)**2)
    return sum(k*np.roll(offsets,n) for n,k in zip(range(-5,6),kernel/kernel.sum()))


def atlas(rig,s,poses,brushes,output,export,shader,save_png):
    w,h,ax,ay=rig.SIZE[s]
    uv,quads,tris=strip_mesh(rig)
    dy=ground_offset(rig,s,poses,shader,brushes,tris,uv)
    margin,sub=2,4
    buffer=target((w+margin*2)*sub,(h+margin*2)*sub)
    gallery=np.zeros((8*h,8*w,4),"B")
    field=np.zeros((128,h,w,2),"f2")
    ink=np.zeros((128,h,w),bool)
    marks=[]
    with buffer[0].bind():
        gpu.state.viewport_set(0,0,(w+margin*2)*sub,(h+margin*2)*sub)
        for i in range(128):
            color,motion=render_frame(rig,poses,s,i,dy,shader,brushes,tris,uv,buffer,sub,margin,True)
            start=margin*sub
            color=color[start:start+h*sub,start:start+w*sub].reshape(h,sub,w,sub,4)
            motion=motion[start:start+h*sub,start:start+w*sub].reshape(h,sub,w,sub,4)
            count=(color[:,:,:,:,3]>0).sum(axis=(1,3));den=np.maximum(1,count)[:,:,None]
            rgb=(color[:,:,:,:,:3].sum(axis=(1,3))/den).round().astype("B")
            alpha=(count*255/sub**2).round().astype("B")
            tile=np.dstack((rgb,alpha))
            # Fill invisible edge texels for straight-alpha sampling in raylib.
            present=alpha>0
            for _ in range(2):
                border=~present
                accum=np.zeros((h,w,3),"f");n=np.zeros((h,w),"f")
                for axis,step in ((0,-1),(0,1),(1,-1),(1,1)):
                    occ=np.roll(present,step,axis=axis); accum+=np.roll(tile[:,:,:3],step,axis=axis)*occ[:,:,None]
                    n+=occ
                add=border&(n>0)
                tile[add,:3]=(accum[add]/n[add,None]).round().astype("B")
                present|=add
            if max(alpha[0].max(),alpha[-1].max(),alpha[:,0].max(),alpha[:,-1].max())>100:
                raise ValueError(f"Art at cell edge {s}/{i}, bbox {np.where(alpha>100)[0].min()}")
            ypos=(i%64)//8*h; xpos=i%8*w
            gallery[ypos:ypos+h,xpos:xpos+w]=tile
            if i%64==63:save_png(export/f"neutral-{s}-{i//64}.png",gallery)
            field[i]=((motion[:,:,:,:,:2].sum(axis=(1,3))/den)/255.-.5)*16.
            ink[i]=count>0
            r=poses[i]
            m={key:[round(float(x),3),round(float(y+dy[i]),3)] for key,(x,y) in r.land.items()}
            m["limbs"]=[[round(float(q[0]),4),round(float(q[1]+dy[i]),4),
                         round(float(q[2]),4),round(float(radius),4)] for q,radius in r.feet]
            candidates=sorted(r.feet,key=lambda f:f[0][1],reverse=True)
            feet=candidates[:2]
            # Pick a true occupied lower edge near the sketched foot centre.
            measured=[]
            for foot,radius in feet:
                fx=round(float(foot[0])+ax)
                lo,hi=max(0,fx-round(radius)),min(w,fx+round(radius)+1)
                ys,xs=np.where(alpha[:,lo:hi]>120)
                if len(xs):
                    bottom=ys.max()
                    points=xs[ys>=bottom-1]+lo
                    measured.append([round(float(points.mean()-ax),2),round(float(bottom+.5-ay),2),radius])
                else:
                    measured.append([round(float(foot[0]),2),round(float(foot[1]+dy[i]),2),radius])
            m["feet"]=measured;marks.append(m)
    np.savez_compressed(output/f"yaw-raw-{s}.npz",flow=field,ink=ink)
    stack=visibility_samples(rig,poses,s,dy,shader,brushes,uv,tris,output,save_png)
    return marks,dy,stack


def run(repo,output,export,select=(0,1,2,3),make_master=True,direct=False,bearings=None):
    repo,output,export=Path(repo),Path(output),Path(export)
    output.mkdir(parents=True,exist_ok=True);export.mkdir(parents=True,exist_ok=True)
    rig=load_script(repo,"marsh_compass_rig")
    save_png=load_script(repo,"marsh_turn_bake").save_png
    start=time.perf_counter();brushes={};scenes=[]
    old_blend=gpu.state.blend_get();old_depth=gpu.state.depth_test_get()
    old_view=gpu.state.viewport_get()
    marks_file=output/"landmarks.json"
    marks=json.loads(marks_file.read_text()) if marks_file.is_file() else [None]*4
    try:
        for src in sorted((output/"parts").glob("*.png")):
            img=bpy.data.images.load(str(src))
            img.colorspace_settings.name="Non-Color"
            raw=np.empty(img.size[0]*img.size[1]*4,"f")
            img.pixels.foreach_get(raw)
            gl=gpu.types.GPUTexture(tuple(img.size),format="RGBA16F",
                                    data=gpu.types.Buffer("FLOAT",len(raw),raw))
            img.colorspace_settings.name="sRGB";img.pack()
            brushes[src.stem]=(img,gl,paint_material(img))
        shader=program()
        gpu.state.blend_set("NONE");gpu.state.depth_test_set("LESS_EQUAL");gpu.state.depth_mask_set(True)
        seconds={}
        for s in select:
            poses=[rig.frame(s,i) for i in range(128)]
            verify_rigid_turn(poses)
            then=time.perf_counter()
            if direct:
                from types import SimpleNamespace
                painter=SimpleNamespace(**globals())
                sheets=load_script(repo,"marsh_direct_bake")
                marks[s],ground,stack=sheets.export(painter,rig,s,poses,brushes,export,shader,save_png,bearings)
            else:
                marks[s],ground,stack=atlas(rig,s,poses,brushes,output,export,shader,save_png)
            seconds[s]=round(time.perf_counter()-then,3)
            if make_master:
                if direct:
                    if bearings is not None:raise ValueError("Master needs all 512 bearings")
                    positions=np.arange(512)*.25
                    poses=[rig.frame(s,p) for p in positions]
                    ground=np.interp(positions,np.arange(128),ground,period=128)
                scenes.append(planar_master(rig,poses,s,brushes,output,ground,stack))
        if make_master:
            stem="Mokradła-512" if direct else "Mokradła-128"
            if len(select)!=4:stem+="-"+"-".join(map(str,select))
            bpy.data.libraries.write(str(output/(stem+".blend")),set(scenes),compress=True,fake_user=True)
        marks_file.write_text(json.dumps(marks,separators=(",",":")),encoding="utf-8")
        count=(512 if bearings is None else len(bearings)) if direct else 128
        return {"species":list(select),"frames":len(select)*count,"render_seconds":seconds,
                "total_seconds":round(time.perf_counter()-start,3)}
    finally:
        gpu.state.blend_set(old_blend);gpu.state.depth_test_set(old_depth);gpu.state.depth_mask_set(False)
        gpu.state.viewport_set(*old_view)
        if scenes:
            # Remove only IDs created by this job, preserving the user's current scene.
            objects=set(obj for scene in scenes for obj in scene.objects)
            meshes=set(obj.data for obj in objects if obj.type=="MESH")
            clipped_paints={slot.material for obj in objects if obj.type=="MESH" for slot in obj.material_slots if slot.link=="OBJECT"}
            cameras=set(obj.data for obj in objects if obj.type=="CAMERA")
            for scene in scenes:bpy.data.scenes.remove(scene)
            for obj in objects:bpy.data.objects.remove(obj)
            for mesh in meshes:bpy.data.meshes.remove(mesh)
            for material in clipped_paints:bpy.data.materials.remove(material)
            for camera in cameras:bpy.data.cameras.remove(camera)
        for img,gl,material in brushes.values():
            bpy.data.materials.remove(material);bpy.data.images.remove(img)

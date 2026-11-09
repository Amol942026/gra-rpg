"""Blender 5.x offline source models for Emberfall's 2D inventory icons.

Run only with Blender --background --factory-startup --python ... -- ROOT.
Models, studio lighting and bevels are baked to RGBA; no 3D runtime assets.
"""
import bpy, math, json, sys, hashlib
from pathlib import Path
from mathutils import Vector

ROOT=Path(sys.argv[sys.argv.index('--')+1])
ITEMS=json.loads((ROOT/'items.json').read_text(encoding='utf-8-sig'))
(ROOT/'renders').mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
scene=bpy.context.scene
scene.render.engine='BLENDER_EEVEE'
scene.render.resolution_x=scene.render.resolution_y=384;scene.render.resolution_percentage=100
scene.render.film_transparent=True;scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA'
scene.view_settings.view_transform='AgX'
scene.view_settings.look='AgX - Medium High Contrast'
scene.view_settings.exposure=-.6
scene.world.color=(.19,.19,.19)
scene.render.threads_mode='FIXED';scene.render.threads=2
bpy.ops.object.camera_add(location=(0,-3.4,10))
camera=bpy.context.object;camera.rotation_euler=(Vector((0,0,0))-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.type='ORTHO';camera.data.ortho_scale=5.5;scene.camera=camera
for loc,power,size,col in [((-3,-1,6),600,4,(1,.86,.67)),((3,2,4),420,3,(.57,.76,1)),((0,4,3),320,2,(1,.63,.27))]:
 bpy.ops.object.light_add(type='AREA',location=loc);o=bpy.context.object;o.data.energy=power;o.data.shape='DISK';o.data.size=size;o.data.color=col;o.rotation_euler=(-o.location).to_track_quat('-Z','Y').to_euler()
studio=set(scene.objects);groups=[]

def rgb(v):
 def linear(x):return x/12.92 if x<=.04045 else ((x+.055)/1.055)**2.4
 return tuple(linear(((v>>s)&255)/255) for s in (16,8,0))
def material(name,color,metal=.0,rough=.46):
 m=bpy.data.materials.new(name);m.use_nodes=True;n=m.node_tree.nodes;p=n.get('Principled BSDF');p.inputs['Base Color'].default_value=(*rgb(color),1);p.inputs['Metallic'].default_value=metal;p.inputs['Roughness'].default_value=rough
 noise=n.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=47;noise.inputs['Detail'].default_value=3
 bump=n.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.15;bump.inputs['Distance'].default_value=.035;m.node_tree.links.new(noise.outputs['Fac'],bump.inputs['Height']);m.node_tree.links.new(bump.outputs['Normal'],p.inputs['Normal'])
 return m
def finish(o,mat,bevel=.035):
 o.data.materials.append(mat)
 if bevel:
  b=o.modifiers.new('Forged rounded edges','BEVEL');b.width=bevel;b.segments=3
  o.modifiers.new('Weighted normals','WEIGHTED_NORMAL')
 return o
def poly(name,points,mat,thick=.16,z=0):
 n=len(points);vs=[(x,y,z+d) for d in (-thick/2,thick/2) for x,y in points];fs=[tuple(reversed(range(n))),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(vs,[],fs);mesh.update();o=bpy.data.objects.new(name,mesh);scene.collection.objects.link(o);return finish(o,mat)
def cube(name,loc,scale,mat,bevel=.06):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.name=name;o.scale=scale;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);return finish(o,mat,bevel)
def line(name,points,radius,mat):
 c=bpy.data.curves.new(name,'CURVE');c.dimensions='3D';c.resolution_u=18;c.bevel_depth=radius;c.bevel_resolution=3;s=c.splines.new('BEZIER');s.bezier_points.add(len(points)-1)
 for b,p in zip(s.bezier_points,points):b.co=p;b.handle_left_type=b.handle_right_type='AUTO'
 o=bpy.data.objects.new(name,c);scene.collection.objects.link(o);o.data.materials.append(mat);return o
def gem(x,y,z,r,mat):
 bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1,radius=r,location=(x,y,z));return finish(bpy.context.object,mat,0)
def rivets(points,mat,z=.24):
 for x,y in points:gem(x,y,z,.055,mat)
def wrap(x,y,count,mat):
 for j in range(count):line('Spiral leather binding',[(x-.14,y+j*.09,.12),(x+.14,y+.065+j*.09,.12)],.022,mat)

for index,item in enumerate(ITEMS):
 before=set(scene.objects);k=item['kind'];shape=item['shape'];theme=(k*7)%5
 steel=material('Tempered material '+str(k),item['primary'],.72,.31);leather=material('Leather or wood '+str(k),item['secondary'],.05,.61);trim=material('Engraved trim '+str(k),item['trim'],.7,.29);jewel=material('Inset stone '+str(k),item['accent'],.36,.24);dark=material('Deep folds '+str(k),0x17212A,.12,.63)
 if shape in (1,24):
  length=2.8 if shape==1 else 1.9;tip=1.9 if shape==1 else 1.25
  poly('Double edged blade',[(-.18,-.65),(-.27,tip-.5),(0,tip),(.27,tip-.5),(.18,-.65)],steel,.12)
  line('Central fuller',[(0,-.45,.095),(0,tip-.35,.095)],.025,trim)
  line('Curved crossguard',[(-.7,-.57,.05),(-.4,-.7,.1),(0,-.64,.13),(.4,-.7,.1),(.7,-.57,.05)],.09,trim)
  cube('Leather grip',(0,-1.1,0),(.23,.76,.23),leather);wrap(0,-1.43,7,trim);gem(0,-1.63,0,.2,trim);gem(0,-.67,.23,.12,jewel)
 elif shape==2:
  cube('Haft',(0,-.05,0),(.2,3.65,.2),leather);wrap(0,-1.65,10,trim)
  poly('Sweeping axe head',[(-.1,.66),(-.45,.82),(-1.1,.5),(-1.35,.9),(-1.35,1.6),(-.7,1.85),(-.15,1.55),(.48,1.62),(.7,1.25),(.44,.86)],steel,.27)
  line('Sharpened crescent',[(-1.3,.65,.16),(-1.42,1.1,.16),(-1.24,1.6,.16),(-.76,1.77,.16)],.045,trim);gem(0,1.2,.24,.18,jewel)
 elif shape==28:
  line('Recurved bow',[(-.14,-2,0),(.22,-1.7,0),(.63,-1.1,0),(.85,0,0),(.63,1.1,0),(.22,1.7,0),(-.14,2,0)],.15,leather)
  line('Limb inlay',[(-.14,-2,.13),(.3,-1.6,.13),(.7,-.8,.13),(.8,0,.13),(.7,.8,.13),(.3,1.6,.13),(-.14,2,.13)],.035,trim)
  line('Taut bowstring',[(-.14,-2,0),(-.24,0,0),(-.14,2,0)],.013,trim);wrap(.8,-.35,8,dark);gem(.83,0,.21,.13,jewel)
 elif shape==3:
  poly('Helmet shell',[(-1.25,-.65),(-1.1,.65),(-.7,1.38),(0,1.65),(.7,1.38),(1.1,.65),(1.25,-.65),(.8,-1.35),(-.8,-1.35)],steel,.6)
  poly('Dark visor opening',[(-1.02,-.2),(-.86,.28),(.86,.28),(1.02,-.2),(.65,-.58),(-.65,-.58)],dark,.04,.34)
  cube('Nose guard',(0,-.32,.43),(.19,1.75,.16),trim);line('Brow ridge',[(-1.05,.42,.4),(0,.62,.49),(1.05,.42,.4)],.09,trim)
  rivets([(-.84,-.82),(.84,-.82),(-.76,.88),(.76,.88)],trim,.37);gem(0,.94,.43,.19,jewel)
 elif shape in (4,25):
  if shape==25:
   poly('Layered mantle',[(-.55,1.6),(.55,1.6),(1.4,-1.7),(.63,-1.54),(0,-1.82),(-.63,-1.54),(-1.4,-1.7)],leather,.22)
   for dx in (-.68,-.34,0,.34,.68):line('Fabric fold',[(dx*.4,1.0,.15),(dx,-.2,.17),(dx*1.6,-1.55,.15)],.04,trim)
   line('Cloak clasp',[(-.55,1.2,.25),(0,.8,.3),(.55,1.2,.25)],.07,trim);gem(0,.95,.34,.18,jewel)
  else:
   poly('Chest cuirass',[(-.62,1.35),(-1.35,.94),(-1.14,.1),(-.78,-.4),(-.88,-1.3),(0,-1.6),(.88,-1.3),(.78,-.4),(1.14,.1),(1.35,.94),(.62,1.35)],steel,.5)
   for side in (-1,1):
    poly('Layered shoulder',[(side*.7,1.32),(side*1.5,1.2),(side*1.76,.57),(side*1.08,.38)],trim,.32,.1)
    line('Chest emboss',[(side*.65,1,.31),(side*.7,.25,.4),(0,-.1,.4)],.055,trim)
   for y in (-.6,-.92,-1.24):line('Articulated belly plate',[(-.72,y,.32),(0,y-.13,.4),(.72,y,.32)],.055,trim)
   gem(0,.62,.4,.23,jewel);rivets([(-1.15,.85),(1.15,.85),(-.65,-1.1),(.65,-1.1)],trim,.37)
 elif shape==5:
  points=[(-1.15,1.35),(1.15,1.35),(1.28,.2),(.82,-.9),(0,-1.75),(-.82,-.9),(-1.28,.2)]
  poly('Shield rim',points,trim,.35);poly('Shield face',[(x*.86,y*.86) for x,y in points],steel,.25,.17)
  for x in (-.6,0,.6):line('Shield rib',[(x,.95,.34),(x,.05,.35),(x*.45,-1.15,.34)],.045,leather)
  gem(0,.15,.5,.42,trim);gem(0,.15,.85,.16,jewel);rivets([(-1,.95),(1,.95),(-.9,-.2),(.9,-.2),(0,-1.4)],trim,.4)
 elif shape==6:
  for x in (-.65,.65):
   poly('Boot',[(x-.42,1.4),(x+.35,1.4),(x+.28,-.5),(x+.59,-1.15),(x+.54,-1.5),(x-.5,-1.5),(x-.5,-.8)],leather,.55)
   cube('Cuff',(x,1.22,.12),(.93,.3,.65),trim);cube('Sole',(x+.04,-1.4,.12),(1.02,.22,.69),dark)
   for y in (-.4,0,.4,.8):line('Cross lacing',[(x-.22,y,.35),(x+.2,y+.17,.35)],.025,trim)
   cube('Toe cap',(x+.03,-1.06,.3),(.88,.45,.27),steel)
 elif shape in (7,26):
  bpy.ops.mesh.primitive_torus_add(major_radius=.95 if shape==7 else .85,minor_radius=.14,major_segments=48,minor_segments=10,location=(0,-.1,0));finish(bpy.context.object,trim,0)
  if shape==26:
   line('Pendant chain',[(-.7,.6,.05),(-.55,1.35,.05),(0,1.65,.05),(.55,1.35,.05),(.7,.6,.05)],.05,trim)
   poly('Pendant mount',[(-.52,-.35),(0,.32),(.52,-.35),(.42,-1.1),(0,-1.55),(-.42,-1.1)],steel,.28,.2);gem(0,-.67,.5,.48,jewel)
  else:
   cube('Stone setting',(0,.84,.12),(.96,.64,.42),steel);gem(0,.86,.5,.5,jewel)
  for a in range(0,360,45):gem(.95*math.cos(math.radians(a)),-.1+.95*math.sin(math.radians(a)),.12,.07,steel)
 # Inlaid unique small ornaments, without changing the established category.
 if shape in (1,2,24):
  for t in range(2+theme):gem(.12 if t%2 else -.12,.05+t*.22,.15,.035,jewel)
 objects=list(set(scene.objects)-before);collection=bpy.data.collections.new('Item '+str(k)+' '+item['name']);scene.collection.children.link(collection)
 for o in objects:
  for c in list(o.users_collection):c.objects.unlink(o)
  collection.objects.link(o)
  if shape in (1,2,24):o.rotation_euler.z+=math.radians(-28);o.location=Vector((o.location.x*math.cos(-.4887)-o.location.y*math.sin(-.4887),o.location.x*math.sin(-.4887)+o.location.y*math.cos(-.4887),o.location.z))
 collection.hide_render=True;groups.append(collection)

bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'equipment-master.blend'))
log=[]
for item,collection in zip(ITEMS,groups):
 destination=ROOT/'renders'/f'{item["kind"]:03}.png'
 if destination.exists():continue
 collection.hide_render=False;scene.render.filepath=str(destination);bpy.ops.render.render(write_still=True);collection.hide_render=True
 log.append({'kind':item['kind'],'path':str(destination),'sha256':hashlib.sha256(destination.read_bytes()).hexdigest()});print('ICON_DONE',item['kind'],flush=True)
(ROOT/'blender-render.json').write_text(json.dumps({'version':bpy.app.version_string,'engine':scene.render.engine,'new_models':True,'2d_delivery_only':True,'renders':log},ensure_ascii=False,indent=2),encoding='utf-8')

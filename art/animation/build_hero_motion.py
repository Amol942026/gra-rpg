"""Author and bake the Emberfall hero armature with Blender. Run silently:
blender --background --factory-startup -noaudio --python art/animation/build_hero_motion.py -- --no-audio
The editable .blend stays in art/; the game ships only compact compiled poses.
"""
import bpy, math, json
from pathlib import Path
from mathutils import Vector, Quaternion, Matrix, Euler

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'src/generated/hero_motion_data.hpp'
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.preferences.filepaths.save_version=0
scene=bpy.context.scene
scene.render.fps=120
V=lambda p:Vector(p)
def qdeg(x=0,y=0,z=0):return Euler(tuple(math.radians(a) for a in (x,y,z)),'XYZ').to_quaternion()
def solve(a,b,l1,l2,pole):
 d=b-a;n=d.normalized();reach=min(d.length,l1+l2-.001)
 b=a+n*reach;along=(l1*l1-l2*l2+reach*reach)/(2*reach)
 ortho=pole-n*pole.dot(n)
 if ortho.length<.001:ortho=V((1,0,0))
 return a+n*along+ortho.normalized()*math.sqrt(max(0,l1*l1-along*along)),b
shoulders={'L':V((-15,0,53)),'R':V((15,0,53))}
wrists={'L':V((-20,3,33)),'R':V((20,3,33))}
elbows={s:solve(shoulders[s],wrists[s],13,13,V((-1 if s=='L' else 1,-.12,.12)))[0] for s in 'LR'}
hips={s:V((-7 if s=='L' else 7,0,31)) for s in 'LR'}
feet={s:V((-7 if s=='L' else 7,0,4)) for s in 'LR'}
knees={s:solve(hips[s],feet[s],14.5,14.5,V((0,1,0)))[0] for s in 'LR'}
spec=[('Root',None,(0,0,31),(0,0,32)),('Pelvis','Root',(0,0,31),(0,0,38)),
 ('Spine','Pelvis',(0,0,38),(0,0,57)),('Head','Spine',(0,0,61),(0,0,82))]
for s in 'LR':
 spec += [('UpperArm'+s,'Spine',shoulders[s],elbows[s]),('Forearm'+s,'UpperArm'+s,elbows[s],wrists[s]),
 ('Hand'+s,'Forearm'+s,wrists[s],wrists[s]+V((0,0,7)))]
for s in 'LR':
 spec += [('Thigh'+s,'Pelvis',hips[s],knees[s]),('Shin'+s,'Thigh'+s,knees[s],feet[s]),
 ('Foot'+s,'Shin'+s,feet[s],feet[s]+V((0,10,0)))]
spec += [('Weapon','HandR',wrists['R'],wrists['R']+V((0,9.25,35.825)))]
arm=bpy.data.armatures.new('Emberfall_Humanoid_Rig')
obj=bpy.data.objects.new('Emberfall_Hero',arm);scene.collection.objects.link(obj)
bpy.context.view_layer.objects.active=obj;obj.select_set(True);bpy.ops.object.mode_set(mode='EDIT')
for name,parent,head,tail in spec:
 b=arm.edit_bones.new(name);b.head=head;b.tail=tail
 if parent:b.parent=arm.edit_bones[parent]
bpy.ops.object.mode_set(mode='OBJECT');obj.show_in_front=True;arm.display_type='BBONE'
for pb in obj.pose.bones:pb.rotation_mode='QUATERNION'
names=[s[0] for s in spec]
rest={n:arm.bones[n].matrix_local.copy() for n in names}
resthead={n:arm.bones[n].head_local.copy() for n in names}
parents=[names.index(s[1]) if s[1] else -1 for s in spec]
restq={n:rest[n].to_quaternion() for n in names}
ctrl={}
for name in ['Root','Pelvis','Spine','Head','FootL','FootR','HandL','HandR','Weapon']:
 o=bpy.data.objects.new('CTRL_'+name,None);scene.collection.objects.link(o);ctrl[name]=o
 o.empty_display_type='SPHERE';o.empty_display_size=1.2;o.hide_render=True
 o.rotation_mode='XYZ'
def clear_controls():
 for o in ctrl.values():o.animation_data_clear();o.location=(0,0,0);o.rotation_euler=(0,0,0)
def key(name,p,loc=None,rot=None):
 o=ctrl[name];f=1+p*120
 if loc is not None:o.location=loc;o.keyframe_insert('location',frame=f)
 if rot is not None:o.rotation_euler=tuple(math.radians(a) for a in rot);o.keyframe_insert('rotation_euler',frame=f)
def keys(name,rows):
 for row in rows:key(name,*row)
def lerp(a,b,t):return a+(b-a)*t
def curve_settings():
 for o in ctrl.values():
  if not o.animation_data or not o.animation_data.action:continue
  action=o.animation_data.action
  try:curves=action.fcurves
  except AttributeError:curves=action.layers[0].strips[0].channelbag(o.animation_data.action_slot).fcurves
  for fc in curves:
   for k in fc.keyframe_points:k.interpolation='BEZIER';k.handle_left_type=k.handle_right_type='AUTO_CLAMPED'
def author(clip):
 clear_controls()
 if clip in ('Idle','Walk','Run'):
  run=clip=='Run';idle=clip=='Idle'
  for i in range(9):
   t=i/8;w=2*math.pi*t
   # Double support compresses the hips; flight raises them in the running cycle.
   dz=0 if idle else (-.8+.65*math.cos(2*w)) if not run else (-2.2+1.6*math.cos(2*w+math.pi))
   key('Root',t,(0 if idle else .65*math.sin(w),0,dz),(0,0,0))
   key('Pelvis',t,(0,0,0),(0,0 if idle else (3 if run else 4.2)*math.sin(w),0 if idle else (6 if run else 5)*math.cos(w)))
   key('Spine',t,(0,0,0),(-.35*math.sin(w) if idle else (-10 if run else -2.5),0 if idle else -2*math.sin(w),0 if idle else (-16 if run else -12)*math.cos(w)))
   key('Head',t,(0,0,0),(.25*math.sin(w) if idle else (5 if run else 1.5),0,0 if idle else (8 if run else 6)*math.cos(w)))
  for side,s in enumerate('LR'):
   sign=-1 if s=='L' else 1
   for i in range(17):
    t=i/16;u=(t+side*.5)%1;w=2*math.pi*u
    if idle:
     target=tuple(feet[s]);hand=wrists[s]+V((0,0,.15*math.sin(2*math.pi*t)))
     pitch=0
    else:
     contact=.40 if run else .62
     if u<contact:
      v=u/contact;f=(13 if run else 9)*(1-2*v)
      pitch=lerp(14,-28 if run else -22,v)
      z=4+max(0,-math.sin(math.radians(pitch))*8)
     else:
      v=(u-contact)/(1-contact);smooth=v*v*(3-2*v)
      f=-(13 if run else 9)+2*(13 if run else 9)*smooth
      z=4+(16 if run else 8)*math.sin(math.pi*v)**1.5
      pitch=lerp(-28 if run else -22,14,smooth)
     target=(sign*7,f,z)
     # Bent relaxed arms counter the legs. The wrist lags the shoulder slightly.
     a=-(.63 if run else .33)*math.cos(w-.20)-.08
     bend=1.35 if run else .42
     ef=13*math.sin(a);ez=53-13*math.cos(a)
     hand=V((sign*(15.4 if run else 17.5),ef+13*math.sin(a+bend),ez-13*math.cos(a+bend)))
     if not run and s=='R':hand=wrists[s]+V((0,-3*math.cos(w),1*math.sin(w)))
    key('Foot'+s,t,target,(pitch,0,sign*(2 if not idle else 0)))
    key('Hand'+s,t,hand,(0,0,0))
   key('Weapon',0,(0,0,0),(0,0,0));key('Weapon',1,(0,0,0),(0,0,0))
 elif clip in ('Slash','Heavy'):
  heavy=clip=='Heavy';a=.30 if heavy else .23;b=.69 if heavy else .61
  rows=[(0,(0,0,0),(0,0,0)),(a,(0,-1,-1.8),(-2,0,0)),((a+b)/2,(0,2,-2.2),(0,0,0)),(b,(0,1,-1),(0,0,0)),(1,(0,0,0),(0,0,0))]
  keys('Root',rows)
  for t,yaw,lean in [(0,0,0),(a,19 if heavy else 13,-4),((a+b)/2,-4,-7),(b,-24 if heavy else -17,-3),(1,0,0)]:
   key('Pelvis',t,(0,0,0),(0,0,-yaw*.6));key('Spine',t,(0,0,0),(lean,0,-yaw*.65));key('Head',t,(0,0,0),(-lean*.5,0,yaw*.5))
  for s in 'LR':
   sign=-1 if s=='L' else 1
   for t in (0,a,(a+b)/2,b,1):
    key('Foot'+s,t,(sign*7,(-2 if s=='R' else 3)*math.sin(t*math.pi),4),(0,0,(8 if s=='R' else -5)*math.sin(t*math.pi)))
   hands=[(0,wrists[s]),(a,(23,-5,49) if s=='R' else (-18,-2,38)),((a+b)/2,(10,23,43) if s=='R' else (-19,-5,39)),(b,(3,15,37) if s=='R' else (-18,0,37)),(1,wrists[s])]
   for t,p in hands:key('Hand'+s,t,p,(0,0,0))
  # Local wrist roll carries one continuous blade arc; the body supplies the weight.
  for t,pitch,yaw in [(0,0,0),(a,-22,91 if heavy else 79),((a+b)/2,-69,0),(b,-90,-91 if heavy else -78),(1,0,0)]:
   key('Weapon',t,(0,0,0),(pitch,0,-yaw))
 elif clip=='Roll':
  # Anticipation -> shoulder contact -> compact tuck -> foot plant -> standing.
  for t,z,angle in [(0,0,0),(.10,-7,-14),(.22,-7,-64),(.42,-7,-145),(.64,-7,-246),(.82,-7,-328),(1,0,-360)]:
   key('Root',t,(0,0,z),(angle,0,0))
  for t,tuck in [(0,0),(.12,.6),(.25,1),(.62,1),(.82,.65),(1,0)]:
   key('Pelvis',t,(0,0,0),(0,0,0));key('Spine',t,(0,0,0),(-77*tuck,0,0));key('Head',t,(0,0,0),(-27*tuck,0,0))
   for s in 'LR':
    sign=-1 if s=='L' else 1
    key('Foot'+s,t,(sign*7,13*tuck,4+19*tuck),(10*tuck,0,sign*5*tuck))
    key('Hand'+s,t,lerp(wrists[s],V((sign*11,17,43)),tuck),(0,0,0))
   key('Weapon',t,(0,0,0),(0,0,0))
 curve_settings()
def orient(a,b,base):
 # Blender bones point along local Y; minimal swing preserves the authored roll.
 return (base @ Vector((0,1,0))).rotation_difference((b-a).normalized()) @ base
def pose_at(t):
 scene.frame_set(1+int(t*120),subframe=(t*120)%1);bpy.context.view_layer.update()
 C=lambda n:ctrl[n].location.copy()
 Q=lambda n:ctrl[n].rotation_euler.to_quaternion()
 rootpos=V((0,0,31))+C('Root');rq=Q('Root')
 pelvisq=Q('Pelvis');spineq=pelvisq@Q('Spine')
 pelvis=V((0,0,31));spine=pelvis+pelvisq@V((0,0,7))
 mat={}
 mat['Root']=Matrix.LocRotScale(rootpos,rq,None)
 # Work in the unrolled root frame, then apply the same root to every bone.
 world={}
 def put(n,p,q):
  world[n]=(p,q);mat[n]=Matrix.LocRotScale(rootpos+rq@(p-V((0,0,31))),rq@q,None)
 put('Pelvis',pelvis,pelvisq@restq['Pelvis'])
 put('Spine',spine,spineq@restq['Spine'])
 head=spine+spineq@V((0,0,23));put('Head',head,spineq@Q('Head')@restq['Head'])
 for s in 'LR':
  h=pelvis+pelvisq@(hips[s]-V((0,0,31)))
  f=C('Foot'+s)
  if clip!='Roll':f-=C('Root')
  k,f=solve(h,f,14.5,14.5,V((0,1,0)))
  put('Thigh'+s,h,orient(h,k,restq['Thigh'+s]));put('Shin'+s,k,orient(k,f,restq['Shin'+s]))
  put('Foot'+s,f,Q('Foot'+s)@restq['Foot'+s])
  sh=spine+spineq@(shoulders[s]-V((0,0,38)))
  hand=C('Hand'+s)
  e,hand=solve(sh,hand,13,13,spineq@V((-1 if s=='L' else 1,-.2,.15)))
  put('UpperArm'+s,sh,orient(sh,e,restq['UpperArm'+s]));put('Forearm'+s,e,orient(e,hand,restq['Forearm'+s]))
  handq=world['Forearm'+s][1]@restq['Forearm'+s].inverted()@restq['Hand'+s]
  if s=='R' and clip in ('Slash','Heavy'):handq=spineq@Q('Weapon')@restq['Hand'+s]
  put('Hand'+s,hand,handq)
 p,q=world['HandR'];put('Weapon',p,spineq@Q('Weapon')@restq['Weapon'])
 return mat
clips=[]
for clip,duration,loop in [('Idle',2,True),('Walk',.85,True),('Run',.57,True),('Slash',.32,False),('Heavy',.5,False),('Roll',.4,False)]:
 author(clip);obj.animation_data_clear()
 action=bpy.data.actions.new(clip);obj.animation_data_create();obj.animation_data.action=action
 frames=[]
 for i in range(121):
  t=i/120;mat=pose_at(t)
  # Ground the compact roll using capsule contacts of the articulated skeleton.
  if clip=='Roll' and i not in (0,120):
   probes=[]
   for name,radius in [('Pelvis',9),('Spine',10),('Head',10),('HandL',4),('HandR',4),('FootL',4),('FootR',4),('ShinL',4),('ShinR',4)]:
    probes.append((mat[name]@V((0,0,0))).z-radius)
   # The head volume extends along its bone, beyond its neck pivot.
   hp=mat['Head']@V((0,12,0));probes.append(hp.z-10)
   dz=max(0,.5-min(probes))
   for n in names:mat[n].translation.z+=dz
  for n in names:
   pb=obj.pose.bones[n];pb.matrix=mat[n];bpy.context.view_layer.update()
  # Read evaluated parent-relative transforms, not Euler angles.
  row=[]
  for j,n in enumerate(names):
   pb=obj.pose.bones[n];m=pb.matrix.copy()
   local=obj.pose.bones[spec[j][1]].matrix.inverted()@m if parents[j]>=0 else m
   p,q,scale=local.decompose();q.normalize()
   row.extend([*p,*q])
   pb.keyframe_insert('location',frame=i+1,group=n);pb.keyframe_insert('rotation_quaternion',frame=i+1,group=n)
  frames.append(row)
 action.use_fake_user=True;action['duration_seconds']=duration;action['loop']=loop
 clips.append((clip,duration,loop,frames))
obj.animation_data.action=bpy.data.actions['Run']
scene.frame_start=1;scene.frame_end=121;scene.frame_set(24)
# A simple editable mannequin shows the skeleton in Blender; native game armor
# is attached to these same bones by the C++ renderer.
materials={}
for name,color in [('Armor',(.23,.44,.40,1)),('Leather',(.23,.13,.07,1)),('Skin',(.68,.48,.30,1))]:
 m=bpy.data.materials.new(name);m.diffuse_color=color;materials[name]=m
for name,parent,head,tail in spec:
 if name in ('Root','Weapon'):continue
 a,b=V(head),V(tail);mid=(a+b)*.5
 bpy.ops.mesh.primitive_uv_sphere_add(segments=12,ring_count=8,location=mid)
 mesh=bpy.context.object;mesh.name='Preview_'+name
 radius=9 if name=='Pelvis' else 10 if name=='Spine' else 9.5 if name=='Head' else 3.5
 mesh.scale=(radius,max(radius,(b-a).length/2),radius)
 mesh.rotation_mode='QUATERNION';mesh.rotation_quaternion=(b-a).to_track_quat('Y','Z')
 bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 mesh.data.materials.append(materials['Skin' if 'Hand' in name or name=='Head' else 'Leather' if 'Foot' in name else 'Armor'])
 bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
 group=mesh.vertex_groups.new(name=name);group.add(list(range(len(mesh.data.vertices))),1,'REPLACE')
 mod=mesh.modifiers.new('Skeletal deformation','ARMATURE');mod.object=obj;mesh.parent=obj
# Store a source file, containing every baked action and the controller curves.
bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/'art/animation/hero_motion.blend'))
def f(x):
 if abs(x)<.0000005:x=0
 return f'{x:.6f}f'
lines=['#pragma once','// Generated by Blender: art/animation/build_hero_motion.py. Do not hand edit.','namespace ember::hero_motion::data {',
 'enum Bone { '+','.join(names)+', BoneCount };',
 'struct Key {float p[3],q[4];};','struct Clip {const Key* keys;int frames;float duration;bool loop;};',
 'inline constexpr int parents[] = {'+','.join(map(str,parents))+'};',
 'inline constexpr float restHeads[][3] = {'+','.join('{'+','.join(f(x) for x in resthead[n])+'}' for n in names)+'};',
 'inline constexpr float restRotations[][4] = {'+','.join('{'+','.join(f(x) for x in restq[n])+'}' for n in names)+'};']
for clip,duration,loop,frames in clips:
 lines.append(f'inline constexpr Key {clip}Keys[] = {{')
 for row in frames:
  for j in range(len(names)):
   v=row[j*7:j*7+7];lines.append('{{'+','.join(f(x) for x in v[:3])+'},{'+','.join(f(x) for x in v[3:])+'}},')
 lines.append('};')
 lines.append(f'inline constexpr Clip {clip}Clip = {{ {clip}Keys,121,{f(duration)},{str(loop).lower()} }};')
lines.append('}')
OUT.write_text('\n'.join(lines)+'\n',encoding='utf-8')
(ROOT/'art/animation/bake-report.json').write_text(json.dumps({'blender':bpy.app.version_string,'bones':len(names),'clips':[{ 'name':n,'duration':d,'frames':len(fr),'loop':lo} for n,d,lo,fr in clips],'audio':'disabled','runtime':'compiled parent-local quaternion poses; no Blender dependency'},indent=2)+'\n')
print('HERO_MOTION_BAKE_OK',len(names),len(clips),OUT)
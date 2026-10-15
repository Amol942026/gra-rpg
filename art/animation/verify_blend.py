"""Reload the saved Blender source and compare its actual poses with the C++ export."""
import bpy,json,re,math
from pathlib import Path
from mathutils import Matrix,Vector,Quaternion
root=Path(__file__).resolve().parents[2]
text=(root/'src/generated/hero_motion_data.hpp').read_text()
names=re.search(r'enum Bone \{ (.*?)\, BoneCount',text).group(1).split(',')
parents=list(map(int,re.search(r'parents\[\] = \{(.*?)\}',text).group(1).split(',')))
arm=bpy.data.objects['Emberfall_Hero']
checks=0;maximum=0;minimum_dot=1
for clip in ['Idle','Walk','Run','Slash','Heavy','Roll']:
 arm.animation_data.action=bpy.data.actions[clip]
 block=text.split(f'{clip}Keys[] = {{')[1].split('};')[0]
 keys=re.findall(r'\{\{([^}]+)\},\{([^}]+)\}\}',block)
 assert len(keys)==121*17
 for frame in range(121):
  bpy.context.scene.frame_set(frame+1);bpy.context.view_layer.update()
  world=[]
  for bone,name in enumerate(names):
   p,q=keys[frame*17+bone]
   p=Vector([float(v.strip().rstrip('f')) for v in p.split(',')])
   q=Quaternion([float(v.strip().rstrip('f')) for v in q.split(',')]);q.normalize()
   local=Matrix.LocRotScale(p,q,None)
   m=local if parents[bone]<0 else world[parents[bone]]@local
   world.append(m)
   actual=arm.pose.bones[name].matrix
   error=(actual.translation-m.translation).length;dot=abs(actual.to_quaternion().normalized().dot(m.to_quaternion().normalized()))
   maximum=max(maximum,error);minimum_dot=min(minimum_dot,dot)
   assert error<.004 and dot>.99999,(clip,frame,name,error,dot)
   checks+=1
report={'source':'art/animation/hero_motion.blend','blender':bpy.app.version_string,'clips':6,'bones':17,'frames_per_clip':121,'checks':checks,'failures':0,'max_position_error':maximum,'min_rotation_dot':minimum_dot,'actual_reloaded_blend_matches_compiled_pose_data':True,'audio':False}
(root/'dist/qa-v47/blender-source-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2))
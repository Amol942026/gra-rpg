"""Native Synfig 2D fall/get-up controls, baked for the existing hero rig.

No new model or external art. Preview uses the existing original Krita parts;
the game applies this clip to its equipped hero, in the direction of the hit.
"""
from pathlib import Path
import argparse, hashlib, json, math, shutil, subprocess
import xml.etree.ElementTree as ET
from walk_synfig import (REPO, rig_layout, add, sub, scale, dot, norm, unit,
                         qmul, conj, rotate, axis, from_to, animated)
from hero_synfig import value, group, image

FRAMES=72
# Frames: recoil, loss of balance, contact, rest, brace, knee, stand, settle.
KEYS={
 'pitch':[(0,0),(5,15),(13,62),(20,84),(30,84),(39,58),(48,30),(60,6),(72,0)],
 'hip_z':[(0,31),(5,29),(13,21),(20,16),(30,16),(39,15),(48,19),(60,30),(72,31)],
 'chest':[(0,0),(8,-5),(20,4),(30,6),(40,-18),(49,-20),(61,-4),(72,0)],
 'head':[(0,0),(8,-12),(20,-8),(30,-5),(40,10),(52,6),(65,0),(72,0)],
 'thigh_l':[(0,0),(10,14),(22,22),(30,25),(40,-36),(49,-28),(62,-4),(72,0)],
 'thigh_r':[(0,0),(10,-5),(22,8),(30,14),(40,-12),(49,-12),(62,-2),(72,0)],
 'knee_l':[(0,0),(13,15),(23,25),(32,35),(42,63),(52,35),(64,3),(72,0)],
 'knee_r':[(0,0),(13,5),(23,18),(32,25),(42,45),(52,28),(64,2),(72,0)],
 'arm_l':[(0,0),(8,-35),(20,-45),(30,-38),(42,-48),(50,-25),(62,0),(72,0)],
 'arm_r':[(0,0),(8,-22),(20,-28),(30,-25),(42,-35),(50,-18),(62,0),(72,0)],
 'elbow_l':[(0,0),(8,20),(20,16),(30,15),(42,5),(53,12),(65,0),(72,0)],
 'elbow_r':[(0,0),(8,30),(20,25),(30,20),(42,12),(53,16),(65,0),(72,0)],
 'brace':[(0,0),(10,0),(19,.8),(30,.8),(39,1),(45,.7),(53,0),(72,0)],
}
NAMES=list(KEYS)

def circle(canvas,positions,radius,color):
 l=ET.SubElement(canvas,'layer',type='circle',active='true',version='0.2')
 value(l,'amount','real',1);value(l,'blend_method','integer',0);value(l,'radius','real',radius)
 c=ET.SubElement(ET.SubElement(l,'param',name='color'),'color')
 for k,v in zip('rgba',color):ET.SubElement(c,k).text=str(v)
 animated(ET.SubElement(l,'param',name='origin'),'vector',[(p[1]+p[0]*.25,p[2]) for p in positions])

def controls(frame):
 out={}
 for name,keys in KEYS.items():
  for (a,x),(b,y) in zip(keys,keys[1:]):
   if a<=frame<=b:
    u=(frame-a)/(b-a);u=u*u*(3-2*u);out[name]=x+(y-x)*u;break
 return out

def idle_world(layout):
 import re
 text=(REPO/'src/generated/hero_motion_data.hpp').read_text()
 block=text.split('inline constexpr Key IdleKeys[] = {',1)[1].split('};',1)[0]
 keys=[tuple(map(float,re.findall(r'[-\d.]+(?=f)',row))) for row in block.splitlines() if row.startswith('{{')][:17]
 parents=layout[2];world=[]
 for b,k in enumerate(keys):
  p,q=k[:3],unit(k[3:]);parent=parents[b]
  if parent>=0:pp,pq=world[parent];p=add(pp,rotate(pq,p));q=qmul(pq,q)
  world.append((p,q))
 return world

def world_pose(row,layout):
 heads,restq,parents=layout;world=idle_world(layout)
 def bend(pivot,angle):
  q=axis((1,0,0),angle);origin=world[pivot][0]
  for b in range(pivot,17):
   ancestor=b
   while ancestor>=0 and ancestor!=pivot:ancestor=parents[ancestor]
   if ancestor<0:continue
   p,r=world[b];world[b]=(add(origin,rotate(q,sub(p,origin))),qmul(q,r))
 for bone,name in [(2,'chest'),(3,'head'),(10,'thigh_l'),(13,'thigh_r'),(11,'knee_l'),(14,'knee_r'),(4,'arm_l'),(7,'arm_r'),(5,'elbow_l'),(8,'elbow_r')]:bend(bone,row[name])
 root=world[0][0];q=axis((1,0,0),row['pitch'])
 world=[(add((0,0,row['hip_z']),rotate(q,sub(p,root))),qmul(q,r)) for p,r in world]
 # The leading boot is the foot pivot, not the hip. It remains planted as
 # weight tips backwards; the body then braces and rises over the same foot.
 shift=-world[15][0][1]
 world=[(add(p,(0,shift,0)),r) for p,r in world]
 for upper,fore,hand,side in [(4,5,6,-1),(7,8,9,1)]:
  shoulder=world[upper][0];natural=world[hand][0]
  planted=(shoulder[0]+side*8,shoulder[1]+5,5)
  target=add(scale(natural,1-row['brace']),scale(planted,row['brace']))
  delta=sub(target,shoulder);length=min(norm(delta),25.7);direction=unit(delta);target=add(shoulder,scale(direction,length))
  pole=(side,-.15,.25);bend_dir=unit(sub(pole,scale(direction,dot(pole,direction))))
  elbow=add(add(shoulder,scale(direction,length*.5)),scale(bend_dir,math.sqrt(13**2-length**2*.25)))
  for bone,a,b,end in [(upper,shoulder,elbow,heads[fore]),(fore,elbow,target,heads[hand])]:
   skin=from_to(sub(end,heads[bone]),sub(b,a));world[bone]=(a,qmul(skin,restq[bone]))
  world[hand]=(target,world[fore][1])
 # Keep the held weapon beside the right hand, away from head and chest.
 world[16]=(world[9][0],qmul(q,restq[16]))
 # Conservative support: real rounded boots and elbows, without runtime IK.
 bottom=min(world[b][0][2]-r for b,r in [(12,5),(15,5),(11,4),(14,4),(5,4),(8,4),(6,4),(9,4)])
 if bottom<.7:world=[(add(p,(0,0,.7-bottom)),r) for p,r in world]
 return world

def local_pose(world,layout):
 out=[]
 for b,(p,q) in enumerate(world):
  parent=layout[2][b]
  if parent>=0:pp,pq=world[parent];p=rotate(conj(pq),sub(p,pp));q=qmul(conj(pq),q)
  out.append((*p,*unit(q)))
 return out

def author(root,parts):
 root.mkdir(parents=True,exist_ok=False);shutil.copytree(parts,root/'parts');layout=rig_layout()
 rows=[controls(f) for f in range(FRAMES+1)];world=[world_pose(r,layout) for r in rows]
 canvas=ET.Element('canvas',version='1.2',width='480',height='360',fps='60',**{'view-box':'-70 95 60 -5','begin-time':'0f','end-time':'71f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
 ET.SubElement(canvas,'name').text='Emberfall / upadek i podparcie / Synfig 2D'
 ET.SubElement(canvas,'desc').text='Native editable controls; existing hero joint layout. 2D control preview; equipped hero uses the same saved motion in game.'
 defs=ET.SubElement(canvas,'defs')
 for name in NAMES:animated(defs,'real',[r[name] for r in rows],'knockdown_'+name)
 for a,b in [(10,11),(11,12),(13,14),(14,15),(4,5),(5,6),(7,8),(8,9)]:
  for n in range(6):circle(canvas,[add(scale(w[a][0],1-n/5),scale(w[b][0],n/5)) for w in world],2.7,(.2,.4,.36,1))
 # Native cutout rotations show the body actually tipping onto the floor.
 for name,bone,rect in [('body',2,(-14,22,14,-8)),('face',3,(-9,21,9,0)),('hair_front',3,(-10,24,10,5))]:
  g=group(canvas,name);image(g,name,rect)
  rot=ET.SubElement(g,'layer',type='rotate',active='true',version='0.1');value(rot,'origin','vector',(0,0))
  animated(ET.SubElement(rot,'param',name='amount'),'angle',[r['pitch'] for r in rows])
  tr=ET.SubElement(g,'layer',type='translate',active='true',version='0.1')
  animated(ET.SubElement(tr,'param',name='origin'),'vector',[(w[bone][0][1],w[bone][0][2]) for w in world])
 ET.indent(canvas);path=root/'knockdown.sif';ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
 return path

def read_controls(path):
 root=ET.parse(path).getroot();channels={}
 for name in NAMES:
  a=root.find(f"defs/animated[@id='knockdown_{name}']")
  channels[name]=[float(w.find('real').get('value')) for w in a]
  assert len(channels[name])==FRAMES+1
 return [{n:channels[n][i] for n in NAMES} for i in range(FRAMES+1)]

def compile_clip(path,output):
 rows=read_controls(path);layout=rig_layout()
 lines=['#pragma once','// Baked from native Synfig knockdown.sif by tools/media/hero_knockdown_synfig.py.','namespace ember::hero_motion::knockdown_synfig {','inline constexpr float Duration=1.2f;','inline constexpr float pitch[] = {',','.join(f'{r["pitch"]:.8f}f' for r in rows),'};','inline constexpr data::Key keys[] = {']
 for row in rows:
  for key in local_pose(world_pose(row,layout),layout):lines.append('{{'+','.join(f'{x:.8f}f' for x in key[:3])+'},{'+','.join(f'{x:.8f}f' for x in key[3:])+'}},')
 lines+=['};','inline constexpr data::Clip clip = {keys,73,Duration,false};','}']
 output.parent.mkdir(parents=True,exist_ok=True);output.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 return rows

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--parts',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--render',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 source=author(a.root,a.parts);rows=compile_clip(source,a.header)
 if a.render:
  r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(source),'-OutputRoot',str(a.root/'renders'),'-ExpectedFrames',str(FRAMES),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
  (a.root/'render.log').write_text(r.stdout+'\n'+r.stderr,encoding='utf-8');assert r.returncode==0,r.stderr
 sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
 receipt={'status':'PASS','source':'native Synfig 2D controls, existing original Krita cutout parts','project':str(source),'project_sha256':sha(source),'header_sha256':sha(a.header),'control_samples':73,'bone_count':17,'duration':1.2,'rendered_frames':72 if a.render else 0,'external_assets':[]}
 (a.root/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8');print(json.dumps(receipt,indent=2))

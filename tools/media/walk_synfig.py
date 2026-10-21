"""Author native Synfig walk controls and bake a replacement walk clip.

Uses the existing game's joint layout only. No Blender, model generation,
external art, network, or runtime file reads. The SIF is the editable master.
"""
from pathlib import Path
import argparse, hashlib, json, math, re, shutil, subprocess
import xml.etree.ElementTree as ET
from hero_synfig import value, group, image

REPO=Path(__file__).resolve().parents[2]
FRAMES=60
NAMES=['hip_r','hip_z','pelvis_roll','pelvis_yaw','chest_roll','chest_pitch','chest_yaw',
       'head_roll','head_pitch','head_yaw']
for side in 'lr':NAMES += [f'{side}_{n}' for n in ['foot_r','foot_f','foot_z','foot_pitch','hand_r','hand_f','hand_z']]
def add(a,b):return tuple(x+y for x,y in zip(a,b))
def sub(a,b):return tuple(x-y for x,y in zip(a,b))
def scale(a,s):return tuple(x*s for x in a)
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def norm(a):return math.sqrt(dot(a,a))
def unit(a):return scale(a,1/max(1e-12,norm(a)))
def qmul(a,b):
 w,x,y,z=a;v,i,j,k=b
 return (w*v-x*i-y*j-z*k,w*i+x*v+y*k-z*j,w*j-x*k+y*v+z*i,w*k+x*j-y*i+z*v)
def conj(a):return (a[0],-a[1],-a[2],-a[3])
def rotate(q,p):return qmul(qmul(q,(0,*p)),conj(q))[1:]
def axis(a,degrees):
 r=math.radians(degrees)*.5
 return (math.cos(r),*(math.sin(r)*x for x in a))
def from_to(a,b):
 a=unit(a);b=unit(b);d=dot(a,b)
 assert d>-.99999,'opposite bone vectors'
 return unit((1+d,a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]))
def solve(hip,target,length,pole):
 d=sub(target,hip);distance=norm(d);assert .001<distance<2*length-.03,('unreachable joint',distance,hip,target)
 direction=unit(d);bend=unit(sub(pole,scale(direction,dot(pole,direction))))
 knee=add(add(hip,scale(direction,distance*.5)),scale(bend,math.sqrt(length**2-distance**2*.25)))
 return knee,target
def smooth(a,b,t):
 t=max(0,min(1,t));t=t*t*t*(t*(t*6-15)+10)
 return a+(b-a)*t
def keyed(rows,t):
 for (a,x),(b,y) in zip(rows,rows[1:]):
  if a<=t<=b:return smooth(x,y,(t-a)/(b-a))
 raise ValueError(t)
def boot_bottom(pitch):
 # Exact support function of the three existing rounded boot volumes.
 a=math.radians(pitch);s,c=math.sin(a),math.cos(a)
 return min(y*s+z*c-math.hypot(ry*s,rz*c) for y,z,ry,rz in [(3,-1,8.5,4),(1,4,5,4.5),(5,-3,7,1)])
def controls(t):
 w=t*2*math.pi
 row=dict(zip(NAMES[:10],[-.4*math.sin(w),32.9-.45*math.cos(2*w),
      .8*math.sin(w),-2*math.cos(w),-.45*math.sin(w),-1.5,1.5*math.cos(w),
      -.35*math.sin(w),1.2,.5*math.cos(w)]))
 for index,side in enumerate('lr'):
  sign=-1 if side=='l' else 1;u=(t+index*.5)%1;support=.62;half=7.5
  pitch=keyed([(0,8),(.08,0),(.46,0),(.62,-16),(.79,0),(.94,8),(1,8)],u)
  if u<=support:f=half-2*half*u/support;lift=0
  else:
   v=(u-support)/(1-support);v2=v*v;v3=v2*v;velocity=-2*half/support*(1-support)
   f=(2*v3-3*v2+1)*-half+(v3-2*v2+v)*velocity+(-2*v3+3*v2)*half+(v3-v2)*velocity
   lift=3.7*math.sin(math.pi*v)**2
  hand_f=(-4.6*math.cos(w-.12)) if side=='l' else (3+1.1*math.cos(w))
  hand_z=(30.5+.5*math.sin(w-.12)) if side=='l' else (33.5+.35*math.sin(w))
  row.update({f'{side}_foot_r':sign*5.8,f'{side}_foot_f':f,f'{side}_foot_z':-boot_bottom(pitch)+lift,
              f'{side}_foot_pitch':pitch,f'{side}_hand_r':-17.5 if side=='l' else 22,
              f'{side}_hand_f':hand_f,f'{side}_hand_z':hand_z})
 return row

def rig_layout():
 text=(REPO/'src/generated/hero_motion_data.hpp').read_text()
 def array(name):return re.search(r'\b'+name+r'\[.*?=\s*\{(.*?)\};',text,re.S)[1]
 heads=[tuple(map(float,re.findall(r'[-\d.]+(?=f)',s))) for s in re.findall(r'\{([^}]+)\}',array('restHeads'))]
 rotations=[unit(tuple(map(float,re.findall(r'[-\d.]+(?=f)',s)))) for s in re.findall(r'\{([^}]+)\}',array('restRotations'))]
 parents=[int(n) for n in re.findall(r'-?\d+',array('parents'))]
 assert len(heads)==len(rotations)==len(parents)==17
 return heads,rotations,parents

def pose(row,layout):
 heads,restq,parents=layout;world=[None]*17
 pelvis=(row['hip_r'],0,row['hip_z']);identity=(1,0,0,0)
 pq=qmul(axis((0,0,1),row['pelvis_yaw']),axis((0,1,0),row['pelvis_roll']))
 cq=qmul(pq,qmul(axis((0,0,1),row['chest_yaw']),qmul(axis((0,1,0),row['chest_roll']),axis((1,0,0),row['chest_pitch']))))
 hq=qmul(cq,qmul(axis((0,0,1),row['head_yaw']),qmul(axis((0,1,0),row['head_roll']),axis((1,0,0),row['head_pitch']))))
 def put(b,p,skin):world[b]=(p,qmul(skin,restq[b]))
 def orient(b,a,end,rest_end):put(b,a,from_to(sub(rest_end,heads[b]),sub(end,a)))
 world[0]=(pelvis,identity);put(1,pelvis,pq)
 chest=add(pelvis,rotate(pq,(0,0,7)));put(2,chest,cq)
 put(3,add(chest,rotate(cq,(0,0,23))),hq)
 for side,thigh,shin,foot,upper,fore,hand in [('l',10,11,12,4,5,6),('r',13,14,15,7,8,9)]:
  hip=add(pelvis,rotate(pq,sub(heads[thigh],heads[1])))
  target=tuple(row[f'{side}_foot_{k}'] for k in 'rfz');knee,target=solve(hip,target,14.5,(0,1,0))
  orient(thigh,hip,knee,heads[shin]);orient(shin,knee,target,heads[foot])
  put(foot,target,axis((1,0,0),row[f'{side}_foot_pitch']))
  shoulder=add(chest,rotate(cq,sub(heads[upper],heads[2])))
  target=tuple(row[f'{side}_hand_{k}'] for k in 'rfz')
  elbow,target=solve(shoulder,target,13,rotate(cq,(-1 if side=='l' else 1,-.12,.12)))
  orient(upper,shoulder,elbow,heads[fore]);orient(fore,elbow,target,heads[hand])
  fore_skin=qmul(world[fore][1],conj(restq[fore]));put(hand,target,fore_skin)
 put(16,world[9][0],cq)
 local=[]
 for b,(p,q) in enumerate(world):
  if parents[b]>=0:
   pp,pq=world[parents[b]];p=rotate(conj(pq),sub(p,pp));q=qmul(conj(pq),q)
  local.append((*p,*unit(q)))
 return local,world

def animated(parent,kind,samples,identifier=None):
 a=ET.SubElement(parent,'animated',type=kind,**({'id':identifier} if identifier else {}))
 for frame,sample in enumerate(samples):
  w=ET.SubElement(a,'waypoint',time=f'{frame}f',before='linear',after='linear')
  if kind=='vector':
   v=ET.SubElement(w,'vector');ET.SubElement(v,'x').text=f'{sample[0]:.9f}';ET.SubElement(v,'y').text=f'{sample[1]:.9f}'
  else:ET.SubElement(w,kind,value=f'{sample:.9f}')
 return a
def project(v):return v[0]+v[1]*.55,v[2]-v[1]*.16
def circle(canvas,positions,radius,color):
 l=ET.SubElement(canvas,'layer',type='circle',active='true',version='0.2')
 value(l,'amount','real',1);value(l,'blend_method','integer',0);value(l,'radius','real',radius)
 c=ET.SubElement(ET.SubElement(l,'param',name='color'),'color')
 for k,v in zip('rgba',color):ET.SubElement(c,k).text=str(v)
 animated(ET.SubElement(l,'param',name='origin'),'vector',[project(p) for p in positions])
def translated_image(canvas,name,rect,positions):
 g=group(canvas,name);image(g,name,rect)
 l=ET.SubElement(g,'layer',type='translate',active='true',version='0.1')
 animated(ET.SubElement(l,'param',name='origin'),'vector',[project(p) for p in positions])
def author(root,parts):
 root.mkdir(parents=True,exist_ok=False);shutil.copytree(parts,root/'parts')
 rows=[controls(i/FRAMES) for i in range(FRAMES+1)];layout=rig_layout();world=[pose(r,layout)[1] for r in rows]
 canvas=ET.Element('canvas',version='1.2',width='400',height='480',fps='60',**{'view-box':'-35 90 40 -5','begin-time':'0f','end-time':'59f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
 ET.SubElement(canvas,'name').text='Emberfall / naturalny chód / native control rig'
 ET.SubElement(canvas,'desc').text='Editable foot contacts, hip weight transfer and contralateral arms. 2D control preview; final equipment is rendered by the game.'
 defs=ET.SubElement(canvas,'defs')
 for name in NAMES:animated(defs,'real',[r[name] for r in rows],'walk_'+name)
 for a,b in [(10,11),(11,12),(13,14),(14,15),(4,5),(5,6),(7,8),(8,9)]:
  for n in range(7):
   positions=[add(scale(w[a][0],1-n/6),scale(w[b][0],n/6)) for w in world]
   circle(canvas,positions,2.4,(.15,.35,.32,1) if a in [10,11,4,5] else (.25,.5,.47,1))
 translated_image(canvas,'body',(-14,22,14,-8),[w[2][0] for w in world])
 translated_image(canvas,'face',(-9,21,9,0),[w[3][0] for w in world])
 translated_image(canvas,'hair_front',(-10,24,10,5),[w[3][0] for w in world])
 for b in [12,15]:circle(canvas,[w[b][0] for w in world],3,(.32,.2,.12,1))
 ET.indent(canvas);path=root/'walk.sif';ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
 return path
def read_controls(path):
 root=ET.parse(path).getroot();channels={}
 for n in NAMES:
  a=root.find(f"defs/animated[@id='walk_{n}']");assert a is not None,n
  values={int(w.get('time')[:-1]):float(w.find('real').get('value')) for w in a};assert set(values)==set(range(FRAMES+1))
  channels[n]=values
 return [{n:channels[n][i] for n in NAMES} for i in range(FRAMES+1)]
def compile_clip(path,output):
 rows=read_controls(path);layout=rig_layout();lines=['#pragma once','// Baked from native Synfig walk.sif controls; tools/media/walk_synfig.py.','namespace ember::hero_motion::walk_synfig {','inline constexpr data::Key keys[] = {']
 for i in range(FRAMES*2+1):
  f=i*.5;a=min(int(f),FRAMES-1);v=f-a;row={n:rows[a][n]*(1-v)+rows[a+1][n]*v for n in NAMES}
  local,_=pose(row,layout)
  for bone in local:lines.append('{{'+','.join(f'{x:.8f}f' for x in bone[:3])+'},{'+','.join(f'{x:.8f}f' for x in bone[3:])+'}},')
 lines+=['};','inline constexpr data::Clip clip = {keys,121,1.f,true};','}']
 output.parent.mkdir(parents=True,exist_ok=True);output.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 return rows
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--parts',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--render',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 source=author(a.root,a.parts);rows=compile_clip(source,a.header)
 if a.render:
  r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(source),'-OutputRoot',str(a.root/'renders'),'-ExpectedFrames','60','-Threads','2','-RequireMotion'],capture_output=True,text=True)
  (a.root/'render.log').write_text(r.stdout+'\n'+r.stderr,encoding='utf-8');assert r.returncode==0,r.stderr
 report={'status':'PASS','source':'native Synfig real waypoints; existing joint layout; no 3D authoring','project':str(source),'project_sha256':sha(source),'header_sha256':sha(a.header),'loop':True,'control_samples':len(rows),'pose_samples':121,'rendered_frames':60 if a.render else 0,'external_assets':[]}
 (a.root/'receipt.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))

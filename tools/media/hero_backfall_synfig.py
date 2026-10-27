"""Author a supine backfall, grounded rest and slow get-up in native Synfig.

Editable 2D contact targets deform only the existing hero rig. Original Krita
parts provide the cutout preview; no model, external asset or service is used.
"""
from pathlib import Path
import argparse,hashlib,json,math,shutil,subprocess,xml.etree.ElementTree as ET
from walk_synfig import REPO,rig_layout,add,sub,scale,dot,norm,unit,qmul,conj,rotate,axis,from_to,animated,boot_bottom
from hero_knockdown_synfig import local_pose,circle
from hero_synfig import value,group,image

FPS=60;DURATION=1.85;FRAMES=111
# Seconds: take-off .10, airborne .20-.38, back contact .45, settle .64,
# supine rest .66-.80, side brace 1.07, kneel 1.24, stand at 1.75.
TIMES=[0,.10,.24,.45,.53,.66,.80,.93,1.07,1.24,1.42,1.60,1.75,1.85]
VALUES={
 'pitch':[0,12,48,89,86,90,90,82,58,-35,-28,-18,0,0],
 'roll':[0,0,0,0,0,0,0,-25,-32,-10,-4,0,0,0],
 'hip_f':[0,-2,-8,-20,-20,-20,-20,-20,-16,-10,-5,-1,0,0],
 'hip_z':[31,34,37,9.5,12,9,9,16,18,15,20,27,31,31],
 'chest':[0,0,0,0,1,0,0,-3,-8,-4,0,0,0,0],
 'head':[0,-8,-14,-7,-5,-3,-3,2,8,6,4,2,0,0],
 'foot_l_f':[0,2,11,7,7,7,7,2,-7,-20,-17,-4,0,0],
 'foot_r_f':[0,1,9,6,6,6,6,7,9,7,4,2,0,0],
 'foot_l_lift':[0,5,16,0,1,0,0,0,0,0,0,0,0,0],
 'foot_r_lift':[0,4,13,0,1,0,0,0,0,0,0,0,0,0],
 'foot_l_pitch':[0,10,45,78,76,76,76,60,25,3,0,0,0,0],
 'foot_r_pitch':[0,8,40,75,74,74,74,55,10,0,0,0,0,0],
 'hand_l_r':[-20,-24,-27,-23,-23,-23,-23,-25,-25,-23,-21,-20,-20,-20],
 'hand_l_f':[3,1,-15,-42,-42,-42,-42,-34,-25,-10,4,4,3,3],
 'hand_l_z':[33,38,39,8,9,7,7,5,4,5,16,28,33,33],
 'hand_r_r':[24,27,30,24,24,24,24,25,26,27,26,25,24,24],
 'hand_r_f':[3,0,-12,-40,-40,-40,-40,-29,-15,5,8,6,3,3],
 'hand_r_z':[33,39,43,10,11,9,9,13,28,35,34,34,33,33],
 'weapon_yaw':[0,25,65,78,78,78,78,70,55,40,25,10,0,0],
 'weapon_pitch':[75.5297,55,25,12,12,12,12,20,30,45,55,65,75.5297,75.5297],
}
NAMES=list(VALUES)

def keyed(values,t):
 # Shape-preserving cubic: smooth velocity without overshoot or stopping at
 # every intermediate pose. Repeated values define true holds and contacts.
 slopes=[(values[i+1]-values[i])/(TIMES[i+1]-TIMES[i]) for i in range(len(TIMES)-1)]
 tangents=[0.]
 for i in range(1,len(TIMES)-1):
  a,b=slopes[i-1],slopes[i]
  tangents.append(0 if a*b<=0 else 2*a*b/(a+b))
 tangents.append(0.)
 for i in range(len(TIMES)-1):
  if TIMES[i]<=t<=TIMES[i+1]+1.e-8:
   dt=TIMES[i+1]-TIMES[i];u=max(0,min(1,(t-TIMES[i])/dt));u2=u*u;u3=u2*u
   return (2*u3-3*u2+1)*values[i]+(u3-2*u2+u)*dt*tangents[i]+(-2*u3+3*u2)*values[i+1]+(u3-u2)*dt*tangents[i+1]
 raise ValueError(t)

def controls(frame):return {name:keyed(values,frame/FPS) for name,values in VALUES.items()}

def world_pose(row,layout):
 heads,restq,parents=layout;world=[None]*17
 body=qmul(axis((0,1,0),row['roll']),axis((1,0,0),row['pitch']))
 chest_q=qmul(body,axis((1,0,0),row['chest']));head_q=qmul(chest_q,axis((1,0,0),row['head']))
 pelvis=(0,row['hip_f'],row['hip_z'])
 def put(b,p,skin):world[b]=(p,qmul(skin,restq[b]))
 def orient(b,start,end,rest_end):
  skin=from_to(sub(rest_end,heads[b]),sub(end,start));put(b,start,skin);return skin
 def joint(start,target,length,pole):
  v=sub(target,start);d=min(2*length-.08,max(.01,norm(v)));direction=unit(v);target=add(start,scale(direction,d))
  n=sub(pole,scale(direction,dot(pole,direction)))
  if norm(n)<.001:n=sub((1,0,0),scale(direction,direction[0]))
  bend=unit(n);center=add(start,scale(direction,d*.5));h=math.sqrt(max(0,length*length-d*d*.25))
  mid=add(center,scale(bend,h))
  if mid[2]<4.5 and h>.001:
   up=unit(sub((0,0,1),scale(direction,direction[2])))
   side=unit((direction[1]*up[2]-direction[2]*up[1],direction[2]*up[0]-direction[0]*up[2],direction[0]*up[1]-direction[1]*up[0]))
   c=max(-1,min(1,(4.5-center[2])/h/max(.001,up[2])))
   bend=add(scale(up,c),scale(side,(1 if dot(side,bend)>=0 else -1)*math.sqrt(max(0,1-c*c))))
   mid=add(center,scale(bend,h))
  return mid,target
 world[0]=(pelvis,(1,0,0,0));put(1,pelvis,body)
 chest=add(pelvis,rotate(body,(0,0,7)));put(2,chest,chest_q)
 put(3,add(chest,rotate(chest_q,(0,0,23))),head_q)
 for side,thigh,shin,foot,upper,fore,hand,sign in [('l',10,11,12,4,5,6,-1),('r',13,14,15,7,8,9,1)]:
  hip=add(pelvis,rotate(body,sub(heads[thigh],heads[1])))
  pitch=row[f'foot_{side}_pitch'];target=(sign*7,row[f'foot_{side}_f'],.6-boot_bottom(pitch)+row[f'foot_{side}_lift'])
  knee,target=joint(hip,target,14.5,rotate(body,(0,1,0)))
  orient(thigh,hip,knee,heads[shin]);orient(shin,knee,target,heads[foot]);put(foot,target,axis((1,0,0),pitch))
  shoulder=add(chest,rotate(chest_q,sub(heads[upper],heads[2])))
  target=tuple(row[f'hand_{side}_{coordinate}'] for coordinate in 'rfz')
  pole=rotate(chest_q,(sign,-.2,.35));pole=(pole[0],pole[1],max(.35,pole[2]))
  elbow,target=joint(shoulder,target,13,pole)
  orient(upper,shoulder,elbow,heads[fore]);skin=orient(fore,elbow,target,heads[hand]);put(hand,target,skin)
 yaw,pitch=map(math.radians,(row['weapon_yaw'],row['weapon_pitch']));rest=math.radians(75.5297)
 direction=(math.sin(yaw)*math.cos(pitch),math.cos(yaw)*math.cos(pitch),math.sin(pitch))
 put(16,world[9][0],from_to((0,math.cos(rest),math.sin(rest)),direction))
 return world

def author(root,parts):
 root.mkdir(parents=True,exist_ok=False);shutil.copytree(parts,root/'parts');layout=rig_layout()
 rows=[controls(f) for f in range(FRAMES+1)];world=[world_pose(r,layout) for r in rows]
 canvas=ET.Element('canvas',version='1.2',width='600',height='440',fps=str(FPS),**{'view-box':'-85 105 50 -5','begin-time':'0f','end-time':f'{FRAMES-1}f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
 ET.SubElement(canvas,'name').text='Emberfall / lot na plecy i powolne wstawanie / Synfig 2D'
 ET.SubElement(canvas,'desc').text='Authored 2D foot/palm contacts, supine landing and slow side-braced recovery. Original Krita cutouts, existing equipped hero rig.'
 defs=ET.SubElement(canvas,'defs')
 for name in NAMES:animated(defs,'real',[r[name] for r in rows],'backfall_'+name)
 for a,b in [(10,11),(11,12),(13,14),(14,15),(4,5),(5,6),(7,8),(8,9)]:
  for n in range(6):circle(canvas,[add(scale(w[a][0],1-n/5),scale(w[b][0],n/5)) for w in world],2.5,(.2,.42,.37,1))
 for name,bone,rect,extra in [('body',2,(-14,22,14,-8),'chest'),('face',3,(-9,21,9,0),'head'),('hair_front',3,(-10,24,10,5),'head')]:
  g=group(canvas,name);image(g,name,rect);rot=ET.SubElement(g,'layer',type='rotate',active='true',version='0.1');value(rot,'origin','vector',(0,0))
  animated(ET.SubElement(rot,'param',name='amount'),'angle',[r['pitch']+r[extra] for r in rows])
  tr=ET.SubElement(g,'layer',type='translate',active='true',version='0.1');animated(ET.SubElement(tr,'param',name='origin'),'vector',[(w[bone][0][1]+w[bone][0][0]*.25,w[bone][0][2]) for w in world])
 ET.indent(canvas);path=root/'knockdown.sif';ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True);return path

def read_controls(path):
 root=ET.parse(path).getroot();channels={}
 for name in NAMES:
  a=root.find(f"defs/animated[@id='backfall_{name}']");channels[name]=[float(w.find('real').get('value')) for w in a];assert len(channels[name])==FRAMES+1
 return [{n:channels[n][i] for n in NAMES} for i in range(FRAMES+1)]

def compile_clip(path,output):
 rows=read_controls(path);layout=rig_layout()
 lines=['#pragma once','// Baked from native Synfig backfall controls by tools/media/hero_backfall_synfig.py.','namespace ember::hero_motion::knockdown_synfig {',f'inline constexpr float Duration={DURATION}f;',f'inline constexpr int Frames={FRAMES};','inline constexpr float pitch[] = {',','.join(f'{r["pitch"]:.8f}f' for r in rows),'};','inline constexpr data::Key keys[] = {']
 for row in rows:
  for key in local_pose(world_pose(row,layout),layout):lines.append('{{'+','.join(f'{x:.8f}f' for x in key[:3])+'},{'+','.join(f'{x:.8f}f' for x in key[3:])+'}},')
 lines+=['};',f'inline constexpr data::Clip clip = {{keys,{FRAMES+1},Duration,false}};','}'];output.write_text('\n'.join(lines)+'\n',encoding='utf-8');return rows

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--parts',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--render',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 source=author(a.root,a.parts);compile_clip(source,a.header)
 if a.render:
  r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(source),'-OutputRoot',str(a.root/'renders'),'-ExpectedFrames',str(FRAMES),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
  (a.root/'render.log').write_text(r.stdout+'\n'+r.stderr,encoding='utf-8');assert r.returncode==0,r.stderr
 sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
 receipt={'status':'PASS','source':'native Synfig 2D backfall/contact targets; existing original Krita cutout parts','project':str(source),'project_sha256':sha(source),'header_sha256':sha(a.header),'control_samples':FRAMES+1,'bone_count':17,'duration':DURATION,'rendered_frames':FRAMES if a.render else 0,'external_assets':[],'stages_seconds':{'takeoff':.10,'airborne':.24,'back_contact':.45,'supine_rest':[.66,.80],'side_brace':1.07,'kneel':1.24,'stand':1.75,'end':1.85}}
 (a.root/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8');print(json.dumps(receipt,indent=2))

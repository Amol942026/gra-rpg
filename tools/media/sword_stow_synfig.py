"""Native Synfig keyframes for reversible sheathing, and exact C++ export.

Named animated value nodes are the authoring source. The 2D cutout preview is
baked from those controls; runtime applies them to the existing arm IK/rig.
"""
from pathlib import Path
import argparse, hashlib, json, math, shutil, subprocess, xml.etree.ElementTree as ET
from hero_synfig import value,group,image,rotate,curve

MOUTH=(-22,8,37)
AXIS=(-.08,-.44,-math.sqrt(1-.08**2-.44**2))
INSERT=[0,0,0,0,.25,.7,1,1,1]
GRIPS=[tuple(m-a*(37-31.8*v) for m,a in zip(MOUTH,AXIS)) for v in INSERT]
PITCH=math.degrees(math.atan2(AXIS[2],AXIS[1]))
NAMES=['reach','alignment','insertion','yaw','lean','pitch','handR','handF','handZ']
# First move the sword out on the right, then turn its tip forward/down.
# Only after the blade clears the head does the hand cross in front to the hip.
KEYS=[[0,1,1,1,1,1,1,.5,0],[0,0,0,1,1,1,1,1,1],INSERT,
      [0,0,40,95,95,95,95,40,0],[0,0,-5,-13,-13,-13,-13,-6,0],
      [75.5297,20,-65,PITCH,PITCH,PITCH,PITCH,PITCH,PITCH],
      [20,23,6]+[v[0] for v in GRIPS[3:]],
      [3,20,28]+[v[1] for v in GRIPS[3:]],
      [33,42,61]+[v[2] for v in GRIPS[3:]]]
FRAMES=36

def animated(parent,name,kind,values):
 a=ET.SubElement(parent,'animated',type=kind,id=name)
 for f,v in enumerate(values):
  w=ET.SubElement(a,'waypoint',time=f'{f}f',before='linear',after='linear')
  if kind=='vector':
   e=ET.SubElement(w,'vector');ET.SubElement(e,'x').text=f'{v[0]:.8f}';ET.SubElement(e,'y').text=f'{v[1]:.8f}'
  else:ET.SubElement(w,kind,value=f'{v:.8f}')
 return a

def build(root,parts,header):
 root.mkdir(parents=True,exist_ok=False);shutil.copytree(parts,root/'parts')
 tracks={n:[curve(k,f/FRAMES) for f in range(FRAMES+1)] for n,k in zip(NAMES,KEYS)}
 canvas=ET.Element('canvas',version='1.2',width='320',height='360',fps='60',**{'view-box':'-45 90 45 -11','begin-time':'0f','end-time':'35f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
 ET.SubElement(canvas,'name').text='Emberfall / schowanie miecza / 0.6s — reverse for draw'
 ET.SubElement(canvas,'desc').text='Native animated control nodes; 2D Krita cutout preview; runtime uses exact curves with rigid arm IK and scabbard clipping.'
 defs=ET.SubElement(canvas,'defs')
 for name,values in tracks.items():animated(defs,'stow_'+name,'real',values)
 for prefix,values in [('mouth',MOUTH),('axis',AXIS)]:
  for axis,component in zip('rfz',values):ET.SubElement(defs,'real',id=f'scabbard_{prefix}_{axis}',value=f'{component:.9f}')
 # Portable cutout preview. Static legs/body keep foot anchors fixed.
 image(canvas,'leg',(-13,32,1,0));image(canvas,'leg',(1,32,15,0));image(canvas,'body',(-20,58,20,29));image(canvas,'face',(-9,81,9,59));image(canvas,'hair_front',(-10,84,10,61))
 image(canvas,'arm',(-27,54,-12,27))
 # Right arm and sword visibly follow the same hand path and insertion amount.
 arm=group(canvas,'Prawa dłoń / chwyt pochwy');image(arm,'arm',(12,54,27,27))
 translate=ET.SubElement(arm,'layer',type='translate',active='true',version='0.1',desc='Hand reach')
 offsets=[]
 for i in range(FRAMES+1):
  reach=tracks['reach'][i];insert=tracks['insertion'][i]
  target=(tracks['handR'][i]+tracks['handF'][i]*.45,tracks['handZ'][i]-tracks['handF'][i]*.15)
  offsets.append(((target[0]-20)*reach,(target[1]-33)*reach))
 animated(ET.SubElement(translate,'param',name='origin'),'preview_hand','vector',offsets)
 sword=group(canvas,'Miecz — rękojeść i ostrze')
 # A rigid full-size sword moves behind the scabbard sprite. Only occlusion,
 # never scaling, hides the blade during insertion in this cutout preview.
 l=ET.SubElement(sword,'layer',type='import',active='true',version='0.1',desc='Krita sword')
 value(l,'filename','string','parts/sword.png');value(l,'amount','real',1);value(l,'blend_method','integer',0);value(l,'gamma_adjust','real',1)
 corners={'tl':[],'br':[]}
 for i in range(FRAMES+1):
  reach=tracks['reach'][i];insert=tracks['insertion'][i];x=20+offsets[i][0];z=33+offsets[i][1]
  if insert>=.9999:x=MOUTH[0]-AXIS[0]*5.2+(MOUTH[1]-AXIS[1]*5.2)*.45;z=MOUTH[2]-AXIS[2]*5.2-(MOUTH[1]-AXIS[1]*5.2)*.15
  corners['tl'].append((x-6,z+37));corners['br'].append((x+6,z-9))
 for name,points in corners.items():animated(ET.SubElement(l,'param',name=name),'preview_'+name,'vector',points)
 turn=ET.SubElement(sword,'layer',type='rotate',active='true',version='0.1',desc='Rigid blade alignment')
 animated(ET.SubElement(turn,'param',name='origin'),'preview_pivot','vector',[(p[0]+6,p[1]-37) for p in corners['tl']])
 angles=[]
 for i in range(FRAMES+1):
  pitch=math.radians(tracks['pitch'][i]);r=AXIS[0]*tracks['alignment'][i];f=math.cos(pitch);z=math.sin(pitch)
  angles.append(math.degrees(math.atan2(-(r+f*.45),z-f*.15)))
 animated(ET.SubElement(turn,'param',name='amount'),'preview_angle','angle',angles)
 mouth=(MOUTH[0]+MOUTH[1]*.45,MOUTH[2]-MOUTH[1]*.15)
 angle=math.degrees(math.atan2(-(AXIS[0]+AXIS[1]*.45),AXIS[2]-AXIS[1]*.15))
 cover=group(canvas,'Pochwa — zasłania wsuwane ostrze');image(cover,'bracer',(mouth[0]-3,mouth[1]+37,mouth[0]+3,mouth[1]));rotate(cover,'scabbard_alignment',mouth,[angle]*(FRAMES+1))
 path=root/'sheathe.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
 export(path,header)
 return path

def export(path,header):
 root=ET.parse(path).getroot();tracks={}
 for n in NAMES:
  a=root.find(f"defs/animated[@id='stow_{n}']");assert a is not None
  points={int(w.get('time')[:-1]):float(w.find('real').get('value')) for w in a};assert set(points)==set(range(FRAMES+1));tracks[n]=points
 lines=['#pragma once','// Exact native Synfig control waypoints; tools/media/sword_stow_synfig.py.','namespace ember::sword_stow::data {','struct Key {float '+','.join(NAMES)+';};','inline constexpr Key keys[] = {']
 for f in range(FRAMES+1):lines.append('{'+','.join(f'{tracks[n][f]:.8f}f' for n in NAMES)+'},')
 lines+=['};','inline constexpr int Frames=37;','inline constexpr float Duration=.6f;']
 for prefix in ['mouth','axis']:
  values=[root.find(f"defs/real[@id='scabbard_{prefix}_{axis}']").get('value') for axis in 'rfz']
  lines.append(f'inline constexpr float {prefix.title()}[] = '+'{'+','.join(v+'f' for v in values)+'};')
 lines+=['}'];header.write_text('\n'.join(lines)+'\n',encoding='utf-8')

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--parts',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 path=build(a.root,a.parts,a.header)
 reverse=ET.parse(path)
 for node in reverse.getroot().iter('animated'):
  values=[list(w)[0] for w in node];values.reverse()
  for waypoint,value_node in zip(node,values):
   for old in list(waypoint):waypoint.remove(old)
   waypoint.append(value_node)
 draw=path.with_name('draw.sif');reverse.write(draw,encoding='utf-8',xml_declaration=True)
 for source in [path,draw]:
  r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(source),'-OutputRoot',str(a.root/'renders'),'-ExpectedFrames','36','-Threads','2','-RequireMotion'],capture_output=True,text=True)
  (a.root/(source.stem+'-render.log')).write_text(r.stdout+'\n'+r.stderr,encoding='utf-8');assert r.returncode==0,r.stderr
 (a.root/'receipt.json').write_text(json.dumps({'status':'PASS','project':str(path),'project_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'header_sha256':hashlib.sha256(a.header.read_bytes()).hexdigest(),'duration':.6,'rendered_frames':72,'reverse_for_draw':True,'source':'Synfig native animated value nodes'},indent=2),encoding='utf-8')
 print('Native Synfig stow/draw: 72 frames and exact C++ curves exported.')

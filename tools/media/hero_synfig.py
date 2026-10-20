"""Author Synfig 2D cutout motion and compile its exact waypoints for Emberfall.

Existing primary locomotion/attack poses remain the base. These authored tracks
add weight shifts, chest breathing, head counterbalance, arm follow-through and
delayed cloth motion. Runtime reads only constexpr data, never SIF or PNG frames.
"""
from pathlib import Path
import argparse, hashlib, json, math, shutil, subprocess, xml.etree.ElementTree as ET

CHANNELS=['spine_roll','spine_pitch','head_roll','arm_swing','cape_sway','cape_lift']
# Nine editable key poses per channel, degrees. First/last match for loops;
# action clips return to zero so the primary combat timing stays unchanged.
CLIPS={
 'idle':(120,True,[[0,.35,.65,.35,0,-.35,-.65,-.35,0],[-.4,0,.8,1.2,.6,0,-.6,-.8,-.4],[0,-.25,-.5,-.2,0,.2,.5,.25,0],[0,.3,.8,1.2,.6,0,-.5,-.6,0],[0,.7,1.2,.8,0,-.7,-1.2,-.8,0],[0,.3,.6,.7,.2,-.2,-.4,-.3,0]]),
 'walk':(60,True,[[0,1.5,2.2,1.4,0,-1.5,-2.2,-1.4,0],[-.6,.2,1.3,.2,-.6,.2,1.3,.2,-.6],[0,-1,-1.5,-1,0,1,1.5,1,0],[0,2.5,4,2.5,0,-2.5,-4,-2.5,0],[-1,1.3,3.5,3,1,-1.3,-3.5,-3,-1],[0,1,2.1,1,0,1,2.1,1,0]]),
 'run':(48,True,[[0,2.2,3.2,2.2,0,-2.2,-3.2,-2.2,0],[0,1.6,3,1.4,0,1.6,3,1.4,0],[0,-1.4,-2.3,-1.4,0,1.4,2.3,1.4,0],[0,4.5,7,4.5,0,-4.5,-7,-4.5,0],[-2,2,6,5,2,-2,-6,-5,-2],[0,2.4,4.5,2.4,0,2.4,4.5,2.4,0]]),
 'slash':(36,False,[[0,-.5,-1.5,-2,2.4,1.8,.5,-.2,0],[0,-1.3,-3,-2,3.5,2,.5,-.3,0],[0,.3,1,1,-1.5,-1,-.2,.1,0],[0,-.8,-2,-1,2.5,1.2,.3,0,0],[0,-1,-3,-4,1,5,2,-1,0],[0,-.5,-1,-2,2,4,2,.5,0]]),
 'heavy':(48,False,[[0,-.7,-2,-3,3,2.2,.5,-.3,0],[0,-2,-4.2,-3,4.5,3,.8,-.4,0],[0,.5,1.5,1.7,-1.8,-1.3,-.3,.2,0],[0,-1,-2.5,-1.8,3,1.8,.4,0,0],[0,-2,-4,-6,2,7,3,-1,0],[0,-1,-2,-3,3,6,3,.6,0]]),
 'roll':(42,False,[[0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0],[0,-2,-5,-7,-5,-2,2,1,0],[0,-1,-4,-6,-4,-1,3,1,0]])
}

def value(parent,name,kind,val):
 p=ET.SubElement(parent,'param',name=name);e=ET.SubElement(p,kind)
 if kind=='string':e.text=str(val)
 elif kind=='vector':ET.SubElement(e,'x').text=str(val[0]);ET.SubElement(e,'y').text=str(val[1])
 else:e.set('value',str(val))
 return e

def curve(keys,t):
 x=max(0,min(1,t))*8;i=min(int(x),7);f=x-i;f=f*f*(3-2*f)
 return keys[i]*(1-f)+keys[i+1]*f

def group(parent,name):
 g=ET.SubElement(parent,'layer',type='PasteCanvas',active='true',version='0.1',desc=name)
 value(g,'amount','real',1);value(g,'blend_method','integer',0);value(g,'origin','vector',(0,0))
 return ET.SubElement(ET.SubElement(g,'param',name='canvas'),'canvas')

def image(parent,name,rect):
 l=ET.SubElement(parent,'layer',type='import',active='true',version='0.1',desc=name)
 value(l,'amount','real',1);value(l,'blend_method','integer',0);value(l,'filename','string','parts/'+name+'.png')
 value(l,'tl','vector',(rect[0],rect[1]));value(l,'br','vector',(rect[2],rect[3]));value(l,'gamma_adjust','real',1);value(l,'interpolation','integer',1)

def rotate(parent,name,origin,samples):
 l=ET.SubElement(parent,'layer',type='rotate',active='true',version='0.1',desc='runtime:'+name)
 value(l,'origin','vector',origin)
 a=ET.SubElement(ET.SubElement(l,'param',name='amount'),'animated',type='angle')
 for f,n in enumerate(samples):
  w=ET.SubElement(a,'waypoint',time=f'{f}f',before='linear',after='linear');ET.SubElement(w,'angle',value=f'{n:.7f}')

def author(root,parts):
 root.mkdir(parents=True,exist_ok=False);shutil.copytree(parts,root/'parts')
 paths=[]
 for name,(frames,loop,keys) in CLIPS.items():
  tracks={c:[curve(k,f/frames) for f in range(frames+1)] for c,k in zip(CHANNELS,keys)}
  canvas=ET.Element('canvas',version='1.2',width='256',height='320',xres='2834.645669',yres='2834.645669',**{'view-box':'-1.6 2.3 1.6 -1.7','antialias':'2','fps':'60','begin-time':'0f','end-time':f'{frames-1}f','bgcolor':'0 0 0 0'})
  ET.SubElement(canvas,'name').text='Emberfall / Krita cutout / '+name
  ET.SubElement(canvas,'desc').text='Secondary motion tracks used by runtime; layered 2D cutout preview, original Krita parts. Primary game gait stays articulated independently.'
  # Cutout preview has a fixed foot anchor; both legs and arms articulate.
  cloth=group(canvas,'Peleryna / opóźnienie')
  image(cloth,'cape',(-.73,1.12,.73,-.9));rotate(cloth,'cape_sway',(0,1.05),tracks['cape_sway']);rotate(cloth,'cape_lift',(0,1.05),tracks['cape_lift'])
  for side in [-1,1]:
   leg=group(canvas,'Noga '+str(side));image(leg,'leg',(side*.22-.21,-.12,side*.22+.21,-1.36))
   swing=[curve([0,8,14,8,0,-8,-14,-8,0],f/frames)*(1 if side==1 else -1) for f in range(frames+1)] if name in ['walk','run'] else [0]*(frames+1)
   rotate(leg,'preview_leg'+str(side),(side*.22,-.12),swing)
  body=group(canvas,'Tułów / ciężar i oddech')
  for side in [-1,1]:
   arm=group(body,'Ramię '+str(side));image(arm,'arm',(side*.59-.22,1.04,side*.59+.22,-.26))
   if side==1 and name in ['slash','heavy']:image(arm,'sword',(.58,.8,.91,-.4))
   rotate(arm,'arm_swing' if side==1 else 'preview_arm_left',(side*.59,.96),[v*side for v in tracks['arm_swing']])
  image(body,'body',(-.56,1.16,.56,-.24))
  head=group(body,'Głowa / przeciwbalans');image(head,'face',(-.36,1.95,.36,1.12));image(head,'hair_front',(-.39,2.04,.39,1.23))
  rotate(head,'head_roll',(0,1.12),tracks['head_roll'])
  image(body,'scarf',(-.42,1.25,.42,.92))
  rotate(body,'spine_roll',(0,-.10),tracks['spine_roll']);rotate(body,'spine_pitch',(0,-.10),tracks['spine_pitch'])
  ET.indent(canvas);path=root/(name+'.sif');ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True);paths.append(path)
 return paths

def compile_tracks(projects,output):
 lines=['#pragma once','// Generated from native Synfig 1.4.5 SIF waypoints by tools/media/hero_synfig.py.','namespace ember::hero_secondary {','struct Key {float spineRoll,spinePitch,headRoll,armSwing,capeSway,capeLift;};','struct Clip {const Key* keys;int frames;float duration;bool loop;};']
 receipt=[]
 for p in projects:
  root=ET.parse(p).getroot();frames=int(root.get('end-time')[:-1])+1;tracks={}
  for l in root.iter('layer'):
   desc=l.get('desc','')
   if not desc.startswith('runtime:') or desc[8:] not in CHANNELS:continue
   animated=l.find("param[@name='amount']/animated")
   records={int(w.get('time')[:-1]):float(w.find('angle').get('value')) for w in animated}
   assert set(records)==set(range(frames+1));tracks[desc[8:]]=records
  assert set(tracks)==set(CHANNELS)
  name=p.stem;lines.append('inline constexpr Key '+name+'Keys[] = {')
  for i in range(frames+1):lines.append('{'+','.join(f'{tracks[c][i]:.7f}f' for c in CHANNELS)+'},')
  lines.append('};');lines.append(f'inline constexpr Clip {name} = {{{name}Keys,{frames+1},{frames/60:.8f}f,{str(CLIPS[name][1]).lower()}}};')
  receipt.append({'project':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'exported_samples':frames+1,'channels':CHANNELS})
 lines.append('}');output.parent.mkdir(parents=True,exist_ok=True);output.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 return receipt

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--parts',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--render',action='store_true');p.add_argument('--no-audio',action='store_true',required=True)
 a=p.parse_args();projects=author(a.root,a.parts);receipt=compile_tracks(projects,a.header)
 if a.render:
  for project in projects:
   r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(project),'-OutputRoot',str(a.root/'renders'),'-ExpectedFrames',str(CLIPS[project.stem][0]),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
   (a.root/(project.stem+'-render.log')).write_text(r.stdout+'\n'+r.stderr,encoding='utf-8')
   if r.returncode:raise RuntimeError('Synfig render failed: '+str(project))
 (a.root/'motion-receipt.json').write_text(json.dumps({'status':'PASS','source':'Synfig native rotate waypoints','runtime':'secondary motion layered on primary skeletal clips','clips':receipt,'header_sha256':hashlib.sha256(a.header.read_bytes()).hexdigest()},indent=2),encoding='utf-8')
 print('Synfig: '+str(len(projects))+' native projects; exact runtime curves exported'+('; all frames rendered' if a.render else ''))

"""Author six native Synfig contact gaits; compile actual saved waypoints.

Foot is in body-relative units (support is linear, swing eases with zero lift
at touchdown). The runtime scales this rig for anatomy, not elapsed frame rate.
"""
from pathlib import Path
import argparse,hashlib,json,math,subprocess,xml.etree.ElementTree as ET
from hero_synfig import value,group,image

FAMILIES=['gel','quadruped','humanoid','arachnid','serpent','elemental']
PARTS=['slime','wolf','bone','spider','scales','bark']
def author(root,parts,header,render):
 root.mkdir(parents=True,exist_ok=False);(root/'parts').mkdir()
 import shutil
 for part in PARTS:shutil.copy2(parts/(part+'.png'),root/'parts'/(part+'.png'))
 lines=['#pragma once','// Native Synfig waypoints. Regenerate with tools/media/creature_synfig.py.',
        'namespace creature_motion {','struct Key {float travel,lift,bob,sway;};']
 receipt=[]
 for family,part in zip(FAMILIES,PARTS):
  frames=60;support={'gel':.55,'quadruped':.63,'humanoid':.64,'arachnid':.68,'serpent':.75,'elemental':.7}[family]
  samples=[]
  for f in range(frames+1):
   t=(f/frames)%1;w=t*math.tau
   if t<=support:travel=1-2*t/support;lift=0
   else:
    v=(t-support)/(1-support);v2=v*v;v3=v2*v;velocity=-2/support*(1-support)
    travel=-(2*v3-3*v2+1)+(v3-2*v2+v)*velocity+(-2*v3+3*v2)+(v3-v2)*velocity
    lift=math.sin(math.pi*v)**2
   samples.append((travel,lift,math.cos(2*w)*.5,math.sin(w)))
  canvas=ET.Element('canvas',version='1.2',width='320',height='256',xres='2834.645669',yres='2834.645669',**{'view-box':'-2 1.6 2 -1.6','antialias':'2','fps':'60','begin-time':'0f','end-time':'59f','bgcolor':'0 0 0 0'})
  ET.SubElement(canvas,'name').text='Emberfall / '+family+' / contact rig'
  ET.SubElement(canvas,'desc').text='Native 2D control rig with original Krita material patches. Actual game anatomy uses these exact sampled origin tracks. No 3D authoring.'
  def translate(parent,name,coords):
   l=ET.SubElement(parent,'layer',type='translate',active='true',version='0.1',desc='runtime:'+name)
   anim=ET.SubElement(ET.SubElement(l,'param',name='origin'),'animated',type='vector')
   for f,(x,y) in enumerate(coords):
    v=ET.SubElement(ET.SubElement(anim,'waypoint',time=f'{f}f',before='linear',after='linear'),'vector')
    ET.SubElement(v,'x').text=f'{x:.7f}';ET.SubElement(v,'y').text=f'{y:.7f}'
  body=group(canvas,'Body balance / Krita material')
  image(body,part,(-.7,.7,.7,-.3));translate(body,'body',[(s[2],s[3]) for s in samples])
  foot=group(canvas,'Support and recovery / Krita material')
  image(foot,part,(-.25,-.6,.25,-1.1));translate(foot,'foot',[(s[0],s[1]) for s in samples])
  path=root/(family+'.sif');ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
  # Compile from the SAVED native source, independently of the author arrays.
  tracks={}
  for l in ET.parse(path).getroot().iter('layer'):
   if l.get('desc','').startswith('runtime:'):
    values=[]
    for w in l.find("param[@name='origin']/animated"):
     v=w.find('vector');values.append((float(v.findtext('x')),float(v.findtext('y'))))
    tracks[l.get('desc')[8:]]=values
  rows=[(*a,*b) for a,b in zip(tracks['foot'],tracks['body'])];assert len(rows)==61 and rows[0]==rows[-1]
  lines.append(f'inline constexpr Key {family}[] = {{')
  lines += ['{'+','.join(f'{v:.7f}f' for v in row)+'},' for row in rows];lines.append('};')
  if render:
   r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(root/'renders'),'-ExpectedFrames','60','-Threads','2','-RequireMotion'],capture_output=True,text=True)
   (root/(family+'-render.log')).write_text(r.stdout+'\n'+r.stderr,encoding='utf-8');assert r.returncode==0,(family,r.stderr)
  receipt.append({'family':family,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'samples':61,'contact_fraction':support})
 lines += ['inline Key sample(int family,float phase){','const Key* clips[]={gel,quadruped,humanoid,arachnid,serpent,elemental};',
 'float f=(phase-std::floor(phase))*60;int i=int(f);float t=f-i;auto a=clips[std::clamp(family,0,5)][i],b=clips[std::clamp(family,0,5)][i+1];',
 'return {a.travel+(b.travel-a.travel)*t,a.lift+(b.lift-a.lift)*t,a.bob+(b.bob-a.bob)*t,a.sway+(b.sway-a.sway)*t};}','}']
 header.parent.mkdir(parents=True,exist_ok=True);header.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 (root/'motion-receipt.json').write_text(json.dumps({'status':'PASS','source':'Synfig 1.4.5 native animated vector waypoints','clips':receipt,'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest()},indent=2),encoding='utf-8')
 print('PASS: six Synfig control rigs, 366 exact compiled samples'+(', 360 rendered PNGs' if render else ''))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--parts',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--render',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root,a.parts,a.header,a.render)

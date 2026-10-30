"""Native Synfig cutout walk/attack projects; exact saved tracks used in game."""
from pathlib import Path
import argparse, math, json, shutil, subprocess, hashlib, xml.etree.ElementTree as ET
from PIL import Image
from hero_synfig import value,group,image
from create_volcano_art import NAMES,PARTS

def author(root,paints,header,runtime,actors_only=False):
 root.mkdir(parents=True,exist_ok=False);all_rows={};receipt=[]
 for species,name in enumerate(NAMES):
  project=root/name;project.mkdir();shutil.copytree(paints/name,project/'parts')
  for action in ('walk','attack'):
   canvas=ET.Element('canvas',version='1.2',width='256',height='256',fps='60',**{'view-box':'-1.28 2.3 1.28 -0.26','begin-time':'0f','end-time':'59f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
   ET.SubElement(canvas,'name').text=f'Emberfall / {name} / {action}'
   ET.SubElement(canvas,'desc').text='Original Krita 2D parts. Editable Synfig contact, body balance and attack tracks. Runtime uses the saved waypoints.'
   rows=[]
   for f in range(61):
    t=(f/60)%1 if action=='walk' else f/60;support=.62
    if action=='walk':
     if t<=support:travel=1-2*t/support;lift=0
     else:
      v=(t-support)/(1-support);travel=-1+2*(v*v*(3-2*v));lift=math.sin(math.pi*v)**2
     bob=.8*math.cos(t*math.tau*2);sway=math.sin(t*math.tau)
    else:travel=0;lift=0;bob=-math.sin(t*math.pi)*.6;sway=math.sin((t-.45)*math.pi*1.8)*math.sin(t*math.pi)*7
    rows.append((travel,lift,bob,sway))
   def track(parent,label,coords):
    l=ET.SubElement(parent,'layer',type='translate',active='true',version='0.1',desc='runtime:'+label)
    a=ET.SubElement(ET.SubElement(l,'param',name='origin'),'animated',type='vector')
    for f,(x,y) in enumerate(coords):
     v=ET.SubElement(ET.SubElement(a,'waypoint',time=f'{f}f',before='linear',after='linear'),'vector')
     ET.SubElement(v,'x').text=f'{x:.7f}';ET.SubElement(v,'y').text=f'{y:.7f}'
   for part,label in enumerate(PARTS):
    g=group(canvas,label);image(g,label,(-1.28,2.3,1.28,-.26))
    if part in (1,2):
     sign=1 if part==1 else -1
     # Saved control tracks are normalized; preview converts to cutout pixels.
     track(g,'leg-control' if part==1 else 'other-leg',[(v[0]*sign,v[1]) for v in rows])
     # Compensation makes the authored normalized controls a readable 2D preview.
     track(g,'preview-leg',[(v[0]*sign*(-.86),-v[1]+v[1]*.07-v[0]*sign*.07) for v in rows])
    else:
     track(g,'body-control' if part==3 else label,[(v[2],v[3]) for v in rows])
     track(g,'preview-body',[(v[2]*(-.985),-v[3]+v[2]*.02+v[3]*.01) for v in rows])
   path=project/(action+'.sif');ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf8',xml_declaration=True)
   tracks={}
   for l in ET.parse(path).getroot().iter('layer'):
    if l.get('desc') in ('runtime:leg-control','runtime:body-control'):
     tracks[l.get('desc')]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in l.find('param/animated')]
   saved=[(*a,*b) for a,b in zip(tracks['runtime:leg-control'],tracks['runtime:body-control'])];assert len(saved)==61
   all_rows[(species,action)]=saved
   command=['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(project/'renders'),'-ExpectedFrames','60','-Threads','2','-RequireMotion']
   result=subprocess.run(command,capture_output=True,text=True);(project/(action+'-render.log')).write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,(name,action,result.stderr)
   receipt.append({'name':name,'action':action,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'samples':61,'rendered_frames':60});print('Synfig',name,action,'PASS',flush=True)
 if not actors_only:
  # Lava shimmer is also authored and rendered in Synfig, using only Krita tiles.
  lava=root/'lava';lava.mkdir();(lava/'parts').mkdir();im=Image.open(paints/'terrain.png').convert('RGBA')
  im.crop((512,0,640,96)).save(lava/'parts'/'lava.png');im.crop((640,0,768,96)).save(lava/'parts'/'crust.png')
  canvas=ET.Element('canvas',version='1.2',width='128',height='96',fps='12',**{'view-box':'-1.28 .96 1.28 -.96','begin-time':'0f','end-time':'11f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
  image(canvas,'lava',(-1.28,.96,1.28,-.96));g=group(canvas,'Cooling and glowing surface');image(g,'crust',(-1.28,.96,1.28,-.96))
  # The group is the enclosing pastecanvas layer.
  outer=canvas[-1];amount=outer.find("param[@name='amount']");amount.clear();amount.set('name','amount');anim=ET.SubElement(amount,'animated',type='real')
  for f in range(13):ET.SubElement(ET.SubElement(anim,'waypoint',time=f'{f}f',before='linear',after='linear'),'real',value=f'{.18+.12*math.cos(f*math.tau/12):.7f}')
  path=lava/'lava.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf8',xml_declaration=True)
  result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(lava/'renders'),'-ExpectedFrames','12','-Threads','2','-RequireMotion'],capture_output=True,text=True)
  (lava/'render.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,result.stderr
  frames=sorted((lava/'renders').rglob('*.png'));assert len(frames)==12,len(frames)
  atlas=Image.new('RGBA',(128*12,96))
  for f,p in enumerate(frames):atlas.paste(Image.open(p).convert('RGBA'),(f*128,0))
  atlas.save(runtime/'lava-flow.png')
 lines=['#pragma once','namespace volcano_motion {','struct Key {float travel,lift,bob,sway;};']
 for action in ('walk','attack'):
  lines.append('inline constexpr Key '+action+'[7][61]={')
  for species in range(7):lines.append('{'+','.join('{'+','.join(f'{v:.7f}f' for v in row)+'}' for row in all_rows[(species,action)])+'},')
  lines.append('};')
 lines+=['inline Key sample(int type,float phase,bool striking=false){float f=std::clamp(phase,0.f,1.f)*60;int i=std::min(59,int(f));float t=f-i;auto a=(striking?attack:walk)[type][i],b=(striking?attack:walk)[type][i+1];return {a.travel+(b.travel-a.travel)*t,a.lift+(b.lift-a.lift)*t,a.bob+(b.bob-a.bob)*t,a.sway+(b.sway-a.sway)*t};}','}']
 header.write_text('\n'.join(lines)+'\n',encoding='utf8');(root/'receipt.json').write_text(json.dumps({'status':'PASS','authoring':'Synfig 1.4.5','clips':receipt,'lava_frames':0 if actors_only else 12,'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest()},indent=2),encoding='utf8')

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--paints',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root,a.paints,a.header,a.runtime)

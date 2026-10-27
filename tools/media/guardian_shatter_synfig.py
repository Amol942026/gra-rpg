"""Native 2D Synfig fracture clip, compiled from its saved control waypoints."""
from pathlib import Path
import argparse,hashlib,json,math,shutil,subprocess,xml.etree.ElementTree as ET
from PIL import Image
from hero_synfig import group,image,rotate
from guardian_throw_synfig import vector

FRAMES=72
SCALE=76/96
REST=28
FIELDS=('x','y','height','angle','alpha')

def author(root,atlas,provenance,header):
 root.mkdir(parents=True,exist_ok=False);(root/'parts').mkdir()
 art=json.loads(provenance.read_text(encoding='utf-8'));assert art['status']=='PASS' and art['original_art']
 assert hashlib.sha256(atlas.read_bytes()).hexdigest()==art['png_sha256']
 with Image.open(atlas) as im:
  for i in range(6):im.crop((i%3*96,i//3*96,i%3*96+96,i//3*96+96)).save(root/'parts'/f'fragment{i}.png')
 ends=[(-51,-15),(3,-32),(56,-12),(-52,24),(-4,36),(48,25)]
 spins=[-95,73,118,-123,-62,96];peaks=[19,32,23,15,21,18]
 poses=[]
 for i,(poly,pivot) in enumerate(zip(art['polygons'],art['pivots'])):
  cx,cy=pivot
  def clearance(angle):
   r=math.radians(angle);return max((math.sin(r)*(x-cx)+math.cos(r)*(y-cy))*SCALE for x,y in poly)
  initial=REST-(cy-48)*SCALE-clearance(0);assert initial>=0
  rows=[]
  for frame in range(FRAMES+1):
   t=frame/60;age=max(0,t-.025);first=.38+.035*(i%3);second=.16+.02*(i%2);settle=first+second
   u=min(1,age/.65);travel=1-(1-u)**2
   x=(cx-48)*SCALE+(ends[i][0]-(cx-48)*SCALE)*travel;y=ends[i][1]*travel
   turn=min(1,age/settle);angle=spins[i]*(1-(1-turn)**2)
   if age<first:
    v=age/first;lift=initial*(1-v)+math.sin(v*math.pi)*peaks[i]
   elif age<settle:lift=math.sin((age-first)/second*math.pi)*peaks[i]*.23
   else:lift=0
   fade=min(1,max(0,(t-.75)/.45));alpha=1-fade*fade*(3-2*fade)
   rows.append((x,y,clearance(angle)+max(0,lift),angle,alpha))
  poses.append(rows)
 canvas=ET.Element('canvas',version='1.2',width='400',height='300',xres='2834.645669',yres='2834.645669',**{'view-box':'-100 100 100 -50','antialias':'2','fps':'60','begin-time':'0f','end-time':f'{FRAMES-1}f','bgcolor':'0 0 0 0'})
 ET.SubElement(canvas,'name').text='Emberfall / Guardian / boulder breaks into six pieces'
 ET.SubElement(canvas,'desc').text='Original native Krita cutouts. Assembled boulder at terrain contact, six independent arcs, spin, staggered ground contacts, small secondary bounces, settling and fade. Ground anchors and rotational clearance prevent floating or sinking fragments. 72 frames at 60 Hz.'
 for i,rows in enumerate(poses):
  cx,cy=art['pivots'][i];part=group(canvas,f'Granite fragment {i+1} / cutout')
  # PasteCanvas amount is animated so the native preview uses the same fade.
  amount=canvas[-1].find("param[@name='amount']");amount.clear();amount.set('name','amount');anim=ET.SubElement(amount,'animated',type='real')
  for frame,row in enumerate(rows):ET.SubElement(ET.SubElement(anim,'waypoint',time=f'{frame}f',before='linear',after='linear'),'real',value=f'{row[4]:.7f}')
  image(part,f'fragment{i}',(-cx*SCALE,cy*SCALE,(96-cx)*SCALE,-(96-cy)*SCALE))
  rotate(part,f'preview_fragment{i}',(0,0),[-r[3] for r in rows])
  move=ET.SubElement(part,'layer',type='translate',active='true',version='0.1',desc='cutout placement');vector(move,'origin',[(r[0],r[2]-r[1]) for r in rows])
  for track,indices in (('ground',(0,1)),('lift',(2,3)),('fade',(4,None))):
   control=group(canvas,f'Authority / fragment {i} / {track}')
   layer=ET.SubElement(control,'layer',type='translate',active='true',version='0.1',desc=f'fracture:{i}:{track}')
   vector(layer,'origin',[(r[indices[0]],r[indices[1]] if indices[1] is not None else 0) for r in rows])
 path=root/'guardian-shatter.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
 tracks={}
 for layer in ET.parse(path).getroot().iter('layer'):
  if layer.get('desc','').startswith('fracture:'):
   _,piece,track=layer.get('desc').split(':');tracks[int(piece),track]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in layer.find("param[@name='origin']/animated")]
 saved=[[(*tracks[i,'ground'][f],*tracks[i,'lift'][f],tracks[i,'fade'][f][0]) for f in range(FRAMES+1)] for i in range(6)]
 lines=['#pragma once','// Generated from saved native 2D Synfig fracture controls. No runtime file I/O.','namespace guardian_break {',f'inline constexpr int Pieces=6,Frames={FRAMES};',f'inline constexpr float Duration={FRAMES}/60.f,RestHeight={REST};','struct Pose {float x,y,height,angle,alpha;};','struct Pivot {float x,y;};','inline constexpr Pivot pivots[]={'+','.join('{'+f'{x}.f,{y}.f'+'}' for x,y in art['pivots'])+'};','inline constexpr Pose poses[Pieces][Frames+1]={']
 for rows in saved:lines+=['{']+['{'+','.join(f'{v:.7f}f' for v in row)+'},' for row in rows]+['},']
 lines+=['};','inline Pose sample(int piece,float progress){float f=std::clamp(progress,0.f,1.f)*Frames;int i=std::min(Frames-1,int(f));float t=f-i;auto a=poses[std::clamp(piece,0,Pieces-1)][i],b=poses[std::clamp(piece,0,Pieces-1)][i+1];return {'+','.join(f'a.{v}+(b.{v}-a.{v})*t' for v in FIELDS)+'};}','}']
 header.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(root/'renders'),'-ExpectedFrames',str(FRAMES),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
 (root/'render.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf-8');assert result.returncode==0,result.stderr
 receipt={'status':'PASS','source':'Synfig native 2D cutout, six original Krita fragments','frames':FRAMES,'samples_per_piece':FRAMES+1,'pieces':6,'duration':FRAMES/60,'master':str(root),'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest(),'atlas_sha256':art['png_sha256']}
 (root/'shatter-receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8');print('PASS: 72 Synfig fracture frames; 438 native control samples compiled',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--atlas',type=Path,required=True);p.add_argument('--provenance',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root,a.atlas,a.provenance,a.header)

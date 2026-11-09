"""Author, render and compile actual Synfig loops for all three spell cores."""
from pathlib import Path
import math,json,hashlib,xml.etree.ElementTree as ET,shutil
from PIL import Image
from hero_synfig import value,group,image,rotate
import synfig_cli
ROOT=Path('C:/AI2D/sprites/emberfall-spells-20261109')
REPO=Path(__file__).resolve().parents[2]
def main():
 out=ROOT/'animation-v2';out.mkdir(exist_ok=False);(out/'parts').mkdir();atlas=Image.open(ROOT/'krita/eldrin-three-elements.png').convert('RGBA');names=['fireball','ice-shard','lightning-orb'];rows=[];reports=[]
 synfig_cli.SYNFIG=Path('C:/AI2D/Tools/Synfig-1.4.5-spells/portable/bin/synfig.exe')
 for index,name in enumerate(names):
  atlas.crop((index*256,256,index*256+256,512)).save(out/'parts'/(name+'.png'))
  canvas=ET.Element('canvas',version='1.2',width='128',height='128',xres='2834.645669',yres='2834.645669',**{'view-box':'-1 1 1 -1','antialias':'2','fps':'60','begin-time':'0f','end-time':'59f','bgcolor':'0 0 0 0'})
  ET.SubElement(canvas,'name').text='Emberfall / '+name+' / elemental loop'
  layer=group(canvas,'Native Krita elemental core');image(layer,name,(-1,1,1,-1))
  angles=[];scales=[];alphas=[]
  for n in range(61):
   phase=n*math.tau/60;angles.append((math.sin(phase)*8 if index==1 else math.sin(phase)*13 if index==0 else math.sin(phase)*28));scales.append(1+(.06 if index==1 else .12)*math.sin(phase*2));alphas.append(.86+.14*math.cos(phase*2))
  rotate(layer,'rotation',(0,0),angles)
  zoom=ET.SubElement(layer,'layer',type='zoom',active='true',version='0.1',desc='runtime:scale');value(zoom,'center','vector',(0,0));a=ET.SubElement(ET.SubElement(zoom,'param',name='amount'),'animated',type='real')
  for n,s in enumerate(scales):w=ET.SubElement(a,'waypoint',time=f'{n}f',before='linear',after='linear');ET.SubElement(w,'real',value=str(math.log(s)))
  imported=layer.find('layer');imported.remove(imported.find("param[@name='amount']"));a=ET.SubElement(ET.SubElement(imported,'param',name='amount'),'animated',type='real')
  for n,alpha in enumerate(alphas):w=ET.SubElement(a,'waypoint',time=f'{n}f',before='linear',after='linear');ET.SubElement(w,'real',value=str(alpha))
  project=out/(name+'.sif');ET.indent(canvas);ET.ElementTree(canvas).write(project,encoding='utf-8',xml_declaration=True)
  # Compile back from the saved native project, not the in-memory recipe.
  saved=ET.parse(project).getroot();tracks={}
  for l in saved.iter('layer'):
   desc=l.get('desc','')
   if desc=='runtime:rotation':tracks['rotation']=[float(w.find('angle').get('value')) for w in l.find("param[@name='amount']/animated")]
   if desc=='runtime:scale':tracks['scale']=[math.exp(float(w.find('real').get('value'))) for w in l.find("param[@name='amount']/animated")]
   if l.get('type')=='import':tracks['alpha']=[float(w.find('real').get('value')) for w in l.find("param[@name='amount']/animated")]
  row=list(zip(tracks['scale'],tracks['rotation'],tracks['alpha']));assert len(row)==61 and all(abs(a-b)<1e-6 for a,b in zip(row[0],row[-1]));rows.append(row)
  reports.append(synfig_cli.render(project,output_root=out/'renders',threads=2,expected_frames=60,require_motion=True))
 header=['#pragma once','#include <algorithm>','#include <cmath>','namespace spell_motion {','struct Key {float scale,rotation,alpha;};','inline constexpr Key Keys[3][61]={']
 for row in rows:header.append('{'+','.join('{'+','.join(f'{v:.7f}f' for v in key)+'}' for key in row)+'},')
 header+=['};','inline Key sample(int spell,float age){float frame=std::fmod(std::max(0.f,age),1.f)*60;int i=std::min(59,int(frame));float t=frame-i;auto a=Keys[std::clamp(spell-1,0,2)][i],b=Keys[std::clamp(spell-1,0,2)][i+1];return {a.scale+(b.scale-a.scale)*t,a.rotation+(b.rotation-a.rotation)*t,a.alpha+(b.alpha-a.alpha)*t};}','}']
 target=REPO/'src/generated/spell_motion.hpp';target.write_text('\n'.join(header)+'\n',encoding='utf-8')
 (out/'render-receipt.json').write_text(json.dumps({'status':'PASS','native_program':str(synfig_cli.SYNFIG),'renders':reports,'fps':60,'frames_per_spell':60,'runtime_header':str(target),'runtime_header_sha256':hashlib.sha256(target.read_bytes()).hexdigest()},indent=2),encoding='utf-8')
 shutil.copy2(ROOT/'krita/eldrin-three-elements.png',REPO/'assets/sprites/spells.png');print('PASS: three native SIF loops / 180 rendered frames, compiled saved animation tracks')
if __name__=='__main__':
 import sys
 if sys.argv[1:]!=['--no-audio']:raise SystemExit(3)
 main()

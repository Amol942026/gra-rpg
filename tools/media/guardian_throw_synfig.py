"""Author the native two-handed 2D cutout, then compile the saved SIF controls."""
from pathlib import Path
import argparse,hashlib,json,math,shutil,subprocess,xml.etree.ElementTree as ET
from hero_synfig import group,image,rotate

FRAMES,RELEASE=114,90
FIELDS='handSide handForward handUp elbowSide elbowForward elbowUp rockForward rockUp bodyForward bodyUp'.split()
TRACKS={'hand':('handForward','handUp'),'elbow':('elbowForward','elbowUp'),'grip':('handSide','elbowSide'),'rock':('rockForward','rockUp'),'body':('bodyForward','bodyUp')}

def vector(layer,name,coords):
 anim=ET.SubElement(ET.SubElement(layer,'param',name=name),'animated',type='vector')
 for frame,(x,y) in enumerate(coords):
  v=ET.SubElement(ET.SubElement(anim,'waypoint',time=f'{frame}f',before='linear',after='linear'),'vector')
  ET.SubElement(v,'x').text=f'{x:.7f}';ET.SubElement(v,'y').text=f'{y:.7f}'

def author(root,stone,header):
 root.mkdir(parents=True,exist_ok=False);(root/'parts').mkdir();shutil.copy2(stone,root/'parts/stone.png')
 # frame, grip half-width, hand f/z, boulder f/z, torso f/z.
 keys=[(0,27,5,33,36,8,0,0),(16,20,34,12,36,14,12,-16),
       (29,20,43,20,44,28,12,-14),(53,20,39,64,40,76,3,-3),
       (61,19,32,85,36,102,3,0),(70,19,8,90,8,106,0,0),(80,19,5,91,5,107,-2,1),
       (90,19,24,86,24,102,4,-2),(98,22,35,52,24,102,7,-7),
       (114,27,5,33,36,8,0,0)]
 rows=[]
 for frame in range(FRAMES+1):
  i=next(i for i in range(len(keys)-1) if keys[i+1][0]>=frame);a,b=keys[i:i+2]
  t=(frame-a[0])/(b[0]-a[0]);t=t*t*(3-2*t)
  hs,hf,hz,rf,rz,bf,bz=[a[j]+(b[j]-a[j])*t for j in range(1,8)]
  es=(23+hs)/2;link=math.sqrt(22**2-((hs-23)/2)**2)
  dx,dy=hf-bf,hz-(53+bz);d=math.hypot(dx,dy);h=math.sqrt(max(0,link**2-d*d/4))
  # Pick the same flexion side as the resting arm: shoulder -> elbow -> hand
  # has positive signed area in the forward/up plane. The opposite IK branch
  # bends behind the hinge and flips through a straight arm during the blend.
  ef,ez=(hf+bf)/2+dy/d*h,(hz+53+bz)/2-dx/d*h
  blend=min(1,frame/24,(FRAMES-frame)/24);blend=blend*blend*(3-2*blend)
  es=25+(es-25)*blend;ef=1+(ef-1)*blend;ez=43+(ez-43)*blend
  rows.append(dict(zip(FIELDS,(hs,hf,hz,es,ef,ez,rf,rz,bf,bz))))
 canvas=ET.Element('canvas',version='1.2',width='480',height='400',xres='2834.645669',yres='2834.645669',**{'view-box':'-65 145 145 -15','antialias':'2','fps':'60','begin-time':'0f','end-time':f'{FRAMES-1}f','bgcolor':'0 0 0 0'})
 ET.SubElement(canvas,'name').text='Emberfall / Stone Guardian / uproot and two-handed overhead throw'
 ET.SubElement(canvas,'desc').text='Original Krita 2D cutouts. Anatomical elbow flexion matches the idle hinge side through crouch, lift, overhead throw and follow-through. Crouch 16f, tear 29f, lift 53f, overhead 70-80f, release 90f, rest 114f. Mirrored hands and torso controls drive the existing game actor; preview flight is illustrative, runtime target is dynamic.'
 def project(r,f,z):return r*.85+f*.55,z-f*.25
 def cutout(name,size,centers):
  part=group(canvas,name);image(part,'stone',(-size[0]/2,size[1]/2,size[0]/2,-size[1]/2))
  transform=ET.SubElement(part,'layer',type='translate',active='true',version='0.1',desc=name+' / placement');vector(transform,'origin',centers)
 def segment(name,starts,ends,width):
  lengths=[math.dist(a,b) for a,b in zip(starts,ends)];length=max(lengths)
  part=group(canvas,name);image(part,'stone',(-width/2,length/2,width/2,-length/2))
  scale=ET.SubElement(part,'layer',type='stretch',active='true',version='0.1',desc='link length');vector(scale,'amount',[(1,d/length) for d in lengths])
  rotate(part,'preview_'+name,(0,0),[-math.degrees(math.atan2(b[0]-a[0],b[1]-a[1])) for a,b in zip(starts,ends)])
  transform=ET.SubElement(part,'layer',type='translate',active='true',version='0.1',desc='joint placement');vector(transform,'origin',[((a[0]+b[0])/2,(a[1]+b[1])/2) for a,b in zip(starts,ends)])
 for side in [-1,1]:
  foot=[project(side*10,3,3) for p in rows];hip=[project(side*10,p['bodyForward']*.35,31+p['bodyUp']) for p in rows]
  segment('leg '+str(side),foot,hip,14)
 cutout('torso',(49,40),[project(0,p['bodyForward'],45+p['bodyUp']) for p in rows])
 cutout('head',(34,31),[project(0,p['bodyForward']*.8,67+p['bodyUp']) for p in rows])
 for side in [-1,1]:
  shoulder=[project(side*23,p['bodyForward'],53+p['bodyUp']) for p in rows]
  elbow=[project(side*p['elbowSide'],p['elbowForward'],p['elbowUp']) for p in rows]
  hand=[project(side*p['handSide'],p['handForward'],p['handUp']) for p in rows]
  segment('upper arm '+str(side),shoulder,elbow,15);segment('forearm '+str(side),elbow,hand,13);cutout('hand '+str(side),(17,17),hand)
 flight=[]
 for frame,p in enumerate(rows):
  if frame<=RELEASE:flight.append(project(0,p['rockForward'],p['rockUp']))
  else:
   t=(frame-RELEASE)/(FRAMES-RELEASE);flight.append(project(0,24+t*140,102*(1-t)+math.sin(t*math.pi)*25))
 cutout('large uprooted boulder',(55,55),flight)
 for name,fields in TRACKS.items():
  part=group(canvas,'Authority / '+name)
  layer=ET.SubElement(part,'layer',type='translate',active='true',version='0.1',desc='runtime:'+name);vector(layer,'origin',[(p[fields[0]],p[fields[1]]) for p in rows])
 path=root/'guardian-throw.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
 tracks={}
 for layer in ET.parse(path).getroot().iter('layer'):
  if layer.get('desc','')[8:] in TRACKS and layer.get('desc','').startswith('runtime:'):
   tracks[layer.get('desc')[8:]]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in layer.find("param[@name='origin']/animated")]
 saved=[]
 for frame in range(FRAMES+1):
  p={field:tracks[track][frame][i] for track,fields in TRACKS.items() for i,field in enumerate(fields)};saved.append(tuple(p[f] for f in FIELDS))
 assert saved[0]==saved[-1]
 lines=['#pragma once','// Compiled from saved native 2D Synfig controls. Mirrored two-hand grip; no runtime I/O.','namespace guardian_motion {','struct Pose {float '+','.join(FIELDS)+';};','inline constexpr Pose poses[]={']
 lines+=['{'+','.join(f'{v:.7f}f' for v in row)+'},' for row in saved]
 lerps=','.join(f'a.{f}+(b.{f}-a.{f})*t' for f in FIELDS)
 lines+=['};',f'inline Pose sample(float progress){{float f=std::clamp(progress,0.f,1.f)*{FRAMES};int i=std::min({FRAMES-1},int(f));float t=f-i;auto a=poses[i],b=poses[i+1];return {{{lerps}}};}}',f'inline constexpr int Frames={FRAMES},ReleaseFrame={RELEASE};',f'inline constexpr float Windup={RELEASE}/60.f,Recovery=({FRAMES}-{RELEASE})/60.f;',f'inline constexpr Pose Release=poses[{RELEASE}];','}']
 header.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(root/'renders'),'-ExpectedFrames',str(FRAMES),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
 (root/'render.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf-8');assert result.returncode==0,result.stderr
 (root/'motion-receipt.json').write_text(json.dumps({'status':'PASS','source':'Synfig 1.4.5 native 2D mirrored cutout and controls','samples':FRAMES+1,'frames':FRAMES,'release_frame':RELEASE,'fields':FIELDS,'tracks':TRACKS,'master':str(root),'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest()},indent=2),encoding='utf-8')
 print(f'PASS: {FRAMES} Synfig frames, {FRAMES+1} saved control samples',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--stone',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root,a.stone,a.header)

"""Native Synfig motion for the revised, Krita-painted Caldera."""
import argparse,math,json,shutil,subprocess,xml.etree.ElementTree as ET,hashlib
from pathlib import Path
from PIL import Image
from hero_synfig import group,image,value
from volcano_synfig import author

def translate(canvas,points):
 l=ET.SubElement(canvas,'layer',type='translate',active='true',version='0.1',desc='Loop motion')
 a=ET.SubElement(ET.SubElement(l,'param',name='origin'),'animated',type='vector')
 for f,(x,y) in enumerate(points):
  v=ET.SubElement(ET.SubElement(a,'waypoint',time=f'{f}f',before='linear',after='linear'),'vector');ET.SubElement(v,'x').text=str(x);ET.SubElement(v,'y').text=str(y)

def effect(root,paints,runtime,name,frames,cell,columns):
 project=root/name;project.mkdir();(project/'parts').mkdir();shutil.copy2(paints/('lava-surface.png' if name=='lava' else 'steam.png'),project/'parts'/(name+'.png'))
 c=ET.Element('canvas',version='1.2',width=str(cell),height=str(cell),fps='24',**{'view-box':'-1.28 1.28 1.28 -1.28','begin-time':'0f','end-time':f'{frames-1}f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
 ET.SubElement(c,'name').text='Emberfall / painted Caldera / '+name
 if name=='lava':
  image(c,name,(-1.28,1.28,1.28,-1.28));g=group(c,'Slow molten shimmer')
  for x in (-2.56,0,2.56):
   for y in (-2.56,0,2.56):image(g,name,(-1.28+x,1.28+y,1.28+x,-1.28+y))
  c[-1].find("param[@name='amount']/real").set('value','.24')
  translate(g,[(.025*math.sin(f*math.tau/frames),.025*(1-math.cos(f*math.tau/frames))) for f in range(frames+1)])
 else:
  g=group(c,'Rising sulphur vapour');image(g,name,(-1.1,.85,1.1,-1.35))
  translate(g,[(.08*math.sin(f*math.tau/frames),f/frames*.8) for f in range(frames+1)])
  p=c[-1].find("param[@name='amount']");p.clear();p.set('name','amount');a=ET.SubElement(p,'animated',type='real')
  for f in range(frames+1):ET.SubElement(ET.SubElement(a,'waypoint',time=f'{f}f',before='linear',after='linear'),'real',value=str(math.sin(f*math.pi/frames)**2))
 path=project/(name+'.sif');ET.indent(c);ET.ElementTree(c).write(path,encoding='utf8',xml_declaration=True)
 r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(project/'renders'),'-ExpectedFrames',str(frames),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
 (project/'render.log').write_text(r.stdout+'\n'+r.stderr,encoding='utf8');assert r.returncode==0,(name,r.stdout,r.stderr)
 paths=sorted((project/'renders').rglob('frame_*.png'));assert len(paths)==frames
 atlas=Image.new('RGBA',(columns*cell,math.ceil(frames/columns)*cell));hashes=set()
 for i,p in enumerate(paths):
  im=Image.open(p).convert('RGBA');assert im.size==(cell,cell);hashes.add(hashlib.sha256(im.tobytes()).hexdigest());atlas.paste(im,(i%columns*cell,i//columns*cell))
 assert len(hashes)>10;atlas.save(runtime/(name+'-flow.png'));print('PASS native Synfig',name,frames,'frames',flush=True)
 return {'name':name,'frames':frames,'cell':cell,'source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'unique_frames':len(hashes)}

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--paints',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--header',type=Path,default=Path('src/generated/volcano_motion.hpp'));p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 author(a.root,a.paints,a.header,a.runtime,actors_only=True)
 fx=[effect(a.root,a.paints,a.runtime,'lava',60,256,8),effect(a.root,a.paints,a.runtime,'steam',36,128,6)]
 (a.root/'effect-receipt.json').write_text(json.dumps({'status':'PASS','effects':fx},indent=2),encoding='utf8')

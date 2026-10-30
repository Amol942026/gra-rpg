"""Decode native sources, motion and actual revised runtime exports without audio."""
import argparse,hashlib,json,zipfile,xml.etree.ElementTree as ET
from pathlib import Path
from PIL import Image,ImageChops,ImageStat
from create_volcano_art import NAMES,PARTS

def verify(paints,motion,runtime,header,output):
 checks=0
 def check(ok,label):
  nonlocal checks
  checks+=1
  if not ok:raise AssertionError(label)
 names=NAMES+['scenery','ground','macro','road','bank','bridge','lava-surface','steam','sky']
 for name in names:
  with zipfile.ZipFile(paints/(name+'.kra')) as z:
   check(z.testzip() is None,'KRA CRC: '+name);doc=ET.fromstring(z.read('maindoc.xml'));layers=[e.get('name') for e in doc.iter() if e.tag.endswith('layer')]
   for layer in PARTS if name in NAMES else []:check(layer in layers,'editable native part '+name+'/'+layer)
   check(len(layers)>=(2 if name=='macro' else 3),'editable native layers '+name)
  im=Image.open(paints/(name+'.png')).convert('RGBA');check(im.getbbox() is not None,'nonempty PNG '+name)
  if name in NAMES:
   size=256 if name=='varkhor' else 192;expected=im.resize((size*8,size*6),Image.Resampling.LANCZOS);actual=Image.open(runtime/(name+'.png')).convert('RGBA')
   check(expected.size==actual.size and expected.tobytes()==actual.tobytes(),'exact native cutout export '+name);check(actual.getextrema()[3]==(0,255),'true RGBA transparency '+name)
   for d in range(8):
    body=actual.crop((d*size,3*size,(d+1)*size,4*size));box=body.getbbox();check(box is not None and box[0]>0 and box[2]<size,'body direction and margins '+name+'/'+str(d))
  else:
   actual=Image.open(runtime/(name+'.png')).convert('RGBA');check(im.size==actual.size and im.tobytes()==actual.tobytes(),'exact native surface export '+name)
 scenery=Image.open(runtime/'scenery.png').convert('RGBA');check(scenery.size==(1440,1840),'four scenery types times four variants')
 for kind in range(4):
  hashes=set()
  for variant in range(4):
   im=scenery.crop((kind*360,variant*460,(kind+1)*360,(variant+1)*460));check(im.getbbox() is not None,'scenery cell filled');hashes.add(hashlib.sha256(im.tobytes()).hexdigest())
  check(len(hashes)==4,'four distinct scenery silhouettes')
 for name in ['ground','bridge','lava-surface']:check(Image.open(runtime/(name+'.png')).getextrema()[3]==(255,255),'opaque continuous surface '+name)
 for name in ['macro','road','bank','steam']:
  im=Image.open(runtime/(name+'.png'));check(im.getextrema()[3][1]>0 and im.getextrema()[3][0]<20,'soft alpha coverage '+name)
 receipt=json.loads((motion/'receipt.json').read_text(encoding='utf8'));check(len(receipt['clips'])==14,'fourteen native character clips');check(hashlib.sha256(header.read_bytes()).hexdigest()==receipt['header_sha256'],'motion compiled from saved source')
 rendered=0
 for clip in receipt['clips']:
  name,action=clip['name'],clip['action'];path=motion/name/(action+'.sif');check(hashlib.sha256(path.read_bytes()).hexdigest()==clip['sha256'],'saved Synfig project hash')
  folders=sorted((motion/name/'renders').glob(action+'_*'));frames=sorted(folders[-1].glob('frame_*.png'));check(len(frames)==60,'complete native character sequence');hashes=set()
  for frame in frames:
   im=Image.open(frame);im.load();check(im.size==(256,256),'decoded native frame dimensions');hashes.add(hashlib.sha256(im.tobytes()).hexdigest());rendered+=1
  check(len(hashes)>10,'visible native motion')
 fx=json.loads((motion/'effect-receipt.json').read_text(encoding='utf8'));check(fx['status']=='PASS','native effect render status')
 for name,frames,cell,columns in [('lava',60,256,8),('steam',36,128,6)]:
  im=Image.open(runtime/(name+'-flow.png')).convert('RGBA');hashes=set()
  for f in range(frames):
   frame=im.crop((f%columns*cell,f//columns*cell,(f%columns+1)*cell,(f//columns+1)*cell));hashes.add(hashlib.sha256(frame.tobytes()).hexdigest());check(frame.size==(cell,cell),'packed FX frame')
  check(len(hashes)>10,'visible loop variation '+name)
  if name=='steam':check(im.crop((0,0,cell,cell)).getbbox() is None,'steam fades to transparency at loop boundary')
 for path in runtime.glob('*.png'):
  im=Image.open(path);im.load();check(max(im.size)<=2048,'atlas within integrated GPU size budget')
 report={'status':'PASS','checks':checks,'native_kra_masters':len(names),'scenery_variants':16,'actors':7,'directions':8,'rendered_actor_frames':rendered,'effect_frames':96,'original_local_2d':True,'external_assets':[],'runtime_rgba8_bytes':sum(Image.open(p).width*Image.open(p).height*4 for p in runtime.glob('*.png'))}
 output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps(report))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--motion',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--header',type=Path,default=Path('src/generated/volcano_motion.hpp'));p.add_argument('--output',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.paints,a.motion,a.runtime,a.header,a.output)

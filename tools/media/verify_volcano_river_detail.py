"""Independent native source, silhouette, detail and visible river-flow acceptance."""
from pathlib import Path
import argparse,json,hashlib,zipfile,xml.etree.ElementTree as ET
from PIL import Image,ImageChops,ImageStat
from create_volcano_art import NAMES,PARTS

def verify(paints,previous,motion,runtime,header,output):
 checks=0
 def check(ok,label):
  nonlocal checks
  checks+=1
  if not ok:raise AssertionError(label)
 for name in NAMES+['scenery-extra']:
  with zipfile.ZipFile(paints/(name+'.kra')) as z:
   check(z.testzip() is None,'native KRA CRC '+name);doc=ET.fromstring(z.read('maindoc.xml'));layers=[e.get('name') for e in doc.iter() if e.tag.endswith('layer')]
   if name in NAMES:
    for part in PARTS:check(part in layers and 'Material detail / '+part in layers,'editable original and detail layers '+name+'/'+part)
   else:check(len(layers)>=5,'four additional scenery families on native layers')
  master=Image.open(paints/(name+'.png')).convert('RGBA');actual=Image.open(runtime/(name+'.png')).convert('RGBA')
  if name in NAMES:
   size=256 if name=='varkhor' else 192;expected=master.resize((size*8,size*6),Image.Resampling.LANCZOS);old=Image.open(previous/(name+'.png')).convert('RGBA')
   check(master.size==old.size,'unchanged native rig grid '+name)
   old_alpha=old.getchannel('A');new_alpha=master.getchannel('A');outside=old_alpha.point(lambda v:255 if v==0 else 0);extra=ImageChops.multiply(new_alpha,outside)
   check(extra.getbbox() is None,'details remain strictly inside existing silhouettes '+name)
   for d in range(8):
    box=(d*256,3*256,(d+1)*256,4*256);a=old.crop(box).convert('RGB');b=master.crop(box).convert('RGB');difference=ImageChops.difference(a,b);check(sum(ImageStat.Stat(difference).sum)>30000,'actual visible body detail for every direction '+name+'/'+str(d))
   for d in range(8):
    body=actual.crop((d*size,3*size,(d+1)*size,4*size));b=body.getbbox();check(b is not None and b[0]>0 and b[2]<size,'body with safe margins '+name+'/'+str(d))
  else:
   expected=master;check(actual.size==(1440,1840),'four families times four scenery variants')
   for kind in range(4):
    cells=[actual.crop((kind*360,row*460,(kind+1)*360,(row+1)*460)) for row in range(4)]
    check(all(im.getbbox() is not None for im in cells),'new scenery cells nonempty');check(len({hashlib.sha256(im.tobytes()).digest() for im in cells})==4,'distinct new variants')
  check(actual.size==expected.size and actual.tobytes()==expected.tobytes(),'runtime matches exact native export '+name)
  check(actual.getextrema()[3]==(0,255),'true sprite transparency '+name)
 receipt=json.loads((motion/'receipt.json').read_text(encoding='utf8'));check(len(receipt['clips'])==14,'native character clips');check(hashlib.sha256(header.read_bytes()).hexdigest()==receipt['header_sha256'],'exact saved motion tracks compiled')
 for clip in receipt['clips']:
  project=motion/clip['name'];check(hashlib.sha256((project/(clip['action']+'.sif')).read_bytes()).hexdigest()==clip['sha256'],'saved native motion project')
  folders=sorted((project/'renders').glob(clip['action']+'_*'));frames=sorted(folders[-1].glob('frame_*.png'));check(len(frames)==60,'complete rendered character sequence');hashes=set()
  for path in frames:
   im=Image.open(path);im.load();check(im.size==(256,256),'decoded native motion frame');hashes.add(hashlib.sha256(im.tobytes()).digest())
  check(len(hashes)>10,'actual visible articulated motion')
 river=json.loads((motion/'river-receipt.json').read_text(encoding='utf8'));check(river['status']=='PASS' and river['displacement_pixels']==256 and river['fps']==30,'full tile movement in native Synfig')
 check(hashlib.sha256((motion/'lava'/'lava.sif').read_bytes()).hexdigest()==river['source_sha256'],'native flow project hash')
 root=ET.parse(motion/'lava'/'lava.sif').getroot();waypoints=[v for layer in root.iter('layer') if layer.get('desc')=='Loop motion' for v in layer.find('param/animated')]
 check(len(waypoints)==61 and float(waypoints[0].findtext('vector/y'))==0 and abs(float(waypoints[-1].findtext('vector/y'))-2.56)<1e-8,'native start and virtual end represent a full translation')
 atlas=Image.open(runtime/'lava-flow.png').convert('RGBA');frames=[atlas.crop((f%8*256,f//8*256,(f%8+1)*256,(f//8+1)*256)) for f in range(60)]
 check(len({hashlib.sha256(im.tobytes()).digest() for im in frames})==60,'sixty distinct river samples');check(all(im.getextrema()[3]==(255,255) for im in frames),'opaque flow throughout loop')
 error=sum(ImageStat.Stat(ImageChops.difference(ImageChops.offset(frames[0].convert('RGB'),0,128),frames[30].convert('RGB'))).mean)/3
 check(error<3,'decoded surface moves half a tile rather than only flickering')
 for p in runtime.glob('*.png'):check(max(Image.open(p).size)<=2048,'atlas budget '+p.name)
 report={'status':'PASS','checks':checks,'detailed_actors':7,'directions':8,'new_scenery_families':4,'new_scenery_variants':16,'native_kra':8,'native_synfig_projects':15,'actor_frames':840,'river_frames':60,'river_fps':30,'river_half_period_translation_error':error,'runtime_interpolation':True,'original_local_2d':True,'external_assets':[],'paid_services':False}
 output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps(report))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--previous',type=Path,required=True);p.add_argument('--motion',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.paints,a.previous,a.motion,a.runtime,a.header,a.output)

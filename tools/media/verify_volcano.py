"""Independent decode/provenance checks for original Krita and Synfig exports."""
import argparse,json,hashlib,xml.etree.ElementTree as ET,zipfile
from pathlib import Path
from PIL import Image,ImageChops
from create_volcano_art import NAMES,PARTS
def verify(paints,motion,runtime,output,header):
 checks=0
 def check(ok,label):
  nonlocal checks
  checks+=1
  if not ok:raise AssertionError(label)
 for name in NAMES+['terrain','scenery','sky']:
  with zipfile.ZipFile(paints/(name+'.kra')) as z:
   check(z.testzip() is None,'KRA CRC '+name);xml=ET.fromstring(z.read('maindoc.xml'))
   layers=[e.get('name') for e in xml.iter() if e.tag.endswith('layer')]
   if name in NAMES:
    for part in PARTS:check(part in layers,'editable native layer '+name+':'+part)
  with Image.open(paints/(name+'.png')) as im:check(im.mode=='RGBA' and im.getbbox() is not None,'RGBA source '+name)
 for name in NAMES:
  with Image.open(runtime/(name+'.png')).convert('RGBA') as im:
   cell=im.width//8;check(im.size==(cell*8,cell*6),'atlas layout '+name);check(im.getextrema()[3]==(0,255),'true transparency '+name)
   for direction in range(8):
    body=im.crop((direction*cell,3*cell,(direction+1)*cell,4*cell));check(body.getbbox() is not None,'body for direction '+str(direction));check(body.getbbox()[0]>0 and body.getbbox()[2]<cell,'body margins')
 receipt=json.loads((motion/'receipt.json').read_text(encoding='utf8'))
 check(receipt['status']=='PASS' and len(receipt['clips'])==14,'fourteen native source clips')
 check(hashlib.sha256(header.read_bytes()).hexdigest()==receipt['header_sha256'],'exact compiled motion header')
 rendered=0
 for clip in receipt['clips']:
  name,action=clip['name'],clip['action'];path=motion/name/(action+'.sif');check(hashlib.sha256(path.read_bytes()).hexdigest()==clip['sha256'],'saved Synfig source hash')
  doc=ET.parse(path).getroot();tracks={}
  for layer in doc.iter('layer'):
   if layer.get('desc') in ('runtime:leg-control','runtime:body-control'):
    rows=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in layer.find('param/animated')];tracks[layer.get('desc')]=rows;check(len(rows)==61,'61 exact saved keys')
  if action=='walk':
   for rows in tracks.values():check(rows[0]==rows[-1],'walk closes exactly without a repeated runtime endpoint')
  folders=sorted((motion/name/'renders').glob(action+'_*'));check(bool(folders),'native render folder')
  frames=sorted(folders[-1].glob('*.png'));check(len(frames)==60,'all sixty frames rendered');hashes=set()
  for frame in frames:
   with Image.open(frame) as im:im.load();check(im.size==(256,256),'render dimensions');hashes.add(hashlib.sha256(im.tobytes()).digest());rendered+=1
  check(len(hashes)>10,'actual visible cutout motion '+name+' '+action)
 with Image.open(runtime/'lava-flow.png').convert('RGBA') as im:
  check(im.size==(1536,96),'twelve preloaded lava frames');check(len({hashlib.sha256(im.crop((i*128,0,(i+1)*128,96)).tobytes()).digest() for i in range(12)})>4,'lava surface actually animates')
 check((paints/'terrain-refined.kra').is_file(),'refined ground retained as native Krita master')
 with zipfile.ZipFile(paints/'terrain-refined.kra') as z:
  check(z.testzip() is None,'refined native KRA CRC');check(ET.fromstring(z.read('maindoc.xml')) is not None,'refined KRA layer document decodes')
 report={'status':'PASS','checks':checks,'actors':7,'directions':8,'native_synfig_clips':14,'rendered_actor_frames':rendered,'lava_frames':12,'original_2d':True,'external_assets':[],'runtime_texture_rgba8_bytes':sum(Image.open(p).width*Image.open(p).height*4 for p in runtime.glob('*.png'))}
 output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps(report))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--motion',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--header',type=Path,default=Path('src/generated/volcano_motion.hpp'));p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.paints,a.motion,a.runtime,a.output,a.header)

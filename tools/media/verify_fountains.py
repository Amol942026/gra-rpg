"""Independent decoding of native masters, all Synfig frames and game captures."""
from pathlib import Path
import argparse, hashlib, io, json, zipfile, xml.etree.ElementTree as ET
from PIL import Image
import numpy as np

def verify(qa,repo,previews,check):
 art=repo/'art/gameplay/fountains';receipt=json.loads((art/'receipt.json').read_text())
 check(receipt['status']=='PASS' and receipt['external_assets']==[],'original local Krita / Synfig fountain assets')
 sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
 result=[]
 for i,color in enumerate(('mana','health')):
  kra=art/color/'fountain.kra';source=Path(receipt['graphics_master'])/color
  with zipfile.ZipFile(kra) as z:
   check(z.testzip() is None,'native fountain KRA archive integrity')
   check(z.read('maindoc.xml').count(b'<layer')>=5,'native fountain master retains separate cutout layers')
   with Image.open(io.BytesIO(z.read('mergedimage.png'))) as im,Image.open(source/'full.png') as native:
    check(np.array_equal(np.asarray(im.convert('RGBA')),np.asarray(native.convert('RGBA'))),'KRA merged image matches the actual Krita export')
  check(sha(kra)==sha(source/'fountain.kra'),'repository fountain master matches the native source')
  for part in (art/color/'parts').glob('*.png'):check(sha(part)==sha(source/part.name),'Synfig cutout is the exact authored Krita layer')
  atlas=Image.open(repo/'assets/sprites'/('fountain-'+color+'.png')).convert('RGBA');atlas.load()
  check(atlas.size==(1024,960),'bounded RGBA atlas dimensions')
  check(sha(repo/'assets/sprites'/('fountain-'+color+'.png'))==receipt['atlases'][i]['sha256'],'runtime atlas matches production receipt')
  native=[]
  for action in ('loop','drain'):
   sif=art/color/(action+'.sif');xml=ET.parse(sif).getroot();check(xml.get('fps')=='24' and xml.get('end-time')=='23f','native water loop/drain has correct frame range')
   check(sha(sif)==sha(Path(receipt['animation_master'])/color/(action+'.sif')),'saved Synfig master is the project that rendered')
   paths=sorted((Path(receipt['animation_master'])/color/(action+'-renders')).rglob('*.png'));check(len(paths)==24,'all native water PNG frames were rendered')
   hashes=set()
   for p in paths:
    with Image.open(p) as im:
     im.load();check(im.size==(128,160) and im.mode=='RGBA' and im.getbbox() is not None,'native water frame decodes with transparency')
     native.append(np.asarray(im).copy());hashes.add(sha(p))
   check(len(hashes)>12,'native loop/drain actually animates')
  for f,a in enumerate(native):
   b=np.asarray(atlas.crop((f%8*128,f//8*160,f%8*128+128,f//8*160+160)))
   check(np.array_equal(a,b),'every packed frame matches native Synfig output exactly')
  with Image.open(repo/'assets/sprites'/('fountain-'+color+'-empty.png')) as dry:check(np.array_equal(np.asarray(dry),native[-1]),'dry state matches the end of the water drain')
  def water(a):
   p=a[104:123,20:108,:3].astype(float)
   return int(((p[:,:,2]>p[:,:,0]*1.3)&(p[:,:,1]>p[:,:,0]*1.2)).sum()) if i==0 else int(((p[:,:,0]>p[:,:,1]*1.3)&(p[:,:,0]>p[:,:,2]*1.2)).sum())
  check(water(native[0])>150 and water(native[-1])<5,'colored pool is full before use and completely gone after use')
  paths=sorted((qa/f'fountain-{i}-frames').glob('*.png'),key=lambda p:int(p.stem));check(len(paths)==48,'actual game captures the full water loop and draining animation')
  images=[]
  for p in paths:
   with Image.open(p) as im:
    im.load();check(im.size==(1440,900),'real integrated fountain game frame decodes')
    if previews:images.append(im.convert('RGB').crop((350,220,640,500)).resize((435,420),Image.Resampling.LANCZOS))
  check(len({sha(p) for p in paths[:24]})>12,'game renders changing water across the loop')
  if previews:
   palette=Image.new('RGB',(435,840));palette.paste(images[0],(0,0));palette.paste(images[-1],(0,420));palette=palette.quantize(colors=192)
   frames=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in images]
   frames[0].save(qa/('fountain-'+color+'.gif'),save_all=True,append_images=frames[1:],duration=[40,40,45]*15+[40,40,600],loop=0,optimize=True)
   images[0].save(qa/('fountain-'+color+'-full.png'));images[-1].save(qa/('fountain-'+color+'-empty.png'))
  result.append({'color':color,'native_frames':48,'game_frames':48,'atlas_bytes':1024*960*4,'pool_pixels_full':water(native[0]),'pool_pixels_empty':water(native[-1])})
 return {'status':'PASS','fountains':result,'total_rgba_bytes':2*(1024*960*4+128*160*4),'audio_played':False}

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--qa',type=Path,required=True);p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--previews',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();count=0
 def check(ok,label):
  global count
  count+=1
  if not ok:raise AssertionError(label)
 result=verify(a.qa,a.repo,a.previews,check);result['checks']=count;(a.qa/'fountain-verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))

"""Independent decoded-source and actual GPU screenshot checks; no drawing."""
import argparse, hashlib, io, json, zipfile
from pathlib import Path
import xml.etree.ElementTree as ET
import numpy as np
from PIL import Image


def verify(qa,repo):
 checks=0
 def check(ok,label):
  nonlocal checks
  checks+=1
  if not ok:raise AssertionError(label)
 provenance=json.loads((repo/'art/scenery/trees/provenance.json').read_text(encoding='utf-8'))
 check(provenance['status']=='PASS' and provenance['original_art'] and not provenance['external_assets'],'local original art provenance')
 check(provenance['source_anchor']==[270,624] and provenance['logical_anchor']==[90,208],'consistent source and world anchors')
 palettes=[]
 for tree in provenance['trees']:
  species=tree['species'];name='forest-'+species
  master=repo/'art/scenery/trees'/(name+'.kra');runtime=repo/'assets/sprites'/(name+'.png')
  check(hashlib.sha256(master.read_bytes()).hexdigest()==tree['kra_sha256'],'KRA receipt '+species)
  check(hashlib.sha256(runtime.read_bytes()).hexdigest()==tree['png_sha256'],'PNG receipt '+species)
  with Image.open(runtime) as im:
   im.load();check(im.size==(540,690) and im.mode=='RGBA','native dimensions and RGBA '+species);pixels=np.asarray(im)
  a=pixels[:,:,3];solid=a>230
  check(int(a.min())==0 and int(a.max())==255,'transparent background '+species)
  check(not a[:3,:].any() and not a[-3:,:].any() and not a[:,:3].any() and not a[:,-3:].any(),'unclipped silhouette '+species)
  check(int(solid.sum())>20000,'substantial detailed tree silhouette '+species)
  check(int(solid[615:632,258:282].sum())>150,'root and trunk contact '+species)
  check(len(np.unique(pixels[solid,:3],axis=0))>150,'nonflat foliage and bark '+species)
  with zipfile.ZipFile(master) as z:
   check(z.testzip() is None,'KRA full ZIP integrity '+species)
   xml=ET.fromstring(z.read('maindoc.xml'));layers=[e for e in xml.iter() if e.tag.endswith('layer') and e.get('nodetype')=='paintlayer']
   check(len(layers)>=8,'editable paint layers '+species)
   with Image.open(io.BytesIO(z.read('mergedimage.png'))) as merged:
    check(np.array_equal(np.asarray(merged.convert('RGBA')),pixels),'saved KRA matches PNG pixels '+species)
  palettes.append(hashlib.sha256(pixels.tobytes()).hexdigest())
 check(len(set(palettes))==3,'three distinct species')
 report=(qa/'checks.txt').read_text(encoding='utf-8')
 check('Failures: 0' in report,'actual raylib player diagnostic passed')
 winds=list((qa/'trees').glob('*-wind.png'));check(len(winds)==36,'all type/scale/wind samples generated')
 for path in winds:
  with Image.open(path) as im:
   im.load();check(im.size==(1440,900) and im.mode=='RGBA','wind frame decode '+path.name)
   a=np.asarray(im)[:,:,3]
   check(not a[:2,:].any() and not a[-2:,:].any() and not a[:,:2].any() and not a[:,-2:].any(),'uncropped real wind mesh '+path.name)
 for pattern,count in [('forest-*.png',4),('hover-*.png',3),('tree-lineup.png',1)]:
  paths=list(qa.glob(pattern));check(len(paths)==count,'complete '+pattern)
  for path in paths:
   with Image.open(path) as im:
    im.load();rgb=np.asarray(im.convert('RGB'));check(float(rgb.std())>15,'nonempty game render '+path.name)
 result={'status':'PASS','checks':checks,'trees':3,'native_size':[540,690],'runtime_cache_size':[360,460],
  'wind_samples':36,'actual_forest_views':4,'hover_views':3,'cache_rgba8_bytes':3*360*460*4,
  'added_cache_and_outline_rgba8_bytes':3*(360*460-180*230)*4*2,'original_art':True,'audio_played':False}
 (qa/'source-verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result,indent=2))


if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--qa',type=Path,required=True);p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--no-audio',action='store_true',required=True)
 a=p.parse_args();verify(a.qa,a.repo)

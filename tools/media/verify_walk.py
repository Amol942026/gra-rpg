"""Validate saved Synfig source, actual C++ joints and decoded rendered frames."""
from pathlib import Path
import argparse, csv, hashlib, json, math, xml.etree.ElementTree as ET
from PIL import Image
from walk_synfig import compile_clip,read_controls
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def verify(project,qa):
 repo=Path(__file__).resolve().parents[2];source=project/'walk.sif';roundtrip=qa/'roundtrip.hpp'
 compile_clip(source,roundtrip);assert roundtrip.read_bytes()==(repo/'src/generated/hero_walk_data.hpp').read_bytes()
 controls=read_controls(source);runtime=list(csv.DictReader((qa/'runtime-controls.csv').open()))
 assert len(runtime)==len(controls)==61;error=0
 for row,actual in zip(controls,runtime):
  for name in ['hip_r','hip_z']+[s+'_'+part+'_'+axis for s in 'lr' for part in ['foot','hand'] for axis in 'rfz']:
   error=max(error,abs(row[name]-float(actual[name])))
 assert error<.001,error
 for manifest in sorted((project/'renders').glob('*/render-manifest.json'),reverse=True):
  data=json.loads(manifest.read_text(encoding='utf-8-sig'))
  if data['ProjectSha256']!=sha(source):continue
  assert data['ActualFrames']==60 and data['Status']=='PASS'
  hashes=set()
  for frame in data['Frames']:
   p=manifest.parent/frame['Name'];assert sha(p)==frame['Sha256'];im=Image.open(p).convert('RGBA');assert im.size==(400,480) and im.getbbox();hashes.add(hashlib.sha256(im.tobytes()).hexdigest())
  assert len(hashes)>50
  break
 else:raise AssertionError('Native render manifest missing')
 frames=sorted((qa/'frames').glob('frame-*.png'));assert len(frames)==120
 movie=[]
 for p in frames:
  im=Image.open(p).convert('RGB');assert im.size==(1080,400);movie.append(im.resize((810,300),Image.Resampling.LANCZOS))
 movie[0].save(qa/'walk-preview.gif',save_all=True,append_images=movie[1:],duration=[16,17,17]*40,loop=0,optimize=True)
 directions=sorted(qa.glob('direction-*.png'));assert len(directions)==8
 for p in directions:
  with Image.open(p) as im:assert im.size==(1800,780);im.load()
 metrics=json.loads((qa/'metrics.json').read_text());assert metrics['after']['highest_support']<.04 and metrics['after']['lowest_boot']>-.04
 render=json.loads((qa/'render-verification.json').read_text());assert render['status']=='PASS' and render['border_pixels']==render['invalid_bones']==render['failed_writes']==0
 report={'status':'PASS','project_sha256':sha(source),'cpp_controls_max_error':error,'native_frames':60,'runtime_frames':120,'directions':8,'source_to_cpp_exact':True,'audio':False,'metrics':metrics}
 (qa/'asset-verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report,indent=2))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--project',type=Path,required=True);p.add_argument('--qa',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.project,a.qa)

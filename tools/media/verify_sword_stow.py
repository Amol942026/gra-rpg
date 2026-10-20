"""Independent native-project/asset validation and silent game preview packing."""
from pathlib import Path
import argparse, hashlib, io, json, zipfile
from PIL import Image
from sword_stow_synfig import export

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def verify(project,qa,backup):
 repo=Path(__file__).resolve().parents[2]
 check=qa/'stow-roundtrip.hpp';export(project/'sheathe.sif',check)
 assert check.read_bytes()==(repo/'src/generated/sword_stow_data.hpp').read_bytes()
 import xml.etree.ElementTree as ET
 a=ET.parse(project/'sheathe.sif').getroot();b=ET.parse(project/'draw.sif').getroot()
 for x,y in zip(a.find('defs'),b.find('defs')):assert [w.find('real').get('value') for w in x]==[w.find('real').get('value') for w in y][::-1]
 for node in a.findall('defs/real'):assert node.get('value')==b.find(f"defs/real[@id='{node.get('id')}']").get('value')
 count=0
 for manifest in (project/'renders').glob('*/render-manifest.json'):
  m=json.loads(manifest.read_text(encoding='utf-8-sig'));assert m['Status']=='PASS' and m['ActualFrames']==36
  hashes=set()
  for f in m['Frames']:
   p=manifest.parent/f['Name'];assert sha(p)==f['Sha256'];im=Image.open(p).convert('RGBA');assert im.size==(320,360) and im.getbbox();hashes.add(hashlib.sha256(im.tobytes()).hexdigest());count+=1
  assert len(hashes)>30
 assert count==72
 png=repo/'assets/sprites/hero-paints.png';im=Image.open(png).convert('RGBA');old=Image.open(backup/'assets/sprites/hero-paints.png').convert('RGBA')
 assert im.crop((128,0,512,128)).tobytes()==old.crop((128,0,512,128)).tobytes()
 assert im.crop((0,128,512,384)).tobytes()==old.crop((0,128,512,384)).tobytes()
 features=im.crop((0,0,128,128));assert features.getchannel('A').getextrema()==(0,255)
 assert all(features.getpixel((x,y))[3]==0 for y in range(128) for x in range(128) if x<30 or x>98 or y<38 or y>100)
 with zipfile.ZipFile(repo/'art/hero/hero-paints.kra') as z:
  assert z.testzip() is None;assert Image.open(io.BytesIO(z.read('mergedimage.png'))).convert('RGBA').tobytes()==im.tobytes()
 frames=sorted((qa/'frames').glob('frame-*.png'));assert len(frames)==180
 clearance=json.loads((repo/'dist/qa-sheath-clearance/clearance.json').read_text())
 assert clearance['status']=='PASS' and clearance['intersections']==clearance['input_intersections']==0
 directions=sorted(qa.glob('clearance-directions-*.png'));assert len(directions)==11
 for p in directions:
  with Image.open(p) as frame:assert frame.size==(1920,880);frame.load()
 movie=[]
 for p in frames:
  frame=Image.open(p).convert('RGB');assert frame.size==(1200,400);movie.append(frame.resize((900,300),Image.Resampling.LANCZOS))
 movie[0].save(qa/'transition-preview.gif',save_all=True,append_images=movie[1:],duration=[16,17,17]*60,loop=0,optimize=True)
 result={'status':'PASS','source':'native Krita facial features; Synfig stow/draw curves','features_only_no_face_border':True,'nonface_cells_unchanged':True,'native_master_matches_png':True,'sif_to_cpp_exact':True,'synfig_frames':72,'game_frames':180,'game_duration':3,'clearance_poses':clearance['poses'],'clearance_input_frames':clearance['input_frames'],'clearance_intersections':0,'eight_direction_stages':len(directions),'audio':False,'hero_png_sha256':sha(png)}
 (qa/'asset-verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result,indent=2))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--project',type=Path,required=True);p.add_argument('--qa',type=Path,required=True);p.add_argument('--backup',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.project,a.qa,a.backup)

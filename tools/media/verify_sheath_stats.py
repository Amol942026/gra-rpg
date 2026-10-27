"""Independent SIF/header, native renders, cursor panels and hero readback."""
import argparse,csv,hashlib,json,math,re
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image
from sword_stow_synfig import export,NAMES

def main():
 p=argparse.ArgumentParser();p.add_argument('--qa',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 root=Path(__file__).resolve().parents[2];receipt=json.loads((root/'art/hero/stow/receipt.json').read_text(encoding='utf-8'));source=Path(receipt['project']).parent;checks=[]
 def check(ok,label):checks.append({'pass':bool(ok),'check':label})
 sha=lambda x:hashlib.sha256(x.read_bytes()).hexdigest()
 check(sha(source/'sheathe.sif')==receipt['project_sha256'] and sha(root/'src/generated/sword_stow_data.hpp')==receipt['header_sha256'],'native source and runtime export match current provenance')
 export(source/'sheathe.sif',a.qa/'stow-roundtrip.hpp');check((a.qa/'stow-roundtrip.hpp').read_bytes()==(root/'src/generated/sword_stow_data.hpp').read_bytes(),'saved native Synfig controls export byte-for-byte to runtime data')
 forward=ET.parse(source/'sheathe.sif').getroot();backward=ET.parse(source/'draw.sif').getroot()
 for name in NAMES:
  f=forward.find(f"defs/animated[@id='stow_{name}']");b=backward.find(f"defs/animated[@id='stow_{name}']")
  check([w.find('real').get('value') for w in f]==[w.find('real').get('value') for w in b][::-1],'draw reverses actual native '+name+' controls')
 mouth=[float(forward.find(f"defs/real[@id='scabbard_mouth_{axis}']").get('value')) for axis in 'rfz']
 check(mouth==[-16,13,36] and abs(mouth[0])<22,'scabbard is six units closer to the torso laterally')
 count=0
 for path in (source/'renders').glob('*/render-manifest.json'):
  meta=json.loads(path.read_text(encoding='utf-8-sig'));check(meta['Status']=='PASS' and meta['ActualFrames']==36,'native Synfig clip rendered all 36 frames');hashes=set()
  for frame in meta['Frames']:
   imagepath=path.parent/frame['Name'];check(sha(imagepath)==frame['Sha256'],'native frame checksum')
   with Image.open(imagepath) as im:im.load();check(im.size==(320,360) and im.getbbox(),'native control-rig frame decodes');hashes.add(hashlib.sha256(im.tobytes()).hexdigest())
   count+=1
  check(len(hashes)>30,'native sheathing/drawing animation contains actual motion')
 check(count==72,'both native animations rendered completely')
 summary=json.loads((a.qa/'summary.json').read_text(encoding='utf-8'));check(summary['status']=='PASS' and summary['failures']==0 and summary['audio_opened'] is False,'all actual player UI and rendering checks passed silently')
 tips=list(csv.DictReader((a.qa/'tooltips.tsv').open(encoding='utf-8'),delimiter='\t'));check(len(tips)==18,'all six cursor descriptions tested at three pixel scales')
 for row in tips:
  x,y,w,h=(float(row[k]) for k in ('x','y','width','height'))
  check(x>=8 and y>=8 and x+w<=1428 and y+h<=888,'complete tooltip stays inside logical screen')
  path=a.qa/f"tooltip-{row['attribute']}-{round(float(row['scale'])*100)}.png"
  with Image.open(path) as im:im.load();check(im.size==(round(1440*float(row['scale'])),round(900*float(row['scale']))),'actual tooltip scene has native framebuffer resolution')
 hero=list((a.qa/'hero').glob('*.png'));check(len(hero)==32,'standing, walking, running and insertion captured in eight directions')
 for path in hero:
  with Image.open(path) as im:im.load();box=im.getchannel('A').getbbox();check(im.size==(480,440) and box and box[0]>1 and box[1]>1 and box[2]<479 and box[3]<439,'full hero with closer scabbard fits its frame')
 text=(a.qa.parent/'qa-v13/progression-tests.txt').read_text(encoding='utf-8');check('FAIL' not in text and 'failures: 0' in text,'existing advancement and spell UI passed')
 result={'status':'PASS' if all(x['pass'] for x in checks) else 'FAIL','checks':len(checks),'failures':[x for x in checks if not x['pass']],'native_frames':count,'hero_views':len(hero),'attribute_tooltips':len(tips),'audio_opened':False}
 (a.qa/'verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result));raise SystemExit(0 if result['status']=='PASS' else 2)
if __name__=='__main__':main()

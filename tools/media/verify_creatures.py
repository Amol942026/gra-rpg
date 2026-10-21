"""Verify native masters, exact Synfig/runtime agreement and real GPU PNGs.

Silent image inspection only. The GIF is QA evidence, never a game resource.
"""
from pathlib import Path
import argparse,hashlib,json,re,xml.etree.ElementTree as ET,zipfile
from PIL import Image

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def verify(project,qa,repo):
 checks=[]
 def check(ok,label):
  checks.append({'pass':bool(ok),'label':label})
  if not ok:raise AssertionError(label)
 header=(repo/'src/generated/creature_motion_data.hpp').read_text()
 for family in ['gel','quadruped','humanoid','arachnid','serpent','elemental']:
  p=project/(family+'.sif');tracks={}
  for l in ET.parse(p).getroot().iter('layer'):
   if l.get('desc','').startswith('runtime:'):
    tracks[l.get('desc')[8:]]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in l.find("param[@name='origin']/animated")]
  native=[(*a,*b) for a,b in zip(tracks['foot'],tracks['body'])]
  match=re.search(r'\b'+family+r'\[\]\s*=\s*\{(.*?)\};',header,re.S)
  compiled=[tuple(map(float,re.findall(r'(-?[\d.]+)f',row))) for row in re.findall(r'\{([^}]+)\}',match[1])]
  check(compiled==native and len(native)==61,f'{family}: every compiled value equals saved Synfig source')
  check(native[0]==native[-1] and min(row[1] for row in native)>=0,f'{family}: loop closure and foot contact')
  check(sum(row[1]==0 for row in native)>=33,f'{family}: grounded support for most of stride')
  manifests=list((project/'renders').glob(f'{family}_*/render-manifest.json'));check(len(manifests)==1,f'{family}: unique native render')
  manifest=json.loads(manifests[0].read_text(encoding='utf-8-sig'));directory=manifests[0].parent
  check(manifest['Status']=='PASS' and manifest['ActualFrames']==60 and manifest['ProjectSha256']==sha(p),f'{family}: complete native render of current source')
  for record in manifest['Frames']:check(sha(directory/record['Name'])==record['Sha256'],f'{family}/{record["Name"]}: native render hash')
  hashes=[]
  for f in sorted(directory.glob('*.png')):
   with Image.open(f) as im:im.load();check(im.size==(320,256),f'{family}/{f.name}: PNG decodes');hashes.append(hashlib.sha256(im.tobytes()).hexdigest())
  check(len(hashes)==60 and len(set(hashes))>40,f'{family}: visible native animation')
  check(sha(p)==sha(repo/'art/creatures/motion'/p.name),f'{family}: repository master matches production source')
 kra=repo/'art/creatures/creature-paints.kra'
 with zipfile.ZipFile(kra) as z:check(z.testzip() is None and b'<layer' in z.read('maindoc.xml'),'layered KRA archive is intact')
 with Image.open(repo/'assets/sprites/creature-paints.png') as im:
  im=im.convert('RGBA');check(im.size==(512,512) and im.getextrema()[3]==(0,255),'RGBA8 transparent Krita atlas')
  for cell in range(16):check(im.crop((cell%4*128,cell//4*128,cell%4*128+128,cell//4*128+128)).getbbox() is not None,f'paint cell {cell} contains detail')
 report=json.loads((qa/'verification.json').read_text());check(report['status']=='PASS' and report['failures']==0 and not report['audio_device_opened'],'native renderer, pointer, shadows and loot QA passed')
 if 'legacy_directory' in report:
  legacy=Path(report['legacy_directory'])
  expected=['qa-v39/fallen-bestiary.png']+[f'qa-v13/body-{n}.png' for n in range(4)]+[f'qa-v13/{name}.png' for name in ['corpse-drag-to-bag','corpse-invalid-drop','empty-body','corpse-panel-80','corpse-panel-100','corpse-panel-120']]+[f'qa-v40/{name}.png' for name in ['forest-shadows','desert-shadows','intro-shadows']+[f'tower-{n}-shadows' for n in range(1,6)]]
  legacy_images=[legacy/p for p in expected];check(len(legacy_images)==19 and all(p.is_file() for p in legacy_images),'all 19 isolated legacy diagnostic PNGs exist')
  for p in legacy_images:
   with Image.open(p) as im:im.load();check(im.getbbox() is not None,p.name+' legacy output decodes')
  for path,count in [('qa-v13/corpse-tests.txt',40),('qa-v40/shadow-tests.txt',490)]:
   contents=(legacy/path).read_text();check(f'Checks: {count}; failures: 0' in contents,path+' full interaction report')
 poses=list((qa/'poses').glob('*.png'));check(len(poses)==288,'all species/directions/actions exported')
 for p in poses:
  with Image.open(p) as im:im.load();check(im.size==(360,420) and im.getbbox() is not None,p.name+' decodes')
 frames=sorted((qa/'frames').glob('frame-*.png'));check(len(frames)==60,'complete actual game motion sequence')
 hashes=[set() for _ in range(12)];gif=[]
 for f,p in enumerate(frames):
  with Image.open(p) as im:
   im.load();check(im.size==(1440,1080),p.name+' full image decodes')
   for species in range(12):
    cell=im.crop((species%4*360,species//4*360,species%4*360+360,species//4*360+360));hashes[species].add(hashlib.sha256(cell.tobytes()).hexdigest())
   if f%2==0:gif.append(im.resize((720,540),Image.Resampling.LANCZOS).convert('RGB'))
 for species,items in enumerate(hashes):check(len(items)>=45,f'species {species}: motion is visible in game render')
 # GIF uses 10 ms ticks: 30/30/40 preserves the real one-second cycle.
 gif[0].save(qa/'creatures-motion.gif',save_all=True,append_images=gif[1:],duration=[30 if i%3<2 else 40 for i in range(len(gif))],loop=0,optimize=False)
 output={'status':'PASS','checks':len(checks),'native_synfig_frames':360,'actual_game_frames':60,'poses':288,'species_unique_frames':[len(h) for h in hashes],
         'atlas_sha256':sha(repo/'assets/sprites/creature-paints.png'),'native_kra_sha256':sha(kra),'motion_header_sha256':sha(repo/'src/generated/creature_motion_data.hpp'),'audio_played':False,'details':checks}
 (qa/'asset-verification.json').write_text(json.dumps(output,indent=2),encoding='utf-8');print(f'PASS: {len(checks)} asset checks, 360 Synfig frames, 288 poses, 60 game frames')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--project',type=Path,required=True);p.add_argument('--qa',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.project,a.qa,Path(__file__).resolve().parents[2])

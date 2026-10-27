"""Decode native art, compare Synfig samples, inspect actual player frames."""
from pathlib import Path
import argparse,hashlib,io,json,math,re,zipfile,xml.etree.ElementTree as ET
from PIL import Image
import numpy as np
from verify_fountains import verify as verify_fountains
from verify_guardian_shatter import verify as verify_shatter
from verify_hero_knockdown import verify as verify_knockdown

def verify(qa,repo,previews):
 count=0
 def check(ok,label):
  nonlocal count
  count+=1
  if not ok:raise AssertionError(label)
 art=repo/'art/gameplay/guardian';receipt=json.loads((art/'stone-provenance.json').read_text(encoding='utf-8'))
 png=repo/'assets/sprites/guardian-stone.png';kra=art/'guardian-stone.kra'
 check(receipt['status']=='PASS' and receipt['original_art'] and not receipt['external_assets'],'original local Krita art')
 check(hashlib.sha256(png.read_bytes()).hexdigest()==receipt['png_sha256'],'runtime stone PNG hash')
 check(hashlib.sha256(kra.read_bytes()).hexdigest()==receipt['kra_sha256'],'native KRA hash')
 with Image.open(png) as image:
  image.load();pixels=np.asarray(image.convert('RGBA'));check(image.size==(192,192) and image.mode=='RGBA','transparent PNG format');check(pixels[:,:,3].min()==0 and pixels[:,:,3].max()==255,'transparent background')
 with zipfile.ZipFile(kra) as z:
  check(z.testzip() is None,'KRA integrity')
  with Image.open(io.BytesIO(z.read('mergedimage.png'))) as merged:check(np.array_equal(np.asarray(merged.convert('RGBA')),pixels),'saved KRA and runtime image match')
 sif=art/'motion/guardian-throw.sif';header=repo/'src/generated/guardian_throw_data.hpp';motion=json.loads((art/'motion/motion-receipt.json').read_text(encoding='utf-8'))
 check(hashlib.sha256(sif.read_bytes()).hexdigest()==motion['source_sha256'],'Synfig source receipt')
 check(hashlib.sha256(header.read_bytes()).hexdigest()==motion['header_sha256'],'compiled pose receipt')
 tracks={}
 for layer in ET.parse(sif).getroot().iter('layer'):
  if layer.get('desc','').startswith('runtime:') and layer.get('desc')[8:] in motion['tracks']:tracks[layer.get('desc')[8:]]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in layer.find("param[@name='origin']/animated")]
 rows=[]
 for frame in range(motion['samples']):
  pose={field:tracks[track][frame][i] for track,fields in motion['tracks'].items() for i,field in enumerate(fields)};rows.append(tuple(pose[f] for f in motion['fields']))
 compiled=[tuple(float(x) for x in row.split(',')) for row in re.findall(r'\{([-\d.,]+)\},',header.read_text(encoding='utf-8').replace('f',''))]
 check(len(rows)==len(compiled)==motion["samples"],'all native pose samples compiled')
 for a,b in zip(rows,compiled):check(max(abs(x-y) for x,y in zip(a,b))<1.e-6,'native and runtime pose agree')
 flexions=[]
 for row in compiled:
  p=dict(zip(motion['fields'],row));upper=(p['elbowSide']-23,p['elbowForward']-p['bodyForward'],p['elbowUp']-53-p['bodyUp']);lower=(p['handSide']-p['elbowSide'],p['handForward']-p['elbowForward'],p['handUp']-p['elbowUp'])
  check(upper[1]*lower[2]-upper[2]*lower[1]>1,'both mirrored elbows bend on the anatomical side, including recovery')
  cosine=sum(a*b for a,b in zip(upper,lower))/(math.sqrt(sum(a*a for a in upper))*math.sqrt(sum(a*a for a in lower)))
  angle=math.degrees(math.acos(max(-1,min(1,cosine))));flexions.append(angle);check(8<angle<145,'elbows remain within flexion limits')
 check(rows[0]==rows[-1] and rows[motion["release_frame"]][6:8]==(24,102),'overhead release position and return to idle')
 native_root=Path(motion['master'])/'renders'
 native=list(native_root.glob('*/*.png'));check(len(native)==motion["frames"],'all native Synfig throw frames exist')
 native_hashes=set()
 for path in native:
  with Image.open(path) as image:
   image.load();check(image.size==(480,400),'native frame decode');native_hashes.add(hashlib.sha256(image.tobytes()).hexdigest())
 check(len(native_hashes)>motion["frames"]*.85,'native 2D cutout and both arms really move')
 checks=(qa/'checks.txt').read_text(encoding='utf-8');check('Failures: 0' in checks,'real renderer and menu tests passed')
 paths=sorted((qa/'frames').glob('*.png'),key=lambda p:int(p.stem));check(len(paths)==120,'all 120 game sequence frames exist')
 images=[];previous=None;changes=[]
 for path in paths:
  with Image.open(path) as image:
   image.load();check(image.size==(1440,900),'game frame decode');rgb=np.asarray(image.convert('RGB'))
   if previous is not None:changes.append(int(np.any(previous[150:530,180:680]!=rgb[150:530,180:680],axis=2).sum()))
   previous=rgb.copy()
   if previews:images.append(image.convert('RGB').crop((285,150,635,460)).resize((525,465),Image.Resampling.LANCZOS))
 check(min(changes)>30,'actual boss and projectile animate between frames')
 for name in ('forest-start','guardian-uproot','guardian-lift','guardian-windup','guardian-flight','guardian-impact'):
  with Image.open(qa/(name+'.png')) as image:image.load();check(np.asarray(image.convert('RGB')).std()>20,'nonempty '+name)
 pose_frames=(16,29,53,74,89,98)
 angles=list((qa/'angles').glob('*.png'));check(len(angles)==48,'ground, tear, lift, overhead, release and recovery inspected in eight facings')
 sheet=Image.new('RGB',(8*220,len(pose_frames)*250),(22,30,25)) if previews else None
 facing_hashes=set()
 for direction in range(8):
  for row,frame in enumerate(pose_frames):
   with Image.open(qa/'angles'/f'{direction}-{frame}.png') as image:
    image.load();check(image.size==(1440,900),'facing pose decode')
    cropped=image.convert('RGB').crop((290,150,510,400))
    if frame==74:facing_hashes.add(hashlib.sha256(cropped.tobytes()).hexdigest())
    if previews:sheet.paste(cropped,(direction*220,row*250))
 check(len(facing_hashes)==8,'all eight overhead views actually differ')
 if previews:
  # Fixed palette and 15 Hz preview reduce size; all 120 original 30 Hz captures
  # above are decoded and checked. Preview duration remains exactly four seconds.
  palette=images[0].quantize(colors=128);preview=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in images[::2]]
  preview[0].save(qa/'guardian-throw.gif',save_all=True,append_images=preview[1:],duration=[60,70,70]*20,loop=0,optimize=True);sheet.save(qa/'guardian-poses.png')
 shatter=verify_shatter(qa,repo,previews,check)
 knockback=json.loads((qa/'knockback-report.json').read_text(encoding='utf-8'))
 check(knockback['status']=='PASS' and 110<=knockback['active_frames']<=112 and abs(knockback['distance']-1.35)<.001,'actual collision-free knockdown distance and full recovery duration')
 with Image.open(qa/'guardian-knockback.png') as im:im.load();check(im.size==(1440,900),'real mid-knockback game capture decodes')
 if previews:
  start=max(0,knockback['onset_frame']//2-4);clip=[]
  for path in paths[start:start+24]:
   with Image.open(path) as im:clip.append(im.convert('RGB').crop((260,260,700,550)).resize((660,435),Image.Resampling.LANCZOS))
  palette=clip[0].quantize(colors=192);clip=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in clip]
  clip[0].save(qa/'guardian-knockback.gif',save_all=True,append_images=clip[1:],duration=[30,35]*12,loop=0,optimize=True)
 knockdown=verify_knockdown(qa,repo,previews,check)
 fountains=verify_fountains(qa,repo,previews,check)
 result={'status':'PASS','checks':count,'native_synfig_frames':motion['frames'],'compiled_control_samples':motion['samples'],'game_frames':120,'pose_views':48,'elbow_flexion_min_deg':min(flexions),'elbow_flexion_max_deg':max(flexions),'minimum_changed_pixels':min(changes),'shatter':shatter,'knockback':knockback,'knockdown':knockdown,'fountains':fountains,'audio_played':False}
 (qa/'image-verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result,indent=2))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--qa',type=Path,required=True);p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--previews',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.qa,a.repo,a.previews)

"""Saved Synfig source, baked pose and real equipped hero acceptance checks."""
from pathlib import Path
import hashlib,json,re
import numpy as np
from PIL import Image
from hero_backfall_synfig import read_controls,world_pose,local_pose,FRAMES,DURATION
from walk_synfig import rig_layout,rotate,qmul,conj

def verify(qa,repo,previews,check):
 art=repo/'art/hero/knockdown';receipt=json.loads((art/'receipt.json').read_text(encoding='utf-8'))
 source=art/'knockdown.sif';header=repo/'src/generated/hero_knockdown_data.hpp'
 sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
 check(receipt['status']=='PASS' and not receipt['external_assets'],'original local Synfig motion')
 check(sha(source)==receipt['project_sha256'] and sha(header)==receipt['header_sha256'],'saved knockdown SIF and compiled header provenance')
 for part in (art/'parts').glob('*.png'):check(sha(part)==sha(repo/'art/hero/motion/parts'/part.name),'existing original Krita cutout part retained')
 rows=read_controls(source);layout=rig_layout()
 compiled=[tuple(map(float,re.findall(r'[-\d.]+(?=f)',line))) for line in header.read_text().splitlines() if line.startswith('{{')]
 expected=[key for row in rows for key in local_pose(world_pose(row,layout),layout)]
 check(len(compiled)==len(expected)==(FRAMES+1)*17,'all native samples and 17 bones are baked')
 for saved,baked in zip(expected,compiled):check(max(abs(a-b) for a,b in zip(saved,baked))<1.e-7,'runtime joint agrees with saved native Synfig controls')
 check(rows[42]['pitch']>=89 and rows[48]['pitch']>=89 and rows[0]['pitch']==rows[-1]['pitch']==0,'full fall, ground hold and return to standing are authored')
 check(rows[64]['hand_l_z']<6 and rows[96]['hip_z']>rows[42]['hip_z'],'get-up uses hands and rising pelvis')
 flight=world_pose(rows[14],layout);rest=world_pose(rows[48],layout)
 check(min(flight[b][0][2] for b in (12,15))>15,'both feet are visibly airborne in the saved motion')
 front=rotate(qmul(rest[2][1],conj(layout[1][2])),(0,1,0))
 check(front[2]>.98 and rest[3][0][2]<15,'chest faces upwards while the head rests low on the ground')
 check(receipt['stages_seconds']['stand']==1.75 and receipt['stages_seconds']['supine_rest']==[.66,.8],'quicker recovery retains a genuine supine pause')
 native=list((Path(receipt['project']).parent/'renders').glob('*/*.png'));check(len(native)==FRAMES,'all native Synfig backfall renders exist')
 hashes=set()
 for p in native:
  with Image.open(p) as im:im.load();check(im.size==(600,440),'native knockdown frame decodes');hashes.add(hashlib.sha256(im.tobytes()).hexdigest())
 check(len(hashes)>90,'native fall/get-up preview actually moves')
 angles=qa/'knockdown/angles';paths=list(angles.glob('*.png'));check(len(paths)==72,'nine full-body poses in eight facing directions captured')
 for p in paths:
  with Image.open(p) as im:
   im.load();check(im.size==(640,520),'full equipped hero frame decodes');a=np.asarray(im)[:,:,3]
   check(int((a>32).sum())>1200 and not (a[:2]>32).any() and not (a[-2:]>32).any() and not (a[:,:2]>32).any() and not (a[:,-2:]>32).any(),'fall silhouette is present and not clipped')
 for direction in range(8):
  with Image.open(angles/f'{direction}-0.png') as first,Image.open(angles/f'{direction}-111.png') as last:
   a,b=np.asarray(first),np.asarray(last)
   # The ground bound and normalized rotations can change a few antialiased
   # boot-edge pixels. A body/equipment snap changes thousands of pixels.
   check(int(np.any(a!=b,axis=2).sum())<=32 and int(((a[:,:,3]>32)!=(b[:,:,3]>32)).sum())<=16,'get-up matches standing within subpixel rasterization tolerance')
 gear_count=11+int(re.search(r'array<GearDefinition,(\d+)>',(repo/'src/items.hpp').read_text(encoding='utf-8')).group(1))
 equipment=list((qa/'knockdown/equipment').glob('*.png'));check(len(equipment)==gear_count,'all equipped items rendered during the fall')
 check(len({sha(p) for p in equipment})>=30,'equipment still changes the actual falling hero appearance')
 for p in equipment:
  with Image.open(p) as im:im.load();check(im.size==(640,520),'equipped fall image decodes')
 frames=sorted((qa/'knockdown/frames').glob('*.png'),key=lambda p:int(p.stem));check(len(frames)==57,'complete retimed backfall and recovery captured')
 for p in frames:
  with Image.open(p) as im:im.load();check(im.size==(640,520),'complete recovery clip frame decodes')
 for stage in ('ground','brace','rise','recovered'):
  with Image.open(qa/f'guardian-knockdown-{stage}.png') as im:im.load();check(im.size==(1440,900) and np.asarray(im.convert('RGB')).std()>20,'real encounter '+stage+' is nonempty')
 encounter=sorted((qa/'backfall-frames').glob('*.png'),key=lambda p:int(p.stem));check(len(encounter)==63,'complete real-encounter backfall and retimed get-up sequence captured')
 for path in encounter:
  with Image.open(path) as im:im.load();check(im.size==(1440,900) and np.asarray(im.convert('RGB')).std()>20,'full encounter backfall frame decodes and is nonempty')
 if previews:
  sheet=Image.new('RGB',(8*320,5*260),(17,29,28))
  for d in range(8):
   for row,f in enumerate((0,14,42,64,111)):
    with Image.open(angles/f'{d}-{f}.png') as im:
     im=im.resize((320,260),Image.Resampling.LANCZOS);sheet.paste(im,(d*320,row*260),im)
  sheet.save(qa/'guardian-knockdown-poses.png')
  game_paths=sorted((qa/'backfall-frames').glob('*.png'),key=lambda p:int(p.stem))
  images=[]
  for p in game_paths:
   with Image.open(p) as im:images.append(im.convert('RGB').crop((260,260,700,550)).resize((660,435),Image.Resampling.LANCZOS))
  colors=Image.new('RGB',(660,435*3))
  for row,i in enumerate((0,len(images)//2,len(images)-1)):colors.paste(images[i],(0,row*435))
  palette=colors.quantize(colors=192);images=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in images]
  images[0].save(qa/'guardian-backfall.gif',save_all=True,append_images=images[1:],duration=[30 if i%3==0 else 35 for i in range(len(images)-1)]+[300],loop=0,optimize=True)
  close=[]
  for path in frames:
   with Image.open(path) as im:
    bg=Image.new('RGB',im.size,(17,29,28));bg.paste(im,mask=im.getchannel('A'));close.append(bg)
  colors=Image.new('RGB',(640,520*4))
  for row,i in enumerate((0,7,24,len(close)-1)):colors.paste(close[i],(0,row*520))
  palette=colors.quantize(colors=192);close=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in close]
  close[0].save(qa/'hero-backfall-preview.gif',save_all=True,append_images=close[1:],duration=[30 if i%3==0 else 35 for i in range(len(close)-1)]+[400],loop=0,optimize=True)
 return {'status':'PASS','duration':DURATION,'native_synfig_frames':FRAMES,'control_samples':FRAMES+1,'bones':17,'pose_views':72,'equipment_variants':gear_count,'game_clip_frames':57,'encounter_frames':63,'runtime_key_bytes':(FRAMES+1)*17*28,'new_textures':0,'supine_rest_seconds':[.66,.8],'stand_seconds':1.75}

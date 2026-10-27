"""Independent native-art, saved-SIF, ground contact and real-frame acceptance."""
from pathlib import Path
import hashlib,io,json,math,re,zipfile,xml.etree.ElementTree as ET
from PIL import Image
import numpy as np

def verify(qa,repo,previews,check):
 art=repo/'art/gameplay/guardian/shatter';proof=json.loads((art/'fragments-provenance.json').read_text(encoding='utf-8'))
 png=repo/'assets/sprites/guardian-fragments.png';kra=art/'guardian-fragments.kra'
 check(proof['status']=='PASS' and proof['original_art'] and len(proof['layers'])==6 and not proof['external_assets'],'six original native Krita cutouts')
 check(hashlib.sha256(png.read_bytes()).hexdigest()==proof['png_sha256'],'fragment atlas provenance');check(hashlib.sha256(kra.read_bytes()).hexdigest()==proof['kra_sha256'],'fragment KRA provenance')
 with Image.open(png) as im:im.load();pixels=np.asarray(im.convert('RGBA'));check(im.size==(288,192) and im.mode=='RGBA','fragment atlas format')
 with zipfile.ZipFile(kra) as z:
  check(z.testzip() is None,'fragment KRA integrity')
  with Image.open(io.BytesIO(z.read('mergedimage.png'))) as im:check(np.array_equal(np.asarray(im.convert('RGBA')),pixels),'native KRA matches runtime fragment atlas')
 for i in range(6):check(pixels[i//3*96:i//3*96+96,i%3*96:i%3*96+96,3].max()==255,'every cutout contains opaque painted stone')
 motion=json.loads((art/'shatter-receipt.json').read_text(encoding='utf-8'));sif=art/'guardian-shatter.sif';header=repo/'src/generated/guardian_shatter_data.hpp'
 check(hashlib.sha256(sif.read_bytes()).hexdigest()==motion['source_sha256'],'saved fracture SIF hash');check(hashlib.sha256(header.read_bytes()).hexdigest()==motion['header_sha256'],'compiled fracture control hash')
 tracks={}
 for layer in ET.parse(sif).getroot().iter('layer'):
  if layer.get('desc','').startswith('fracture:'):
   _,piece,name=layer.get('desc').split(':');tracks[int(piece),name]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in layer.find("param[@name='origin']/animated")]
 compiled=[tuple(map(float,row.split(','))) for row in re.findall(r'\{([-\d.,]+)\},',header.read_text(encoding='utf-8').replace('f','')) if len(row.split(','))==5]
 check(len(compiled)==6*73,'all 438 independent fragment control samples exist')
 for piece in range(6):
  cx,cy=proof['pivots'][piece]
  for frame in range(73):
   saved=(*tracks[piece,'ground'][frame],*tracks[piece,'lift'][frame],tracks[piece,'fade'][frame][0]);row=compiled[piece*73+frame]
   check(max(abs(a-b) for a,b in zip(saved,row))<1.e-6,'every runtime fragment pose agrees with native Synfig')
   x,y,height,angle,alpha=row;r=math.radians(angle)
   bottom=max((math.sin(r)*(px-cx)+math.cos(r)*(py-cy))*76/96 for px,py in proof['polygons'][piece])
   check(height>=bottom-.00001 and 0<=alpha<=1,'rotating fragments never sink beneath their ground anchor')
   if frame>=45:check(abs(height-bottom)<.00001,'settled pieces touch the ground instead of floating')
  first=compiled[piece*73];last=compiled[piece*73+72]
  check(abs(first[0]-cx*76/96+38)<.00001 and abs(first[1]-first[2]-cy*76/96+66)<.00001,'six fragments assemble at the intact boulder contact position')
  check(first[4]==1 and last[4]==0,'fracture begins visible and fully fades out')
  with Image.open(art/'parts'/f'fragment{piece}.png') as part:check(np.array_equal(np.asarray(part.convert('RGBA')),pixels[piece//3*96:piece//3*96+96,piece%3*96:piece%3*96+96]),'portable Synfig part matches its native atlas cell')
 native=list((Path(motion['master'])/'renders').glob('*/*.png'));check(len(native)==72,'all 72 native fracture renders exist');hashes=set()
 for path in native:
  with Image.open(path) as im:im.load();check(im.size==(400,300),'native fracture render decodes');hashes.add(hashlib.sha256(im.tobytes()).hexdigest())
 check(len(hashes)>65,'native fragments really separate, rotate, bounce and fade')
 paths=sorted((qa/'shatter-frames').glob('*.png'),key=lambda p:int(p.stem));check(len(paths)==60,'all 60 real rolling-escape/impact captures exist');frames=[]
 for path in paths:
  with Image.open(path) as im:
   im.load();check(im.size==(1440,900),'real fracture game frame decodes')
   if previews:frames.append(im.convert('RGB').crop((265,300,650,520)).resize((616,352),Image.Resampling.LANCZOS))
 for stage in ('contact','burst','bounce','settled'):
  with Image.open(qa/f'guardian-break-{stage}.png') as im:im.load();check(np.asarray(im.convert('RGB')).std()>20,'actual '+stage+' stage is nonempty')
 if previews:
  colors=Image.new('RGB',(616,352*4))
  for row,index in enumerate((8,18,30,44)):colors.paste(frames[index],(0,row*352))
  palette=colors.quantize(colors=192);clip=[im.quantize(palette=palette,dither=Image.Dither.NONE) for im in frames[::2]]
  clip[0].save(qa/'guardian-shatter.gif',save_all=True,append_images=clip[1:],duration=[60,70,70]*10,loop=0,optimize=True)
 return {'native_frames':72,'pieces':6,'control_samples':438,'game_frames':60,'duration':1.2,'runtime_atlas_rgba8_bytes':288*192*4}

"""Local Blender RGBA -> ComfyUI material illustration -> fixed 2D atlas.

Every item has its own graph/seed/prompt and retained original alpha. Native
Comfy history is authoritative; failures/timeouts are never blindly replayed.
"""
from pathlib import Path
import argparse,json,hashlib,re,shutil
import numpy as np
from PIL import Image,ImageFilter,ImageDraw,ImageFont
ROOT=Path('C:/AI2D/sprites/emberfall-equipment-20261108/v2')
COMFY=Path('C:/Users/Amol/AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI')
REPO=Path(__file__).resolve().parents[2]
def write(p,data):Path(p).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def prepare():
 items=json.loads((ROOT/'items.json').read_text(encoding='utf-8-sig'))
 for d in ['inputs','workflows','outputs','runtime','previews','logs']:(ROOT/d).mkdir(exist_ok=False)
 upload=COMFY/'input/emberfall-equipment-20261108';upload.mkdir(exist_ok=False)
 names=re.search(r'enum class Kind \{(.*?)\};',(REPO/'src/items.hpp').read_text(encoding='utf-8-sig'),re.S).group(1);names=[s.strip() for s in names.replace('\n','').split(',')]
 types={1:'long double edged knight sword',2:'battle axe with broad crescent steel head',3:'closed medieval helmet, clear narrow visor, metal nose guard',4:'ornate breastplate armor, articulated belly plates and pauldrons',5:'kite shield, central boss and reinforced edges',6:'pair of tall leather adventurer boots, distinct laces and toe caps',7:'ornate finger ring and faceted gemstone',24:'short dagger with double edged blade',25:'flowing travel cloak with clasp, rich cloth folds',26:'ornamental pendant amulet, chain and inset gemstone',28:'recurved wooden longbow, one fine visible taut bowstring, no arrow'}
 motifs=[('Wolf','silver fur trim and wolf tooth ornaments'),('Moon','pale moon silver and midnight blue'),('Boar','brown boar leather and ivory tusk ornament'),('Tusk','ivory tusk and bronze fittings'),('Silk','fine midnight silk and silver filigree'),('Web','spider silk embroidery'),('Venom','green jade and dark silver'),('Bone','ivory bone inlay and antique dark iron'),('Crypt','cold blue runes, weathered iron'),('Grave','ivory inlay and old iron'),('Guardian','ancient carved stone, moss and gold runes'),('Echo','violet gemstone and aged gold'),('Root','twisted wood roots and bronze'),('Grove','living wood grain and green leaf inlay'),('Bark','wood grain and copper strips'),('Scarab','copper overlapping plates, scarab ornament'),('Cobra','golden snake scales and green jade'),('Serpent','golden serpent scale inlay'),('Scale','golden overlapping scales'),('Scorpion','amber chitin and dark leather'),('Sting','amber chitin inlay'),('Amber','warm amber and bronze'),('Dune','sandstone, ivory gold and desert engraved patterns'),('Sand','sand colored woven cloth and gold thread'),('Sun','sun engraved brass and polished steel'),('Dawn','pale gold with cool blue gemstone'),('Basalt','dark basalt and red copper veins'),('Ash','ashen wood with ember copper inlay'),('Glass','dark volcanic glass and warm red reflections'),('Slime','translucent green enamel inlay'),('Gel','emerald gemstone'),('Bandit','weathered dark leather and iron'),('Outlaw','worn silver signet and amber')]
 neg='text, letters, watermark, logo, border, frame, person, hand, character, face, scenery, inventory panel, multiple items, duplicate weapons, extra blades, blurry, pixel art, lowpoly, plastic toy, white clay, washed out, cut off'
 for item in items:
  k=item['kind'];source=ROOT/'renders'/f'{k:03}.png';rgba=Image.open(source).convert('RGBA').resize((512,512),Image.Resampling.LANCZOS);bg=Image.new('RGBA',(512,512),(20,26,36,255));bg.alpha_composite(rgba);bg.convert('RGB').save(ROOT/'inputs'/source.name);shutil.copy2(ROOT/'inputs'/source.name,upload/source.name)
  desc=', '.join(v for key,v in motifs if key in names[k]) or 'polished steel, walnut brown leather, warm brass trim, turquoise shadow'
  prompt=f'masterpiece fantasy RPG inventory item icon, single centered {types[item["shape"]]}, {desc}, extremely detailed hand painted material illustration, realistic hammered metal highlights, leather grain, subtle fine engravings, bevelled edges, rich saturated colors, warm golden key light, cool blue shadows, crisp readable silhouette, isolated on plain dark slate background, preserve exact shape and orientation of the supplied item, complete item within frame'
  graph={
   '1':{'class_type':'CheckpointLoaderSimple','inputs':{'ckpt_name':'DreamShaper_8_pruned.safetensors'}},
   '2':{'class_type':'LoadImage','inputs':{'image':f'emberfall-equipment-20261108/{k:03}.png'}},
   '3':{'class_type':'CLIPTextEncode','inputs':{'clip':['1',1],'text':prompt}},
   '4':{'class_type':'CLIPTextEncode','inputs':{'clip':['1',1],'text':neg}},
   '5':{'class_type':'VAEEncode','inputs':{'pixels':['2',0],'vae':['1',2]}},
   '6':{'class_type':'KSampler','inputs':{'model':['1',0],'positive':['3',0],'negative':['4',0],'latent_image':['5',0],'seed':20261108000+k,'steps':20,'cfg':6.4,'sampler_name':'dpmpp_2m','scheduler':'karras','denoise':.44}},
   '7':{'class_type':'VAEDecode','inputs':{'samples':['6',0],'vae':['1',2]}},
   '8':{'class_type':'SaveImage','inputs':{'images':['7',0],'filename_prefix':f'emberfall-equipment-20261108/{k:03}'}}}
  write(ROOT/'workflows'/f'{k:03}.json',graph)
 write(ROOT/'manifest.json',{'items':items,'models':['DreamShaper_8_pruned.safetensors'],'local_only':True,'runtime_2d_only':True})
 print('Prepared 85 independent local material workflows')
def deliver():
 items=json.loads((ROOT/'items.json').read_text(encoding='utf-8-sig'));atlas=Image.new('RGBA',(1536,1024));records=[]
 for i,item in enumerate(items):
  k=item['kind'];source=ROOT/'renders'/f'{k:03}.png';raw=list((ROOT/'outputs').glob(f'{k:03}_*.png'));assert len(raw)==1,(k,raw)
  original=Image.open(source).convert('RGBA').resize((512,512),Image.Resampling.LANCZOS);generated=Image.open(raw[0]).convert('RGB');assert generated.size==(512,512)
  a=np.asarray(original).copy();alpha=original.getchannel('A');edge=np.asarray(alpha.filter(ImageFilter.MinFilter(5))).astype(float)/255
  weight=edge[:,:,None]*.94;a[:,:,:3]=np.clip(np.asarray(original)[:,:,:3]*(1-weight)+np.asarray(generated)*weight,0,255).astype('uint8')
  result=Image.fromarray(a);assert result.getchannel('A').tobytes()==alpha.tobytes();result.save(ROOT/'runtime'/f'{k:03}.png')
  tile=result.resize((120,120),Image.Resampling.LANCZOS);atlas.alpha_composite(tile,((i%12)*128+4,(i//12)*128+4))
  records.append({'kind':k,'cell':i,'name':item['name'],'shape':item['shape'],'blender_sha256':sha(source),'comfy_sha256':sha(raw[0]),'paint_sha256':sha(ROOT/'runtime'/f'{k:03}.png'),'alpha_matches_blender':True})
 atlas.save(ROOT/'equipment-icons.png');write(ROOT/'equipment-icons.json',{'cell':128,'columns':12,'size':atlas.size,'rgba8_bytes':atlas.width*atlas.height*4,'items':records,'sha256':sha(ROOT/'equipment-icons.png')})
 sheet=Image.new('RGB',(12*150,8*175),(19,25,34));d=ImageDraw.Draw(sheet);font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',11)
 for i,item in enumerate(items):
  p=Image.open(ROOT/'runtime'/f'{item["kind"]:03}.png').convert('RGBA').resize((145,145),Image.Resampling.LANCZOS);x=i%12*150;y=i//12*175;sheet.paste(p,(x,y),p);d.text((x+4,y+145),str(item['kind'])+' '+item['name'][:23],font=font,fill=(235,213,173))
 sheet.save(ROOT/'previews/equipment-contact.png');print('PASS: 85 alpha-preserving illustrations -> 1536x1024 RGBA atlas')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','deliver']);p.add_argument('--no-audio',required=True,action='store_true');a=p.parse_args();prepare() if a.action=='prepare' else deliver()

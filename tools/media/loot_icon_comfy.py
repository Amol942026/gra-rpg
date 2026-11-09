"""Native Krita cutouts -> local material painting -> inventory atlas.
Keeps native masters and exact alpha; never touches the 85 equipment cells.
"""
from pathlib import Path
import json,argparse,shutil,hashlib
import numpy as np
from PIL import Image,ImageDraw,ImageFont,ImageFilter
ROOT=Path('C:/AI2D/sprites/emberfall-loot-20261109')
COMFY=Path('C:/Users/Amol/AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI')
ITEMS=[(8,'Mikstura zdrowia','clear glass potion flask, deep crimson red health liquid, cork stopper, brass band, small gold medical cross medallion'),(9,'Mikstura many','clear glass potion flask, sapphire blue mana liquid, cork stopper, brass band, small round gold medallion'),(10,'Odłamek echa','faceted teal crystal shard, translucent quartz, cool inner light, fine natural crystal fracture detail'),(11,'Złote monety','four stacked ancient gold coins, engraved sun markings, round milled edges, polished gold highlights'),(12,'Kieł wilka','single curved ivory wolf fang, slightly rough broken root, naturally tapered pointed end, fine ivory striations'),(13,'Sierść wilka','folded silver grey wolf fur pelt, many fine visible individual hairs, soft tufted edges'),(14,'Kieł dzika','single thick curved boar tusk, polished ivory, weathered root and fine surface grain'),(15,'Skóra dzika','folded brown boar leather hide, natural leather pores, wrinkles and subtle bristles'),(16,'Pajęcza nić','wooden spool wrapped with fine pale spider silk thread, individual strands, loose silk end'),(17,'Woreczek jadu','closed organic green venom gland, dark green membranes and subtle veins, tied tan neck'),(18,'Fragment kości','weathered broken ivory bone, porous bone ends and smooth shaft'),(19,'Śluz szlama','glossy emerald green slime globule, translucent gelatin, small trapped bubbles and wet highlights'),(20,'Prastara kora','piece of ancient tree bark, deep rugged wood grooves, brown layers with small patches of moss'),(21,'Esencja lasu','faceted luminous emerald forest crystal, soft green inner light and small golden magical sparks'),(22,'Odznaka rozbójnika','antique bronze shield shaped badge, worn steel inset and two crossed small swords'),(23,'Kamień runiczny','weathered carved grey rune stone, one glowing cyan angular rune, porous stone grain'),(27,'Borowik','single porcini mushroom, warm brown rounded cap, cream thick stem, organic pores and gills'),(29,'Strzały','bundle of exactly three arrows, walnut shafts, sharp steel triangular arrowheads, pale feather fletching, leather binding'),(30,'Część Serca Świtu','large amber gold faceted artifact crystal, ancient broken golden circular setting, bright warm inner light'),(31,'Żądło skorpiona','curved amber scorpion tail stinger, natural segmented chitin, sharp dark venom tip'),(32,'Pancerz skarabeusza','one oval copper teal beetle carapace plate, central seam and engraved natural segment grooves'),(33,'Łuska kobry','one large golden cobra scale, diamond shaped layered scale, central ridge and finely etched lamellae'),(34,'Esencja piasku','glass corked bottle filled with glowing golden enchanted sand, tiny granular sand texture, brass band and gold medallion')]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write(p,d):Path(p).write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def prepare():
 for d in ['inputs','workflows','outputs','runtime','previews','logs']:(ROOT/d).mkdir(exist_ok=False)
 upload=COMFY/'input/emberfall-loot-20261109';upload.mkdir(exist_ok=False)
 native=Image.open(ROOT/'krita/remaining-inventory-items.png').convert('RGBA')
 for index,(kind,name,desc) in enumerate(ITEMS):
  x=index%6*256;y=index//6*256;seed=native.crop((x,y,x+256,y+256)).resize((512,512),Image.Resampling.LANCZOS);seed.save(ROOT/'inputs'/f'{kind:03}-alpha.png');bg=Image.new('RGBA',(512,512),(20,26,36,255));bg.alpha_composite(seed);bg.convert('RGB').save(ROOT/'inputs'/f'{kind:03}.png');shutil.copy2(ROOT/'inputs'/f'{kind:03}.png',upload/f'{kind:03}.png')
  prompt=f'beautiful detailed hand painted fantasy RPG inventory item illustration, {desc}, single complete centered object, rich physical material detail, fine realistic texture, polished highlights, warm golden studio light, cool slate blue shadow, crisp readable silhouette, preserve the input shape and orientation, isolated on plain dark slate background'
  graph={'1':{'class_type':'CheckpointLoaderSimple','inputs':{'ckpt_name':'DreamShaper_8_pruned.safetensors'}},'2':{'class_type':'LoadImage','inputs':{'image':f'emberfall-loot-20261109/{kind:03}.png'}},'3':{'class_type':'CLIPTextEncode','inputs':{'clip':['1',1],'text':prompt}},'4':{'class_type':'CLIPTextEncode','inputs':{'clip':['1',1],'text':'text, letters, label writing, watermark, logo, frame, border, UI, hand, person, character, landscape, pixel art, flat shading, low detail, plastic toy, extra items, cropped, blurry'}},'5':{'class_type':'VAEEncode','inputs':{'pixels':['2',0],'vae':['1',2]}},'6':{'class_type':'KSampler','inputs':{'model':['1',0],'positive':['3',0],'negative':['4',0],'latent_image':['5',0],'seed':20261109000+kind,'steps':24,'cfg':6.6,'sampler_name':'dpmpp_2m','scheduler':'karras','denoise':.5}},'7':{'class_type':'VAEDecode','inputs':{'samples':['6',0],'vae':['1',2]}},'8':{'class_type':'SaveImage','inputs':{'images':['7',0],'filename_prefix':f'emberfall-loot-20261109/{kind:03}'}}}
  write(ROOT/'workflows'/f'{kind:03}.json',graph)
 for batch in range(2):
  graph={}
  for i,(kind,_,_) in enumerate(ITEMS[batch*12:(batch+1)*12]):
   original=json.loads((ROOT/'workflows'/f'{kind:03}.json').read_text());mapping={old:str(i*10+int(old)) for old in original};mapping['1']='1';mapping['4']='4'
   for key,node in original.items():
    if key in ('1','4') and i:continue
    for field,value in node['inputs'].items():
     if isinstance(value,list):node['inputs'][field]=[mapping[value[0]],value[1]]
    graph[mapping[key]]=node
  write(ROOT/'workflows'/f'batch-{batch}.json',graph)
 print('Prepared 23 local item workflows in two sequential batches')
def deliver():
 atlas=Image.new('RGBA',(1024,384));rows=[];contact=Image.new('RGB',(6*180,4*200),(20,26,36));draw=ImageDraw.Draw(contact);font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',13)
 for index,(kind,name,_) in enumerate(ITEMS):
  refined=list((ROOT/'refined-outputs').glob(f'{kind:03}_*.png'))
  raw=refined or list((ROOT/'outputs').glob(f'{kind:03}_*.png'));assert len(raw)==1,(kind,raw);original=Image.open(ROOT/'inputs'/f'{kind:03}-alpha.png').convert('RGBA');generated=Image.open(raw[0]).convert('RGB');assert generated.size==(512,512)
  arr=np.array(original);mask=np.asarray(original.getchannel('A').filter(ImageFilter.MinFilter(5))).astype(float)/255;w=mask[:,:,None]*.96;arr[:,:,:3]=np.clip(arr[:,:,:3]*(1-w)+np.asarray(generated)*w,0,255).astype('uint8');paint=Image.fromarray(arr);assert paint.getchannel('A').tobytes()==original.getchannel('A').tobytes();paint.save(ROOT/'runtime'/f'{kind:03}.png')
  atlas.alpha_composite(paint.resize((120,120),Image.Resampling.LANCZOS),(index%8*128+4,index//8*128+4));small=paint.resize((175,175),Image.Resampling.LANCZOS);contact.paste(small,(index%6*180,index//6*200),small);draw.text((index%6*180+5,index//6*200+176),name,font=font,fill=(237,211,163))
  rows.append({'kind':kind,'name':name,'cell':index,'comfy_sha256':sha(raw[0]),'paint_sha256':sha(ROOT/'runtime'/f'{kind:03}.png'),'alpha_preserved':True})
 atlas.save(ROOT/'loot-icons.png');contact.save(ROOT/'previews/loot-contact.png');write(ROOT/'loot-icons.json',{'size':atlas.size,'cell':128,'columns':8,'rgba8_bytes':atlas.width*atlas.height*4,'items':rows,'sha256':sha(ROOT/'loot-icons.png')});print('PASS: 23 new material illustrations, alpha retained, 1024x384 atlas')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','deliver']);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();prepare() if a.action=='prepare' else deliver()

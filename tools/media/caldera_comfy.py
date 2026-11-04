"""Local ComfyUI img2img material pass, projected onto existing Synfig cutouts.

Preserves exact alpha, dimensions, pivots and hidden overlap pixels. Generation
uses the complete assembled figure, never independently generated animation frames.
Native animation and gameplay files are not touched. All stages retain receipts.
"""
from pathlib import Path
import argparse, hashlib, json, shutil
import numpy as np
from PIL import Image, ImageFilter, ImageDraw, ImageFont

REPO=Path(__file__).resolve().parents[2]
NAMES=['slag-crawler','lava-salamander','fire-imp','ash-wraith','obsidian-sentinel','ember-drake','varkhor']
DESCRIPTIONS=[
 'volcanic armored crawler, broad overlapping obsidian carapace plates, carved basalt, chipped edges, amber molten fissures, insect legs',
 'lava salamander, red copper scales, reptilian face, black dorsal plates, long muscular tail, molten orange belly, four clawed legs',
 'small horned fire imp demon, leathery copper skin, mischievous angular face, gold ember eyes, clawed fingers, leather belt, burning right fist',
 'ash wraith, charred layered tattered hooded robes, deep folds, ember eyes inside dark hood, floating ashen spirit, scorched fabric',
 'obsidian sentinel warrior, heavy engraved black steel armor, volcanic helmet, layered shoulder guards, battered shield and long sword, warm gold runes',
 'ember drake, powerful dark red reptile, leathery bat wings, wing membrane veins, black horns, individual overlapping scales, molten amber eyes',
 'Varkhor the caldera titan, massive ancient basalt armor, huge shoulders and horns, magma core inside chest, orange glowing carved runes, volcanic stone demon',
]
DIRS=['right three-quarter view','front right three-quarter view','front view','front left three-quarter view','left three-quarter view','back left three-quarter view','back view','back right three-quarter view']
STYLE='hand painted 2D isometric dark fantasy RPG creature sprite, detailed material illustration, sculpted painted shading, crisp edges, fine surface texture, copper orange rim light from lava below, cool blue violet shadows, dramatic readable volumes, full body, centered, isolated on plain dark grey background, preserve pose and silhouette'
NEG='photograph, realistic photo, 3d model, scenery, landscape, text, logo, letters, watermark, frame, border, extra limbs, extra heads, additional weapons, blurry, noisy, flat shading, low detail, plastic, adorable, chibi'

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write(p,d):Path(p).write_text(json.dumps(d,ensure_ascii=False,indent=2),encoding='utf-8')
def order(d):return [0,1,2,4,5,3] if d>=5 else list(range(6))
def assemble(im,d):
 c=im.width//8;out=Image.new('RGBA',(c,c))
 for p in order(d):out.alpha_composite(im.crop((d*c,p*c,(d+1)*c,(p+1)*c)))
 return out
def workflow(name,d):
 return {
  '1':{'class_type':'CheckpointLoaderSimple','inputs':{'ckpt_name':'DreamShaper_8_pruned.safetensors'}},
  '2':{'class_type':'LoadImage','inputs':{'image':f'caldera-comfy-20261104/{name}-{d}.png'}},
  '3':{'class_type':'CLIPTextEncode','inputs':{'clip':['1',1],'text':STYLE+', '+DESCRIPTIONS[NAMES.index(name)]+', '+DIRS[d]}},
  '4':{'class_type':'CLIPTextEncode','inputs':{'clip':['1',1],'text':NEG}},
  '5':{'class_type':'VAEEncode','inputs':{'pixels':['2',0],'vae':['1',2]}},
  '6':{'class_type':'KSampler','inputs':{'model':['1',0],'positive':['3',0],'negative':['4',0],'latent_image':['5',0],'seed':202611040+NAMES.index(name)*10+d,'steps':28,'cfg':6.5,'sampler_name':'dpmpp_2m','scheduler':'karras','denoise':.42}},
  '7':{'class_type':'VAEDecode','inputs':{'samples':['6',0],'vae':['1',2]}},
  '8':{'class_type':'SaveImage','inputs':{'images':['7',0],'filename_prefix':f'caldera-comfy-20261104/{name}-{d}'}}
 }
def prepare(root,comfy):
 if root.exists():raise RuntimeError('Use a fresh source directory')
 for folder in ['backup','inputs','workflows','outputs','runtime','previews','logs']:(root/folder).mkdir(parents=True)
 upload=comfy/'input/caldera-comfy-20261104';upload.mkdir(parents=True,exist_ok=False)
 records={};originals={}
 for name in NAMES:
  src=REPO/f'assets/volcano/{name}.png';shutil.copy2(src,root/'backup'/src.name);im=Image.open(src).convert('RGBA');c=im.width//8
  originals[name]={'sha256':sha(src),'size':im.size,'cell':c}
  full=[assemble(im,d) for d in range(8)];boxes=[i.getbbox() for i in full]
  box=(max(0,min(b[0] for b in boxes)-4),max(0,min(b[1] for b in boxes)-4),min(c,max(b[2] for b in boxes)+4),min(c,max(b[3] for b in boxes)+4))
  scale=min(448/(box[2]-box[0]),448/(box[3]-box[1]));size=(round((box[2]-box[0])*scale),round((box[3]-box[1])*scale));off=((512-size[0])//2,(512-size[1])//2)
  records[name]={'box':box,'size':size,'offset':off}
  for d,full_image in enumerate(full):
   canvas=Image.new('RGBA',(512,512),(31,33,41,255));canvas.alpha_composite(full_image.crop(box).resize(size,Image.Resampling.LANCZOS),off)
   path=root/'inputs'/f'{name}-{d}.png';canvas.convert('RGB').save(path);shutil.copy2(path,upload/path.name)
   write(root/'workflows'/f'{name}-{d}.json',workflow(name,d))
 write(root/'projection.json',records);write(root/'originals.json',originals)
 contact(root,'before',root/'backup')
 print(json.dumps({'root':str(root),'inputs':56,'source_atlases':7,'no_audio':True}))
def project(root,partial=False):
 records=json.loads((root/'projection.json').read_text());logs=[]
 for name in NAMES:
  original=Image.open(root/'backup'/f'{name}.png').convert('RGBA');out=original.copy();c=original.width//8;rec=records[name];box=rec['box'];w,h=rec['size'];ox,oy=rec['offset']
  for d in range(8):
   candidates=list((root/'outputs').glob(f'{name}-{d}_*.png'))
   if partial and not candidates:continue
   assert len(candidates)==1,(name,d,candidates)
   raw=Image.open(candidates[0]).convert('RGB');assert raw.size==(512,512)
   generated=Image.new('RGB',(c,c));generated.paste(raw.crop((ox,oy,ox+w,oy+h)).resize((box[2]-box[0],box[3]-box[1]),Image.Resampling.LANCZOS),(box[0],box[1]));paint=np.asarray(generated).astype(float)
   front=np.zeros((c,c),dtype=float)
   for p in reversed(order(d)):
    region=(d*c,p*c,(d+1)*c,(p+1)*c);part=original.crop(region);arr=np.array(part);alpha=arr[:,:,3].astype(float)/255
    # The unoccluded material receives the new paint. Retain hidden joint
    # overlaps and edge pixels, so articulation never reveals painted body
    # details on an arm or leaves grey fringes. Keep original emissive accents.
    visibility=(1-front);mask=Image.fromarray(np.uint8(visibility*255)).filter(ImageFilter.GaussianBlur(.65));weight=np.asarray(mask).astype(float)/255
    edge=np.asarray(part.getchannel('A').filter(ImageFilter.MinFilter(3))).astype(float)/255
    weight*=edge*.94
    color=arr[:,:,:3].astype(float);result=color*(1-weight[:,:,None])+paint*weight[:,:,None]
    emissive=(color[:,:,0]>190)&(color[:,:,0]>color[:,:,2]*1.5)&(color[:,:,1]>90)
    result[emissive]=np.maximum(result[emissive],color[emissive]*.94)
    arr[:,:,:3]=np.clip(result,0,255).astype(np.uint8);out.paste(Image.fromarray(arr),region[:2]);front=alpha+front*(1-alpha)
   logs.append({'name':name,'direction':d,'output':str(candidates[0]),'sha256':sha(candidates[0])})
  dest=root/'runtime'/f'{name}.png';out.save(dest)
  assert out.size==original.size and np.array_equal(np.array(out)[:,:,3],np.array(original)[:,:,3])
 write(root/'generation-outputs.json',logs);contact(root,'after',root/'runtime');print(f'Projected {len(logs)} outputs into 7 alpha-identical cutout atlases; partial={partial}')
def contact(root,label,source):
 font=ImageFont.truetype('C:/Windows/Fonts/arial.ttf',19);sheet=Image.new('RGB',(7*270,360),(27,29,38));draw=ImageDraw.Draw(sheet)
 for i,name in enumerate(NAMES):
  im=assemble(Image.open(source/f'{name}.png').convert('RGBA'),1);b=im.getbbox();sprite=im.crop(b);sprite.thumbnail((245,275),Image.Resampling.LANCZOS);sheet.paste(sprite,(i*270+(270-sprite.width)//2,302-sprite.height),sprite);draw.text((i*270+8,321),name,font=font,fill=(232,205,157))
 sheet.save(root/'previews'/f'{label}.png')
def deploy(root):
 assert len(json.loads((root/'generation-outputs.json').read_text()))==56,'All eight directions must be generated before deployment'
 rows=[]
 for name in NAMES:
  dst=REPO/f'assets/volcano/{name}.png';original=root/'backup'/dst.name;src=root/'runtime'/dst.name
  assert sha(dst)==sha(original),'Active source changed since backup: '+name
  assert src.is_file()
 for name in NAMES:
  dst=REPO/f'assets/volcano/{name}.png';src=root/'runtime'/dst.name;shutil.copy2(src,dst)
  rows.append({'path':str(dst.relative_to(REPO)),'before':sha(root/'backup'/dst.name),'after':sha(dst)})
 write(root/'deployment.json',{'files':rows,'status':'APPLIED','local_only':True,'alpha_unchanged':True})
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','project','preview','deploy']);p.add_argument('--root',type=Path,required=True);p.add_argument('--comfy',type=Path);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 if a.action=='prepare':prepare(a.root,a.comfy)
 elif a.action in ('project','preview'):project(a.root,a.action=='preview')
 else:deploy(a.root)

"""Independently validate new native Krita materials, item IDs and knee extension."""
import argparse,csv,hashlib,json,math,re,zipfile
from pathlib import Path
from PIL import Image

def main():
 p=argparse.ArgumentParser();p.add_argument('--qa',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 root=Path(__file__).resolve().parents[2];art=root/'art/items/expansion';catalog=json.loads((art/'catalog.json').read_text(encoding='utf-8'));checks=[]
 def check(ok,label):checks.append({'pass':bool(ok),'check':label})
 meta=json.loads((art/'provenance.json').read_text(encoding='utf-8'))
 check(meta['status']=='PASS' and meta['runtime_textures_added']==0,'native authored inlays add no runtime textures')
 check(hashlib.sha256(Path(meta['png']).read_bytes()).hexdigest()==meta['png_sha256'],'saved native material export hash')
 with zipfile.ZipFile(meta['master']) as archive:
  xml=archive.read('maindoc.xml').decode('utf-8');check(all(x['id']+' / materials and inlay' in xml for x in catalog),'all 36 new equipment layers are editable in the actual KRA')
  with archive.open('mergedimage.png') as stream,Image.open(stream) as merged,Image.open(meta['png']) as exported:check(merged.convert('RGBA').tobytes()==exported.convert('RGBA').tobytes(),'saved KRA composite agrees exactly with PNG export')
 header=(root/'src/generated/equipment_paints.hpp').read_text(encoding='utf-8');backup=root/'art/pipeline/backups/guardian-extension-loot-20261027-133708/src/generated/equipment_paints.hpp'
 def rows(code,name):return re.search(r'inline constexpr \w+ '+name+r'\[\d+\]=\{\n(.*?)\n\};',code,re.S).group(1).splitlines()
 old=backup.read_text(encoding='utf-8');newpaints=rows(header,'paints');newspans=rows(header,'spans');newstrokes=rows(header,'strokes')
 for name in ('paints','spans','strokes'):check(rows(header,name)[:len(rows(old,name))]==rows(old,name),'previous authored '+name+' preserved exactly')
 check(len(newpaints)==len(newspans)==99,'every stable item ID has an in-bounds appearance entry')
 with Image.open(meta['png']) as im:
  im=im.convert('RGBA')
  for index,item in enumerate(catalog):
   x,y=index%8*64,index//8*64;colors=['0x%02x%02x%02x'%im.getpixel((x+8+n*16,y+4))[:3] for n in range(4)]
   check(all(c in newpaints[63+index] for c in colors),'runtime material colors come from Krita pixels: '+item['id'])
   spans=list(map(int,re.findall(r'\d+',newspans[63+index])));check(spans[1]>0 and spans[0]+spans[1]<=len(newstrokes),'new inlay span references real native strokes')
 check(hashlib.sha256((root/'src/generated/equipment_paints.hpp').read_bytes()).hexdigest()==meta['generated_sha256'],'generated appearance header matches provenance')
 itemrows=list(csv.DictReader((a.qa/'items.tsv').open(encoding='utf-8'),delimiter='\t'));check(len(itemrows)==36,'all new items exercised by production UI')
 chance={'Common':60,'Rare':15,'VeryRare':4,'ExtremelyRare':1}
 for index,(item,row) in enumerate(zip(catalog,itemrows)):
  check(int(row['id'])==63+index and row['name']==item['name'] and int(row['beast'])==item['beast'] and int(row['chance'])==chance[item['rarity']] and int(row['power'])==item['power'],'actual runtime item and rarity data agree with catalogue: '+item['id'])
 images=list((a.qa/'equipment').glob('*.png'));check(len(images)==36 and len({hashlib.sha256(x.read_bytes()).hexdigest() for x in images})==36,'every new item changes the rendered equipped hero uniquely')
 for path in images:
  with Image.open(path) as im:im.load();box=im.getchannel('A').getbbox();check(im.size==(400,320) and box and box[0]>1 and box[1]>1 and box[2]<399 and box[3]<319,'equipped hero is fully visible: '+path.name)
 for path in (a.qa/'items').glob('*.png'):
  with Image.open(path) as im:im.load();check(im.size==(1440,900),'real loot tooltip scene decodes: '+path.name)
 for page in range(1,4):
  with Image.open(a.qa/f'new-items-{page}.png') as im:im.load();check(im.size==(1440,900),'new gear overview page is complete')
 feet=list(csv.DictReader((a.qa/'feet.csv').open(encoding='utf-8')));byframe={};angles=[]
 for row in feet:
  if row['sequence'] not in ('chase','patrol'):continue
  if float(row['phase'])>.64:continue
  h,f,z,k,kz=(float(row[x]) for x in ('hip_z','foot_forward','foot_z','knee_forward','knee_z'))
  u=(-k,h-kz);v=(f-k,z-kz);cos=sum(x*y for x,y in zip(u,v))/(math.hypot(*u)*math.hypot(*v));angle=math.degrees(math.acos(max(-1,min(1,cos))));byframe.setdefault((row['sequence'],row['frame']),[]).append(angle)
 for key,values in byframe.items():check(max(values)>179.8,'support knee fully straight in actual captured phase: '+str(key));angles.append(max(values))
 check(len(byframe)==240 and min(angles)>179.8,'at least one support knee is extended throughout both real walking sequences')
 for folder,name in [('qa-v39','rarity-ui-tests.txt'),('qa-v42','trade-tests.txt')]:
  text=(a.qa.parent/folder/name).read_text(encoding='utf-8');check('FAIL' not in text and 'failures: 0' in text,'entire expanded catalogue passes '+folder)
 summary=json.loads((a.qa/'summary.json').read_text(encoding='utf-8'));check(summary['status']=='PASS' and summary['failures']==0 and summary['audio_opened'] is False,'combined actual renderer tests pass silently')
 result={'status':'PASS' if all(x['pass'] for x in checks) else 'FAIL','checks':len(checks),'failures':[x for x in checks if not x['pass']],'new_items':36,'equipment_variants':75,'knee_extended_frames':len(byframe),'min_support_knee_degrees':min(angles),'runtime_textures_added':0,'audio_opened':False}
 (a.qa/'expansion-verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result));raise SystemExit(0 if result['status']=='PASS' else 2)
if __name__=='__main__':main()

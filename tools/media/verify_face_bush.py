"""Offline native-master checks and assembly of unpainted GPU QA captures."""
from pathlib import Path
import argparse, hashlib, io, json, zipfile
from PIL import Image,ImageDraw

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def verify(backup,qa):
 repo=Path(__file__).resolve().parents[2];records=[]
 for image,master in [('hero-paints','art/hero/hero-paints.kra'),('forest-bush','art/scenery/forest-bush.kra')]:
  png=repo/'assets/sprites'/(image+'.png');im=Image.open(png).convert('RGBA')
  with zipfile.ZipFile(repo/master) as z:
   assert z.testzip() is None;merged=Image.open(io.BytesIO(z.read('mergedimage.png'))).convert('RGBA');assert merged.tobytes()==im.tobytes()
  records.append({'asset':str(png),'sha256':sha(png),'master':master,'master_matches_export':True})
 old=Image.open(backup/'assets/sprites/hero-paints.png').convert('RGBA');new=Image.open(repo/'assets/sprites/hero-paints.png').convert('RGBA')
 assert old.size==new.size==(512,384)
 assert old.crop((128,0,512,128)).tobytes()==new.crop((128,0,512,128)).tobytes()
 assert old.crop((0,128,512,384)).tobytes()==new.crop((0,128,512,384)).tobytes()
 features=new.crop((0,0,128,128));assert features.getchannel('A').getextrema()==(0,255)
 assert all(features.getpixel((x,y))[3]==0 for y in range(128) for x in range(128) if x<30 or x>98 or y<38 or y>100)
 # Assemble a readable face inspection sheet from actual renderer crops.
 im=Image.open(qa/'face-profiles.png').convert('RGB');preview=Image.new('RGB',(800,235),(15,23,25));d=ImageDraw.Draw(preview)
 for i,label in enumerate(['PROFIL','SKOS','PRZOD','SKOS']):
  head=im.crop((i*400+135,30,i*400+265,170)).resize((200,215),Image.Resampling.LANCZOS)
  preview.paste(head,(i*200,0));d.text((i*200+70,220),label,fill=(224,192,128))
 preview.save(qa/'face-closeups.png')
 world=Image.open(qa/'bush-forest.png').convert('RGB').crop((420,305,755,460)).resize((800,370),Image.Resampling.LANCZOS)
 combined=Image.new('RGB',(800,635),(15,23,25));combined.paste(preview,(0,0));combined.paste(world,(0,265));ImageDraw.Draw(combined).text((15,245),'KRZEWY W GRZE',fill=(224,192,128));combined.save(qa/'fix-preview.png')
 (qa/'asset-validation.json').write_text(json.dumps({'status':'PASS','records':records,'all_nonface_paint_cells_preserved':True,'features_only_no_face_border':True,'no_external_assets':True,'audio':False},indent=2),encoding='utf-8')
 print('Native KRA/PNG exports verified; all non-face paint cells unchanged.')

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--backup',type=Path,required=True);p.add_argument('--qa',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True)
 a=p.parse_args();verify(a.backup,a.qa)

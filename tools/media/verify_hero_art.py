"""Independent offline checks and silent preview assembly from native captures."""
from pathlib import Path
import argparse, hashlib, json, re
from PIL import Image,ImageDraw
from hero_synfig import compile_tracks

def verify(project,qa):
 manifests=sorted((project/'renders').glob('*/render-manifest.json'))
 assert len(manifests)==6
 count=0;records=[]
 for p in manifests:
  m=json.loads(p.read_text(encoding='utf-8-sig'));directory=p.parent;frames=[]
  assert m['Status']=='PASS' and m['ActualFrames']==m['ExpectedFrames']
  for record in m['Frames']:
   source=directory/record['Name'];assert hashlib.sha256(source.read_bytes()).hexdigest()==record['Sha256']
   with Image.open(source) as im:
    im.load();assert im.size==(256,320);im=im.convert('RGBA');assert im.getchannel('A').getbbox();frames.append(im.copy())
  hashes={hashlib.sha256(i.tobytes()).hexdigest() for i in frames};assert len(hashes)>1
  count+=len(frames);records.append({'name':Path(m['Project']).stem,'frames':len(frames),'unique_pixel_frames':len(hashes)})
 assert count==354
 # Recompile independently from the saved native files, then compare bytes.
 output=qa/'secondary-roundtrip.hpp';compile_tracks(sorted(project.glob('*.sif')),output)
 runtime=Path(__file__).resolve().parents[2]/'src/generated/hero_secondary_data.hpp'
 arrays=lambda path:dict(re.findall(r'inline constexpr Key (\w+)Keys\[\] = \{(.*?)\};',path.read_text(),re.S))
 clips=lambda path:dict(re.findall(r'inline constexpr Clip (\w+) = (.*?);',path.read_text()))
 assert arrays(output)==arrays(runtime) and clips(output)==clips(runtime)
 native={}
 for mode in ['idle','walk','run','slash','heavy','bow','roll']:
  paths=sorted((qa/'frames').glob(mode+'-*.png'));assert len(paths)==60
  images=[Image.open(p).convert('RGBA') for p in paths]
  assert all(i.size==(400,420) and i.getchannel('A').getbbox() for i in images)
  assert len({hashlib.sha256(i.tobytes()).hexdigest() for i in images})>10
  native[mode]=images
 # Assemble real game captures, indexed with base walk speed / attack
 # durations. No repainting or invented interpolation of the character.
 movie=[]
 for f in range(120):
  t=f/60;canvas=Image.new('RGB',(792,300),(15,23,25));draw=ImageDraw.Draw(canvas)
  for column,(mode,label,phase) in enumerate([
    ('walk','CHOD',(t*(3.8*.38))%1),('run','BIEG',(t*(3.8*1.5*.31))%1),
    ('slash' if t%.8<.32 else 'idle','MIECZ',(t%.8)/.32 if t%.8<.32 else 0)]):
   im=native[mode][min(59,int(phase*60))].resize((264,277),Image.Resampling.LANCZOS)
   canvas.paste(im,(column*264,12),im);draw.text((column*264+100,282),label,fill=(224,192,128))
  movie.append(canvas)
 movie[0].save(qa/'hero-preview.gif',save_all=True,append_images=movie[1:],duration=[16,17,17]*40,loop=0,optimize=True)
 result={'status':'PASS','synfig_frames':count,'synfig_clips':records,'native_renderer_frames':420,'SIF_to_runtime_exact':True,'preview':'hero-preview.gif','audio':False}
 (qa/'asset-verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
 print(json.dumps(result,indent=2))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--project',type=Path,required=True);p.add_argument('--qa',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True)
 a=p.parse_args();verify(a.project,a.qa)

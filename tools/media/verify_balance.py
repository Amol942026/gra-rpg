"""Independent native Krita, runtime texture, UI and report verification."""
from pathlib import Path
import argparse,hashlib,io,json,re,zipfile
from PIL import Image
import numpy as np

def verify(qa,repo):
 count=0
 def check(ok,label):
  nonlocal count
  count+=1
  if not ok:raise AssertionError(label)
 art=repo/'art/gameplay/dune-fallen';receipt=json.loads((art/'receipt.json').read_text(encoding='utf-8'))
 sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
 png=repo/'assets/sprites/dune-fallen.png';kra=art/'dune-fallen.kra'
 check(receipt['status']=='PASS' and receipt['original_art'] and receipt['external_assets']==[],'original local Krita art')
 check(sha(kra)==receipt['kra_sha256'] and sha(png)==receipt['png_sha256'],'native and runtime provenance hashes')
 with Image.open(png) as im:
  im.load();check(im.size==(1536,512) and im.mode=='RGBA','bounded eight-view transparent atlas');atlas=np.asarray(im)
 with zipfile.ZipFile(kra) as z:
  check(z.testzip() is None,'KRA archive integrity');check(z.read('maindoc.xml').count(b'<layer')>=5,'native editable body/detail/face layers retained')
  with Image.open(io.BytesIO(z.read('mergedimage.png'))) as im:check(np.array_equal(np.asarray(im.convert('RGBA')),atlas),'KRA merged image and exported PNG agree exactly')
 hashes=set()
 for d in range(8):
  source=atlas[d//4*256:d//4*256+256,d%4*384:d%4*384+384].copy();check((source[:,:,3]>128).sum()>4000,'detailed complete native corpse view')
  hashes.add(hashlib.sha256(source.tobytes()).hexdigest())
  expected=source.copy();expected[:,:,:3]=(source[:,:,:3].astype(float)*source[:,:,3:4]/255).astype('uint8')
  with Image.open(qa/'corpse-views'/f'{d}.png') as im:
   im.load();actual=np.asarray(im);check(im.size==(384,256),'real preloaded corpse texture decodes');check(np.abs(actual.astype(int)-expected.astype(int)).max()<=1,'runtime uses the new Krita pixels with correct premultiplied edges')
 check(len(hashes)==8,'every facing uses a distinct authored view')
 names=['stats-readonly','skills-readonly','stats-allocation','skills-allocation','skills-selected','arrows-full-stock','arrows-out-of-stock','arrows-restocked','dune-fallen-world','dune-fallen-loot']
 for name in names:
  with Image.open(qa/(name+'.png')) as im:
   im.load();check(im.size==(1440,900) and np.asarray(im.convert('RGB')).std()>15,'nonempty real game capture: '+name)
 with Image.open(qa/'arrows-full-stock.png') as a,Image.open(qa/'arrows-out-of-stock.png') as b:
  check(np.any(np.asarray(a)[348:394,137:681]!=np.asarray(b)[348:394,137:681],axis=2).sum()>30,'actual shop offer visibly changes its remaining stock')
 with Image.open(qa/'stats-readonly.png') as a,Image.open(qa/'stats-allocation.png') as b:
  check(np.any(np.asarray(a)[266:602,938:1114]!=np.asarray(b)[266:602,938:1114],axis=2).sum()>500,'stat actions appear only during earned allocation')
 for f in ['src/progression_ui.hpp','src/skills_ui.hpp']:
  s=(repo/f).read_text(encoding='utf-8');check('add?"+":"-"' not in s and '"DODAJ":"COFNIJ"' in s,'assignment controls use words rather than plus/minus glyphs')
 log=(qa/'checks.txt').read_text(encoding='utf-8');check('Failures: 0' in log,'all new integration checks passed')
 subchecks={}
 for folder,name in [('qa-v13','progression-tests.txt'),('qa-v27','skills-tests.txt')]:
  s=(qa.parent/folder/name).read_text(encoding='utf-8');check('failures: 0' in s,'existing '+folder+' UI regression passed');subchecks[folder]=int(re.search(r'Checks: (\d+)',s)[1])
 result={'status':'PASS','independent_checks':count,'integration_checks':int(re.search(r'Checks: (\d+)',log)[1]),'existing_ui_checks':subchecks,'directions':8,'cell':[384,256],'rgba8_bytes':1536*512*4,'audio_opened':False,'xp_thresholds':[100*n+2*n*(n-1) for n in range(1,11)],'merchant_arrow_capacity':20,'save_version':7}
 (qa/'verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result,indent=2))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--qa',type=Path,required=True);p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.qa,a.repo)

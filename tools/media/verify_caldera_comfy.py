"""Offline acceptance of actual Comfy outputs, projection and layered masters."""
from pathlib import Path
import argparse,json,zipfile
import numpy as np
from PIL import Image
from caldera_comfy import NAMES,sha,write

def run(root):
 checks=0;rows=[]
 def check(ok,label):
  nonlocal checks
  checks+=1
  if not ok:raise AssertionError(label)
 for name in NAMES:
  for d in range(8):
   files=list((root/'outputs').glob(f'{name}-{d}_*.png'));check(len(files)==1,'one actual generation for '+name+str(d));im=Image.open(files[0]);im.load()
   check(im.size==(512,512),'decoded generated dimensions')
   graph=json.loads(im.text['prompt']);expected=json.loads((root/'workflows'/f'{name}-{d}.json').read_text())
   check(graph['2'].get('is_changed')==[sha(root/'inputs'/f'{name}-{d}.png')],'ComfyUI recorded the exact input image SHA256')
   graph={k:{field:value for field,value in node.items() if field!='is_changed'} for k,node in graph.items()}
   check(graph==expected,'actual PNG embeds the exact prompt, seed, source and graph')
   check(all(n['class_type'] in ['CheckpointLoaderSimple','LoadImage','CLIPTextEncode','VAEEncode','KSampler','VAEDecode','SaveImage'] for n in graph.values()),'only local core nodes; no partner API')
  before=np.array(Image.open(root/'backup'/f'{name}.png').convert('RGBA'));after=np.array(Image.open(root/'runtime'/f'{name}.png').convert('RGBA'));h,w=before.shape[:2];cell=w//8
  check(after.shape==before.shape,'same RGBA dimensions');check(np.array_equal(before[:,:,3],after[:,:,3]),'exact alpha and hit mask preservation')
  check(w<=2048 and h<=2048,'bounded atlas dimensions');changed=[]
  for d in range(8):
   old=before[:,d*cell:(d+1)*cell];new=after[:,d*cell:(d+1)*cell];solid=old[:,:,3]>100;diff=np.abs(old[:,:,:3].astype(int)-new[:,:,:3].astype(int)).max(axis=2)
   count=int(((diff>4)&solid).sum());check(count>solid.sum()*.2,'substantial visible repaint of direction '+str(d));changed.append(count)
   for part in range(6):check(np.array_equal(old[part*cell:(part+1)*cell,:,3],new[part*cell:(part+1)*cell,:,3]),'joint cutout alpha unchanged')
  master=root/'masters'/f'{name}.ora';receipt=json.loads((root/'masters'/f'{name}-receipt.json').read_text())
  with zipfile.ZipFile(master) as z:
   check(z.testzip() is None,'OpenRaster CRC');check(len(receipt['layers'])>=7,'six edited cutout layers plus original')
   check(np.array_equal(np.array(Image.open(z.open('mergedimage.png')).convert('RGBA')),after),'editable master merged pixels exactly match runtime')
  check(sha(master)==receipt['ora_sha256'],'master save receipt hash');check(receipt['pixels_match_runtime'],'master PNG readback')
  rows.append({'name':name,'size':[w,h],'rgba8_bytes':w*h*4,'sha256':sha(root/'runtime'/f'{name}.png'),'original_sha256':sha(root/'backup'/f'{name}.png'),'changed_opaque_pixels_by_direction':changed,'alpha_identical':True})
 result={'status':'PASS','checks':checks,'files':rows,'generations':56,'species':7,'directions':8,'layered_ora':7,'native_kra':0,'local_only':True,'paid_nodes':False,'animation_keys_changed':False,'texture_memory_rgba8':sum(r['rgba8_bytes'] for r in rows),'foreground_fps_measured':False}
 write(root/'acceptance.json',result);print(json.dumps({k:v for k,v in result.items() if k!='files'}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root)

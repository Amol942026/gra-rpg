"""Sequential local-only ComfyUI queue driver. No automatic mutation retries.

The native /prompt endpoint validates every submitted graph. Its prompt IDs,
history, original graphs and image SHA-256 are retained for reproducibility.
"""
from pathlib import Path
import argparse,json,time,urllib.request,shutil,uuid
from caldera_comfy import NAMES,write,sha

def request(path,payload=None):
 data=None if payload is None else json.dumps(payload).encode()
 req=urllib.request.Request('http://127.0.0.1:8188'+path,data=data,headers={'Content-Type':'application/json'})
 with urllib.request.urlopen(req,timeout=30) as response:return json.load(response)
def main(root,comfy):
 source=comfy/'output/caldera-comfy-20261104';client=str(uuid.uuid4());completed=0
 for name in NAMES:
  for d in range(8):
   files=list(source.glob(f'{name}-{d}_*.png'))
   if not files:
    graph=json.loads((root/'workflows'/f'{name}-{d}.json').read_text())
    assert all(n['class_type'] in ['CheckpointLoaderSimple','LoadImage','CLIPTextEncode','VAEEncode','KSampler','VAEDecode','SaveImage'] for n in graph.values())
    response=request('/prompt',{'prompt':graph,'client_id':client})
    write(root/'logs'/f'{name}-{d}-submission.json',response)
    if response.get('node_errors') or not response.get('prompt_id'):raise RuntimeError(response)
    pid=response['prompt_id'];deadline=time.monotonic()+420
    while True:
     history=request('/history/'+pid)
     if pid in history:
      result=history[pid];write(root/'logs'/f'{name}-{d}-history.json',result)
      if result['status']['status_str']!='success':raise RuntimeError(result['status'])
      break
     if time.monotonic()>deadline:raise TimeoutError('Inspect native queue before resuming: '+pid)
     time.sleep(2)
    files=list(source.glob(f'{name}-{d}_*.png'))
   assert len(files)==1,(name,d,files)
   dest=root/'outputs'/files[0].name;shutil.copy2(files[0],dest);completed+=1
   print(json.dumps({'completed':completed,'total':56,'name':name,'direction':d,'sha256':sha(dest)}),flush=True)
 write(root/'batch-complete.json',{'status':'PASS','count':completed,'server':'http://127.0.0.1:8188','local_only':True,'no_paid_nodes':True})
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--comfy',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();main(a.root,a.comfy)

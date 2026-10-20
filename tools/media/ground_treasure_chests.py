"""Correct the native Krita contact-shadow layers while retaining original art."""
from pathlib import Path
import argparse, hashlib, json, shutil, zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed

def main(a):
 root=Path(__file__).resolve().parents[2]
 source=Path(r'C:\AI2D\sprites\emberfall_treasure_chests_20261019_v1.kra')
 original=hashlib.sha256(source.read_bytes()).hexdigest()
 kra=source.with_name(a.name+'.kra');png=kra.with_suffix('.png')
 if kra.exists() or png.exists():raise FileExistsError('Use a fresh master name')
 start_krita_if_needed()
 with KritaMCP(a.log_directory) as c:
  opened=c.call('krita_document',action='open',path=str(source))
  doc=opened['affected_document_id']
  state=c.call('krita_state',document_id=doc)
  shadows=[n for n in state['layers'] if 'contact shadow' in n['name']]
  assert len(shadows)==3
  for node in shadows:
   layer=node['id'];frame=int(node['name'].split()[0]);offset=frame*256
   c.call('krita_edit',action='select_all',document_id=doc)
   c.call('krita_edit',action='clear',document_id=doc,layer_id=layer)
   c.call('krita_edit',action='deselect',document_id=doc)
   c.call('krita_draw',shape='polygon',document_id=doc,layer_id=layer,
          points=[{'x':x+offset,'y':y} for x,y in [(34,177),(117,133),(229,186),(141,234)]],
          foreground='#182b2d',fill_style='ForegroundColor',stroke_style='None',brush_size=1)
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
  c.call('krita_preview',document_id=doc,max_width=1024,max_height=384)
  c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 assert hashlib.sha256(source.read_bytes()).hexdigest()==original
 from PIL import Image
 with Image.open(png) as im:
  im.load();assert im.size==(768,256) and im.mode=='RGBA' and im.getchannel('A').getextrema()==(0,255)
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None and 'maindoc.xml' in z.namelist()
 runtime=root/'assets/sprites/treasure-chests.png';meta=runtime.with_suffix('.json')
 a.backup_directory.mkdir(parents=True,exist_ok=False)
 shutil.copy2(runtime,a.backup_directory/runtime.name);shutil.copy2(meta,a.backup_directory/meta.name)
 data=json.loads(meta.read_text(encoding='utf-8'))
 data.update(master=str(kra),source_png=str(png),sha256=hashlib.sha256(png.read_bytes()).hexdigest(),anchor=[128,183],ground_contact='Base-footprint center; contact shadow follows the bottom diamond')
 shutil.copy2(png,runtime);meta.write_text(json.dumps(data,indent=2),encoding='utf-8')
 print(json.dumps({'status':'PASS','master':str(kra),'original_master_unchanged':True,'anchor':[128,183]}))

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--name',required=True);p.add_argument('--log-directory',type=Path,required=True);p.add_argument('--backup-directory',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);main(p.parse_args())

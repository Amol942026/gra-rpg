"""Original layered Krita projectile for the Stone Guardian. No external art."""
from pathlib import Path
import argparse,hashlib,json,zipfile
from krita_mcp_client import KritaMCP,start_krita_if_needed

def author(root):
 root.mkdir(parents=True,exist_ok=False);start_krita_if_needed()
 with KritaMCP(root/'logs') as c:
  doc=c.call('krita_document',action='create',width=192,height=192,name='Emberfall / Stone Guardian / uprooted boulder')['affected_document_id']
  state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
  lid=state['active_layer_id'];c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=lid);c.call('krita_edit',action='deselect',document_id=doc)
  layers=[]
  def layer(name):
   nonlocal lid
   c.call('krita_layers',action='create',document_id=doc,name=name);lid=c.call('krita_state',document_id=doc)['active_layer_id'];layers.append({'name':name,'uuid':lid})
  def poly(color,pts):c.call('krita_draw',shape='polygon',document_id=doc,layer_id=lid,foreground=color,points=[{'x':x,'y':y} for x,y in pts],fill_style='ForegroundColor',stroke_style='None')
  def path(color,pts,width):c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,foreground=color,points=[{'x':x,'y':y} for x,y in pts],fill_style='None',stroke_style='ForegroundColor',brush_size=width)
  # Author on a 96-unit design grid into a native 192 px layered master.
  native_poly,native_path=poly,path
  def poly(color,pts):native_poly(color,[(x*2,y*2) for x,y in pts])
  def path(color,pts,width):native_path(color,[(x*2,y*2) for x,y in pts],width*2)
  layer('01 / Uprooted granite silhouette')
  poly('#354b44',[(14,49),(20,29),(36,17),(59,14),(77,28),(84,47),(77,72),(55,82),(31,78),(17,65)])
  layer('02 / Upper left light and weathered planes')
  poly('#7b8c79',[(14,49),(20,29),(36,17),(59,14),(62,38),(43,53),(24,58)])
  poly('#9fa98d',[(20,29),(36,17),(59,14),(49,28),(32,40)])
  poly('#5b7568',[(62,38),(59,14),(77,28),(84,47),(68,61),(43,53)])
  poly('#4b655b',[(43,53),(68,61),(77,72),(55,82),(31,78),(17,65),(24,58)])
  poly('#718676',[(24,58),(43,53),(49,66),(31,74),(19,64)])
  layer('03 / Cracks, moss and mineral flecks')
  path('#344c45',[(48,26),(44,39),(51,46),(45,57),(49,67)],2)
  path('#3b5249',[(49,67),(61,68),(67,76)],1.4)
  path('#bbc0a0',[(24,32),(35,22),(49,19)],1.7)
  for x,y in [(30,56),(27,62),(35,66),(67,36),(71,40),(58,74)]:
   poly('#6d8354',[(x,y),(x+4,y-2),(x+7,y+1),(x+4,y+4),(x,y+3)])
  for x,y in [(32,30),(37,27),(58,32),(66,47),(26,48),(57,62)]:poly('#b1b69a',[(x,y),(x+2,y-1),(x+3,y+1),(x+1,y+2)])
  layer('04 / Soil clods and torn roots')
  poly('#544c37',[(18,63),(30,68),(37,72),(48,73),(61,70),(76,64),(77,72),(65,79),(55,83),(41,83),(30,79),(24,73)])
  poly('#79664a',[(22,66),(31,69),(36,74),(47,77),(58,75),(62,79),(51,82),(39,79),(29,76)])
  for x,y in [(27,72),(40,78),(56,77),(69,72)]:
   path('#9d8b5f',[(x-3,y-4),(x,y),(x+2,y+5),(x+5,y+6)],.7)
  for x,y in [(21,66),(33,74),(46,79),(62,74),(71,69)]:poly('#3d4030',[(x,y),(x+3,y-1),(x+5,y+2),(x+2,y+3)])
  layer('05 / Fine granite fractures and lichen')
  for pts in [[(19,45),(29,43),(33,48)],[(62,26),(66,34),(74,37)],[(23,58),(30,60),(34,66)],[(52,53),(59,55),(62,64)]]:path('#475b50',pts,.65)
  for x,y in [(22,39),(34,24),(42,29),(55,21),(71,45),(31,52),(52,64),(63,53),(38,62),(59,36)]:
   poly('#8e9b77',[(x,y),(x+2,y-1),(x+4,y+1),(x+3,y+2),(x,y+2)])
   path('#bac2a3',[(x,y),(x+1,y-.5)],.6)
  layer('06 / Faint guardian rune')
  path('#bfad6a',[(54,39),(61,46),(58,53),(51,47),(54,39)],1.8)
  kra=root/'guardian-stone.kra';png=root/'guardian-stone.png'
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False);c.call('krita_preview',document_id=doc,max_width=384,max_height=384);c.call('krita_state',document_id=doc);c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 from PIL import Image
 with Image.open(png) as im:im.load();assert im.size==(192,192) and im.mode=='RGBA' and im.getchannel('A').getextrema()==(0,255)
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None
 receipt={'status':'PASS','authoring':'Krita native MCP draw','original_art':True,'external_assets':[],'layers':layers,'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest()}
 (root/'provenance.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')
 print('PASS: original layered Krita stone',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root)

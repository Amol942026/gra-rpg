"""Original fractured granite, painted as six matching cutouts in native Krita."""
from pathlib import Path
import argparse,hashlib,json,zipfile
from krita_mcp_client import KritaMCP,start_krita_if_needed

POLYGONS=[[(14,49),(20,29),(36,17),(43,34),(40,42),(47,47),(31,50)],
 [(36,17),(59,14),(66,27),(61,37),(53,39),(47,47),(40,42),(43,34)],
 [(66,27),(59,14),(77,28),(84,47),(68,52),(55,47),(47,47),(53,39),(61,37)],
 [(14,49),(31,50),(47,47),(43,59),(38,64),(43,73),(31,78),(17,65)],
 [(47,47),(55,47),(59,61),(57,70),(55,82),(31,78),(43,73),(38,64),(43,59)],
 [(55,47),(68,52),(84,47),(77,72),(55,82),(57,70),(59,61)]]
PIVOTS=[(31,36),(49,30),(69,39),(29,62),(48,67),(68,66)]

def author(root):
 root.mkdir(parents=True,exist_ok=False);start_krita_if_needed()
 with KritaMCP(root/'logs') as c:
  doc=c.call('krita_document',action='create',width=288,height=192,name='Emberfall / Guardian / six fractured boulder pieces')['affected_document_id']
  state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
  c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id']);c.call('krita_edit',action='deselect',document_id=doc)
  layers=[]
  for i,(points,pivot) in enumerate(zip(POLYGONS,PIVOTS)):
   c.call('krita_layers',action='create',document_id=doc,name=f'{i+1:02} / granite fracture / cutout')
   lid=c.call('krita_state',document_id=doc)['active_layer_id'];layers.append({'name':f'fragment-{i}','uuid':lid})
   ox,oy=i%3*96,i//3*96
   def poly(color,pts):
    c.call('krita_draw',shape='polygon',document_id=doc,layer_id=lid,foreground=color,points=[{'x':x+ox,'y':y+oy} for x,y in pts],fill_style='ForegroundColor',stroke_style='None')
   def line(color,pts,width):
    c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,foreground=color,points=[{'x':x+ox,'y':y+oy} for x,y in pts],fill_style='None',stroke_style='ForegroundColor',brush_size=width)
   poly(['#7b8c79','#87947c','#5d7769','#5d7565','#566d5d','#486358'][i],points)
   # All triangles stay within the jagged silhouette; bright edges reveal fresh
   # fracture planes instead of drawing six small copies of the whole boulder.
   for j in range(len(points)):
    a,b=points[j],points[(j+1)%len(points)]
    if j%3==0:poly('#93a089' if i<3 else '#7f8e78',[pivot,a,b])
    elif j%3==2:poly('#52695d',[pivot,a,b])
   for j in (2,4):
    a,b=points[j%len(points)],points[(j+1)%len(points)]
    line('#b9bda3',[a,b],1)
   px,py=pivot;poly('#a2aa8c',[(px-3,py-1),(px,py-3),(px+2,py),(px-1,py+2)])
   if i<3:poly('#6d8354',[(px-5,py+4),(px-1,py+2),(px+2,py+4),(px,py+7),(px-4,py+7)])
   else:line('#79664a',[(px-4,py+8),(px,py+9),(px+3,py+7)],2)
  kra=root/'guardian-fragments.kra';png=root/'guardian-fragments.png'
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False);c.call('krita_preview',document_id=doc,max_width=576,max_height=384);c.call('krita_state',document_id=doc);c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 from PIL import Image
 with Image.open(png) as im:
  im.load();assert im.size==(288,192) and im.mode=='RGBA' and im.getchannel('A').getextrema()==(0,255)
  for i in range(6):assert im.crop((i%3*96,i//3*96,i%3*96+96,i//3*96+96)).getbbox()
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None
 receipt={'status':'PASS','authoring':'Krita native MCP draw','original_art':True,'external_assets':[],'layers':layers,'polygons':POLYGONS,'pivots':PIVOTS,'cell':96,'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest()}
 (root/'provenance.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8');print('PASS: six original layered Krita fracture cutouts',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root)

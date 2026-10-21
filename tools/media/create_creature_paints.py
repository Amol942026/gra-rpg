"""Original transparent creature material details, drawn natively in Krita MCP.

Pillow only validates/exports saved cells. No external art or image generation.
"""
from pathlib import Path
import argparse, hashlib, json, zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed

NAMES=['slime','wolf','bone','stone','boar','spider','cloth','bark',
       'scorpion','scarab','scales','sand','leaves','wolf_face','boar_face','hood']

def create(root):
 from PIL import Image
 root.mkdir(parents=True,exist_ok=False);start_krita_if_needed()
 with KritaMCP(root/'logs') as c:
  doc=c.call('krita_document',action='create',width=512,height=512,name='Emberfall / bestiariusz / materiały 2D')['affected_document_id']
  state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
  c.call('krita_edit',action='select_all',document_id=doc)
  c.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id'])
  c.call('krita_edit',action='deselect',document_id=doc)
  layers=[]
  for cell,name in enumerate(NAMES):
   c.call('krita_layers',action='create',document_id=doc,name=f'{cell:02} / {name} / malowane detale')
   lid=c.call('krita_state',document_id=doc)['active_layer_id'];layers.append(lid)
   ox,oy=cell%4*128,cell//4*128
   def path(col,pts,size=2):
    c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,points=[{'x':ox+x,'y':oy+y} for x,y in pts],foreground=col,fill_style='None',stroke_style='ForegroundColor',brush_size=size)
   def oval(col,x,y,w,h):
    c.call('krita_draw',shape='ellipse',document_id=doc,layer_id=lid,x=ox+x,y=oy+y,width=w,height=h,foreground=col,fill_style='ForegroundColor',stroke_style='None')
   if name in ['wolf','boar']:
    colors=['#667970','#c2c7a7'] if name=='wolf' else ['#4c382d','#b08a5c']
    for row in range(5):
     for col in range(5):
      x=17+col*21+(row%2)*4;y=14+row*21
      path(colors[(row+col)%2],[(x-4,y),(x+1,y+8),(x-2,y+18)],1.6)
    path(colors[1],[(59,12),(55,37),(64,58),(60,85),(67,115)],3)
   elif name in ['scorpion','scarab','scales']:
    dark,light={'scorpion':('#805329','#dcb779'),'scarab':('#294e4a','#d3ad69'),'scales':('#886a37','#edcb81')}[name]
    for row in range(5):
     for col in range(4):
      x=15+col*26+row%2*6;y=15+row*21
      path(dark,[(x-7,y),(x,y+8),(x+8,y)],1.7)
      path(light,[(x-5,y+2),(x,y+5),(x+5,y+2)],1)
    if name=='scarab':
     path('#e0c388',[(64,8),(64,120)],3)
     for y in [32,58,84]:path('#e0c388',[(26,y),(47,y+8),(64,y+20),(81,y+8),(102,y)],2)
   elif name in ['stone','bone','sand','bark']:
    dark,light={'stone':('#405c55','#b0ba8c'),'bone':('#99967a','#e5d9b1'),'sand':('#ab7f46','#f0d495'),'bark':('#51432e','#b3965f')}[name]
    for n in range(7):
     x=14+n*16;y=17+n%3*19
     path(dark,[(x,y),(x-3,y+19),(x+3,y+32),(x,y+65)],1.4 if name=='bone' else 2.3)
     path(light,[(x+3,y+2),(x+1,y+19),(x+5,y+29)],1.2)
    if name=='stone':
     path('#8ec9ac',[(64,27),(47,47),(64,66),(80,47),(64,27),(64,98)],4)
     path('#dfd296',[(64,32),(52,47),(64,59),(75,47)],1.5)
    if name=='bark':
     oval('#51432e',43,44,34,43);oval('#a68b59',48,49,24,33);oval('#59482f',55,55,11,22)
   elif name=='slime':
    for n in range(13):
     x=17+(n*29)%89;y=17+(n*47)%89;r=3+n%4
     oval('#91ba79',x,y,r+3,r+5);oval('#d5e8a6',x+1,y+1,2,3)
    path('#c9e4a0',[(30,32),(24,43),(24,58)],3)
   elif name=='spider':
    for side in [-1,1]:
     path('#9eaf61',[(64+side*6,15),(64+side*20,39),(64+side*10,55),(64+side*19,79),(64+side*5,108)],4)
     for y in [34,60,85]:oval('#c3c17a',64+side*27-3,y,6,5)
   elif name in ['cloth','hood']:
    for x in [24,101]:
     path('#c8a66b',[(x,12),(x-3,55),(x+2,115)],2)
     for y in range(18,110,12):path('#e0c490',[(x+4,y),(x+7,y+3)],1)
    for x in [43,70,87]:path('#563b32',[(x,20),(x-5,58),(x+2,111)],2)
   elif name=='leaves':
    for row in range(4):
     for col in range(4):
      x=21+col*25;y=15+row*26
      oval('#73935c',x-7,y,16,21);path('#bdd08b',[(x,y+3),(x,y+17)],1.3)
   else:
    for x in [31,80]:
     path('#51422f',[(x-4,45),(x+3,42),(x+12,46)],2)
     oval('#e3c77e',x,47,10,6);oval('#263534',x+4,47,3,6);oval('#f7e9c0',x+2,48,2,2)
    if name=='wolf_face':
     path('#d5d5b8',[(38,69),(49,78),(64,83),(79,78),(90,69)],3)
    else:
     for x in [50,72]:oval('#5c3c2e',x-3,83,6,9)
  kra=root/'creature-paints.kra';png=root/'creature-paints.png'
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
  c.call('krita_preview',document_id=doc,max_width=1024,max_height=1024)
  c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None and 'maindoc.xml' in z.namelist()
 im=Image.open(png).convert('RGBA');assert im.size==(512,512) and im.getextrema()[3]==(0,255)
 (root/'parts').mkdir()
 for cell,name in enumerate(NAMES):
  part=im.crop((cell%4*128,cell//4*128,cell%4*128+128,cell//4*128+128));assert part.getbbox();part.save(root/'parts'/f'{name}.png')
 report={'status':'PASS','authoring':'Krita 5.3.4.1 native MCP draw','layers':len(layers),'cells':NAMES,'external_assets':[],'original_art':True,'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'rgba8_bytes':512*512*4}
 (root/'provenance.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report))

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();create(a.root)

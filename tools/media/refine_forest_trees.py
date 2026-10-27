"""Native Krita finishing pass: fine foliage, living spruce tip and bark.

Opens the owned first-pass KRA masters, writes fresh KRA/PNG, leaves originals.
"""
import argparse, hashlib, json, math, random, zipfile
from pathlib import Path
from krita_mcp_client import KritaMCP, start_krita_if_needed
from create_menu_forest import smooth

CLUSTERS={
'birch':[(114,106,41,42),(157,76,38,42),(204,66,37,36),(248,89,38,36),
 (276,124,37,37),(292,164,25,34),(253,184,39,35),(211,151,45,40),
 (168,128,44,42),(126,161,37,35),(88,149,32,35),(78,191,26,29),
 (129,205,38,32),(178,190,34,34),(223,214,33,31),(264,235,25,27)],
'oak':[(66,171,33,34),(87,128,43,35),(124,94,43,36),(171,77,41,37),
 (216,78,39,38),(256,108,46,36),(285,142,34,39),(303,177,28,29),
 (269,200,42,32),(226,194,47,39),(181,162,46,44),(137,156,43,45),
 (99,206,37,28),(145,217,42,34),(194,216,42,30)]}


def refine(source,root):
 root.mkdir(parents=True,exist_ok=False);start_krita_if_needed();results=[]
 prior=json.loads((source/'provenance.json').read_text(encoding='utf-8'))
 for index,species in enumerate(('spruce','birch','oak')):
  rng=random.Random(20261023+index);directory=root/species;directory.mkdir()
  with KritaMCP(directory/'logs') as c:
   doc=c.call('krita_document',action='open',path=str(source/species/('forest-'+species+'.kra')))['affected_document_id']
   state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
   c.call('krita_layers',action='create',document_id=doc,name='08 / Fine foliage, needles and organic edge detail')
   lid=c.call('krita_state',document_id=doc)['active_layer_id']
   def draw(**kw):
    try:c.call('krita_draw',document_id=doc,layer_id=lid,**kw)
    except RuntimeError as error:
     if 'DRAW_NO_CHANGE' not in str(error):raise
   def poly(col,pts):draw(shape='polygon',foreground=col,points=[{'x':round(x*1.5,3),'y':round(y*1.5,3)} for x,y in pts],fill_style='ForegroundColor',stroke_style='None')
   def path(col,pts,w=1):draw(shape='path',foreground=col,points=[{'x':round(x*1.5,3),'y':round(y*1.5,3)} for x,y in smooth(pts,False,4)],fill_style='None',stroke_style='ForegroundColor',brush_size=w*1.5)
   def leaf(x,y,size,angle,col,shade=False):
    dx,dy=math.cos(angle),math.sin(angle)
    def p(a,b):return(x+dx*a-dy*b,y+dy*a+dx*b)
    poly(col,[p(0,0),p(size*.17,-size*.28),p(size*.36,-size*.45),p(size*.68,-size*.32),p(size,0),p(size*.69,size*.23),p(size*.4,size*.35),p(size*.15,size*.21)])
    if shade:poly('#8f9d61' if species=='birch' else '#829559',[p(size*.1,0),p(size*.39,-size*.3),p(size*.75,-size*.2),p(size*.91,0)])
   if species=='spruce':
    # A living tapered leader covers the exposed wooden stub of the block-in.
    poly('#294d3c',[(173,15),(177,28),(181,33),(177,32),(183,42),(190,46),(185,50),(197,65),(187,61),(204,83),(178,75),(156,83),(171,59),(163,64),(171,40),(167,41),(171,30),(168,32)])
    path('#658357',[(173,21),(176,42),(177,50),(182,66)],1.2)
    for n in range(24):
     y=33+n*1.9;side=(-1 if n%2 else 1);w=3+(y-33)*.27
     path('#78915f' if n%3 else '#406846',[(179,y),(179+side*w,y+3),(179+side*(w+2),y+6)],.7)
    for row in range(9):
     y=329-row*32;spread=129-row*13
     for side in (-1,1):
      for n in range(7):
       xx=179+side*(8+spread*n/7)+rng.uniform(-3,3);yy=y+rng.uniform(-12,7)
       length=rng.uniform(8,15)
       path('#274e3a',[(xx-side*3,yy+2),(xx+side*length*.65,yy+6),(xx+side*length,yy+12)],1.8)
       for k in range(3):
        x=xx+side*(k*length*.29);v=yy+k*2.5
        path(rng.choice(['#597f52','#759163','#416e49']),[(x-side*2,v-2),(x+side*4,v+2),(x+side*3,v+6)],.8)
   else:
    for cx,cy,rx,ry in CLUSTERS[species]:
     # Offset a dense field of small leaf planes from the broad lighting mass.
     # Smaller, darker leaves below; highlights broken up along the upper left.
     for n in range(33):
      angle=rng.uniform(0,math.tau);r=math.sqrt(rng.random())
      x=cx+math.cos(angle)*rx*r;y=cy+math.sin(angle)*ry*r
      upper=(x-cx)/rx+(y-cy)/ry<-.25
      palette=(['#719350','#87a05c','#65884a','#9ca968'] if upper else ['#456a40','#5a7d46','#6c8b4d','#537441']) if species=='birch' else (
       ['#64864d','#759352','#5a7d46','#8c9d5b'] if upper else ['#345a39','#486e40','#527945','#3c633d'])
      leaf(x,y,rng.uniform(4,8),rng.uniform(-3,-.2),rng.choice(palette),n%11==0)
     for n in range(6):
      angle=rng.uniform(0,math.tau)
      x=cx+math.cos(angle)*rx*.99;y=cy+math.sin(angle)*ry*.99
      leaf(x,y,rng.uniform(4,8),angle,'#63854a' if species=='birch' else '#4d7443')
    # Narrow bark facets follow the curved trunk, rather than drawing a border
    # around it; small lichen spots retain the pale identity of the birch.
    c.call('krita_layers',action='create',document_id=doc,name='09 / Bark furrows and scattered lichen')
    lid=c.call('krita_state',document_id=doc)['active_layer_id']
    if species=='oak':
     for n in range(15):
      y=402-n*6;x=173+math.sin(n*.7)*5+n%3*4
      path('#a58b59',[(x,y),(x-1,y-5),(x+2,y-11)],.6)
      path('#514831',[(x+2,y-1),(x+3,y-6)],.7)
    else:
     for n in range(9):
      y=305+n*11;x=168+(y-300)*.075
      poly('#d8d7b1',[(x,y),(x+2,y-5),(x+4,y-2),(x+3,y+2)])
      if n%2:leaf(x+4,y-2,3,-1.2,'#889274')
   kra=directory/('forest-'+species+'.kra');png=directory/('forest-'+species+'.png')
   c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
   c.call('krita_preview',document_id=doc,max_width=540,max_height=690)
   state=c.call('krita_state',document_id=doc)
   c.call('krita_document',action='close',document_id=doc,discard_changes=False)
  with zipfile.ZipFile(kra) as z:assert z.testzip() is None
  from PIL import Image
  with Image.open(png) as im:
   im.load();assert im.size==(540,690) and im.mode=='RGBA' and im.getchannel('A').getextrema()==(0,255)
   assert im.getpixel((270,624))[3]>160;bounds=im.getbbox()
   assert bounds[0]>2 and bounds[1]>2 and bounds[2]<538 and bounds[3]<688
  results.append({'species':species,'kra':str(kra),'png':str(png),'layers':[{'name':l['name'],'uuid':l['id']} for l in state['layers']],
   'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'alpha_bounds':bounds})
  print('Finished Krita detail pass: '+species,flush=True)
 report={**prior,'trees':results,'block_in_provenance':str(source/'provenance.json'),'block_in_sha256':hashlib.sha256((source/'provenance.json').read_bytes()).hexdigest()}
 (root/'provenance.json').write_text(json.dumps(report,indent=2),encoding='utf-8')


if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True)
 a=p.parse_args();refine(a.source,a.root)

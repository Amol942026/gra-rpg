"""Paint the fallen Living Dune in native Krita, eight 2D facing views."""
from pathlib import Path
import argparse,hashlib,json,math,shutil,zipfile
from krita_mcp_client import KritaMCP,start_krita_if_needed

def smooth(points):
 out=[]
 for i,b in enumerate(points):
  a,c,d=points[i-1],points[(i+1)%len(points)],points[(i+2)%len(points)]
  for n in range(6):
   t=n/6
   out.append(tuple(.5*(2*b[k]+(-a[k]+c[k])*t+(2*a[k]-5*b[k]+4*c[k]-d[k])*t*t+(-a[k]+3*b[k]-3*c[k]+d[k])*t*t*t) for k in (0,1)))
 return out

def author(root,repo):
 root.mkdir(parents=True,exist_ok=False);start_krita_if_needed()
 with KritaMCP(root/'logs') as c:
  doc=c.call('krita_document',action='create',width=1536,height=512,name='Emberfall / Living Dune / collapsed remains / eight views')['affected_document_id']
  s=c.call('krita_state',document_id=doc);assert s['runtime_compatibility']['status']=='compatible'
  c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=s['active_layer_id']);c.call('krita_edit',action='deselect',document_id=doc)
  layers={}
  for name in ['01 / settled sand and torn silhouette','02 / collapsed body and sediment planes','03 / limp tendrils and broken fingers','04 / fallen face, closed eyes and fractured crest','05 / grains, cracks and spent amber']:
   c.call('krita_layers',action='create',document_id=doc,name=name);layers[name[:2]]=c.call('krita_state',document_id=doc)['active_layer_id']
  lid=layers['01'];ox=oy=0
  def poly(col,pts):c.call('krita_draw',shape='polygon',document_id=doc,layer_id=lid,foreground=col,points=[{'x':ox+x*2,'y':oy+(hy+(y-hy)*.68 if lid==layers['04'] else y)*2} for x,y in pts],fill_style='ForegroundColor',stroke_style='None')
  def line(col,pts,w=1):c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,foreground=col,points=[{'x':ox+x*2,'y':oy+(hy+(y-hy)*.68 if lid==layers['04'] else y)*2} for x,y in pts],fill_style='None',stroke_style='ForegroundColor',brush_size=w*2)
  def organic(col,pts):poly(col,smooth(pts))
  for direction in range(8):
   ox,oy=direction%4*384,direction//4*256;a=direction*math.tau/8
   # A deflated asymmetric silhouette with a torn peak, settled into the ground.
   lid=layers['01']
   organic('#98713f',[(20,83),(25,72),(40,66),(46,58),(70,53),(94,48),(116,52),(132,63),(156,72),(169,88),(154,99),(128,105),(100,101),(74,108),(45,101),(24,96)])
   organic('#bc9257',[(23,81),(40,72),(51,62),(73,58),(95,52),(117,57),(134,69),(157,77),(165,87),(147,97),(122,100),(97,96),(72,103),(48,96),(28,94)])
   lid=layers['02']
   organic('#cba468',[(31,81),(48,68),(66,58),(84,58),(103,54),(120,64),(129,75),(148,84),(126,93),(103,89),(79,95),(56,89)])
   organic('#d8b677',[(40,77),(62,62),(82,62),(100,57),(112,65),(102,74),(80,76),(68,84),(53,82)])
   organic('#e1c58b',[(45,75),(63,64),(78,65),(91,62),(85,67),(68,69),(57,78)])
   organic('#ab824b',[(73,93),(93,83),(111,81),(126,77),(149,85),(129,94),(105,90),(87,98)])
   # Broken sediment bands terminate at sand slumps, not a uniform oval stack.
   for pts in [[(35,84),(54,77),(65,79)],[(61,87),(82,80),(100,82)],[(91,65),(103,65),(112,70)],[(115,89),(129,86),(143,88)],[(48,94),(62,96),(73,93)]]:
    line('#927044',pts,.8);line('#e7c992',[(x,y-1) for x,y in pts],.65)
   lid=layers['03']
   for sign in (-1,1):
    x=96+sign*(45+3*math.sin(a));y=78+sign*6*math.cos(a)
    organic('#ad824b',[(x-sign*13,y-3),(x+sign*4,y),(x+sign*16,y+8),(x+sign*12,y+13),(x+sign*4,y+10),(x-sign*10,y+4)])
    line('#dabc80',[(x-sign*9,y-2),(x+sign*3,y+2),(x+sign*12,y+9)],1.4)
    for f in range(3):line('#85663d',[(x+sign*(5+f*3),y+8),(x+sign*(9+f*3),y+11)],.7)
   lid=layers['04']
   hx,hy=96+math.cos(a)*22,69+math.sin(a)*11
   # The broken crest lies sideways, and the face loses its living yellow glow.
   organic('#9f7744',[(hx-22,hy-4),(hx-19,hy-16),(hx-7,hy-23),(hx+5,hy-20),(hx+15,hy-11),(hx+25,hy-3),(hx+21,hy+12),(hx+6,hy+18),(hx-12,hy+13)])
   organic('#c7a063',[(hx-22,hy-5),(hx-17,hy-15),(hx-6,hy-21),(hx+4,hy-18),(hx+12,hy-9),(hx+21,hy-3),(hx+13,hy+9),(hx-7,hy+10)])
   poly('#dfbd7f',[(hx-18,hy-13),(hx-6,hy-20),(hx-2,hy-15),(hx-7,hy-10),(hx-14,hy-8)])
   line('#80613d',[(hx+1,hy-18),(hx-3,hy-10),(hx+2,hy-4),(hx-1,hy+4)],1.4)
   line('#e7c68c',[(hx+3,hy-16),(hx,hy-10),(hx+4,hy-5)],.8)
   facing=math.sin(a)
   if facing>-.5:
    for side in (-1,1):
     x=hx+side*9;line('#594c36',[(x-4,hy),(x,hy+1),(x+4,hy-1)],1.6)
     line('#d3b37a',[(x-5,hy-3),(x,hy-4),(x+4,hy-3)],1.1)
    line('#695236',[(hx-11,hy+9),(hx-5,hy+8),(hx-1,hy+10),(hx+3,hy+8),(hx+11,hy+8)],1.4)
    for n in (1,4):
     x=hx-9+n*3;poly('#b99a62',[(x,hy+8),(x+2,hy+8),(x+1,hy+11)])
   lid=layers['05']
   for n in range(13):
    t=n*2.39996+a*.3;x=96+math.cos(t)*(54+n%4*5);y=84+math.sin(t)*(17+n%3*4)
    poly('#a27c46',[(x-2,y),(x-1,y-3),(x+3,y-2),(x+4,y+1),(x,y+2)])
    line('#d9ba7d',[(x-1,y-2),(x+2,y-2)],.6)
   for n in range(20):
    x=33+(n*23+direction*7)%125;y=63+(n*13)%38
    line('#e5c88f' if n%3 else '#927044',[(x,y),(x+1.2,y-.25)],.55)
   # Small spent amber shards tie the remains to the original sand spirit.
   for x,y in [(hx+20,hy+20),(hx-15,hy+18)]:
    poly('#7c6846',[(x-2,y),(x,y-3),(x+3,y),(x+1,y+3)]);line('#c3a16c',[(x,y-2),(x+1,y)],.7)
  kra=root/'dune-fallen.kra';png=root/'dune-fallen.png'
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False);c.call('krita_preview',document_id=doc,max_width=1536,max_height=512);c.call('krita_state',document_id=doc);c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 from PIL import Image
 with Image.open(png) as im:
  im.load();assert im.size==(1536,512) and im.mode=='RGBA'
  for d in range(8):
   cell=im.crop((d%4*384,d//4*256,d%4*384+384,d//4*256+256));box=cell.getbbox();assert box and min(box[:2])>=4 and box[2]<380 and box[3]<252
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None
 sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
 receipt={'status':'PASS','authoring':'Krita native MCP polygons and brush paths','original_art':True,'external_assets':[],'master':str(root),'layers':layers,'cell':[384,256],'directions':8,'foot_anchor':[192,160],'kra_sha256':sha(kra),'png_sha256':sha(png),'rgba_bytes':1536*512*4}
 (root/'receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')
 target=repo/'assets/sprites/dune-fallen.png';shutil.copy2(png,target);art=repo/'art/gameplay/dune-fallen';art.mkdir(parents=True,exist_ok=True)
 for name in ['dune-fallen.kra','receipt.json']:shutil.copy2(root/name,art/name)
 print('PASS: original layered Krita fallen dune, 8 views',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root,a.repo)

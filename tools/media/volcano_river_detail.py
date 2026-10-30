"""Native Krita detail pass and additional Caldera scenery. Local, original 2D."""
from pathlib import Path
import argparse,math,random,shutil,json,hashlib,zipfile,time
from PIL import Image,ImageFilter,ImageChops
from polish_volcano_art import Paint,PARTS,NAMES,ANGLES
from krita_mcp_client import KritaMCP,start_krita_if_needed

def details(p,kind,part,direction):
 rng=random.Random(71531+kind*773+part*97+direction*23);a=ANGLES[direction];fx,fy=math.cos(a),math.sin(a);front=fy>=-.1;quad=kind in (0,1,5);boss=kind==6
 # Native airbrush modelling is clipped to the actual existing part silhouette.
 p.soft('#e4c89b',51,70,62,.36);p.soft('#111d2a',83,95,53,.6)
 if part==3:
  if quad:
   w=(48 if kind==0 else 40)+abs(fx)*(24 if kind==0 else 27);cy=84 if kind!=0 else 85
   for n in range(44):
    x=64+rng.uniform(-.44,.44)*w;y=cy+rng.uniform(-15,13)
    if kind==0:
     p.line('#303b43',[(x,y),(x+1.2,y-2),(x+3,y-1)],.45)
     p.oval('#879087',x-.5,y-.7,.8,.65)
    else:
     p.line('#a1a58b',[(x-1.3,y),(x,y+1.4),(x+1.8,y+.1)],.55)
     p.line('#283a3d',[(x,y+2),(x+1.8,y+.8)],.55)
   if kind==0:
    for plate in range(5):
     x=64+(plate-2)*(8+abs(fx)*3);y=78+abs(plate-2)*2
     p.line('#27353e',[(x-3,y-8),(x+1,y-4),(x-2,y+1),(x+2,y+7)],.8)
     p.line('#bd8155',[(x+1,y-4),(x+3,y-1)],.5)
     p.line('#a7aca0',[(x-4,y-10),(x-2,y-15)],.7)
   hx=64+fx*(24 if kind==0 else 30);hy=86+fy*10
   p.soft('#aeb195',hx-4,hy-4,18,.45)
   for n in range(7):
    x=hx-9+n*2.5;p.line('#303e40',[(x,hy-6),(x+1,hy-3)],.5)
   if front or abs(fx)>.8:
    ex=hx+fx*5;p.oval('#15252d',ex-5.8,hy-5.1,3.8,2.4);p.line('#f6bd68',[(ex-5,hy-4.1),(ex-3,hy-4.3)],1)
    p.oval('#fff1c4',ex-4.3,hy-4.5,.65,.65);p.oval('#172a30',hx+fx*10-1,hy-1.6,2.6,1.3)
    p.line('#97a187',[(hx-4,hy+6),(hx+5,hy+5)],.5)
  elif kind==2:
   p.soft('#d99a68',57,72,20,.6);p.soft('#402d31',71,80,20,.6)
   for y in (75,80,85):p.line('#734536',[(59,y),(62,y+1),(66,y-.3)],.6)
   hx=64+fx*3;hy=51
   if front:
    p.poly('#ad7654',[(hx-8,hy-1),(hx,hy),(hx+8,hy-2),(hx+6,hy+5),(hx,hy+6),(hx-6,hy+4)],True)
    p.soft('#efd0a0',hx-3,hy+1,10,.5)
    for side in (-1,1):
     x=hx+side*4;p.line('#442f2c',[(x-1.6,hy-.6),(x+1.3,hy-1.2)],.8);p.oval('#202a2c',x-1,hy+.6,2.4,1.8);p.oval('#f6cd71',x-.2,hy+1.1,.8,.8)
     p.line('#695044',[(hx+side*9,hy-6),(hx+side*11,hy-10)],.55)
    p.line('#dca776',[(hx,hy+2),(hx+1,hy+5)],.8);p.line('#473731',[(hx-4,hy+8),(hx,hy+9),(hx+4,hy+7)],.6)
    p.line('#dcc39a',[(hx-2,hy+9),(hx+2,hy+8)],.7)
  elif kind==3:
   for n in range(12):
    x=47+n*3;y=68+(n%3)*8;p.line('#8d969e',[(x,y),(x-2,88),(x+2,103+(n%2)*5)],.45)
    p.line('#1c2935',[(x+1,y+5),(x-1,92),(x+3,108)],.65)
   for side in (-1,1):
    x=64+side*9;p.line('#a29e86',[(x,65),(x+side*4,77),(x+side*3,89)],.55)
   for n in range(16):p.oval('#596675',rng.uniform(48,80),rng.uniform(70,102),.7,.7)
   p.line('#c2b38e',[(62,73),(64,76),(66,73)],.6)
  else:
   for n in range(32 if boss else 22):
    x=rng.uniform(43 if boss else 50,84 if boss else 78);y=rng.uniform(56,94)
    p.line('#202f38',[(x,y),(x+1.7,y+1),(x+3,y+.2)],.4);p.oval('#a5a994' if n%3 else '#797367',x-.5,y-.7,.85,.65)
   for side in (-1,1):
    x=64+side*(19 if boss else 13);p.oval('#26343c',x-2,62,3.3,3.3);p.oval('#aab2a5',x-1.4,62.4,1.1,1.1)
   for n in range(3):
    y=81+n*5;p.line('#a8aba0',[(52 if boss else 56,y),(62,y+3),(70 if boss else 68,y+1)],.6)
    p.line('#24333c',[(58,y+3),(63,y+4),(72,y+2)],.55)
   if boss:
    for side in (-1,1):
     p.line('#2a343a',[(64+side*14,69),(64+side*10,76),(64+side*14,83)],1.2)
     p.line('#df9550',[(64+side*14,69),(64+side*10,76)],.55)
     p.line('#c89a63',[(64+side*18,57),(64+side*22,61),(64+side*19,64)],.7)
   hx=64+fx*3;hy=43 if boss else 51
   p.soft('#c1c0a0',hx-5,hy-5,22,.45);p.soft('#152833',hx+8,hy+5,18,.55)
   if front:
    for side in (-1,1):
     p.line('#b0b2a0',[(hx+side*2,hy-2),(hx+side*7,hy-4),(hx+side*9,hy-1)],.7)
     p.line('#f5bd72',[(hx+side*2,hy+2),(hx+side*6,hy+1)],1);p.oval('#fff2be',hx+side*4-.3,hy+1,.7,.7)
    p.poly('#81897d',[(hx-1,hy+3),(hx+1,hy+3),(hx+2,hy+8),(hx-2,hy+9)],True)
    p.line('#313b3c',[(hx-4,hy+11),(hx+1,hy+12),(hx+5,hy+10)],.7)
   if boss:
    for n in range(5):
     x=hx+(n-2)*4;p.line('#a1ab9d',[(x-1,hy-18),(x-.3,hy-12)],.6)
 elif part==0:
  if kind==5:
   for side in (-1,1):
    for n in range(6):
     p.line('#ba9475',[(64+side*(15+n*2),68+n),(64+side*(24+n*2),57+n*3),(64+side*(33+n),52+n*4)],.45)
  elif kind in (3,4,6):
   for n in range(12):p.line('#84908f' if n%3 else '#1a2937',[(46+n*3,79),(45+n*3,96),(48+n*3,103)],.45)
 else:
  side=-1 if part in (1,4) else 1
  if quad:
   fore=part>=4;cx=64+fx*(22 if fore else -18)+side*(6 if abs(fx)>.65 else 16);cy=99+fy*(8 if fore else -8)
   p.soft('#c5ba8f',cx-2,cy-6,14,.5)
   for n in range(5):p.line('#2b3c40',[(cx-3,cy-12+n*3),(cx+2,cy-11+n*3)],.5)
   for n in range(3):p.line('#ece1bc',[(cx-2+n*3,cy+9),(cx-1+n*3+fx*2,cy+11)],.7)
  elif kind in (4,6):
   cx=64+side*((27 if boss else 18) if part>=4 else (13 if boss else 9))
   for n in range(12):
    x=cx+rng.uniform(-6,6);y=rng.uniform(70 if part>=4 else 87,108)
    p.line('#27373d',[(x,y),(x+1,y+3),(x-1,y+4)],.6);p.oval('#adb09b',x-.7,y-.6,.8,.6)
   if boss:p.line('#d89756',[(cx+2,77),(cx-2,83),(cx+1,90),(cx-1,94)],.65)
   if kind==4 and part==4:
    for n in range(4):p.line('#768b92',[(cx-6,83+n*4),(cx-4,87+n*4)],.5)
    p.oval('#bfb28d',cx-6,84,1.2,1.2);p.oval('#bfb28d',cx+4,86,1.2,1.2)
   if kind==4 and part==5:p.line('#e0dcc0',[(cx+6,95),(cx+13,65),(cx+15,58)],.8)
  elif kind==2:
   cx=64+side*(18 if part>=4 else 9)
   p.line('#d5a878',[(cx-1,75 if part>=4 else 90),(cx+1,82 if part>=4 else 98)],.7)
   for n in range(3):p.line('#5c4236',[(cx-2+n*2,96 if part>=4 else 111),(cx-1+n*2,99 if part>=4 else 114)],.45)

def additional_scenery(p,kind,variant):
 rng=random.Random(4500+kind*73+variant*51)
 if kind==0:
  # Split, low boulders rather than another row of columns.
  for n in range(4):
   x=65+n*19;y=190+(n%2)*12;p.rock(x,y,43+variant*3,34+(n%3)*13,'#5c5b58',True,n+variant*31)
   p.line('#e2a463',[(x-5,y-10),(x+1,y-3),(x-3,y+8)],.65)
 elif kind==1:
  h=36+variant*8;p.poly('#303c40',[(65,209),(70,208-h),(109,205-h),(117,211),(98,215)],True)
  p.poly('#6e7166',[(71,207-h),(81,201-h),(108,205-h),(113,212-h),(94,216-h),(73,213-h)],True)
  p.oval('#323e42',76,204-h,30,10);p.oval('#978e73',80,206-h,22,6);p.oval('#555b50',85,207-h,13,3)
  for n in range(7):p.line('#858476' if n%2 else '#202f39',[(72+n*6,214-h),(71+n*6,206)],.8)
  for side in (-1,1):p.line('#4e5a55',[(90,207),(90+side*24,214),(90+side*38,210)],7)
  for n in range(14):
   x=50+n*6;lean=rng.uniform(-8,8);p.line('#91896e',[(x,213),(x+lean,204-rng.uniform(2,19))],.65)
 elif kind==2:
  for n in range(7):p.rock(56+n*11,203+(n%2)*5,28,20+(n%3)*8,'#696c5c',False,n+variant)
  p.oval('#8d8560',62,187,63,26);p.oval('#293a3e',68,188,49,19);p.oval('#151f2a',74,189,35,12)
  p.line('#c1b176',[(64,195),(76,186),(98,184),(115,190)],2)
  for n in range(16):p.oval('#cbbf87',rng.uniform(59,123),rng.uniform(191,214),1.4,1)
 else:
  h=105+variant*17;lean=(variant-1)*5
  p.poly('#263541',[(73,207),(75+lean,210-h),(91+lean,194-h),(108+lean,209-h),(109,210),(94,218)],False)
  p.poly('#55636c',[(76,204),(78+lean,210-h),(90+lean,200-h),(91,210)],False)
  p.line('#a9ac99',[(79+lean,211-h),(90+lean,201-h),(103+lean,211-h)],1)
  for n in range(3):
   y=224-h+n*19;p.line('#d5a365',[(85+lean,y),(92+lean,y+6),(85+lean,y+11),(97+lean,y+14)],1)
  p.line('#253541',[(94+lean,213-h),(89,192),(97,204)],1.2)
 for n in range(8):p.rock(52+rng.random()*78,210+rng.random()*6,7+rng.random()*10,4+rng.random()*6,'#515d5e',False,n+kind*11)

def retain_finish(p,root,name):
 p.deselect();kra=root/(name+'.kra');png=root/(name+'.png');p.c.call('krita_save_export',document_id=p.doc,kra_path=str(kra),png_path=str(png),overwrite=False)
 for attempt in range(3):
  try:p.c.call('krita_preview',document_id=p.doc,max_width=1000,max_height=750);break
  except RuntimeError:
   if attempt==2:raise
   time.sleep(1)
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None
 return {'name':name,'layers':p.layers,'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'retained_saved_document_id':p.doc,'retention_reason':'Avoid native close crash observed after editing a loaded KRA'}

def run(source,root,runtime,resume=False):
 root.mkdir(parents=True,exist_ok=resume);runtime.mkdir(parents=True,exist_ok=True);(root/'input').mkdir(exist_ok=resume);start_krita_if_needed()
 for kind,name in enumerate(NAMES):
  if resume and (root/(name+'-receipt.json')).exists():
   receipt=json.loads((root/(name+'-receipt.json')).read_text(encoding='utf8'))
   assert hashlib.sha256((root/(name+'.kra')).read_bytes()).hexdigest()==receipt['kra_sha256']
   assert hashlib.sha256((root/(name+'.png')).read_bytes()).hexdigest()==receipt['png_sha256']
   assert (runtime/(name+'.png')).exists();print('REUSED verified',name,flush=True);continue
  assert not (root/(name+'.kra')).exists(),'Inspect existing saved master before resuming'
  shutil.copy2(source/(name+'.kra'),root/'input'/(name+'.kra'))
  with KritaMCP(root/('logs-'+name),timeout=240) as c:
   opened=c.call('krita_document',action='open',path=str(root/'input'/(name+'.kra')));doc=opened['affected_document_id'];state=c.call('krita_state',document_id=doc)
   assert state['runtime_compatibility']['status']=='compatible';layers={l['name']:l['id'] for l in state['layers']}
   p=Paint.__new__(Paint);p.c=c;p.doc=doc;p.width=2048;p.height=1536;p.lid=state['active_layer_id'];p.layers=list(layers);p.s=2;p.ox=p.oy=0
   for part,label in enumerate(PARTS):
    c.call('krita_edit',action='select_opaque',document_id=doc,layer_id=layers[label]);p.layer('Material detail / '+label)
    for direction in range(8):p.ox=direction*256;p.oy=part*256;details(p,kind,part,direction)
    p.deselect();print(name,label,'detailed',flush=True)
   receipt=retain_finish(p,root,name)
  size=256 if name=='varkhor' else 192;im=Image.open(root/(name+'.png')).convert('RGBA').resize((size*8,size*6),Image.Resampling.LANCZOS);im.save(runtime/(name+'.png'));(root/name).mkdir()
  for part,label in enumerate(PARTS):im.crop((size,part*size,size*2,(part+1)*size)).save(root/name/(label+'.png'))
  receipt['original_kra_sha256']=hashlib.sha256((source/(name+'.kra')).read_bytes()).hexdigest();(root/(name+'-receipt.json')).write_text(json.dumps(receipt,indent=2),encoding='utf8')
 with KritaMCP(root/'logs-extra-scenery',timeout=240) as c:
  p=Paint(c,1440,1840,'Caldera additional scenery')
  for kind,name in enumerate(['Cracked volcanic boulders','Burnt stumps and ash grass','Sulphur craters','Runic basalt monoliths']):
   p.layer(name)
   for variant in range(4):p.s=2;p.ox=kind*360;p.oy=variant*460;additional_scenery(p,kind,variant)
  receipt=retain_finish(p,root,'scenery-extra')
 im=Image.open(root/'scenery-extra.png').convert('RGBA');im.save(runtime/'scenery-extra.png');a=im.getchannel('A');edge=ImageChops.subtract(a.filter(ImageFilter.MaxFilter(7)),a);out=Image.new('RGBA',im.size,(255,255,255,0));out.putalpha(edge);out.save(runtime/'scenery-extra-edge.png');(root/'scenery-extra-receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf8')
 print('PASS: native actor details and sixteen additional scenery sprites',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--resume',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.source,a.root,a.runtime,a.resume)

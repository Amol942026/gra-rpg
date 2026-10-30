"""Original layered 2D Krita cutouts and terrain for the third realm. Offline MCP."""
from pathlib import Path
import argparse, json, math, hashlib, shutil, zipfile
from PIL import Image
from krita_mcp_client import KritaMCP, start_krita_if_needed

NAMES=['slag-crawler','lava-salamander','fire-imp','ash-wraith','obsidian-sentinel','ember-drake','varkhor']
PARTS=['back','leg-left','leg-right','body','arm-left','arm-right']
ANGLES=[0,math.atan(.5),math.pi/2,math.pi-math.atan(.5),math.pi,math.pi+math.atan(.5),math.pi*1.5,math.tau-math.atan(.5)]

def create(root:Path, runtime:Path):
 root.mkdir(parents=True,exist_ok=False);runtime.mkdir(parents=True,exist_ok=True)
 start_krita_if_needed();receipts=[]
 with KritaMCP(root/'mcp',timeout=240) as c:
  for name in NAMES+['terrain','scenery','sky']:
   creature=name in NAMES
   width,height=(1536,1152) if creature else (768,96) if name=='terrain' else (1440,460) if name=='scenery' else (1600,700)
   doc=c.call('krita_document',action='create',width=width,height=height,name='Emberfall / Kaldera Popiołu / '+name)['affected_document_id']
   state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
   lid=state['active_layer_id'];c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=lid);c.call('krita_edit',action='deselect',document_id=doc)
   ox=oy=0;s=1
   def layer(label):
    nonlocal lid
    c.call('krita_layers',action='create',document_id=doc,name=label);lid=c.call('krita_state',document_id=doc)['active_layer_id']
   def draw(**kw):
    try:c.call('krita_draw',document_id=doc,layer_id=lid,**kw)
    except RuntimeError as e:
     if 'DRAW_NO_CHANGE' not in str(e):raise
   def poly(color,points):draw(shape='polygon',points=[{'x':round(ox+x*s,3),'y':round(oy+y*s,3)} for x,y in points],foreground=color,fill_style='ForegroundColor',stroke_style='None')
   def oval(color,x,y,w,h):draw(shape='ellipse',x=ox+x*s,y=oy+y*s,width=w*s,height=h*s,foreground=color,fill_style='ForegroundColor',stroke_style='None')
   def line(color,points,w=1):draw(shape='path',points=[{'x':round(ox+x*s,3),'y':round(oy+y*s,3)} for x,y in points],foreground=color,fill_style='None',stroke_style='ForegroundColor',brush_size=w*s)
   def stone(x,y,w,h,color='#42414b',hot=False):
    poly('#211f2b',[(x-w*.52,y),(x-w*.36,y-h*.55),(x+w*.16,y-h*.62),(x+w*.52,y-h*.23),(x+w*.43,y+h*.42),(x-w*.22,y+h*.5)])
    poly(color,[(x-w*.45,y),(x-w*.29,y-h*.48),(x+w*.14,y-h*.52),(x+w*.42,y-h*.2),(x+w*.34,y+h*.3),(x-w*.22,y+h*.36)])
    line('#76707a',[(x-w*.3,y-h*.37),(x-w*.12,y-h*.43),(x+w*.14,y-h*.41)],1.2)
    if hot:line('#e26435',[(x,y-h*.4),(x-2,y),(x+4,y+h*.3)],1.7)
   if creature:
    species=NAMES.index(name);quad=species in (0,1,5);boss=species==6
    for part in range(6):
     layer(PARTS[part]);print(name,PARTS[part],flush=True)
     for direction,a in enumerate(ANGLES):
      ox=direction*192;oy=part*192;s=1.5;fx,fy=math.cos(a),math.sin(a);front=fy>=-.1
      dark='#292b36';base='#51434a' if species in (1,2,5) else '#45434f';light='#85808a';ember='#fa843b';gold='#ffd58b'
      if part==0:
       if species in (1,5):
        tail=[(64-fx*13,88-fy*5),(64-fx*31,84-fy*9),(64-fx*40,92-fy*9),(64-fx*47,86-fy*8)]
        line(dark,tail,8);line('#9b4739',tail,4);line(ember,tail[1:],1.2)
       if species==5:
        for side in (-1,1):
         poly('#262635',[(64+side*13,76),(64+side*42,47),(64+side*39,73),(64+side*49,86),(64+side*20,91)])
         poly('#824135',[(64+side*15,76),(64+side*39,52),(64+side*35,74),(64+side*44,83),(64+side*20,88)])
         line('#bd6d49',[(64+side*13,78),(64+side*39,52),(64+side*35,74),(64+side*44,83)],1.2)
       if species==3:
        for side in (-1,1):poly('#393642',[(64+side*9,78),(64+side*26,96),(64+side*20,110),(64+side*16,98),(64+side*11,116),(64,96)])
       if species in (2,4,6):
        poly('#302d3b',[(49,64),(75,64),(87 if boss else 81,105),(68,101),(53,111),(42,102)])
        line('#796575',[(51,69),(48,96),(54,104)],1.2)
      elif part in (1,2) or (quad and part in (4,5)):
       side=-1 if part in (1,4) else 1
       if quad:
        fore=part>=4;cx=64+fx*(17 if fore else -15)+side*(7 if abs(fx)>.6 else 13);cy=97+fy*(8 if fore else -6)
        poly(dark,[(cx-5,cy-12),(cx+5,cy-12),(cx+6,cy+4),(cx+side*7,cy+11),(cx-5,cy+11),(cx-7,cy+3)])
        line('#a5624d',[(cx-2,cy-9),(cx+1,cy+2),(cx+side*3,cy+7)],3)
        for toe in (-1,1):poly('#dfc7aa',[(cx+toe*3,cy+7),(cx+toe*3+4,cy+13),(cx+toe*3-2,cy+11)])
       elif species==3:
        poly('#4d4654',[(64+side*4,91),(64+side*16,103),(64+side*13,113),(64+side*7,105),(64+side*4,118),(64-side*4,102)])
        line('#a19899',[(64+side*6,96),(64+side*9,105)],1)
       else:
        cx=64+side*(13 if boss else 9);stone(cx,99,13 if boss else 9,24,base,boss or species==4)
        poly(dark,[(cx-7,105),(cx+5,105),(cx+8+fx*3,114),(cx-8+fx*3,115)])
        line(light,[(cx-5,108),(cx-6+fx*2,112),(cx+5+fx*2,112)],1.2)
      elif part==3:
       if quad:
        w=30+abs(fx)*29;h=28 if species==0 else 24
        oval(dark,64-w/2-2,76,w+4,h+3);oval(base,64-w/2,76,w,h);oval('#72606a',64-w/2+3,78,w*.65,h*.43)
        if species==0:
         for n in range(4):stone(64+(n-1.5)*9,81-(n%2)*3,14,20,'#514e59',True)
        else:
         for n in range(6):
          px=64-fx*22+fx*n*7;py=77-fy*5+fy*n*2
          poly('#332c39',[(px-4,py+2),(px,py-9-(n%2)*4),(px+5,py+2)]);line(ember,[(px,py-5),(px+2,py)],1)
        hx=64+fx*(25 if species else 19);hy=86+fy*7
        oval(dark,hx-12,hy-10,24,21);oval('#814b43' if species else '#5e515a',hx-10,hy-9,20,16)
        oval('#b36f52',hx+fx*6-8,hy-3+fy*3,16,10)
        if front or abs(fx)>.7:
         oval('#241e29',hx+fx*5-7,hy-6,5,4);oval(gold,hx+fx*5-6,hy-6,3,2)
         line('#33252e',[(hx+fx*5-6,hy+5),(hx+fx*10,hy+6)],1)
         for tooth in (-1,1):poly('#efe0bd',[(hx+tooth*4,hy+5),(hx+tooth*4+2,hy+9),(hx+tooth*4+3,hy+5)])
       else:
        if species==3:
         poly('#292632',[(54,58),(73,58),(83,101),(73,99),(68,112),(59,101),(47,106)])
         poly('#68616b',[(56,67),(61,70),(61,94),(55,101),(51,101)])
         line('#b1a3a0',[(70,73),(75,96),(69,102)],1.3)
        else:
         stone(64,76,44 if boss else 29,38 if boss else 31,base,species==4 or boss)
         if boss:
          for side in (-1,1):stone(64+side*17,65,17,23,'#5a5560',True)
         oval('#8b3d38',58 if not boss else 54,70 if not boss else 66,12 if not boss else 20,18 if not boss else 26)
         oval(ember,60 if not boss else 57,73 if not boss else 70,7 if not boss else 13,11 if not boss else 17)
         oval(gold,62,76 if not boss else 73,3 if not boss else 6,6 if not boss else 10)
        hx=64+fx*3;hy=49 if boss else 55
        if species==3:
         oval('#282430',hx-14,hy-15,28,31);oval('#625965',hx-10,hy-13,17,26)
        else:stone(hx,hy,26 if boss else 21,25 if boss else 21,'#63525b',False)
        for side in (-1,1):
         if species in (2,6):poly('#b4a08e',[(hx+side*7,hy-6),(hx+side*16,hy-20),(hx+side*15,hy-8),(hx+side*11,hy-1)])
         if species==4 or boss:poly('#3b3444',[(hx+side*8,hy-7),(hx+side*10,hy-20),(hx+side*15,hy-9),(hx+side*11,hy+2)])
        if front:
         oval('#241e2b',hx-8,hy-3,16,9)
         for side in (-1,1):line(gold,[(hx+side*3,hy-1),(hx+side*6,hy-2)],1.5 if boss else 1)
         line('#b58972',[(hx-4,hy+7),(hx+4,hy+7)],1)
        else:line('#ad6658',[(hx-5,hy-6),(hx,hy+5),(hx+4,hy-2)],1.2)
      else:
       side=-1 if part==4 else 1;cx=64+side*(25 if boss else 18);cy=79
       if species==3:
        poly('#3c3645',[(cx-side*5,65),(cx+side*6,75),(cx+side*12,96),(cx+side*3,91),(cx-side*3,78)])
        line('#b09a91',[(cx+side*3,78),(cx+side*7,89)],1.1)
       else:
        stone(cx,cy,18 if boss else 10,27 if boss else 22,base,boss)
        stone(cx+side*4,94,23 if boss else 12,21 if boss else 12,'#57505b',boss)
       if part==5 and species==2:
        oval('#a64332',cx-8,cy+7,18,22);oval(ember,cx-5,cy+9,12,17);oval(gold,cx-2,cy+12,6,10)
       if part==5 and species==4:
        poly('#171d29',[(cx-2,91),(cx+4,89),(cx+12,65),(cx+15,65),(cx+11,92),(cx+3,100)])
        line('#e48e59',[(cx+5,91),(cx+13,68)],1.5);line('#ac8b77',[(cx-4,94),(cx+10,96)],2)
      # Repeated tiny cinder accents remain part of the native artwork.
   elif name=='terrain':
    s=1.5;layer('Basalt / popiół / droga / most / lawa / skorupa')
    for tile,col in enumerate(['#37333f','#51464c','#6d5d5c','#827171','#e36332','#482f38']):
     ox=tile*128;oy=0
     poly(col,[(0,32),(42.6667,0),(85.3333,32),(42.6667,64)])
     for n in range(15):
      x=24+(n*17%37);y=19+(n*13%29)
      if abs(x-42.7)/42.7+abs(y-32)/32>.85:continue
      if tile==4:line('#ffc065' if n%3 else '#8e3e31',[(x-4,y-2),(x+2,y),(x-1,y+4)],1.4)
      else:line('#917674' if n%3==0 else '#292734',[(x-4,y-2),(x,y),(x+4,y-2)],.8)
     if tile==3:
      for n in range(5):line('#413a47',[(19+n*7,19),(11+n*7,32),(26+n*7,44)],1.1)
   elif name=='scenery':
    s=2;layer('Bazalt / zwęglone drzewo / siarkowy komin / obsydian')
    for kind in range(4):
     ox=kind*360;oy=0
     if kind==0:
      for j in range(5):stone(61+j*14,185-j%3*13,32,65+j%3*27,'#534c59',j%3==0)
     elif kind==1:
      line('#242330',[(90,207),(87,129),(95,60),(89,18)],17);line('#71616c',[(85,202),(84,135),(92,64),(88,23)],4)
      for side in (-1,1):
       line('#2b2834',[(91,136),(91+side*32,106),(91+side*39,72)],10)
       line('#75626d',[(90,137),(91+side*30,106),(91+side*37,76)],2)
       line('#3d3039',[(86,204),(86+side*30,213)],8)
     elif kind==2:
      stone(90,188,82,56,'#77715f',False);oval('#292635',60,162,60,27);oval('#bd9856',67,170,46,12)
      for n in range(5):line('#b09972',[(72+n*8,158),(69+n*9,143),(75+n*7,130-n%2*8)],3)
     else:
      for j in range(4):
       x=65+j*17;poly('#1d2331',[(x-13,202),(x-9,154-j%2*50),(x+1,135-j%2*42),(x+13,159-j%2*29),(x+8,210)])
       line('#6c7183',[(x-9,199),(x-6,156-j%2*47),(x+1,140-j%2*42)],2)
       line('#c26756',[(x+2,155-j%2*29),(x+5,192)],1.5)
   else:
    layer('Nocny żar / góry i dym');s=1
    for row in range(18):
     y=row*40;v=row/17;poly('#%02x%02x%02x'%(int(35+v*32),int(29+v*10),int(47+v*2)),[(0,y),(1600,y),(1600,y+42),(0,y+42)])
    for ridge in range(3):
     color=['#4d3d50','#3c3346','#292b39'][ridge];points=[(0,700)]
     for x in range(-80,1681,80):points.append((x,410+ridge*65-130*abs(math.sin(x*.005+ridge*2))-35*math.sin(x*.012)))
     points.append((1600,700));poly(color,points)
    poly('#282a38',[(780,620),(1090,252),(1152,270),(1210,226),(1450,620)])
    line('#cb603b',[(1090,254),(1120,281),(1151,278),(1210,230)],8)
    line('#a2493b',[(1148,281),(1170,329),(1149,366),(1194,420)],4)
    for n in range(12):oval('#4f4354',1060-n*10,230-n*13,100+n*8,29+n*3)
   kra=root/(name+'.kra');png=root/(name+'.png')
   c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
   c.call('krita_preview',document_id=doc,max_width=1024,max_height=768)
   c.call('krita_document',action='close',document_id=doc,discard_changes=False)
   with zipfile.ZipFile(kra) as z:assert z.testzip() is None and 'maindoc.xml' in z.namelist()
   im=Image.open(png).convert('RGBA');assert im.size==(width,height)
   if creature:
    size=192 if name=='varkhor' else 128
    if size!=192:im=im.resize((size*8,size*6),Image.Resampling.LANCZOS)
    im.save(runtime/(name+'.png'));parts=root/name;parts.mkdir()
    for p,part in enumerate(PARTS):im.crop((size, p*size, size*2,(p+1)*size)).save(parts/(part+'.png'))
   else:shutil.copy2(png,runtime/(name+'.png'))
   receipts.append({'name':name,'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'native_size':[width,height]})
   print('SAVED',name,flush=True)
 (root/'provenance.json').write_text(json.dumps({'authoring':'Krita native MCP 0.2.0','original_2d':True,'external_assets':[],'assets':receipts},indent=2),encoding='utf8')
 print('PASS: seven layered Krita actors, terrain, scenery and sky',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();create(a.root,a.runtime)

"""Painted Caldera revision: native Krita contours, airbrush, grain and cutouts.

No downloaded imagery, AI image service or 3D authoring. Pillow only exports
cells, creates technical alpha masks and packs atlases after Krita exports.
"""
from pathlib import Path
import argparse,math,random,json,hashlib,zipfile,time
from PIL import Image,ImageFilter,ImageChops
from krita_mcp_client import KritaMCP,start_krita_if_needed
from create_menu_forest import smooth,mix
from create_volcano_art import NAMES,PARTS,ANGLES

class Paint:
 def __init__(self,c,width,height,name):
  self.width=width;self.height=height
  self.c=c;self.doc=c.call('krita_document',action='create',width=width,height=height,name='Emberfall / painted Caldera / '+name)['affected_document_id']
  state=c.call('krita_state',document_id=self.doc);assert state['runtime_compatibility']['status']=='compatible';self.lid=state['active_layer_id'];self.layers=[];self.ox=self.oy=0;self.s=1
  c.call('krita_edit',action='select_all',document_id=self.doc);c.call('krita_edit',action='clear',document_id=self.doc,layer_id=self.lid);c.call('krita_edit',action='deselect',document_id=self.doc)
 def layer(self,name,opacity=1):
  self.c.call('krita_layers',action='create',document_id=self.doc,name=name);self.lid=self.c.call('krita_state',document_id=self.doc)['active_layer_id'];self.layers.append(name)
  if opacity!=1:self.c.call('krita_layers',action='set_properties',document_id=self.doc,layer_id=self.lid,opacity=opacity)
 def draw(self,**kw):
  try:self.c.call('krita_draw',document_id=self.doc,layer_id=self.lid,**kw)
  except RuntimeError as e:
   if 'NO_CHANGE' not in str(e) and 'does not intersect the canvas' not in str(e):raise
 def poly(self,color,points,rounded=False):
  if rounded:points=smooth(points,True,4)
  self.draw(shape='polygon',foreground=color,points=[{'x':round(self.ox+x*self.s,3),'y':round(self.oy+y*self.s,3)} for x,y in points],fill_style='ForegroundColor',stroke_style='None')
 def oval(self,color,x,y,w,h):self.draw(shape='ellipse',foreground=color,x=self.ox+x*self.s,y=self.oy+y*self.s,width=w*self.s,height=h*self.s,fill_style='ForegroundColor',stroke_style='None')
 def line(self,color,points,w=1):
  points=smooth(points,False,4) if len(points)>2 else points
  self.draw(shape='path',foreground=color,preset_name='b) Basic-5 Size',points=[{'x':round(self.ox+x*self.s,3),'y':round(self.oy+y*self.s,3)} for x,y in points],fill_style='None',stroke_style='ForegroundColor',brush_size=w*self.s)
 def soft(self,color,x,y,size,opacity=.25):
  px=self.ox+x*self.s;py=self.oy+y*self.s;r=size*self.s*.5
  if px < -r or py < -r or px>=self.width+r or py>=self.height+r:return
  px=max(0,min(self.width-1,px));py=max(0,min(self.height-1,py))
  self.c.call('krita_brush',action='set',document_id=self.doc,preset_name='b) Airbrush Soft',brush_size=size*self.s,opacity=opacity,foreground=color,disable_pressure=True)
  try:self.c.call('krita_paint_path',document_id=self.doc,layer_id=self.lid,preset_name='b) Airbrush Soft',brush_size=size*self.s,points=[{'x':max(0,px-2),'y':py,'pressure':1},{'x':min(self.width-1,px+2),'y':py,'pressure':1}])
  except RuntimeError as e:
   if 'NO_CHANGE' not in str(e):raise
  self.c.call('krita_brush',action='set',document_id=self.doc,preset_name='b) Basic-5 Size',opacity=1)
 def select(self):self.c.call('krita_edit',action='select_opaque',document_id=self.doc,layer_id=self.lid)
 def deselect(self):self.c.call('krita_edit',action='deselect',document_id=self.doc)
 def rock(self,x,y,w,h,color='#4c5155',hot=False,seed=0):
  pts=[(x-w*.5,y+h*.24),(x-w*.51,y-h*.2),(x-w*.22,y-h*.5),(x+w*.21,y-h*.49),(x+w*.48,y-h*.15),(x+w*.43,y+h*.34),(x-w*.17,y+h*.46)]
  self.poly(mix(color,'#141c23',.42),pts,True)
  inner=[(x+(a-x)*.94,y+(b-y)*.94-.35) for a,b in pts];self.poly(color,inner,True)
  self.poly(mix(color,'#a6a29b',.23),[(x-w*.44,y-h*.2),(x-w*.21,y-h*.45),(x+w*.17,y-h*.44),(x+w*.1,y-h*.1),(x-w*.23,y+h*.2)],True)
  self.poly(mix(color,'#222b34',.23),[(x+w*.12,y-h*.4),(x+w*.4,y-h*.14),(x+w*.37,y+h*.28),(x-w*.02,y+h*.4),(x+w*.03,y)],True)
  self.line(mix(color,'#c1b6a2',.38),[(x-w*.37,y-h*.24),(x-w*.19,y-h*.41),(x+w*.05,y-h*.42)],.7)
  rng=random.Random(seed)
  for n in range(5):
   xx=x+rng.uniform(-.31,.3)*w;yy=y+rng.uniform(-.28,.25)*h
   self.line(mix(color,'#a19c8b',.18) if n%2 else mix(color,'#202932',.35),[(xx,yy),(xx+w*.11,yy-h*.04)],.35)
  if hot:
   self.line('#753b32',[(x-1,y-h*.35),(x+2,y-h*.12),(x-2,y+h*.05),(x+1,y+h*.28)],2)
   self.line('#d78143',[(x-1,y-h*.35),(x+2,y-h*.12),(x-2,y+h*.05),(x+1,y+h*.28)],.65)
 def finish(self,root,name):
  kra=root/(name+'.kra');png=root/(name+'.png');self.deselect();self.c.call('krita_save_export',document_id=self.doc,kra_path=str(kra),png_path=str(png),overwrite=False)
  for attempt in range(3):
   try:self.c.call('krita_preview',document_id=self.doc,max_width=1000,max_height=750);break
   except RuntimeError:
    if attempt==2:raise
    time.sleep(1)
  self.c.call('krita_document',action='close',document_id=self.doc,discard_changes=False)
  with zipfile.ZipFile(kra) as z:assert z.testzip() is None
  return {'name':name,'layers':self.layers,'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest()}

def creature(p,kind,part,direction):
 a=ANGLES[direction];fx,fy=math.cos(a),math.sin(a);front=fy>=-.1;sideView=abs(fx);quad=kind in (0,1,5);boss=kind==6
 skin=['#4c5054','#4d5145','#9c624b','#4a4b57','#454f5b','#5d5149','#4e5355'][kind]
 dark=mix(skin,'#151f29',.53);rim=mix(skin,'#c4b496',.45)
 if part==0:
  if quad:
   tail=[(64-fx*13,88-fy*4),(64-fx*32,87-fy*8),(64-fx*44,97-fy*9),(64-fx*52,93-fy*8)]
   p.line(dark,tail,7 if kind!=0 else 4);p.line(skin,tail,4 if kind!=0 else 2);p.line(rim,[(x-.6,y-1) for x,y in tail],.65)
  elif kind==2:
   p.line(dark,[(63,83),(47,95),(38,105),(39,115)],3);p.line('#b07b58',[(63,83),(47,95),(38,105),(39,115)],1.2);p.poly('#8b5a46',[(39,112),(34,118),(43,116)])
  else:
   p.poly('#303842',[(48,62),(72,58),(85,83),(84,102),(77,99),(73,114),(64,108),(57,119),(52,104),(41,108)],True)
   for j in range(6):
    x=46+j*6;p.line('#65707a' if j%2 else '#242e39',[(x,72),(x-4,93),(x+2,106-(j%2)*9)],.65)
  if kind==5:
   for side in (-1,1):
    stretch=.76 if side*fx<0 else 1
    pts=[(64+side*9,75),(64+side*23*stretch,50),(64+side*40*stretch,38+fy*5),(64+side*35*stretch,63),(64+side*48*stretch,78),(64+side*29,74),(64+side*23,88)]
    p.poly('#2d343c',pts,True);p.poly('#765b4c',[(x,y+2) for x,y in pts[1:]],True)
    for tip in (pts[2],pts[4],pts[5]):p.line('#b18a60',[(64+side*10,75),(64+side*22,65),tip],.8)
    p.line('#d0aa79',[pts[1],pts[2]],.65)
  return
 if part in (1,2) or (quad and part>=4):
  side=-1 if part in (1,4) else 1
  if quad:
   fore=part>=4;cx=64+fx*(22 if fore else -18)+side*(6 if sideView>.65 else 16);cy=99+fy*(8 if fore else -8)
   joints=[(cx-side*2,cy-16),(cx+side*6,cy-6),(cx+side*1,cy+7)]
   p.line(dark,joints,8);p.line(skin,[(x-.7,y-.7) for x,y in joints],5);p.line(rim,[(x-1.6,y-.8) for x,y in joints],1)
   p.poly(skin,[(cx-5,cy+3),(cx+3,cy+3),(cx+8+fx*2,cy+9),(cx+1,cy+12),(cx-6,cy+10)],True)
   for n in range(3):p.line('#c6bea6',[(cx-2+n*3,cy+8),(cx-2+n*3+fx*3,cy+12)],.7)
   if kind==0:p.rock(cx+side*4,cy-6,10,11,'#55595b',True,part*11+direction)
  elif kind==3:
   cx=64+side*10;p.poly('#4b515c',[(cx-6,91),(cx+9,97),(cx+side*7,110),(cx+3,106),(cx-2,119),(cx-9,105)],True)
   p.line('#9b9993',[(cx-5,96),(cx-3,109)],.55);p.line('#262f3c',[(cx+3,99),(cx+5,108)],1)
  else:
   cx=64+side*(13 if boss else 9);cy=96
   p.line(dark,[(cx,84),(cx+side*2,99),(cx,111)],12 if boss else 8)
   if kind==2:
    p.line(skin,[(cx,85),(cx+side*3,99),(cx,112)],5);p.line('#c99c77',[(cx-1,88),(cx+side*2-1,97),(cx-1,108)],1)
   else:
    p.rock(cx,94,14 if boss else 10,24,'#51585d' if boss else '#515e68',boss,part+direction*7)
    p.rock(cx+side*1,108,16 if boss else 11,13,'#46505b',False,part+16)
   p.poly(dark,[(cx-7,109),(cx+4,108),(cx+9+fx*3,115),(cx-8+fx*2,117)],True)
   p.line(rim,[(cx-5,113),(cx-3,115),(cx+5+fx*2,114)],.65)
  return
 if part==3:
  if quad:
   w=(48 if kind==0 else 40)+sideView*(24 if kind==0 else 27);cy=84 if kind!=0 else 85;h=35 if kind==0 else 27
   p.oval(dark,64-w/2-1,cy-h*.55,w+2,h+3);p.oval(skin,64-w/2,cy-h*.55,w,h)
   p.select();p.soft('#a3a38a',55,cy-11,w*.9,.38);p.soft('#1b2832',78,cy+11,w*.8,.5);p.deselect()
   if kind==0:
    for n in range(5):
     x=64+(n-2)*(8+sideView*3);y=78+abs(n-2)*2
     p.rock(x,y,19,29,'#5c6569',n%2==1,n+direction*17)
     p.poly('#6b7374',[(x-5,y-10),(x-1,y-20-abs(n-2)),(x+5,y-8)],True)
   else:
    for n in range(25):
     x=64-w*.36+(n%9)*w*.08;y=75+(n//9)*7
     p.line('#89927b' if n%3 else '#303f40',[(x-1.5,y),(x,y+1.5),(x+2,y)],.45)
    for n in range(6):
     x=64-fx*22+fx*n*8;y=72-fy*5+fy*n*2
     p.poly('#4b3e37',[(x-3,y+3),(x,y-7-(n%2)*3),(x+5,y+2)],True);p.line('#b67c48',[(x,y-3),(x+2,y+2)],.8)
   hx=64+fx*(30 if kind else 24);hy=86+fy*10
   p.poly(dark,[(hx-12,hy-9),(hx+5,hy-11),(hx+13+fx*4,hy-1),(hx+12,hy+7),(hx-6,hy+9),(hx-13,hy+2)],True)
   p.poly('#677163' if kind else '#535e63',[(hx-10,hy-7),(hx+3,hy-9),(hx+11+fx*4,hy-1),(hx+8,hy+3),(hx-8,hy+4)],True)
   if kind==5:
    for side in (-1,1):p.poly('#a99e7f',[(hx+side*7,hy-6),(hx+side*11-fx*5,hy-18),(hx+side*3,hy-8)],True)
   if front or sideView>.8:
    ex=hx+fx*5; p.line('#222b30',[(ex-7,hy-4),(ex-3,hy-5),(ex,hy-3)],1.8);p.line('#edb46e',[(ex-6,hy-4),(ex-3,hy-4.5)],.9)
    if sideView<.7:p.line('#edb46e',[(hx+3,hy-4),(hx+6,hy-4.3)],.9)
    p.line('#29343a',[(hx-3+fx*4,hy+5),(hx+9+fx*2,hy+3)],.8)
    if kind!=0:
     for n in range(3):p.poly('#cbc4a7',[(hx+n*3-1,hy+4),(hx+n*3+.7,hy+7),(hx+n*3+1.5,hy+3)])
   return
  if kind==3:
   p.poly('#262e3c',[(55,57),(72,56),(80,73),(84,92),(76,98),(76,109),(69,103),(65,116),(57,105),(49,113),(45,97),(48,76)],True)
   for n in range(7):
    x=49+n*4;p.line('#687080' if n%2==0 else '#182535',[(x+2,66),(x-1,84),(x+3,101+(n%3)*3)],1.7 if n%2==0 else 2.2)
   p.select();p.soft('#b2aaa0',54,69,35,.24);p.soft('#111e2d',79,93,32,.45);p.deselect()
   hx=64+fx*4;p.poly('#4c5363',[(hx-14,59),(hx-13,43),(hx-5,34),(hx+6,35),(hx+14,48),(hx+12,63)],True)
   p.poly('#121f2e',[(hx-10,55),(hx-8,44),(hx,39),(hx+9,46),(hx+8,58),(hx,63)],True)
   p.line('#a7a6a3',[(hx-13,55),(hx-10,42),(hx-4,36)],.8)
   if front:
    p.line('#ccc8ae',[(hx-5,49),(hx-2,49)],.7);p.line('#ccc8ae',[(hx+3,49),(hx+5,49)],.7)
    p.oval('#aaa185',62,73,4,6);p.line('#b2a68a',[(54,62),(64,77),(74,62)],.5)
   else:p.line('#6d737b',[(hx-5,40),(hx,51),(hx+1,62)],.65)
   return
  if kind==2:
   p.poly(dark,[(54,64),(70,63),(76,76),(69,91),(56,92),(49,77)],True);p.poly(skin,[(55,65),(68,64),(73,76),(67,87),(56,89),(52,76)],True)
   p.line('#c18b65',[(55,70),(60,72),(64,69),(69,72)],1.2);p.line('#634a40',[(63,75),(61,82),(65,86)],.65)
   p.line('#a18466',[(54,90),(71,89)],2);p.oval('#333943',61,87,5,5)
  else:
   p.rock(64,75,48 if boss else 31,38 if boss else 33,'#51585a' if boss else '#4b5964',False,100+direction)
   for side in (-1,1):
    p.rock(64+side*(19 if boss else 13),64,21 if boss else 13,19,'#596269' if boss else '#526570',boss,side+direction*7)
    if boss:
     for n in range(2):p.poly('#5d6567',[(64+side*(17+n*5),61),(64+side*(19+n*6),46-n*7),(64+side*(24+n*5),62)],True)
   for n in range(3):
    p.poly('#424f59',[(49 if boss else 53,79+n*5),(64,82+n*5),(79 if boss else 75,78+n*5),(75 if boss else 72,86+n*5),(64,89+n*5),(52 if boss else 56,86+n*5)],True)
    p.line('#7a8587',[(52 if boss else 56,81+n*5),(63,84+n*5)],.6)
   if boss and front:
    p.poly('#252d34',[(57,65),(67,62),(74,69),(72,84),(63,91),(55,80)],True)
    p.oval('#8e482e',57,67,14,18);p.oval('#df8b43',59,69,10,14);p.oval('#f6cf81',61,71,5,9)
    for side in (-1,1):p.line('#c58246',[(64+side*7,71),(64+side*12,77),(64+side*17,74)],.65)
   elif boss:
    p.line('#a96843',[(63,58),(65,70),(61,81),(64,89)],1)
   elif front:
    p.line('#9e9b82',[(60,69),(64,76),(69,69)],.8);p.line('#cb8b51',[(64,74),(64,79)],.6)
  hx=64+fx*3;hy=43 if boss else 51
  if kind==2:
   p.poly('#724f42',[(hx-10,hy-7),(hx+7,hy-8),(hx+12+fx*3,hy),(hx+5,hy+13),(hx-6,hy+11),(hx-12,hy+1)],True)
   p.poly('#b27d59',[(hx-8,hy-6),(hx+4,hy-7),(hx+10,hy),(hx+2,hy+6),(hx-8,hy+4)],True)
   for side in (-1,1):p.poly('#b3a58b',[(hx+side*6,hy-6),(hx+side*14,hy-17),(hx+side*13,hy-7),(hx+side*10,hy-1)],True)
  else:
   p.rock(hx,hy,25 if boss else 22,25 if boss else 23,'#626a6b' if boss else '#566673',boss,94+direction)
   if boss:
    for n in range(5):
     xx=hx+(n-2)*4;p.poly('#535e65',[(xx-3,hy-9),(xx-2,hy-21-(n%2)*3),(xx+2,hy-23+(n%2)*3),(xx+4,hy-6)],True);p.line('#969b90',[(xx-2,hy-18),(xx-1,hy-10)],.55)
   else:
    p.poly('#627584',[(hx-10,hy-9),(hx-5,hy-17),(hx+8,hy-13),(hx+13,hy-3),(hx+1,hy-4)],True);p.line('#a2a8a0',[(hx-7,hy-10),(hx-3,hy-14),(hx+5,hy-12)],.7)
  if front:
   p.poly('#202d36',[(hx-9,hy-1),(hx,hy+1),(hx+9,hy-2),(hx+6,hy+5),(hx,hy+6),(hx-7,hy+4)],True)
   for side in (-1,1):p.line('#edb96e' if boss or kind==2 else '#d39064',[(hx+side*2,hy+2),(hx+side*6,hy+1)],.8)
   if kind==2:p.line('#d6c39c',[(hx-4,hy+9),(hx+4,hy+8)],.7)
   else:p.line('#8a938c',[(hx-5,hy+10),(hx,hy+12),(hx+5,hy+9)],.8)
  else:p.line('#8d958c',[(hx-4,hy-7),(hx-1,hy+4),(hx+3,hy+9)],.6)
  return
 # Arms / weapons for bipedal species.
 side=-1 if part==4 else 1;cx=64+side*(27 if boss else 18);cy=79
 if kind==3:
  p.poly('#353f4e',[(cx-side*5,63),(cx+side*5,73),(cx+side*10,91),(cx+side*14,97),(cx+side*6,94),(cx,87),(cx-side*5,75)],True)
  p.line('#929898',[(cx,69),(cx+side*5,83),(cx+side*9,92)],.8)
  for finger in range(3):p.line('#a9a89a',[(cx+side*9,92),(cx+side*(11+finger*2),98+finger)],.65)
 elif kind==2:
  p.line(dark,[(cx-side*3,65),(cx+side*4,79),(cx+side*1,92)],7);p.line(skin,[(cx-side*3,65),(cx+side*4,79),(cx+side*1,92)],5);p.line('#d2a77b',[(cx-side*3-1,66),(cx+side*4-1,77)],.9)
  p.oval(skin,cx-4,89,8,9)
  for n in range(3):p.line('#ac8061',[(cx-2+n*2,94),(cx-3+n*3,100)],.8)
  if part==5:
   p.oval('#74402e',cx-6,91,13,15);p.oval('#cb7f35',cx-4,90,10,14);p.oval('#f2c577',cx-2,92,5,9)
   for n in range(3):p.poly('#dba753',[(cx-5+n*4,93),(cx-4+n*4,83-n%2*3),(cx-1+n*4,94)],True)
 else:
  p.line(dark,[(cx-side*3,65),(cx+side*3,80),(cx-side*1,96)],15 if boss else 9)
  p.rock(cx,73,18 if boss else 12,25,'#525c64',boss,part+direction)
  p.rock(cx+side*2,90,19 if boss else 12,23,'#4b5862',boss,part+direction*11)
  p.poly('#3d4a53',[(cx-8,94),(cx+7,93),(cx+10,99),(cx+6,108),(cx-6,107),(cx-10,101)],True)
  for finger in range(3):p.line('#8f9c99',[(cx-5+finger*4,99),(cx-4+finger*4,106)],.7)
  if kind==4 and part==5:
   p.poly('#252f3d',[(cx-1,99),(cx+5,100),(cx+11,59),(cx+15,49),(cx+18,64),(cx+8,105),(cx+1,108)],True)
   p.poly('#667b8a',[(cx+6,94),(cx+13,62),(cx+15,56),(cx+13,75)],True);p.line('#afaa91',[(cx+4,99),(cx+14,59)],.7);p.line('#9e8c63',[(cx-6,100),(cx+10,103)],2)
  if kind==4 and part==4:
   p.poly('#253341',[(cx-11,77),(cx+5,76),(cx+12,85),(cx+9,104),(cx,112),(cx-12,101)],True);p.poly('#526773',[(cx-8,79),(cx+3,79),(cx+8,87),(cx+6,101),(cx,107),(cx-9,99)],True)
   p.line('#ad9f7b',[(cx-7,81),(cx+3,81),(cx+6,88),(cx+4,100),(cx,104),(cx-7,98),(cx-7,81)],.7)
   p.line('#bd9763',[(cx,85),(cx-3,92),(cx+3,94),(cx,101)],1)

def scenery(p,kind,variant):
 rng=random.Random(1400+kind*97+variant*13)
 # Connected ground rubble gives every shape a visible, grounded contact.
 if kind==0:
  count=[5,4,7,4][variant]
  for n in range(count):
   x=53+n*(73/(count-1));height=[55,129,81,105][variant]+rng.uniform(-20,30);y=200+rng.uniform(-5,5);w=rng.uniform(18,28)
   p.poly('#28333b',[(x-w/2,y),(x-w/2-2,y-height*.55),(x-w*.35,y-height),(x+w*.22,y-height-5),(x+w*.48,y-height*.85),(x+w*.45,y-height*.18),(x+w*.6,y+2)],True)
   p.poly('#515d63',[(x-w*.4,y-2),(x-w*.39,y-height*.56),(x-w*.25,y-height+1),(x+2,y-height-1),(x+w*.1,y-2)],True)
   p.poly('#374650',[(x+1,y-height),(x+w*.38,y-height*.85),(x+w*.35,y-height*.14),(x+w*.1,y)],True)
   p.line('#939990',[(x-w*.28,y-height+1),(x+1,y-height-2),(x+w*.3,y-height*.86)],.85)
   for j in range(6):
    yy=y-height*(.14+j*.13);p.line('#29353f',[(x-w*.33,yy-2),(x+w*.08,yy),(x+w*.35,yy-3)],.6)
   p.line('#78837f',[(x-w*.34,y-height*.84),(x-w*.38,y-14)],.6)
 elif kind==1:
  h=[90,158,137,110][variant];lean=[-12,16,-6,23][variant];base=[(90,208),(86,165),(91+lean*.5,112),(90+lean,208-h)]
  p.line('#27333c',base,17);p.line('#586166',[(x-2,y) for x,y in base],8);p.line('#8d9087',[(x-5,y) for x,y in base],1.2)
  for n in range(5):
   side=-1 if n%2 else 1;yy=179-n*19;xx=89+lean*(208-yy)/h
   pts=[(xx,yy),(xx+side*(18+n),yy-12),(xx+side*(27+n*2),yy-29),(xx+side*(22+n*3),yy-48)]
   p.line('#303c43',pts,9-n*.7);p.line('#7c817a',[(x-2,y) for x,y in pts],1)
   p.line('#a49a83',[pts[-1],(pts[-1][0]+side*2,pts[-1][1]-2)],2)
  for side in (-1,1):
   p.line('#36434a',[(90,205),(90+side*18,209),(90+side*33,213)],9);p.line('#8b8a7c',[(88,204),(89+side*18,207),(90+side*28,210)],1)
  for n in range(8):p.line('#2d383f',[(85+lean*n/10,199-n*12),(90+lean*n/10,193-n*12)],.8)
 elif kind==2:
  for n in range(7):p.rock(64+n*9,197+(n%3)*3,26,18,'#5c6260',False,n+variant*4)
  p.poly('#79745a',[(58,194),(65,180),(84,173),(104,176),(121,187),(119,202),(99,208),(75,203)],True)
  p.oval('#323c3c',67,181,46,20);p.oval('#202e33',73,184,35,14)
  p.line('#b2a66c',[(59,193),(67,183),(84,177),(105,180),(116,188)],2)
  for n in range(18):
   a=n*math.tau/18;x=90+math.cos(a)*rng.uniform(21,28);y=191+math.sin(a)*rng.uniform(10,14)
   p.oval('#c6b87c' if n%3 else '#8c8a63',x,y,rng.uniform(1.5,4),rng.uniform(1,2))
 else:
  for n in range(5):
   x=59+n*17;h=rng.uniform(45,105)+(variant%2)*22;lean=rng.uniform(-10,10);y=205+rng.uniform(-3,3)
   p.poly('#202e3c',[(x-11,y),(x-9,y-h*.77),(x+lean,y-h),(x+13,y-h*.67),(x+9,y+3)],False)
   p.poly('#415b70',[(x-8,y-h*.76),(x+lean,y-h+3),(x+3,y-h*.54),(x,y-7)],False)
   p.poly('#2b4055',[(x+lean+2,y-h+5),(x+11,y-h*.66),(x+7,y-1),(x+2,y-8)],False)
   p.line('#91a3a8',[(x-8,y-h*.7),(x+lean,y-h+5)],.8);p.line('#6f8193',[(x+3,y-h*.52),(x+2,y-8)],.6)
 for n in range(10):p.rock(55+rng.random()*70,207+rng.random()*7,6+rng.random()*9,4+rng.random()*6,'#505b60',False,n+kind*22)

def surfaces(p,name):
 if name=='lava-surface':lava_surface(p);return
 size=512 if name!='macro' else 1024;rng=random.Random(2817+len(name)*37)
 if name=='macro':
  p.layer('Distant ash and broad cooling fields')
  for n in range(45):p.soft(rng.choice(['#b2a393','#706f70','#252e37']),rng.randrange(size),rng.randrange(size),rng.uniform(90,270),.09)
  return
 p.layer('Base mineral colour');base='#41484b' if name=='ground' else '#8c482e' if name=='lava-surface' else '#756f65' if name=='bridge' else '#7d7c71'
 if name in ('ground','lava-surface','bridge'):p.poly(base,[(0,0),(512,0),(512,512),(0,512)])
 p.layer('Painted soft strata and weathering')
 if name=='road':
  for x in range(-40,570,70):p.soft('#a49b85',x,256,210,.35)
 elif name=='bank':
  for x in range(-40,570,70):p.soft('#232d34',x,256,280,.7)
 elif name=='steam':
  for n in range(8):p.soft('#b3b6ab',255+math.sin(n*1.7)*40,420-n*40,160+n*9,.16)
  return
 else:
  for n in range(40):
   x=rng.randrange(512);y=rng.randrange(512);col=rng.choice(['#898c80','#252f38','#655e54']) if name!='lava-surface' else rng.choice(['#d47b32','#522c2a','#c59b4c'])
   for dx,dy in [(0,0),(-512,0),(512,0),(0,-512),(0,512)]:p.soft(col,x+dx,y+dy,rng.uniform(60,160),.22)
 p.layer('Fine grains, fissures and exposed edges')
 if name=='lava-surface':
  for n in range(24):
   x=n*512/24;pts=[(x+math.sin(j*1.1+n)*12,j*64) for j in range(-1,10)]
   for off in (-512,0,512):
    p.line('#9b582e',[(x+off,y) for x,y in pts],14);p.line('#d28a3f',[(x+off,y+2) for x,y in pts],6);p.line('#e8b465',[(x+off,y+3) for x,y in pts],1.7)
  for n in range(85):
   x=rng.randrange(512);y=rng.randrange(512);w=rng.uniform(4,17);h=rng.uniform(3,10)
   for dx in (-512,0,512):
    p.poly('#34363a',[(x+dx-w,y),(x+dx-w*.4,y-h),(x+dx+w*.8,y-h*.5),(x+dx+w,y+h*.5),(x+dx,y+h)],True)
    p.line('#ad6f38',[(x+dx-w*.5,y+h*.8),(x+dx+w*.5,y+h*.7)],.8)
 elif name=='bridge':
  for row in range(8):
   y=row*64;p.line('#454d51',[(0,y),(512,y)],3)
   for col in range(5):
    x=col*128+(row%2)*64;p.line('#464d51',[(x,y),(x-3,y+30),(x+2,y+64)],2);p.line('#ada594',[(x+5,y+5),(x+106,y+4)],1)
  p.line('#c0b7a2',[(0,17),(512,17)],7);p.line('#414c55',[(0,35),(512,35)],5);p.line('#c0b7a2',[(0,485),(512,485)],7);p.line('#414c55',[(0,465),(512,465)],5)
 else:
  for n in range(160 if name=='ground' else 60):
   x=rng.randrange(512);y=rng.randrange(512)
   if name in ('road','bank') and abs(y-256)>85:continue
   col=rng.choice(['#647072','#323e48','#85877b','#59605f'])
   p.oval(col,x,y,rng.uniform(1,6),rng.uniform(1,3))
  for n in range(20):
   x=rng.randrange(512);y=rng.randrange(512)
   if name in ('road','bank') and abs(y-256)>65:continue
   p.line('#303c44' if name!='road' else '#77766b',[(x,y),(x+5,y+7),(x+1,y+13),(x+8,y+18)],.65)

def skyline(p):
 p.layer('Distant caldera air');p.poly('#262d37',[(0,0),(1600,0),(1600,700),(0,700)])
 for x in range(0,1700,220):p.soft('#9b6351',x,490,650,.22)
 for row,color in enumerate(['#55535a','#444650','#303a47']):
  rng=random.Random(910+row);base=350+row*105;points=[(-70,700),(-70,base)]
  for x in range(0,1700,60):points.append((x,base-rng.uniform(15,130)-(90 if 700<x<1000 else 0)))
  points.extend([(1670,700)]);p.layer('Ash ridge '+str(row));p.poly(color,points,True)
  for n in range(18):
   x=rng.randrange(1600);y=base+rng.uniform(30,120);p.line('#5a5558' if row<2 else '#414750',[(x,y),(x+20,y-15),(x+60,y+4)],1)
 p.layer('Volcanic glow and suspended ash')
 for x,y in [(180,570),(840,480),(1430,590)]:
  p.soft('#b27344',x,y,270,.2);p.soft('#bba188',x+35,y-180,230,.08)
  p.line('#b67b4c',[(x-40,y+12),(x,y),(x+48,y+14)],1)

def lava_surface(p):
 rng=random.Random(90218);p.layer('Molten rock beneath crust');p.poly('#df8038',[(0,0),(512,0),(512,512),(0,512)])
 for n in range(25):p.soft('#f4c175' if n%2 else '#99412b',rng.randrange(512),rng.randrange(512),130,.35)
 # Periodic, irregular cooling plates; geometry is authored as native 2D paths.
 # Half-plane clipping avoids an ordered row of round stones or parallel lines.
 seeds=[(rng.uniform(0,512),rng.uniform(0,512)) for _ in range(42)]
 def cell(cx,cy):
  poly=[(cx-512,cy-512),(cx+512,cy-512),(cx+512,cy+512),(cx-512,cy+512)]
  for sx,sy in seeds:
   for dx in (-512,0,512):
    for dy in (-512,0,512):
     x,y=sx+dx,sy+dy;nx,ny=x-cx,y-cy
     if abs(nx)+abs(ny)<.001:continue
     boundary=(x*x+y*y-cx*cx-cy*cy)/2;out=[]
     for a,b in zip(poly,poly[1:]+poly[:1]):
      da=a[0]*nx+a[1]*ny-boundary;db=b[0]*nx+b[1]*ny-boundary
      if da<=0:out.append(a)
      if (da<0)!=(db<0):
       t=da/(da-db);out.append((a[0]+(b[0]-a[0])*t,a[1]+(b[1]-a[1])*t))
     poly=out
     if not poly:return []
  return [(cx+(x-cx)*.91,cy+(y-cy)*.91) for x,y in poly]
 p.layer('Fractured basalt cooling plates')
 for cx,cy in seeds:
  polygon=cell(cx,cy);color=rng.choice(['#3e3c3c','#48433f','#42403e','#4b4540'])
  for dx in (-512,0,512):
   for dy in (-512,0,512):
    q=[(x+dx,y+dy) for x,y in polygon]
    if not q:continue
    if max(x for x,y in q)<0 or min(x for x,y in q)>512 or max(y for x,y in q)<0 or min(y for x,y in q)>512:continue
    p.poly('#ad5833',q);p.poly(color,[(cx+dx+(x-cx-dx)*.95,cy+dy+(y-cy-dy)*.95) for x,y in q])
    top=sorted(q,key=lambda xy:xy[1])[:3];p.line('#9e7250',top,.8)
    p.line('#282f34',[(cx+dx-9,cy+dy-5),(cx+dx+2,cy+dy+2),(cx+dx-1,cy+dy+11)],.7)
 p.select()
 for n in range(24):p.soft('#a18d70' if n%2 else '#171f29',rng.randrange(512),rng.randrange(512),90,.22)
 p.deselect();p.layer('Fine mineral grain')
 for n in range(75):p.oval('#78614c',rng.randrange(512),rng.randrange(512),1,1)

def create(root,runtime,only):
 root.mkdir(parents=True,exist_ok=True);runtime.mkdir(parents=True,exist_ok=True);start_krita_if_needed();receipts=[]
 assets=NAMES+['scenery','ground','macro','road','bank','bridge','lava-surface','steam','sky'] if only=='all' else only.split(',')
 for name in assets:
  if (root/(name+'.kra')).exists():raise FileExistsError('Use fresh sources: '+name)
  creature_name=name in NAMES
  width,height=(2048,1536) if creature_name else (1440,1840) if name=='scenery' else (1600,700) if name=='sky' else (1024,1024) if name=='macro' else (512,512)
  with KritaMCP(root/('logs-'+name),timeout=240) as c:
   p=Paint(c,width,height,name)
   if creature_name:
    kind=NAMES.index(name)
    for part,label in enumerate(PARTS):
     p.layer(label);print(name,label,flush=True)
     for direction in range(8):p.ox=direction*256;p.oy=part*256;p.s=2;creature(p,kind,part,direction)
   elif name=='scenery':
    for kind,label in enumerate(['Layered weathered basalt','Splintered burned trees','Sulphur fumaroles','Fractured obsidian']):
     p.layer(label)
     for variant in range(4):p.ox=kind*360;p.oy=variant*460;p.s=2;scenery(p,kind,variant)
     print('scenery',label,flush=True)
   elif name=='sky':skyline(p)
   else:surfaces(p,name)
   receipt=p.finish(root,name);receipts.append(receipt)
  im=Image.open(root/(name+'.png')).convert('RGBA')
  if creature_name:
   size=256 if name=='varkhor' else 192;im=im.resize((size*8,size*6),Image.Resampling.LANCZOS);im.save(runtime/(name+'.png'));folder=root/name;folder.mkdir()
   for part,label in enumerate(PARTS):im.crop((size,part*size,size*2,(part+1)*size)).save(folder/(label+'.png'))
  else:im.save(runtime/(name+'.png'))
  if name=='scenery':
   alpha=im.getchannel('A');edge=ImageChops.subtract(alpha.filter(ImageFilter.MaxFilter(7)),alpha);mask=Image.new('RGBA',im.size,(255,255,255,0));mask.putalpha(edge);mask.save(runtime/'scenery-edge.png')
  (root/(name+'-receipt.json')).write_text(json.dumps(receipt,indent=2),encoding='utf8');print('SAVED painted',name,flush=True)
 print('PASS: native Krita painted sources and runtime exports',flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--only',default='all');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();create(a.root,a.runtime,a.only)

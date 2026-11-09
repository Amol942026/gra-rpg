"""Original layered 2D spell illustrations inside the retained Krita worker."""
import math,random
from PyQt5.QtGui import QColor,QRadialGradient,QPainterPath
from PyQt5.QtCore import Qt
from krita_abyss_chests import Master,polygon,line,ellipse,gradient,curve

def glow(p,x,y,r,color,alpha):
 g=QRadialGradient(x,y,r);c=QColor(color);c.setAlpha(alpha);g.setColorAt(0,c);c.setAlpha(0);g.setColorAt(1,c);ellipse(p,x-r,y-r,r*2,r*2,g)
def draw(p,spell,row):
 rng=random.Random(9141+spell*31+row)
 if spell==0:
  glow(p,0,0,112,'#ff972e',150)
  for i in range(7):
   a=i*math.tau/7;path=QPainterPath();path.moveTo(-14,38);path.cubicTo(-56,-4,-18,-47,-12,-72);path.cubicTo(-6,-26,21,-34,8,-93);path.cubicTo(61,-40,64,16,12,43);path.closeSubpath();p.save();p.rotate(i*51 if row==2 else i*4-12);p.setPen(Qt.NoPen);p.setBrush(QColor(239,75+i*14,21,45+i*19));p.drawPath(path);p.restore()
  glow(p,0,10,51,'#ffce6d',220);ellipse(p,-27,-24,54,65,gradient((-18,-22),(18,40),[(0,'#fff6ce'),(.4,'#ffe39a'),(1,'#ff9c28')]))
  for n in range(22):
   a=rng.uniform(0,math.tau);r=rng.uniform(35,100);ellipse(p,math.cos(a)*r,math.sin(a)*r,2.2,4,'#ffdc82')
 elif spell==1:
  glow(p,0,0,112,'#57bce7',140)
  if row==2:
   for n in range(12):
    a=n*math.tau/12;p.save();p.rotate(n*30);polygon(p,[(24,-3),(76+n%3*10,-6),(101,0),(62,7)],gradient((20,-5),(100,5),[(0,'#275e9b'),(.4,'#7fe1ee'),(1,'#efffff')]),'#bdeeff',.7);p.restore()
  else:
   points=[(-92,-11),(-45,-24),(52,-18),(106,0),(47,20),(-44,26),(-89,8)]
   if row==0:p.rotate(-45)
   polygon(p,points,gradient((-85,-20),(83,22),[(0,'#306999'),(.25,'#a1e5ef'),(.65,'#5aacda'),(1,'#f1ffff')]),'#c4f7ff',1.1)
   polygon(p,[(-88,-11),(-32,0),(106,0),(52,-18),(-45,-24)],'#a8eff1')
   line(p,[(-87,-8),(-32,0),(97,0)],'#f2ffff',2);line(p,[(-32,0),(-44,23)],'#497fae',1.4)
   for n in range(8):line(p,[(-63+n*17,-9),(-57+n*15,5)],QColor(238,255,255,145),.8)
  for n in range(15):
   a=rng.uniform(0,math.tau);r=rng.uniform(38,104);polygon(p,[(math.cos(a)*r,math.sin(a)*r),(math.cos(a)*r+2,math.sin(a)*r+5),(math.cos(a)*r-3,math.sin(a)*r+2)],'#b6f2ff')
 else:
  glow(p,0,0,117,'#6483ff',140);glow(p,0,0,78,'#8f60e5',185)
  ellipse(p,-56,-56,112,112,gradient((-48,-52),(49,55),[(0,'#c4f7ff'),(.22,'#91ceef'),(.52,'#5169b6'),(.8,'#273978'),(1,'#6b5bba')]))
  glow(p,-12,-12,37,'#efffff',220)
  for n in range(14 if row==2 else 10):
   a=n*math.tau/(14 if row==2 else 10);points=[]
   for j in range(7):
    r=8+j*(14 if row==2 else 8);side=rng.uniform(-9,9);points.append((math.cos(a)*r-math.sin(a)*side,math.sin(a)*r+math.cos(a)*side))
   line(p,points,QColor(147,163,255,95),8);line(p,points,'#a2dcff',2.7);line(p,points,'#f0fdff',.9)
 if row==2:glow(p,0,0,34,'#fff8d7' if spell==0 else '#d9fcff',230)

def produce(output,retained):
 output.mkdir(parents=True,exist_ok=False);m=Master(output,'eldrin-three-elements',768,768,retained)
 def paint(p):
  for row in range(3):
   for spell in range(3):p.save();p.translate(spell*256+128,row*256+128);draw(p,spell,row);p.restore()
 m.layer('Ember, faceted ice and storm plasma with transparent glow',paint)
 def glints(p):
  for row in range(3):
   for spell in range(3):
    x=spell*256+128;y=row*256+128
    if spell==1:line(p,[(x+21,y-27),(x+27,y-27)],'#efffff',1);line(p,[(x+24,y-30),(x+24,y-24)],'#efffff',1)
    else:glow(p,x-8,y-8,15,'#fff4c7' if spell==0 else '#eaffff',175)
 m.layer('Hot core, frost glints and electrical highlights',glints)
 return {'assets':[m.save()],'tool':'Krita native layered 2D','spells':['fireball','ice-shard','lightning-orb'],'roles':['icon','projectile','impact'],'cell':256,'external_inputs':[]}

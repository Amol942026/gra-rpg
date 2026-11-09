"""Layered 2D inventory art authored in the retained native Krita worker."""
import math,random,json
from PyQt5.QtGui import QColor,QPainterPath
from PyQt5.QtCore import Qt
from krita_abyss_chests import Master,polygon,line,ellipse,gradient,curve

KINDS=[8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,27,29,30,31,32,33,34]
def body(p,k):
 edge='#17242d';ivory=gradient((-25,-35),(20,38),[(0,'#ffedc1'),(.4,'#d5c6a3'),(1,'#887c67')]);gold=gradient((-30,-40),(30,40),[(0,'#fff0b1'),(.28,'#dca74a'),(.7,'#a8742d'),(1,'#59432b')])
 if k in (8,9,34):
  path=QPainterPath();path.moveTo(-11,-38);path.lineTo(-11,-20);path.cubicTo(-12,-12,-30,-9,-31,9);path.cubicTo(-32,43,-16,45,0,45);path.cubicTo(17,45,33,41,31,9);path.cubicTo(31,-8,12,-12,11,-20);path.lineTo(11,-38);path.closeSubpath()
  p.setPen(QColor(edge));p.setBrush(gradient((-25,-24),(28,40),[(0,'#c1d9d5'),(.22,'#689a9f'),(.7,'#2d505e'),(1,'#172e40')]));p.drawPath(path)
  liquid='#c44351' if k==8 else '#327ec4' if k==9 else '#dda637'
  ellipse(p,-25,-1,50,40,gradient((-20,0),(25,40),[(0,liquid),(.5,liquid),(1,'#482f3c' if k==8 else '#183c63' if k==9 else '#896029')]))
  ellipse(p,-25,-2,50,10,liquid,'#dcb981' if k==34 else '#a4cbcc',.7)
  polygon(p,[(-12,-43),(12,-43),(13,-34),(-13,-34)],gradient((-12,-43),(12,-32),[(0,'#dbbb78'),(1,'#775136')]),edge,1)
  line(p,[(-14,-29),(14,-29)],'#c7ae6b',4);ellipse(p,-8,10,16,17,gold)
  if k==8:line(p,[(-4,19),(4,19)],'#e8e2c5',2);line(p,[(0,15),(0,23)],'#e8e2c5',2)
  elif k==9:ellipse(p,-4,14,8,8,'#a2d9ea')
  else:polygon(p,[(-4,16),(4,16),(0,23)],'#ffedb1')
 elif k in (10,21,30):
  color='#7fd3d5' if k==10 else '#80c687' if k==21 else '#f6cf69';shadow='#27697b' if k==10 else '#366d57' if k==21 else '#a66a2c'
  polygon(p,[(-5,-44),(22,-16),(18,25),(0,44),(-27,11),(-22,-24)],gradient((-25,-35),(25,40),[(0,'#ecf5d9' if k!=30 else '#fff1b7'),(.3,color),(1,shadow)]),edge,1.2)
  polygon(p,[(-5,-44),(-2,-7),(0,44),(-27,11),(-22,-24)],color)
  line(p,[(-5,-40),(-2,-7),(17,24)],'#f9efbe',1.2)
  if k in (21,30):
   for n in range(20):
    a=n*math.tau/20
    if k==30 and n in (3,4,5):continue
    line(p,[(35*math.cos(a),35*math.sin(a)),(37*math.cos(a+.12),37*math.sin(a+.12))],gold,2)
 elif k==11:
  for x,y in [(-15,15),(13,-3),(4,20),(-11,-10)]:
   ellipse(p,x-18,y-4,36,14,'#805625',edge,.7);ellipse(p,x-18,y-8,36,12,gold,'#ead18a',.8);ellipse(p,x-13,y-6,26,8,'#d6a348','#aa7630',.8)
   line(p,[(x-3,y-5),(x+3,y-1)],'#ffedb0',1)
 elif k in (12,14):
  path=QPainterPath();path.moveTo(-19,-34);path.cubicTo(17,-45,28,-11,14,17);path.cubicTo(7,31,-7,38,-20,43);path.cubicTo(-6,19,1,-8,-19,-34);path.closeSubpath();p.setBrush(ivory);p.setPen(QColor(edge));p.drawPath(path)
  if k==14:curve(p,(-14,-29),[(5,-19,17,5,-5,28)],'#fae9bc',5)
  line(p,[(-18,-30),(-3,-32),(8,-26)],'#9f8c70',2)
 elif k in (13,15):
  points=[(-34,-31),(-16,-41),(-3,-33),(14,-39),(35,-28),(27,-7),(38,17),(20,38),(0,30),(-26,39),(-38,18),(-27,-2)]
  polygon(p,points,gradient((-30,-35),(32,35),[(0,'#a7b5b2' if k==13 else '#bd8955'),(.5,'#66757b' if k==13 else '#8c5e3c'),(1,'#374953' if k==13 else '#503b2b')]),edge,1)
  rng=random.Random(k)
  for _ in range(950 if k==13 else 100):
   x,y=rng.uniform(-27,27),rng.uniform(-26,27);line(p,[(x,y),(x+rng.uniform(-3,3),y+rng.uniform(3,9))],QColor(220,225,206,rng.randint(65,145)) if k==13 else QColor(44,31,24,55),.6)
  if k==13:
   for x,y in points:
    for j in range(7):line(p,[(x*.9+j*.4,y*.9),(x*1.08+j*.35,y*1.08+3)],'#9ba8a4',.8)
  if k==15:line(p,[(-28,-27),(-19,-8),(-23,12),(0,25),(23,30)],'#cd9a62',1)
 elif k==16:
  polygon(p,[(-24,-35),(24,-35),(20,-24),(17,25),(27,33),(-26,33),(-19,24),(-17,-23)],'#795137',edge,1)
  ellipse(p,-27,-40,54,14,gold);ellipse(p,-29,26,58,14,gold)
  for y in range(-26,27,3):line(p,[(-20,y),(20,y-3)],'#eee5c7',3)
  curve(p,(19,19),[(39,32,19,42,35,44)],'#e8dec4',2)
 elif k in (17,19):
  if k==17:
   ellipse(p,-27,-25,54,66,gradient((-18,-20),(28,36),[(0,'#bad37b'),(.5,'#6c9a50'),(1,'#2c5a44')]),edge,1)
   polygon(p,[(-17,-24),(-8,-37),(8,-37),(16,-24)],'#8c9a63');line(p,[(-15,-30),(15,-30)],'#a87e48',5)
  else:
   path=QPainterPath();path.moveTo(-36,24);path.cubicTo(-41,1,-23,-34,-1,-35);path.cubicTo(23,-37,24,-6,33,9);path.cubicTo(50,33,18,42,-5,39);path.cubicTo(-15,43,-32,39,-36,24);p.setBrush(gradient((-25,-30),(35,40),[(0,'#bbe391'),(.3,'#79b36b'),(1,'#2c6449')]));p.setPen(QColor(edge));p.drawPath(path)
 elif k==18:
  line(p,[(-23,28),(22,-29)],edge,19);line(p,[(-23,28),(22,-29)],ivory,14)
  for x,y in [(-29,25),(-19,33),(17,-34),(27,-26)]:ellipse(p,x-9,y-9,18,18,ivory,edge,.7)
 elif k==20:
  polygon(p,[(-28,-34),(1,-43),(31,-25),(25,31),(-10,41),(-34,23)],gradient((-25,-30),(28,38),[(0,'#be9861'),(.4,'#84613c'),(1,'#403b2f')]),edge,1)
  for x in (-20,-9,3,15):curve(p,(x,-25),[(x+7,-8,x-5,16,x,30)],'#3e352d',3)
  for x,y in [(-18,-19),(13,21),(-20,12)]:ellipse(p,x,y,9,7,'#68825a')
 elif k==22:
  polygon(p,[(-28,-32),(28,-32),(32,5),(0,41),(-32,5)],gold,edge,2);polygon(p,[(-21,-24),(21,-24),(23,2),(0,30),(-23,2)],'#59626a','#e5cb88',1)
  for side in (-1,1):line(p,[(-16*side,15),(14*side,-15)],'#e3d3ac',3)
 elif k==23:
  polygon(p,[(-31,-28),(-2,-42),(31,-19),(25,27),(-4,42),(-34,18)],gradient((-25,-35),(30,35),[(0,'#a0aea7'),(.45,'#647d7b'),(1,'#354f5a')]),edge,1.2)
  line(p,[(-10,25),(-10,-20),(17,-6),(-10,7)],'#63c5dd',5);line(p,[(-10,25),(-10,-20),(17,-6),(-10,7)],'#c1efdf',1.2)
 elif k==27:
  polygon(p,[(-11,-4),(10,-4),(15,36),(-15,36)],ivory,edge,1);ellipse(p,-38,-8,76,24,'#5f3826',edge,1)
  path=QPainterPath();path.moveTo(-40,0);path.cubicTo(-36,-46,26,-54,40,-5);path.cubicTo(26,10,-27,12,-40,0);p.setBrush(gradient((-35,-40),(35,7),[(0,'#d1a065'),(.35,'#a66d3e'),(1,'#633c2b')]));p.setPen(QColor(edge));p.drawPath(path)
  for x in (-23,-12,0,12,24):line(p,[(x,-2),(x*.6,4)],'#c39a69',1)
 elif k==29:
  for n in (-1,0,1):
   dx=n*13;line(p,[(-17+dx,36),(18+dx,-32)],'#332e2c',4);line(p,[(-17+dx,36),(18+dx,-32)],'#af8750',2)
   polygon(p,[(18+dx,-44),(24+dx,-24),(12+dx,-29)],gradient((10+dx,-40),(22+dx,-25),[(0,'#e4e4ce'),(1,'#718a91')]),edge,.7)
   polygon(p,[(-16+dx,22),(-29+dx,27),(-24+dx,37),(-18+dx,35)],'#d9d4b6',edge,.6);line(p,[(-27+dx,28),(-18+dx,30)],'#9c9e8d',.7)
  line(p,[(-8,0),(12,10)],'#70543b',5);line(p,[(-10,4),(9,14)],'#b69864',2)
 elif k==31:
  curve(p,(-25,32),[(-4,33,25,23,23,-3),(23,-20,4,-23,10,-37)],'#4c3b32',17)
  curve(p,(-25,32),[(-4,33,25,23,23,-3),(23,-20,4,-23,10,-37)],'#b48040',12)
  for x,y in [(-17,31),(-4,28),(10,20),(20,8),(21,-8),(13,-20)]:line(p,[(x-5,y-4),(x+5,y+4)],'#e8bb6c',1.2)
  polygon(p,[(10,-25),(5,-45),(21,-35)],'#e7b65c',edge,.8)
 elif k==32:
  ellipse(p,-31,-40,62,80,gradient((-24,-35),(25,35),[(0,'#bfaa68'),(.25,'#83a58f'),(.6,'#4b817d'),(1,'#315b64')]),edge,2)
  curve(p,(0,-36),[(0,-10,0,14,0,37)],'#d8b66d',3)
  for y in (-20,-3,14):curve(p,(-27,y),[(-18,y+6,-6,y+8,0,y+11)],'#bc9c5a',2);curve(p,(27,y),[(18,y+6,6,y+8,0,y+11)],'#bc9c5a',2)
 elif k==33:
  polygon(p,[(0,-44),(29,-19),(33,9),(0,43),(-33,9),(-29,-19)],gold,edge,1.2)
  for y in (-23,-5,13):line(p,[(-22,y),(0,y+17),(22,y)],'#9b7137',2)
  line(p,[(0,-36),(0,30)],'#f1d58a',1.4)

def details(p,k):
 if k in (8,9,34):
  curve(p,(-17,-5),[(-27,6,-24,23,-19,29)],QColor(234,245,224,190),3);curve(p,(19,5),[(24,10,25,14,24,20)],QColor(218,237,214,125),1.5)
  line(p,[(-6,-23),(-6,-16)],'#dce8d6',2);line(p,[(-8,-39),(7,-39)],'#e4ce9b',1)
 elif k in (10,21,30):
  line(p,[(-19,-23),(-20,6),(-7,23)],'#d2ebd4' if k!=30 else '#ffecb4',1)
 elif k in (17,19):
  ellipse(p,-16,-15,8,16,QColor(235,246,190,150));ellipse(p,7,16,4,5,QColor(235,246,190,150))
 elif k==27:curve(p,(-25,-22),[(-19,-30,-8,-34,2,-33)],'#e4c190',2)
 else:
  # Sparse material scratches/highlights, confined well inside the silhouette.
  for j in range(3):line(p,[(-7+j*4,-10+j*5),(-3+j*4,-14+j*5)],QColor(244,220,166,90),.7)

def produce(output,retained):
 output.mkdir(parents=True,exist_ok=False);master=Master(output,'remaining-inventory-items',1536,1024,retained)
 def layer(p,fn):
  for i,k in enumerate(KINDS):p.save();p.translate(i%6*256+128,i//6*256+128);p.scale(2.35,2.35);fn(p,k);p.restore()
 master.layer('Item bodies, cutouts and material gradients',lambda p:layer(p,body))
 master.layer('Glass glints, engraving and material details',lambda p:layer(p,details))
 record=master.save();return {'assets':[record],'kinds':KINDS,'columns':6,'cell':256,'tool':'Krita native 2D QPainter / layered KRA','external_inputs':[],'audio':False}

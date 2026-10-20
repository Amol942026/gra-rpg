"""Original grounded woodland shrub, native layered Krita drawing only."""
from pathlib import Path
import argparse, hashlib, json, math, random, zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed

def create(root,logs):
 root.mkdir(parents=True,exist_ok=False);kra=root/'forest-bush.kra';png=root/'forest-bush.png'
 start_krita_if_needed()
 with KritaMCP(logs) as c:
  doc=c.call('krita_document',action='create',width=360,height=460,name='Emberfall — leszczyna leśna / krzew')['affected_document_id']
  state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
  c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id']);c.call('krita_edit',action='deselect',document_id=doc)
  layers=[]
  def layer(name,opacity=1):
   c.call('krita_layers',action='create',document_id=doc,name=name)
   lid=c.call('krita_state',document_id=doc)['active_layer_id'];layers.append({'id':lid,'name':name})
   if opacity!=1:c.call('krita_layers',action='set_properties',document_id=doc,layer_id=lid,opacity=opacity)
   return lid
  def poly(l,col,pts):c.call('krita_draw',shape='polygon',document_id=doc,layer_id=l,points=[{'x':round(x,2),'y':round(y,2)} for x,y in pts],foreground=col,fill_style='ForegroundColor',stroke_style='None',brush_size=1)
  def path(l,col,pts,size):c.call('krita_draw',shape='path',document_id=doc,layer_id=l,points=[{'x':x,'y':y} for x,y in pts],foreground=col,fill_style='None',stroke_style='ForegroundColor',brush_size=size)
  def oval(l,col,x,y,w,h):c.call('krita_draw',shape='ellipse',document_id=doc,layer_id=l,x=x,y=y,width=w,height=h,foreground=col,fill_style='ForegroundColor',stroke_style='None')
  l=layer('01 Miękki cień kontaktowy',.28);oval(l,'#152e24',122,408,120,18)
  l=layer('02 Ściółka i nasada pędów');poly(l,'#374730',[(124,417),(148,410),(178,407),(206,410),(235,416),(214,422),(174,424),(143,422)])
  for x,y in [(139,418),(155,423),(205,420),(220,416)]:poly(l,'#6a6743',[(x-5,y),(x,y-3),(x+6,y),(x+1,y+2)])
  l=layer('03 Zdrewniałe pędy od ziemi')
  branches=[[(180,418),(168,391),(142,361),(110,331)],[(179,418),(183,383),(184,350),(175,306)],[(181,418),(200,387),(224,356),(243,327)],[(180,418),(147,398),(119,378),(83,369)],[(183,417),(219,400),(247,380),(280,367)]]
  for pts in branches:
   path(l,'#443f2e',pts,6);path(l,'#82704a',[(x-1,y-1) for x,y in pts],2)
  for pts in [[(147,365),(154,340),(145,319)],[(184,360),(205,334),(216,309)],[(224,355),(262,348),(280,333)],[(123,379),(103,350),(82,343)],[(244,382),(266,393),(284,389)]]:path(l,'#5b5837',pts,3)
  l=layer('04 Głęboka zieleń podszytu')
  poly(l,'#294b36',[(61,370),(75,352),(91,356),(91,337),(110,327),(125,330),(137,312),(153,318),(168,294),(188,300),(199,315),(217,303),(235,316),(249,316),(260,336),(281,333),(294,352),(290,369),(307,381),(297,397),(274,398),(262,412),(241,407),(220,420),(199,411),(178,420),(153,412),(130,416),(115,403),(92,408),(86,393),(68,391)])
  rng=random.Random(282041)
  l=layer('05 Liście środkowe i tylne')
  def leaf(l,x,y,length,width,angle,col,shine=False):
   dx,dy=math.cos(angle),math.sin(angle);px,py=-dy,dx
   def p(a,b):return (x+dx*a+px*b,y+dy*a+py*b)
   poly(l,col,[p(0,0),p(length*.16,-width*.37),p(length*.43,-width*.53),p(length*.66,-width*.31),p(length,0),p(length*.66,width*.38),p(length*.38,width*.48),p(length*.13,width*.30)])
   if shine:poly(l,'#7c9655',[p(length*.1,0),p(length*.42,-width*.40),p(length*.68,-width*.23),p(length*.92,0)])
  for row in range(6):
   y=320+row*15;half=43+math.sin((row+1)/7*math.pi)*57
   for col in range(8):
    x=180+(col/7*2-1)*half+rng.uniform(-8,8);yy=y+rng.uniform(-7,7)
    a=(-math.pi*.83 if col<4 else -math.pi*.17)+rng.uniform(-.42,.42)
    leaf(l,x,yy,rng.uniform(23,34),rng.uniform(14,22),a,rng.choice(['#3a6040','#456c43','#507548','#5a7c48']),col%3==0)
  l=layer('06 Przednie pędy i młode liście')
  for pts in [[(171,417),(156,394),(132,380),(106,366)],[(185,416),(209,396),(231,383),(256,368)],[(183,409),(177,379),(164,360)]]:path(l,'#5d6340',pts,2)
  for x,y,a in [(89,382,-2.3),(111,397,-2.0),(137,400,-2.6),(156,411,-2.0),(174,395,-1.9),(198,405,-.8),(222,405,-.9),(244,395,-.5),(266,387,-.7),(285,374,-.3),(193,347,-.7),(144,354,-2.5),(175,323,-1.9)]:leaf(l,x,y,28,18,a,'#5b804a',True)
  l=layer('07 Nerwy i rozproszone refleksy')
  for x,y,a in [(89,382,-2.3),(137,400,-2.6),(174,395,-1.9),(222,405,-.9),(266,387,-.7),(193,347,-.7),(175,323,-1.9)]:path(l,'#a2ac68',[(x+3*math.cos(a),y+3*math.sin(a)),(x+23*math.cos(a),y+23*math.sin(a))],1)
  for x,y in [(136,349),(244,371),(204,332)]:oval(l,'#783e35',x,y,5,5);oval(l,'#af6b45',x,y,2,2)
  c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
  c.call('krita_preview',document_id=doc,max_width=720,max_height=920)
  c.call('krita_state',document_id=doc);c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 from PIL import Image
 with Image.open(png) as im:
  im.load();assert im.size==(360,460) and im.mode=='RGBA';assert im.getchannel('A').getextrema()==(0,255)
  bbox=im.getbbox();assert bbox[1]>=250 and bbox[3]>=416 and bbox[3]<440
 with zipfile.ZipFile(kra) as z:assert z.testzip() is None
 report={'status':'PASS','created_in':'Krita native MCP drawing','original_art':True,'external_assets':[],
  'kra':str(kra),'png':str(png),'canvas':[360,460],'anchor':[180,416],'runtime_scale':.5,
  'layers':layers,'alpha_bounds':bbox,'sha256':hashlib.sha256(png.read_bytes()).hexdigest()}
 (root/'provenance.json').write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8');return report

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--logs',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True)
 a=p.parse_args();print(json.dumps(create(a.root,a.logs),indent=2,ensure_ascii=False))

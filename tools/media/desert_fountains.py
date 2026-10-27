"""Original layered Krita fountains and native Synfig water loops/draining.

Only local, existing tools. RGBA atlas packing changes no authored pixels.
"""
from pathlib import Path
import argparse, hashlib, json, math, shutil, subprocess, xml.etree.ElementTree as ET
from krita_mcp_client import KritaMCP, start_krita_if_needed
from hero_synfig import group, image, value

def author(root):
 root.mkdir(parents=True,exist_ok=False);start_krita_if_needed()
 with KritaMCP(root/'logs') as c:
  for color in ('mana','health'):
   out=root/color;out.mkdir();layers=[]
   doc=c.call('krita_document',action='create',width=256,height=320,name='Emberfall / desert / '+color+' fountain')['affected_document_id']
   state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
   c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id']);c.call('krita_edit',action='deselect',document_id=doc)
   lid=None
   def layer(name):
    nonlocal lid
    c.call('krita_layers',action='create',document_id=doc,name=name);lid=c.call('krita_state',document_id=doc)['active_layer_id'];layers.append((name,lid))
   def poly(col,pts):c.call('krita_draw',shape='polygon',document_id=doc,layer_id=lid,foreground=col,points=[{'x':x*2,'y':y*2} for x,y in pts],fill_style='ForegroundColor',stroke_style='None')
   def line(col,pts,w=1):c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,foreground=col,points=[{'x':x*2,'y':y*2} for x,y in pts],fill_style='None',stroke_style='ForegroundColor',brush_size=w*2)
   def oval(col,x,y,rx,ry):poly(col,[(x+rx*math.cos(i*math.tau/64),y+ry*math.sin(i*math.tau/64)) for i in range(64)])
   dark,mid,bright=('#174d70','#268cb6','#a1e9f3') if color=='mana' else ('#772b3c','#cb485b','#ffd0b5')
   layer('base')
   oval('#554a38',64,138,57,16);oval('#877455',64,134,57,17)
   poly('#b09a70',[(11,129),(27,120),(64,115),(101,120),(117,129),(106,140),(64,149),(22,140)])
   line('#d6bf89',[(13,130),(25,140),(64,147),(105,139),(115,130)],1.5)
   oval('#665b45',64,123,51,19);oval('#c8ae7e',64,111,53,21)
   poly('#a38b63',[(11,112),(17,128),(36,139),(65,144),(94,139),(111,128),(117,112),(102,123),(64,132),(26,123)])
   poly('#bba373',[(11,112),(26,123),(64,132),(64,144),(36,139),(17,128)])
   oval('#e0c597',64,109,53,20);oval('#71624b',64,110,44,14);oval('#92805b',64,114,39,10)
   for x,y in [(22,124),(39,134),(64,138),(88,134),(105,124)]:line('#78684d',[(x,y-4),(x+1,y+4)],.8)
   # Carved pedestal, scalloped capital and inset colored crystal.
   poly('#77654b',[(48,100),(48,50),(43,44),(48,37),(80,37),(85,44),(80,50),(80,100),(91,108),(77,115),(51,115),(38,108)])
   poly('#baa47a',[(48,50),(59,53),(59,104),(49,109),(40,107),(48,100)])
   poly('#a28c62',[(59,53),(72,53),(72,105),(59,110)])
   poly('#d9be8b',[(44,43),(50,38),(78,38),(84,43),(75,48),(53,48)])
   line('#dfc99d',[(51,56),(51,97)],1);line('#65583f',[(76,56),(76,99)],1.5)
   poly('#c4a577',[(64,17),(78,29),(78,42),(64,50),(50,42),(50,29)])
   poly(dark,[(64,21),(74,31),(74,40),(64,46),(54,40),(54,31)])
   poly(mid,[(64,22),(64,43),(55,38),(55,31)]);poly(bright,[(63,23),(65,30),(60,37),(58,32)])
   # Water flows from two small bronze mouths. Runes remain visible when dry.
   for x in (48,80):
    oval('#584f3f',x,65,5,4);oval('#b49862',x,63,5,3);oval('#333c36',x,64,2.6,1.6)
   line(mid,[(63,61),(63,83),(68,78),(62,73),(57,78)],1.8)
   line('#dbbd82',[(57,90),(63,96),(70,90)],1)
   for x,y in [(28,119),(94,123),(52,133),(76,137)]:line('#e0c494',[(x,y),(x+3,y-1)],.6)
   layer('surface');oval(dark,64,113,40,11);oval(mid,61,111,35,8)
   oval(bright,43,110,11,1.2);oval(bright,78,115,14,.7)
   layer('ripples')
   for x,y,rx,ry in [(48,112,12,3),(80,112,10,2.5),(64,117,17,2.1)]:
    line(bright,[(x+rx*math.cos(i*math.pi/24),y+ry*math.sin(i*math.pi/24)) for i in range(49)],.7)
   layer('streams')
   for x in (48,80):
    poly(mid,[(x-1.7,64),(x+1.5,64),(x+2.5,109),(x+5,111),(x,114),(x-4,111),(x-1,108)])
    line(bright,[(x-.3,66),(x+.4,81),(x,96),(x+1,109)],.85)
   layer('droplets')
   for x,y in [(44,102),(52,105),(76,103),(84,99),(40,108),(87,109)]:oval(bright,x,y,.9,1.7)
   c.call('krita_save_export',document_id=doc,kra_path=str(out/'fountain.kra'),png_path=str(out/'full.png'),overwrite=False)
   c.call('krita_preview',document_id=doc,max_width=512,max_height=640)
   # Export every authored layer through Krita (not a substitute renderer).
   for name,uuid in layers:
    for _,other in layers:c.call('krita_layers',action='set_properties',document_id=doc,layer_id=other,visible=uuid==other)
    c.call('krita_save_export',document_id=doc,png_path=str(out/(name+'.png')),overwrite=False)
   for _,uuid in layers:c.call('krita_layers',action='set_properties',document_id=doc,layer_id=uuid,visible=True)
   c.call('krita_save_export',document_id=doc,kra_path=str(out/'fountain.kra'),overwrite=True)
   c.call('krita_state',document_id=doc);c.call('krita_document',action='close',document_id=doc,discard_changes=False)
   (out/'layers.json').write_text(json.dumps(layers,indent=2))
 print('PASS: native Krita fountain masters and cutout layers',flush=True)

def animated(layer,param,kind,values):
 old=layer.find("param[@name='"+param+"']")
 if old is not None:layer.remove(old)
 a=ET.SubElement(ET.SubElement(layer,'param',name=param),'animated',type=kind)
 for i,v in enumerate(values):
  w=ET.SubElement(a,'waypoint',time=f'{i}f',before='linear',after='linear');n=ET.SubElement(w,kind)
  if kind=='vector':ET.SubElement(n,'x').text=str(v[0]);ET.SubElement(n,'y').text=str(v[1])
  else:n.set('value',str(v))

def animate(source,root,repo):
 from PIL import Image
 root.mkdir(parents=True,exist_ok=False);delivery=repo/'assets/sprites';delivery.mkdir(exist_ok=True)
 receipts=[]
 for color in ('mana','health'):
  work=root/color;(work/'parts').mkdir(parents=True)
  for part in ('base','surface','ripples','streams','droplets'):shutil.copy2(source/color/(part+'.png'),work/'parts'/(part+'.png'))
  frames=[]
  for action in ('loop','drain'):
   canvas=ET.Element('canvas',version='1.2',width='128',height='160',xres='2834.645669',yres='2834.645669',**{'view-box':'0 160 128 0','antialias':'2','fps':'24','begin-time':'0f','end-time':'23f','bgcolor':'0 0 0 0'})
   ET.SubElement(canvas,'name').text='Emberfall / '+color+' fountain / '+action
   for part in ('base','surface','ripples','streams','droplets'):
    sub=group(canvas,part);image(sub,part,(0,160,128,0))
    if part!='base':
     parent=next(l for l in canvas.findall('layer') if l.get('desc')==part)
     animated(parent,'amount','real',[(max(0,1-i/23)**(.65 if part=='surface' else 3)) if action=='drain' else 1 for i in range(25)])
     trans=ET.SubElement(sub,'layer',type='translate',active='true',version='0.1',desc='water motion')
     if part=='droplets':samples=[(0,-3*math.sin(i*math.tau/24)) for i in range(25)]
     elif part=='ripples':samples=[(1.7*math.sin(i*math.tau/24),.7*math.sin(i*math.tau/24)) for i in range(25)]
     elif part=='streams':samples=[(.35*math.sin(i*math.tau/12),0) for i in range(25)]
     else:samples=[(0,-.45*math.sin(i*math.tau/24)) for i in range(25)]
     if action=='drain':samples=[(x,y-i/23*5 if part in ('surface','ripples') else y) for i,(x,y) in enumerate(samples)]
     animated(trans,'origin','vector',samples)
   path=work/(action+'.sif');ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
   r=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(work/(action+'-renders')),'-ExpectedFrames','24','-Threads','2','-RequireMotion'],capture_output=True,text=True)
   (work/(action+'.log')).write_text(r.stdout+'\n'+r.stderr);assert r.returncode==0,r.stderr
   paths=sorted((work/(action+'-renders')).rglob('*.png'));assert len(paths)==24
   ims=[Image.open(p).convert('RGBA') for p in paths];assert all(im.size==(128,160) and im.getbbox() for im in ims)
   assert len({hashlib.sha256(im.tobytes()).hexdigest() for im in ims})>12
   frames+=ims
  atlas=Image.new('RGBA',(8*128,6*160));
  for i,im in enumerate(frames):atlas.paste(im,(i%8*128,i//8*160))
  target=delivery/('fountain-'+color+'.png');atlas.save(target)
  # Dry image is the exact same native Synfig base projection, preserving color.
  dry=frames[-1];dry.save(delivery/('fountain-'+color+'-empty.png'))
  frames[0].save(work/'first.png');frames[-1].save(work/'empty.png')
  receipts.append({'color':color,'atlas':str(target),'sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'frames':48,'rgba_bytes':atlas.width*atlas.height*4})
 receipt={'status':'PASS','original_art':'Krita native MCP','animation':'Synfig native loop and drain, 24 Hz','external_assets':[],'graphics_master':str(source),'animation_master':str(root),'atlases':receipts}
 (root/'receipt.json').write_text(json.dumps(receipt,indent=2));print(json.dumps(receipt,indent=2),flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--no-audio',action='store_true',required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--animation',type=Path);p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);a=p.parse_args()
 if not a.source.exists():author(a.source)
 if a.animation:animate(a.source,a.animation,a.repo)

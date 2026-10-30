"""Refine native Krita ground and molten seams; preserve the first masters."""
from pathlib import Path
import math,random,json,hashlib,shutil,subprocess,argparse
from PIL import Image
from krita_mcp_client import KritaMCP

def main(root,runtime,animation):
 with KritaMCP(root/'refine-mcp') as c:
  doc=c.call('krita_document',action='create',width=768,height=96,name='Emberfall / Kaldera / eroded basalt and molten channels')['affected_document_id'];state=c.call('krita_state',document_id=doc);assert state['runtime_compatibility']['status']=='compatible'
  lid=state['active_layer_id'];c.call('krita_edit',action='select_all',document_id=doc);c.call('krita_edit',action='clear',document_id=doc,layer_id=lid);c.call('krita_edit',action='deselect',document_id=doc)
  for tile in range(6):
   c.call('krita_layers',action='create',document_id=doc,name=['Basalt and small fractures','Ash deposits','Worn basalt trail','Bridge masonry','Molten seams and cooled islands','Cooling skin'][tile]);lid=c.call('krita_state',document_id=doc)['active_layer_id'];ox=tile*128;rng=random.Random(791+tile)
   def draw(**kw):
    try:c.call('krita_draw',document_id=doc,layer_id=lid,**kw)
    except RuntimeError as e:
     if 'DRAW_NO_CHANGE' not in str(e):raise
   def poly(color,pts):draw(shape='polygon',foreground=color,fill_style='ForegroundColor',stroke_style='None',points=[{'x':ox+x,'y':y} for x,y in pts])
   def line(color,pts,w):draw(shape='path',foreground=color,fill_style='None',stroke_style='ForegroundColor',brush_size=w,points=[{'x':ox+x,'y':y} for x,y in pts])
   def oval(color,x,y,w,h):draw(shape='ellipse',foreground=color,fill_style='ForegroundColor',stroke_style='None',x=ox+x,y=y,width=w,height=h)
   poly('#81352f' if tile==4 else '#6c5d65' if tile==3 else '#3c3542',[(0,48),(64,0),(128,48),(64,96)])
   c.call('krita_edit',action='select_opaque',document_id=doc,layer_id=lid)
   if tile in (1,2):
    for n in range(8):
     color='#%02x%02x%02x'%(62+n*2,54+n,65+n)
     oval(color,5+n*4,8+n*3,118-n*8,80-n*6)
   if tile==4:
    for n in range(5):
     y=7+n*20;pts=[(-15,y),(15,y-5),(46,y+4),(77,y-2),(107,y+8),(145,y+3)]
     line('#a9492e',pts,19);line('#d56d34',pts,11);line('#f6ad54',[(x,y+1) for x,y in pts],4);line('#ffd780',[(x,y+2) for x,y in pts],1)
    for n in range(9):
     x=rng.uniform(3,123);y=rng.uniform(8,86);w=rng.uniform(9,18);h=rng.uniform(3,8)
     poly('#4b3034',[(x-w,y),(x-w*.4,y-h),(x+w*.8,y-h*.7),(x+w,y+2),(x+w*.2,y+h),(x-w*.6,y+h*.5)])
     line('#be5c3c',[(x-w*.5,y+h*.7),(x+w*.2,y+h+1),(x+w*.8,y+3)],1)
   elif tile==3:
    for n in range(10):line('#413a47',[(n*16-20,10),(n*16+18,48),(n*16-20,86)],1.7)
    for n in range(6):line('#9b8585',[(n*22-5,28),(n*22+13,45)],1.2)
   else:
    for n in range(60):
     x=rng.uniform(0,128);y=rng.uniform(0,96);color=rng.choice(['#47404b','#332e3b','#584b53','#423943'])
     if tile==5:color=rng.choice(['#6a3b3a','#4c3039','#8b4234'])
     oval(color,x,y,rng.uniform(1.5,6),rng.uniform(1,3))
    for n in range(4):
     x=rng.uniform(5,80);y=rng.uniform(6,70);pts=[(x,y),(x+5,y+6),(x+1,y+12),(x+10,y+15),(x+8,y+24)]
     line('#272633' if tile!=5 else '#c36a3d',pts,.8)
     if tile!=5:line('#65505a',[(x+.9,y+.3) for x,y in pts],.45)
   c.call('krita_edit',action='deselect',document_id=doc)
  kra=root/'terrain-refined.kra';png=root/'terrain-refined.png';c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False);c.call('krita_preview',document_id=doc,max_width=1024,max_height=768);c.call('krita_document',action='close',document_id=doc,discard_changes=False)
 shutil.copy2(runtime/'terrain.png',root/'terrain-runtime-before-refine.png');shutil.copy2(png,runtime/'terrain.png')
 image=Image.open(png).convert('RGBA')
 for name,left in [('lava',512),('crust',640)]:
  target=animation/'parts'/(name+'.png');shutil.copy2(target,animation/(name+'-before-refine.png'));image.crop((left,0,left+128,96)).save(target)
 result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(animation/'lava.sif'),'-OutputRoot',str(animation/'refined-renders'),'-ExpectedFrames','12','-Threads','2','-RequireMotion'],capture_output=True,text=True)
 (root/'refine-lava-render.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,result.stderr
 frames=sorted((animation/'refined-renders').rglob('*.png'));assert len(frames)==12
 atlas=Image.new('RGBA',(1536,96))
 for i,p in enumerate(frames):atlas.paste(Image.open(p).convert('RGBA'),(i*128,0))
 shutil.copy2(runtime/'lava-flow.png',root/'lava-flow-before-refine.png');atlas.save(runtime/'lava-flow.png')
 (root/'terrain-refinement.json').write_text(json.dumps({'status':'PASS','authoring':'Krita native MCP, Synfig native surface loop','kra':str(kra),'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'rendered_frames':12},indent=2),encoding='utf8');print('PASS: refined basalt and molten seams, twelve new native Synfig frames')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--animation',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();main(a.root,a.runtime,a.animation)

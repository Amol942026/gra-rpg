"""Author a visibly flowing, seamless lava loop in native Synfig, then render it."""
from pathlib import Path
import argparse,json,shutil,subprocess,hashlib,xml.etree.ElementTree as ET
from PIL import Image,ImageChops,ImageStat
from hero_synfig import group,image
from polish_volcano_motion import translate
from volcano_synfig import author

def flowing_lava(root,source,runtime):
 project=root/'lava';project.mkdir();(project/'parts').mkdir();shutil.copy2(source/'lava-surface.png',project/'parts'/'lava.png')
 canvas=ET.Element('canvas',version='1.2',width='256',height='256',fps='30',**{'view-box':'-1.28 1.28 1.28 -1.28','begin-time':'0f','end-time':'59f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
 ET.SubElement(canvas,'name').text='Emberfall / Caldera / flowing molten river'
 ET.SubElement(canvas,'desc').text='One complete periodic translation over 60 frames at 30 Hz. Virtual frame 60 equals frame 0. Runtime interpolates preloaded samples at the game frame rate.'
 flow=group(canvas,'Continuous current - full tile displacement')
 for x in (-2.56,0,2.56):
  # Two rendered pixels of overscan cover Synfig import edge antialiasing.
  # Neighbouring imports must overlap or their half-alpha edges leave a seam.
  for y in (-2.56,0,2.56):image(flow,'lava',(-1.30+x,1.30+y,1.30+x,-1.30+y))
 translate(flow,[(0,2.56*f/60) for f in range(61)])
 path=project/'lava.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf8',xml_declaration=True)
 result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(project/'renders'),'-ExpectedFrames','60','-Threads','2','-RequireMotion'],capture_output=True,text=True)
 (project/'render.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,(result.stdout,result.stderr)
 frames=sorted((project/'renders').rglob('frame_*.png'));assert len(frames)==60
 images=[Image.open(p).convert('RGBA') for p in frames];assert len({hashlib.sha256(im.tobytes()).digest() for im in images})==60
 # This checks translation, not just that brightness flickers.
 first=images[0].convert('RGB');half=images[30].convert('RGB');expected=ImageChops.offset(first,0,128)
 error=sum(ImageStat.Stat(ImageChops.difference(expected,half)).mean)/3
 static_error=sum(ImageStat.Stat(ImageChops.difference(first,half)).mean)/3
 assert error<3 and static_error>10,(error,static_error)
 for im in images:assert im.size==(256,256) and im.getextrema()[3]==(255,255)
 atlas=Image.new('RGBA',(2048,2048))
 for f,im in enumerate(images):atlas.paste(im,(f%8*256,f//8*256))
 atlas.save(runtime/'lava-flow.png')
 seam=sum(ImageStat.Stat(ImageChops.difference(images[-1].convert('RGB'),first)).mean)/3
 adjacent=sum(ImageStat.Stat(ImageChops.difference(images[28].convert('RGB'),images[29].convert('RGB'))).mean)/3
 assert seam<adjacent*1.8,(seam,adjacent)
 receipt={'status':'PASS','source_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'frames':60,'fps':30,'displacement_pixels':256,'half_period_translation_error':error,'half_period_static_error':static_error,'loop_seam_mean_difference':seam,'adjacent_mean_difference':adjacent,'source':'native Synfig translate waypoints','runtime_interpolation':True}
 (root/'river-receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf8');print(json.dumps(receipt),flush=True)

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--paints',type=Path,required=True);p.add_argument('--lava-source',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args()
 author(a.root,a.paints,a.header,a.runtime,actors_only=True);flowing_lava(a.root,a.lava_source,a.runtime)

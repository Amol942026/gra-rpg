"""Native 60 Hz Synfig rotating vortex, stretched into the gateway aperture."""
from pathlib import Path
import argparse,math,json,shutil,subprocess,hashlib,xml.etree.ElementTree as ET
from PIL import Image,ImageChops,ImageStat
from hero_synfig import group,image,rotate,value

def run(root,paints,runtime):
    root.mkdir(parents=True,exist_ok=False);(root/'parts').mkdir();shutil.copy2(paints/'portal-core.png',root/'parts/core.png')
    c=ET.Element('canvas',version='1.2',width='128',height='208',fps='60',**{'view-box':'-0.55 1.90 0.55 -0.05','begin-time':'0f','end-time':'119f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
    ET.SubElement(c,'name').text='Emberfall / Eldrin gateway / seamless rotating rift'
    g=group(c,'Vortex behind carved stone');image(g,'core',(-.79,1.84,.79,.26));rotate(g,'rift-rotation',(0,1.05),[f*3 for f in range(121)])
    stretch=ET.SubElement(g,'layer',type='stretch',active='true',version='0.1',desc='Elliptical aperture');value(stretch,'amount','vector',(.53,1));value(stretch,'center','vector',(0,1.05))
    amount=c.find('layer/param[@name="amount"]')
    amount.clear();amount.set('name','amount');anim=ET.SubElement(amount,'animated',type='real')
    for f in range(121):ET.SubElement(ET.SubElement(anim,'waypoint',time=f'{f}f',before='linear',after='linear'),'real',value=f'{.94+.055*math.sin(f*math.tau/120):.7f}')
    path=root/'portal.sif';ET.indent(c);ET.ElementTree(c).write(path,encoding='utf8',xml_declaration=True)
    result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path.resolve()),'-OutputRoot',str((root/'renders').resolve()),'-ExpectedFrames','120','-Threads','2','-RequireMotion'],capture_output=True,text=True)
    (root/'render.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,result.stderr
    frames=sorted((root/'renders').rglob('frame_*.png'));assert len(frames)==120
    atlas=Image.new('RGBA',(1980,1696));images=[]
    for f,p in enumerate(frames):
        im=Image.open(p).convert('RGBA');box=im.getbbox();assert im.size==(128,208) and box and box[0]>3 and box[2]<125,(p,box)
        atlas.paste(im,(f%15*132+2,f//15*212+2));images.append(im)
    assert len({hashlib.sha256(im.tobytes()).digest() for im in images})==120
    seam=sum(ImageStat.Stat(ImageChops.difference(images[0],images[-1])).mean);adjacent=sum(ImageStat.Stat(ImageChops.difference(images[30],images[31])).mean);assert seam<adjacent*2,(seam,adjacent)
    atlas.save(runtime/'portal-loop.png');receipt={'status':'PASS','frames':120,'fps':60,'duration':2,'size':[128,208],'cell':[132,212],'columns':15,'atlas':[1980,1696],'native_project_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'source_png_sha256':hashlib.sha256((paints/'portal-core.png').read_bytes()).hexdigest(),'atlas_sha256':hashlib.sha256((runtime/'portal-loop.png').read_bytes()).hexdigest(),'loop_seam':seam,'adjacent_difference':adjacent}
    (root/'receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf8');print(json.dumps(receipt))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--paints',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root,a.paints,a.runtime)

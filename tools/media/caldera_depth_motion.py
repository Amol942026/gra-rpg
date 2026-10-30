"""Native Synfig shoulder-led basalt warrior swing and molten current.

The runtime header is compiled from saved native angle/translation waypoints.
Eight directions share one authored performance and retain the original cutouts.
"""
from pathlib import Path
import argparse, hashlib, json, math, subprocess, xml.etree.ElementTree as ET
from PIL import Image
from hero_synfig import group,image,rotate
from volcano_river_motion import flowing_lava
from create_volcano_art import PARTS,ANGLES


def curve(keys,t):
    for (a,x),(b,y) in zip(keys,keys[1:]):
        if a<=t<=b:
            f=(t-a)/(b-a);f=f*f*(3-2*f);return x+(y-x)*f
    return keys[-1][1]


def translate(parent,label,rows):
    l=ET.SubElement(parent,'layer',type='translate',active='true',version='0.1',desc='runtime:'+label)
    a=ET.SubElement(ET.SubElement(l,'param',name='origin'),'animated',type='vector')
    for f,(x,y) in enumerate(rows):
        v=ET.SubElement(ET.SubElement(a,'waypoint',time=f'{f}f',before='linear',after='linear'),'vector')
        ET.SubElement(v,'x').text=f'{x:.7f}';ET.SubElement(v,'y').text=f'{y:.7f}'


def swing(root,header):
    project=root/'basalt-swing';project.mkdir(exist_ok=False);(project/'parts').mkdir()
    atlas=Image.open('assets/volcano/obsidian-sentinel.png').convert('RGBA');cell=atlas.width//8
    channels={
        'blade':[(0,0),(12,-30),(24,-71),(27,-76),(36,68),(43,55),(53,16),(60,0)],
        'shield':[(0,0),(20,-12),(27,-16),(36,11),(46,8),(60,0)],
        'torso':[(0,0),(23,-5),(27,-6),(36,7),(43,5),(60,0)],
        'lunge':[(0,0),(22,-1.6),(27,-2),(36,5),(43,4),(60,0)]}
    samples={k:[curve(v,f) for f in range(61)] for k,v in channels.items()}
    receipts=[]
    for d,heading in enumerate(ANGLES):
        for p,label in enumerate(PARTS):atlas.crop((d*cell,p*cell,(d+1)*cell,(p+1)*cell)).save(project/'parts'/f'{d}-{label}.png')
        canvas=ET.Element('canvas',version='1.2',width='384',height='384',fps='60',**{'view-box':'-1.92 2.8 1.92 -1.04','begin-time':'0f','end-time':'59f','antialias':'2','bgcolor':'0 0 0 0','xres':'2834.645669','yres':'2834.645669'})
        ET.SubElement(canvas,'name').text=f'Bazaltowy Wojownik / full sword windup / direction {d}'
        ET.SubElement(canvas,'desc').text='Native original Krita cutouts. Shoulder pivot, pelvis counter rotation, shield brace and forward weight transfer. Full windup at normalized action .45; fast follow-through and return. No gameplay timing changes.'
        sign=-1 if math.cos(heading)<-.1 else 1
        order=[0,1,2,4,5,3] if d>=5 else list(range(6))
        for p in order:
            g=group(canvas,PARTS[p]);image(g,f'{d}-{PARTS[p]}',(-1.28,2.3,1.28,-.26))
            if p in (4,5):
                name='blade' if p==5 else 'shield';origin=((18 if p==5 else -18)*.02,.96)
                rotate(g,name,origin,[-sign*v for v in samples[name]])
            if p in (0,3,4,5):rotate(g,'torso',(0,.5),[-sign*v for v in samples['torso']])
            translate(g,'lunge',[(math.cos(heading)*v*.02,-math.sin(heading)*v*.01) for v in samples['lunge']])
        path=project/f'attack-{d}.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf8',xml_declaration=True)
        if d==0:
            # Compile the actual saved native tracks, reverse Synfig's y-up angle.
            saved=ET.parse(path).getroot();tracks={}
            for label in ('blade','shield','torso'):
                layer=next(l for l in saved.iter('layer') if l.get('desc')=='runtime:'+label)
                tracks[label]=[-float(w.find('angle').get('value')) for w in layer.find('param[@name="amount"]/animated')]
            layer=next(l for l in saved.iter('layer') if l.get('desc')=='runtime:lunge')
            tracks['lunge']=[float(w.findtext('vector/x'))/.02 for w in layer.find('param/animated')]
            lines=['#pragma once','namespace basalt_swing {','struct Key {float blade,shield,torso,lunge;};','inline constexpr Key keys[61]={']
            for f in range(61):lines.append('{'+','.join(f'{tracks[k][f]:.7f}f' for k in channels)+'},')
            lines+=['};','inline Key sample(float phase){float f=std::clamp(phase,0.f,1.f)*60;int i=std::min(59,int(f));float t=f-i;auto a=keys[i],b=keys[i+1];return {a.blade+(b.blade-a.blade)*t,a.shield+(b.shield-a.shield)*t,a.torso+(b.torso-a.torso)*t,a.lunge+(b.lunge-a.lunge)*t};}','}']
            header.write_text('\n'.join(lines)+'\n',encoding='utf8')
        result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path.resolve()),'-OutputRoot',str((project/f'renders-{d}').resolve()),'-ExpectedFrames','60','-Threads','2','-RequireMotion'],capture_output=True,text=True)
        (project/f'render-{d}.log').write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,result.stderr
        frames=sorted((project/f'renders-{d}').rglob('frame_*.png'));assert len(frames)==60
        for frame in frames:
            im=Image.open(frame).convert('RGBA');box=im.getbbox();assert box and min(box[:2])>3 and max(box[2:])<381,(frame,box)
        receipts.append({'direction':d,'frames':60,'sif_sha256':hashlib.sha256(path.read_bytes()).hexdigest()});print('Native Synfig swing',d,'PASS',flush=True)
    receipt={'status':'PASS','clips':receipts,'sword_arc_degrees':144,'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest(),'source':'actual saved Synfig angle and translation tracks'}
    (root/'swing-receipt.json').write_text(json.dumps(receipt,indent=2),encoding='utf8')


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--paints',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();a.root.mkdir(exist_ok=False,parents=True);swing(a.root,a.header);flowing_lava(a.root,a.paints,a.runtime)

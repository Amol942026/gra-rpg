"""Author a seamless layered 2D title scene in Synfig; compile saved tracks."""
import argparse
import hashlib
import json
import math
import shutil
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path
from hero_synfig import value, group, image

FPS=20
FRAMES=240
CHANNELS=('camera','foreground','mist','glow')


def author(root,art,header,render):
    root.mkdir(parents=True,exist_ok=False);(root/'parts').mkdir()
    for name in ('background','foreground','mist','glow'):shutil.copy2(art/(name+'.png'),root/'parts'/(name+'.png'))
    canvas=ET.Element('canvas',version='1.2',width='320',height='200',xres='2834.645669',yres='2834.645669',**{'view-box':'-7.2 4.5 7.2 -4.5','antialias':'2','fps':str(FPS),'begin-time':'0f','end-time':f'{FRAMES-1}f','bgcolor':'0 0 0 1'})
    ET.SubElement(canvas,'name').text='Emberfall / Brama Świtu / 12 second title loop'
    ET.SubElement(canvas,'desc').text='Layered original Krita art. Native Synfig parallax, drifting mist and breathing portal light. Runtime interpolates these exact tracks at presentation rate.'
    tracks={key:[] for key in CHANNELS}
    for n in range(FRAMES+1):
        a=n/FRAMES*math.tau
        tracks['camera'].append((math.sin(a)*2.5,math.cos(a)*1.5))
        tracks['foreground'].append((-math.sin(a)*3.5,-math.cos(a)*1.5))
        tracks['mist'].append((math.sin(a)*18,math.cos(a)*3))
        tracks['glow'].append((.62+.25*math.sin(2*a),1+.012*math.cos(2*a)))
    for key,asset in [('camera','background'),('glow','glow'),('mist','mist'),('foreground','foreground')]:
        g=group(canvas,'Krita / '+asset)
        image(g,asset,(-7.4,4.625,7.4,-4.625))
        if key=='glow':
            l=g.find('layer');l.set('desc','runtime:glow');l.remove(l.find("param[@name='amount']"))
            a=ET.SubElement(ET.SubElement(l,'param',name='amount'),'animated',type='real')
            for n,(alpha,scale) in enumerate(tracks[key]):
                w=ET.SubElement(a,'waypoint',time=f'{n}f',before='linear',after='linear');ET.SubElement(w,'real',value=f'{alpha:.7f}')
            # Runtime also reuses a second scalar for subtle light breathing.
            l=ET.SubElement(g,'layer',type='zoom',active='true',version='0.1',desc='runtime:glowScale');value(l,'center','vector',(0,0))
            a=ET.SubElement(ET.SubElement(l,'param',name='amount'),'animated',type='real')
            for n,(_,scale) in enumerate(tracks[key]):
                w=ET.SubElement(a,'waypoint',time=f'{n}f',before='linear',after='linear');ET.SubElement(w,'real',value=f'{math.log(scale):.7f}')
        else:
            l=ET.SubElement(g,'layer',type='translate',active='true',version='0.1',desc='runtime:'+key)
            a=ET.SubElement(ET.SubElement(l,'param',name='origin'),'animated',type='vector')
            for n,(x,y) in enumerate(tracks[key]):
                v=ET.SubElement(ET.SubElement(a,'waypoint',time=f'{n}f',before='linear',after='linear'),'vector');ET.SubElement(v,'x').text=f'{x/100:.7f}';ET.SubElement(v,'y').text=f'{-y/100:.7f}'
    path=root/'forest-menu.sif';ET.indent(canvas);ET.ElementTree(canvas).write(path,encoding='utf-8',xml_declaration=True)
    # Read actual saved SIF; source data and runtime data cannot drift apart.
    records={}
    for l in ET.parse(path).getroot().iter('layer'):
        desc=l.get('desc','')
        if not desc.startswith('runtime:'):continue
        key=desc[8:]
        if key in ('glow','glowScale'):records[key]=[float(w.find('real').get('value')) for w in l.find("param[@name='amount']/animated")]
        else:records[key]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in l.find("param[@name='origin']/animated")]
    rows=[]
    for n in range(FRAMES+1):
        row=[]
        for key in ('camera','foreground','mist'):
            x,y=records[key][n];row.extend((x*100,-y*100))
        row.extend((records['glow'][n],math.exp(records['glowScale'][n])));rows.append(row)
    assert all(abs(a-b)<1.e-6 for a,b in zip(rows[0],rows[-1]))
    lines=['#pragma once','#include <algorithm>','#include <cmath>','namespace menu_motion {','struct Key {float cameraX,cameraY,foregroundX,foregroundY,mistX,mistY,glow,scale;};','inline constexpr float Duration=12.f;','inline constexpr Key Keys[] = {']
    lines+=['{'+','.join(f'{x:.7f}f' for x in row)+'},' for row in rows]
    lines+=['};','inline Key sample(float time){float p=std::fmod(std::max(0.f,time),Duration)*20;int i=std::min(239,int(p));float t=p-i;auto a=Keys[i],b=Keys[i+1];return {a.cameraX+(b.cameraX-a.cameraX)*t,a.cameraY+(b.cameraY-a.cameraY)*t,a.foregroundX+(b.foregroundX-a.foregroundX)*t,a.foregroundY+(b.foregroundY-a.foregroundY)*t,a.mistX+(b.mistX-a.mistX)*t,a.mistY+(b.mistY-a.mistY)*t,a.glow+(b.glow-a.glow)*t,a.scale+(b.scale-a.scale)*t};}','}']
    header.parent.mkdir(parents=True,exist_ok=True);header.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    if render:
        job=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\animations\Render-Synfig.ps1','-ProjectPath',str(path),'-OutputRoot',str(root/'renders'),'-ExpectedFrames',str(FRAMES),'-Threads','2','-RequireMotion'],capture_output=True,text=True)
        (root/'render.log').write_text(job.stdout+'\n'+job.stderr,encoding='utf-8');assert job.returncode==0,job.stderr
    result={'status':'PASS','source':'Synfig 1.4.5 native SIF waypoints','project':str(path),'frames':FRAMES,'fps':FPS,'duration':12,'runtime_samples':len(rows),'project_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'header_sha256':hashlib.sha256(header.read_bytes()).hexdigest()}
    (root/'motion-receipt.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result))


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--art',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--render',action='store_true');p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();author(a.root,a.art,a.header,a.render)

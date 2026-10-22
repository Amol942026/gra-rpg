"""Independent source/PNG checks for the Krita/Synfig title and moving treant."""
import argparse
import hashlib
import json
import math
import re
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path
import numpy as np
from PIL import Image

REPO=Path(__file__).resolve().parents[2]


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--qa',type=Path,required=True)
    parser.add_argument('--native',type=Path,default=Path('C:/AI2D/animations/emberfall_menu_20261022_v2'))
    parser.add_argument('--previews',action='store_true')
    parser.add_argument('--no-audio',action='store_true',required=True)
    args=parser.parse_args();checks=[]
    def check(ok,label):
        if not ok:raise ValueError(label)
        checks.append(label)
    def read(path,size):
        with Image.open(path) as image:
            image.load();check(image.size==size,str(path.name)+' decoded with correct dimensions')
            return np.array(image.convert('RGBA'))
    with zipfile.ZipFile(REPO/'art/menu/forest/forest-gate.kra') as archive:
        check(archive.testzip() is None and 'maindoc.xml' in archive.namelist(),'layered Krita master is valid')
        xml=archive.read('maindoc.xml');check(xml.count(b'nodetype="paintlayer"')>=8,'Krita source retains separate painted layers')
    memory=0
    for name in ('background','foreground','mist','glow'):
        runtime=REPO/'assets/menu'/(name+'.png');part=REPO/'art/menu/motion/parts'/(name+'.png')
        check(runtime.read_bytes()==part.read_bytes(),'runtime asset matches Synfig source: '+name)
        pixels=read(runtime,(1440,900));memory+=pixels.size
        check(int(pixels[:,:,3].max())>0,'nonempty Krita layer: '+name)
        check(int(pixels[:,:,3].min())==(255 if name=='background' else 0),'correct alpha: '+name)
    check(memory==20736000,'four resident textures use 19.8 MiB RGBA8')
    project=REPO/'art/menu/motion/forest-menu.sif'
    receipt=json.loads((REPO/'art/menu/motion/motion-receipt.json').read_text(encoding='utf-8'))
    check(hashlib.sha256(project.read_bytes()).hexdigest()==receipt['project_sha256'],'Synfig master matches production receipt')
    check(hashlib.sha256((REPO/'src/generated/menu_motion_data.hpp').read_bytes()).hexdigest()==receipt['header_sha256'],'compiled tracks match production receipt')
    check(project.read_bytes()==(args.native/'forest-menu.sif').read_bytes(),'native rendered project equals packaged source master')
    source=ET.parse(project).getroot();tracks={}
    for layer in source.iter('layer'):
        label=layer.get('desc','')
        if not label.startswith('runtime:'):continue
        key=label[8:]
        if key in ('glow','glowScale'):
            tracks[key]=[float(w.find('real').get('value')) for w in layer.find("param[@name='amount']/animated")]
        else:tracks[key]=[(float(w.findtext('vector/x')),float(w.findtext('vector/y'))) for w in layer.find("param[@name='origin']/animated")]
    text=(REPO/'src/generated/menu_motion_data.hpp').read_text(encoding='utf-8')
    rows=[list(map(float,re.findall(r'(-?\d+\.\d+)f',line))) for line in text.splitlines() if line.startswith('{')]
    check(len(rows)==241 and set(tracks)=={'camera','foreground','mist','glow','glowScale'},'complete saved Synfig runtime tracks')
    for n,row in enumerate(rows):
        expected=[]
        for key in ('camera','foreground','mist'):
            x,y=tracks[key][n];expected.extend((x*100,-y*100))
        expected.extend((tracks['glow'][n],math.exp(tracks['glowScale'][n])))
        check(len(row)==8 and max(abs(x-y) for x,y in zip(expected,row))<1.e-6,'Synfig to runtime exact sample '+str(n))
    check(max(abs(x-y) for x,y in zip(rows[0],rows[-1]))<1.e-6,'native title loop closes')
    native=list(args.native.glob('renders/*/frame_*.png'))
    check(len(native)==240,'complete native Synfig render (240 frames)')
    native_hashes=[]
    for path in sorted(native):native_hashes.append(hashlib.sha256(read(path,(320,200)).tobytes()).hexdigest())
    check(len(set(native_hashes))==240,'every native frame contains motion')
    frames=sorted((args.qa/'animation').glob('frame-*.png'))
    check(len(frames)==60,'complete actual game title animation')
    hashes=[];first=None;label_mask=None;previews=[]
    for path in frames:
        pixels=read(path,(1152,720));check(np.all(pixels[:,:,3]==255),'opaque animated menu: '+path.name)
        hashes.append(hashlib.sha256(pixels.tobytes()).hexdigest())
        if first is None:first=pixels
        # The menu's text/targets remain stationary over the moving painting.
        roi=pixels[258:533,806:1038,:3];mask=(roi[:,:,0]>170)&(roi[:,:,1]>150)&(roi[:,:,2]>110)
        if label_mask is None:label_mask=mask
        check(np.mean(mask!=label_mask)<.002,'static readable button labels: '+path.name)
        if args.previews:previews.append(Image.fromarray(pixels).convert('RGB').resize((720,450)))
    check(len(set(hashes))==60,'actual title background visibly moves in every sampled frame')
    end=read(args.qa/'animation/loop-end.png',(1152,720))
    loop_pixels=int(np.sum(np.max(np.abs(first.astype(np.int16)-end.astype(np.int16)),axis=2)>3))
    check(loop_pixels<200,'actual complete title loop closes without visible jump')
    treants=sorted((args.qa/'treant').glob('frame-*.png'));check(len(treants)==120,'complete actual treant chase (120 frames)')
    centers=[];actor_frames=[]
    for n,path in enumerate(treants):
        pixels=read(path,(900,600));region=pixels[:435,:,:3];mask=(region[:,:,0]>50)&(region[:,:,1]>40)
        ys,xs=np.where(mask);check(len(xs)>2000,'treant body is rendered: '+path.name);centers.append(float(np.median(xs)))
        check(xs.min()>2 and xs.max()<897 and ys.min()>2,'moving treant not clipped: '+path.name)
        if args.previews and n%6==0:actor_frames.append(Image.fromarray(pixels).convert('RGB'))
    check(300<centers[-1]-centers[0]<400,'rendered treant travels at real world speed')
    if args.previews:
        previews[0].save(args.qa/'menu-motion.gif',save_all=True,append_images=previews[1:],duration=200,loop=0,optimize=False)
        actor_frames[0].save(args.qa/'treant-motion.gif',save_all=True,append_images=actor_frames[1:],duration=100,loop=0,optimize=False)
    result={'status':'PASS','checks':len(checks),'native_frames':240,'runtime_menu_frames':60,'runtime_treant_frames':120,'resident_rgba_bytes':memory,'loop_changed_pixels':loop_pixels,'audio_device_opened':False}
    (args.qa/'forest-menu-verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(result))


if __name__=='__main__':main()

"""Native Krita creature/material revision. Existing original KRA anatomy is
refined on separate layers; selection masks preserve the six-part cutout rig.
Pillow is used only for resizing, part export and technical alpha masks.
"""
from pathlib import Path
import argparse,json,math,random,shutil,hashlib
from PIL import Image
from polish_volcano_art import Paint,PARTS,NAMES,ANGLES
from volcano_river_detail import retain_finish
from krita_mcp_client import KritaMCP,start_krita_if_needed

def actor_detail(p,kind,part,d):
    a=ANGLES[d];fx,fy=math.cos(a),math.sin(a);front=fy>=-.1;quad=kind in (0,1,5);boss=kind==6;r=random.Random(8047+kind*311+part*41+d)
    if part==3:
        p.soft(['#b8ac8f','#de8749','#e7a16b','#9da4b0','#a9b5bb','#c29166','#a5a598'][kind],53,69,53,.34)
        p.soft(['#292b37','#57312e','#49313a','#1a2539','#1e2c3b','#45323a','#242d37'][kind],79,91,49,.48)
        if quad:
            w=48+abs(fx)*26;cy=84;hx=64+fx*(24 if kind==0 else 30);hy=86+fy*10
            for n in range(15):
                x=64+r.uniform(-.4,.4)*w;y=cy+r.uniform(-11,12)
                p.line('#28323b',[(x-2,y),(x,y+2),(x+2.6,y+.5)],.65)
                p.line('#b7a58b' if kind==0 else '#d69b65',[(x-1.7,y-.8),(x+.7,y+.2)],.45)
            if kind==0:
                for n in range(4):
                    x=64+(n-1.5)*(8+abs(fx)*2);y=77+abs(n-1.5)*2
                    q=[(x-2,y-6),(x+2,y-1),(x-1,y+5)];p.line('#713c32',q,2);p.line('#ef9950',q,.55)
            else:
                for n in range(7):
                    x=64+(n-3)*6;y=83+(n%2)*6
                    p.oval('#723d30',x-2.4,y-1.7,4.8,3.5);p.oval('#f0a04c',x-1.5,y-.8,2.5,1.8)
            if front or abs(fx)>.8:
                ex=hx+fx*5;p.line('#27313b',[(ex-7,hy-4),(ex-3,hy-5.5),(ex+.5,hy-3)],1.8)
                p.line('#ffc776',[(ex-5.6,hy-4.4),(ex-2.4,hy-4.8)],.95);p.oval('#fff0b6',ex-4.4,hy-4.8,.7,.7)
                p.line('#4b3733',[(hx-5,hy+5),(hx+3,hy+6),(hx+9,hy+3)],.75)
                if kind!=0:
                    for n in range(3):p.line('#dfd1ac',[(hx+n*3,hy+4),(hx+n*3-.4,hy+6.7)],.65)
        elif kind==2:
            hx=64+fx*3;hy=51
            if front:
                p.line('#724238',[(hx-7,hy-2),(hx-3,hy-3),(hx-1,hy-1)],.8);p.line('#724238',[(hx+1,hy-1),(hx+4,hy-3),(hx+7,hy-2)],.8)
                for side in (-1,1):p.oval('#f4c077',hx+side*4-1,hy+.6,2,1.4);p.oval('#fff0ba',hx+side*4-.3,hy+.8,.55,.55)
                p.line('#e9b98b',[(hx,hy+2),(hx+1.5,hy+5)],.9);p.line('#5d3430',[(hx-4,hy+8),(hx,hy+9.5),(hx+4,hy+7.5)],.8)
            for n in range(8):
                y=69+n*2;p.line('#b47b53' if n%2 else '#765343',[(56,y),(60,y+1),(68,y-1)],.55)
        elif kind==3:
            for n in range(10):
                x=48+n*3.1;p.line('#a1a8b0' if n%3 else '#293446',[(x,72),(x-2,89),(x+2,105+n%3*3)],.6)
            if front:
                hx=64+fx*4
                for side in (-1,1):p.line('#b7d1ca',[(hx+side*2.5,49),(hx+side*5,48.6)],.8)
            p.line('#baa37a',[(60,70),(64,77),(69,70)],.65)
        else:
            for n in range(15):
                x=r.uniform(45 if boss else 52,82 if boss else 76);y=r.uniform(58,92)
                p.line('#263240',[(x,y),(x+2,y+1),(x+1,y+3)],.6);p.line('#b9b29a',[(x-.5,y-1),(x+1.5,y-.5)],.45)
            for side in (-1,1):
                x=64+side*(16 if boss else 10);q=[(x,65),(x-side*3,73),(x+side,81),(x-side*2,88)]
                p.line('#713e35',q,2);p.line('#dc8748' if boss else '#b98657',q,.7)
            hx=64+fx*3;hy=43 if boss else 51
            p.line('#b4b9ac',[(hx-9,hy-6),(hx-3,hy-11),(hx+5,hy-9)],.75)
            if front:
                for side in (-1,1):p.line('#ffca7b',[(hx+side*2,hy+2),(hx+side*6,hy+1)],1);p.oval('#fff0ba',hx+side*4-.3,hy+1,.6,.6)
                p.line('#38434a',[(hx-4,hy+10),(hx,hy+12),(hx+5,hy+9)],.85)
            if boss:
                p.oval('#b7582e',59,69,10,14);p.oval('#ef9f4a',61,71,6,10);p.oval('#ffe1a0',62,72,3,6)
    else:
        side=-1 if part in (1,4) else 1
        if quad:
            if part==0:
                p.line('#9a967e',[(64-fx*21,88-fy*5),(64-fx*34,90-fy*8),(64-fx*45,96-fy*9)],.6)
                if kind==5:
                    for side in (-1,1):
                        for n in range(4):p.line('#ba8b68',[(64+side*14,74),(64+side*(26+n*3),54+n*5),(64+side*(35+n*2),59+n*5)],.55)
            else:
                fore=part>=4;cx=64+fx*(22 if fore else -18)+side*(6 if abs(fx)>.65 else 16);cy=99+fy*(8 if fore else -8)
                p.line('#c0ab83',[(cx-2,cy-11),(cx+1,cy-4),(cx,cy+3)],.85)
                for n in range(3):p.line('#ddd0aa',[(cx-3+n*3,cy+8),(cx-2+n*3+fx*2,cy+11)],.65)
                for n in range(3):p.line('#34363b',[(cx-3,cy-9+n*4),(cx+2,cy-8+n*4)],.6)
        elif part==0:
            for n in range(7):p.line('#929491' if n%2 else '#24303e',[(48+n*5,77),(46+n*5,94),(49+n*5,106)],.6)
        else:
            cx=64+side*((27 if boss else 18) if part>=4 else (13 if boss else 9));cy=83 if part>=4 else 100
            p.line('#bbc2b2' if kind in (4,6) else '#c39b77',[(cx-3,cy-12),(cx-4,cy-4),(cx-2,cy+5)],.7)
            for n in range(4):p.line('#2c3740',[(cx-4,cy-6+n*4),(cx+3,cy-4+n*4)],.55)
            if kind in (4,6):
                p.line('#be8254',[(cx+3,cy-10),(cx+1,cy-1),(cx+4,cy+8)],.6)
                for n in range(3):p.line('#afbaaf',[(cx-3+n*3,cy+15),(cx-2+n*3,cy+19)],.55)
            if kind==4 and part==5:
                p.line('#ebdab0',[(cx+6,95),(cx+13,64),(cx+15,57)],.9)
                p.line('#e3b36b',[(cx-5,99),(cx+9,102)],.8)
            if kind==4 and part==4:
                p.line('#c2ae7d',[(cx-5,84),(cx,81),(cx+5,85),(cx+4,99),(cx,103)],.8)
                for x in (-5,4):p.oval('#d9c29a',cx+x,87,1.1,1.1)


def open_copy(c,source,root,name,width,height):
    private=root/'input'/(name+'.kra');shutil.copy2(source,private)
    doc=c.call('krita_document',action='open',path=str(private))['affected_document_id'];state=c.call('krita_state',document_id=doc)
    assert state['runtime_compatibility']['status']=='compatible'
    p=Paint.__new__(Paint);p.c=c;p.doc=doc;p.width=width;p.height=height;p.lid=state['active_layer_id'];p.layers=[l['name'] for l in state['layers']];p.s=1;p.ox=p.oy=0
    return p,{l['name']:l['id'] for l in state['layers']}

def scenery_detail(p,kind,v,extra):
    r=random.Random(407+kind*73+v*5+extra*144)
    p.soft('#c39976',73,146,100,.23);p.soft('#192c3a',111,185,92,.4)
    if (not extra and kind in (0,3)) or (extra and kind in (0,3)):
        for n in range(24):
            x=r.uniform(46,137);y=r.uniform(80 if not extra or kind==3 else 174,211)
            p.line('#23313e',[(x,y),(x+2,y+4),(x-1,y+7)],.65);p.line('#afa99a',[(x-1,y-1),(x+2,y)],.45)
        for n in range(4):
            x=58+n*22;y=180+r.random()*18;p.line('#c4784d',[(x,y-14),(x+2,y-7),(x-1,y)],.75)
    elif kind==1:
        for n in range(14):
            x=r.uniform(73,106);y=r.uniform(82 if not extra else 166,205)
            p.line('#293039',[(x,y),(x-2,y+9),(x+1,y+16)],.65);p.line('#b19c7c',[(x-1,y),(x-2,y+7)],.45)
        p.line('#d8b27a',[(79,201),(91,196),(104,199)],.7)
    else:
        for n in range(25):
            x=r.uniform(53,130);y=r.uniform(184,211);p.oval('#ddc88a' if n%3 else '#5b695d',x,y,r.uniform(.6,1.7),r.uniform(.5,1.2))
        p.line('#ddbe76',[(62,191),(79,181),(102,183),(117,190)],1.1)

def run(root):
    root.mkdir(parents=True,exist_ok=True);(root/'input').mkdir(exist_ok=True);(root/'albedo').mkdir(exist_ok=True);start_krita_if_needed()
    jobs=[(n,Path('C:/AI2D/sprites/emberfall-volcano-river-detail-20261029')/(n+'.kra'),2048,1536) for n in NAMES]
    jobs += [('scenery',Path('art/volcano/depth/masters/scenery.kra').resolve(),1440,1840),('scenery-extra',Path('C:/AI2D/sprites/emberfall-volcano-river-detail-20261029/scenery-extra.kra'),1440,1840),('ground',Path('art/volcano/depth/masters/ground.kra').resolve(),512,512),('channel-wall',Path('art/volcano/depth/masters/channel-wall.kra').resolve(),512,256),('macro',Path('art/volcano/depth/masters/macro.kra').resolve(),1024,1024)]
    for name,source,w,h in jobs:
        receipt=root/(name+'-receipt.json')
        if receipt.exists():
            saved=json.loads(receipt.read_text());assert hashlib.sha256((root/(name+'.kra')).read_bytes()).hexdigest()==saved['kra_sha256'];assert hashlib.sha256((root/'albedo'/(name+'.png')).read_bytes()).hexdigest()==saved['albedo_sha256'];print('Reused verified',name,flush=True);continue
        assert not (root/(name+'.kra')).exists(),'Inspect completed native save before replay'
        with KritaMCP(root/('logs-'+name),timeout=240) as c:
            p,layers=open_copy(c,source,root,name,w,h)
            if name in NAMES:
                for part,label in enumerate(PARTS):
                    c.call('krita_edit',action='select_opaque',document_id=p.doc,layer_id=layers[label]);p.layer('Caldera material / '+label)
                    for d in range(8):p.s=2;p.ox=d*256;p.oy=part*256;actor_detail(p,NAMES.index(name),part,d)
                    p.deselect()
            elif name.startswith('scenery'):
                labels=['Fractured basalt outcrops','Charred split trees','Sulphur vents','Obsidian glass'] if name=='scenery' else ['Cracked volcanic boulders','Burnt stumps and ash grass','Sulphur craters','Runic basalt monoliths']
                for kind,label in enumerate(labels):
                    c.call('krita_edit',action='select_opaque',document_id=p.doc,layer_id=layers[label]);p.layer('Mineral weathering / '+label)
                    for v in range(4):p.s=2;p.ox=kind*360;p.oy=v*460;scenery_detail(p,kind,v,name.endswith('extra'))
                    p.deselect()
            elif name=='macro':
                p.layer('Broad iron rich ash and sulphur fields',.5);r=random.Random(5427)
                for n in range(15):
                    x=r.uniform(30,950);y=r.uniform(40,950);p.soft(r.choice(['#a87f59','#b8a176','#243544','#6f463e']),x,y,r.uniform(110,270),.33)
                p.layer('Ancient overlapping cooling flow ridges',.32)
                for n in range(10):
                    x=r.uniform(30,800);y=r.uniform(40,850);w=r.uniform(90,180);h=r.uniform(25,60)
                    q=[(x-w*.2,y+h),(x,y-h*.1),(x+w*.55,y-h*.3),(x+w,y),(x+w*.7,y+h*.6),(x+w*.2,y+h*1.2)]
                    p.poly('#343844' if n%2 else '#8c6852',q,True)
                    for j in range(3):p.line('#ba9771',[(x+6,y+j*5),(x+w*.32,y-10+j*5),(x+w*.7,y-5+j*5)],1.2)
            else:
                p.layer('Porous scoria inclusions and mineral edges');r=random.Random(611+len(name))
                for n in range(45):
                    x=r.randrange(512);y=r.randrange(h);s=r.uniform(1.5,5)
                    p.poly('#33343e',[(x-s,y),(x-s*.3,y-s),(x+s,y-s*.4),(x+s*.5,y+s*.5),(x-s*.6,y+s*.6)])
                    p.line('#9c8470',[(x-s,y-.5),(x-s*.3,y-s),(x+s,y-s*.4)],.55)
            saved=retain_finish(p,root,name)
        im=Image.open(root/(name+'.png')).convert('RGBA');size=256 if name=='varkhor' else 192
        if name in NAMES:im=im.resize((size*8,size*6),Image.Resampling.LANCZOS)
        im.save(root/'albedo'/(name+'.png'))
        saved.update({'source_kra':str(source),'source_kra_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'albedo_sha256':hashlib.sha256((root/'albedo'/(name+'.png')).read_bytes()).hexdigest(),'albedo_size':list(im.size)})
        receipt.write_text(json.dumps(saved,indent=2),encoding='utf8');print('Saved native Krita',name,flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root)

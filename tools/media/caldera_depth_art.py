"""Original Caldera canyon art, drawn by native Krita MCP. No external inputs.

Pillow only validates/exports alpha masks. Saved documents remain retained:
closing edited KRA documents is unsafe in the installed Krita build.
"""
from pathlib import Path
import argparse, json, math, random, shutil
from PIL import Image, ImageFilter, ImageChops
from polish_volcano_art import Paint
from volcano_river_detail import retain_finish
from krita_mcp_client import KritaMCP, start_krita_if_needed


def ground(p):
    r=random.Random(30510)
    p.layer('Basalt bed');p.poly('#34343b',[(0,0),(512,0),(512,512),(0,512)])
    p.layer('Overlapping cooled lava plates')
    for row in range(-1,8):
        for col in range(-1,7):
            x=col*96+(row%2)*48;y=row*81
            w=r.uniform(62,104);h=r.uniform(48,77)
            pts=[(x-w*.55,y),(x-w*.37,y-h*.48),(x+w*.17,y-h*.56),(x+w*.52,y-h*.14),(x+w*.44,y+h*.31),(x-w*.09,y+h*.45)]
            color=r.choice(['#424047','#48434a','#3e3d44','#51494b','#3b3d45'])
            for dx,dy in [(0,0),(-512,0),(512,0),(0,-512),(0,512)]:
                if not (-w<x+dx<512+w and -h<y+dy<512+h):continue
                q=[(a+dx,b+dy) for a,b in pts];p.poly('#242932',q)
                p.poly(color,[(x+dx+(a-x)*.96,y+dy+(b-y)*.94-2) for a,b in pts])
                p.line('#6c6260',[(q[0][0]+4,q[0][1]-2),(q[1][0]+3,q[1][1]+3),(q[2][0]-5,q[2][1]+4)],1.2)
                p.line('#2a2d35',[(x+dx-7,y+dy-18),(x+dx+2,y+dy-5),(x+dx-3,y+dy+9),(x+dx+12,y+dy+20)],1.5)
    p.layer('Ash, red mineral weathering and granular inclusions')
    for n in range(30):
        x=r.randrange(512);y=r.randrange(512);p.soft(r.choice(['#a5876e','#1d2530','#886352']),x,y,r.uniform(40,120),.12)
    for n in range(150):
        x=r.randrange(512);y=r.randrange(512);p.oval(r.choice(['#8b7766','#282d34','#6c6664']),x,y,r.uniform(.6,2.7),r.uniform(.5,1.5))


def macro(p):
    r=random.Random(981)
    p.layer('Broad ash drifts and iron oxide fields')
    for n in range(30):p.soft(r.choice(['#bc9c79','#8c5140','#202b39']),r.randrange(1024),r.randrange(1024),r.uniform(80,230),.19)
    p.layer('Ancient cooling flow ridges')
    for n in range(18):
        x=r.randrange(1024);y=r.randrange(1024)
        for j in range(3):p.line('#766255',[(x-30,y+j*4),(x,y-12+j*4),(x+22,y+2+j*4),(x+55,y-5+j*4)],1.6)


def wall(p):
    r=random.Random(571)
    p.layer('Vertical basalt strata');p.poly('#302a32',[(0,0),(512,0),(512,256),(0,256)])
    for col in range(12):
        x=col*47-20;w=r.uniform(36,58)
        p.poly(r.choice(['#49414a','#403c46','#53454a']),[(x,0),(x+w,0),(x+w-8,256),(x+3,256)])
        p.poly('#242630',[(x+w-9,0),(x+w,0),(x+w-8,256),(x+w-16,256)])
        p.line('#726168',[(x+3,0),(x+1,85),(x+7,151),(x+3,227)],2)
        for y in (45,104,178):p.line('#22242c',[(x,y),(x+w*.45,y+7),(x+w,y-2)],3)
    p.layer('Reflected molten light at the foot of the cliff')
    for x in range(0,550,60):p.soft('#e15b2e',x,253,130,.4)
    for n in range(22):
        x=r.randrange(512);y=r.randrange(150,245);p.line('#b3654d',[(x,y),(x+3,y+12),(x-1,y+23)],1)
    p.layer('Broken dark upper rim');p.line('#171e29',[(0,3),(512,3)],13)


def lava(p):
    r=random.Random(1097)
    p.layer('Molten current');p.poly('#d74b20',[(0,0),(256,0),(256,256),(0,256)])
    p.layer('Broad incandescent ribbons and rotating folds')
    for n in range(8):
        x=n*34
        # Periodic wave in both axes, with overlapping copies across the edges.
        pts=[(x+math.sin(j*math.tau/16+n*.9)*13,j*16) for j in range(-2,19)]
        for dx in (-256,0,256):
            q=[(a+dx,b) for a,b in pts];p.line('#fa792c',q,23);p.line('#ffb34a',q,9)
            if n%2==0:p.line('#ffe18a',[(a+dx+2,b) for a,b in pts],2)
    p.layer('Floating crust islands with incandescent underside')
    for n in range(24):
        x=r.randrange(256);y=r.randrange(256);w=r.uniform(8,25);h=r.uniform(5,14)
        for dx in (-256,0,256):
            for dy in (-256,0,256):
                if not (-w<x+dx<256+w and -h<y+dy<256+h):continue
                q=[(x+dx-w,y+dy),(x+dx-w*.4,y+dy-h),(x+dx+w*.7,y+dy-h*.6),(x+dx+w,y+dy+h*.4),(x+dx-w*.2,y+dy+h)]
                p.poly('#7b342e',q);p.poly('#34303a',[(x+dx+(a-x-dx)*.8,y+dy+(b-y-dy)*.8-1) for a,b in q]);p.line('#ff9e3e',[q[4],q[3]],1.6)


def chunk(p,x,y,w,h,seed=0):
    r=random.Random(seed)
    q=[(x-w*.5,y-5),(x-w*.52,y-h*.48),(x-w*.22,y-h),(x+w*.24,y-h*.91),(x+w*.5,y-h*.47),(x+w*.43,y),(x,y+6)]
    p.poly('#222832',q)
    p.poly('#5a5760',[q[0],q[1],q[2],(x,y-h*.86),(x-3,y-2)])
    p.poly('#3b3d49',[(x,y-h*.86),q[3],q[4],q[5],q[6],(x-3,y-2)])
    p.poly('#82716d',[q[1],q[2],q[3],(x+w*.15,y-h*.76),(x-w*.18,y-h*.79)])
    p.line('#af9380',[(q[1][0]+2,q[1][1]),q[2],q[3]],.85)
    p.line('#1d2632',[(x-3,y-h*.78),(x+3,y-h*.52),(x-5,y-h*.28),(x+3,y-5)],1.2)
    for n in range(3):
        yy=y-h*(.22+n*.2);p.line('#2b2d38',[(x-w*.4,yy),(x-w*.1,yy+2),(x+w*.35,yy-3)],.7)
    for n in range(5):
        xx=x+r.uniform(-.3,.3)*w;yy=y-r.uniform(.1,.72)*h;p.line('#777078',[(xx,yy),(xx+3,yy-1)],.45)


def props(p,kind,v):
    r=random.Random(900+kind*33+v)
    p.oval('#24252e',37,198,109,18)
    if kind==0:
        # Varied broken hexagonal outcrops rather than parallel round poles.
        for n in range(4):
            x=52+n*23;y=199+(n%2)*8;h=[68,128,94,46][(n+v)%4]+v*4
            chunk(p,x,y,31+r.randrange(12),h,n+v*8)
            if n%2:p.line('#b96540',[(x+5,y-h*.5),(x+2,y-h*.3),(x+8,y-7)],.9)
    elif kind==1:
        # Charred, tapered wood with split branch ends, bark and visible roots.
        lean=(v-1.5)*9;h=105+v*13
        p.poly('#242b35',[(76,205),(83,205-h),(91+lean,188-h),(101,204-h),(106,207),(94,214)])
        p.poly('#57535a',[(80,205),(86,206-h),(91+lean,193-h),(94,205-h),(94,210)])
        for side in (-1,1):
            for n in range(3):
                y=177-n*24;tip=90+side*(26+n*7);q=[(91,y),(tip,y-15),(tip+side*6,y-38)]
                p.line('#242b34',q,7-n);p.line('#807166',[(x-1,y-1) for x,y in q],.8)
            p.line('#3f4149',[(90,204),(90+side*24,210),(90+side*39,207)],7)
            p.line('#8a7362',[(90,203),(90+side*25,207),(90+side*35,206)],.7)
        for n in range(5):p.line('#1d2935',[(83+n*4,205-h*.7),(81+n*4,197)],.8)
        p.line('#ca8957',[(87,199),(89,187),(87,178)],.7)
    elif kind==2:
        p.oval('#675b45',46,174,92,39);p.oval('#b39b65',54,179,77,29);p.oval('#3c3739',61,180,64,23);p.oval('#121f29',69,182,47,16)
        for n in range(7):chunk(p,50+n*13,202+(n%2)*7,21,15+n%3*7,n+v*22)
        p.line('#d4b777',[(51,186),(66,176),(94,175),(121,184)],1.5)
        for n in range(20):p.oval('#c9ad73',r.uniform(47,134),r.uniform(194,213),1.8,1.1)
    else:
        # Faceted glassy obsidian spires with asymmetric broken tops.
        for n in range(4):
            x=57+n*23;y=205+n%2*4;h=48+(n*39+v*23)%102;lean=(-1 if n%2 else 1)*(5+v*2)
            p.poly('#172531',[(x-13,y),(x-11,y-h*.52),(x+lean,y-h),(x+14,y-h*.66),(x+11,y+3)])
            p.poly('#3f5064',[(x-10,y-h*.52),(x+lean,y-h+3),(x+1,y-h*.45),(x-4,y-3)])
            p.poly('#293548',[(x+lean,y-h+3),(x+12,y-h*.65),(x+9,y),(x+1,y-7)])
            p.line('#bdc1bd',[(x-10,y-h*.52),(x+lean,y-h+4)],.8);p.line('#76879a',[(x+1,y-h*.45),(x-3,y-9)],.7)
            p.line('#b35b3e',[(x+7,y-h*.3),(x+6,y-2)],.8)
    for n in range(7):chunk(p,40+r.random()*100,208+r.random()*6,7+r.random()*9,4+r.random()*6,n+kind*9)


def run(root,runtime):
    root.mkdir(parents=True,exist_ok=True);runtime.mkdir(parents=True,exist_ok=True);start_krita_if_needed()
    jobs=[('ground',512,512,ground),('macro',1024,1024,macro),('channel-wall',512,256,wall),('lava-surface',256,256,lava),('scenery',1440,1840,None)]
    for name,w,h,draw in jobs:
        receipt=root/(name+'-receipt.json')
        if receipt.exists():
            import hashlib
            saved=json.loads(receipt.read_text());assert hashlib.sha256((root/(name+'.png')).read_bytes()).hexdigest()==saved['png_sha256'];print('Verified saved',name,flush=True);continue
        assert not (root/(name+'.kra')).exists(),'Inspect saved document before replaying edits'
        with KritaMCP(root/('logs-'+name),timeout=240) as c:
            p=Paint(c,w,h,'Caldera recessed lava / '+name)
            if draw:draw(p)
            else:
                for kind,label in enumerate(['Fractured basalt outcrops','Charred split trees','Sulphur vents','Obsidian glass']):
                    p.layer(label)
                    for v in range(4):p.s=2;p.ox=kind*360;p.oy=v*460;props(p,kind,v)
            result=retain_finish(p,root,name)
        receipt.write_text(json.dumps(result,indent=2),encoding='utf8')
        if name!='lava-surface':shutil.copy2(root/(name+'.png'),runtime/(name+'.png'))
        if name=='scenery':
            im=Image.open(root/'scenery.png').convert('RGBA');a=im.getchannel('A');edge=ImageChops.subtract(a.filter(ImageFilter.MaxFilter(7)),a);out=Image.new('RGBA',im.size,(255,255,255,0));out.putalpha(edge);out.save(runtime/'scenery-edge.png')
        print('Saved native Krita',name,flush=True)


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root,a.runtime)

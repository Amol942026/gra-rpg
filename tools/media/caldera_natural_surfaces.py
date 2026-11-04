"""Native Krita geological surface refinement after actual in-game review."""
from pathlib import Path
import argparse,random,math,json,shutil
from polish_volcano_art import Paint
from volcano_river_detail import retain_finish
from krita_mcp_client import KritaMCP,start_krita_if_needed

def repeats(points,size=512):
    for dx,dy in [(0,0),(-size,0),(size,0),(0,-size),(0,size),(-size,-size),(size,size),(-size,size),(size,-size)]:
        q=[(x+dx,y+dy) for x,y in points]
        if max(x for x,y in q)>=0 and min(x for x,y in q)<size and max(y for x,y in q)>=0 and min(y for x,y in q)<size:yield q

def ground(p):
    r=random.Random(41126);p.layer('Unbroken volcanic ash substrate');p.poly('#414048',[(0,0),(512,0),(512,512),(0,512)])
    p.layer('Scumbled weathering and iron rich cooling fields')
    for n in range(58):
        x=r.randrange(512);y=r.randrange(512);size=r.uniform(40,165);c=r.choice(['#777065','#292e3b','#70594f','#585760'])
        for q in repeats([(x-size/2,y-size/2),(x+size/2,y+size/2)]):
            a,b=q;p.soft(c,(a[0]+b[0])/2,(a[1]+b[1])/2,size,.21)
    p.layer('Twisted pahoehoe folds and fractured basalt')
    for n in range(19):
        x=r.randrange(512);y=r.randrange(512);a=r.random()*math.tau;length=r.uniform(40,135)
        points=[]
        for j in range(7):
            t=j/6;xx=t*length;yy=math.sin(t*math.pi)*r.uniform(10,23)
            points.append((x+xx*math.cos(a)-yy*math.sin(a),y+xx*math.sin(a)+yy*math.cos(a)))
        for q in repeats(points):
            p.line('#30323b',q,4.7);p.line('#69605e',[(x-1,y-2) for x,y in q],.9)
            p.line('#4c4951',[(x+3,y+3) for x,y in q],1.6)
    for n in range(22):
        x=r.randrange(512);y=r.randrange(512);q=[(x,y),(x+6,y+9),(x+2,y+16),(x+13,y+22),(x+8,y+34)]
        for pts in repeats(q):p.line('#292e38',pts,.8)
    p.layer('Porous stone grains and sparse scoria')
    for n in range(155):
        x=r.randrange(512);y=r.randrange(512);p.oval(r.choice(['#8b7c69','#292d36','#625a58','#35343e']),x,y,r.uniform(.5,2.9),r.uniform(.4,1.5))
    for n in range(16):
        x=r.randrange(512);y=r.randrange(512);w=r.uniform(3,9)
        for q in repeats([(x-w,y),(x-2,y-3),(x+w,y-2),(x+w*.6,y+2),(x-1,y+3)]):p.poly('#35353e',q);p.line('#72655c',q[:3],.6)

def wall(p):
    r=random.Random(714);p.layer('Canyon basalt base');p.poly('#373039',[(0,0),(512,0),(512,256),(0,256)])
    p.layer('Irregular fractured faces and weathered strata')
    for row in range(3):
        for col in range(6):
            x=col*105-35+(row%2)*45;y=row*90-15;w=r.uniform(83,126);h=r.uniform(70,104)
            q=[(x,y+8),(x+w*.7,y),(x+w,y+h*.4),(x+w*.86,y+h),(x+18,y+h*.93),(x-4,y+h*.45)]
            p.poly(r.choice(['#48404a','#4f424a','#3e3b45','#57464d']),q)
            p.line('#786065',q[:3],1.5);p.line('#272832',[q[3],q[4],q[5]],2)
            p.line('#292934',[(x+w*.56,y+12),(x+w*.48,y+h*.35),(x+w*.57,y+h*.6)],1)
    p.layer('Porosity and molten bounce light')
    for n in range(60):
        x=r.randrange(512);y=r.randrange(256);p.line('#7c6063' if n%3 else '#272733',[(x,y),(x+r.uniform(2,9),y-1)],.7)
    for x in range(0,570,65):p.soft('#e86b39',x,248,132,.32)
    p.layer('Rim contact shadow');p.line('#202631',[(0,2),(512,2)],7)

def bank(p):
    r=random.Random(822);p.layer('Contact ash on the fractured canyon lips')
    for y in (82,429):
        for x in range(-20,550,55):p.soft('#211f29',x,y,85,.55)
    p.layer('Jagged basalt blocks at ground level')
    for band in (77,428):
        for n in range(19):
            x=n*29-12;y=band+r.uniform(-9,9);w=r.uniform(22,43);h=r.uniform(9,20)
            q=[(x-w/2,y),(x-w*.35,y-h*.4),(x+w*.26,y-h*.52),(x+w*.5,y+h*.1),(x+w*.25,y+h*.45),(x-w*.25,y+h*.4)]
            for dx in (-512,0,512):
                pts=[(a+dx,b) for a,b in q]
                if max(a for a,b in pts)<0 or min(a for a,b in pts)>=512:continue
                p.poly('#35333d',pts);p.poly('#63545a',[pts[0],pts[1],pts[2],(x+dx,y)]);p.line('#8f756b',pts[:3],.8)

def lava(p):
    r=random.Random(30017);p.layer('Deep vermilion molten bed');p.poly('#c14223',[(0,0),(256,0),(256,256),(0,256)])
    p.layer('Irregular incandescent convection currents')
    for n in range(5):
        x=n*57+12;pts=[(x+math.sin(j*math.tau/16+n*1.7)*23,j*16) for j in range(-2,19)]
        for q in repeats(pts,256):
            p.line('#ed712c',q,38);p.line('#ffa13c',q,18)
            if n%2==0:p.line('#ffd377',[(x-3,y) for x,y in q],3)
    p.layer('Broken irregular cooling crust with ember edges')
    for n in range(35):
        x=r.randrange(256);y=r.randrange(256);w=r.uniform(8,27);h=r.uniform(7,21)
        q=[(x-w,y),(x-w*.6,y-h*.8),(x+w*.1,y-h),(x+w*.75,y-h*.3),(x+w,y+h*.5),(x+w*.2,y+h),(x-w*.7,y+h*.6)]
        for pts in repeats(q,256):
            cx=sum(a for a,b in pts)/7;cy=sum(b for a,b in pts)/7
            p.poly('#e87d31',pts);p.poly('#55333a',[(cx+(a-cx)*.91,cy+(b-cy)*.89-1) for a,b in pts]);p.poly('#36303a',[(cx+(a-cx)*.79,cy+(b-cy)*.74-2) for a,b in pts]);p.line('#82605b',pts[1:4],.65)
            p.line('#ba4929',[(cx-4,cy-4),(cx+2,cy),(cx-1,cy+6)],.7)

def run(root,runtime):
    root.mkdir(exist_ok=False,parents=True);start_krita_if_needed()
    for name,w,h,fn in [('ground',512,512,ground),('channel-wall',512,256,wall),('bank',512,512,bank),('lava-surface',256,256,lava)]:
        with KritaMCP(root/('logs-'+name),timeout=240) as c:
            p=Paint(c,w,h,'Natural Caldera / '+name);fn(p);receipt=retain_finish(p,root,name)
        (root/(name+'-receipt.json')).write_text(json.dumps(receipt,indent=2),encoding='utf8')
        if name!='lava-surface':shutil.copy2(root/(name+'.png'),runtime/(name+'.png'))
        print('Native refinement saved',name,flush=True)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root,a.runtime)

"""Native Krita gateway and genuinely continuous volcanic ground.

The tile is painted with repeated features on a larger canvas, then technically
cropped with a safe margin. No sprite-edge normal/AO bake is applied to terrain.
"""
from pathlib import Path
import argparse,math,random,json,hashlib
from PIL import Image,ImageChops,ImageFilter
from polish_volcano_art import Paint
from volcano_river_detail import retain_finish
from krita_mcp_client import KritaMCP,start_krita_if_needed

def frame(p):
    p.s=2;p.layer('Contact with the ground',.38);p.oval('#111c25',23,201,134,17)
    p.layer('Carved basalt arch and pedestal')
    p.poly('#232e3c',[(26,195),(51,182),(128,183),(154,199),(139,214),(43,214)])
    p.poly('#697179',[(26,195),(51,183),(128,184),(153,198),(130,204),(49,204)])
    p.poly('#414c59',[(49,204),(130,204),(139,213),(43,213)])
    p.line('#a4aaa6',[(36,194),(55,187),(124,188),(141,196)],1)
    for side in (-1,1):
        x=90+side*48
        p.poly('#263240',[(x-13,195),(x-11,94),(x-6,86),(x+9,93),(x+13,195),(x+5,202)])
        p.poly('#576776',[(x-10,191),(x-8,99),(x-3,92),(x+2,98),(x+3,195)])
        p.poly('#344653',[(x+3,98),(x+8,96),(x+10,192),(x+4,198)])
        p.line('#a6b1b2',[(x-8,180),(x-7,105),(x-3,95)],.8)
        for n in range(5):
            y=112+n*17;p.line('#1f2c39',[(x-9,y),(x+1,y+3),(x+10,y)],1)
            p.line('#79858a',[(x-9,y-1),(x-1,y+1)],.6)
        p.poly('#6d777b',[(x-16,188),(x+14,190),(x+17,199),(x-16,199)])
        p.line('#b19b76',[(x-11,189),(x+10,191)],1)
    # Connected band beneath the individually carved keystones.
    outer=[(90+53*math.cos(a),99+79*math.sin(a)) for a in [math.pi+i*math.pi/40 for i in range(41)]]
    inner=[(90+37*math.cos(a),99+63*math.sin(a)) for a in [2*math.pi-i*math.pi/40 for i in range(41)]]
    p.poly('#243340',outer+inner)
    for n in range(13):
        a=math.pi+n*math.pi/13+.006;b=math.pi+(n+1)*math.pi/13-.006
        q=[(90+52*math.cos(a),99+78*math.sin(a)),(90+52*math.cos(b),99+78*math.sin(b)),(90+38*math.cos(b),99+64*math.sin(b)),(90+38*math.cos(a),99+64*math.sin(a))]
        p.poly(['#627180','#596b7b','#6c7a84'][n%3],q)
        p.line('#a5b0b0',q[:2],.8);p.line('#273846',[q[2],q[3]],1.2)
        if n%3==1:
            x=sum(x for x,y in q)/4;y=sum(y for x,y in q)/4;p.line('#344552',[(x-1,y-4),(x+2,y),(x-1,y+4)],.65)
    p.poly('#394c5c',[(79,25),(89,13),(101,24),(98,42),(88,49),(80,40)])
    p.poly('#819399',[(81,25),(89,17),(97,25),(94,37),(87,43),(82,37)])
    p.line('#d1b27d',[(88,23),(85,30),(91,33),(87,39)],1)
    p.layer('Eldrin violet runes and worn metal inlays')
    for side in (-1,1):
        x=90+side*48
        for n in range(4):
            y=111+n*21;p.line('#a49072',[(x-4,y-5),(x-3,y+5),(x+4,y+5)],.7)
            p.line('#91b9e6',[(x-2,y-3),(x+2,y),(x-1,y+3)],1.1)
    for n in range(11):
        a=math.pi+(n+.5)*math.pi/11;x=90+44*math.cos(a);y=99+71*math.sin(a)
        p.oval('#d1ad76',x-1,y-1,2,2)
    for side in (-1,1):
        q=[(90+side*38,179),(90+side*39,105),(90+side*33,71),(90+side*19,43),(90,35)]
        p.line('#7569ae',q,2);p.line('#b2bff0',q,.7)
    p.line('#aa9575',[(52,204),(73,205),(75,208),(108,208),(111,204),(131,204)],.8)

def core(p):
    p.layer('Deep circular rift');p.oval('#121d43',10,10,236,236);p.select()
    p.layer('Nebula and inner light');p.soft('#6566b9',105,87,170,.7);p.soft('#345b9c',145,158,160,.55);p.soft('#a9c4e7',111,121,67,.32)
    p.layer('Spiral filaments and drifting stars')
    for arm in range(7):
        pts=[]
        for n in range(23):
            r=14+n*4;a=arm*math.tau/7+r*.027;pts.append((128+math.cos(a)*r,128+math.sin(a)*r))
        p.line('#4e5094',pts,10);p.line('#8999dc',pts,3.2);p.line('#c9dbf6',pts[6:18],.75)
    r=random.Random(1104)
    for n in range(33):
        a=r.random()*math.tau;d=r.uniform(28,108);x=128+math.cos(a)*d;y=128+math.sin(a)*d;p.oval('#c9d8f0',x,y,r.uniform(.6,1.9),r.uniform(.6,1.9))
    p.deselect()

def ground(p):
    p.layer('Continuous cooled basalt');p.poly('#45434a',[(0,0),(1024,0),(1024,1024),(0,1024)])
    p.layer('Wrapped soft ash deposits');r=random.Random(110411)
    features=[(r.uniform(0,512),r.uniform(0,512),r.uniform(55,145),r.choice(['#73695e','#2b323e','#795e50'])) for n in range(17)]
    for dx in (-512,0,512):
        for dy in (-512,0,512):
            p.ox=256+dx;p.oy=256+dy
            for x,y,size,color in features:p.soft(color,x,y,size,.2)
    p.layer('Wrapped cooling folds and subtle mineral grains')
    cracks=[]
    for n in range(10):
        x=r.uniform(0,512);y=r.uniform(0,512);a=r.random()*math.tau;length=r.uniform(35,100)
        cracks.append([(x+math.cos(a)*length*j/7-math.sin(a)*math.sin(j*.45)*12,y+math.sin(a)*length*j/7+math.cos(a)*math.sin(j*.45)*12) for j in range(8)])
    grains=[(r.uniform(0,512),r.uniform(0,512),r.uniform(.6,2.3)) for n in range(40)]
    for dx in (-512,0,512):
        for dy in (-512,0,512):
            p.ox=256+dx;p.oy=256+dy
            for pts in cracks:p.line('#32343e',pts,1.4);p.line('#68615d',[(x,y-1) for x,y in pts],.5)
            for x,y,s in grains:
                if 0<=p.ox+x<1024 and 0<=p.oy+y<1024:p.oval('#76695c',x,y,s,s*.55)
    p.ox=p.oy=0

def macro(p):
    r=random.Random(421104);p.layer('Feathered ash and mineral transitions')
    for n in range(55):p.soft(r.choice(['#8c735e','#a19075','#2a3340','#6e544a']),r.uniform(0,1024),r.uniform(0,1024),r.uniform(90,260),.11)
    p.layer('Broad translucent soot drift')
    for n in range(14):p.soft('#26313c',r.uniform(0,1024),r.uniform(0,1024),r.uniform(180,360),.085)

def run(root,runtime):
    root.mkdir(parents=True,exist_ok=False);runtime.mkdir(parents=True,exist_ok=True);start_krita_if_needed()
    for name,w,h,paint in [('portal-frame',360,460,frame),('portal-core',256,256,core),('ground',1024,1024,ground),('macro',1024,1024,macro)]:
        with KritaMCP(root/('logs-'+name),timeout=240) as c:
            p=Paint(c,w,h,'Caldera gateway / '+name);paint(p);receipt=retain_finish(p,root,name)
        (root/(name+'-receipt.json')).write_text(json.dumps(receipt,indent=2),encoding='utf8')
        if name in ('portal-frame','ground','macro'):
            im=Image.open(root/(name+'.png')).convert('RGBA')
            if name=='ground':im=im.crop((256,256,768,768))
            im.save(runtime/(name+'.png'))
        print('Saved native Krita',name,flush=True)
    # A stable interaction mask includes the magical opening, not its faint glow.
    frame_im=Image.open(runtime/'portal-frame.png').convert('RGBA');a=frame_im.getchannel('A');mask=a.point(lambda v:255 if v>=160 else 0)
    from PIL import ImageDraw
    ImageDraw.Draw(mask).ellipse((102,56,258,358),fill=255)
    mask.save(runtime/'portal-mask.png');edge=ImageChops.subtract(mask.filter(ImageFilter.MaxFilter(7)),mask);im=Image.new('RGBA',frame_im.size,(255,255,255,0));im.putalpha(edge);im.save(runtime/'portal-outline.png')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.root,a.runtime)

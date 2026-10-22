"""Original layered title illustration, drawn with native Krita MCP tools.

All shapes are authored here, no external images, image AI or cloud services.
Krita owns painting/export; Pillow below only validates the finished files.
"""
import argparse
import hashlib
import json
import math
import random
import zipfile
from pathlib import Path
from krita_mcp_client import KritaMCP, start_krita_if_needed


def smooth(points, closed=False, steps=7):
    points = list(points)
    extended = [points[-1]] + points + points[:2] if closed else [points[0]] + points + [points[-1]]
    result = []
    for n in range(1, len(extended)-2):
        a,b,c,d = extended[n-1:n+3]
        for i in range(steps):
            t=i/steps
            result.append(tuple(.5*((2*b[k])+(-a[k]+c[k])*t+(2*a[k]-5*b[k]+4*c[k]-d[k])*t*t+(-a[k]+3*b[k]-3*c[k]+d[k])*t*t*t) for k in range(2)))
    if not closed:result.append(points[-1])
    return result


def mix(a,b,t):
    av=[int(a[n:n+2],16) for n in (1,3,5)];bv=[int(b[n:n+2],16) for n in (1,3,5)]
    return '#'+''.join(f'{round(x+(y-x)*t):02x}' for x,y in zip(av,bv))


def author(root,resume=False):
    root.mkdir(parents=True,exist_ok=resume)
    start_krita_if_needed()
    rng=random.Random(20261022)
    with KritaMCP(root/'logs') as c:
        if resume:
            records=[json.loads(line) for line in (root/'logs/mcp-calls.jsonl').read_text(encoding='utf-8').splitlines()]
            completed=[]
            previous=None
            for record in records:
                probe=record['tool']=='krita_state' and previous and (previous.get('isError') or previous['tool']=='krita_state')
                if not record.get('isError') and not probe:completed.append(record)
                previous=record
            owner=completed[0]['metadata']['affected_document_id']
            actual=c.call
            current=actual('krita_state',document_id=owner)
            assert current['runtime_compatibility']['status']=='compatible'
            # Replay only the verified call receipts, never the mutations. The
            # first unfinished operation continues in the already-owned canvas.
            cursor=0
            def continued(tool_name,**arguments):
                nonlocal cursor
                if cursor<len(completed):
                    saved=completed[cursor];cursor+=1
                    assert saved['tool']==tool_name and saved['arguments']==arguments,(tool_name,'resume mismatch')
                    return saved['metadata']
                return actual(tool_name,**arguments)
            c.call=continued
        doc=c.call('krita_document',action='create',width=1440,height=900,name='Emberfall / Brama Świtu / title painting')['affected_document_id']
        state=c.call('krita_state',document_id=doc)
        assert state['runtime_compatibility']['status']=='compatible'
        base=state['active_layer_id']
        c.call('krita_edit',action='select_all',document_id=doc)
        c.call('krita_edit',action='clear',document_id=doc,layer_id=base)
        c.call('krita_edit',action='deselect',document_id=doc)
        layers={};lid=base
        def layer(name,opacity=1):
            nonlocal lid
            c.call('krita_layers',action='create',document_id=doc,name=name)
            lid=c.call('krita_state',document_id=doc)['active_layer_id'];layers[name]=lid
            if opacity!=1:c.call('krita_layers',action='set_properties',document_id=doc,layer_id=lid,opacity=opacity)
            print('Krita: '+name,flush=True)
            return lid
        def poly(col,pts):
            if max(x for x,y in pts)<=0 or min(x for x,y in pts)>=1440 or max(y for x,y in pts)<=0 or min(y for x,y in pts)>=900:return
            c.call('krita_draw',shape='polygon',document_id=doc,layer_id=lid,foreground=col,points=[{'x':round(x,3),'y':round(y,3)} for x,y in pts],fill_style='ForegroundColor',stroke_style='None')
        def shape(col,pts):poly(col,smooth(pts,True))
        def oval(col,x,y,w,h):
            if x+w<=0 or x>=1440 or y+h<=0 or y>=900:return
            c.call('krita_draw',shape='ellipse',document_id=doc,layer_id=lid,foreground=col,x=x,y=y,width=w,height=h,fill_style='ForegroundColor',stroke_style='None')
        def path(col,pts,width=2):
            if max(x for x,y in pts)<-width or min(x for x,y in pts)>1440+width or max(y for x,y in pts)<-width or min(y for x,y in pts)>900+width:return
            c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,foreground=col,points=[{'x':round(x,3),'y':round(y,3)} for x,y in smooth(pts)],fill_style='None',stroke_style='ForegroundColor',brush_size=width)
        def rect(col,x,y,w,h):poly(col,[(x,y),(x+w,y),(x+w,y+h),(x,y+h)])
        def pine(x,y,h,col):
            pts=[(x,y-h)]
            for side in (1,-1):
                seq=range(1,8) if side==1 else range(7,0,-1)
                for n in seq:
                    yy=y-h+n*h/8;ww=h*(.035+n*.018)
                    pts.extend([(x+side*ww,yy+h*.075),(x+side*ww*.43,yy+h*.06)])
            poly(col,pts)
        layer('01 / Cielęcy zmierzch i światło księżyca')
        for n in range(60):rect(mix('#101b30','#42615d',n/59),0,n*15,1440,16)
        # Broad, soft illumination is built from nested native ellipses.
        for n in range(18):
            k=n/17;oval(mix('#243d4a','#718f7c',k),465+n*3,69+n*3,330-n*6,260-n*6)
        oval('#d8d9b3',585,125,86,86);oval('#e9e4be',591,130,71,72)
        for x,y,w,h in [(611,146,14,8),(638,173,9,11),(601,184,13,7)]:oval('#cbd2ad',x,y,w,h)
        for n in range(36):
            x=rng.randrange(180,1220);y=rng.randrange(25,320)
            if math.hypot(x-628,y-168)>100:oval('#91b4ae' if n%3 else '#ddcda5',x,y,1+n%2,1+n%2)
        layer('02 / Odległe grzbiety i warstwy lasu')
        shape('#293f4c',[(0,372),(134,308),(246,332),(385,251),(488,327),(708,282),(882,333),(1072,266),(1230,325),(1440,288),(1480,700),(-20,700)])
        shape('#304e54',[(0,429),(143,388),(268,415),(412,354),(552,392),(715,361),(871,407),(1052,341),(1237,401),(1450,350),(1480,700),(-20,700)])
        for row,(hue,baseY) in enumerate([('#2b494c',479),('#223e42',529)]):
            for n in range(22):pine(n*71-36+row*21,baseY+rng.randrange(-15,16),rng.randrange(76,157),hue)
        # Long banks of low cloud weave between the distant hills.
        for n in range(3):shape(['#3e5b5b','#3d5b59','#42645e'][n],[(110,430+n*35),(350,403+n*37),(677,425+n*33),(1070,390+n*42),(1446,421+n*35),(1448,452+n*32),(950,444+n*35),(510,457+n*34),(90,453+n*35)])
        layer('03 / Sol et sentier des ruines')
        shape('#182f32',[(-10,578),(163,529),(307,562),(499,535),(704,542),(870,503),(1108,529),(1450,500),(1460,920),(-20,920)])
        shape('#24413e',[(-10,682),(188,606),(349,618),(520,587),(681,609),(874,552),(1086,606),(1460,566),(1460,900),(-20,900)])
        shape('#3a5145',[(337,920),(424,781),(524,698),(556,619),(525,572),(577,570),(605,634),(579,713),(558,785),(653,920)])
        shape('#58634d',[(417,920),(451,814),(551,720),(580,637),(554,585),(575,583),(601,638),(574,732),(505,823),(520,920)])
        for n in range(24):
            x=rng.randrange(90,1320);y=rng.randrange(652,889)
            if abs(x-(535+(y-700)*-.25))<55:continue
            oval(['#314b3c','#294737','#3a553e'][n%3],x,y,30+n%5*8,6+n%4*3)
        # A shallow pool catches the portal, broken into slender reflections.
        oval('#243e41',313,602,458,107);oval('#36534e',358,615,360,65)
        for n in range(9):oval(mix('#557364','#8f9e76',n/9),451+n%3*8,631+n*4,180-n*12,2)
        layer('04 / Intérieur lumineux de la porte')
        oval('#1a3a40',421,292,218,345)
        for n in range(16):
            k=n/15;oval(mix('#284b4d','#6fa293',k),426+n*3.8,302+n*7.5,208-n*7.6,328-n*15)
        path('#a4c6aa',[(531,329),(524,414),(532,512),(527,596)],3)
        layer('05 / Arche de pierre, mousse et inscriptions')
        cx,cy,rx,ry=530,485,151,211
        outer=[(cx+math.cos(math.pi+i*math.pi/60)*rx,cy+math.sin(math.pi+i*math.pi/60)*ry) for i in range(61)]
        inner=[(cx+math.cos(i*math.pi/60)*(rx-39),cy-math.sin(i*math.pi/60)*(ry-43)) for i in range(61)]
        poly('#152b31',outer+inner)
        for side in (-1,1):
            x=379 if side<0 else 642
            poly('#314646',[(x,481),(x+38,481),(x+43,629),(x-8,629)])
            for n in range(5):
                yy=485+n*27
                poly('#50605a' if side<0 else '#3d5551',[(x+3,yy),(x+34,yy-1),(x+34,yy+24),(x+2,yy+24)])
                path('#728073',[(x+5,yy+2),(x+30,yy+1)],1.5)
        for n in range(13):
            a=math.pi+n*math.pi/13+.015;b=math.pi+(n+1)*math.pi/13-.015
            poly(['#52645f','#5c6b60','#465d58'][n%3],[(cx+math.cos(a)*151,cy+math.sin(a)*211),(cx+math.cos(b)*151,cy+math.sin(b)*211),(cx+math.cos(b)*112,cy+math.sin(b)*168),(cx+math.cos(a)*112,cy+math.sin(a)*168)])
            path('#7c8976',[(cx+math.cos(a)*144,cy+math.sin(a)*204),(cx+math.cos(b)*144,cy+math.sin(b)*204)],1.5)
            if n%2==0:
                x=cx+math.cos((a+b)/2)*132;y=cy+math.sin((a+b)/2)*188
                path('#b4c49a',[(x-3,y+6),(x+3,y),(x-1,y-5),(x+4,y-10)],2)
        for n in range(16):
            a=math.pi+(n/15)*math.pi;x=cx+math.cos(a)*155;y=cy+math.sin(a)*212
            oval('#36583b',x-10,y-4,22,10);oval('#52734a',x-8,y-5,13,5)
        for n in range(4):
            oval('#172e30',376-n*12,628+n*8,312+n*24,15)
            path('#76816b',[(400-n*9,628+n*8),(524,630+n*8),(659+n*9,628+n*8)],1.5)
        # Broken companions, ivy, fluted edges and a hanging lantern.
        for x,height in [(232,148),(787,114)]:
            poly('#263e3e',[(x,638),(x+12,638-height),(x+50,650-height),(x+57,640)])
            poly('#425650',[(x+12,638-height),(x+50,650-height),(x+42,659-height),(x+7,648-height)])
            for offset in (17,31,44):path('#52665b',[(x+offset,665-height),(x+offset-6,628)],1.5)
            for n in range(6):oval('#3d623f',x+10+n%2*23,595-n*14,20,9)
        path('#142e32',[(780,511),(797,468),(831,468),(842,506)],5)
        rect('#9b8255',825,505,15,25);rect('#e3c991',829,509,7,14)
        layer('06 / Feuillage et arbres au premier plan')
        for side in (-1,1):
            def tx(x):return x if side<0 else 1440-x
            shape('#0b1c27',[(tx(-10),-30),(tx(146),-20),(tx(134),122),(tx(103),284),(tx(120),472),(tx(104),668),(tx(162),840),(tx(236),910),(tx(-10),950)])
            path('#29403d',[(tx(109),39),(tx(84),222),(tx(90),404),(tx(74),658),(tx(114),799)],6)
            for pts in [[(85,312),(171,255),(230,205),(300,193)],[(99,182),(215,98),(349,55),(458,17)],[(81,544),(177,473),(208,397)]]:
                path('#102630',[(tx(x),y) for x,y in pts],11)
            for n in range(13):
                x=tx(66+n*28);y=30+int(math.sin(n*.8)*24)
                oval('#0d252d',x-75,y-63,159,87)
            shape('#0d2529',[(tx(-20),743),(tx(94),715),(tx(178),751),(tx(259),812),(tx(375),873),(tx(548),923),(tx(-20),940)])
            for n in range(11):
                x=tx(56+n*24);y=792+n*9;h=48+n%4*14
                pts=[(x,y-h)]
                for k in range(1,7):pts.extend([(x+side*(5+k*2),y-h+k*h/7),(x+side*2,y-h+(k+.3)*h/7)])
                for k in range(6,0,-1):pts.extend([(x-side*(5+k*2),y-h+k*h/7),(x-side*2,y-h+(k-.3)*h/7)])
                poly('#244535' if n%3 else '#365b40',pts)
                path('#5a714c',[(x,y-h+8),(x+side*2,y)],1)
        foreground=lid
        layer('07 / Brume peinte et nappes de rosée',.17)
        for n in range(4):
            shape('#9bc1ac',[(115,587+n*47),(292,565+n*47),(521,583+n*47),(769,555+n*47),(996,578+n*47),(1382,552+n*47),(1400,575+n*47),(1100,595+n*47),(756,586+n*47),(418,604+n*47),(110,606+n*47)])
        mist=lid
        layer('08 / Halo de la porte',.12)
        for n in range(5):oval(mix('#9dd3be','#d2e8ca',n/4),439+n*9,323+n*18,182-n*18,302-n*36)
        glow=lid
        kra=root/'forest-gate.kra'
        c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(root/'poster.png'),overwrite=False)
        c.call('krita_preview',document_id=doc,max_width=1440,max_height=900)
        for name,identifier in [('foreground',foreground),('mist',mist),('glow',glow)]:
            c.call('krita_asset',action='export_layer',document_id=doc,layer_id=identifier,output_path=str(root/(name+'.png')),crop='canvas',overwrite=False)
        for identifier in (foreground,mist,glow):c.call('krita_layers',action='set_properties',document_id=doc,layer_id=identifier,visible=False)
        c.call('krita_save_export',document_id=doc,png_path=str(root/'background.png'),overwrite=False)
        # Restore the editable master without changing its saved full composition.
        for identifier in (foreground,mist,glow):c.call('krita_layers',action='set_properties',document_id=doc,layer_id=identifier,visible=True)
        c.call('krita_state',document_id=doc)
        c.call('krita_document',action='close',document_id=doc,discard_changes=True)
    from PIL import Image
    outputs={}
    for name in ('poster','background','foreground','mist','glow'):
        p=root/(name+'.png')
        with Image.open(p) as im:im.load();assert im.size==(1440,900) and im.mode=='RGBA'
        outputs[name]={'file':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
    with zipfile.ZipFile(kra) as z:assert z.testzip() is None and 'maindoc.xml' in z.namelist()
    receipt={'status':'PASS','authoring':'Krita 5.3.4.1 native MCP draw','layers':layers,'kra':str(kra),'outputs':outputs,'external_assets':[],'original_art':True}
    (root/'provenance.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')
    print('PASS: Krita layered forest gate, five checked PNG exports',flush=True)


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);p.add_argument('--resume',action='store_true');a=p.parse_args();author(a.root,a.resume)

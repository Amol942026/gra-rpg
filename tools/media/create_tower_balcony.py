"""Original valley and terrace paintings, authored only by native Krita MCP."""
from pathlib import Path
import argparse, hashlib, json, math, random, zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed
from create_menu_forest import smooth, mix


def author(root,resume=False):
    root.mkdir(parents=True, exist_ok=resume)
    start_krita_if_needed()
    rng = random.Random(20261027)
    receipts = []
    with KritaMCP(root/'logs') as c:
        if resume:
            records=[json.loads(line) for line in (root/'logs/mcp-calls.jsonl').read_text(encoding='utf-8').splitlines()]
            completed=[r for r in records if not r.get('isError')]
            owner=completed[0]['metadata']['affected_document_id'];actual=c.call
            current=actual('krita_state',document_id=owner)
            assert current['runtime_compatibility']['status']=='compatible'
            cursor=0
            def continued(tool_name,**arguments):
                nonlocal cursor
                if cursor<len(completed):
                    saved=completed[cursor];cursor+=1
                    assert saved['tool']==tool_name and saved['arguments']==arguments,(tool_name,'resume mismatch')
                    return saved['metadata']
                return actual(tool_name,**arguments)
            c.call=continued
        def document(name, width, height):
            nonlocal doc, lid, layers, canvas_size
            canvas_size=(width,height)
            doc = c.call('krita_document', action='create', width=width, height=height,
                         name='Emberfall / Eldrin / '+name)['affected_document_id']
            state = c.call('krita_state', document_id=doc)
            assert state['runtime_compatibility']['status'] == 'compatible'
            lid = state['active_layer_id']; layers = []
            c.call('krita_edit', action='select_all', document_id=doc)
            c.call('krita_edit', action='clear', document_id=doc, layer_id=lid)
            c.call('krita_edit', action='deselect', document_id=doc)
        def layer(name):
            nonlocal lid
            c.call('krita_layers', action='create', document_id=doc, name=name)
            lid = c.call('krita_state', document_id=doc)['active_layer_id']
            layers.append({'name':name, 'uuid':lid})
            print('Krita: '+name, flush=True)
        def poly(color, points):
            if max(x for x,y in points)<=0 or min(x for x,y in points)>=canvas_size[0] or max(y for x,y in points)<=0 or min(y for x,y in points)>=canvas_size[1]:return
            c.call('krita_draw', shape='polygon', document_id=doc, layer_id=lid,
                   foreground=color, points=[{'x':round(x,3),'y':round(y,3)} for x,y in points],
                   fill_style='ForegroundColor', stroke_style='None')
        def shape(color, points): poly(color, smooth(points, True, 5))
        def oval(color, x, y, w, h):
            c.call('krita_draw', shape='ellipse', document_id=doc, layer_id=lid,
                   foreground=color, x=x,y=y,width=w,height=h,
                   fill_style='ForegroundColor',stroke_style='None')
        def path(color, points, width=2):
            if max(x for x,y in points)<=0 or min(x for x,y in points)>=canvas_size[0] or max(y for x,y in points)<=0 or min(y for x,y in points)>=canvas_size[1]:return
            try:
                c.call('krita_draw', shape='path', document_id=doc, layer_id=lid,
                       foreground=color,points=[{'x':round(x,3),'y':round(y,3)} for x,y in points],
                       fill_style='None',stroke_style='ForegroundColor',brush_size=width)
            except RuntimeError as error:
                if 'DRAW_NO_CHANGE' not in str(error):raise
        def pine(x,y,h,color):
            points=[(x,y-h)]
            for k in range(1,6): points.extend([(x+h*(.06+k*.025),y-h+k*h/6),(x+h*.035,y-h+(k+.3)*h/6)])
            points.append((x,y))
            for k in range(5,0,-1): points.extend([(x-h*(.06+k*.025),y-h+(k+.3)*h/6),(x-h*.035,y-h+k*h/6)])
            poly(color,points)
        def save(name, size):
            kra=root/(name+'.kra'); png=root/(name+'.png')
            c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
            c.call('krita_preview',document_id=doc,max_width=1050,max_height=900)
            c.call('krita_state',document_id=doc)
            c.call('krita_document',action='close',document_id=doc,discard_changes=False)
            from PIL import Image
            with Image.open(png) as im:
                im.load();assert im.size==size and im.mode=='RGBA'
            with zipfile.ZipFile(kra) as z: assert z.testzip() is None and 'maindoc.xml' in z.namelist()
            receipts.append({'name':name,'layers':layers,'size':size,
                'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),
                'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest()})
        doc=lid=None;layers=[];canvas_size=(0,0)
        document('Dolina Switu / panorama',1050,900)
        layer('01 / Zmierzch turkusowy i zlota poswiata')
        for n in range(60):
            t=n/59;color=mix('#203f58','#d6b882',min(1,t*2)) if t<.5 else mix('#d6b882','#3b6765',(t-.5)*2)
            poly(color,[(0,n*15),(1050,n*15),(1050,n*15+16),(0,n*15+16)])
        for n in range(14):
            r=96-n*5;oval(mix('#c9b183','#f4dfab',n/14),724-r,193-r,r*2,r*2)
        oval('#f9e6b5',703,172,42,42)
        layer('02 / Cienkie obloki nad grzbietami')
        for x,y,w in [(55,102,295),(400,76,240),(776,113,310),(160,182,290),(650,248,450)]:
            shape('#a4ada0',[(x,y+12),(x+w*.2,y),(x+w*.4,y+4),(x+w*.65,y-3),(x+w,y+9),(x+w*.7,y+15),(x+w*.3,y+17)])
            path('#c9c7ac',smooth([(x+20,y+8),(x+w*.3,y+6),(x+w*.6,y+8),(x+w*.85,y+9)]),1)
        layer('03 / Odlegle gory i granie')
        for color,points in [
          ('#708b90',[(-20,347),(70,288),(120,306),(212,208),(281,274),(356,244),(423,310),(520,267),(601,284),(681,232),(770,294),(884,211),(976,281),(1070,253)]),
          ('#54777e',[(-20,385),(91,322),(170,338),(247,300),(328,357),(445,302),(527,335),(598,310),(697,365),(782,301),(898,332),(1004,297),(1070,338)])]:
            poly(color,points+[(1070,600),(-20,600)])
        for pts in [[(171,254),(212,208),(259,254),(233,241),(218,230),(207,245),(194,240)],[(848,247),(884,211),(928,251),(897,236),(882,228),(871,241)]]:poly('#a0b0a6',pts)
        layer('04 / Dolina, jezioro i wstega rzeki')
        shape('#426c6b',[(-30,417),(123,381),(263,417),(374,387),(529,401),(701,383),(867,405),(1075,372),(1080,905),(-30,905)])
        shape('#547a6e',[(341,473),(471,417),(649,425),(807,466),(888,539),(728,638),(552,809),(187,911),(75,900),(242,724),(490,598),(587,520)])
        shape('#86a19a',[(520,438),(590,425),(671,433),(721,455),(709,471),(616,479),(568,470),(531,465)])
        shape('#a1b7a4',[(618,471),(653,475),(649,501),(606,524),(548,539),(526,559),(546,577),(604,599),(621,623),(605,655),(538,699),(435,740),(365,804),(316,900),(224,920),(274,797),(363,722),(508,668),(559,631),(556,613),(498,587),(477,559),(504,528),(585,505),(617,486)])
        path('#d1c7a0',smooth([(629,480),(613,511),(522,547),(513,566),(586,614),(577,640),(522,677),(401,733),(307,823),(280,900)]),3)
        for n in range(8): path('#c2c5aa',[(565+n*6,441+n*3),(671-n*3,440+n*3)],1)
        layer('05 / Lasy na zboczach i pasma mgiel')
        for bank in (-1,1):
            points=[(-20,435),(71,414),(156,435),(261,477),(351,543),(324,605),(204,713),(-20,781)] if bank<0 else [(1070,401),(956,427),(839,482),(752,559),(700,657),(785,753),(1070,819)]
            shape('#315c57',points)
            for n in range(65):
                y=rng.randrange(440,775);x=rng.randrange(0,225)+int((y-440)*.14) if bank<0 else rng.randrange(835,1070)-int((y-440)*.12)
                pine(x,y,rng.randrange(14,39),['#294f4d','#376451','#416d58'][n%3])
        for points in [[(-20,523),(143,503),(283,537),(360,564),(326,579),(155,545),(-20,550)],[(759,549),(862,518),(1070,492),(1070,514),(895,542),(772,565)]]:shape('#72968a',points)
        layer('06 / Trakt, most i pradawne ruiny')
        path('#8e9872',smooth([(1023,653),(858,625),(777,649),(698,688),(582,715),(508,711),(460,694)]),4)
        # A small stone bridge across the bend, with its own reflected arch.
        path('#324c49',[(492,676),(532,700),(566,692)],7)
        path('#a9a17a',[(492,669),(532,693),(566,685)],5)
        for x,y,h in [(774,491,42),(794,497,29),(817,488,38)]:
            poly('#47625a',[(x,y),(x,y-h),(x+10,y-h-3),(x+15,y-h),(x+15,y)])
            path('#b3ae86',[(x+1,y-h+1),(x+9,y-h-1)],1)
            path('#263f40',[(x+6,y-h+8),(x+6,y-h+17)],3)
        layer('07 / Bliskie urwiska, trawy i lesne szczegoly')
        shape('#244a45',[(-40,687),(83,645),(169,679),(208,765),(306,848),(372,913),(-30,930)])
        shape('#183d3d',[(1080,624),(980,663),(905,730),(827,805),(780,910),(1070,930)])
        for side in (-1,1):
            for n in range(18):
                x=rng.randrange(0,210) if side<0 else rng.randrange(907,1060);y=rng.randrange(740,940)
                pine(x,y,rng.randrange(45,100),['#15383a','#21483e','#2b5142'][n%3])
        for n in range(35):
            x=rng.randrange(0,200) if n%2 else rng.randrange(923,1050);y=rng.randrange(784,900)
            path('#657756',[(x,y),(x-3,y-10),(x-6,y-13)],1)
        save('tower-valley',(1050,900))

        factor=math.sqrt(2)
        def pt(u,v,z=0):return (64+(u-6.1)*40*factor,160+v*20*factor-z)
        boundary=[(6.1,-2.4),(10.5,-2.4),(11.4,-1.5),(11.4,1.5),(10.5,2.4),(6.1,2.4)]
        def railing(a,b):
            length=math.dist(pt(*a),pt(*b));steps=max(1,round(length/30))
            for n in range(steps+1):
                t=n/steps;u=a[0]+(b[0]-a[0])*t;v=a[1]+(b[1]-a[1])*t;x,y=pt(u,v)
                poly('#344450',[(x-5,y+2),(x-5,y-49),(x+5,y-49),(x+5,y+2)])
                poly('#829094',[(x-3,y),(x-3,y-47),(x,y-47),(x,y)])
                poly('#9a8b64',[(x-7,y-47),(x-7,y-52),(x+7,y-52),(x+7,y-47)])
            path('#344653',[pt(*a,51),pt(*b,51)],9)
            path('#bcac7b',[pt(*a,56),pt(*b,56)],4)
            path('#5e737d',[pt(*a,17),pt(*b,17)],3)
        document('Balkon / kamienny taras i tylna balustrada',480,320)
        layer('01 / Podparcie tarasu i kamienne warstwy')
        poly('#202e3e',[pt(u,v,-21) for u,v in boundary])
        poly('#526577',[pt(u,v,-11) for u,v in boundary])
        poly('#b4a47b',[pt(u,v,-3) for u,v in boundary])
        layer('02 / Blekitna posadzka i zlote fugi')
        poly('#51697a',[pt(u,v) for u,v in boundary])
        for u in [6.35+n*.54 for n in range(8)]:
            path('#7a8e99',[pt(u,-2.32),pt(u,2.32)],1)
        for v in [-1.75,-.9,0,.9,1.75]:path('#3c5267',[pt(6.15,v),pt(10.55,v)],1.3)
        path('#c4b483',[pt(6.15,-2.25),pt(10.43,-2.25),pt(11.22,-1.43),pt(11.22,1.43),pt(10.43,2.25),pt(6.15,2.25)],2)
        for n in range(14):
            u=rng.uniform(6.35,10.4);v=rng.uniform(-2,2);x,y=pt(u,v)
            path('#5e7787',[(x,y),(x+5,y-2),(x+9,y+1)],.7)
        # Compass rosette, painted into the paving rather than a runtime overlay.
        x,y=pt(8.6,0)
        oval('#445e70',x-27,y-19,54,38)
        for n in range(8):
            a=n*math.pi/4;poly('#a9a17b' if n%2==0 else '#71848b',[(x,y),(x+math.cos(a)*23,y+math.sin(a)*16),(x+math.cos(a+.24)*8,y+math.sin(a+.24)*6)])
        layer('03 / Tylna balustrada i wejscie')
        railing(boundary[0],boundary[1]);railing(boundary[1],boundary[2])
        x,y=pt(6.12,-2.4)
        poly('#3e5267',[(x-10,y+5),(x-10,y-76),(x+10,y-76),(x+10,y+5)])
        poly('#b9ab80',[(x-13,y-72),(x-13,y-80),(x+13,y-80),(x+13,y-72)])
        path('#d3c18a',[(x,y-60),(x-4,y-51),(x+4,y-44),(x,y-35)],1.7)
        save('tower-balcony-back',(480,320))
        document('Balkon / przednia balustrada i bluszcz',480,320)
        layer('01 / Przednia balustrada i bezpieczna krawedz')
        railing(boundary[5],boundary[4]);railing(boundary[4],boundary[3]);railing(boundary[3],boundary[2])
        layer('02 / Kamienny slupek przy wejsciu')
        x,y=pt(6.12,2.4)
        poly('#34495e',[(x-10,y+5),(x-10,y-76),(x+10,y-76),(x+10,y+5)])
        poly('#b9ab80',[(x-13,y-72),(x-13,y-80),(x+13,y-80),(x+13,y-72)])
        path('#d3c18a',[(x,y-60),(x-4,y-51),(x+4,y-44),(x,y-35)],1.7)
        layer('03 / Bluszcz na kamieniu i detale mosiezu')
        for u in [6.6,7.9,9.5,10.6]:
            v=2.4 if u<10.5 else 2.3;x,y=pt(u,v)
            path('#3d5a43',smooth([(x,y-48),(x+5,y-26),(x-4,y-11),(x+3,y+11),(x-6,y+25)]),2)
            for n in range(7):
                xx=x+(5 if n%2 else -5);yy=y-40+n*9
                poly('#6f895a' if n%3 else '#8c9b68',[(xx,yy),(xx+7,yy-4),(xx+9,yy+1),(xx+3,yy+7)])
        save('tower-balcony-front',(480,320))
    receipt={'status':'PASS','authoring':'Krita native MCP draw','original_art':True,
             'external_assets':[],'documents':receipts,'runtime_rgba_bytes':(1050*900+2*480*320)*4,
             'balcony_anchor':{'u':6.1,'v':0,'pixel':[64,160]},'floor':4}
    (root/'provenance.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')
    print('PASS: three original layered Krita paintings and PNG exports',flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--root',type=Path,required=True)
    parser.add_argument('--no-audio',action='store_true',required=True);parser.add_argument('--resume',action='store_true');args=parser.parse_args();author(args.root,args.resume)

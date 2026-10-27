"""Author original spruce, birch and oak masters with native Krita MCP drawing.

No downloaded artwork, cloud service or raster drawing fallback. Pillow only
inspects the exports. Each finished document is saved and closed independently.
"""
import argparse
import hashlib
import json
import math
import random
import zipfile
from pathlib import Path
from krita_mcp_client import KritaMCP, start_krita_if_needed
from create_menu_forest import smooth, mix


def author(root,resume=False):
    root.mkdir(parents=True, exist_ok=resume)
    start_krita_if_needed()
    receipts = json.loads((root/'progress.json').read_text(encoding='utf-8')) if resume and (root/'progress.json').exists() else []
    for species in ('spruce', 'birch', 'oak'):
        if any(r['species']==species for r in receipts):continue
        rng = random.Random({'spruce': 8911, 'birch': 4839, 'oak': 77631}[species])
        with KritaMCP(root / species / 'logs') as c:
            call_log=root/species/'logs/mcp-calls.jsonl'
            if resume and call_log.exists():
                completed=[r for r in map(json.loads,call_log.read_text(encoding='utf-8').splitlines()) if not r.get('isError')]
                owner=completed[0]['metadata']['affected_document_id']
                actual=c.call
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
            doc = c.call('krita_document', action='create', width=540, height=690,
                         name='Emberfall / woodland / ' + species)['affected_document_id']
            state = c.call('krita_state', document_id=doc)
            assert state['runtime_compatibility']['status'] == 'compatible'
            lid = state['active_layer_id']
            c.call('krita_edit', action='select_all', document_id=doc)
            c.call('krita_edit', action='clear', document_id=doc, layer_id=lid)
            c.call('krita_edit', action='deselect', document_id=doc)
            layers = []

            def native_draw(**arguments):
                try:return c.call('krita_draw',**arguments)
                except RuntimeError as error:
                    # A solid leaf wholly within an existing solid patch of
                    # the same color is a valid no-op. Do not replay it.
                    if 'DRAW_NO_CHANGE' not in str(error):raise

            def layer(name):
                nonlocal lid
                c.call('krita_layers', action='create', document_id=doc, name=name)
                lid = c.call('krita_state', document_id=doc)['active_layer_id']
                layers.append({'name': name, 'uuid': lid})
                print(species + ': ' + name, flush=True)

            def poly(col, pts):
                native_draw(shape='polygon', document_id=doc, layer_id=lid,
                       foreground=col, points=[{'x': round(x*1.5, 3), 'y': round(y*1.5, 3)} for x, y in pts],
                       fill_style='ForegroundColor', stroke_style='None')

            def shape(col, pts):
                poly(col, smooth(pts, True, 5))

            def path(col, pts, width=1):
                native_draw(shape='path', document_id=doc, layer_id=lid,
                       foreground=col, points=[{'x': round(x*1.5, 3), 'y': round(y*1.5, 3)} for x, y in smooth(pts, False, 5)],
                       fill_style='None', stroke_style='ForegroundColor', brush_size=width*1.5)

            def oval(col, x, y, w, h):
                native_draw(shape='ellipse', document_id=doc, layer_id=lid,
                       foreground=col, x=x*1.5, y=y*1.5, width=w*1.5, height=h*1.5,
                       fill_style='ForegroundColor', stroke_style='None')

            def branch(pts, width, color='#66533a'):
                # Three nested, genuinely tapered contours, with a light-facing
                # edge. The same design is retained in the editable KRA layers.
                for fraction, tint, shift in [(1, '#302e25', 0), (.76, color, -width*.1), (.19, '#a18a58', -width*.30)]:
                    contour = []
                    for side in (-1, 1):
                        for n in (range(len(pts)) if side == -1 else range(len(pts)-1, -1, -1)):
                            x, y = pts[n]
                            radius = width * fraction * (.5-.46*n/(len(pts)-1))
                            contour.append((x+side*radius+shift, y))
                    shape(tint, contour)

            def leaf(x, y, length, width, angle, color, vein=False):
                dx, dy = math.cos(angle), math.sin(angle)
                def p(a, b):return (x+dx*a-dy*b, y+dy*a+dx*b)
                # Small serrated, pointed leaves, with separate illuminated
                # planes. They read as leaves even at the normal game zoom.
                poly(color, [p(0, 0), p(length*.17, -width*.32), p(length*.29, -width*.43),
                             p(length*.37, -width*.37), p(length*.52, -width*.49),
                             p(length*.70, -width*.29), p(length, 0), p(length*.70, width*.31),
                             p(length*.53, width*.43), p(length*.39, width*.35), p(length*.23, width*.37)])
                if vein:path(mix(color, '#b0b779', .43), [p(length*.07, 0), p(length*.85, 0)], .55)

            layer('01 / Buttress roots and forest-floor litter')
            # No broad painted black ellipse: the real game casts the shadow.
            shape('#414935', [(140,418),(153,411),(175,409),(195,410),(222,417),(207,422),(177,424),(151,422)])
            for pts in [[(177,389),(164,411),(138,418)], [(181,396),(194,415),(222,420)],
                        [(176,405),(176,420),(160,426)], [(184,404),(198,420),(210,423)]]:
                branch(pts, 12 if species == 'oak' else 7, '#6c6040')
            for n in range(10):
                x, y = rng.uniform(143,219), rng.uniform(416,425)
                leaf(x, y, rng.uniform(4,8), 3, rng.uniform(-3.1,0), '#6e7047')

            layer('02 / Trunk, branch structure and forks')
            if species == 'spruce':
                trunk = [(180,418),(180,332),(176,240),(183,131),(179,35)]
                branch(trunk, 21, '#796044')
                for row in range(9):
                    y = 330-row*32; spread = 125-row*12
                    for side in (-1,1):
                        branch([(180,y+9),(180+side*spread*.53,y-9),(180+side*spread,y+12)], 7-row*.5)
                for n in range(24):
                    x=175+rng.uniform(0,8); y=rng.uniform(320,413)
                    path('#453b2d',[(x,y),(x-2,y-7),(x+1,y-14)],.7)
            else:
                if species == 'birch':
                    trunk = [(178,419),(178,349),(168,276),(177,199),(188,115),(181,62)]
                    branch(trunk, 25, '#c4c5a3')
                    branch([(170,281),(202,226),(220,175),(237,111),(236,64)],15,'#acb79a')
                    branch([(176,219),(145,184),(126,135),(101,99)],12,'#a7b397')
                    branch([(218,179),(265,156),(285,113)],7,'#adb895')
                else:
                    trunk = [(180,419),(177,367),(187,317),(179,268),(191,216),(182,154)]
                    branch(trunk, 40, '#7b6645')
                    for pts,w in [([(183,301),(144,267),(117,215),(83,178)],19),
                                  ([(179,269),(222,237),(243,187),(280,159)],18),
                                  ([(188,223),(160,186),(148,139),(124,100)],15),
                                  ([(186,231),(213,185),(208,140),(237,89)],13),
                                  ([(117,215),(78,212),(51,192)],7),
                                  ([(240,194),(276,209),(311,187)],7)]:branch(pts,w)
                for n in range(20):
                    side = -1 if n%2 else 1
                    x=rng.uniform(76,279);y=rng.uniform(85,208)
                    path('#52513a',[(x,y+24),(x+side*8,y+6),(x+side*19,y-7)],1.1)

            if species == 'spruce':
                layer('03 / Deep evergreen boughs')
                def spray(x,y,length,side,color):
                    pts=[(x,y-9),(x+side*length*.28,y-14),(x+side*length*.46,y-10),
                         (x+side*length*.59,y-11),(x+side*length*.83,y-2),(x+side*length,y+3),
                         (x+side*length*.86,y+7),(x+side*length*.81,y+3),
                         (x+side*length*.65,y+13),(x+side*length*.62,y+7),
                         (x+side*length*.43,y+16),(x+side*length*.40,y+10),
                         (x+side*length*.21,y+19),(x+side*length*.16,y+12),(x,y+15)]
                    poly(color,pts)
                for row in range(9):
                    y=334-row*32;spread=133-row*13
                    for side in (-1,1):
                        spray(180,y,spread,side,'#203e35')
                        spray(180+side*4,y-12,spread*.92,side,'#2d5741')
                layer('04 / Layered needle sprays and broken silhouette')
                for row in range(9):
                    y=329-row*32;spread=129-row*13
                    for side in (-1,1):
                        for n in range(5):
                            xx=180+side*spread*(.08+n*.165);yy=y-17+n*4+rng.uniform(-4,4)
                            length=spread*(.33+rng.random()*.12)
                            spray(xx,yy,length,side,['#376345','#426f4a','#4c7850','#365e43'][n%4])
                            path('#638459',[(xx,yy-4),(xx+side*length*.45,yy-5),(xx+side*length*.91,yy+2)],.85)
                layer('05 / Fine needles, cones and lichens')
                for n in range(135):
                    row=n%9;y=317-row*32;spread=123-row*13
                    x=180+rng.uniform(-spread,spread); y+=rng.uniform(-7,13)
                    side=1 if x>180 else -1
                    path(rng.choice(['#749166','#4a7650','#8c9d6b']),[(x-side*4,y+2),(x,y-3),(x+side*6,y-1)],.65)
                for x,y in [(90,306),(264,274),(132,243),(213,206)]:
                    oval('#675337',x,y,4,9);path('#af8b51',[(x+1,y+2),(x+2,y+6)],.7)
            else:
                layer('03 / Bark, knots and climbing moss')
                if species == 'birch':
                    for n in range(28):
                        y=411-n*7.2;center=178+(168-178)*max(0,1-abs(y-276)/80)
                        length=rng.uniform(4,12);x=center+rng.uniform(-8,4)
                        poly('#454a3c',[(x,y),(x+length,y-1),(x+length*.85,y+1),(x+1,y+2.5)])
                        if n%3==0:path('#e0d9b5',[(x-1,y-2),(x+length*.7,y-3)],1.3)
                    oval('#42483a',166,297,7,12);oval('#b3bb97',168,299,4,8)
                    for y in (358,381,402):path('#798668',[(170,y),(173,y-6),(174,y-13)],1.5)
                else:
                    for n in range(18):
                        x=166+n%6*4.5;y=410-n//6*29
                        path('#443d2d',[(x,y),(x+rng.uniform(-3,3),y-11),(x+1,y-24)],1.2)
                        if n%2==0:path('#a68b57',[(x-1,y-2),(x-1,y-13)],.75)
                    oval('#413c2c',177,326,10,17);oval('#947b4e',179,328,6,13);oval('#423d2d',181,331,3,8)
                    path('#b29b61',[(169,371),(174,355),(180,347)],1)
                for n in range(24):
                    y=rng.uniform(388,416);x=rng.uniform(165,191)
                    oval(rng.choice(['#495d3b','#64764a','#8b9158']),x,y,3+rng.random()*4,2+rng.random()*3)

                layer('04 / Backlit canopy masses')
                clusters=[]
                if species == 'birch':
                    centers=[(114,106,41,42),(157,76,38,42),(204,66,37,36),(248,89,38,36),
                             (276,124,37,37),(292,164,25,34),(253,184,39,35),(211,151,45,40),
                             (168,128,44,42),(126,161,37,35),(88,149,32,35),(78,191,26,29),
                             (129,205,38,32),(178,190,34,34),(223,214,33,31),(264,235,25,27)]
                    palette=['#31513b','#3d603e','#456c42','#517744']
                else:
                    centers=[(66,171,33,34),(87,128,43,35),(124,94,43,36),(171,77,41,37),
                             (216,78,39,38),(256,108,46,36),(285,142,34,39),(303,177,28,29),
                             (269,200,42,32),(226,194,47,39),(181,162,46,44),(137,156,43,45),
                             (99,206,37,28),(145,217,42,34),(194,216,42,30)]
                    palette=['#254835','#2b5239','#315b3c','#3b6540']
                # Contours combine broad leaf masses with uneven, fine outer
                # lobes; no repeated circles or regular polygonal crowns.
                for n,(cx,cy,rx,ry) in enumerate(centers):
                    pts=[]
                    for k in range(38):
                        angle=k*math.tau/38;radius=rng.uniform(.86,1.10)
                        pts.append((cx+math.cos(angle)*rx*radius,cy+math.sin(angle)*ry*radius))
                    poly(palette[n%4],pts);clusters.append((cx,cy,rx,ry))

                layer('05 / Lit foliage planes and branch windows')
                for n,(cx,cy,rx,ry) in enumerate(clusters):
                    col=(['#69894d','#779553','#5c8148','#63884b'] if species=='birch' else
                         ['#4e7548','#5b804c','#406e43','#527a49'])[n%4]
                    pts=[]
                    for k in range(21):
                        angle=k*math.tau/21;radius=rng.uniform(.80,1.10)
                        pts.append((cx-rx*.18+math.cos(angle)*rx*.72*radius,cy-ry*.23+math.sin(angle)*ry*.58*radius))
                    poly(col,pts)
                    for j in range(5):
                        x=cx+rng.uniform(-.65,.65)*rx;y=cy+rng.uniform(-.6,.6)*ry
                        leaf(x,y,rng.uniform(8,14),rng.uniform(5,9),rng.uniform(-3.0,-.25),mix(col,'#afaf71',rng.uniform(0,.24)),j==0)

                layer('06 / Individual leaves, twigs and dappled highlights')
                # Peripheral hanging sprays soften the outline and expose air
                # around the birch. Oak leaves are broader and more clustered.
                for n,(cx,cy,rx,ry) in enumerate(clusters):
                    for j in range(7):
                        angle=rng.uniform(0,math.tau);edge=rng.uniform(.58,.96)
                        x=cx+math.cos(angle)*rx*edge;y=cy+math.sin(angle)*ry*edge
                        col=rng.choice(['#809c5b','#94a566','#6d8e51','#526f43'] if species=='birch'
                                       else ['#6d8b50','#829958','#5b814a','#446b40'])
                        leaf(x,y,rng.uniform(7,12),rng.uniform(4,7),rng.uniform(-3,0),col,j%3==0)
                    if n%3==0:
                        y=cy+ry*.6;x=cx+rx*.4
                        path('#566045',[(x,y),(x+4,y+12),(x+1,y+21)],.7)
                        leaf(x+4,y+11,10,5,-.2,'#587a46')
                        leaf(x+2,y+18,9,5,-2.6,'#708b4e')

            layer('07 / Root moss, fern shoots and contact detail')
            for n in range(13):
                x=151+n*4.3;y=417+rng.uniform(-2,4)
                path('#6a7b48',[(x,y),(x-2,y-6),(x-4,y-8)],.85)
                if n%3==0:leaf(x,y-3,6,3,-2.4,'#87915a')

            directory=root/species
            kra=directory/('forest-'+species+'.kra');png=directory/('forest-'+species+'.png')
            c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
            c.call('krita_preview',document_id=doc,max_width=540,max_height=690)
            c.call('krita_state',document_id=doc)
            c.call('krita_document',action='close',document_id=doc,discard_changes=False)
            from PIL import Image
            with Image.open(png) as im:
                im.load();assert im.mode=='RGBA' and im.size==(540,690)
                assert im.getchannel('A').getextrema()==(0,255)
                bbox=im.getbbox();assert bbox[0]>2 and bbox[1]>2 and bbox[2]<538 and bbox[3]<688
                assert im.getpixel((270,624))[3]>160
            with zipfile.ZipFile(kra) as z:assert z.testzip() is None and 'maindoc.xml' in z.namelist()
            receipts.append({'species':species,'layers':layers,'kra':str(kra),'png':str(png),
                             'kra_sha256':hashlib.sha256(kra.read_bytes()).hexdigest(),
                             'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'alpha_bounds':bbox})
            (root/'progress.json').write_text(json.dumps(receipts,indent=2),encoding='utf-8')
    report={'status':'PASS','authoring':'Krita native MCP draw','canvas':[540,690],
            'logical_canvas':[180,230],'logical_anchor':[90,208],'source_anchor':[270,624],
            'original_art':True,'external_assets':[],'trees':receipts}
    (root/'provenance.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print('PASS: three original layered woodland trees',flush=True)


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True)
    p.add_argument('--no-audio',action='store_true',required=True)
    p.add_argument('--resume',action='store_true')
    a=p.parse_args();author(a.root,a.resume)

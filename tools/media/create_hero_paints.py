"""Original hero paint/cutout sheet, authored with the existing native Krita MCP.

No generated bitmap or external asset is imported. Pillow only extracts cells
after Krita has saved its layered master. All outputs use a new directory.
"""
from pathlib import Path
import argparse, hashlib, json, shutil, zipfile
from krita_mcp_client import KritaMCP, start_krita_if_needed

NAMES=['face','hair_front','hair_back','tunic','bracer','boot','scarf','cape',
       'body','arm','leg','sword']

def create(root, logs):
    from PIL import Image
    root=Path(root);root.mkdir(parents=True,exist_ok=False)
    kra=root/'hero-paints.kra';png=root/'hero-paints.png'
    start_krita_if_needed()
    with KritaMCP(logs) as c:
        doc=c.call('krita_document',action='create',width=512,height=384,name='Emberfall — wędrowiec / malowane części 2D')['affected_document_id']
        state=c.call('krita_state',document_id=doc)
        assert state['runtime_compatibility']['status']=='compatible'
        c.call('krita_edit',action='select_all',document_id=doc)
        c.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id'])
        c.call('krita_edit',action='deselect',document_id=doc)
        layers=[]
        def layer(name,cell):
            c.call('krita_layers',action='create',document_id=doc,name=name)
            lid=c.call('krita_state',document_id=doc)['active_layer_id']
            layers.append({'name':name,'id':lid,'cell':cell});return lid
        def poly(lid,col,pts):
            i=next(x['cell'] for x in layers if x['id']==lid);ox=i%4*128;oy=i//4*128
            c.call('krita_draw',shape='polygon',document_id=doc,layer_id=lid,points=[{'x':ox+x,'y':oy+y} for x,y in pts],foreground=col,fill_style='ForegroundColor',stroke_style='None',brush_size=1)
        def oval(lid,col,x,y,w,h):
            i=next(x['cell'] for x in layers if x['id']==lid)
            c.call('krita_draw',shape='ellipse',document_id=doc,layer_id=lid,x=i%4*128+x,y=i//4*128+y,width=w,height=h,foreground=col,fill_style='ForegroundColor',stroke_style='None')
        def path(lid,col,pts,size=2):
            i=next(x['cell'] for x in layers if x['id']==lid)
            c.call('krita_draw',shape='path',document_id=doc,layer_id=lid,points=[{'x':i%4*128+x,'y':i//4*128+y} for x,y in pts],foreground=col,fill_style='None',stroke_style='ForegroundColor',brush_size=size)
        l=layer('01 Skóra zapewniana przez głowę gry — przezroczysta baza',0)
        # Only facial features are painted. The original head supplies all
        # skin and lighting, so no face patch, contour or material seam exists.
        l=layer('02 Oczy, brwi, nos i usta',0)
        for x in [33,74]:
            path(l,'#9d795c',[(x,51),(x+10,48),(x+20,51)],1.5)
            poly(l,'#f0dfb9',[(x+2,51),(x+10,49),(x+19,52),(x+14,56),(x+5,55)])
            oval(l,'#3f6662',x+9,49,7,8);oval(l,'#192d32',x+11,50,3,6);oval(l,'#fff1ce',x+10,50,2,2)
            path(l,'#4c3b32',[(x,44),(x+9,41),(x+22,44)],3)
        path(l,'#e6c399',[(63,54),(63,68),(59,74)],2)
        path(l,'#aa8364',[(59,77),(64,79),(70,77)],1.3)
        path(l,'#895e4b',[(50,92),(63,90),(76,92)],2)
        path(l,'#f3cea0',[(55,96),(65,97),(72,95)],1.5)
        layer('03 Czyste policzki i skronie — kolor głowy',0)
        l=layer('04 Włosy — pasma z przodu',1)
        poly(l,'#293437',[(14,71),(13,38),(23,18),(43,7),(75,5),(102,17),(115,42),(110,81),(102,70),(96,39),(83,31),(69,41),(54,45),(62,28),(39,43),(27,48),(24,78),(18,90)])
        poly(l,'#503f34',[(18,40),(28,21),(47,13),(75,11),(97,22),(104,41),(83,26),(66,32),(52,37),(56,23),(35,37)])
        path(l,'#7c6146',[(24,36),(42,22),(61,18),(80,20)],2)
        path(l,'#66503c',[(30,43),(47,31),(57,29)],2)
        path(l,'#9b7851',[(43,21),(62,16),(74,17)],1.5)
        path(l,'#3d3a33',[(96,24),(106,44),(105,64)],3)
        l=layer('05 Włosy — tył i kark',2)
        poly(l,'#293437',[(18,37),(27,18),(49,10),(81,9),(102,23),(111,49),(105,87),(90,106),(81,99),(68,115),(51,104),(39,109),(24,88)])
        poly(l,'#503f34',[(22,39),(33,22),(55,15),(79,15),(98,27),(104,49),(95,70),(93,91),(78,101),(69,104),(59,95),(43,99),(32,80)])
        for x,y in [(35,31),(49,25),(65,24),(81,26),(94,37)]:path(l,'#70543e',[(x,y),(x-4,y+28),(x,y+45)],2)
        l=layer('06 Tkanina — szwy, fałdy i obszycie',3)
        # Neutral light/dark detail is tinted by the equipped material at runtime.
        for x in [18,110]:path(l,'#c3c8b6',[(x,8),(x-3 if x<64 else x+3,58),(x,119)],2)
        for x in [26,99]:
            for y in range(16,117,12):path(l,'#a6ad9b',[(x,y),(x+2,y+4)],1)
        poly(l,'#5c7570',[(32,14),(42,53),(35,83),(29,110),(36,71)])
        poly(l,'#aebdac',[(90,10),(87,48),(96,76),(103,113),(96,66)])
        path(l,'#e2d5ae',[(20,117),(39,120),(88,120),(109,117)],2)
        l=layer('07 Skórzane karwasze — szwy i sprzączki',4)
        poly(l,'#4a3b30',[(27,9),(101,9),(106,111),(96,119),(31,115),(23,109)])
        poly(l,'#805a3d',[(32,15),(91,15),(96,110),(34,108)])
        poly(l,'#aa7951',[(32,15),(45,15),(43,105),(34,105)])
        for y in [32,83]:
            poly(l,'#332f29',[(25,y),(102,y+2),(103,y+10),(25,y+8)])
            poly(l,'#c4a26a',[(73,y-2),(94,y-1),(94,y+13),(73,y+12)])
            poly(l,'#4c3b2d',[(78,y+2),(90,y+3),(90,y+9),(78,y+8)])
        for y in range(17,113,10):path(l,'#ca9b68',[(52,y),(57,y+2)],1.5)
        l=layer('08 Buty — szwy i zużycie',5)
        for x in [29,100]:path(l,'#d2b486',[(x,10),(x,85),(x+(-8 if x<64 else 8),113)],2)
        for y in [31,47,63,79]:path(l,'#5c4a38',[(42,y),(81,y+2)],3)
        path(l,'#f0d7a9',[(30,111),(59,115),(97,112)],2)
        l=layer('09 Szal — malowane fałdy',6)
        poly(l,'#243f41',[(8,39),(28,20),(92,18),(119,41),(112,82),(75,109),(37,102),(13,79)])
        poly(l,'#39706b',[(13,39),(34,27),(91,24),(111,42),(101,66),(68,86),(31,80)])
        path(l,'#71998a',[(19,43),(51,56),(83,51),(105,39)],3)
        path(l,'#28504f',[(25,64),(55,73),(89,61)],5)
        path(l,'#b8bb90',[(29,86),(60,96),(91,81)],2)
        l=layer('10 Peleryna wycinanki',7)
        poly(l,'#203f43',[(34,8),(94,8),(103,52),(116,113),(97,118),(85,113),(69,121),(52,115),(35,120),(12,112),(25,53)])
        poly(l,'#376963',[(41,12),(87,12),(89,53),(101,112),(86,106),(68,114),(53,108),(32,113),(39,54)])
        for x in [43,61,82]:path(l,'#4f8074',[(x,19),(x-3,59),(x+3,106)],2)
        path(l,'#b8ad78',[(17,110),(35,116),(53,111),(69,117),(85,109),(98,114),(111,110)],2)
        l=layer('11 Tułów wycinanki — tunika',8)
        poly(l,'#233b3d',[(25,9),(103,9),(117,33),(98,54),(98,118),(30,118),(30,54),(11,32)])
        poly(l,'#4b716b',[(29,14),(99,14),(108,32),(91,48),(91,109),(36,109),(36,48),(20,32)])
        poly(l,'#7e9581',[(29,16),(39,16),(42,68),(37,103),(36,46),(22,30)])
        path(l,'#bda97b',[(92,15),(84,52),(45,102)],7)
        poly(l,'#483a2e',[(29,96),(97,96),(98,111),(29,111)])
        poly(l,'#dbbb78',[(55,96),(72,96),(72,112),(55,112)])
        poly(l,'#463a2e',[(60,100),(68,100),(68,108),(60,108)])
        l=layer('12 Ramię wycinanki',9)
        poly(l,'#273f40',[(31,11),(91,11),(98,48),(85,64),(87,110),(70,123),(42,114),(39,65),(27,48)])
        poly(l,'#5d8276',[(37,16),(86,16),(90,45),(78,57),(45,57),(34,44)])
        poly(l,'#c19671',[(46,58),(80,58),(80,85),(46,85)])
        poly(l,'#79583e',[(42,80),(84,80),(84,103),(43,103)])
        poly(l,'#e1b486',[(44,103),(84,104),(80,117),(66,123),(45,115)])
        path(l,'#bb9b67',[(45,85),(80,85)],3)
        l=layer('13 Noga wycinanki',10)
        poly(l,'#253d3c',[(36,7),(89,7),(87,61),(83,80),(86,111),(108,114),(109,123),(33,123),(30,109),(37,77)])
        poly(l,'#46615b',[(39,11),(58,11),(53,59),(58,78),(43,78),(40,60)])
        poly(l,'#6e4b34',[(35,78),(85,78),(84,110),(106,115),(105,120),(36,120),(34,109)])
        path(l,'#b58b59',[(42,82),(43,109),(88,115)],3)
        l=layer('14 Miecz wycinanki',11)
        poly(l,'#273b44',[(62,3),(74,15),(73,88),(55,88),(56,15)])
        poly(l,'#bbd5cf',[(63,6),(70,16),(68,85),(61,85)])
        path(l,'#f3edce',[(64,12),(63,82)],2)
        poly(l,'#c5a16a',[(40,86),(87,86),(87,93),(40,93)])
        poly(l,'#563b2b',[(58,94),(69,94),(69,119),(58,119)])
        oval(l,'#d1b277',57,117,14,8)
        c.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
        c.call('krita_preview',document_id=doc,max_width=1024,max_height=768)
        state=c.call('krita_state',document_id=doc)
        c.call('krita_document',action='close',document_id=doc,discard_changes=False)
    with zipfile.ZipFile(kra) as z:assert z.testzip() is None and 'maindoc.xml' in z.namelist()
    im=Image.open(png).convert('RGBA');assert im.size==(512,384)
    parts=root/'parts';parts.mkdir()
    records=[]
    for i,name in enumerate(NAMES):
        cell=im.crop((i%4*128,i//4*128,i%4*128+128,i//4*128+128));assert cell.getbbox()
        output=parts/(name+'.png');cell.save(output)
        records.append({'name':name,'cell':i,'sha256':hashlib.sha256(output.read_bytes()).hexdigest()})
    report={'status':'PASS','authored_in':'Krita 5.3.4.1 native MCP draw tools','original_art':True,'external_assets':[],
            'kra':str(kra),'png':str(png),'layers':layers,'parts':records,'runtime_compatibility':state['runtime_compatibility'],
            'sha256':hashlib.sha256(png.read_bytes()).hexdigest()}
    (root/'provenance.json').write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8')
    return report

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--logs',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True)
    a=p.parse_args();print(json.dumps(create(a.root,a.logs),indent=2,ensure_ascii=False))

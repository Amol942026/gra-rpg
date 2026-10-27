"""Author the 36 additional equipment materials/inlays in native Krita.

Preserve all previously compiled palettes and strokes; append native exports.
"""
from pathlib import Path
import argparse,hashlib,json,re,shutil
from PIL import Image
from krita_mcp_client import KritaMCP,start_krita_if_needed
from create_equipment_paints import PATTERNS

ROOT=Path(__file__).resolve().parents[2]
CATALOG=json.loads((ROOT/'art/items/expansion/catalog.json').read_text(encoding='utf-8'))

def bake(kra,png):
    path=ROOT/'src/generated/equipment_paints.hpp';code=path.read_text(encoding='utf-8')
    enum=re.search(r'enum class Kind \{(.*?)\};',(ROOT/'src/items.hpp').read_text(encoding='utf-8'),re.S).group(1)
    kinds=[v.strip() for v in enum.split(',') if v.strip()!='Count']
    def body(name):return re.search(r'inline constexpr \w+ '+name+r'\[\d+\]=\{\n(.*?)\n\};',code,re.S).group(1).splitlines()
    paints,strokes,spans=(body(name) for name in ('paints','strokes','spans'))
    if len(paints)==len(spans)==99:
        legacy_stroke_end=max(sum(map(int,re.findall(r'\d+',row))) for row in spans[:63])
        paints,spans,strokes=paints[:63],spans[:63],strokes[:legacy_stroke_end]
    assert len(paints)==len(spans)==63 and len(kinds)==99 and kinds[63:]==[x['id'] for x in CATALOG]
    image=Image.open(png).convert('RGBA')
    for index,item in enumerate(CATALOG):
        x,y=index%8*64,index//8*64
        colors=['0x%02x%02x%02x'%image.getpixel((x+8+n*16,y+4))[:3] for n in range(4)]
        assert colors==['0x'+c for c in item['palette']]
        paints.append(' {'+','.join(colors)+'}, // '+item['id']);start=len(strokes)
        for row in range(16):
            col=0
            while col<16:
                if image.getpixel((x+17+col*2,y+25+row*2))[3]<100:col+=1;continue
                left=col;col+=1
                while col<16 and image.getpixel((x+17+col*2,y+25+row*2))[3]>=100:col+=1
                strokes.append(' {'+','.join(map(str,(left,row,col-left)))+'},')
        assert len(strokes)>start
        spans.append(' {'+str(start)+','+str(len(strokes)-start)+'},')
    for name,typ,rows in (('paints','Paint',paints),('strokes','Stroke',strokes),('spans','Span',spans)):
        code=re.sub(r'inline constexpr '+typ+' '+name+r'\[\d+\]=\{\n.*?\n\};',lambda _:f'inline constexpr {typ} {name}[{len(rows)}]={{\n'+'\n'.join(rows)+'\n};',code,count=1,flags=re.S)
    path.write_text(code,encoding='utf-8')
    report={'status':'PASS','master':str(kra),'png':str(png),'authoring':'Native Krita 5.3.4.1 via existing local MCP; original palettes and inlays; no external or paid assets','items':[x['id'] for x in CATALOG],'preserved_legacy_items':63,'runtime_textures_added':0,'png_sha256':hashlib.sha256(png.read_bytes()).hexdigest(),'generated_sha256':hashlib.sha256(path.read_bytes()).hexdigest()}
    out=ROOT/'art/items/expansion';shutil.copy2(png,out/'equipment-materials.png')
    (out/'provenance.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))

def main(args):
    source=Path(r'C:\AI2D\sprites')/args.name;kra,png=source.with_suffix('.kra'),source.with_suffix('.png')
    if args.bake_only:return bake(kra,png)
    if kra.exists() or png.exists():raise FileExistsError(source)
    start_krita_if_needed()
    with KritaMCP(args.log_directory) as client:
        try:client.call('krita_state')
        except RuntimeError as error:
            if 'NO_ACTIVE_DOCUMENT' not in str(error):raise
        doc=client.call('krita_document',action='create',width=512,height=320,name=args.name)['affected_document_id']
        state=client.call('krita_state',document_id=doc)
        assert state['runtime_compatibility']['status']=='compatible'
        client.call('krita_edit',action='select_all',document_id=doc)
        client.call('krita_edit',action='clear',document_id=doc,layer_id=state['active_layer_id'])
        client.call('krita_edit',action='deselect',document_id=doc)
        for index,item in enumerate(CATALOG):
            client.call('krita_layers',action='create',document_id=doc,name=item['id']+' / materials and inlay')
            layer=client.call('krita_state',document_id=doc)['active_layer_id'];x,y=index%8*64,index//8*64
            for n,color in enumerate(item['palette']):client.call('krita_draw',shape='rectangle',document_id=doc,layer_id=layer,x=x+n*16,y=y,width=16,height=8,foreground='#'+color,fill_style='ForegroundColor',stroke_style='None')
            for points in PATTERNS[item['pattern']]:client.call('krita_draw',shape='path',document_id=doc,layer_id=layer,points=[{'x':x+16+a*2,'y':y+24+b*2} for a,b in points],foreground='#'+item['palette'][3],brush_size=3,fill_style='None',stroke_style='ForegroundColor')
        client.call('krita_save_export',document_id=doc,kra_path=str(kra),png_path=str(png),overwrite=False)
        client.call('krita_preview',document_id=doc,max_width=768,max_height=480)
        client.call('krita_document',action='close',document_id=doc,discard_changes=False)
    bake(kra,png)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--name',required=True);p.add_argument('--log-directory',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);p.add_argument('--bake-only',action='store_true');main(p.parse_args())

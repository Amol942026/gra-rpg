"""Edit the directional imp as six named layers and export the same layers for the rig."""
import argparse
import json
from pathlib import Path
from zipfile import ZipFile

import numpy as np
from PIL import Image
from krita_mcp_client import KritaMCP,start_krita_if_needed
from wanderer_paints import write_ora
from fire_imp_paint import CELL,LABELS


def produce(cells,out):
    out.mkdir(parents=True,exist_ok=False)
    size=(CELL*8,CELL*4)
    layers=[Image.new('RGBA',size) for name in LABELS]
    retakes=[]
    for d in range(32):
        x,y=d%8*CELL,d//8*CELL
        for p,layer in enumerate(layers):
            with Image.open(cells/f'{d:02}-{p}.png') as src:
                layer.paste(src,(x,y))
                if p==3 and d in (1,2,3,5,6,7,9,10,11,13,14,15):
                    # The catchlight follows the hottest pixel of the iris on
                    # intermediate front views, below the keratin horn and brow.
                    rgba=np.asarray(src,dtype=float)
                    window=rgba[28:56,36:127]
                    score=window[...,0]+window[...,1]*1.15-window[...,2]*1.9
                    ok=(window[...,0]>125)&(window[...,1]>60)&(window[...,3]>200)
                    if ok.any():
                        row,col=np.unravel_index(np.argmax(np.where(ok,score,-1e6)),score.shape)
                        retakes.append((x+36+int(col),y+28+int(row)))
    ora=out/'chochlik.ora'
    order=(5,4,3,2,1,0)
    write_ora(ora,[(LABELS[p],layers[p]) for p in order],'Ognisty chochlik · części i obrót')
    start_krita_if_needed()
    kra=out/'chochlik.kra'
    exported=out/'cells'
    exported.mkdir()
    with KritaMCP(out/'mcp') as editor:
        doc=editor.call('krita_document',action='open',path=str(ora))['affected_document_id']
        state=editor.call('krita_state',document_id=doc)
        ids={item['name']:item['id'] for item in state['layers']}
        if state['runtime_compatibility']['status']!='compatible' or not set(LABELS)<=ids.keys():
            raise ValueError('Could not locate imp layers in Krita')
        for x,y in retakes:
            editor.call('krita_draw',shape='ellipse',document_id=doc,layer_id=ids[LABELS[3]],
                        x=x+.05,y=y+.07,width=.93,height=.83,foreground='#ffd28a',
                        fill_style='ForegroundColor',stroke_style='None')
        editor.call('krita_save_export',document_id=doc,kra_path=str(kra),
                    png_path=str(out/'preview.png'),overwrite=False)
        editor.call('krita_preview',document_id=doc,max_width=720,max_height=360)
        for p,name in enumerate(LABELS):
            dest=out/f'part-{p}.png'
            try:
                editor.call('krita_asset',action='export_layer',document_id=doc,
                            layer_id=ids[name],output_path=str(dest),crop='canvas',overwrite=False,
                            preview_max_width=160,preview_max_height=80)
            except RuntimeError as err:
                if 'BRIDGE_UNAVAILABLE' not in str(err) or not dest.is_file():raise
            with Image.open(dest) as layer:
                if layer.size!=size:raise ValueError('Unexpected page dimensions')
                for d in range(32):
                    x,y=d%8*CELL,d//8*CELL
                    im=layer.crop((x,y,x+CELL,y+CELL))
                    if not im.getchannel('A').getbbox():raise ValueError((d,p))
                    im.save(exported/f'{d:02}-{p}.png')
    with ZipFile(kra) as doc:
        assert doc.testzip() is None and 'maindoc.xml' in doc.namelist()
    (out/'receipt.json').write_text(json.dumps({'layers':6,'cutouts':192,'iris_retakes':retakes,
                                                'krita':state['runtime_compatibility']},indent=2))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cells',type=Path)
    parser.add_argument('output',type=Path)
    parser.add_argument('--no-audio',required=True,action='store_true')
    args=parser.parse_args()
    produce(args.cells.resolve(),args.output.resolve())

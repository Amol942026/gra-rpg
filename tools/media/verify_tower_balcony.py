"""Inspect native Krita masters, shipped textures and actual game captures."""
from pathlib import Path
from PIL import Image, ImageChops
import argparse, hashlib, io, json, math, zipfile


def verify(master, qa):
    checks=0;failures=[]
    def check(ok,label):
        nonlocal checks
        checks+=1
        if not ok:failures.append(label)
    repo=Path(__file__).resolve().parents[2]
    receipt=json.loads((master/'provenance.json').read_text(encoding='utf-8'))
    check(receipt['authoring']=='Krita native MCP draw' and receipt['external_assets']==[],'original local Krita authoring')
    calls=[json.loads(line) for line in (master/'logs/mcp-calls.jsonl').read_text(encoding='utf-8').splitlines()]
    for entry in receipt['documents']:
        name=entry['name'];source=master/(name+'.png');kra=master/(name+'.kra')
        check(hashlib.sha256(source.read_bytes()).hexdigest()==entry['png_sha256'],name+' native PNG hash')
        check(hashlib.sha256(kra.read_bytes()).hexdigest()==entry['kra_sha256'],name+' native KRA hash')
        check((repo/'assets/sprites'/source.name).read_bytes()==source.read_bytes(),name+' shipped texture equals native export')
        with Image.open(source) as im,zipfile.ZipFile(kra) as z:
            im.load();check(im.mode=='RGBA' and list(im.size)==entry['size'],name+' dimensions and alpha format')
            check(z.testzip() is None,name+' valid KRA archive')
            xml=z.read('maindoc.xml').decode('utf-8')
            for layer in entry['layers']:check(layer['name'] in xml,name+' native layer '+layer['name'])
            merged=Image.open(io.BytesIO(z.read('mergedimage.png'))).convert('RGBA')
            check(ImageChops.difference(im,merged).getbbox() is None,name+' KRA composition matches PNG')
            alpha=im.getchannel('A').getextrema()
            check(alpha==((255,255) if name=='tower-valley' else (0,255)),name+' correct solid/transparent background')
            check(len(im.convert('RGB').getcolors(im.width*im.height) or [])>100,name+' nonempty painted detail')
        save=next(r for r in calls if r['tool']=='krita_save_export' and r['arguments'].get('png_path')==str(source))
        owner=save['arguments']['document_id']
        check(any(r['tool']=='krita_state' and r['arguments'].get('document_id')==owner for r in calls),name+' actual state read')
        check(any(r['tool']=='krita_preview' and r['arguments'].get('document_id')==owner and r['images'] for r in calls),name+' actual native preview')
    summary=json.loads((qa/'summary.json').read_text(encoding='utf-8'))
    check(summary['status']=='PASS' and summary['failures']==0,'real collision, movement, save and floor QA pass')
    check(not summary['audio_opened'],'game audio remained closed')
    for factor in (.8,1,1.2):
        for scene in range(4):
            file=qa/f'balcony-{scene}-{int(factor*100)}.png'
            with Image.open(file) as im:
                im.load();check(im.size==(round(1440*factor),round(900*factor)),file.name+' actual game resolution')
                u=0 if scene==0 else 6.7 if scene==1 else 9.7
                v=1.9 if scene==3 else 0
                x=915+u*math.sqrt(2)*5.5;y=220+v*math.sqrt(2)*2.75
                check(845<x<1018 and 136<y<285,file.name+' hero stays inside extended minimap')
                pixel=im.convert('RGB').getpixel((round(x*factor),round(y*factor)))
                check(max(abs(a-b) for a,b in zip(pixel,(239,241,230)))<8,file.name+' real minimap draws the hero on the terrace')
                # Landscape remains visible beyond the architecture at the right.
                crop=im.crop((int(700*factor),int(300*factor),int(830*factor),int(650*factor)))
                check(len(crop.convert('RGB').getcolors(crop.width*crop.height) or [])>20,file.name+' landscape has visible content')
    timing=json.loads((qa/'render-timing.json').read_text(encoding='utf-8'))
    check(not timing['audio_opened'] and not timing['presentation_measured'],'timing records its silent offscreen scope')
    for scene in timing['scenes']:
        check(scene['frames']==120 and 0<scene['p95_ms']<1000/60,scene['name']+' measured GPU render budget for 60 Hz')
    result={'status':'FAIL' if failures else 'PASS','checks':checks,'failures':failures,
            'masters':3,'runtime_rgba_bytes':receipt['runtime_rgba_bytes'],'captures':12,'audio_opened':False}
    (qa/'verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(result));return bool(failures)


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--master',type=Path,default=Path(r'C:\AI2D\sprites\emberfall_tower_balcony_20261027_v3'))
    parser.add_argument('--qa',type=Path,required=True);parser.add_argument('--no-audio',action='store_true',required=True)
    args=parser.parse_args();raise SystemExit(verify(args.master,args.qa))

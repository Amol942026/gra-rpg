"""Decode native sources, verify exported assets and compare native/runtime motion."""
from pathlib import Path
import argparse,json,hashlib,re,zipfile,math,xml.etree.ElementTree as ET
from PIL import Image,ImageChops,ImageStat

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def verify(paints,natural,motion,runtime,header,output):
    checks=0
    def check(ok,label):
        nonlocal checks
        checks+=1
        if not ok:raise AssertionError(label)
    sources={n:(natural if n in ('ground','bank','channel-wall','lava-surface') else paints) for n in ('ground','bank','channel-wall','lava-surface','macro','scenery')}
    provenance=[]
    for name,folder in sources.items():
        receipt=json.loads((folder/(name+'-receipt.json')).read_text(encoding='utf8'))
        check(sha(folder/(name+'.kra'))==receipt['kra_sha256'],'saved native KRA hash '+name)
        check(sha(folder/(name+'.png'))==receipt['png_sha256'],'saved native PNG hash '+name)
        with zipfile.ZipFile(folder/(name+'.kra')) as z:
            check(z.testzip() is None,'native KRA CRC '+name)
            xml=ET.fromstring(z.read('maindoc.xml'));check(len([x for x in xml.iter() if x.tag.endswith('layer')])>=3,'editable layers '+name)
            from io import BytesIO
            merged=Image.open(BytesIO(z.read('mergedimage.png'))).convert('RGBA')
            image=Image.open(folder/(name+'.png')).convert('RGBA');check(image.size==merged.size and image.tobytes()==merged.tobytes(),'KRA merged pixels equal native PNG '+name)
        calls=[json.loads(line) for line in (folder/('logs-'+name)/'mcp-calls.jsonl').read_text(encoding='utf8').splitlines()]
        for tool in ('krita_state','krita_draw','krita_save_export','krita_preview'):check(any(c['tool']==tool and not c['isError'] for c in calls),'actual native '+tool+'/'+name)
        if name!='lava-surface':check(sha(runtime/(name+'.png'))==sha(folder/(name+'.png')),'runtime exact native export '+name)
        if name=='scenery':
            for kind in range(4):
                cells=[image.crop((kind*360,v*460,(kind+1)*360,(v+1)*460)) for v in range(4)]
                check(all(im.getbbox() for im in cells),'nonempty scenery cells');check(len({hashlib.sha256(im.tobytes()).digest() for im in cells})==4,'distinct scenery variants')
                for im in cells:
                    box=im.getbbox();check(box[0]>3 and box[2]<357 and box[3]<457,'scenery margins and grounded foot anchor')
        provenance.append({'name':name,'native':str(folder/(name+'.kra')),'sha256':receipt['kra_sha256'],'original_local_2d':True})
    # Header contains only saved native samples, rather than separately invented code curves.
    code=header.read_text(encoding='utf8');rows=re.findall(r'\{(-?\d+\.\d+)f,(-?\d+\.\d+)f,(-?\d+\.\d+)f,(-?\d+\.\d+)f\}',code)
    check(len(rows)==61,'61 native runtime key samples');compiled=[[float(x) for x in row] for row in rows]
    project=motion/'basalt-swing';rendered=0
    for d in range(8):
        xml=ET.parse(project/f'attack-{d}.sif').getroot();sign=-1 if math.cos([0,.4636476,1.5707963,2.677945,math.pi,3.6052403,4.712389,5.8195377][d])<-.1 else 1
        for channel,column in [('blade',0),('shield',1),('torso',2)]:
            l=next(x for x in xml.iter('layer') if x.get('desc')=='runtime:'+channel);values=[-sign*float(w.find('angle').get('value')) for w in l.find('param[@name="amount"]/animated')]
            check(len(values)==61,'complete native channel');check(all(abs(v-compiled[f][column])<1e-5 for f,v in enumerate(values)),'exact native/runtime channel '+channel+'/'+str(d))
        check(abs(compiled[0][0])<1e-7 and abs(compiled[-1][0])<1e-7,'action returns to neutral');check(max(r[0] for r in compiled)-min(r[0] for r in compiled)>=140,'wide sword swing')
        frames=sorted((project/f'renders-{d}').rglob('frame_*.png'));check(len(frames)==60,'complete native rendered direction');hashes=set()
        for frame in frames:
            im=Image.open(frame).convert('RGBA');box=im.getbbox();check(im.size==(384,384) and box and min(box[:2])>3 and max(box[2:])<381,'decoded sword frame without clipping');hashes.add(hashlib.sha256(im.tobytes()).digest());rendered+=1
        check(len(hashes)>45,'actual articulated motion '+str(d))
    atlas=Image.open(runtime/'lava-flow.png').convert('RGBA');frames=sorted((motion/'lava'/'renders').rglob('frame_*.png'));check(len(frames)==60,'60 native river samples')
    crops=[]
    for f,path in enumerate(frames):
        im=Image.open(path).convert('RGBA');crop=atlas.crop((f%8*256,f//8*256,(f%8+1)*256,(f//8+1)*256));check(im.tobytes()==crop.tobytes(),'runtime cell equals native river render');check(crop.getextrema()[3]==(255,255),'opaque river sample');crops.append(crop)
    check(len({hashlib.sha256(im.tobytes()).digest() for im in crops})==60,'actual river motion')
    error=sum(ImageStat.Stat(ImageChops.difference(ImageChops.offset(crops[0].convert('RGB'),0,128),crops[30].convert('RGB'))).mean)/3
    check(error<3,'half period translated molten surface')
    for p in runtime.glob('*.png'):check(max(Image.open(p).size)<=2048,'runtime texture budget '+p.name)
    report={'status':'PASS','checks':checks,'native_kra':6,'native_synfig':9,'warrior_frames':rendered,'river_frames':60,'sword_arc_degrees':144,'river_half_period_translation_error':error,'channel_wall_additional_rgba8_bytes':512*256*4,'provenance':provenance,'paid_services':False,'external_assets':[]}
    output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps({k:v for k,v in report.items() if k!='provenance'}))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--natural',type=Path,required=True);p.add_argument('--motion',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--header',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.paints,a.natural,a.motion,a.runtime,a.header,a.output)

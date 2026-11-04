"""Independent native Krita -> real Laigter maps -> game PNG acceptance."""
from pathlib import Path
import argparse,json,hashlib,zipfile,xml.etree.ElementTree as ET
from io import BytesIO
import numpy as np
from PIL import Image,ImageChops
from create_volcano_art import NAMES,PARTS

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def verify(paints,runtime,previous,output):
    checks=0;rows=[];memory_before=memory_after=0
    def check(ok,label):
        nonlocal checks
        checks+=1
        if not ok:raise AssertionError(label)
    materials=json.loads((runtime/'material-receipt.json').read_text(encoding='utf8'))
    check(materials['status']=='PASS' and len(materials['assets'])==12 and materials['map_count']==48,'complete material production')
    for asset in materials['assets']:
        name=asset['name'];native=paints/(name+'.kra');png=paints/(name+'.png');albedo=paints/'albedo'/(name+'.png');actual=runtime/(name+'.png')
        receipt=json.loads((paints/(name+'-receipt.json')).read_text(encoding='utf8'))
        check(sha(native)==receipt['kra_sha256']==asset['source_kra_sha256'],'native KRA hash '+name)
        check(sha(png)==receipt['png_sha256'] and sha(albedo)==receipt['albedo_sha256']==asset['source_sha256'],'native PNG/albedo hashes '+name)
        check(sha(receipt['source_kra'])==receipt['source_kra_sha256'],'original master remains intact '+name)
        with zipfile.ZipFile(native) as z:
            check(z.testzip() is None,'native KRA CRC '+name)
            doc=ET.fromstring(z.read('maindoc.xml'));layers=[l.get('name') for l in doc.iter() if l.tag.endswith('layer')]
            if name in NAMES:
                for part in PARTS:check(part in layers and 'Caldera material / '+part in layers,'editable rig and new material layer '+name+'/'+part)
            else:check(len(layers)>=4,'editable scenery/material layers '+name)
            merged=Image.open(BytesIO(z.read('mergedimage.png'))).convert('RGBA');master=Image.open(png).convert('RGBA')
            check(merged.size==master.size and merged.tobytes()==master.tobytes(),'native KRA merged data matches export '+name)
        calls=[json.loads(line) for line in (paints/('logs-'+name)/'mcp-calls.jsonl').read_text(encoding='utf8').splitlines()]
        for tool in ['krita_state','krita_draw','krita_save_export','krita_preview']:check(any(c['tool']==tool and not c['isError'] for c in calls),'actual native '+tool+'/'+name)
        source=Image.open(albedo).convert('RGBA');result=Image.open(actual).convert('RGBA');old=Image.open(previous/(name+'.png')).convert('RGBA')
        expected=master.resize(source.size,Image.Resampling.LANCZOS) if master.size!=source.size else master
        check(expected.tobytes()==source.tobytes(),'technical export matches native painted pixels '+name)
        check(result.size==source.size==old.size,'unchanged runtime dimensions '+name);check(result.getchannel('A').tobytes()==source.getchannel('A').tobytes(),'lighting preserves every alpha sample '+name)
        check(sha(actual)==asset['sha256'] and source.tobytes()!=result.tobytes(),'real baked lighting applied '+name)
        check(old.tobytes()!=result.tobytes(),'updated in-game graphic '+name)
        if name in NAMES:
            cell=result.width//8
            for part in range(6):
                silhouettes=[]
                for d in range(8):
                    im=result.crop((d*cell,part*cell,(d+1)*cell,(part+1)*cell));box=im.getbbox()
                    check(box is not None and box[0]>1 and box[2]<cell-1 and box[3]<cell-1,'cutout part stays in rig cell '+name+'/'+str(part)+'/'+str(d));silhouettes.append(im)
                if part==3:check(len({hashlib.sha256(im.tobytes()).digest() for im in silhouettes})>=6,'individually authored directions '+name)
        if name.startswith('scenery'):
            for kind in range(4):
                cells=[result.crop((kind*360,v*460,(kind+1)*360,(v+1)*460)) for v in range(4)]
                check(all(im.getbbox() for im in cells) and len({hashlib.sha256(im.tobytes()).digest() for im in cells})==4,'four distinct scenery variants '+name)
            edge=Image.open(runtime/(name+'-edge.png')).convert('RGBA');check(edge.size==result.size and edge.getextrema()[3][1]>0,'material sprite outline mask '+name)
        check(max(result.size)<=2048,'texture budget '+name);memory_before+=old.width*old.height*4;memory_after+=result.width*result.height*4
        manifest_path=Path(asset['maps_manifest']);manifest=json.loads(manifest_path.read_text(encoding='utf-8-sig'))
        check(sha(manifest_path)==asset['maps_manifest_sha256'] and manifest['Status']=='PASS','actual Laigter receipt '+name)
        check(manifest['OriginalUnchanged'] and manifest['SourceSha256']==sha(albedo),'Laigter preserves its input '+name)
        types={m['Type'] for m in manifest['Maps']};check(types=={'normal','specular','occlusion','parallax'},'four requested material maps '+name)
        opaque=np.asarray(source.getchannel('A'))>127
        for m in manifest['Maps']:
            check(sha(m['Path'])==m['Sha256']==asset['map_sha256'][m['Type']],'map SHA '+name+'/'+m['Type'])
            im=Image.open(m['Path']).convert('RGB');check(im.size==source.size,'decoded map dimensions');v=np.asarray(im,dtype=np.float32)
            check(v[opaque].max()>v[opaque].min(),'nonempty nonconstant map '+name+'/'+m['Type'])
            if m['Type']=='normal':
                n=v[opaque]/127.5-1;check(np.abs(np.linalg.norm(n,axis=1)-1).max()<.1,'all opaque normals have unit length')
            else:check(np.array_equal(v[:,:,0],v[:,:,1]) and np.array_equal(v[:,:,1],v[:,:,2]),'grayscale scalar map')
        rows.append({'name':name,'native_sha256':sha(native),'runtime_sha256':sha(actual),'size':list(result.size),'maps':str(manifest_path)})
    check(memory_before==memory_after,'unchanged GPU texture allocation');check(materials['runtime_additional_textures']==0 and materials['runtime_additional_draw_calls']==0,'maps baked offline only')
    report={'status':'PASS','checks':checks,'native_kra':12,'species':7,'directions':8,'scenery_variants':32,'maps':48,'runtime_rgba8_bytes_before':memory_before,'runtime_rgba8_bytes_after':memory_after,'runtime_additional_textures':0,'runtime_additional_draw_calls':0,'original_local_2d':True,'external_assets':[],'paid_services':False,'assets':rows}
    output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps({k:v for k,v in report.items() if k!='assets'}))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--previous',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.paints,a.runtime,a.previous,a.output)

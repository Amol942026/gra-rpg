"""Independent native gateway, loop, seamless terrain and pixel clarity QA."""
from pathlib import Path
import argparse,json,hashlib,zipfile,xml.etree.ElementTree as ET
from io import BytesIO
import numpy as np
from PIL import Image

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def verify(paints,motion,runtime,before,after,output):
    checks=0
    def check(ok,label):
        nonlocal checks
        checks+=1
        if not ok:raise AssertionError(label)
    for name in ['portal-frame','portal-core','ground','macro']:
        r=json.loads((paints/(name+'-receipt.json')).read_text());check(sha(paints/(name+'.kra'))==r['kra_sha256'],'native KRA hash '+name);check(sha(paints/(name+'.png'))==r['png_sha256'],'native PNG hash '+name)
        with zipfile.ZipFile(paints/(name+'.kra')) as z:
            check(z.testzip() is None,'KRA CRC '+name);doc=ET.fromstring(z.read('maindoc.xml'));check(len([l for l in doc.iter() if l.tag.endswith('layer')])>=3,'editable native layers '+name)
            im=Image.open(BytesIO(z.read('mergedimage.png'))).convert('RGBA');master=Image.open(paints/(name+'.png')).convert('RGBA');check(im.tobytes()==master.tobytes(),'merged native/export pixels '+name)
        calls=[json.loads(x) for x in (paints/('logs-'+name)/'mcp-calls.jsonl').read_text().splitlines()]
        for t in ['krita_state','krita_save_export','krita_preview']:check(any(c['tool']==t and not c['isError'] for c in calls),'actual native '+t+'/'+name)
        check(any(c['tool'] in ['krita_draw','krita_paint_path'] and not c['isError'] for c in calls),'actual native painting/'+name)
        if name in ['portal-frame','macro']:check(Image.open(runtime/(name+'.png')).convert('RGBA').tobytes()==master.tobytes(),'runtime pixels equal native export '+name)
        if name=='ground':check(master.crop((256,256,768,768)).tobytes()==Image.open(runtime/'ground.png').convert('RGBA').tobytes(),'runtime tile is technical centre crop of native wrapped painting')
    ground=np.asarray(Image.open(runtime/'ground.png').convert('RGB'),dtype=np.float32)
    seam_x=float(abs(ground[:,0]-ground[:,-1]).mean());seam_y=float(abs(ground[0]-ground[-1]).mean())
    check(seam_x<2 and seam_y<2,'no visible edge colour discontinuity in repeating ground')
    edges=np.concatenate([ground[:3].reshape(-1,3),ground[-3:].reshape(-1,3),ground[:,:3].reshape(-1,3),ground[:,-3:].reshape(-1,3)])
    check(abs(float(edges.mean())-float(ground[32:-32,32:-32].mean()))<5,'no dark rectangular sprite-edge/AO border on ground')
    alpha=np.asarray(Image.open(runtime/'macro.png').getchannel('A'),dtype=np.int16);gradient=max(int(abs(np.diff(alpha,axis=0)).max()),int(abs(np.diff(alpha,axis=1)).max()));check(gradient<=5,'macro uses continuous feathered transitions instead of hard plates')
    r=json.loads((motion/'receipt.json').read_text());check(sha(motion/'portal.sif')==r['native_project_sha256'],'native Synfig project hash');check(sha(runtime/'portal-loop.png')==r['atlas_sha256'],'packed runtime animation hash')
    project=ET.parse(motion/'portal.sif').getroot();rotation=next(l for l in project.iter('layer') if l.get('desc')=='runtime:rift-rotation');keys=rotation.find('param[@name="amount"]/animated');check(len(keys)==121 and float(keys[-1].find('angle').get('value'))==360,'full native rotation and virtual endpoint')
    paths=sorted((motion/'renders').rglob('frame_*.png'));check(len(paths)==120 and r['fps']==60,'120 real rendered frames at native 60 Hz');atlas=Image.open(runtime/'portal-loop.png').convert('RGBA');hashes=set()
    for f,path in enumerate(paths):
        im=Image.open(path).convert('RGBA');box=im.getbbox();check(im.size==(128,208) and box and box[0]>3 and box[2]<125,'decoded aperture without clipping');crop=atlas.crop((f%15*132+2,f//15*212+2,f%15*132+130,f//15*212+210));check(im.tobytes()==crop.tobytes(),'runtime cell equals actual Synfig render');hashes.add(hashlib.sha256(im.tobytes()).digest())
    check(len(hashes)==120 and r['loop_seam']<r['adjacent_difference']*2,'actual seamless motion rather than a static portal')
    check(max(atlas.size)<=2048,'animation atlas size budget')
    mask=Image.open(runtime/'portal-mask.png').convert('L');check(mask.size==(360,460) and mask.getpixel((180,206))==255 and mask.getpixel((8,8))==0,'opening and empty corners in interaction mask')
    old=json.loads(before.read_text(encoding='utf-8-sig'));new=json.loads(after.read_text(encoding='utf-8-sig'))
    check(old['rgb_mismatches']>0 and new['rgb_mismatches']==0 and new['max_rgb_error']==0,'real DXGI framebuffer repairs measured double-alpha loss')
    check(new['width']==new['physical_client_width'] and new['height']==new['physical_client_height'],'no hidden OS/client-resolution stretch')
    check(new['monitor_hz']==60 and new['swap_interval']==new['actual_last_sync_interval']==1,'actual monitor refresh and positive DXGI V-sync')
    check(new['hidden'] and not new['audio_opened'] and not new['foreground_fps_measured'] and new['dpi_modes']==4,'silent hidden DPI probe and truthful scope')
    report={'status':'PASS','checks':checks,'native_kra':4,'native_synfig':1,'portal_frames':120,'portal_fps':60,'ground_seam_mean_rgb':[seam_x,seam_y],'macro_max_alpha_gradient':gradient,'before_clarity':old,'after_clarity':new,'additional_portal_rgba8_bytes':360*460*8+1980*1696*4,'paid_services':False,'external_assets':[]}
    output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps({k:v for k,v in report.items() if k not in ('before_clarity','after_clarity')}))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--motion',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--before',type=Path,required=True);p.add_argument('--after',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();verify(a.paints,a.motion,a.runtime,a.before,a.after,a.output)

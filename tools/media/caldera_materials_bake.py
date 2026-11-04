"""Real local Laigter CLI maps and offline lighting for native Krita art.

Only albedo PNGs from the declared sprites root are passed to the existing
wrapper, which gives Laigter a private copy. No runtime map shader is added.
"""
from pathlib import Path
import argparse,hashlib,json,subprocess
import numpy as np
from PIL import Image,ImageFilter,ImageChops
from asset_tools import bake_lighting
from create_volcano_art import NAMES

NAMES_ALL=NAMES+['scenery','scenery-extra','ground','channel-wall','macro']
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def run(paints,maps,runtime,preset):
    maps.mkdir(parents=True,exist_ok=True);runtime.mkdir(parents=True,exist_ok=True)
    records=[]
    for name in NAMES_ALL:
        source=paints/'albedo'/(name+'.png');receipt=json.loads((paints/(name+'-receipt.json')).read_text())
        assert sha(source)==receipt['albedo_sha256']
        matches=[]
        for p in maps.rglob('maps-manifest.json'):
            m=json.loads(p.read_text(encoding='utf-8-sig'))
            if Path(m['Source']).resolve()==source.resolve() and m['SourceSha256']==sha(source) and m.get('PresetSha256')==sha(preset):matches.append(p)
        if not matches:
            result=subprocess.run(['pwsh','-NoProfile','-File',r'C:\AI2D\Tools\Generate-LaigterMaps.ps1','-ImagePath',str(source.resolve()),'-OutputRoot',str(maps.resolve()),'-PresetPath',str(preset.resolve()),'-TimeoutSeconds','180'],capture_output=True,text=True,creationflags=subprocess.CREATE_NO_WINDOW)
            (maps/(name+'-wrapper.log')).write_text(result.stdout+'\n'+result.stderr,encoding='utf8');assert result.returncode==0,(name,result.stderr)
            matches=[p for p in maps.rglob('maps-manifest.json') if json.loads(p.read_text(encoding='utf-8-sig'))['SourceSha256']==sha(source) and json.loads(p.read_text(encoding='utf-8-sig')).get('PresetSha256')==sha(preset)]
        assert len(matches)==1,(name,matches)
        manifest=matches[0];m=json.loads(manifest.read_text(encoding='utf-8-sig'));assert m['Status']=='PASS' and m['OriginalUnchanged'] and len(m['Maps'])==4
        out=runtime/(name+'.png');record=runtime/(name+'-bake.json')
        if record.exists():
            saved=json.loads(record.read_text());assert sha(out)==saved['sha256'] and sha(source)==saved['source_sha256'] and sha(manifest)==saved['maps_manifest_sha256'],'Use a fresh output folder when changing a preset';records.append(saved);print('Verified existing bake',name,flush=True);continue
        raw=runtime/(name+'-lighting.png');assert not out.exists()
        if raw.exists():raise RuntimeError('Inspect uncommitted bake before resuming: '+str(raw))
        baked=bake_lighting(source,manifest,raw)
        original=np.asarray(Image.open(source).convert('RGBA'),dtype=np.float32)/255
        lit=np.asarray(Image.open(raw).convert('RGBA'),dtype=np.float32)/255
        # Preserve the artist's colour masses while adding map-derived relief.
        rgb=original[:,:,:3]*.2+lit[:,:,:3]*.8
        # The painted incandescent palette acts as an emission mask: normal/AO
        # estimation must not dim hot eyes, fire, molten hearts or glowing cracks.
        red,green,blue=[original[:,:,i] for i in range(3)]
        emission=(red>.5)&(red>green*1.18)&(green>blue*1.16)
        rgb=np.where(emission[:,:,None],np.maximum(rgb,original[:,:,:3]),rgb)
        pixels=np.dstack((np.clip(rgb,0,1),original[:,:,3]));Image.fromarray(np.round(pixels*255).astype(np.uint8)).save(out)
        before=Image.open(source).convert('RGBA');after=Image.open(out).convert('RGBA');assert before.getchannel('A').tobytes()==after.getchannel('A').tobytes()
        assert before.tobytes()!=after.tobytes()
        if name.startswith('scenery'):
            alpha=after.getchannel('A');edge=ImageChops.subtract(alpha.filter(ImageFilter.MaxFilter(7)),alpha);outline=Image.new('RGBA',after.size,(255,255,255,0));outline.putalpha(edge);outline.save(runtime/(name+'-edge.png'))
        saved={'name':name,'status':'PASS','source':str(source),'source_sha256':sha(source),'source_kra_sha256':receipt['kra_sha256'],'path':str(out),'sha256':sha(out),'maps_manifest':str(manifest),'maps_manifest_sha256':sha(manifest),'map_types':[x['Type'] for x in m['Maps']],'map_sha256':{x['Type']:x['Sha256'] for x in m['Maps']},'size':list(after.size),'rgba8_bytes':after.width*after.height*4,'alpha_unchanged':True,'preserved_emissive_pixels':int(emission.sum()),'bake':'80% native map-derived lighting + 20% painted albedo, preserve incandescent palette','runtime_material_shader':False}
        record.write_text(json.dumps(saved,indent=2),encoding='utf8');records.append(saved);print('Laigter + baked relief PASS',name,flush=True)
    report={'status':'PASS','assets':records,'map_count':len(records)*4,'free_local_tools_only':True,'runtime_additional_textures':0,'runtime_additional_draw_calls':0}
    (runtime/'material-receipt.json').write_text(json.dumps(report,indent=2),encoding='utf8')

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--paints',type=Path,required=True);p.add_argument('--maps',type=Path,required=True);p.add_argument('--runtime',type=Path,required=True);p.add_argument('--preset',type=Path,required=True);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();run(a.paints,a.maps,a.runtime,a.preset)

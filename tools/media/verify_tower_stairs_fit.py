"""Independent image and footprint validation of the existing tower stairs."""
from pathlib import Path
import argparse, csv, json, math, re
import numpy as np
from PIL import Image

def verify(qa):
    checks=0;failures=[]
    def check(ok,label):
        nonlocal checks
        checks+=1
        if not ok:failures.append(label)
    repo=Path(__file__).resolve().parents[2]
    text=(repo/'src/tower_layout.hpp').read_text(encoding='utf-8')
    match=re.search(r'TowerStairUp\{([^}]+)\},TowerStairDown\{([^}]+)\}',text)
    bases=[tuple(float(x.strip().rstrip('f')) for x in match[n].split(',')) for n in (1,2)]
    max_edge=0
    for name,(bx,by) in zip(('upstairs','downstairs'),bases):
        with Image.open(qa/(name+'.png')) as im:
            im.load();check(im.size==(800,800),name+' native cached image dimensions')
            pixels=np.asarray(im.convert('RGBA'));y,x=np.where(pixels[:,:,3]>=210)
            dx=(bx-by)*40+(x+.5)/2-174;dy=(bx+by-24)*20+(y+.5)/2-298
            edge=(dx/(7.2*math.sqrt(2)*40))**2+(dy/(7.2*math.sqrt(2)*20))**2
            check(len(edge)>20000 and bool(np.all(edge<1)),name+' every opaque pixel fits the actual circular floor')
            check(int(x.min())>=4 and int(y.min())>=4 and int(x.max())<796 and int(y.max())<796,name+' no clipping in cached art')
            max_edge=max(max_edge,float(edge.max()))
    rows=list(csv.DictReader((qa/'footprints.tsv').open(encoding='utf-8'),delimiter='\t'))
    by_floor={}
    for row in rows:by_floor.setdefault((int(row['floor']),int(row['direction'])),[]).append((float(row['x']),float(row['y'])))
    expected={(floor,direction) for floor in range(5) for direction in (-1,1) if (direction==1 and floor<4) or (direction==-1 and floor>0)}
    check(set(by_floor)==expected,'all eight real flights on all five floors were sampled')
    max_radius=0
    for key,points in by_floor.items():
        radii=[math.hypot(x-12,y-12) for x,y in points];max_radius=max(max_radius,max(radii))
        check(len(points)>100 and max(radii)+.3<7.2,str(key)+' all geometry and hero clearance remain inside the map')
    summary=json.loads((qa/'summary.json').read_text(encoding='utf-8'))
    check(summary['status']=='PASS' and summary['failures']==0,'real movement, collision, loading and pointer checks pass')
    check(summary['motion_cases']>=28 and summary['motion_frames']==summary['motion_cases']*270,'perimeter tests actually simulate walk, sprint and roll')
    check(abs(summary['max_ground_radius']-max_radius)<.0001,'reported ground clearance matches independent vertex calculation')
    check(abs(summary['max_silhouette_ellipse']-max_edge)<.00001,'reported image contour matches independently decoded pixels')
    check(not summary['audio_opened'],'no audio device opened')
    for floor in range(1,6):
        for scale in (80,100,120):
            with Image.open(qa/f'floor-{floor}-{scale}.png') as im:
                im.load();check(im.size==(1440*scale//100,900*scale//100),f'floor {floor} at {scale}% actual render dimensions')
    for dir in (-1,1):
        for side in range(4):
            with Image.open(qa/f'contact-{dir}-{side}.png') as im:
                im.load();check(im.size==(1440,900),'physical contact view of side '+str((dir,side)))
    for log in ('qa-v31/stairs-tests.txt','qa-v28/tower-floors-tests.txt'):
        report=(qa.parent/log).read_text(encoding='utf-8')
        check('FAIL:' not in report and 'failures: 0' in report,log+' existing regression suite')
    timing=json.loads((qa/'render-timing.json').read_text(encoding='utf-8'))
    check(not timing['audio_opened'] and not timing['presentation_measured'],'timing is explicitly silent and offscreen')
    result={'status':'FAIL' if failures else 'PASS','checks':checks,'failures':failures,'flights':len(by_floor),
            'ground_vertices':len(rows),'max_ground_radius':max_radius,'max_silhouette_ellipse':max_edge,
            'renders':23,'audio_opened':False,'all_floors_within_60hz_render_budget':all(s['p95_ms']<=1000/60 for s in timing['scenes'])}
    (qa/'verification.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result));return bool(failures)

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--qa',type=Path,required=True);parser.add_argument('--no-audio',action='store_true',required=True)
    args=parser.parse_args();raise SystemExit(verify(args.qa))

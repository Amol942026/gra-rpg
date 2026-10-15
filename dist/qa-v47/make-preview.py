from pathlib import Path
from PIL import Image
import json, hashlib
root=Path(__file__).resolve().parents[2];qa=root/'dist/qa-v47'
captures=[p for p in qa.glob('capture-*') if (p/'capture.json').is_file()]
capture=max(captures,key=lambda p:p.stat().st_mtime)
meta=json.loads((capture/'capture.json').read_text())
assert meta['fps']==60 and meta['frames']==120 and meta['audio']==False
frames=[Image.open(capture/'frames'/f'frame-{i:03d}.png').convert('RGBA') for i in range(120)]
durations=[17,17,16]*40
expected=[]
for im,ms in zip(frames,durations):
 raw=im.tobytes()
 if expected and expected[-1][0]==raw:expected[-1][1]+=ms
 else:expected.append([raw,ms])
target=qa/'hero-animation-preview.png'
frames[0].save(target,save_all=True,append_images=frames[1:],duration=durations,loop=0,disposal=0,blend=0,optimize=False)
animation=Image.open(target);assert animation.n_frames==len(expected)
total=0
for i,(raw,ms) in enumerate(expected):
 animation.seek(i);assert animation.convert('RGBA').tobytes()==raw,i
 assert abs(animation.info['duration']-ms)<.1
 total+=animation.info['duration']
assert total==2000
(qa/'eight-directions-final.png').write_bytes((capture/'eight-directions.png').read_bytes())
report={'source':str(capture),'rendered_frames':120,'encoded_frames':animation.n_frames,'nominal_fps':60,'duration_ms':total,'width':900,'height':640,'lossless_frame_comparison':True,'preview_is_not_fps_benchmark':True,'audio':False,'sha256':hashlib.sha256(target.read_bytes()).hexdigest()}
(qa/'preview-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2))
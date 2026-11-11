"""Recorded CC0 foley layered by real local ComfyUI, mastered in Audacity.
No generation service, playback, tonal synthesis, or saved preference changes.
"""
from pathlib import Path
import argparse,hashlib,json,math,shutil,subprocess,wave
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
WORK=Path('C:/AI2D/audio/emberfall-steps-loot-20261111')
SOURCES=Path('C:/AI2D/audio/emberfall-physical-20261108/sources')
RATE=48000
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save(p,data):Path(p).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def write(p,x):
 p=Path(p)
 if p.exists():raise FileExistsError(p)
 with wave.open(str(p),'wb') as w:
  w.setparams((2,2,RATE,0,'NONE','not compressed'));w.writeframes(np.rint(np.clip(x,-1,32767/32768)*32768).astype('<i2').tobytes())
def read(p):
 with wave.open(str(p),'rb') as w:
  assert (w.getnchannels(),w.getsampwidth(),w.getframerate())==(2,2,RATE)
  return np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,2).astype(float)/32768
def prepare():
 WORK.mkdir(parents=True,exist_ok=False);(WORK/'layers').mkdir()
 comfy=Path.home()/'AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI'
 stage=comfy/'input/emberfall-steps-loot-20261111';stage.mkdir(exist_ok=False)
 graph={};counter=0;receipts=[];outputs=[]
 def node(kind,**inputs):
  nonlocal counter
  counter+=1;key=str(counter);graph[key]={'class_type':kind,'inputs':inputs};return [key,0]
 def layer(name,source,gain,delay=0):
  path=WORK/'layers'/(name+'.wav')
  r=subprocess.run([shutil.which('ffmpeg'),'-hide_banner','-nostdin','-v','error','-i',str(source),'-af','highpass=f=70','-ar',str(RATE),'-ac','2','-f','f32le','pipe:1'],capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW,timeout=30)
  assert r.returncode==0,r.stderr
  x=np.frombuffer(r.stdout,dtype='<f4').reshape(-1,2).astype(float);level=np.max(np.abs(x),axis=1);active=np.flatnonzero(level>level.max()*.025)
  start=max(0,int(active[0])-96);end=min(len(x),int(active[-1])+240);x=x[start:end].copy();x-=x.mean(axis=0);x*=.55/np.abs(x).max()
  fade=min(96,len(x)//5);x[:fade]*=np.linspace(0,1,fade)[:,None];x[-fade:]*=np.linspace(1,0,fade)[:,None]
  write(path,x);shutil.copy2(path,stage/path.name)
  receipts.append({'source':str(source),'source_sha256':sha(source),'layer':str(path),'sha256':sha(path),'trim_source_frames':[start,end],'license':'CC0 1.0'})
  audio=node('LoadAudio',audio='emberfall-steps-loot-20261111/'+path.name)
  audio=node('AudioAdjustVolume',audio=audio,volume=gain)
  if delay:
   silence=node('EmptyAudio',duration=delay,sample_rate=RATE,channels=2)
   audio=node('AudioConcat',audio1=silence,audio2=audio,direction='after')
  return audio
 for cue in ['step','pickup']:
  for v in range(3):
   duration=.30 if cue=='step' else .42
   mixed=node('EmptyAudio',duration=duration,sample_rate=RATE,channels=2)
   if cue=='step':
    layers=[layer(f'{cue}-{v}-earth',SOURCES/'impact/Audio'/f'footstep_grass_{v:03}.ogg',0),
            layer(f'{cue}-{v}-sole',SOURCES/'impact/Audio'/f'footstep_concrete_{v:03}.ogg',-12,.008),
            layer(f'{cue}-{v}-cloth',SOURCES/'rpg/Audio'/f'cloth{v+1}.ogg',-19,.025)]
   else:
    leather='handleSmallLeather2.ogg' if v==1 else 'handleSmallLeather.ogg'
    layers=[layer(f'{cue}-{v}-leather',SOURCES/'rpg/Audio'/leather,0),
            layer(f'{cue}-{v}-cloth',SOURCES/'rpg/Audio'/f'cloth{v+1}.ogg',-8,.018),
            layer(f'{cue}-{v}-coins',SOURCES/'rpg/Audio'/('handleCoins2.ogg' if v==1 else 'handleCoins.ogg'),-20,.055)]
   for source in layers:mixed=node('AudioMerge',audio1=mixed,audio2=source,merge_method='add')
   mixed=node('AudioEqualizer3Band',audio=mixed,low_gain_dB=-2.5,low_freq=100,mid_gain_dB=-1.5,mid_freq=2100,mid_q=.7,high_gain_dB=-3.0,high_freq=7000)
   out=node('SaveAudio',audio=mixed,filename_prefix=f'emberfall-steps-loot-20261111/{cue}-{v}')
   outputs.append({'cue':cue,'variant':v,'node':out[0],'duration':duration})
 save(WORK/'sources.json',receipts);save(WORK/'outputs.json',outputs);save(WORK/'workflow-api.json',graph)
 print(f'Prepared {len(graph)} native ComfyUI nodes, 6 cues, 18 recorded layers')
def make_reel():
 outputs=json.loads((WORK/'comfy-history.json').read_text())['outputs'];clips=[];slices=[]
 for entry in json.loads((WORK/'outputs.json').read_text()):
  files=outputs[entry['node']]['audio'];source=files[0]
  base=Path.home()/'AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI/output'
  audio=base/source.get('subfolder','')/source['filename']
  r=subprocess.run([shutil.which('ffmpeg'),'-v','error','-nostdin','-i',str(audio),'-ar',str(RATE),'-ac','2','-f','f32le','pipe:1'],capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW);assert r.returncode==0,r.stderr
  x=np.frombuffer(r.stdout,dtype='<f4').reshape(-1,2).astype(float);assert len(x)==round(entry['duration']*RATE)
  # A guard interval makes independent Audacity selections unambiguous.
  start=sum(len(c) for c in clips);clips.append(x);slices.append({**entry,'start_frame':start,'frames':len(x),'comfy_audio':str(audio),'sha256':sha(audio)})
  clips.append(np.zeros((RATE//2,2)))
 write(WORK/'comfy-reel.wav',np.concatenate(clips));save(WORK/'slices.json',slices);print('ComfyUI outputs decoded into Audacity reel')
def deliver(master):
 samples=read(master);delivery=WORK/'delivery';delivery.mkdir(exist_ok=False);report=[]
 for clip in json.loads((WORK/'slices.json').read_text()):
  x=samples[clip['start_frame']:clip['start_frame']+clip['frames']].copy();assert len(x)==clip['frames'];x-=x.mean(axis=0)
  peak=-3 if clip['cue']=='step' else -5;x*=10**(peak/20)/np.abs(x).max()
  x[:96]*=np.linspace(0,1,96)[:,None];x[-480:]*=np.linspace(1,0,480)[:,None]
  name=clip['cue']+('' if clip['variant']==0 else '-v'+str(clip['variant']+1))+'.wav';path=delivery/name;write(path,x);actual=read(path)
  report.append({'file':name,'sha256':sha(path),'bytes':path.stat().st_size,'frames':len(actual),'seconds':len(actual)/RATE,'peak_dbfs':20*math.log10(np.abs(actual).max()),'rms_dbfs':20*math.log10(np.sqrt(np.mean(actual*actual))),'dc':float(actual.mean()),'clipped_samples':int((np.abs(actual)>=.999).sum())})
 assert all(r['clipped_samples']==0 and abs(r['dc'])<.001 for r in report)
 save(WORK/'delivery-report.json',{'status':'PASS','effects':report,'recorded_foley':True,'comfyui_executed':True,'audacity_master':str(master),'audio_played':False});print('PASS: six mastered 48 kHz stereo PCM16 cues')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','reel','deliver']);p.add_argument('--master',type=Path);p.add_argument('--no-audio',required=True,action='store_true');a=p.parse_args()
 prepare() if a.action=='prepare' else make_reel() if a.action=='reel' else deliver(a.master)

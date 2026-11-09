"""Recorded CC0 foley -> offline Csound layers -> native Audacity -> runtime.
Never starts a transport or opens an audio device. Existing files are retained.
"""
from pathlib import Path
import argparse,hashlib,json,math,re,shutil,subprocess,wave
import numpy as np
WORK=Path('C:/AI2D/audio/emberfall-physical-20261108')
ROOT=Path(__file__).resolve().parents[2]
RATE=48000
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save_json(p,data):Path(p).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def write_wav(p,x):
 p=Path(p)
 if p.exists():raise FileExistsError(p)
 with wave.open(str(p),'wb') as w:
  w.setnchannels(2);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(np.rint(np.clip(x,-1,32767/32768)*32768).astype('<i2').tobytes())
def read_wav(p):
 with wave.open(str(p),'rb') as w:
  assert (w.getnchannels(),w.getsampwidth(),w.getframerate())==(2,2,RATE)
  n=w.getnframes();raw=w.readframes(n);assert len(raw)==n*4
 return np.frombuffer(raw,dtype='<i2').reshape(-1,2).astype(np.float64)/32768
def prepare():
 ff=shutil.which('ffmpeg');src=WORK/'sources';layers=WORK/'layers';layers.mkdir(exist_ok=False)
 paths={p['name']:Path(p['source']) for p in json.loads((WORK/'prepared.json').read_text())}
 for i in range(3):paths[f'impactMining_{i:03}']=src/'impact/Audio'/f'impactMining_{i:03}.ogg'
 receipts=[];sources={}
 for name,path in paths.items():
  result=subprocess.run([ff,'-hide_banner','-nostdin','-v','error','-i',str(path),'-af','volume=0.5,highpass=f=65','-ar',str(RATE),'-ac','2','-f','f32le','pipe:1'],capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW,timeout=30)
  assert result.returncode==0,result.stderr;x=np.frombuffer(result.stdout,dtype='<f4').reshape(-1,2).astype(np.float64)
  if name=='snarl':
   windows=[];n=round(.42*RATE)
   for at in range(0,len(x)-n,round(.08*RATE)):
    part=x[at:at+n];windows.append((float(np.sqrt(np.mean(part*part))),at))
   picked=[]
   for energy,at in sorted(windows,reverse=True):
    if all(abs(at-old)>RATE*1.7 for old in picked):picked.append(at)
    if len(picked)==3:break
   chunks=[(f'snarl-{i}',x[at:at+n],at/RATE) for i,at in enumerate(sorted(picked))]
  else:
   mono=np.sqrt(np.mean(x*x,axis=1));active=np.flatnonzero(mono>max(.0015,float(mono.max())*.025));assert len(active)
   at=max(0,int(active[0])-96);end=min(len(x),int(active[-1])+480);chunks=[(name,x[at:end],at/RATE)]
  for label,part,offset in chunks:
   part=part.copy();part-=part.mean(axis=0);peak=float(np.abs(part).max());part*=.5/peak
   fade=min(96,len(part)//4);part[:fade]*=np.linspace(0,1,fade)[:,None];part[-fade:]*=np.linspace(1,0,fade)[:,None]
   target=layers/(label+'.wav');write_wav(target,part);sources[label]=target
   receipts.append({'layer':label,'source':str(path),'source_sha256':sha(path),'offset':offset,'frames':len(part),'wav':str(target),'sha256':sha(target)})
 save_json(WORK/'layers.json',receipts)
 # All combat gestures use a real recorded layer. Filtered noise supports air,
 # hiss and flame, without periodic oscillators or electronic pitch sweeps.
 cues=[('swing',.24),('heavy-swing',.36),('blade-attack',.3),('enemy-attack',.3),
  ('bone-attack',.35),('chitin-attack',.28),('serpent-attack',.32),('reptile-attack',.56),
  ('wood-attack',.56),('armored-attack',.4),('beast-attack',.5),('stone-attack',.58),
  ('slime-hit',.24),('wolf-hit',.22),('bone-hit',.27),('stone-hit',.3),('chitin-hit',.25),
  ('fire-hit',.35),('enemy-fall',.65),('hurt',.28),('death',.8),('bow-shot',.26),
  ('spell',.62),('magic-attack',.55),('venom-cast',.42),('copper-wave',.7),
  ('obsidian-cleave',.4),('fire-breath',1.05)]
 score=[];slices=[];at=.25
 for cue,duration in cues:
  for v in range(3):
   swish=('swish-4','swish-6','swish-8')[v];light=('swish-10','swish-11','swish-12')[v]
   def sample(name,gain=1,delay=0,speed=1,hp=90,lp=11000):
    path=sources[name].as_posix();score.append(f'i 1 {at+delay:.6f} {max(.03,duration-delay):.6f} "{path}" {speed} {gain} {hp} {lp}')
   def impact(family='impactPunch_heavy',gain=1,delay=0,lp=6500):sample(f'{family}_{v:03}',gain,delay,lp=lp)
   def air(gain=.1,hp=700,lp=6500):score.append(f'i 2 {at:.6f} {duration} {gain} {hp} {lp}')
   if cue in ('swing','heavy-swing','blade-attack','obsidian-cleave'):
    sample(swish,1.1,speed=.82 if cue in ('heavy-swing','obsidian-cleave') else 1);sample('cloth'+str(v+1),.28)
    if cue in ('heavy-swing','obsidian-cleave'):impact('impactMetal_light',.18,.05,3300)
   elif cue in ('bone-attack','bone-hit','chitin-attack','chitin-hit'):
    impact('impactWood_light',1);sample(light,.5);sample('metalClick',.16,.035,lp=5000)
   elif cue in ('serpent-attack','venom-cast'):
    sample(swish,.55,hp=2200,lp=10000);sample(light,.35,.05);air(.24,2400,10000)
   elif cue in ('beast-attack','reptile-attack'):
    sample(f'snarl-{v}',1,speed=.94 if cue=='beast-attack' else .82,lp=5600);sample(swish,.4,.045);impact('impactSoft_medium',.2)
   elif cue=='wood-attack':
    sample(f'creak{v+1}',.9,lp=7000);sample(swish,.45,.04);impact('impactWood_medium',.3,.045)
   elif cue=='armored-attack':
    impact('impactPlate_light',.65,lp=5800);sample(swish,.75,.035);sample(f'drawKnife{v+1}',.28)
   elif cue in ('stone-attack','stone-hit','copper-wave'):
    impact('impactMining',1,lp=4200);impact('impactPunch_heavy',.5,.015,1200);sample(swish,.3,.03,speed=.8)
   elif cue in ('enemy-attack','slime-hit','wolf-hit','hurt'):
    impact('impactSoft_heavy' if cue in ('enemy-attack','slime-hit') else 'impactPunch_heavy',1,lp=7000)
    sample('handleSmallLeather' if v%2==0 else 'handleSmallLeather2',.4,.005)
   elif cue in ('enemy-fall','death'):
    impact('impactSoft_heavy',.9,lp=5000);sample('cloth'+str(v+1),.7,.045,speed=.8);impact('impactWood_medium',.35,.1,1800)
   elif cue=='bow-shot':
    impact('impactWood_light',.42,lp=4500);sample(light,.9);sample('knifeSlice2' if v%2 else 'knifeSlice',.14,lp=8000)
   else:
    sample(swish,.5,speed=.8);sample('cloth'+str(v+1),.3,lp=2600);air(.36 if cue=='fire-breath' else .2,180,4500 if cue in ('fire-breath','fire-hit') else 8000)
   slices.append({'cue':cue,'variant':v,'start_frame':round(at*RATE),'frames':round(duration*RATE)})
   at+=duration+.35
 orchestra='''sr=48000
ksmps=32
nchnls=2
0dbfs=1
seed 20261108
instr 1
 Sfile strget p4
 aL,aR diskin2 Sfile,p5,0,0
 aL butterhp aL,p7
 aR butterhp aR,p7
 aL butterlp aL,p8
 aR butterlp aR,p8
 kenv linseg 0,.002,1,p3-.009,1,.007,0
 outs aL*p6*kenv,aR*p6*kenv
endin
instr 2
 anoise rand 1
 aair butterhp anoise,p5
 aair butterlp aair,p6
 kenv linseg 0,.015,.7,p3*.2,1,p3*.6,.45,p3*.2-.015,0
 outs aair*p4*kenv,aair*p4*kenv*.97
endin
'''
 csd='<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n'+orchestra+'\n</CsInstruments>\n<CsScore>\n'+'\n'.join(score)+f'\nf 0 {at:.6f}\ne\n</CsScore>\n</CsoundSynthesizer>\n'
 assert not re.search(r'\b(poscil|oscili?|foscil|buzz|vco2)\b',csd)
 (WORK/'combat.csd').write_text(csd,encoding='utf-8');save_json(WORK/'slices.json',slices)
 print(json.dumps({'recorded_layers':len(receipts),'combat_cues':len(cues),'variants':len(slices),'reel_seconds':at,'tonal_oscillators':False}))

def deliver():
 from audio_tools import deliver_music
 reel=read_wav(json.loads((WORK/'audacity-export.json').read_text())['wav'])
 delivery=WORK/'delivery';delivery.mkdir(exist_ok=False);effects=[]
 for entry in json.loads((WORK/'slices.json').read_text()):
  start=entry['start_frame'];x=reel[start:start+entry['frames']].copy();assert len(x)==entry['frames'];x-=x.mean(axis=0)
  x*=10**(-2.2/20)/np.abs(x).max();x[:96]*=np.linspace(0,1,96)[:,None];x[-144:]*=np.linspace(1,0,144)[:,None]
  filename=entry['cue']+('' if entry['variant']==0 else f'-v{entry["variant"]+1}')+'.wav';target=delivery/filename;write_wav(target,x)
  effects.append({'file':filename,'sha256':sha(target),'frames':len(x),'peak_dbfs':20*math.log10(np.abs(x).max()),'rms_dbfs':20*math.log10(np.sqrt(np.mean(x*x))),'dc':float(x.mean()),'clipping_samples':int((np.abs(x)>=.999).sum())})
 music=[]
 for realm in ('forest','desert','volcano','tower'):
  source=Path('C:/AI2D/audio/emberfall-suite-20261108')/(realm+'-middle-cycle.wav')
  target=delivery/('echoes-of-the-ruins.wav' if realm=='forest' else realm+'.wav')
  music.append(deliver_music(source,target,target_lufs=-17 if realm=='tower' else -16,seam_ms=1.5))
 assert all(x['clipping_samples']==0 and abs(x['dc'])<.01 for x in effects)
 save_json(WORK/'delivery-report.json',{'status':'PASS','effects':effects,'music':music,'audio_played':False})
 print('PASS: recorded combat masters and four louder decoded music loops')

if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('action',choices=['prepare','deliver']);parser.add_argument('--no-audio',required=True,action='store_true');args=parser.parse_args();globals()[args.action]()

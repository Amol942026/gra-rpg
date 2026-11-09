"""Offline modern fire, portal arrival and physical bow tension.

Reuses the retained CC0 foley, never outputs real-time audio. Render the CSD
through the native Csound tool; import/normalize/save the reel in Audacity,
then supply that exported WAV to deliver. Originals are not replaced here.
"""
from pathlib import Path
import argparse,json,hashlib,math,re
import numpy as np
from refine_combat_audio import read_wav,write_wav,save_json,RATE

ROOT=Path('C:/AI2D/audio/emberfall-flame-20261108')
OLD=Path('C:/AI2D/audio/emberfall-physical-20261108')
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def prepare():
 (ROOT/'layers').mkdir(exist_ok=False)
 layers={p.stem:p for p in (OLD/'layers').glob('*.wav')}
 receipts=[]
 for label in ['swish-4','swish-6','swish-8','creak1','creak2','creak3','cloth1','cloth2','cloth3','impactSoft_heavy_000','impactSoft_heavy_001','impactSoft_heavy_002','impactWood_light_000','impactWood_light_001','impactWood_light_002']:
  source=layers[label];dest=ROOT/'layers'/(label+'.wav');dest.write_bytes(source.read_bytes());receipts.append({'source':str(source),'source_sha256':sha(source),'output':str(dest),'sha256':sha(dest)})
 for v,label in enumerate(['swish-4','swish-6','swish-8']):write_wav(ROOT/'layers'/f'reversed-{v}.wav',read_wav(layers[label])[::-1].copy())
 score=[];slices=[];at=.2
 for cue,duration in [('fireball-cast',.78),('fireball-impact',.95),('teleport-arrival',1.55),('bow-draw',1.4)]:
  for v in range(3):
   def sample(label,gain,delay=0,speed=1,hp=80,lp=9000):score.append(f'i 1 {at+delay:.6f} {duration-delay:.6f} "{(ROOT/"layers"/(label+".wav")).as_posix()}" {speed} {gain} {hp} {lp}')
   def flame(gain,hp,lp,delay=0,length=None):score.append(f'i 2 {at+delay:.6f} {length or duration-delay:.6f} {gain} {hp} {lp} {v}')
   if cue=='fireball-cast':
    sample(['swish-4','swish-6','swish-8'][v],.65,speed=.78,lp=7600);sample(f'cloth{v+1}',.22,lp=2600)
    flame(.6,90,7800);flame(.3,850,11000,.05,.52)
   elif cue=='fireball-impact':
    sample(f'impactSoft_heavy_{v:03}',.9,lp=2000);sample(['swish-4','swish-6','swish-8'][v],.35,speed=.7,lp=4200)
    flame(.9,65,6400);flame(.4,1500,12000,.03,.64)
   elif cue=='teleport-arrival':
    sample('reversed-'+str(v),.7,speed=.65,hp=350,lp=9500);sample(['swish-4','swish-6','swish-8'][v],.48,.22,speed=.65,hp=180,lp=8000)
    score.append(f'i 3 {at:.6f} {duration:.6f} {v}')
   else:
    sample(f'creak{v+1}',.85,speed=.62+v*.025,hp=110,lp=5400);sample(f'cloth{v+1}',.2,.03,speed=.72,hp=200,lp=4000)
    sample(f'impactWood_light_{v:03}',.08,.24,speed=.8,hp=120,lp=2600)
   slices.append({'cue':cue,'variant':v,'start_frame':round(at*RATE),'frames':round(duration*RATE)});at+=duration+.5
 orchestra='''sr=48000
ksmps=32
nchnls=2
0dbfs=1
seed 202611082
instr 1
 Sfile strget p4
 aL,aR diskin2 Sfile,p5,0,0
 aL butterhp aL,p7
 aR butterhp aR,p7
 aL butterlp aL,p8
 aR butterlp aR,p8
 kenv linseg 0,.004,1,p3-.024,1,.02,0
 outs aL*p6*kenv,aR*p6*kenv
endin
instr 2
 aL rand 1
 aR rand 1
 kbody linseg .04,.024,1,p3*.19,.7,p3*.56,.22,p3*.25-.024,0
 kflutter randomi .35,1,23+p7*3
 aL butterhp aL,p5
 aR butterhp aR,p5
 aL butterlp aL,p6
 aR butterlp aR,p6
 afireL=aL*kbody*(.5+.5*kflutter)*p4
 afireR=aR*kbody*(.5+.5*kflutter)*p4
 averbL,averbR reverbsc afireL,afireR,.39,6500
 outs afireL+averbL*.18,afireR+averbR*.18
endin
instr 3
 aL rand 1
 aR rand 1
 kfreq linseg 1400,.25,5200,p3-.25,1800
 aL reson aL,kfreq,1350,1
 aR reson aR,kfreq*.93,1500,1
 aL butterhp aL,180
 aR butterhp aR,180
 kenv linseg 0,.2,.48,.09,.85,.26,.32,p3-.55,0
 aL=aL*kenv*.19
 aR=aR*kenv*.19
 averbL,averbR reverbsc aL,aR,.73,8500
 outs aL+averbL*.42,aR+averbR*.42
endin
'''
 csd='<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n'+orchestra+'\n</CsInstruments>\n<CsScore>\n'+'\n'.join(score)+f'\nf 0 {at}\ne\n</CsScore>\n</CsoundSynthesizer>\n'
 assert not re.search(r'\b(poscil|oscili?|foscil|buzz|vco2)\b',csd)
 (ROOT/'effects.csd').write_text(csd,encoding='utf-8');save_json(ROOT/'slices.json',slices);save_json(ROOT/'sources.json',receipts)
 print(json.dumps({'variants':len(slices),'seconds':at,'periodic_oscillators':False}))
def deliver(path):
 reel=read_wav(path);delivery=ROOT/'delivery';delivery.mkdir(exist_ok=False);report=[]
 for e in json.loads((ROOT/'slices.json').read_text()):
  x=reel[e['start_frame']:e['start_frame']+e['frames']].copy();assert len(x)==e['frames'];x-=x.mean(axis=0)
  x*=10**(-2.5/20)/np.abs(x).max();x[:144]*=np.linspace(0,1,144)[:,None];x[-480:]*=np.linspace(1,0,480)[:,None]
  p=delivery/(e['cue']+('' if not e['variant'] else '-v'+str(e['variant']+1))+'.wav');write_wav(p,x);actual=read_wav(p)
  report.append({'file':p.name,'sha256':sha(p),'frames':len(actual),'peak_dbfs':20*math.log10(np.abs(actual).max()),'rms_dbfs':20*math.log10(np.sqrt(np.mean(actual*actual))),'dc':float(actual.mean()),'clipped_samples':int((np.abs(actual)>=.999).sum())})
 assert all(e['clipped_samples']==0 and abs(e['dc'])<.001 for e in report)
 save_json(ROOT/'delivery-report.json',{'status':'PASS','files':report,'audio_played':False});print('PASS: 12 stereo 48 kHz PCM16 variants, no clipping')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','deliver']);p.add_argument('--no-audio',action='store_true',required=True);p.add_argument('--master',type=Path);a=p.parse_args();prepare() if a.action=='prepare' else deliver(a.master)

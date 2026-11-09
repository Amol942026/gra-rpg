"""Recorded fabric, leather and earth contact for the existing Roll cue.
Offline-only, fresh masters, no tonal synthesis or playback.
"""
from pathlib import Path
import argparse,json,hashlib,shutil,subprocess,math
import numpy as np
from refine_combat_audio import RATE,write_wav,read_wav,save_json
ROOT=Path('C:/AI2D/audio/emberfall-roll-20261109')
SOURCES=Path('C:/AI2D/audio/emberfall-physical-20261108/sources')
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def prepare():
 (ROOT/'layers').mkdir(exist_ok=False);receipt=[]
 sources={}
 for v in range(3):
  sources[f'cloth-{v}']=SOURCES/'rpg/Audio'/f'cloth{v+1}.ogg'
  sources[f'earth-{v}']=SOURCES/'impact/Audio'/f'footstep_grass_{v:03}.ogg'
  sources[f'contact-{v}']=SOURCES/'impact/Audio'/f'impactSoft_medium_{v:03}.ogg'
 sources['leather']=SOURCES/'rpg/Audio/dropLeather.ogg'
 for label,source in sources.items():
  r=subprocess.run([shutil.which('ffmpeg'),'-hide_banner','-nostdin','-v','error','-i',str(source),'-af','highpass=f=80','-ar',str(RATE),'-ac','2','-f','f32le','pipe:1'],capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW,timeout=30);assert r.returncode==0,r.stderr
  x=np.frombuffer(r.stdout,dtype='<f4').reshape(-1,2).astype(float);energy=np.sqrt(np.mean(x*x,axis=1));active=np.flatnonzero(energy>energy.max()*.02);first=max(0,int(active[0])-96);last=min(len(x),int(active[-1])+240);x=x[first:last].copy();x-=x.mean(axis=0);x*=.45/np.abs(x).max();fade=min(144,len(x)//4);x[:fade]*=np.linspace(0,1,fade)[:,None];x[-fade:]*=np.linspace(1,0,fade)[:,None]
  dest=ROOT/'layers'/(label+'.wav');write_wav(dest,x);receipt.append({'source':str(source),'source_sha256':sha(source),'trim_frames':[first,last],'layer':str(dest),'sha256':sha(dest)})
 score=[];slices=[]
 for v in range(3):
  at=.2+v*.95;duration=.52
  def layer(name,delay,length,gain,speed,hp,lp):score.append(f'i 1 {at+delay:.6f} {length} "{(ROOT/"layers"/(name+".wav")).as_posix()}" {speed} {gain} {hp} {lp}')
  # Clothing folds lead; a continuous low, textured rub carries the body roll.
  layer(f'cloth-{v}',0,.32,.65,.92+v*.025,120,6900)
  layer('leather',.025,.32,.38,.87+v*.03,130,4900)
  layer(f'earth-{v}',.045,.34,.7,.64+v*.035,180,7200)
  layer(f'earth-{(v+1)%3}',.14,.27,.36,.82,250,6200)
  # Soft shoulder/hip contact, followed by the last clothing/soil scrape.
  layer(f'contact-{v}',.25,.19,.26,.86,65,1900)
  layer(f'cloth-{(v+2)%3}',.27,.25,.3,.84,210,5200)
  slices.append({'cue':'roll','variant':v,'start_frame':round(at*RATE),'frames':round(duration*RATE)})
 orchestra='''sr=48000
ksmps=32
nchnls=2
0dbfs=1
instr 1
 Sfile strget p4
 aL,aR diskin2 Sfile,p5,0,0
 aL butterhp aL,p7
 aR butterhp aR,p7
 aL butterlp aL,p8
 aR butterlp aR,p8
 kenv linseg 0,.012,1,p3-.052,.65,.04,0
 outs aL*p6*kenv,aR*p6*kenv
endin
'''
 csd='<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n'+orchestra+'\n</CsInstruments>\n<CsScore>\n'+'\n'.join(score)+'\nf 0 3.0\ne\n</CsScore>\n</CsoundSynthesizer>\n'
 (ROOT/'ground-roll.csd').write_text(csd,encoding='utf-8');save_json(ROOT/'sources.json',receipt);save_json(ROOT/'slices.json',slices);print('Prepared three 0.52-second recorded ground rolls')
def deliver(master):
 reel=read_wav(master);(ROOT/'delivery').mkdir(exist_ok=False);report=[]
 for e in json.loads((ROOT/'slices.json').read_text()):
  x=reel[e['start_frame']:e['start_frame']+e['frames']].copy();assert len(x)==24960;x-=x.mean(axis=0);x*=10**(-3/20)/np.abs(x).max();x[:144]*=np.linspace(0,1,144)[:,None];x[-480:]*=np.linspace(1,0,480)[:,None]
  target=ROOT/'delivery'/('roll'+('' if not e['variant'] else '-v'+str(e['variant']+1))+'.wav');write_wav(target,x);actual=read_wav(target)
  report.append({'file':target.name,'sha256':sha(target),'frames':len(actual),'peak_dbfs':20*math.log10(np.abs(actual).max()),'rms_dbfs':20*math.log10(np.sqrt(np.mean(actual*actual))),'dc':float(actual.mean()),'clipped_samples':int((np.abs(actual)>=.999).sum())})
 assert all(r['clipped_samples']==0 and abs(r['dc'])<.001 for r in report)
 save_json(ROOT/'delivery-report.json',{'status':'PASS','files':report,'duration':.52,'recorded_layers_only':True,'tonal_synthesis':False,'audio_played':False});print('PASS: three PCM16 48 kHz ground rolls, no clipping')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','deliver']);p.add_argument('--master',type=Path);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();prepare() if a.action=='prepare' else deliver(a.master)

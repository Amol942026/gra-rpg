"""Real bow recordings and elemental gestures, offline Csound/Audacity reel."""
from pathlib import Path
import argparse,json,hashlib,shutil,subprocess,math
import numpy as np
from refine_combat_audio import RATE,write_wav,read_wav,save_json
ROOT=Path('C:/AI2D/audio/emberfall-spells-20261109')
OLD=Path('C:/AI2D/audio/emberfall-physical-20261108/layers')
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def decode(p,filters='highpass=f=100'):
 r=subprocess.run([shutil.which('ffmpeg'),'-hide_banner','-nostdin','-v','error','-i',str(p),'-af',filters,'-ar',str(RATE),'-ac','2','-f','f32le','pipe:1'],capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW,timeout=30);assert r.returncode==0,r.stderr
 return np.frombuffer(r.stdout,dtype='<f4').reshape(-1,2).astype(float)
def prepare():
 (ROOT/'layers').mkdir(exist_ok=False);receipt=[];src=ROOT/'sources'
 inputs={'longbow':src/'bow/English Longbow Draw.wav','recurve':src/'bow/Scythian Recurve Draw.wav','flame':src/'torch.wav'}
 for v in range(3):inputs['ice-'+str(v)]=src/'magic/qubodupIceAndElectricitySpells'/f'qubodupIceDamage0{v+1}.flac'
 for v in range(2):inputs['electric-'+str(v)]=src/'magic/qubodupIceAndElectricitySpells'/f'qubodupElectricityDamage0{v+1}.flac'
 for name in ['swish-4','swish-6','swish-8','impactSoft_heavy_000','impactSoft_heavy_001','impactSoft_heavy_002','impactMining_000','impactMining_001','impactMining_002','cloth1','cloth2','cloth3']:inputs[name]=OLD/(name+'.wav')
 def store(label,x,source,offset=0):
  x=x.copy();x-=x.mean(axis=0);x*=.45/np.abs(x).max();n=min(144,len(x)//4);x[:n]*=np.linspace(0,1,n)[:,None];x[-n:]*=np.linspace(1,0,n)[:,None];p=ROOT/'layers'/(label+'.wav');write_wav(p,x);receipt.append({'layer':label,'source':str(source),'source_sha256':sha(source),'source_offset_seconds':offset,'frames':len(x),'wav':str(p),'sha256':sha(p)})
 for label,source in inputs.items():
  x=decode(source,'highpass=f=220' if label in ('longbow','recurve') else 'highpass=f=80');energy=np.sqrt(np.mean(x*x,axis=1));active=np.flatnonzero(energy>energy.max()*.025);start=max(0,int(active[0])-96);end=min(len(x),int(active[-1])+240);x=x[start:end]
  if label=='flame':
   n=round(.48*RATE);windows=[(float(np.mean(x[i:i+n]**2)),i) for i in range(0,len(x)-n,max(1,RATE//10))];selected=[]
   for _,at in sorted(windows,reverse=True):
    if all(abs(at-old)>RATE*.55 for old in selected):selected.append(at)
    if len(selected)==3:break
   assert len(selected)==3,'Need three separate flame gestures'
   for v,at in enumerate(sorted(selected)):store('flame-'+str(v),x[at:at+n],source,(start+at)/RATE)
  elif label in ('longbow','recurve'):
   # Keep the natural recording pitch. Fit its active gesture to a 1.30 s draw.
   duration=len(x)/RATE;factor=duration/1.3;filters=['highpass=f=220'];speed=factor
   while speed>2:filters.append('atempo=2');speed/=2
   while speed<.5:filters.append('atempo=0.5');speed*=2
   filters.append('atempo='+str(speed));fitted=decode(source,','.join(filters));e=np.sqrt(np.mean(fitted*fitted,axis=1));active=np.flatnonzero(e>e.max()*.025);at=max(0,int(active[0])-144);fitted=fitted[at:at+round(1.31*RATE)];store(label,fitted,source,start/RATE)
  else:store(label,x,source,start/RATE)
 score=[];slices=[];at=.2
 for cue,duration in [('bow-draw',1.4),('fireball-cast',.45),('ice-cast',.42),('ice-impact',.65),('lightning-cast',.65),('lightning-impact',.9),('spell-learn',.8)]:
  for v in range(3):
   def sample(label,gain,delay=0,speed=1,hp=80,lp=11000):score.append(f'i 1 {at+delay:.6f} {duration-delay:.6f} "{(ROOT/"layers"/(label+".wav")).as_posix()}" {speed} {gain} {hp} {lp}')
   if cue=='bow-draw':sample('longbow' if v!=1 else 'recurve',1,speed=1,hp=220,lp=10000)
   elif cue=='fireball-cast':sample('flame-'+str(v),1.1,speed=1,hp=100,lp=10500);sample(['swish-4','swish-6','swish-8'][v],.1,hp=450,lp=7500)
   elif cue=='ice-cast':sample('ice-'+str(v),.9,speed=1.12,hp=1600,lp=12500);sample(['swish-4','swish-6','swish-8'][v],.3,hp=1000,lp=11000)
   elif cue=='ice-impact':sample('ice-'+str(v),1,hp=700,lp=12500);sample('ice-'+str((v+1)%3),.27,.1,speed=1.15,hp=1800,lp=13000)
   elif cue=='lightning-cast':sample('electric-'+str(v%2),.9,hp=180,lp=11500);sample(['swish-4','swish-6','swish-8'][v],.18,hp=300,lp=8500)
   elif cue=='lightning-impact':sample('electric-'+str(v%2),1,hp=130,lp=10500);sample('impactSoft_heavy_'+f'{v:03}',.32,hp=60,lp=1200);sample('impactMining_'+f'{v:03}',.18,.03,hp=140,lp=4000)
   else:sample('ice-'+str(v),.4,speed=.8,hp=1700,lp=10000);sample('cloth'+str(v+1),.25,.03,hp=300,lp=6000)
   slices.append({'cue':cue,'variant':v,'start_frame':round(at*RATE),'frames':round(duration*RATE)});at+=duration+.3
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
 kenv linseg 0,.008,1,p3-.043,1,.035,0
 outs aL*p6*kenv,aR*p6*kenv
endin
'''
 csd='<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n'+orchestra+'\n</CsInstruments>\n<CsScore>\n'+'\n'.join(score)+f'\nf 0 {at}\ne\n</CsScore>\n</CsoundSynthesizer>\n';(ROOT/'elemental.csd').write_text(csd,encoding='utf-8');save_json(ROOT/'layers.json',receipt);save_json(ROOT/'slices.json',slices);print(json.dumps({'cue_variants':len(slices),'seconds':at,'actual_bow_draw':True,'creaking_door_samples':False,'source_layers':len(receipt)}))
def deliver(master):
 reel=read_wav(master);(ROOT/'delivery').mkdir(exist_ok=False);report=[]
 for e in json.loads((ROOT/'slices.json').read_text()):
  x=reel[e['start_frame']:e['start_frame']+e['frames']].copy();assert len(x)==e['frames'];x-=x.mean(axis=0);x*=10**(-2.8/20)/np.abs(x).max();x[:144]*=np.linspace(0,1,144)[:,None];x[-480:]*=np.linspace(1,0,480)[:,None];target=ROOT/'delivery'/(e['cue']+('' if not e['variant'] else '-v'+str(e['variant']+1))+'.wav');write_wav(target,x);final=read_wav(target)
  report.append({'file':target.name,'sha256':sha(target),'frames':len(final),'peak_dbfs':20*math.log10(np.abs(final).max()),'rms_dbfs':20*math.log10(np.sqrt(np.mean(final*final))),'dc':float(final.mean()),'clipped_samples':int((np.abs(final)>=.999).sum())})
 assert all(r['clipped_samples']==0 and abs(r['dc'])<.001 for r in report);save_json(ROOT/'delivery-report.json',{'status':'PASS','files':report,'audio_played':False});print('PASS: 21 native processed elemental and bow variants, PCM16 stereo 48 kHz')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','deliver']);p.add_argument('--master',type=Path);p.add_argument('--no-audio',action='store_true',required=True);a=p.parse_args();prepare() if a.action=='prepare' else deliver(a.master)

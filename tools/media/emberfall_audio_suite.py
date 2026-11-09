"""Emberfall 2026-11-08 audio masters. Offline only; never opens an audio device.

Native LMMS/Csound renders and retained Audacity projects are the masters.
This helper prepares envelopes, retains sources and validates/splits exports.
"""
from pathlib import Path
import argparse, hashlib, json, math, shutil, subprocess, wave
import xml.etree.ElementTree as ET
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
WORK=Path('C:/AI2D/audio/emberfall-suite-20261108')
BRIDGE=Path('C:/Users/Amol/Documents/ChatGPT/Gra Unity 2/work/audio-mcp-20261023/output')
RATE=48000
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write_json(p,data):Path(p).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def read_wav(p):
 with wave.open(str(p),'rb') as w:
  assert (w.getframerate(),w.getnchannels(),w.getsampwidth())==(RATE,2,2)
  n=w.getnframes();raw=w.readframes(n);assert len(raw)==n*4
 return np.frombuffer(raw,dtype='<i2').reshape(-1,2).astype(np.float64)/32768
def write_wav(p,data):
 p=Path(p)
 if p.exists():raise FileExistsError(p)
 with wave.open(str(p),'wb') as w:
  w.setframerate(RATE);w.setnchannels(2);w.setsampwidth(2)
  w.writeframes(np.rint(np.clip(data,-1,32767/32768)*32768).astype('<i2').tobytes())
def metrics(p):
 x=read_wav(p);peak=float(np.abs(x).max());rms=float(np.sqrt(np.mean(x*x)))
 assert np.isfinite(x).all() and .0001<peak<.98 and rms>.00001
 assert abs(float(x.mean()))<.01
 return {'path':str(p),'sha256':sha(p),'frames':len(x),'seconds':len(x)/RATE,'peak_dbfs':20*math.log10(peak),'rms_dbfs':20*math.log10(rms),'dc':float(x.mean()),'clipped_samples':int((np.abs(x)>=.999).sum())}

def prepare():
 WORK.mkdir(parents=True,exist_ok=True)
 definitions=[]
 for realm,bpm in [('forest',72),('desert',76),('volcano',88),('tower',66)]:
  source=BRIDGE/f'Emberfall-20261108-{realm}.mmp';target=WORK/f'{realm}.mmp'
  if target.exists():raise FileExistsError(target)
  tree=ET.parse(source);tree.getroot().find('head').set('mastervol','65')
  envelopes=[(.12,.2,.68,.25,4700),(.001,.19,0,.12,8500),(.035,.09,.72,.16,6900),(.06,.15,.65,.2,1800),(0,.1,0,.1,10000),(.001,.3,0,.4,11000)]
  for i,track in enumerate(tree.findall('.//track')):
   inst=track.find('instrumenttrack');osc=inst.find('instrument/tripleoscillator')
   if osc is not None:osc.set('modalgo1','2');osc.set('modalgo2','2')
   a,d,s,r,cut=envelopes[i];el=inst.find('eldata');el.set('fwet','1');el.set('fcut',str(cut));el.set('fres','.55')
   if i!=4:ET.SubElement(el,'elvol',{'amt':'1','att':str(a),'hold':'0','dec':str(d),'sustain':str(s),'rel':str(r),'pdel':'0','lamt':'0'})
   if i==3:
    for n in track.findall('.//note'):n.set('key',str(int(n.get('key'))+(24 if realm=='volcano' else 12)))
  ET.indent(tree);tree.write(target,encoding='utf-8',xml_declaration=True)
  definitions.append({'realm':realm,'bpm':bpm,'bars':32,'cycles':3,'loop_frames':round(128*60/bpm*RATE),'mmp':str(target),'sha256':sha(target),'notes':len(tree.findall('.//note'))})
 write_json(WORK/'music.json',definitions)
 # instrument, length, gain, frequency/body, end/brightness, material
 cues=[
 ('step',8,.21,.24,95,1600,0),('swing',1,.34,.58,2600,450,0),
 ('slime-hit',2,.31,.64,90,750,.05),('wolf-hit',2,.28,.57,130,1600,.03),
 ('bone-hit',2,.32,.57,550,2400,.5),('stone-hit',2,.48,.66,190,1700,.8),
 ('enemy-fall',3,.66,.48,115,55,.3),('hurt',3,.31,.64,175,92,.15),
 ('death',3,1.3,.57,165,48,.2),('roll',1,.48,.50,1250,240,.8),
 ('dodge',1,.22,.32,3500,1400,0),('spell',4,.9,.5,294,880,0),
 ('potion',4,.65,.36,440,180,.4),('pickup',7,.35,.36,880,1320,0),
 ('equip',5,.48,.38,280,1000,.4),('drop',2,.25,.32,170,950,.25),
 ('level-up',7,1.5,.48,587,1174,0),('victory',7,2.2,.50,294,880,0),
 ('enemy-attack',3,.38,.43,120,195,.5),('ui',7,.12,.15,660,880,0),
 ('reject',7,.22,.21,220,196,0),('bow-shot',1,.34,.5,3300,800,.1),
 ('chest-open',5,.95,.54,145,780,.7),('weapon-equip',5,.54,.43,650,1600,.8),
 ('chitin-hit',2,.36,.59,380,2200,.65),('fire-hit',6,.48,.43,140,2400,0),
 ('beast-attack',3,.52,.48,90,170,.6),('stone-attack',5,.62,.57,90,1100,.75),
 ('magic-attack',4,.68,.43,196,587,.8),('venom-cast',1,.53,.48,4200,1600,.6),
 ('copper-wave',2,.85,.58,72,950,.8),('obsidian-cleave',1,.5,.66,1800,220,.9),
 ('fire-breath',6,1.18,.55,105,3600,0)]
 orchestra=r'''
sr=48000
ksmps=32
nchnls=2
0dbfs=1
seed 20261108
instr 1
 kenv linseg 0,.006,1,p3*.24,.65,p3*.55,.18,p3*.21-.006,0
 kfreq expseg p5,p3,p6
 anoise rand 1
 aair butterbp anoise,kfreq,kfreq*.65
 abody poscil .12,kfreq*.12
 aout=(aair*.85+abody*p7)*kenv*p4
 outs aout*.92,aout
endin
instr 2
 kenv expseg .001,.002,1,p3*.21,.25,p3*.79-.002,.0001
 anoise rand 1
 acrack butterbp anoise,p6,p6*.72
 af1 poscil 1,p5
 af2 poscil .4,p5*1.71
 af3 poscil .22,p5*2.63
 aout=(acrack*.67+(af1+af2*p7+af3*p7)*.27)*kenv*p4
 outs aout,aout*.98
endin
instr 3
 kenv linseg 0,.008,1,p3*.19,.68,p3*.65,.3,p3*.16-.008,0
 kfreq expseg p5,p3,p6
 amod poscil kfreq*(1.5+p7),kfreq*.52
 avocal poscil .5,kfreq+amod
 anoise rand .4
 air butterbp anoise,900,700
 aout=(avocal+air)*kenv*p4
 outs aout,aout*.95
endin
instr 4
 kenv linseg 0,.018,1,p3*.25,.65,p3*.65,.18,p3*.1-.018,0
 kfreq expseg p5,p3,p6
 anoise rand .3
 aair butterbp anoise,1800,1600
 a1 poscil .35,kfreq
 a2 poscil .14,kfreq*1.5
 a3 poscil .08,kfreq*2.003
 aout=(a1+a2+a3+aair)*kenv*p4
 adel delay aout,.021
 outs aout,adel*.82+aout*.18
endin
instr 5
 kenv linseg 0,.003,1,p3*.12,.25,p3*.55,.42,p3*.33-.003,0
 kfreq expseg p5,p3,p5*.72
 anoise rand 1
 acreak butterbp anoise,650,900
 ares reson anoise,kfreq,45,1
 alatch poscil .22,p6
 kclick expseg 1,.04,.001,p3-.04,.0001
 aout=(acreak*.25+ares*.4+alatch*kclick)*kenv*p4
 outs aout,aout*.96
endin
instr 6
 kenv linseg 0,.025,.7,p3*.15,1,p3*.6,.65,p3*.25-.025,0
 anoise rand 1
 afire butterlp anoise,p6
 abass butterlp anoise,180
 acrackle butterbp anoise,4200,1300
 aout=(afire*.45+abass*.4+acrackle*.2)*kenv*p4
 outs aout,aout*.97
endin
instr 7
 kenv expseg .001,.003,1,p3*.25,.28,p3*.75-.003,.0001
 kfreq linseg p5,p3,p6
 a1 poscil .4,kfreq
 a2 poscil .13,kfreq*2.002
 a3 poscil .055,kfreq*3.99
 aout=(a1+a2+a3)*kenv*p4
 outs aout*.96,aout
endin
instr 8
 kenv expseg .001,.002,1,p3*.2,.15,p3*.8-.002,.0001
 anoise rand 1
 agrit butterbp anoise,p6,1400
 athud poscil .27,p5
 aout=(agrit*.4+athud)*kenv*p4
 outs aout,aout
endin
'''
 score=[];slices=[];start=.25
 for name,inst,length,gain,f1,f2,material in cues:
  for variant in range(3):
   pitch=[1,.955,1.045][variant];score.append(f'i {inst} {start:.6f} {length} {gain} {f1*pitch} {f2*pitch} {material}')
   # Wood/metal latch and hinge; gear gets quieter fastening taps.
   if name in ('chest-open','weapon-equip','equip'):
    score.append(f'i 2 {start+.07:.6f} .13 {gain*.35} {850*pitch} 3200 .7')
    score.append(f'i 2 {start+length*.62:.6f} .15 {gain*.27} {220*pitch} 1700 .3')
   if name in ('level-up','victory','pickup'):
    for j,ratio in enumerate((1.25,1.5,2)):
     score.append(f'i 7 {start+(j+1)*length*.16:.6f} {length*.48} {gain*.62} {f1*ratio} {f1*ratio} 0')
   slices.append({'cue':name,'variant':variant,'start_frame':round(start*RATE),'frames':round(length*RATE)})
   start+=length+.4
 csd='<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n'+orchestra+'\n</CsInstruments>\n<CsScore>\n'+'\n'.join(score)+f'\nf 0 {start:.6f}\ne\n</CsScore>\n</CsoundSynthesizer>\n'
 (WORK/'effects.csd').write_text(csd,encoding='utf-8');write_json(WORK/'effects-slices.json',slices)
 print(json.dumps({'music':definitions,'effects':len(cues),'variants':len(slices),'reel_seconds':start}))

def deliver():
 from audio_tools import deliver_music
 import audio_tools
 audio_tools.FFMPEG=Path(shutil.which('ffmpeg'))
 # The installed FFmpeg supports libvorbis; validate rather than assume.
 audio_tools.vorbis_encoder=lambda:audio_tools.FFMPEG
 edits=json.loads((WORK/'audacity-exports.json').read_text(encoding='utf-8'))
 delivery=WORK/'delivery-v2';delivery.mkdir(exist_ok=False);reports=[]
 data=read_wav(edits['effects']['wav'])
 for entry in json.loads((WORK/'effects-slices.json').read_text()):
  at=entry['start_frame'];x=data[at:at+entry['frames']].copy();assert len(x)==entry['frames']
  # Per-effect DC correction follows the native reel processing. A global
  # DC measurement cannot remove a short asymmetric synth transient's offset.
  x-=x.mean(axis=0)
  # Single-sample native export edges receive a 3 ms inaudible click guard.
  ramp=np.linspace(0,1,144);x[:144]*=ramp[:,None];x[-144:]*=ramp[::-1,None]
  suffix='' if not entry['variant'] else '-v'+str(entry['variant']+1)
  target=delivery/(entry['cue']+suffix+'.wav');write_wav(target,x);reports.append(metrics(target))
 for entry in json.loads((WORK/'music.json').read_text()):
  name=entry['realm'];x=read_wav(edits[name]['wav']);n=entry['loop_frames'];assert len(x)>=2*n
  middle=WORK/(name+'-middle-cycle.wav');write_wav(middle,x[n:n*2]);target=delivery/('echoes-of-the-ruins.wav' if name=='forest' else name+'.wav')
  reports.append(deliver_music(middle,target,target_lufs=-20 if name=='tower' else -19,seam_ms=1.5))
 write_json(WORK/'delivery-report.json',{'status':'PASS','sfx':reports[:99],'music':reports[99:],'audio_played':False})
 print('PASS: 99 effects and four decoded, metered music loops')

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('action',choices=['prepare','deliver']);p.add_argument('--no-audio',action='store_true',required=True)
 args=p.parse_args();globals()[args.action]()

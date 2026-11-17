"""Render death cries separately from the enemy material landing banks."""

import argparse
import json
import math
from pathlib import Path
import re

import numpy as np

from asset_tools import sha, write_json
from enemy_voice_audio import (BASE, CSOUND, FFMPEG, ORCHESTRA, PROFILES, RATE, REPO,
                               checked_process, read_pcm, write_pcm)
from game_import import import_pack, validate_audio


# Each death sound combines a species-specific voice/resonance contour and
# supporting foley. Pitch rises on the initial gasp and breaks as it falls.
# (seasoned foley, rate, level, delay, usable duration, high pass, low pass).
def foley(name, speed, gain, pause, seconds, high=80, low=5200):
    return (name, speed, gain, pause, seconds, high, low)


# duration, gain in dBFS, pitch at gasp/crest/release, formants, roughness,
# turbulent breath amplitude, breath band, physical detail
BANK = {
    "slime": (.63,-4.7,(88,114,38),(310,860),34,.51,(240,2240),
              [foley("handleSmallLeather2",.75,.26,.015,.30,140,1930)]),
    "wolf": (.88,-3.5,(265,406,75),(650,1450),30,.27,(780,5250),
             [foley("snarl-2",1.13,.65,.008,.27,145,4900),foley("snarl-1",.73,.38,.26,.39)]),
    "skeleton": (.68,-4.4,(146,169,51),(700,2060),50,.57,(650,6040),
                 [foley("impactWood_light_002",1.7,.51,.015,.16,560),
                  foley("impactWood_light_000",1.48,.36,.139,.18,390),
                  foley("impactWood_light_001",1.66,.19,.262,.13,510)]),
    "stone-guardian": (1.04,-3.6,(54,66,23),(260,880),25,.60,(95,3200),
                       [foley("creak2",.69,.50,.016,.45,140,3000),foley("impactMining_002",.81,.34,.18,.46)]),
    "boar": (.94,-3.5,(156,248,47),(470,1150),49,.34,(400,4040),
             [foley("snarl-0",.80,.67,.009,.34,80,2910),foley("snarl-1",.69,.27,.235,.48,90,1860)]),
    "forest-spider": (.61,-4.5,(298,425,91),(935,2880),72,.66,(1200,6520),
                      [foley("impactWood_light_000",1.8,.38,.031,.10,940)]),
    "forest-bandit": (.91,-3.4,(162,241,64),(700,1370),25,.51,(330,3900),
                      [foley("cloth2",.84,.19,.16,.37,420)]),
    "treant": (.93,-4.2,(65,108,27),(365,925),33,.52,(180,4290),
               [foley("creak3",.60,.66,0,.68,110,3860),foley("chop",.73,.21,.24,.24)]),
    "amber-scorpion": (.68,-4.5,(180,281,71),(1260,2670),61,.48,(790,4990),
                       [foley("impactWood_medium_001",1.60,.40,.059,.14,370),
                        foley("handleSmallLeather",1.14,.27,.088,.32,210,2660)]),
    "copper-scarab": (.77,-4.0,(147,213,57),(870,2470),88,.48,(730,4600),
                      [foley("impactPlate_light_002",.73,.43,.056,.27,360),
                       foley("metalClick",.88,.21,.207,.18,850)]),
    "sand-cobra": (.85,-4.1,(228,301,51),(1440,3280),57,.85,(1480,7600),
                   [foley("swish-1",.78,.38,.021,.45,1000,5860)]),
    "living-dune": (1.17,-3.8,(49,62,23),(282,610),14,.84,(220,4840),
                    [foley("earth-2",.61,.42,.043,.47),foley("swish-11",.68,.48,.019,.53,255)]),
    "slag-crawler": (.70,-4.2,(98,151,41),(582,1730),46,.57,(510,5620),
                     [foley("impactMining_002",1.28,.43,.047,.22,235),
                      foley("impactMining_001",1.19,.20,.197,.20,380)]),
    "lava-salamander": (.88,-3.9,(117,175,43),(685,1640),52,.52,(780,6000),
                        [foley("snarl-2",.83,.52,.018,.33,160,3650)]),
    "fire-imp": (.86,-3.4,(335,525,115),(998,2460),46,.46,(820,6550),
                 [foley("swish-9",1.14,.40,.034,.39,640,6510)]),
    "ash-wraith": (1.19,-3.7,(190,295,45),(640,1920),15,.80,(520,5900),
                   [foley("cloth3",.66,.34,.04,.50,460),foley("swish-12",.65,.34,.22,.63,380)]),
    "obsidian-sentinel": (1.03,-3.8,(68,107,27),(289,1300),40,.63,(440,5560),
                          [foley("impactMetal_heavy_001",.68,.46,.093,.43,260),
                           foley("impactMining_000",.75,.32,.247,.43,145)]),
    "ember-drake": (1.12,-3.5,(98,162,31),(410,1090),40,.62,(460,5420),
                    [foley("snarl-0",.61,.64,.011,.62,74,3690),
                     foley("swish-8",.59,.30,.12,.51,230)]),
    "varkhor": (1.43,-3.2,(57,82,23),(269,856),28,.66,(200,5200),
                 [foley("snarl-1",.57,.46,.028,.68,90,2900),
                  foley("creak3",.68,.31,.15,.61,190,2920),
                  foley("impactMining_000",.62,.32,.305,.58,175)]),
    "sporekin": (.82,-4.2,(119,158,35),(445,1040),52,.62,(380,4700),
                 [foley("ace-sporekin-fall",1.01,.36,.022,.40,280),
                  foley("handleSmallLeather2",.84,.30,.081,.32,160,2700)]),
    "dune-eye": (.90,-4.2,(126,266,42),(647,1520),37,.66,(490,5650),
                 [foley("ace-dune-eye-fall",.92,.36,.041,.45,355),
                  foley("earth-0",.72,.24,.12,.33)]),
    "cinderhorn": (1.03,-3.6,(83,132,30),(407,940),42,.54,(400,4860),
                   [foley("snarl-1",.72,.58,.008,.55,75,3330),
                    foley("ace-cinderhorn-fall",.93,.37,.058,.52,155)])
}

VOCAL_ORCHESTRA = """
instr 4
 kFund expseg p5,p3*.17,p6,p3*.19,p6*.83,p3*.31,p5*.83,p3*.33,p7
 kInstability randomi .89,1.12,p10+7
 kBreak oscili .26,p10,1
 aPhonation vco2 .46,kFund*kInstability,0
 aSubharmonic vco2 .18,kFund*kInstability*.5,0
 aAspirate rand .44
 aGlottis = aPhonation*(.70+kBreak)+aSubharmonic*.75+aAspirate*.46
 kFirst expseg p8*.81,p3*.19,p8*1.13,p3*.81,p8*.68
 kSecond expseg p9*1.11,p3*.28,p9*.95,p3*.72,p9*.76
 aThroat reson aGlottis,kFirst,max(110,p8*.52),1
 aMouth reson aGlottis,kSecond,max(170,p9*.34),1
 aChest butterlp aGlottis,395
 aCry = tanh((aThroat*1.25+aMouth*.87+aChest*.75)*2)
 kEnergy expseg .002,.018,1,p3*.26,.76,p3*.25,.44,p3*.31,.13,p3*.18-.018,.002
 outs aCry*kEnergy*p4,(aCry*.97+aMouth*.035)*kEnergy*p4
endin
"""


def source_catalog(model_sources):
    src={}
    physical=BASE/"emberfall-physical-20261108"
    for row in json.loads((physical/"layers.json").read_text(encoding="utf-8")):
        src[row["layer"]]=(row["wav"],row["source"],row["sha256"],row["source_sha256"],"CC0 1.0")
    for row in json.loads((BASE/"emberfall-roll-20261109/sources.json").read_text(encoding="utf-8")):
        name=Path(row["layer"]).stem
        if name.startswith("earth-"):
            src[name]=(row["layer"],row["source"],row["sha256"],row["source_sha256"],"CC0 1.0")
    for name, row in json.loads(model_sources.read_text(encoding="utf-8")).items():
        if name.startswith("ace-"):
            src[name]=(row["layer"],row["source"],row["layer_sha256"],row["sha256"],row["license"])
    return src


def render(root, model_sources):
    root.mkdir(parents=True,exist_ok=False)
    assert tuple(BANK)==tuple(PROFILES)
    catalog=source_catalog(model_sources)
    sources={}
    cuts=[]
    start=.16
    notes=["f 1 0 4096 10 1"]
    for take in range(3):
        for index,(species,profile) in enumerate(BANK.items()):
            duration,target,pitch,formant,roughness,breath,band,layers=profile
            rate=(1,.92,1.09)[take]
            shift=(1,1.09,.94)[take]
            volume=.29 if breath>=.60 else .40
            if species in ("forest-bandit","wolf","fire-imp"):
                volume=.53
            notes.append(" ".join(map(str,["i 4",round(start,6),round(duration*.91,6),volume,
                                          *(round(x*rate,3) for x in pitch),*(round(x*shift,3) for x in formant),
                                          roughness+take*3])))
            notes.append(" ".join(map(str,["i 2",round(start+.003,6),duration-.025,breath*.43,
                                          band[0]*shift,band[1]*shift,.028+(index%4)*.004,roughness])))
            for stem,speed,weight,delay,seconds,high,low in layers:
                if stem not in sources:
                    layer,original,layer_sha,original_sha,license=catalog[stem]
                    if sha(Path(layer))!=layer_sha or sha(Path(original))!=original_sha:
                        raise ValueError(f"Sound source no longer matches its receipt: {stem}")
                    sources[stem]={"layer":layer,"original":original,"layer_sha256":layer_sha,
                                   "sha256":original_sha,"license":license}
                layer=Path(sources[stem]["layer"]).as_posix()
                notes.append(" ".join(map(str,["i 1",round(start+delay,6),seconds,f'"{layer}"',
                                              speed*rate,weight*.60,high,low*shift])))
            name=species+"-dying"+(f"-v{take+1}" if take else "")
            cuts.append(dict(name=name,species=species,offset=round(start*RATE),
                             frames=round(duration*RATE),target=target))
            start+=duration+.093
    csd=root/"dying.csd"
    csd.write_text("<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n"+
                   ORCHESTRA.replace("seed 261117","seed 496701")+VOCAL_ORCHESTRA+
                   "\n</CsInstruments>\n<CsScore>\n"+"\n".join(notes)+f"\nf 0 {start:.6f}\ne\n</CsScore>\n</CsoundSynthesizer>\n",
                   encoding="utf-8")
    reel=root/"unmastered-reel.wav"
    checked_process([CSOUND,"-+rtaudio=null","-+rtmidi=null","-W","-s","-d","-o",reel,csd],root/"csound.log")
    pcm=read_pcm(reel)
    if np.max(np.abs(pcm))>.96:
        raise ValueError("Raw render ran out of headroom")
    write_json(root/"cuts.json",cuts)
    write_json(root/"sources.json",sources)
    print(len(cuts),round(len(pcm)/RATE,4))


def deliver(root,reel):
    captures=json.loads((root/"cuts.json").read_text(encoding="utf-8"))
    refined=read_pcm(reel)
    if refined.shape!=read_pcm(root/"unmastered-reel.wav").shape:
        raise ValueError("Mastering changed clip timing")
    registered=set(re.findall(r'"([a-z-]+-dying)"',(REPO/"src/audio.hpp").read_text(encoding="utf-8")))
    assert registered=={name+"-dying" for name in BANK}
    out=root/"delivery"
    out.mkdir(exist_ok=False)
    manifest=[]
    measured=[]
    parts=[]
    for row in captures:
        speech=refined[row["offset"]:row["offset"]+row["frames"]].copy()
        speech-=speech.mean(axis=0)
        peak=np.max(np.abs(speech))
        assert peak>.007,row["name"]
        # Reinforce the body of the cry, with enough reserve for other voices.
        speech=np.tanh(speech*(1.8/peak))
        speech[:96]*=np.linspace(0,1,96)[:,None]
        fade=int(.033*RATE)
        speech[-fade:]*=np.linspace(1,0,fade)[:,None]
        speech*=10**(row["target"]/20)/np.max(np.abs(speech))
        path=out/(row["name"]+".wav")
        write_pcm(path,speech)
        validate_audio(path)
        speech=read_pcm(path)
        peak=float(np.max(np.abs(speech)))
        rms=float(np.sqrt(np.mean(speech**2)))
        first=np.flatnonzero(np.max(np.abs(speech),axis=1)>peak*.09)[0]
        block=int(.028*RATE)
        block_levels=np.sqrt(np.mean(speech[:len(speech)//block*block].reshape(-1,block,2)**2,axis=(1,2)))
        active=float(np.sum(block_levels>.035))*.028
        if not (.46<peak<.71 and rms>.068 and first<.03*RATE and active>=.22):
            raise ValueError(f"Unclear death sound for {row['name']}: {peak=}, {rms=}, {first=}, {active=}")
        checksum=sha(path)
        measured.append({**row,"duration":round(len(speech)/RATE,3),"onset_ms":round(first/48,2),
                         "active_seconds":round(active,3),"rms_dbfs":round(20*math.log10(rms),2),
                         "peak_dbfs":round(20*math.log10(peak),2),"sha256":checksum})
        manifest.append(dict(source=str(path),path=path.name,role="enemy-dying",sha256=checksum))
        parts.extend((speech,np.zeros((int(.04*RATE),2),dtype=np.float32)))
    assert len(manifest)==66 and len({m["sha256"] for m in manifest})==66
    combined=root/"delivery-reel.wav"
    write_pcm(combined,np.concatenate(parts))
    log=root/"true-peak.log"
    checked_process([FFMPEG,"-hide_banner","-nostdin","-i",combined,"-af","ebur128=peak=true","-f","null","-"],log)
    analysis=log.read_text(encoding="utf-8",errors="replace").split("Summary:")[-1]
    true_peak=float(re.search(r"Peak:\s*([-.\d]+)\s*dBFS",analysis).group(1))
    assert -7<true_peak<-2
    write_json(root/"measurements.json",{"true_peak_dbtp":true_peak,"variants":measured})
    write_json(root/"manifest.json",{"schema":1,"id":root.name,"files":manifest,"provenance":{
        "session":str(root/"dying.csd"),"sources":str(root/"sources.json"),
        "measurements":str(root/"measurements.json"),"master":str(reel)}})
    print(import_pack(root/"manifest.json")["status"],len(manifest),true_peak)


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action",choices=("render","deliver"))
    parser.add_argument("root",type=Path)
    parser.add_argument("source",type=Path,help="Verified prior source receipt for render, 48 kHz mastering reel for deliver")
    parser.add_argument("--no-audio",action="store_true",required=True)
    args=parser.parse_args()
    if args.action=="render":
        render(args.root.resolve(),args.source.resolve())
    else:
        deliver(args.root.resolve(),args.source.resolve())


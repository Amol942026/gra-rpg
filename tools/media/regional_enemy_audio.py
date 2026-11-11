"""Cut ACE-Step foley and local CC0 transients into a silent Audacity reel."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import shutil
import subprocess
import urllib.request
import wave

import numpy as np


RATE=48000
COMFY=Path.home()/"AppData/Local/Comfy-Desktop/ComfyUI-Installs/Amol/ComfyUI"
FOLEY=Path("C:/AI2D/audio/emberfall-physical-20261108/sources")
MATERIAL={
    ("sporekin","attack"):("rpg/Audio/cloth3.ogg",.46,.42),
    ("sporekin","hit"):("impact/Audio/impactWood_light_001.ogg",.25,.51),
    ("sporekin","fall"):("impact/Audio/footstep_grass_003.ogg",.55,.43),
    ("dune-eye","attack"):("rpg/Audio/clothBelt2.ogg",.52,.35),
    ("dune-eye","hit"):("impact/Audio/impactWood_medium_002.ogg",.24,.48),
    ("dune-eye","fall"):("impact/Audio/impactWood_heavy_003.ogg",.64,.44),
    ("cinderhorn","attack"):("impact/Audio/impactGeneric_light_002.ogg",.50,.42),
    ("cinderhorn","hit"):("impact/Audio/impactMetal_heavy_003.ogg",.28,.40),
    ("cinderhorn","fall"):("impact/Audio/impactWood_heavy_001.ogg",.67,.46),
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(path,highpass):
    ffmpeg=shutil.which("ffmpeg")
    out=subprocess.run([ffmpeg,"-v","error","-nostdin","-i",str(path),"-af",f"highpass=f={highpass}",
                        "-ar",str(RATE),"-ac","2","-f","f32le","pipe:1"],
                       capture_output=True,check=True,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
    return np.frombuffer(out.stdout,dtype="<f4").reshape(-1,2).astype(float)


def write_wave(path,samples):
    assert np.all(np.isfinite(samples)) and np.max(np.abs(samples))<1
    with path.open("xb") as stream,wave.open(stream,"wb") as wav:
        wav.setparams((2,2,RATE,0,"NONE","not compressed"))
        wav.writeframes(np.rint(samples*32767).astype("<i2").tobytes())


def peak(samples):
    return float(np.max(np.abs(samples)))


def faded(samples,begin=96,end=1104):
    samples[:begin]*=np.linspace(0,1,begin)[:,None]
    samples[-end:]*=np.linspace(1,0,end)[:,None]
    return samples


def cut_event(raw,length,alternate=0):
    # Pick a strong rise in energy, prioritizing an audible onset over any
    # diffuse tail or constant texture in a five-second model render.
    hop=240
    count=len(raw)//hop
    if count<8:
        raise ValueError("Empty generated effect")
    blocks=raw[:count*hop].reshape(count,hop,2)
    level=np.sqrt(np.mean(blocks*blocks,axis=(1,2)))
    floor=np.pad(level,(5,0))[:count]
    energy=np.convolve(level,np.full(9,1/9),mode="same")
    score=(np.maximum(0,level-floor)*.65+.35*energy)
    usable=score[2:max(3,count-length//hop)].copy()
    onset=int(np.argmax(usable))+2
    for _ in range(alternate):
        radius=max(12,length//hop)
        center=onset-2
        usable[max(0,center-radius):center+radius+1]=-1
        if np.max(usable)<.001:
            break
        onset=int(np.argmax(usable))+2
    lower=max(0,(onset-2)*hop)
    return raw[lower:lower+length],lower


def variant(samples,rate,length):
    at=np.arange(length)*rate
    return np.stack([np.interp(at,np.arange(len(samples)),samples[:,ch],right=0) for ch in range(2)],axis=1)


def assemble(run,work,prompt):
    with urllib.request.urlopen(f"http://127.0.0.1:8188/history/{prompt}",timeout=15) as response:
        history=json.load(response)[prompt]
    if history["status"]["status_str"]!="success":
        raise RuntimeError(history["status"])
    work.mkdir(exist_ok=False)
    (work/"comfy-history.json").write_text(json.dumps(history,indent=2),encoding="utf-8")
    clips=[]
    slices=[]
    position=0
    sources=json.loads((run/"audio-long-outputs.json").read_text(encoding="utf-8"))
    raw_folder=work/"model-outputs"
    raw_folder.mkdir()
    candidates={}
    for item in sources:
        output=history["outputs"][item["node"]]["audio"][0]
        name=item["species"]+"-"+item["cue"]
        raw=raw_folder/(name+".flac")
        shutil.copyfile(COMFY/"output"/output["subfolder"]/output["filename"],raw)
        candidates[name]=raw
    for item in sources:
        species,cue=item["species"],item["cue"]
        name=f"{species}-{cue}"
        material,seconds,material_gain=MATERIAL[species,cue]
        recorded=FOLEY/material
        raw=candidates[name]
        length=round(seconds*RATE)
        model,onset=cut_event(decode(raw,82),length)
        if peak(model)<.025:
            # The music model sometimes responds with an almost silent take.
            # Extract a separate accent from a successful render for that body.
            raw=candidates[f"{species}-attack"]
            material_track=decode(raw,120 if cue=="hit" else 60)
            model,onset=cut_event(material_track,length,1 if cue=="hit" else 2)
            if peak(model)<.025:
                model,onset=cut_event(material_track,length)
        foley,recorded_onset=cut_event(decode(recorded,65),length)
        assert peak(model)>=.025 and peak(foley)>.001,(name,peak(model),peak(foley))
        for i,rate in enumerate((1.,1.055,.942)):
            gesture=variant(model/peak(model),rate,length)
            transient=variant(foley/peak(foley),1+(rate-1)*1.7,length)
            model_gain=.52 if raw!=candidates[name] else .66
            signal=faded(model_gain*gesture+material_gain*transient)
            signal*=.69/max(.69,peak(signal))
            stem=name+("" if i==0 else f"-v{i+1}")
            clips.extend([signal,np.zeros((RATE//5,2))])
            slices.append({**item,"file":stem+".wav","start":position,"frames":length,
                           "model_output":str(raw),"model_sha256":digest(raw),"model_onset":onset,
                           "foley_source":str(recorded),"foley_sha256":digest(recorded),
                           "foley_onset":recorded_onset,"foley_license":"CC0 1.0","rate":rate})
            position+=length+RATE//5
    write_wave(work/"unmastered-reel.wav",np.concatenate(clips))
    (work/"regions.json").write_text(json.dumps(slices,indent=2),encoding="utf-8")
    print("ComfyUI model audio and recorded material in 27 isolated regions",flush=True)


def export_regions(work,master):
    with wave.open(str(master),"rb") as wav:
        assert (wav.getframerate(),wav.getsampwidth(),wav.getnchannels())==(RATE,2,2)
        audio=np.frombuffer(wav.readframes(wav.getnframes()),dtype="<i2").reshape(-1,2).astype(float)/32768
    folder=work/"delivery"
    folder.mkdir()
    results=[]
    for entry in json.loads((work/"regions.json").read_text(encoding="utf-8")):
        start,count=entry["start"],entry["frames"]
        clip=audio[start:start+count].copy()
        assert len(clip)==count and peak(clip)>.08,(entry,peak(clip))
        clip-=clip.mean(axis=0)
        faded(clip,48,288)
        clip*=.63/peak(clip)
        path=folder/entry["file"]
        write_wave(path,clip)
        results.append({**entry,"sha256":digest(path),"peak_dbfs":20*math.log10(peak(clip)),
                        "rms_dbfs":20*math.log10(float(np.sqrt(np.mean(clip*clip)))),
                        "dc":float(clip.mean()),"clipped_samples":int(np.count_nonzero(np.abs(clip)>=.999))})
    assert len(results)==27 and len({entry["sha256"] for entry in results})==27
    assert all(abs(entry["dc"])<.001 and entry["clipped_samples"]==0 for entry in results)
    (work/"delivery-receipt.json").write_text(json.dumps({
        "status":"PASS","model":"ACE-Step 1.5 Turbo","weights_license":"Apache-2.0",
        "model_url":"https://huggingface.co/Comfy-Org/ACE-Step-1.5","audacity_export":str(master),
        "rate":RATE,"channels":2,"bits":16,"audio_played":False,"effects":results,
    },indent=2)+"\n",encoding="utf-8")
    print("27 stereo PCM16 cues at 48 kHz, with peak, RMS, DC and clipping checks",flush=True)


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action",choices=["assemble","export"])
    parser.add_argument("--run",type=Path,required=True)
    parser.add_argument("--prompt")
    parser.add_argument("--master",type=Path)
    parser.add_argument("--no-audio",required=True,action="store_true")
    args=parser.parse_args()
    work=Path("C:/AI2D/audio")/args.run.name
    if args.action=="assemble":
        assemble(args.run,work,args.prompt)
    else:
        export_regions(work,args.master)

"""Offline recipe fallback for PCM audio: trim, gain, mix, Echo, fades and peak normalization.

Preserves source files and writes WAV plus an editable recipe, not a fictitious
Audacity project. Unsupported effects/project_source are rejected before export.
"""
import json,math,sys,uuid,wave
from pathlib import Path
import numpy as np
from asset_tools import sha,write_json
from audio_tools import ffmpeg
from game_import import validate_audio


def process_offline(recipe,directory):
    if recipe.get("project_source"):raise ValueError("AUP3 input requires the Audacity workflow.")
    inputs=[Path(p).resolve() for p in recipe.get("inputs",[])]
    if not inputs:raise ValueError("No audio inputs.")
    effects=recipe.get("effects",[])
    for effect in effects:
        if effect.get("command")!="Echo" or set(effect.get("params",{}))!={"Delay","Decay"}:
            raise ValueError("Offline fallback supports Echo only; choose Audacity for other effects.")
    output=Path(recipe["output"]).resolve()
    if output.suffix.lower()!=".wav" or output.exists() or output in inputs:raise FileExistsError("Use a fresh WAV output.")
    if recipe.get("channels",2)!=2 or recipe.get("sample_rate",48000)!=48000:
        raise ValueError("Offline game delivery is stereo / 48 kHz.")
    hashes={str(p):sha(p) for p in inputs};durations=[]
    for source in inputs:
        if source.suffix.lower()!=".wav":raise ValueError("Offline input currently requires WAV.")
        validate_audio(source)
        with wave.open(str(source),"rb") as w:durations.append(w.getnframes()/w.getframerate())
    trim=recipe.get("trim") or {"start":0,"end":max(durations)}
    start,end=float(trim["start"]),float(trim["end"])
    if not 0<=start<end or start>=min(durations):raise ValueError("Invalid offline trim range.")
    duration=max(min(end,d)-start for d in durations)
    directory=Path(directory)/("offline_"+uuid.uuid4().hex[:10]);directory.mkdir(parents=True,exist_ok=False)
    write_json(directory/"recipe.json",recipe)
    args=[];filters=[]
    for i,source in enumerate(inputs):
        args.extend(["-i",source]);gain=float(recipe.get("track_gain_db",{}).get(str(i),0))
        if not math.isfinite(gain) or not -60<=gain<=24:raise ValueError("Invalid gain.")
        filters.append(f"[{i}:a]atrim=start={start}:end={end},asetpts=PTS-STARTPTS,volume={gain}dB[a{i}]")
    if len(inputs)>1:filters.append("".join(f"[a{i}]" for i in range(len(inputs)))+f"amix=inputs={len(inputs)}:normalize=0:duration=longest[mix]")
    chain="[mix]" if len(inputs)>1 else "[a0]"
    operations=[]
    for effect in effects:
        delay=float(effect["params"]["Delay"]);decay=float(effect["params"]["Decay"])
        if not .001<=delay<=5 or not 0<decay<1:raise ValueError("Invalid Echo parameters.")
        operations.append(f"aecho=1:1:{delay*1000}:{decay}")
    fade_in=float(recipe.get("fade_in",0));fade_out=float(recipe.get("fade_out",0))
    for value in (fade_in,fade_out):
        if not 0<=value<duration:raise ValueError("Invalid fade length.")
    if fade_in:operations.append(f"afade=t=in:st=0:d={fade_in}")
    if fade_out:operations.append(f"afade=t=out:st={duration-fade_out}:d={fade_out}")
    # Keep the recipe's musical/SFX duration; Echo tails may extend internally.
    operations.extend([f"atrim=duration={duration}","aresample=48000","aformat=sample_fmts=flt:channel_layouts=stereo"])
    filters.append(chain+",".join(operations)+"[out]")
    raw=directory/"processed_float.wav"
    ffmpeg([*args,"-filter_complex",";".join(filters),"-map","[out]","-c:a","pcm_f32le",raw],directory/"render.log")
    sys.path.insert(0,r"C:\AI2D\audio\scripts")
    from audacity_workflow import decode_wav
    stats,values=decode_wav(raw)
    samples=np.array(values,dtype=np.float64)
    if recipe.get("normalize_db") is not None:
        db=float(recipe["normalize_db"])
        if not -60<=db<=-1:raise ValueError("Choose a peak target between -60 and -1 dBFS.")
        samples*=10**(db/20)/float(np.max(np.abs(samples)))
    if np.abs(samples).max()>=.999:raise ValueError("Offline mix lacks headroom; normalize it.")
    output.parent.mkdir(parents=True,exist_ok=True)
    with output.open("xb") as f:
        with wave.open(f,"wb") as w:
            w.setnchannels(2);w.setsampwidth(2);w.setframerate(48000)
            w.writeframes(np.rint(samples*32768).astype("<i2").tobytes())
    final,_=decode_wav(output)
    if final["clipped_samples"]:raise ValueError("Offline delivery clips.")
    if any(sha(p)!=digest for p,digest in hashes.items()):raise IOError("An original input changed.")
    result={"status":"PASS","engine":"FFmpeg/Python offline fallback","output":str(output),"project":None,
            "project_recipe":str(directory/"recipe.json"),"wav":final,"source_hashes":hashes,
            "work_directory":str(directory),"audio_played":False}
    write_json(directory/"job-report.json",result)
    return result

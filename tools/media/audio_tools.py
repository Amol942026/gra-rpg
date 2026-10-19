"""Offline loudness metering, fixed-length loop seam repair and OGG delivery."""
from pathlib import Path
import json, math, re, subprocess, wave
import numpy as np
from asset_tools import sha, write_json

FFMPEG=Path(r"C:\Program Files\Krita (x64)\bin\ffmpeg.exe")


def ffmpeg(arguments, log_path, engine=FFMPEG):
    process=subprocess.run([str(engine),"-hide_banner","-nostdin","-n",*map(str,arguments)],
        capture_output=True,timeout=180,creationflags=subprocess.CREATE_NO_WINDOW)
    Path(log_path).write_bytes(process.stderr)
    if process.returncode:
        raise RuntimeError(f"FFmpeg failed ({process.returncode}); see {log_path}")
    return process.stderr.decode("utf-8","replace")


def vorbis_encoder():
    for engine in (FFMPEG,Path(r"C:\Program Files\Synfig\bin\ffmpeg.exe")):
        if not engine.is_file():continue
        result=subprocess.run([str(engine),"-hide_banner","-encoders"],capture_output=True,
                              timeout=20,creationflags=subprocess.CREATE_NO_WINDOW)
        if result.returncode==0 and b"libvorbis" in result.stdout:
            return engine
    raise RuntimeError("No installed FFmpeg provides libvorbis. Keep the PCM master; do not use an unverified encoder.")


def loudness(path, log_path):
    text=ffmpeg(["-i",path,"-af","loudnorm=I=-18:TP=-2:LRA=11:print_format=json","-f","null","-"],log_path)
    matches=re.findall(r'\{\s*"input_i".*?\}',text,re.S)
    if not matches:raise RuntimeError("FFmpeg did not return loudness measurements.")
    report=json.loads(matches[-1])
    if not all(math.isfinite(float(report[k])) for k in ("input_i","input_tp","input_lra","input_thresh")):
        raise ValueError("Silent or invalid loudness measurement.")
    return report


def deliver_music(source, output, *, target_lufs=-18, seam_ms=1):
    """New PCM master and Vorbis stream. Repair <2ms around the circular seam.

    A cyclic Hermite bridge preserves exact bar/sample count. This is not a
    substitute for composing a cyclic phrase or rendering release tails first.
    """
    output=Path(output)
    ogg=output.with_suffix(".ogg")
    encoder=vorbis_encoder()
    for p in (output,ogg):
        if p.exists():raise FileExistsError(p)
    output.parent.mkdir(parents=True,exist_ok=True)
    measured=loudness(source,output.with_suffix(".measure.log"))
    normalized=output.with_name(output.stem+"_loudness.wav")
    spec=(f"loudnorm=I={target_lufs}:TP=-2:LRA=11:measured_I={measured['input_i']}:"
          f"measured_TP={measured['input_tp']}:measured_LRA={measured['input_lra']}:"
          f"measured_thresh={measured['input_thresh']}:offset={measured['target_offset']}:linear=true")
    ffmpeg(["-i",source,"-af",spec,"-ar","48000","-ac","2","-c:a","pcm_s16le",normalized],output.with_suffix(".normalize.log"))
    with wave.open(str(normalized),"rb") as stream:
        rate=stream.getframerate();channels=stream.getnchannels()
        assert stream.getsampwidth()==2
        samples=np.frombuffer(stream.readframes(stream.getnframes()),dtype="<i2").reshape(-1,channels).astype(np.float64)/32768
    before_jump=float(np.max(np.abs(samples[0]-samples[-1])))
    n=max(4,round(rate*seam_ms/1000))
    if len(samples)<=4*n:raise ValueError("Music loop too short.")
    t=np.linspace(0,1,2*n)[:,None]
    y0,y1=samples[-n].copy(),samples[n-1].copy()
    # Monotone bounded bridge avoids introducing an overshooting click.
    bridge=(2*t**3-3*t**2+1)*y0+(-2*t**3+3*t**2)*y1
    samples[-n:]=bridge[:n];samples[:n]=bridge[n:]
    peak=float(np.abs(samples).max())
    if peak>=.999:raise ValueError("Master lacks headroom.")
    with wave.open(str(output),"wb") as stream:
        stream.setnchannels(channels);stream.setsampwidth(2);stream.setframerate(rate)
        stream.writeframes(np.rint(np.clip(samples,-1,32767/32768)*32768).astype("<i2").tobytes())
    ffmpeg(["-i",output,"-c:a","libvorbis","-q:a","5",ogg],output.with_suffix(".encode.log"),engine=encoder)
    # Decode the exact compressed delivery too; measure its loop seam and duration.
    decoded=output.with_name(output.stem+"_ogg_decoded.wav")
    ffmpeg(["-i",ogg,"-c:a","pcm_s16le",decoded],output.with_suffix(".decode.log"))
    with wave.open(str(decoded),"rb") as stream:
        decoded_frames=stream.getnframes()
        data=np.frombuffer(stream.readframes(decoded_frames),dtype="<i2").reshape(-1,channels).astype(np.float64)/32768
    if decoded_frames!=len(samples):raise ValueError("Compressed loop has changed sample count.")
    after_jump=float(np.max(np.abs(samples[0]-samples[-1])))
    compressed_jump=float(np.max(np.abs(data[0]-data[-1])))
    usual=float(np.quantile(np.abs(np.diff(data,axis=0)),.999))
    if after_jump>.005 or compressed_jump>max(.015,usual*4):
        raise ValueError("Loop boundary does not pass de-click checks.")
    measured_final=loudness(output,output.with_suffix(".final-loudness.log"))
    if abs(float(measured_final["input_i"])-target_lufs)>.5 or float(measured_final["input_tp"])>-1.5:
        raise ValueError("Music delivery missed its loudness/headroom target.")
    report={"status":"PASS","source":str(source),"source_sha256":sha(source),
            "wav":str(output),"ogg":str(ogg),"frames":len(samples),"sample_rate":rate,
            "seconds":len(samples)/rate,"seam_repair_ms_each_side":seam_ms,
            "seam_jump_before":before_jump,"seam_jump_after":after_jump,"ogg_seam_jump":compressed_jump,
            "ogg_decode_frames":decoded_frames,"peak_dbfs":20*math.log10(peak),
            "integrated_loudness":measured_final,"encoder":str(encoder),"encoder_sha256":sha(encoder),
            "wav_sha256":sha(output),"ogg_sha256":sha(ogg),"audio_played":False}
    write_json(output.with_suffix(".loop.json"),report)
    return report

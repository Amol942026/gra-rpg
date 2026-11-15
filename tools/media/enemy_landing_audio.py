"""Render material-specific enemy ground-contact effects offline."""

import argparse
from array import array
import json
import math
from pathlib import Path
import re
import subprocess
import wave

from asset_tools import sha, write_json
from audio_tools import ffmpeg


RATE = 48000
PHYSICAL = Path("C:/AI2D/audio/emberfall-physical-20261108")
ROLL = Path("C:/AI2D/audio/emberfall-roll-20261109")
CINDERHORN = Path("C:/AI2D/audio/rogacz-zaru-20261113")

# (stem, gain, delay, speed, high-pass, low-pass, usable length in seconds).
# A thud, its material detail and settling grit are separate tracks.
BANKS = {
    "land-slime": (.40, -8.5, (
        ("impactSoft_medium_{i:03}", .82, 0, .85, 48, 840, .14),
        ("handleSmallLeather", .23, .011, .77, 360, 2400, .12),
        ("impactSoft_heavy_{j:03}", .27, .049, 1.18, 180, 870, .09),
    ), (.14, .005, .15, 260, 1100)),
    "land-beast": (.50, -5.3, (
        ("impactSoft_heavy_{i:03}", 1.00, 0, .76, 49, 1520, .18),
        ("earth-{i}", .32, .006, .85, 295, 5000, .13),
        ("cloth{i1}", .23, .016, .74, 300, 6500, .26),
        ("earth-{j}", .18, .092, 1.04, 450, 3650, .13),
    ), None),
    "land-bone": (.44, -6.8, (
        ("impactWood_light_{i:03}", .76, 0, 1.40, 350, 6700, .11),
        ("impactSoft_medium_{i:03}", .32, 0, 1.12, 70, 1650, .11),
        ("impactWood_light_{j:03}", .29, .037, 1.64, 630, 6800, .10),
        ("impactWood_light_{k:03}", .15, .086, 1.16, 720, 6000, .12),
    ), None),
    "land-stone": (.58, -4.5, (
        ("impactMining_{i:03}", .88, 0, .71, 95, 4600, .38),
        ("impactPunch_heavy_{i:03}", .64, 0, .68, 48, 590, .16),
        ("impactMining_{j:03}", .18, .084, 1.19, 430, 6000, .24),
    ), (.11, .024, .28, 123, 2360)),
    "land-wood": (.53, -6.0, (
        ("impactWood_heavy_{i:03}", .89, 0, .72, 66, 3900, .25),
        ("impactSoft_heavy_{i:03}", .36, 0, .73, 44, 740, .16),
        ("creak{i1}", .27, .015, .84, 280, 4400, .29),
        ("earth-{i}", .21, .074, .91, 440, 4800, .14),
    ), None),
    "land-shell": (.42, -7.0, (
        ("impactWood_medium_{i:03}", .62, 0, 1.19, 125, 5370, .12),
        ("impactPlate_light_{i:03}", .29, 0, .95, 770, 5300, .095),
        ("earth-{i}", .29, .009, 1.10, 350, 5700, .13),
        ("impactWood_light_{j:03}", .11, .054, 1.23, 710, 5000, .11),
    ), None),
    "land-sand": (.59, -6.8, (
        ("impactSoft_heavy_{i:03}", .71, 0, .69, 45, 730, .18),
        ("earth-{i}", .68, 0, .65, 360, 3900, .21),
        ("earth-{j}", .22, .051, .70, 460, 4300, .18),
        ("swish-{wind}", .15, .063, .72, 350, 4800, .22),
    ), (.22, 0, .37, 330, 5800)),
    "land-ember": (.47, -7.5, (
        ("impactMining_{i:03}", .59, 0, 1.06, 145, 3940, .23),
        ("impactSoft_medium_{i:03}", .43, 0, .84, 54, 900, .14),
        ("swish-{wind}", .18, .023, .86, 940, 4900, .13),
        ("impactWood_light_{j:03}", .12, .061, 1.36, 1100, 7200, .09),
    ), (.15, .011, .24, 1000, 6600)),
}

ORCHESTRA = """
sr=48000
ksmps=16
nchnls=2
0dbfs=1
seed 151126
instr 1
 Sfile strget p4
 aL,aR diskin2 Sfile,p5,0,0
 aL butterhp aL,p7
 aR butterhp aR,p7
 aL butterlp aL,p8
 aR butterlp aR,p8
 kenv linseg 0,.001,1,p3-.029,1,.028,0
 outs aL*p6*kenv,aR*p6*kenv
endin
instr 2
 aGrain rand 1
 aGrain butterhp aGrain,p5
 aGrain butterlp aGrain,p6
 kDecay expseg .015,.009,1,p3-.009,.001
 outs aGrain*p4*kDecay,aGrain*p4*kDecay*.96
endin
"""


def read_wav(path):
    with wave.open(str(path), "rb") as source:
        assert (source.getnchannels(), source.getsampwidth(), source.getframerate()) == (2, 2, RATE), path
        frames = source.getnframes()
        signal = array("h", source.readframes(frames))
    assert len(signal) == 2*frames
    return signal


def write_wav(path, signal):
    if path.exists():
        raise FileExistsError(path)
    with wave.open(str(path), "wb") as target:
        target.setparams((2, 2, RATE, 0, "NONE", "not compressed"))
        target.writeframes(signal.tobytes())


def crest(signal):
    step = 2 * RATE // 200
    levels = []
    for offset in range(0, len(signal), step):
        block = signal[offset:offset+step]
        levels.append(math.sqrt(sum(value*value for value in block)/len(block)))
    return levels


def early_window(levels, fraction):
    peak = max(levels)
    return next(i for i, value in enumerate(levels) if value >= fraction*peak)


def shape(signal, target_peak, short_fade=False):
    frames = len(signal)//2
    tail = 0 if short_fade else 1080
    mean_left = sum(signal[::2])/frames
    mean_right = sum(signal[1::2])/frames
    means = (mean_left, mean_right)
    output = array("d")
    for i, value in enumerate(signal):
        frame = i//2
        gain = min(1, frame/48, (frames-1-frame)/tail) if tail else min(1,frame/36)
        output.append((value-means[i%2])*gain)
    shift = 32768*10**(target_peak/20)/max(abs(min(output)),max(output))
    mastered = array("h", (round(x*shift) for x in output))
    assert max(abs(min(mastered)), max(mastered)) < 32768
    return mastered


def source_records():
    with (PHYSICAL/"layers.json").open(encoding="utf-8") as stream:
        physical = {row["layer"]: {"original": row["source"], "source_sha256": row["source_sha256"],
                                  "layer": row["wav"], "layer_sha256": row["sha256"]} for row in json.load(stream)}
    with (ROLL/"sources.json").open(encoding="utf-8") as stream:
        for row in json.load(stream):
            name = Path(row["layer"]).stem
            if name.startswith("earth-"):
                physical[name] = {"original": row["source"], "source_sha256": row["source_sha256"],
                                  "layer": row["layer"], "layer_sha256": row["sha256"]}
    return physical


def render_reel(root, csound):
    sources = source_records()
    dependencies = {}
    rows, takes = [], []
    start = .20
    for cue, (seconds, peak, layers, noise) in BANKS.items():
        for i in range(3):
            for stem, gain, delay, speed, high, low, duration in layers:
                name = stem.format(i=i, i1=i+1, j=(i+1)%3, k=(i+2)%3, wind=10+i)
                info = sources[name]
                wav = Path(info["layer"])
                if name not in dependencies:
                    assert sha(wav) == info["layer_sha256"]
                    assert sha(Path(info["original"])) == info["source_sha256"]
                    dependencies[name] = {**info,"license":"CC0 1.0"}
                rows.append(f'i 1 {start+delay:.6f} {duration:.6f} "{wav.as_posix()}" {speed} {gain*.55} {high} {low}')
            if noise:
                gain, delay, duration, high, low = noise
                rows.append(f'i 2 {start+delay:.6f} {duration:.6f} {gain*.55} {high} {low}')
            suffix = f"-v{i+1}" if i else ""
            takes.append((cue+suffix, round(start*RATE), round(seconds*RATE), peak))
            start += seconds+.10
    csd = root/"enemy-landings.csd"
    csd.write_text("<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n"
                   + ORCHESTRA + "\n</CsInstruments>\n<CsScore>\n"
                   + "\n".join(rows) + f"\nf 0 {start:.6f}\ne\n</CsScore>\n</CsoundSynthesizer>\n",encoding="utf-8")
    reel = root/"rendered-reel.wav"
    result = subprocess.run([str(csound),"-+rtaudio=null","-+rtmidi=null","-W","-s","-o",str(reel),str(csd)],
                            capture_output=True,timeout=60,creationflags=subprocess.CREATE_NO_WINDOW)
    (root/"csound.log").write_bytes(result.stdout+result.stderr)
    result.check_returncode()
    decoded = read_wav(reel)
    assert max(abs(min(decoded)),max(decoded)) < 32767
    for name, start, frames, peak in takes:
        part = decoded[2*start:2*(start+frames)]
        assert len(part) == 2*frames
        write_wav(root/(name+".wav"), shape(part, peak))
    return csd, dependencies


def align_cinderhorn(root):
    records = []
    for suffix in ("", "-v2", "-v3"):
        filename = "cinderhorn-fall"+suffix+".wav"
        original = CINDERHORN/filename
        sound = read_wav(original)
        impact = early_window(crest(sound), .5)*5
        trim_ms = max(0, impact-5)
        cropped = sound[2*round(RATE*trim_ms/1000):]
        write_wav(root/filename,shape(cropped,-4.5,short_fade=True))
        records.append({"file": filename,"original":str(original),"source_sha256":sha(original),
                        "original_half_crest_ms":impact,"removed_ms":trim_ms})
    return records


def measurements(path, log_path):
    sound = read_wav(path)
    levels = crest(sound)
    peak = max(abs(min(sound)),max(sound))
    rms = math.sqrt(sum(value*value for value in sound)/len(sound))
    half_ms = early_window(levels,.5)*5
    quarter_ms = early_window(levels,.25)*5
    response = ffmpeg(["-i",path,"-af","loudnorm=I=-18:TP=-2:LRA=11:print_format=json","-f","null","-"],log_path)
    raw = json.loads(re.findall(r'\{\s*"input_i".*?\}',response,re.S)[-1])
    assert peak<32767 and rms>50 and quarter_ms<=10 and half_ms<=15, (path,quarter_ms,half_ms)
    assert float(raw["input_tp"]) < -2.6, (path,raw["input_tp"])
    return {"file":path.name,"sha256":sha(path),"bytes":path.stat().st_size,"frames":len(sound)//2,
            "onset_quarter_crest_ms":quarter_ms,"onset_half_crest_ms":half_ms,
            "onset_sample_ms":round(next(i for i,x in enumerate(sound) if abs(x)>16)*500/RATE,2),
            "peak_dbfs":round(20*math.log10(peak/32768),2),"rms_dbfs":round(20*math.log10(rms/32768),2),
            "true_peak_dbtp":float(raw["input_tp"]),"integrated_lufs":float(raw["input_i"]),
            "clipping_samples":0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,required=True)
    parser.add_argument("--csound",type=Path,required=True)
    parser.add_argument("--no-audio",action="store_true",required=True)
    args = parser.parse_args()
    root = args.root.resolve()
    root.mkdir(parents=True,exist_ok=False)
    (root/"meters").mkdir()
    csd, used = render_reel(root,args.csound.resolve())
    cinderhorn = align_cinderhorn(root)
    paths = sorted(path for path in root.glob("*.wav") if path.name != "rendered-reel.wav")
    assert len(paths) == 27
    report = [measurements(path,root/"meters"/(path.stem+".log")) for path in paths]
    write_json(root/"bindings.json",{path.stem:str(path) for path in paths})
    write_json(root/"report.json",{"status":"PASS","audio_played":False, "csd":str(csd),"csd_sha256":sha(csd),
                                  "source_credits":"assets/licenses/audio-foley-CC0.txt","sources":used,
                                  "realigned_cinderhorn":cinderhorn,"sample_rate":RATE,"channels":2,"sample_width":2,
                                  "files":report})
    print(f"PASS: {len(report)} landing variants, onset within {max(x['onset_half_crest_ms'] for x in report)} ms")


if __name__ == "__main__":
    main()

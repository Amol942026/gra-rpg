"""Build Rogacz Żaru foley from the licensed source stems and offline Csound."""
import argparse
import json
from pathlib import Path
import subprocess

import numpy as np

from refine_combat_audio import RATE, WORK, read_wav, sha, write_wav

ORCHESTRA = """
sr=48000
ksmps=32
nchnls=2
0dbfs=1
seed 45197
instr 1
 Sfile strget p4
 aL,aR diskin2 Sfile,p5,0,0
 aL butterhp aL,p7
 aR butterhp aR,p7
 aL butterlp aL,p8
 aR butterlp aR,p8
 kenv linseg 0,.004,1,p3-.021,1,.017,0
 outs aL*p6*kenv,aR*p6*kenv
endin
instr 2
 anoise rand 1
 abody butterlp anoise,p5
 abody butterhp abody,44
 kenv expseg .005,.014,1,p3-.014,.001
 outs abody*p4*kenv,abody*p4*kenv*.96
endin
"""


def render(root, csound, charge_only=False):
    root.mkdir(parents=True)
    sources = WORK / "layers"
    used = {}
    rows, takes = [], []
    start = .25
    events = (("charge",.72),) if charge_only else (("attack", .64), ("hit", .34), ("fall", .78))
    for event, duration in events:
        for variant in range(3):
            def layer(name, gain, delay, speed, high, low):
                stem = sources / (name + ".wav")
                used[str(stem)] = sha(stem)
                rows.append(f'i 1 {start+delay:.6f} {duration-delay:.6f} '
                            f'"{stem.as_posix()}" {speed} {gain} {high} {low}')

            def body(gain, delay, length, lowpass):
                rows.append(f"i 2 {start+delay:.6f} {length} {gain} {lowpass}")

            def stone(gain, delay, speed):
                layer(f"impactMining_{variant:03}", gain, delay, speed, 98, 3250)

            if event == "charge":
                layer(f"snarl-{variant}", .92, 0, (.75,.81,.78)[variant], 105, 4100)
                layer(f"swish-{10+variant}", .37, .041, .89, 275, 4800)
                for step, delay in enumerate((.035,.15,.27,.41,.55)):
                    layer(f"impactSoft_heavy_{(variant+step)%3:03}", .32-step*.03, delay, 1.28, 69, 1640)
                    layer(f"impactMining_{(variant+step)%3:03}", .19-step*.019, delay+.014, 1.16, 220, 4150)
                body(.24,.013,.46,270)
            elif event == "attack":
                layer(f"snarl-{variant}", .84, 0, (.77, .83, .79)[variant], 108, 4600)
                layer(f"swish-{10+variant}", .24, .024, .77, 330, 5400)
                stone(.32, .043, .78)
                body(.20, .038, .24, 305)
            elif event == "hit":
                stone(.65, 0, 1.05)
                layer(f"snarl-{(variant+1)%3}", .29, .012, 1.24, 320, 3700)
                layer(f"impactPunch_heavy_{variant:03}", .36, .003, .98, 65, 1450)
            else:
                layer(f"impactSoft_heavy_{variant:03}", .64, .026, .72, 58, 1700)
                stone(.46, .068, .76)
                layer(f"snarl-{(variant+2)%3}", .34, 0, .70, 105, 2400)
                layer(f"impactMining_{(variant+1)%3:03}", .18, .23, 1.04, 140, 2400)
                body(.27, .032, .34, 250)
            name = f"cinderhorn-{event}" + (f"-v{variant+1}" if variant else "")
            takes.append((name, round(start*RATE), round(duration*RATE)))
            start += duration + .27

    project = root / "cinderhorn.csd"
    project.write_text("<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n"
                       + ORCHESTRA + "\n</CsInstruments>\n<CsScore>\n"
                       + "\n".join(rows) + f"\nf 0 {start}\ne\n</CsScore>\n</CsoundSynthesizer>\n",
                       encoding="utf-8")
    reel = root / "reel.wav"
    result = subprocess.run([str(csound), "-+rtaudio=null", "-+rtmidi=null", "-W", "-s",
                             "-o", str(reel), str(project)], capture_output=True, timeout=60,
                            creationflags=subprocess.CREATE_NO_WINDOW)
    (root / "csound.log").write_bytes(result.stdout + result.stderr)
    result.check_returncode()

    samples = read_wav(reel)
    output, metrics = {}, []
    for name, offset, frames in takes:
        signal = samples[offset:offset+frames].copy()
        assert signal.shape == (frames, 2) and np.abs(signal).max() < 1
        signal -= signal.mean(axis=0)
        signal[:144] *= np.linspace(0, 1, 144)[:, None]
        signal[-960:] *= np.linspace(1, 0, 960)[:, None]
        signal *= 10**(-4.5/20)/np.abs(signal).max()
        target = root / (name + ".wav")
        write_wav(target, signal)
        decoded = read_wav(target)
        peak = float(np.abs(decoded).max())
        rms = float(np.sqrt(np.mean(decoded*decoded)))
        assert peak < .62 and rms > .015
        output[name] = str(target)
        metrics.append({"file": target.name, "sha256": sha(target), "frames": frames,
                        "peak_dbfs": float(20*np.log10(peak)), "rms_dbfs": float(20*np.log10(rms))})
    (root/"bindings.json").write_text(json.dumps(output, indent=2), encoding="utf-8")
    (root/"receipt.json").write_text(json.dumps({"status": "PASS", "audio_played": False,
        "sources_cc0": used, "project_sha256": sha(project), "files": metrics}, indent=2), encoding="utf-8")
    print("Csound:", len(metrics), "verified stereo PCM16 WAV files")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--csound", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    parser.add_argument("--charge-only", action="store_true")
    args = parser.parse_args()
    render(args.root, args.csound, args.charge_only)

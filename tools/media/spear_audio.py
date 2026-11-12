"""Render the four variant spear sound banks offline with Csound."""
import argparse
from array import array
import hashlib
import json
from pathlib import Path
import subprocess
import wave


# Duration, air filter centre, body resonance, high ring amount, air mix.
VOICES = {
    "briar": {"thrust": (.25, 760, 162, .015, .61), "impact": (.19, 1140, 218, .021, .36)},
    "oasis": {"thrust": (.30, 1910, 329, .055, .68), "impact": (.23, 2460, 492, .107, .29)},
    "basalt": {"thrust": (.34, 530, 112, .012, .55), "impact": (.25, 670, 91, .031, .51)},
    "dawn": {"thrust": (.31, 1580, 388, .096, .54), "impact": (.28, 2070, 515, .148, .25)},
}

ORCHESTRA = """<CsoundSynthesizer>
<CsOptions>
-d -m0 -+rtaudio=null -+rtmidi=null
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 16
nchnls = 2
0dbfs = 1
instr 1
 iAir = p4
 iBody = p5
 iRing = p6
 iNoise = p7
 iImpact = p8
 iSeed = p9
 iOnset = .004 + .087*(1-iImpact)
 aAirShape linseg 0, iOnset, 1, p3-iOnset-.020, .02, .020, 0
 aBodyShape expseg .001, iOnset, 1, p3-iOnset-.012, .001, .012, .00001
 kFilter expseg iAir*.61, iOnset, iAir*1.57, p3-iOnset, iAir*.53
 kPitch expseg iBody*1.13, p3*.28, iBody, p3*.72, iBody*.73
 aNoise rand .44, iSeed
 aAir reson aNoise, kFilter, iAir*.7, 1
 aCrack reson aNoise, iAir*2.9, iAir*1.3, 1
 aBody oscili .16*(.24+iImpact*.76), kPitch
 aRing oscili iRing, iBody*3.71
 aOvertone oscili iRing*.29, iBody*5.29
 aDry = aAirShape*(iNoise*aAir + .24*iImpact*aCrack) + aBodyShape*(aBody+aRing+aOvertone)
 aSignal butterhp aDry, 48
 aSpace delay aSignal, .00075
 outs aSignal, aSignal*.94+aSpace*.06
endin
</CsInstruments>
<CsScore>
i1 0 {duration} {air} {body} {ring} {noise} {impact} {seed}
e
</CsScore>
</CsoundSynthesizer>
"""


def master(raw, output, impact):
    with wave.open(str(raw)) as file:
        assert (file.getnchannels(), file.getsampwidth(), file.getframerate()) == (2, 2, 48000)
        samples = array("h")
        samples.frombytes(file.readframes(file.getnframes()))
        frames = file.getnframes()
    peak = max(abs(value) for value in samples)
    assert 100 < peak < 32767
    target = 10 ** ((-4.5 if impact else -6.0) / 20) * 32767
    samples = array("h", (round(value * target / peak) for value in samples))
    assert abs(sum(samples) / len(samples)) < 100
    assert max(abs(value) for value in samples[-192:]) < 120
    with wave.open(str(output), "wb") as file:
        file.setparams((2, 2, 48000, frames, "NONE", "not compressed"))
        file.writeframes(samples.tobytes())
    return {"file": output.name, "frames": frames,
            "sha256": hashlib.sha256(output.read_bytes()).hexdigest(), "bytes": output.stat().st_size}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--csound", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    receipts = []
    for voice_index, (voice, events) in enumerate(VOICES.items()):
        for event, (duration, air, body, ring, noise) in events.items():
            impact = event == "impact"
            for variant, factor in enumerate((1, .938, 1.071), start=1):
                stem = f"spear-{voice}-{event}" + (f"-v{variant}" if variant != 1 else "")
                path = args.output / f"{stem}.csd"
                path.write_text(ORCHESTRA.format(
                    duration=duration, air=air*factor, body=body*factor, ring=ring,
                    noise=noise, impact=int(impact),
                    seed=(voice_index*7+variant+int(impact)*3)/41), encoding="ascii")
                raw = path.with_suffix(".raw.wav")
                result = subprocess.run(
                    [str(args.csound), "-W", "-s", "-o", str(raw), str(path)],
                    capture_output=True, timeout=60, creationflags=subprocess.CREATE_NO_WINDOW)
                path.with_suffix(".log").write_bytes(result.stdout + result.stderr)
                result.check_returncode()
                receipts.append(master(raw, path.with_suffix(".wav"), impact))
    assert len({entry["sha256"] for entry in receipts}) == 24
    (args.output / "report.json").write_text(
        json.dumps({"files": receipts, "audio_played": False}, indent=2), encoding="utf-8")
    print(f"{len(receipts)} original Csound variants rendered and verified")


if __name__ == "__main__":
    main()

"""Original Emberfall score and Foley, synthesized locally; no sampled recordings.

Python + numpy + soundfile. Writes release-ready OGG/WAV and a signal QA report.
Run from any directory. The game never needs Python or this script.
"""
from pathlib import Path
import json
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools/audio-python"))
import soundfile as sf

OUT = ROOT / "assets/audio"
OUT.mkdir(parents=True, exist_ok=True)
SR = 32000
RNG = np.random.default_rng(72419)
TAU = 2 * np.pi
report = {}


def time(seconds):
    return np.arange(round(seconds * SR)) / SR


def noise(n, low=80, high=4500):
    f = np.fft.rfftfreq(n, 1 / SR)
    spectrum = np.fft.rfft(RNG.normal(size=n))
    band = (1 - np.exp(-(f / low) ** 4)) * np.exp(-(f / high) ** 4)
    result = np.fft.irfft(spectrum * band, n)
    return result / max(1e-8, np.std(result))


def envelope(t, attack=.006, decay=.12):
    return (1 - np.exp(-t / attack)) * np.exp(-t / decay)


def tone(t, start, end=None):
    if end is None:
        end = start
    return np.sin(TAU * (start * t + .5 * (end - start) / max(t[-1], .001) * t * t))


def bell(t, freq):
    return sum(np.sin(TAU * freq * ratio * t) * np.exp(-t / decay) * gain
               for ratio, decay, gain in [(1, .7, 1), (2.01, .24, .28), (3.96, .11, .1)])


def fx(name, signal, peak=.72):
    signal = np.asarray(signal, dtype=float)
    signal -= np.mean(signal)
    fade = min(160, len(signal) // 8)
    signal[:fade] *= np.linspace(0, 1, fade)
    signal[-fade:] *= np.linspace(1, 0, fade)
    signal *= peak / max(.001, np.max(np.abs(signal)))
    sf.write(OUT / (name + ".wav"), signal, SR, subtype="PCM_16")
    report[name] = {"seconds": len(signal) / SR, "peak": float(np.max(np.abs(signal))),
                    "rms": float(np.sqrt(np.mean(signal ** 2)))}


def create_effects():
    t = time(.22)
    fx("step", (.4 * noise(len(t), 110, 1700) + tone(t, 105, 55)) * envelope(t, .002, .038), .48)
    t = time(.32)
    fx("swing", noise(len(t), 600, 6500) * np.sin(np.pi * t / t[-1]) ** 2 * np.exp(-t * 5), .56)
    t = time(.38)
    fx("slime-hit", (tone(t, 240, 55) + .45 * tone(t, 520, 85) + .22 * noise(len(t), 90, 800)) * envelope(t, .003, .1))
    t = time(.4)
    fx("wolf-hit", (tone(t, 160, 65) * (1 + .6 * np.sin(TAU * 31 * t)) + .6 * noise(len(t), 150, 1500)) * envelope(t, .004, .11))
    t = time(.32)
    fx("bone-hit", (noise(len(t), 500, 4500) * np.maximum(0, np.sin(TAU * 37 * t)) ** 6 + .5 * bell(t, 690)) * envelope(t, .001, .07))
    t = time(.56)
    fx("stone-hit", (.6 * noise(len(t), 45, 2400) + tone(t, 95, 40) + .22 * bell(t, 247)) * envelope(t, .002, .14))
    t = time(.7)
    fx("enemy-fall", (tone(t, 110, 35) + .75 * noise(len(t), 35, 1600)) * envelope(t, .025, .17))
    t = time(.27)
    fx("hurt", (tone(t, 190, 85) + .4 * tone(t, 283, 135) + .7 * noise(len(t), 80, 2300)) * envelope(t, .002, .075))
    t = time(1.7)
    fx("death", (tone(t, 146.83, 73.41) + .38 * tone(t, 220, 110) + .2 * noise(len(t), 50, 600)) * envelope(t, .045, .42))
    t = time(.44)
    fx("roll", (noise(len(t), 180, 2400) * (.25 + np.sin(np.pi * t / t[-1]) ** 2) + .3 * tone(t, 90, 40)) * envelope(t, .012, .16), .6)
    t = time(.24)
    fx("dodge", (noise(len(t), 1200, 7000) + .22 * tone(t, 640, 1050)) * envelope(t, .02, .065), .5)
    t = time(1.25)
    flare = noise(len(t), 100, 3300) * (.6 + .4 * np.sin(TAU * 17 * t))
    fx("spell", (flare * .55 + tone(t, 125, 44) + .2 * bell(t, 587.33)) * envelope(t, .035, .24), .8)
    t = time(.85)
    bubbles = np.zeros(len(t))
    for start, f in [(0, 740), (.09, 420), (.19, 570), (.31, 830), (.44, 1174)]:
        q = np.maximum(0, t - start)
        bubbles += (t >= start) * tone(q, f, f * 1.5) * envelope(q, .003, .045)
    fx("potion", bubbles + .13 * bell(t, 1174.66) * envelope(t, .06, .35), .65)
    t = time(.4)
    fx("pickup", (bell(t, 1046.5) + .5 * bell(t, 1567.98)) * envelope(t, .002, .16), .6)
    t = time(.42)
    fx("equip", (.4 * noise(len(t), 400, 4500) + bell(t, 523.25) + .4 * bell(t, 831)) * envelope(t, .001, .09), .65)
    t = time(.24)
    fx("drop", (.7 * noise(len(t), 140, 1800) + tone(t, 140, 55)) * envelope(t, .002, .055), .55)
    for name, notes in [("level-up", [62, 65, 69, 74]), ("victory", [50, 57, 62, 65, 69, 74])]:
        t = time(2.6 if name == "victory" else 1.65)
        signal = np.zeros(len(t))
        for i, note in enumerate(notes):
            q = np.maximum(0, t - i * .13)
            signal += (t >= i * .13) * bell(q, 440 * 2 ** ((note - 69) / 12)) * envelope(q, .008, .52)
        fx(name, signal, .68)
    t = time(.36)
    fx("enemy-attack", (noise(len(t), 220, 2200) + .55 * tone(t, 150, 55)) * envelope(t, .02, .08), .55)
    t = time(.12)
    fx("ui", (bell(t, 880) + noise(len(t), 700, 2800) * .16) * envelope(t, .001, .025), .33)
    t = time(.18)
    fx("reject", (tone(t, 196) + .35 * tone(t, 207.65)) * envelope(t, .004, .04), .28)


def create_music():
    # 32 bars, 76 BPM, D minor. All reverberation and note tails wrap around the
    # loop boundary, preserving the atmosphere without a restart or silent gap.
    beat = 60 / 76
    duration = 32 * 4 * beat
    n = round(duration * SR)
    music = np.zeros((n, 2), dtype=np.float64)

    def add(signal, start, gain, pan):
        stereo = signal[:, None] * np.array([np.cos(pan * np.pi / 2), np.sin(pan * np.pi / 2)]) * gain
        first = round(start * SR) % n
        count = min(len(stereo), n - first)
        music[first:first + count] += stereo[:count]
        if count < len(stereo):
            music[:len(stereo) - count] += stereo[count:]

    def instrument(note, seconds, style):
        f = 440 * 2 ** ((note - 69) / 12)
        t = time(seconds)
        if style == "pad":
            signal = sum(np.sin(TAU * f * h * t + .008 * np.sin(TAU * .3 * t)) / h ** 2
                         for h in range(1, 5))
            signal += .23 * np.sin(TAU * f * 1.0017 * t)
            env = np.minimum(1, t / .75) * np.minimum(1, (seconds - t) / 1.5)
        elif style == "flute":
            phase = TAU * f * t + .019 * np.sin(TAU * 4.7 * t) * np.minimum(1, t * 3)
            signal = np.sin(phase) + .19 * np.sin(phase * 2) + .045 * noise(len(t), 1300, 3200)
            env = np.minimum(1, t / .12) * np.minimum(1, (seconds - t) / .3)
        elif style == "harp":
            signal = sum(np.sin(TAU * f * h * t) * np.exp(-t * h * 1.5) / h ** 1.8 for h in range(1, 7))
            env = (1 - np.exp(-t / .004)) * np.minimum(1, (seconds - t) / .06)
        else:
            signal = np.sin(TAU * f * t) + .25 * np.sin(TAU * 2 * f * t)
            env = np.minimum(1, t / .18) * np.minimum(1, (seconds - t) / .4)
        return signal * env

    progression = [([50, 57, 65, 69], 38), ([46, 53, 62, 65], 34),
                   ([48, 57, 60, 64], 41), ([48, 55, 62, 64], 36),
                   ([43, 50, 58, 62], 31), ([46, 53, 60, 65], 34),
                   ([45, 52, 62, 67], 33), ([50, 57, 65, 69], 38)]
    for section in range(16):
        chord, bass = progression[section % 8]
        start = section * 8 * beat
        for j, note in enumerate(chord):
            add(instrument(note, 8 * beat + 1.5, "pad"), start, .055, .16 + j * .22)
        add(instrument(bass, 7.6 * beat, "bass"), start, .105, .5)
        for b in range(2):
            for k, offset in enumerate([0, 1.5, 2.5, 3.5]):
                note = chord[(k + section + b) % 4] + 12
                add(instrument(note, 2.4, "harp"), start + (b * 4 + offset) * beat,
                    .058 if b else .075, .32 if k % 2 else .68)
    # Original call-and-response melody, with space between phrases.
    phrases = [
        [(0, 74, 1.4), (2, 69, .8), (3, 65, 1.7), (6, 64, 1), (7.5, 62, 2.8)],
        [(1, 65, 1.2), (3, 69, 1), (4.5, 72, 1.8), (7, 74, 2.8)],
        [(0, 74, 1.6), (2.5, 77, 1.1), (4, 76, 1.7), (7, 72, 2.4)],
        [(1, 69, 1.3), (3, 67, 1), (4.5, 64, 1.4), (7, 62, 3)],
        [(0, 70, 1.7), (2.5, 69, 1), (4, 67, 1.5), (7, 65, 2.6)],
        [(1, 65, 1.1), (3, 69, 1.2), (5, 74, 1.5), (8, 72, 2.8)],
        [(0, 69, 1.4), (2, 67, 1), (4, 64, 2), (7, 61, 1.4)],
        [(0, 62, 2), (3, 65, 1.3), (5, 64, 1.3), (7.5, 62, 3.5)],
    ]
    for p, phrase in enumerate(phrases):
        for offset, note, beats in phrase:
            add(instrument(note, beats * beat, "flute"), (p * 16 + offset + 1) * beat, .102, .52)
    for bar in range(32):
        for offset, velocity in [(0, .073), (2.5, .038)]:
            t = time(.55)
            drum = (tone(t, 92, 42) + .17 * noise(len(t), 65, 1000)) * envelope(t, .003, .09)
            add(drum, (bar * 4 + offset) * beat, velocity, .45)
        if bar % 8 >= 2:
            for offset in [1, 2, 3, 3.5]:
                t = time(.13)
                shaker = noise(len(t), 2400, 6500) * envelope(t, .008, .024)
                add(shaker, (bar * 4 + offset) * beat, .012, .7)
    dry = music.copy()
    for delay, gain in [(.193, .15), (.389, .12), (.617, .095), (1.031, .07), (1.613, .042)]:
        music += np.roll(dry[:, ::-1], round(delay * SR), axis=0) * gain
    music -= np.mean(music, axis=0)
    music *= .76 / np.max(np.abs(music))
    path = OUT / "echoes-of-the-ruins.ogg"
    # Small chunks also keep the Vorbis encoder's Windows stack use bounded.
    with sf.SoundFile(path, "w", samplerate=SR, channels=2, format="OGG", subtype="VORBIS") as stream:
        for start in range(0, n, 8192):
            stream.write(music[start:start + 8192])
    with sf.SoundFile(path) as stream:
        rate = stream.samplerate
        decoded = np.concatenate(list(stream.blocks(blocksize=8192)))
    report["music"] = {"title": "Echoes of the Ruins", "bpm": 76, "bars": 32,
        "seconds": len(decoded) / rate, "channels": 2, "sample_rate": rate,
        "peak": float(np.max(np.abs(decoded))), "rms": float(np.sqrt(np.mean(decoded ** 2))),
        "loop_seam_delta": float(np.max(np.abs(decoded[0] - decoded[-1]))), "bytes": path.stat().st_size}
    assert report["music"]["peak"] < .95
    assert report["music"]["loop_seam_delta"] < .05
    # Short preview for playback outside the game; not part of the release.
    sf.write(ROOT / "audio/music-preview.wav", decoded[:SR * 24], SR, subtype="PCM_16")


if __name__ == "__main__":
    create_effects()
    create_music()
    (ROOT / "audio/signal-report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Created {len(report)-1} sound effects and {report['music']['seconds']:.1f}s of original looped music.")
    print(json.dumps(report["music"], indent=2))

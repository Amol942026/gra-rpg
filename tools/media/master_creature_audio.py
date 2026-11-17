"""Prepare, verify and bind the Audacity masters of the Stable Audio creature bank."""

import argparse
import json
from pathlib import Path
import wave

import av
import numpy as np
from scipy.signal import resample_poly

from asset_tools import sha, write_json
from game_import import bind_audio, restore_bindings, validate_audio
from stable_creature_audio import REPO, SUFFIXES, profiles


RATE = 48000
MIN_DURATION = {"attack": .20, "hit": .16, "fall": .20, "dying": .37, "ability": .28}
MAX_DURATION = {"attack": .82, "hit": .47, "fall": .74, "dying": 1.42, "ability": 1.02}
PEAK_DB = {"attack": -8.0, "hit": -9.3, "fall": -10.3, "dying": -5.8, "ability": -7.6}
MAX_ONSET = {"attack": .050, "hit": .033, "fall": .023, "dying": .065, "ability": .055}
LARGE = ("stone-guardian", "living-dune", "varkhor", "cinderhorn", "treant", "ember-drake")
LIGHT = ("slime", "forest-spider", "ash-wraith", "sporekin")
REELS = {"attacks": ("attack", "ability"), "contacts": ("hit", "fall"), "dying": ("dying",)}
LOCAL_MODEL = "https://huggingface.co/Comfy-Org/stable-audio-3"
MODEL_LICENSE = "https://kb.stability.ai/knowledge-base/stable-audio-commerical-and-usage-licensing"


def decode_flac(source):
    resampler = av.AudioResampler(format="s16", layout="stereo", rate=RATE)
    with av.open(str(source)) as audio:
        track = audio.streams.audio[0]
        if track.codec_context.name != "flac" or track.layout.nb_channels != 2:
            raise ValueError(f"Expected a stereo FLAC from ComfyUI: {source}")
        decoded = [block.to_ndarray() for frame in audio.decode(track) for block in resampler.resample(frame)]
        decoded.extend(block.to_ndarray() for block in resampler.resample(None))
    return np.concatenate(decoded, axis=1).reshape(-1, 2).astype(np.float32) / 32768


def read_wave(path):
    with wave.open(str(path), "rb") as audio:
        if (audio.getnchannels(), audio.getsampwidth(), audio.getframerate()) != (2, 2, RATE):
            raise ValueError(f"Unexpected PCM16 format: {path}")
        frames = audio.getnframes()
        samples = np.frombuffer(audio.readframes(frames), dtype="<i2").reshape(-1, 2)
    if len(samples) != frames:
        raise ValueError(f"Incomplete PCM16: {path}")
    return samples.astype(np.float32) / 32768


def write_wave(path, samples):
    if not len(samples) or not np.isfinite(samples).all() or np.max(np.abs(samples)) >= .99:
        raise ValueError(f"Invalid samples before quantization: {path}")
    pcm = np.rint(samples * 32768).astype("<i2")
    with path.open("xb") as stream, wave.open(stream, "wb") as wav:
        wav.setparams((2, 2, RATE, 0, "NONE", "not compressed"))
        wav.writeframes(pcm.tobytes())


def trim_transient(signal, event, previous_frames):
    # Windowed RMS rejects isolated codec impulses. The peak onset then keeps
    # a small, deliberate lead before the point of contact in the game.
    block = RATE // 250
    windows = signal[:len(signal) // block * block].reshape(-1, block, 2)
    envelope = np.sqrt(np.mean(windows ** 2, axis=(1, 2)))
    if envelope.max() < .0004:
        raise ValueError("Stable Audio returned a near-silent take")
    body = int(np.flatnonzero(envelope > envelope.max() * .15)[0])
    before = max(0, body * block - round(.04 * RATE))
    after = (body + 2) * block
    local = np.max(np.abs(signal[before:after]), axis=1)
    onset = before + int(np.flatnonzero(local > local.max() * .13)[0])
    start = max(0, onset - round(.005 * RATE))
    min_frames = round(MIN_DURATION[event] * RATE)
    cap = min(round(MAX_DURATION[event] * RATE), max(min_frames, round(previous_frames * 1.20)))
    stop = min(len(signal), start + cap)
    tail = round(RATE * (.065 if event == "dying" else .033))
    active = np.flatnonzero(envelope[start // block: stop // block] > envelope.max() * .038)
    if len(active):
        last = (start // block + int(active[-1]) + 1) * block
        stop = min(stop, max(start + min_frames, last + tail))
    samples = signal[start:stop].copy()
    if len(samples) < min_frames:
        samples = np.pad(samples, ((0, min_frames - len(samples)), (0, 0)))
    # Narrow the captured stereo image for the game's spatial panner.
    samples = .78 * samples + .22 * samples.mean(axis=1, keepdims=True)
    samples[:144] *= np.linspace(0, 1, 144)[:, None]
    fade = min(tail, len(samples) // 4)
    samples[-fade:] *= np.linspace(1, 0, fade)[:, None] ** 1.5
    return samples, start, stop


def db_to_gain(db):
    return 10 ** (db / 20)


def level(samples, target):
    peak = float(np.max(np.abs(samples)))
    if peak < 1e-6:
        raise ValueError("No usable signal after trimming")
    return samples * (db_to_gain(target) / peak)


def make_reel(root, name, cues):
    directory = root / name
    directory.mkdir(exist_ok=False)
    parts = []
    cuts = []
    silence = np.zeros((5760, 2), dtype=np.float32)
    offset = 0
    for cue in cues:
        adjustment = .9 if cue["name"].startswith(LARGE) else -.8 if cue["name"].startswith(LIGHT) else 0
        for take in cue["takes"]:
            source = Path(take["path"])
            if sha(source) != take["sha256"]:
                raise ValueError(f"Generated FLAC changed: {source}")
            old = REPO / "assets/audio" / take["runtime"]
            with wave.open(str(old), "rb") as previous:
                prior_length = round(previous.getnframes() / previous.getframerate() * RATE)
            try:
                samples, start, stop = trim_transient(decode_flac(source), cue["event"], prior_length)
            except ValueError as error:
                raise ValueError(f"{take['runtime']}: {error}") from error
            peak_db = PEAK_DB[cue["event"]] + adjustment
            parts.extend((level(samples, peak_db - 1), silence))
            cuts.append({**take, "event": cue["event"], "cue": cue["name"], "start": offset,
                         "frames": len(samples), "peak_db": peak_db, "trim_start": start, "trim_stop": stop})
            offset += len(samples) + len(silence)
    parts.pop()
    incoming = directory / "unmastered.wav"
    write_wave(incoming, np.concatenate(parts))
    write_json(directory / "cuts.json", cuts)
    labels = "\n".join(f"{row['start']/RATE:.6f}\t{(row['start']+row['frames'])/RATE:.6f}\t{row['runtime']}" for row in cuts)
    (directory / "audacity-labels.txt").write_text(labels + "\n", encoding="utf-8")
    effects = [
        {"command": "Normalize", "params": {"ApplyVolume": False, "RemoveDcOffset": True,
                                                "StereoIndependent": False}},
        {"command": "Compressor", "params": {"thresholdDb": -18, "makeupGainDb": 0,
                                                 "kneeWidthDb": 4, "compressionRatio": 1.65,
                                                 "lookaheadMs": 1, "attackMs": 3.5, "releaseMs": 62}},
    ]
    write_json(directory / "recipe.json", {"inputs": [str(incoming)], "effects": effects,
                                             "output": str(directory / "mastered.wav"),
                                             "project": str(directory / f"creatures-{name}.aup3"),
                                             "work_directory": str(directory / "audacity")})
    print(f"{name}: {len(cuts)} cuts / {len(parts)} sections / {offset/RATE:.2f} s", flush=True)


def prepare(root):
    rows = json.loads((root / "generation.json").read_text(encoding="utf-8"))
    names = set(profiles())
    if len(rows) != len(names) or {cue["name"] for cue in rows} != names:
        raise ValueError("Finish every registered ComfyUI prompt first")
    for key, events in REELS.items():
        make_reel(root, key, [cue for cue in rows if cue["event"] in events])


def analyze(path, event):
    validate_audio(path)
    pcm = read_wave(path)
    magnitude = np.max(np.abs(pcm), axis=1)
    peak = float(magnitude.max())
    rms = float(np.sqrt(np.mean(pcm ** 2)))
    oversized = float(np.max(np.abs(resample_poly(pcm, 4, 1, axis=0))))
    onset = int(np.flatnonzero(magnitude > peak * .10)[0])
    if not 0.001 < rms < peak < .65 or not .15 < oversized < .75 or len(pcm) < round(MIN_DURATION[event] * RATE):
        raise ValueError(f"Unexpected signal level or duration: {path}")
    if onset > RATE * MAX_ONSET[event] or np.max(np.abs(pcm.mean(axis=0))) > .008:
        raise ValueError(f"Late onset ({onset / RATE:.3f} s) or DC offset: {path}")
    if np.any(magnitude >= .999):
        raise ValueError(f"Clipped PCM: {path}")
    return {"frames": len(pcm), "peak_dbfs": round(20 * np.log10(peak), 2),
            "rms_dbfs": round(20 * np.log10(rms), 2), "true_peak_dbtp": round(20 * np.log10(oversized), 2),
            "onset_ms": round(onset / 48, 2), "clipped_samples": 0}


def finish_cut(part, cut):
    part -= part.mean(axis=0)
    magnitude = np.max(np.abs(part), axis=1)
    onset = int(np.flatnonzero(magnitude > magnitude.max() * .10)[0])
    if onset > int(RATE * MAX_ONSET[cut["event"]] * .90):
        advance = onset - round(.009 * RATE)
        part = np.pad(part[advance:], ((0, advance), (0, 0)))
        part -= part.mean(axis=0)
    part[:120] *= np.linspace(0, 1, 120)[:, None]
    part[-240:] *= np.linspace(1, 0, 240)[:, None]
    part = level(part, cut["peak_db"])
    true_peak = np.max(np.abs(resample_poly(part, 4, 1, axis=0)))
    return part * min(1, db_to_gain(-3.5) / true_peak)


def finish(root):
    delivery = root / "delivery"
    delivery.mkdir(exist_ok=False)
    files = []
    for group in REELS:
        directory = root / group
        report = json.loads((directory / "audacity/job-report.json").read_text(encoding="utf-8"))
        if report["status"] != "PASS" or report["audio_played"] or not Path(report["project"]).is_file():
            raise ValueError(f"Missing native Audacity master: {group}")
        source = read_wave(directory / "unmastered.wav")
        master = read_wave(Path(report["output"]))
        if source.shape != master.shape or np.max(np.abs(source - master)) < .005:
            raise ValueError(f"Unexpected change in timing or no compression: {group}")
        for cut in json.loads((directory / "cuts.json").read_text(encoding="utf-8")):
            start = cut["start"]
            part = master[start:start + cut["frames"]].copy()
            target = delivery / cut["runtime"]
            write_wave(target, finish_cut(part, cut))
            files.append({"path": target.name, "source": str(target), "sha256": sha(target),
                          "role": "creature-" + cut["event"], **analyze(target, cut["event"])})
        print(f"Mastered and checked {group}", flush=True)
    expected = {name + suffix + ".wav" for name in profiles() for suffix in SUFFIXES}
    if {item["path"] for item in files} != expected or len({item["sha256"] for item in files}) != len(expected):
        raise ValueError("Final bank is incomplete or has duplicate takes")
    write_json(root / "manifest.json", {"schema": 1, "id": root.name, "files": files,
                                       "provenance": {"model": LOCAL_MODEL, "license": MODEL_LICENSE,
                                                      "generation": str(root / "generation.json"),
                                                      "audacity_projects": [str(root / key / f"creatures-{key}.aup3") for key in REELS]}})
    print(f"PASS: {len(files)} stereo PCM16 takes at 48 kHz", flush=True)


def deploy(root, asset_roots):
    manifest = json.loads((root / "manifest.json").read_text(encoding="utf-8"))
    mapping = {}
    for item in manifest["files"]:
        source = Path(item["source"])
        if not source.is_relative_to(root / "delivery") or sha(source) != item["sha256"]:
            raise ValueError(f"Delivery changed: {source}")
        mapping[Path(item["path"]).stem] = source
    expected = {name + suffix for name in profiles() for suffix in SUFFIXES}
    if set(mapping) != expected:
        raise ValueError("Deployment requires the entire verified creature sound bank")
    for asset_root in asset_roots:
        audio = asset_root / "audio"
        if not (audio / "production.json").is_file() or any(not (audio / (name + ".wav")).is_file() for name in expected):
            raise ValueError(f"Incomplete or older Emberfall asset tree: {asset_root}")
    completed = []
    try:
        for asset_root in asset_roots:
            bound = bind_audio(mapping, asset_root=asset_root)
            completed.append(bound["receipt"])
            if any(sha(record["target"]) != record["sha256"] for record in bound["records"]):
                raise ValueError(f"Bound files differ from reviewed masters: {asset_root}")
    except Exception:
        for receipt in reversed(completed):
            restore_bindings(receipt)
        raise
    write_json(root / "binding-receipts.json", completed)
    print(f"Deployed {len(mapping)} files to {len(completed)} assets trees", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("prepare", "finish", "deploy"))
    parser.add_argument("root", type=Path)
    parser.add_argument("--no-audio", action="store_true", required=True)
    args = parser.parse_args()
    root = args.root.resolve()
    if args.action == "deploy":
        deploy(root, (REPO / "assets", REPO / "dist/assets"))
    else:
        {"prepare": prepare, "finish": finish}[args.action](root)

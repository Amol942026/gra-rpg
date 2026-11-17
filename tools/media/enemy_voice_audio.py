"""
Design and render the 22 enemy sound sets as a silent 48 kHz stereo reel.

Original Csound synthesis, local CC0 foley and the three existing ACE-Step
foley sessions are mixed as independent layers. Keep the CSD, cut sheet and
sources with the master. Use the 'deliver' stage after a silent PCM16 export.
"""

import argparse
import json
import math
import os
from pathlib import Path
import re
import subprocess
import sys
import wave

import numpy as np

from asset_tools import sha, write_json
from audio_tools import FFMPEG
from game_import import import_pack, validate_audio

RATE = 48000
BASE = Path("C:/AI2D/audio")
PROGRAMS = Path(os.environ["LOCALAPPDATA"]) / "Programs/CodexAudio"
CSOUND = PROGRAMS / "Csound-6.18.1/build/Release/csound.exe"
EFFECTS = ("attack", "hit", "fall")
REPO = Path(__file__).resolve().parents[2]


def sample(stem, speed, gain, duration, delay=0, high=65, low=6500):
    return (1, delay, duration, stem, speed, gain, high, low)


def air(gain, high, low, duration, delay=0, rise=.014, crackle=0):
    return (2, delay, duration, gain, high, low, rise, crackle)


def call(gain, start, end, formant, duration, delay=0, rasp=24):
    return (3, delay, duration, gain, start, end, formant, rasp)


S, A, V = sample, air, call

# Each row is an authored attack, hit, and landing timeline. Numeric times are
# relative to the event, so the fall transient begins on the contact frame.
PROFILES = {
    "slime": (
        (.35, (S("handleSmallLeather", .70, .62, .25, low=1650), A(.13, 240, 2100, .25, .018, .024, 19), V(.22, 85, 39, 480, .28, .022, 18))),
        (.25, (S("impactSoft_medium_001", 1.18, .69, .15, low=2300), V(.23, 200, 66, 650, .16, .014, 27), S("handleSmallLeather2", 1.27, .24, .13, .058))),
        (.52, (S("impactSoft_heavy_001", .76, .77, .21, low=1500), V(.18, 137, 37, 440, .33, .017, 28), S("handleSmallLeather", .85, .37, .25, .100), A(.11, 175, 1250, .32, .068, .027, 12)))),
    "wolf": (
        (.37, (S("snarl-1", 1.28, .66, .31, high=125, low=5400), S("knifeSlice", .81, .31, .16, .043, 290), V(.14, 134, 82, 760, .26, .012, 29))),
        (.28, (S("impactSoft_medium_002", 1.14, .54, .15), V(.37, 327, 142, 1260, .19, .017, 34), S("snarl-2", 1.72, .24, .14, .038, 460, 4750))),
        (.53, (S("impactSoft_heavy_002", .94, .67, .18), S("earth-0", .92, .41, .23, .009), V(.29, 129, 62, 800, .36, .053, 23), S("cloth1", .91, .34, .21, .10)))),
    "skeleton": (
        (.32, (S("impactWood_light_001", 1.45, .64, .11, .012, 350), S("drawKnife2", 1.07, .32, .22), S("impactWood_light_000", 1.63, .32, .10, .072, 400), A(.08, 930, 6800, .24, 0, .026))),
        (.25, (S("impactWood_light_002", 1.28, .80, .14, high=225), S("impactWood_light_001", 1.88, .43, .11, .023, 400), S("metalClick", 1.4, .10, .09, .053, 760))),
        (.58, (S("impactWood_medium_001", 1.28, .60, .15), S("impactWood_light_000", 1.67, .52, .12, .047), S("impactWood_light_002", 1.43, .45, .12, .110), S("impactWood_light_001", 1.82, .37, .13, .180), S("earth-1", 1.08, .26, .32, .183)))),
    "stone-guardian": (
        (.53, (S("swish-10", .72, .49, .33, high=130, low=2650), S("impactMining_001", .82, .67, .25, .013, 68, 3000), A(.17, 66, 610, .29, .030, .017, 14))),
        (.38, (S("impactMining_002", .94, .83, .27), S("impactWood_heavy_000", .68, .32, .18, .014, 76, 860), S("impactMining_001", 1.47, .23, .13, .073, 850))),
        (.96, (S("impactMining_000", .70, .73, .48), S("impactPunch_heavy_002", .72, .56, .18, low=730), S("impactMining_002", .86, .44, .24, .116), S("impactMining_001", 1.17, .31, .20, .270), A(.20, 144, 3110, .66, .025, .05, 19)))),
    "boar": (
        (.39, (V(.42, 87, 117, 630, .28, 0, 37), S("snarl-0", .69, .40, .27, .014, 70, 2500), S("swish-7", .94, .40, .21, .038, 390))),
        (.31, (S("impactPunch_medium_002", .89, .67, .15, low=2500), V(.32, 190, 89, 860, .22, .021, 45), A(.15, 430, 2700, .19, .050, .023, 19))),
        (.65, (S("impactSoft_heavy_000", .74, .80, .23), S("earth-2", .75, .51, .28, .014), V(.25, 86, 42, 590, .33, .047, 31), S("cloth3", .68, .27, .27, .151)))),
    "forest-spider": (
        (.28, (S("impactWood_light_002", 2.02, .55, .08, high=820), S("knifeSlice2", 1.52, .38, .14, .027, 940), A(.22, 1900, 6600, .21, 0, .025, 35))),
        (.20, (S("impactWood_light_000", 1.62, .68, .10, high=480), S("impactSoft_medium_000", 1.51, .23, .11), A(.17, 2340, 7200, .13, .015, .010, 48))),
        (.41, (S("impactWood_light_001", 1.36, .57, .12), S("impactWood_light_002", 1.84, .35, .11, .033), S("earth-0", 1.32, .34, .19, .041), A(.16, 1300, 5750, .28, .023, .029, 35)))),
    "forest-bandit": (
        (.38, (S("drawKnife1", .96, .55, .29, high=280), S("swish-4", 1.04, .57, .22, .024), V(.26, 168, 89, 810, .22, .019, 17))),
        (.32, (S("impactPunch_medium_001", .91, .71, .18), S("cloth2", 1.14, .30, .18, .013), V(.34, 159, 66, 970, .22, .020, 25))),
        (.69, (S("impactSoft_heavy_000", .83, .72, .21), S("cloth2", .73, .58, .39, .008), V(.23, 113, 58, 810, .33, .052, 14), S("metalLatch", .95, .17, .13, .233)))),
    "treant": (
        (.55, (S("creak3", .69, .67, .37, high=140, low=4100), S("swish-12", .71, .42, .36, .030), S("impactWood_heavy_001", 1.03, .61, .25, .068, 110), A(.12, 690, 4750, .23, .114, .02, 14))),
        (.38, (S("chop", .87, .70, .25), S("impactWood_medium_000", 1.25, .43, .13, .016), S("creak1", .77, .25, .23, .060))),
        (.88, (S("impactWood_heavy_002", .64, .73, .31), S("impactSoft_heavy_002", .75, .60, .17, low=590), S("creak2", .68, .52, .52, .021), S("chop", .78, .32, .24, .166), S("earth-1", .58, .35, .34, .245)))),
    "amber-scorpion": (
        (.32, (S("impactWood_medium_001", 1.49, .63, .12, high=270), S("impactWood_light_000", 1.66, .42, .11, .050), A(.22, 1010, 5820, .25, .007, .027, 29))),
        (.28, (S("impactPlate_light_001", 1.24, .62, .13, high=450), S("impactWood_light_001", 1.44, .52, .12, .011), A(.12, 1000, 4740, .18, .023, .019, 34))),
        (.54, (S("impactWood_medium_002", 1.11, .56, .17), S("earth-2", .83, .48, .26, .012), S("impactWood_light_001", 1.19, .30, .12, .080), S("impactPlate_light_000", 1.28, .15, .14, .144), A(.10, 820, 4950, .26, .075, .024, 22)))),
    "copper-scarab": (
        (.38, (S("impactPlate_medium_002", 1.18, .59, .18, high=510), S("metalClick", 1.08, .38, .13, .034), A(.18, 880, 4350, .25, .010, .024, 52))),
        (.31, (S("impactMetal_light_001", 1.14, .63, .21, high=210), S("impactPlate_light_002", .97, .39, .15, .019), V(.12, 288, 176, 1940, .19, .036, 44))),
        (.59, (S("impactPlate_heavy_000", .97, .68, .21), S("earth-1", .72, .43, .29, .016), S("impactMetal_light_002", .93, .34, .26, .084), S("metalClick", .83, .17, .16, .191)))),
    "sand-cobra": (
        (.42, (A(.45, 1750, 7390, .38, 0, .043, 27), S("swish-1", 1.32, .43, .23, .025, 900), V(.13, 201, 96, 1670, .22, .070, 45))),
        (.29, (S("impactSoft_medium_001", 1.13, .62, .14), A(.39, 2100, 8300, .23, .012, .016, 41), S("handleSmallLeather2", 1.43, .16, .16, .030))),
        (.57, (S("impactSoft_medium_000", .77, .50, .18), S("earth-0", .72, .50, .32, .008), A(.32, 1730, 6200, .42, .028, .035, 34), S("swish-3", .72, .18, .28, .191, 560, 3400)))),
    "living-dune": (
        (.75, (A(.34, 108, 6200, .58, 0, .056, 13), S("swish-11", .63, .55, .52, .010, 250, 5790), S("earth-2", .56, .42, .39, .049), V(.26, 48, 74, 503, .51, 0, 16))),
        (.37, (S("earth-1", .81, .55, .31), S("impactSoft_heavy_002", .58, .46, .20, high=68, low=990), A(.27, 328, 5700, .34, .011, .012, 21))),
        (1.04, (S("impactSoft_heavy_000", .63, .61, .23, low=750), S("earth-2", .64, .75, .40), S("earth-0", .59, .49, .44, .199), A(.38, 380, 6650, .93, .009, .070, 15), V(.18, 61, 38, 380, .44, .015, 12)))),
    "slag-crawler": (
        (.42, (S("impactMining_002", 1.28, .64, .20, high=240), S("impactWood_medium_000", 1.18, .44, .16, .019), A(.15, 520, 2810, .27, .034, .025, 24))),
        (.31, (S("impactMining_001", 1.31, .78, .21, high=195), S("impactPlate_light_002", .92, .34, .16, .022, 610), A(.17, 1100, 4770, .22, .013, .016, 38))),
        (.61, (S("impactMining_002", 1.06, .68, .28), S("impactMining_000", 1.24, .29, .25, .09, 410), S("earth-0", 1.13, .34, .28, .044), A(.16, 460, 5250, .42, .053, .037, 24)))),
    "lava-salamander": (
        (.43, (S("snarl-2", .89, .49, .31, high=180), V(.31, 102, 154, 1130, .30, .032, 41), A(.26, 802, 5800, .32, .019, .040, 34))),
        (.29, (S("impactPunch_medium_000", 1.1, .54, .14), V(.28, 209, 81, 1300, .20, .020, 41), A(.26, 980, 6700, .24, .030, .020, 48))),
        (.58, (S("impactSoft_heavy_001", .87, .62, .19), S("earth-1", 1.02, .26, .22, .013), V(.26, 104, 44, 1070, .37, .034, 39), A(.25, 1150, 5800, .38, .088, .041, 64)))),
    "fire-imp": (
        (.43, (V(.34, 251, 128, 1610, .22, 0, 38), V(.16, 312, 78, 2160, .14, .071, 35), S("swish-9", 1.23, .50, .30, .028, 440), A(.32, 520, 7700, .32, .036, .027, 64))),
        (.28, (S("impactPunch_medium_002", 1.3, .35, .14), V(.43, 345, 140, 2110, .21, .013, 52), A(.21, 1425, 8050, .23, .029, .014, 55))),
        (.55, (S("impactSoft_medium_002", 1.1, .48, .17), V(.33, 242, 73, 1470, .28, .025, 43), A(.30, 923, 6800, .47, .023, .027, 61), S("earth-1", 1.19, .22, .22, .077)))),
    "ash-wraith": (
        (.57, (S("cloth3", .61, .45, .40, high=490), A(.34, 220, 4380, .48, .005, .074, 11), V(.31, 152, 91, 610, .45, .017, 19))),
        (.32, (S("cloth2", 1.01, .55, .23, high=490), A(.25, 420, 4750, .27, 0, .015, 15), V(.21, 213, 82, 920, .23, .018, 27))),
        (.76, (S("cloth1", .73, .69, .42, high=210), S("cloth3", .67, .46, .42, .042, 450), A(.27, 275, 4100, .68, .027, .062, 9), V(.17, 122, 52, 696, .48, .048, 17)))),
    "obsidian-sentinel": (
        (.48, (S("swish-13", .91, .52, .33, high=290), S("drawKnife3", .81, .39, .33, .023, 430), S("impactMetal_medium_000", .87, .56, .24, .053, 150, 5400))),
        (.37, (S("impactMetal_heavy_001", .91, .69, .25), S("impactMining_000", 1.16, .42, .21, .013), S("impactPlate_medium_002", .83, .26, .17, .056, 640, 5600))),
        (.92, (S("impactMetal_heavy_002", .74, .74, .31), S("impactMining_001", .96, .48, .24, .030), S("impactPlate_heavy_001", .88, .38, .31, .128), S("impactMining_002", 1.25, .24, .29, .227), A(.16, 480, 5070, .55, .071, .027, 25)))),
    "ember-drake": (
        (.65, (S("snarl-0", .77, .55, .45, high=120, low=4400), S("swish-8", .61, .38, .49, .037, 270), V(.35, 87, 52, 760, .48, .006, 33), A(.21, 625, 4900, .52, .079, .045, 54))),
        (.38, (S("impactWood_medium_000", .94, .51, .19), S("impactPunch_heavy_002", .86, .37, .17, low=1300), V(.36, 156, 59, 1000, .29, .013, 28), A(.16, 850, 5300, .25, .08, .03, 47))),
        (.85, (S("impactSoft_heavy_002", .65, .74, .25), S("cloth3", .62, .44, .56, .018, 280), V(.34, 102, 38, 721, .61, .037, 27), A(.18, 705, 4400, .56, .116, .064, 27)))),
    "varkhor": (
        (.82, (S("impactMining_000", .62, .57, .43), S("swish-12", .67, .45, .50, .040), V(.39, 59, 38, 417, .67, 0, 25), A(.29, 75, 5300, .68, .022, .041, 39))),
        (.49, (S("impactMining_002", .73, .73, .33), S("impactMetal_heavy_000", .62, .36, .27, .017), V(.26, 74, 44, 508, .35, .015, 24), A(.22, 513, 6550, .29, .064, .035, 53))),
        (1.18, (S("impactMining_001", .59, .68, .46, high=80), S("impactPunch_heavy_001", .63, .52, .21, low=620), V(.25, 58, 37, 320, .61, .025, 17), A(.35, 282, 5250, 1.10, .021, .077, 25), S("earth-0", .67, .37, .46, .257), S("impactMining_000", 1.08, .20, .20, .362)))),
    "sporekin": (
        (.45, (S("ace-sporekin-attack", 1.11, .30, .39, high=270), S("handleSmallLeather2", .93, .42, .28), V(.22, 154, 87, 810, .3, .011, 40), A(.31, 490, 5400, .37, .035, .027, 21))),
        (.32, (S("impactWood_light_000", 1.03, .49, .15, low=3750), S("ace-sporekin-hit", .91, .31, .26, .007), A(.26, 495, 3950, .22, .018, .035, 31))),
        (.64, (S("impactSoft_medium_001", .88, .57, .17), S("ace-sporekin-fall", .84, .35, .45, .012), S("earth-1", 1.14, .33, .26, .037), A(.25, 697, 5780, .52, .054, .079, 17)))),
    "dune-eye": (
        (.51, (S("ace-dune-eye-attack", 1.23, .36, .44, high=260), S("swish-11", 1.05, .43, .32, .013), V(.22, 115, 187, 1630, .43, .018, 51))),
        (.33, (S("impactWood_medium_001", 1.2, .57, .16), S("ace-dune-eye-hit", 1.13, .33, .25, .014), A(.26, 1125, 6200, .25, .018, .019, 24))),
        (.66, (S("impactMining_000", 1.13, .56, .26, high=270), S("ace-dune-eye-fall", .94, .37, .46, .010), S("earth-2", .93, .37, .29, .045), A(.22, 456, 6400, .49, .067, .060, 31)))),
    "cinderhorn": (
        (.55, (S("ace-cinderhorn-attack", .90, .36, .41, high=130), S("snarl-1", .68, .37, .40, high=93, low=3180), S("impactPunch_heavy_001", .87, .55, .17, .023), V(.33, 74, 98, 670, .40, .013, 34))),
        (.35, (S("impactWood_heavy_001", .91, .55, .19), S("ace-cinderhorn-hit", .95, .30, .28, .009), V(.23, 145, 58, 677, .24, .020, 41))),
        (.79, (S("impactSoft_heavy_002", .70, .69, .22), S("ace-cinderhorn-fall", .83, .40, .49, .012), S("impactMining_001", 1.07, .28, .24, .043), V(.21, 85, 35, 555, .39, .044, 34), S("earth-2", .79, .37, .40, .143)))),
}

ORCHESTRA = """
sr=48000
ksmps=16
nchnls=2
0dbfs=1
seed 261117
instr 1
 Sfile strget p4
 aL,aR diskin2 Sfile,p5,0,0
 aL butterhp aL,p7
 aR butterhp aR,p7
 aL butterlp aL,p8
 aR butterlp aR,p8
 kEdge linseg 0,.003,1,p3-.022,1,.019,0
 outs aL*p6*kEdge,aR*p6*kEdge
endin

instr 2
 aL rand 1
 aR rand 1
 aL butterhp aL,p5
 aR butterhp aR,p5
 aL butterlp aL,p6
 aR butterlp aR,p6
 kFray randomi .32,1,p8+5
 kEnvelope expseg .003,p7,1,p3-p7,.002
 outs (.86*aL+.14*aR)*p4*kEnvelope*kFray,(.81*aL+.19*aR)*p4*kEnvelope*kFray
endin

instr 3
 kPitch expseg p5,p3,p6
 kGrain randomi .86,1.14,p8
 kPulse oscili .48,p8*.9,1
 aReed vco2 .45,kPitch*kGrain,0
 aNoise rand .25
 aExcite = aReed*(.52+kPulse)+aNoise
 kVowel expseg p7*.65,p3*.23,p7*1.18,p3*.77,p7*.77
 aForm reson aExcite,kVowel,max(125,p7*.48),1
 aThroat butterlp aReed,440
 aBreath reson aNoise,kVowel*1.76,max(210,p7*.61),1
 kContour expseg .004,.019,1,p3*.32,.63,p3*.68-.019,.002
 aVoice = (aForm*.76+aThroat*.36+aBreath*.26)*kContour*p4
 outs aVoice,(aVoice*.96+aBreath*kContour*p4*.055)
endin
"""


def checked_process(args, log, timeout=120):
    result = subprocess.run(list(map(str, args)), capture_output=True, timeout=timeout,
                            creationflags=subprocess.CREATE_NO_WINDOW)
    log.write_bytes(result.stdout+result.stderr)
    result.check_returncode()


def read_pcm(path):
    with wave.open(str(path), "rb") as audio:
        if (audio.getnchannels(), audio.getsampwidth(), audio.getframerate()) != (2, 2, RATE):
            raise ValueError(f"Unexpected PCM format: {path}")
        data = np.frombuffer(audio.readframes(audio.getnframes()), dtype="<i2").reshape(-1, 2)
        if len(data) != audio.getnframes():
            raise ValueError(f"Incomplete PCM: {path}")
        return data.astype(np.float32)/32768


def write_pcm(path, samples):
    with path.open("xb") as stream, wave.open(stream, "wb") as target:
        target.setparams((2, 2, RATE, 0, "NONE", "not compressed"))
        target.writeframes(np.rint(samples*32767).astype("<i2").tobytes())


def name_for(species, gesture):
    if species in ("sporekin","dune-eye","cinderhorn"):
        return f"{species}-{gesture}"
    return f"{species}-voice-{gesture}"


def render(root, take=1):
    root.mkdir(parents=True, exist_ok=False)
    samples = json.loads((BASE/"emberfall-physical-20261108/layers.json").read_text(encoding="utf-8"))
    sources = {row["layer"]:(Path(row["wav"]), Path(row["source"]),row["sha256"],row["source_sha256"]) for row in samples}
    for row in json.loads((BASE/"emberfall-roll-20261109/sources.json").read_text(encoding="utf-8")):
        stem=Path(row["layer"]).stem
        if stem.startswith("earth-"):
            sources[stem]=(Path(row["layer"]),Path(row["source"]),row["sha256"],row["source_sha256"])
    regional = BASE/"region-enemies-20261111"
    model_recipe = json.loads((regional/"delivery-receipt.json").read_text(encoding="utf-8"))
    assert model_recipe["weights_license"] == "Apache-2.0"
    used={}
    rows=["f 1 0 4096 10 1"]
    cuts=[]
    start=.20
    for index,(species,gestures) in enumerate(PROFILES.items()):
        for gesture,(duration, tracks) in zip(EFFECTS,gestures):
            for track,(instrument,delay,length,*params) in enumerate(tracks):
                if delay+length>duration+.001:
                    raise ValueError(f"{species} {gesture}: a layer crosses its cut")
                if take>1:
                    direction=(-1 if (index+track+take)%2 else 1)
                    if instrument==1:
                        match=re.fullmatch(r"(.*_)(00[0-2])",params[0])
                        if match:
                            other=match[1]+f"{(int(match[2])+take-1)%3:03}"
                            if other in sources:
                                params[0]=other
                        params[1]*=1+direction*.07
                        params[2]*=1+direction*.12
                    elif instrument==2:
                        params[1]*=1+direction*.12
                        params[2]*=1+direction*.08
                        params[4]+=3*take
                    else:
                        params[1]*=1+direction*.09
                        params[2]*=1+direction*.08
                        params[3]*=1+direction*.10
                if instrument==1:
                    stem=params[0]
                    if stem not in used:
                        if stem.startswith("ace-"):
                            original=regional/"model-outputs"/f"{stem[4:]}.flac"
                            layer=root/f"{stem}.wav"
                            # Trim to the most distinct onset of the saved generated take.
                            raw=root/f"{stem}-decoded.wav"
                            checked_process([FFMPEG,"-v","error","-nostdin","-i",original,
                                             "-ar",RATE,"-ac",2,"-c:a","pcm_s16le",raw],
                                            root/f"{stem}-decode.log")
                            data=read_pcm(raw)
                            frame=240
                            energies=np.sqrt(np.mean(data[:len(data)//frame*frame].reshape(-1,frame,2)**2,axis=(1,2)))
                            rises=energies[4:-100]-energies[:-104]
                            onset=(int(np.argmax(rises))+2)*frame
                            segment=data[onset:onset+int(.72*RATE)].copy()
                            segment*=.67/max(.001,float(np.max(np.abs(segment))))
                            write_pcm(layer,segment)
                            license="ACE-Step 1.5 Turbo weights Apache-2.0; generated output reused from original Emberfall session"
                            original_sha,layer_sha=sha(original),sha(layer)
                        else:
                            layer,original,layer_sha,original_sha=sources[stem]
                            if sha(original)!=original_sha or sha(layer)!=layer_sha:
                                raise ValueError(f"Previously verified source changed: {stem}")
                            license="CC0 1.0"
                        used[stem]={"source":str(original),"sha256":original_sha,"layer":str(layer),"layer_sha256":layer_sha,"license":license}
                    params[0]=f'"{Path(used[stem]["layer"]).as_posix()}"'
                    params[2]*=.46
                else:
                    params[0]*=.46
                rows.append(" ".join(map(str,[f"i {instrument}",f"{start+delay:.6f}",f"{length:.6f}",*params])))
            target=(-8.0,-8.2,-7.1)[EFFECTS.index(gesture)]
            target+=1.0 if species in ("stone-guardian","living-dune","varkhor") else -.6 if species in ("slime","forest-spider","ash-wraith") else 0
            suffix=f"-v{take}" if take>1 else ""
            cuts.append(dict(species=species,gesture=gesture,name=name_for(species,gesture)+suffix,take=take,
                             start=round(start*RATE),frames=round(duration*RATE),peak_db=target))
            start+=duration+.105
    csd=root/"enemy-audio.csd"
    orchestra=ORCHESTRA.replace("seed 261117",f"seed {261116+take}")
    csd.write_text("<CsoundSynthesizer>\n<CsOptions>\n</CsOptions>\n<CsInstruments>\n"+orchestra+
                   "\n</CsInstruments>\n<CsScore>\n"+"\n".join(rows)+f"\nf 0 {start:.6f}\ne\n</CsScore>\n</CsoundSynthesizer>\n",
                   encoding="utf-8")
    reel=root/"unmastered-reel.wav"
    checked_process([CSOUND,"-+rtaudio=null","-+rtmidi=null","-W","-s","-d","-o",reel,csd],root/"csound.log")
    signal=read_pcm(reel)
    if np.max(np.abs(signal))>.98:
        raise ValueError("Reel clipped")
    write_json(root/"sources.json",used)
    write_json(root/"cuts.json",cuts)
    print(f"Rendered {len(cuts)} events, {len(signal)/RATE:.3f} s; peak {np.max(np.abs(signal)):.3f}")


def deliver(root, reel, imported):
    data=read_pcm(reel)
    raw=read_pcm(root/"unmastered-reel.wav")
    if data.shape!=raw.shape:
        raise ValueError("Mastering changed the event timing")
    takes=[(root,data)]
    for number in (2,3):
        folder=root/f"take-{number}"
        takes.append((folder,read_pcm(folder/"unmastered-reel.wav")))
    expected={cue["name"] for cue in json.loads((root/"cuts.json").read_text(encoding="utf-8"))}
    named_bank=re.search(r"Names\[CueCount\]\s*=\s*\{(.*?)\};",(REPO/"src/audio.hpp").read_text(encoding="utf-8"),re.S)
    all_cues=re.findall(r'"([a-z-]+)"',named_bank.group(1))
    assert len(expected)==66 and expected.issubset(all_cues) and len(all_cues)==len(set(all_cues))
    out=root/"delivery"
    out.mkdir(exist_ok=False)
    report=[]
    files=[]
    for cut,signal in ((cut,signal) for folder,signal in takes
                       for cut in json.loads((folder/"cuts.json").read_text(encoding="utf-8"))):
        lo,frames=cut["start"],cut["frames"]
        part=signal[lo:lo+frames].copy()
        assert len(part)==frames
        part-=part.mean(axis=0)
        if cut["gesture"]=="fall":
            # Cloth and other soft contacts occasionally have quiet lead-in in
            # the field recording. Align their first meaningful transient to
            # the game event while retaining the same duration and decay.
            envelope=np.max(np.abs(part),axis=1)
            threshold=float(envelope.max())*.08
            onset=np.flatnonzero(envelope>threshold)
            preroll=int(RATE*.009)
            if len(onset) and onset[0]>preroll:
                advance=onset[0]-preroll
                part[:-advance]=part[advance:]
                part[-advance:]=0
                cut["contact_trim_ms"]=round(advance/48,3)
        part[:72]*=np.linspace(0,1,72)[:,None]
        tail=round(RATE*.028)
        part[-tail:]*=np.linspace(1,0,tail)[:,None]
        peak=float(np.max(np.abs(part)))
        assert peak>.005,cut
        part*=10**(cut["peak_db"]/20)/peak
        dest=out/f'{cut["name"]}.wav'
        write_pcm(dest,part)
        validate_audio(dest)
        pcm=read_pcm(dest)
        peak=float(np.max(np.abs(pcm)))
        rms=float(np.sqrt(np.mean(pcm**2)))
        first=np.flatnonzero(np.max(np.abs(pcm),axis=1)>peak*.08)
        if not 0<rms<peak<.58 or len(first)==0 or first[0]>RATE*(.023 if cut["gesture"]=="fall" else .08):
            raise ValueError(f"Bad dynamics or late transient: {cut['name']}, {peak=}, {rms=}, first={first[:1]}")
        report.append({**cut,"onset_ms":round(first[0]/48,3),"peak_dbfs":round(20*math.log10(peak),3),
                       "rms_dbfs":round(20*math.log10(rms),3),"sha256":sha(dest)})
        files.append({"source":str(dest),"path":dest.name,"role":"enemy-"+cut["gesture"],"sha256":sha(dest)})
    assert len(report)==198 and len(set(r["sha256"] for r in report))==len(report)
    write_json(root/"measurements.json",report)
    manifest={"schema":1,"id":root.name,"files":files,"provenance":{"sources":str(root/"sources.json"),
              "scores":[str(folder/"enemy-audio.csd") for folder,signal in takes],
              "reel":str(reel),"measurements":str(root/"measurements.json"),
              "local_ace_step_session":str(BASE/"region-enemies-20261111/delivery-receipt.json"),
              "foley_downloads":str(BASE/"emberfall-physical-20261108/sources/download-receipt.json")}}
    write_json(root/"manifest.json",manifest)
    if imported:
        print(import_pack(root/"manifest.json")["status"],len(files))


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action",choices=("render","deliver"))
    parser.add_argument("root",type=Path)
    parser.add_argument("--no-audio",action="store_true",required=True)
    parser.add_argument("--reel",type=Path)
    parser.add_argument("--take",type=int,choices=(1,2,3),default=1)
    parser.add_argument("--import-pack",action="store_true")
    args=parser.parse_args()
    if args.action=="render":
        render(args.root.resolve(),args.take)
    else:
        deliver(args.root.resolve(),args.reel.resolve(),args.import_pack)


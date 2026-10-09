from pathlib import Path
import math, random, wave, struct
# Deterministic string twang and air transient. This script never plays audio.
rng=random.Random(2301);rate=44100;frames=[];filtered=0.0
for i in range(int(rate*.27)):
 t=i/rate;noise=rng.uniform(-1,1);filtered=.72*filtered+.28*noise
 twang=(math.sin(2*math.pi*(190*t-70*t*t))+.35*math.sin(2*math.pi*510*t))*math.exp(-t*23)
 air=(noise-filtered)*math.sin(math.pi*min(1,t/.09))*math.exp(-t*25)
 value=.35*twang+.21*air;frames.append(struct.pack('<h',int(max(-.9,min(.9,value))*32767)))
out=Path(__file__).resolve().parents[1]/'assets/audio/bow-shot.wav'
with wave.open(str(out),'wb') as wav:
 wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(rate);wav.writeframes(b''.join(frames))
print(str(out))

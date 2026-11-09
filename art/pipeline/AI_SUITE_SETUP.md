# Lokalne modele do Emberfall

Modele, skrypty i wyniki prób są na tym komputerze w
`C:\AI2D\pipeline\runs\media-ai-suite-20261109`. Wagi ComfyUI są
w istniejących katalogach modeli. Skrypty zapisują pliki bez odtwarzania
dźwięku. Wersje, adresy źródeł i sumy pobranych plików są w raportach
`*-models.json` i `*-weight-qa.json`. Zbiorczy odbiór prób jest w
`suite-qa.json`.

## ComfyUI

Do pracy w tle użyj `Start-ComfyWorker.ps1` z katalogu powyżej.
Uruchamia ComfyUI na `127.0.0.1:8191` w trybie offline i low VRAM,
z węzłami GGUF oraz AnimateDiff-Evolved. Ponowne uruchomienie programu
ComfyUI Desktop wczyta nowe węzły także do zwykłej instancji.

```powershell
$ai = 'C:/AI2D/pipeline/runs/media-ai-suite-20261109'
$py = 'C:/Users/Amol/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
pwsh -NoProfile -File "$ai/Start-ComfyWorker.ps1"
# Poczekaj na gotowość portu, następnie wybierz jeden model.
& $py "$ai/render-comfy.py" sdxl --prompt 'A painted fantasy game item icon'
& $py "$ai/render-comfy.py" flux --prompt 'A painted fantasy game item icon'
& $py "$ai/render-comfy.py" ace --seconds 10
& $py "$ai/render-comfy.py" animatediff
```

| Model | Konfiguracja i wynik próby |
| --- | --- |
| SDXL 1.0 | Bazowy checkpoint, obraz 512 × 512. [Model i licencja](https://huggingface.co/stabilityai/stable-diffusion-xl-base-1.0) |
| FLUX.1 Schnell | GGUF Q3_K_S, enkoder T5 Q4_K_M, cztery kroki, obraz 512 × 512. [Model](https://huggingface.co/black-forest-labs/FLUX.1-schnell) · [Wagi GGUF](https://huggingface.co/city96/FLUX.1-schnell-gguf) |
| ACE-Step 1.5 | Turbo, osiem kroków i wyłączone generowanie kodów przez model językowy dla 6 GB VRAM. Próba 10 s stereo 48 kHz, FLAC i WAV. [Model](https://huggingface.co/Comfy-Org/ACE-Step-1.5) |
| AnimateDiff | V3 SD 1.5 z DreamShaper 8, osiem różnych klatek 256 × 256. Wyniki w `comfy-output/research`. [Model do prób badawczych](https://github.com/guoyww/AnimateDiff) |

W folderze testu są grafy API, historie z ComfyUI i wygenerowane pliki.
Na RTX 3060 6 GB uruchamiaj generowanie kolejno.

## Mowa i efekty dźwiękowe

Piper TTS 1.8.0 korzysta z osobnego środowiska `piper-env` i działa
na CPU. Darkman i Gosia wygenerowali polską kwestię w WAV mono 22,05 kHz.
Podaj tekst w pliku UTF-8 i nową ścieżkę WAV.
[Piper](https://pypi.org/project/piper-tts/) · [Polskie głosy](https://huggingface.co/rhasspy/piper-voices/tree/main/pl/pl_PL)

```powershell
& "$ai/piper-env/Scripts/python.exe" "$ai/render-piper.py" --voice darkman `
    --text-file "$ai/piper-check.txt" --out "$ai/nowa-kwestia.wav" --no-audio
```

AudioLDM 2 i TangoFlux korzystają z osobnego środowiska `research-env`.
AudioLDM 2 przenosi moduły między CPU i GPU, a TangoFlux wykonuje kodowanie
tekstu na CPU. Każdy model zapisał dwusekundowy WAV, AudioLDM 2 w mono
16 kHz, a TangoFlux w stereo 44,1 kHz. Wyniki trafiają do `research`.
Ze względu na licencje używaj ich wyłącznie do prób badawczych.
[AudioLDM 2](https://huggingface.co/cvssp/audioldm2) · [TangoFlux](https://huggingface.co/declare-lab/TangoFlux)

```powershell
& "$ai/research-env/Scripts/python.exe" "$ai/render-research-audio.py" audioldm2 --seconds 2
& "$ai/research-env/Scripts/python.exe" "$ai/render-research-audio.py" tangoflux --seconds 2
```

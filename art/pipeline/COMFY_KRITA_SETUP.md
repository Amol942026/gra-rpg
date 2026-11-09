# ComfyUI i Krita — konfiguracja lokalna 09.11.2026

Comfy Desktop i lokalny serwer ComfyUI 0.44.0 widzą ten sam katalog
`%LOCALAPPDATA%/Comfy-Desktop/ComfyUI-Shared/models`. Plik
`extra_model_paths.yaml` w instalacji ComfyUI odwzorowuje konfigurację
Desktop przy uruchomieniu serwera poza aplikacją. Dla RTX 3060 6 GB
uruchamiaj serwer na `127.0.0.1` z `--lowvram --disable-auto-launch`.

| Waga | Katalog | Sprawdzona próba |
| --- | --- | --- |
| `control_v11p_sd15_canny_fp16.safetensors` | `controlnet` | DreamShaper 8 z konturem, 512 × 512 |
| `Z-Image-Turbo-Fun-Controlnet-Union-2.1-lite-2602-8steps.safetensors` | `model_patches` | Z-Image Turbo GGUF z konturem, 512 × 512 |
| `flux-2-klein-4b-Q4_K_M.gguf` | `diffusion_models` | Zmiana płonącego pocisku w lodowy na podstawie obrazu, 512 × 512 |
| `flux2-vae.safetensors` | `vae` | VAE użyty w powyższej edycji Kleina |

Z-Image i Klein używają tego samego istniejącego `Qwen3-4B-Q4_K_M.gguf`
przez ComfyUI-GGUF. Dla Z-Image pozostaje właściwy `ae.safetensors`,
a Klein używa swojego `flux2-vae.safetensors`. Pobrane pliki mają sumy
SHA-256 zgodne z plikami wydawców. Źródła i licencje są w pokwitowaniach.

Trzy grafy API, testowy obraz Emberfall, skrypt `render-comfy-local.py`
i raport `QA.json` znajdują się w `C:/AI2D/pipeline/runs/comfy-krita-20261109`.
W tej konfiguracji walidator Comfy CLI pomijał pliki z dodatkowego katalogu.
Skrypt sprawdza rzeczywisty katalog węzłów ComfyUI i wysyła graf do lokalnego
`/prompt`. Zachowuje ID, historię i zdekodowany PNG dla każdego renderu.
Próby wykonano kolejno na pojedynczych obrazach 512 × 512.

Krita z `C:/Program Files/Krita (x64)/bin/krita.exe` zgłasza wersję 5.3.4.1.
Zainstalowany `krita-codex-mcp==0.2.0` udostępnia 13 narzędzi przez STDIO.
Włączony plugin Krita Codex Bridge przyjmuje uwierzytelnione połączenia na
`127.0.0.1:17641` i ogranicza operacje plikowe do `C:/AI2D`. Sekret znajduje
się w prywatnej konfiguracji instalatora.

Codex ma wpis użytkownika `krita` uruchamiający przypięty pakiet z cache UV.
Po odświeżeniu katalogu narzędzi można użyć natywnego wpisu. W bieżącym
procesie dostęp działa przez `tools/media/krita_mcp_client.py`, który w razie
potrzeby uruchamia zainstalowaną Kritę w tle. Próba przez MCP utworzyła
osobny dokument, narysowała dwie warstwy, zapisała KRA i RGBA PNG oraz
pobrała podgląd. Log operacji i pliki są w `krita-draw-check/` w katalogu
przebiegu. Próbki integracji są poza zasobami gry.

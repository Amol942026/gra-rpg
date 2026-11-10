# Bieg bohatera i pustynne kaktusy — 10.11.2026

Synfig 1.4.5, [edytowalny rig ramion](run-arms.sif). Wymach, zgięcie
łokcia i przywiedzenie ramienia mają osobne ścieżki. Eksporter
`tools/media/run_arms_synfig.py` zapisuje 49 próbek z punktem zamknięcia pętli.
Gra stosuje je do istniejącego szkieletu i miesza z chodem. Praca nóg
i mocowanie miecza podczas biegu zachowują swój dotychczasowy cykl.

Trzy generacje img2img 448 × 448 wykonano w ComfyUI 0.44.0 na CPU.
Użyto DreamShaper 8 (Lykon) i LCM-LoRA SD 1.5 (OpenRAIL++), z weryfikacją
sum modeli względem [rejestru pochodzenia](../creatures/desert-comfy/provenance.json).
Krita dopracowała żebra i brzegi kaktusów, płatki opuncji i szwy karwaszy.
W atlasy wklejono dwie komórki `forest-desert-props.png` i jedną
`hero-paints.png`. Oba atlasy zachowują dotychczasowy rozmiar.

Przebieg `C:/AI2D/pipeline/runs/emberfall-run-cacti-20261110` zawiera
grafy ComfyUI, surowe rendery, cztero- i dwuwarstwowe mastery KRA, pokwitowania
oraz podglądy z gry. Do lokalnego procesu ComfyUI nie przekazano
`SSLKEYLOGFILE`, co umożliwiło start OpenSSL. Praca odbyła się w trybie offline.

Wyniki obejmują 48 klatek Synfiga, 249 kontroli rzeczywistego renderera
i 485 856 prób pozy miecza bez przecięcia głowy lub tułowia. Obie
rozpakowane paczki sprawdzono z ukrytym oknem i bez otwierania audio.

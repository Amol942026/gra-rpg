# Wędrowiec — 09.11.2026

Nowa twarz, zaczesane ciemnobrązowe włosy i strój podróżny z miedzianymi
naramiennikami, turkusowym płaszczem i rozdzielonymi połami kaftana.
Zbroja początkowa nadaje wygląd stroju podróżnego. Pozostałe zbroje,
płaszcze, hełmy, buty, tarcze, broń i biżuteria korzystają z faktycznie
założonych przedmiotów. Ten sam renderer wyświetla bohatera w świecie,
portrecie i scenach fabularnych.

## Materiały i ruch

ComfyUI 0.44.2, lokalne generowanie 512 × 512. FLUX.1 Schnell Q3_K_S
dostarczył projekt postaci oraz fragmenty tkaniny, skóry, włosów i rysów.
SDXL 1.0 dostarczył pasma na tył włosów. W Kricie dopracowano powieki,
tęczówki, pasma, szwy i lamówki. Gra wczytuje jeden atlas RGBA 512 × 384
(0,75 MiB pamięci tekstury). Materiał włosów ma ten sam ciemny brąz po obu
stronach głowy. Płaszcze i poły używają tekstur z atlasu.

Synfig 1.4.5 dostarczył krzywe kołysania i unoszenia płaszcza dla spoczynku,
chodu, biegu, cięcia, mocnego cięcia i turlania. Poły podążają za udami.
Zachowane są ruch i czasy bazowego rigu, mocowania broni, gesty chowania
i dobywania miecza, celowanie z łuku i upadek z powrotem do pozycji stojącej.
Gra używa tablicy wygenerowanej z plików SIF.

## Odtworzenie

- Master KRA, PNG, OpenRaster, raport i logi Krita MCP są w
  `C:/AI2D/sprites/emberfall-wanderer-20261109-v2`.
- Mastery SIF, wycinanki, podglądy i raport 354 klatek są w
  `C:/AI2D/animations/emberfall-wanderer-20261109`.
- Grafy, surowe PNG, historie i sumy modeli są w
  `C:/AI2D/pipeline/runs/media-ai-suite-20261109`. ID FLUX
  `fd4e934f-3deb-4415-a593-b56de0b536f0`, ID SDXL
  `f79a2961-4569-4eba-9095-61eb983882be`.
- Kopie wcześniejszych zasobów i paczek są w
  `C:/AI2D/pipeline/runs/wanderer-20261109/backup`.
- Skrypty produkcyjne `tools/media/wanderer_paints.py` i
  `tools/media/wanderer_motion.py` przyjmują argument `--no-audio`
  oraz nowy katalog wyjściowy.

[FLUX.1 Schnell, Apache-2.0](https://huggingface.co/black-forest-labs/FLUX.1-schnell) ·
[Wagi FLUX GGUF](https://huggingface.co/city96/FLUX.1-schnell-gguf) ·
[SDXL, CreativeML Open RAIL++-M](https://huggingface.co/stabilityai/stable-diffusion-xl-base-1.0/blob/main/LICENSE.md)

## Sprawdzenie w grze

Renderer gracza przeszedł 452 kontrole, w tym 444 klatki spoczynku, chodu,
biegu, cięcia, mocnego cięcia, naciągu i puszczenia łuku, przeturlania oraz
przewrócenia i wstawania. Sprawdzenie wyposażenia, portretu i scen dało
594/594. Logika i animacje przeszły 340 709 kontroli, zapis 19 047.
Eksport z sześciu natywnych SIF odtworzył bajtowo nagłówek użyty w grze.

Oba ZIP-y po wypakowaniu przeszły weryfikację 287 zasobów i próbę
180 klatek. Testy gry działały w tle bez otwierania urządzenia audio.
Zrzuty z renderera, animowany podgląd i raport zbiorczy są w
`C:/AI2D/pipeline/runs/wanderer-20261109`.

# Rogacz Żaru — obrót i chód

Cztery nowe ujęcia zachowują długie brązowe rogi, bazaltowe płyty, ciemną sierść i racice Rogacza. Ujęcie od tyłu w trzech czwartych pochodzi z dotychczasowej grafiki. [Ilustracje i polecenie generowania](paint/prompt.json) · [Warstwowy projekt Krity](paint/rogacz.kra)

W grze jeden kąt steruje głową, tułowiem i czterema nogami. Renderer składa pozę z atlasu 32 kierunków. Przejście pomiędzy sąsiednimi ujęciami dopasowuje obrys poszczególnych części. Kopyta wykonują cykl zsynchronizowany z przebytą drogą. W miejscu zwierzę lekko przestawia nogi pod ciężarem ciała, a podczas przygotowania szarży pochyla głowę do czasu wybiegu. Cień i wybór kursorem korzystają z aktualnej pozy. Pierwsza klatka upadku bierze kąt i fazę z chwili trafienia.

Atlas ma 1024 × 670 px i zajmuje 2 744 320 bajtów RGBA8 na GPU. Wczesne wersje gry i instalacje bez tego atlasu mogą nadal korzystać ze starszej grafiki regionalnej.

## Eksport

1. Eksportuj warstwy projektu Krity do `paint/rogacz.png`. Kolejne kolumny zawierają prawy profil, widok w trzech czwartych z przodu, przód, tył i tył w trzech czwartych. Każda warstwa ma własny wiersz.
2. Uruchom `tools/media/cinderhorn_paint.py` z `paint/rogacz.png`, katalogiem roboczym i opcjami `--prepare-film <katalog>/flow.png --no-audio`. Wczytaj APNG do [lokalnego grafu FILM](paint/film-workflow.json) i wyeksportuj 33 klatki. Bieżąca produkcja użyła tego samego modelu i architektury FILMNet na CPU.
3. Ponów polecenie z `--film <katalog klatek> --output <katalog atlasu> --header <plik nagłówka> --no-audio`. Dopasowanie obrysu i odtworzenie ostrej faktury są wykonywane w tym kroku.
4. Klucze chodu, szarży i pochylenia rogów edytuj w [projekcie Synfiga](motion/rogacz-chod.sif). Eksportuj przez `tools/media/cinderhorn_turn_synfig.py art/volcano/cinderhorn/motion/rogacz-chod.sif src/generated/cinderhorn_synfig.hpp --no-audio`.

[Krótki obrót w chodzie](C:/AI2D/pipeline/runs/rogacz-plynny-skret-20261120/rogacz-obrot.webp) · [Wyniki kontroli](C:/AI2D/pipeline/runs/rogacz-plynny-skret-20261120/report.json)

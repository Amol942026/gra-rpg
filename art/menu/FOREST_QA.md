# Leśne menu, grzyby i enty — wynik 22.10.2026

**PASS.** Zakończono wszystkie trzy poprawki. Przebudowano obie wersje EXE
i sprawdzono świeżo rozpakowany ZIP. Wszystkie uruchomienia gry i testy
przeprowadzono z `--no-audio`; urządzenie audio pozostało zamknięte,
a zapisane preferencje głośności zachowały identyczny hash.

## Rezultat

- **48 grzybów zamiast64**, czyli25% mniej. Nowe wyprawy zachowują
  pozycje pierwszych48 roślin ze starego generatora. Wczytanie starej wyprawy
  usuwa tylko nadmiarowe dzikie rośliny, także gdy bohater jest na innej mapie.
  Zebrane rośliny nie odrastają; przedmioty w plecaku pozostają zachowane.
- Enty korzystają z dłuższego wykroku, stałej długości części nóg i fazy
  liczonej z faktycznego przesunięcia po podłożu. Kierunek uwzględnia
  izometrię. Prędkości patrolu/pościgu i mechaniki walki pozostają takie same.
- Menu ma nową warstwową ilustrację Bramy Świtu wykonaną w Kricie,
  przebudowaną typografię i numerowane przyciski. Synfig opracował12s
  paralaksy, mgły i pulsowania portalu. Drobiny unoszą się w tle; przyciski
  i napisy pozostają nieruchome. Dostępne opcje i trzy sloty zapisu działają.

## Weryfikacja

| Kontrola | Wynik |
| --- | --- |
| Mechanika i animacje | 87 555 sprawdzeń, PASS |
| Zapis/odczyt | 1124 sprawdzenia, PASS |
| Interfejs, migracja starych roślin, rzeczywiste rendery | 351 sprawdzeń, PASS |
| Niezależne dekodowanie ekranów menu | 80 sprawdzeń, 20 PNG, PASS |
| KRA, źródłowy SIF, krzywe runtime i wszystkie klatki | 1053 sprawdzenia, PASS |
| Natywny renderer Synfig | 240 różnych PNG320×200,20Hz,12s |
| Animowane tło w rzeczywistym rendererze gry | 60 różnych PNG1152×720 przez12s |
| Ent w prawdziwej symulacji nad nieruchomą siatką | 120 PNG900×600,60Hz,2s |
| Koniec pełnej pętli menu vs początek | 0 zmienionych pikseli |
| Zasoby rozpakowanego ZIP-a | 101 zgodnych SHA-256 |
| Zwykły start rozpakowanego playera | 180 klatek, menu przed prologiem, brak DEV |

Testy enta obejmują8 kierunków, patrol i pościg przy30/60/120FPS.
W fazie podparcia przy ustalonym chodzie błąd przesunięcia stopy po ekranie
jest poniżej0,003px na krok symulacji. Sprawdzono również zatrzymanie fazy
bez ruchu, stałą długość nóg w241 fazach i podłączenie do `Game::update`.
Oceniono rzeczywiste pozy; test nie opiera się wyłącznie na arkuszu Synfig.

SIF ma241 zapisanych próbek, zgodnych z nagłówkiem w tolerancji1e-6.
Sprawdzono receipty SHA-256, identyczność projektu renderowanego natywnie
i kopii w repozytorium oraz poprawność warstwowego KRA (9 warstw malarskich,
w tym przezroczysta baza). Podczas animacji nie powstają nowe fonty/tekstury.
Cztery tekstury1440×900 zużywają20 736 000 bajtów RGBA8, ok.19,8MiB.
Gra interpoluje krzywe podczas odświeżania; nie odtwarza atlasu240 klatek.

## Płynność

Świeże developerskie EXE, po900 klatek na scenę, OpenGL z V-sync,
natywny bufor1688×1055, bez dźwięku:

| Scena | Średnia FPS | CPU średnio | CPU P95 |
| --- | ---: | ---: | ---: |
| Animowane menu | 60,0654 | 0,487193 ms | 0,6967 ms |
| Las | 58,6596 | 10,9376 ms | 13,8596 ms |
| Pustynia | 59,1396 | 6,65609 ms | 9,4345 ms |

Cel to60FPS; wyniki nie oznaczają gwarancji stałych60FPS każdej klatki.
Menu zmierzono przez `--menu-benchmark` bez eksportowania PNG w trakcie.
Krótki test startowy paczki zawiera kosztowny zrzut obrazu w klatce119;
jego surowa średnia54,98FPS nie zastępuje tego benchmarku menu.

## Pliki i odtwarzanie

- [Opis produkcji i skrypty](README.md).
- [Końcowy raport paczki](../../dist/qa-menu-package-20261022-174639/package-verification.json).
- [Kontrole rzeczywistego interfejsu](../../dist/qa-menu-package-20261022-174639/Emberfall/qa-menu/checks.txt).
- [Niezależna kontrola źródeł i animacji](../../dist/qa-menu-package-20261022-174639/Emberfall/qa-menu/forest-menu-verification.json).
- [Benchmarki](../../dist/qa-menu-benchmark-20261022-174211/benchmarks.json).
- [Podgląd menu](forest-preview.png), [animacja menu](forest-motion.gif),
  [ruch enta](treant-motion.gif). GIF-y są zrzutami runtime;
  podgląd menu ma5FPS, podgląd enta10FPS, bez zmiany czasu ruchu.
  Nie są pomiarem ani ograniczeniem FPS gry.

SHA-256 ZIP-a:
`d9ceba2a7e4a7b895b40402d3eaccb9804a33b48200f4f230f62d62a7def77ed`.

SHA-256 EXE gracza:
`2757fedb604166d33735a0813559829cf22e04f22c7210638562bb5719c6bf0d`.

SHA-256 EXE developerskiego:
`62bee3ffdcf78900002653f58bb4d417659e9e220c84fc4410a3c00449540055`.

Paczka nie zawiera masterów KRA/SIF, zapisów gracza ani programów QA.
Po kontroli uzupełniono nową informację o własnych grafikach w LICENSES.md
i sprawdzono zgodność dokumentów w świeżym rozpakowaniu. Oryginały przed
zmianami pozostają w `art/pipeline/backups/forest-menu-20261022`.
Bez nowych instalacji, płatnych usług, obcych obrazów i publikacji projektu.

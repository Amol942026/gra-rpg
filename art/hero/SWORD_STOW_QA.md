# Twarz i gesty miecza — 20.10.2026

Raport historyczny sprzed korekty przenikania miecza. Kontrole chwytu,
długości i płynności nie wykrywały przecięcia głowy w pośredniej pozie.
Aktualny tor i testy geometrii: [SWORD_CLEARANCE_QA.md](SWORD_CLEARANCE_QA.md).

Twarz ma przezroczystą warstwę oczu, brwi, nosa i ust, wykonaną natywnie
w Kricie. Pozostała skóra pochodzi z istniejącej głowy i zachowuje ciągłe
oświetlenie; nie ma malowanej obwódki ani osobnej płaszczyzny twarzy.
Pozostałe jedenaście komórek atlasu nie zmieniło pikseli.

Chód → bieg chowa miecz przez 0,6 s, bieg → chód go wyciąga. Zmiana kierunku
gestu kontynuuje aktualną fazę. Natywne krzywe Synfiga sterują tułowiem,
orientacją miecza i IK ręki. Po wsunięciu miecz przejmuje mocowanie przy
miednicy, a ręka wraca do biegu. Atak przerywa gest. Żaden etap nie zmienia
długości ostrza, kości ramienia ani ruchu nóg.

## Źródła i kopie

- Krita: `C:\AI2D\sprites\emberfall_hero_20261020_features_only_v4`.
- Synfig: `C:\AI2D\animations\emberfall_sword_stow_20261020_v2`.
- Repo: `hero-paints.kra`, `paint-provenance.json`, `stow/*.sif`, `stow/parts`.
- Kopia poprzednich źródeł i programu:
  `dist/qa-sheath/backup-20261020-165508`.
- PNG SHA-256:
  `b9a426d0b76e46c6ae3fbaa22df7b2a5a07465d674098606f5060d3bcc9538e7`.

## Wyniki rzeczywistych kontroli

Wszystkie uruchomienia gry i diagnostyki miały `--no-audio`.

| Kontrola | Wynik |
| --- | --- |
| Testy jednostkowe gry | 78 280 PASS |
| Twarz, przejścia miecza i wejście gracza | 724/724 PASS |
| Bohater, osiem kierunków i pełne animacje | 424/424 PASS |
| Wszystkie 39 przedmiotów wyposażenia | 364/364 PASS |
| Kierunek celowania podczas ruchu | 47/47 PASS |
| Istniejące uderzenia mieczem | 440/440 PASS |
| Łuk | 65/65 PASS |
| Sterowanie | 42/42 PASS |
| Synfig: dekodowanie i ruch | 72/72 PNG PASS |
| Zgodność krzywych SIF z C++ | dokładna |
| KRA mergedimage i runtime PNG | dokładna zgodność |
| Wszystkie inne komórki atlasu | piksele bez zmian |

Testy gestu obejmują 30/60/120 Hz, szybkie odwrócenie przejścia, pauzę,
stałą długość ramienia i ostrza, kontakt ręki z pochwą w 16 fazach kroku,
brak skoku przy przejęciu miecza przez miednicę i natychmiastową reakcję
na atak. Zapisano 180 klatek rzeczywistej gry z trzema widokami ciała.

`dist/qa-sheath/face-closeups.png` pokazuje twarz z kilku kierunków.
`dist/qa-sheath/transition-preview.gif` to trzysekundowy podgląd z klatek gry.
Natywny podgląd wycinanki SIF służy do edycji ruchu; końcowy wygląd oceniano
w rendererze gry.

## Wydajność i paczka

900 klatek benchmarku, 1440×900, synchronizacja OpenGL 60 Hz:
średnio **60,0609 fps**, CPU **6,78079 ms**, CPU p95 **7,4184 ms**,
p99 całej klatki **17,7806 ms**, maksimum **20,607 ms**.
Zero klatek powyżej 25 ms. Jest to wynik tego przebiegu, nie gwarancja dla
wszystkich lokacji i sytuacji.

`Emberfall-Windows-x64.zip` został ponownie zbudowany i rozpakowany do
`dist/qa-hero-art/package-20261020-171136/Emberfall`.
Sprawdzono hashe wszystkich 95 zasobów i EXE, 180 klatek uruchomienia
wydania gracza, brak natywnych źródeł oraz zamknięte urządzenie audio.
Na rozpakowanych zasobach ponownie przeszły kontrole bohatera 424/424
i gestów 724/724.

SHA-256 ZIP:
`f49e8b7633c77d2f35730ebb35890a0d88d73a74101fdd62c4ac871e7b11f731`.

Raporty maszynowe i logi: `dist/qa-sheath/asset-verification.json`,
`package-verification.json`, `benchmark.txt`, `tests.txt`,
`packaged-tests.txt` oraz `dist/qa-hero-art/package-verification.json`.

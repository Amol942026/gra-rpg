# Bestiariusz: wynik końcowy z 21.10.2026

Status: **PASS**. Produkcja lokalna: Krita MCP / Krita 5.3.4.1 i Synfig 1.4.5.
Oryginalne materiały; bez zewnętrznych grafik, płatnych usług i modelowania 3D.

| Kontrola | Wynik |
| --- | --- |
| KRA / atlas | 16 warstw detali, 512×512 RGBA8, 1 MiB tekstury |
| Natywny Synfig | 6 rigów, 366 próbek zgodnych z kodem, 360 poprawnych PNG przy 60 Hz |
| Końcowy renderer | 1435 kontroli, 12 gatunków, 8 kierunków, 288 póz, 60 klatek |
| Zgodność rysowania | 288 porównań GPU z poprzednią metodą: 0 różnych pikseli; sceny las/pustynia: 0/1 piksel |
| Bohater | 4 obrazy wzorcowe sprzed pracy bez zmienionych pikseli: spoczynek, chód, bieg, miecz |
| Grafika w skali świata | 873 kontrole bez błędów, w tym kursor i cienkie kończyny |
| Ciała, łupy i cienie | 40 kontroli interakcji, 490 kontroli cieni, komplet 19 nowych PNG |
| Pliki i ruch | 1165 kontroli, wszystkie gatunki mają 60 różnych klatek |
| Logika gry | 87 214 kontroli bez błędów |
| ZIP gracza | 97 zgodnych zasobów, 180 klatek uruchomienia, bez trybu deweloperskiego i źródeł natywnych |

Wszystkie uruchomienia miały `--no-audio`; urządzenie audio pozostawało zamknięte.
Nie zmieniano preferencji dźwięku użytkownika.

## Pomiar wydajności

Po 900 klatek na scenariusz, okno 1440×900, natywny bufor 1688×1055,
monitor 60 Hz, OpenGL V-sync, bez dźwięku. Pomiar dotyczy kompilacji
deweloperskiej z wyłączonym trybem deweloperskim i tym samym rendererem.

| Scena | Średnie FPS | Średni czas CPU |
| --- | --- | --- |
| Las | 58.7132 | 10.6462 ms |
| Pustynia | 59.4369 | 6.3839 ms |

To wynik bliski celowi 60 fps, a nie gwarancja stałych 60 fps w każdej klatce.
Krzywe Synfig nie są generowane podczas gry. Atlas i bufory ładowane są przed
rozgrywką; mniejsze elementy mają LOD, a scena unika czyszczenia głębi dla
każdej postaci. Nie zmieniano statystyk, łupów ani sterowania grą.

## Pakiet i dowody

ZIP: `Emberfall-Windows-x64.zip`  
SHA-256: `e3a9b355f7e9b8a592f74b126ce2bb2abc69f41182c99ccd55ae469a0052d6d7`

- [Podgląd gatunków](../../dist/qa-creatures/final/bestiary.png)
- [Animacja z rzeczywistego renderera](../../dist/qa-creatures/final/creatures-motion.gif)
- [Zbiorczy raport maszynowy](QA.json)
- [Kontrola rozpakowanego ZIP-a](../../dist/qa-creatures/package-verification.json)
- [Benchmarki](../../dist/qa-creatures/benchmarks.json)
- [Edytowalne źródła i powtarzalna produkcja](README.md)

Pełne klatki i odizolowane diagnostyki pozostają w katalogu rozpakowanego
pakietu wskazanym przez package-verification.json. Starszych chronionych
plików QA nie używa się jako dowodu nowych eksportów.

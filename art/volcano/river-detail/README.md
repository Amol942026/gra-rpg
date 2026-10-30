# Kaldera — płynąca lawa, dodatkowa scenografia i detale bestii

Zakończono 29.10.2026. Odbiór i hashe końcowe: `QA.json`.

## Produkcja

- Wyłącznie lokalne, oryginalne 2D: Krita 5.3.4.1 / istniejący natywny
  Krita MCP 0.2.0 oraz Synfig 1.4.5. Bez usług płatnych, obrazów zewnętrznych
  i wysyłania projektu.
- `tools/media/volcano_river_detail.py` otwiera prywatne kopie poprzednich
  KRA i dodaje osobne warstwy detalu dla sześciu części każdego z siedmiu
  zestawów postaci, w ośmiu kierunkach. Rysowanie jest ograniczone do
  istniejącej alfy części: kotwice stóp, siatka i proporcje pozostają spójne.
- Dodatkowy atlas zawiera cztery odmiany spękanych głazów, pniaków z trawami,
  kraterów siarkowych i runicznych monolitów. 16 nowych sprite'ów.
- `tools/media/volcano_river_motion.py` renderuje 14 projektów ruchu z
  nowymi częściami oraz nową pętlę rzeki. Lawa przesuwa się o pełny kafel
  w 60 klatkach / 30 Hz / 2 s. Wirtualna klatka 60 zamyka pętlę.
  Renderer interpoluje gotowe próbki i grupuje dwa przebiegi tekstury.
- `tools/media/verify_volcano_river_detail.py` sprawdza CRC i warstwy KRA,
  faktyczną zmianę detalu w każdym kierunku, brak pikseli poza poprzednią
  sylwetką, dokładność eksportów i każdą klatkę. Porównuje przesuniętą
  powierzchnię lawy z połowy pętli: sama zmiana jasności nie przejdzie testu.

Master KRA: `C:/AI2D/sprites/emberfall-volcano-river-detail-20261029`.
Master SIF: `C:/AI2D/animations/emberfall-volcano-river-detail-20261029`.
Staging: `C:/AI2D/pipeline/volcano-river-detail-20261029/runtime`.
Kopia poprzednich źródeł gry, zasobów i ZIP-ów:
`art/pipeline/backups/volcano-river-detail-20261029`.
Poprzednie KRA pozostają w katalogu `polish/masters` oraz C:/AI2D.

Wymagany interpreter Python i PowerShell 7 są wskazane w AGENTS.md.
Wszystkie wejścia produkcyjne wymagają `--no-audio`; renderer Synfig
pracuje sekwencyjnie na dwóch wątkach. Pillow jest używane do eksportowania
części, skalowania, masek technicznych, pakowania i kontroli, nie malowania.

## Integracja

Dwa atlasy dekoracji mają po 1440×1840 px i stałą kotwicę (90;208).
Wybór jednej z ośmiu odmian danego typu jest deterministyczny. Rysowanie,
cień i maska kursora wybierają dokładnie ten sam atlas i komórkę.
Nowe grupy dekoracji omijają szlaki, mosty, punkty interakcji i pozycje bestii.

Format zapisu nadal v10; rewizja scenografii 3. Starszy odwiedzony poziom
otrzymuje samą nową scenografię, z zachowaniem reszty pól. Prawdziwy fixture
v10 z rewizją 2 utworzono przed zmianą. Migracja jest jednorazowa i pomija
przeszkody na pozycji bohatera oraz aktualnej pozycji przeciwnika/ciała.

Ładowanie grafik, maski i klatki są przygotowywane raz. Pętla rysowania
lawy ma stały bufor stosowy, brak I/O i alokacji; odrzuca niewidoczne pasy.
Testy gry zawsze używają `--no-audio --background-test` i ukrytych okien.
Ukryty koszt ukończonego renderowania nie jest pomiarem FPS prezentacji.

## Uwaga produkcyjna o Kricie

Po zapisaniu pierwszego aktora Krita uległa awarii przy przejściu od zamknięcia
zmodyfikowanego KRA do otwierania kolejnego. Zapisane KRA/PNG i receipt były
poprawne; nie powtarzano edycji ukończonego aktora. Po sprawdzeniu procesu
i plików uruchomiono Kritę ponownie. Skrypt `--resume` sprawdza SHA-256,
korzysta z gotowych wyników i pozostawia kolejne zapisane dokumenty otwarte,
bez natywnego Close. Identyfikatory własnych dokumentów są w receipts.
Nie zamykać ani nie modyfikować niezwiązanych dokumentów użytkownika.

## Wyniki odbioru

PASS: 194309 kontroli logiki/animacji, 4090 zapisu, 1102 źródeł/eksportów,
624 renderera gracza i 257 DEV. Rzeczywiste testy obejmują 224 pozy postaci,
32 odmiany dekoracji i izolowane porównanie ruchu rzeki bez aktorów/cząsteczek.
Wektor translacji z SIF przesuwa powierzchnię o 256 px; błąd dopasowania
połowy pętli wyniósł 0,252 poziomu RGB. Kontrola samej zmiany jasności
nie wystarcza do akceptacji. Koniec pętli ma ten sam krok co środek.

Oba ZIP-y wypakowano i uruchomiono: po 137 zweryfikowanych zasobów;
180 klatek zwykłego startu gracza oraz 540 klatek startów DEV w trzech
miejscach Kaldery. Preferencje audio pozostały identyczne, urządzenia audio
nie otwarto; wszystkie okna testowe pozostały ukryte. Natywne KRA/SIF oraz
helpery nie trafiły do paczek.

Ukryty ukończony render w wypakowanej grze: średnio 5.08 ms, P95 8.01 ms. To nie jest pomiar FPS prezentacji.

Podglądy są w `previews/`. `flowing-river.gif` pochodzi z 30 rzeczywiście
wyrenderowanych klatek silnika gry; kadr jest przycięty wyłącznie do prezentacji.
Kopie nowych KRA i SIF są w `masters/` oraz `motion/`.

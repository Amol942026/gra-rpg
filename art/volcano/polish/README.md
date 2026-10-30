# Dopracowanie Kaldery Popiołu — 29.10.2026

Oryginalna grafika 2D, natywne warstwy Krita 5.3.4.1 i Synfig 1.4.5.
Bez modeli 3D, pobranych obrazów, płatnych usług i wysyłania projektu.

## Zmiany

- Ciągłe podłoże mineralne i szerokie pola popiołu zamiast szachownicy kafli.
- Ciągłe, kręte koryta lawy i cztery kamienne przeprawy zgodne z kolizjami.
- Cztery odmiany bazaltu, spalonych drzew, fumaroli i obsydianu. Wszystkie
  korzystają z właściwego wariantu grafiki, zaznaczenia kursorem i cienia.
- Skupiska pogorzeliska i skał rozmieszczone wokół szlaków. Zachowane szerokie
  przejścia, podejścia do skrzyń/kapliczek i wolna przestrzeń walki z bossem.
- Siedem nowych zestawów części w ośmiu kierunkach: sześć bestii i Varkhor.
  Rozdzielone kończyny, ogony, skrzydła, pancerz i detale. Zachowana animacja
  kroku zależna od rzeczywiście przebytej drogi i dotychczasowe parametry walk.
- Natywne pętle Synfig: żar powierzchni lawy i unoszące się opary.

## Źródła i odtwarzanie

- Krita: `C:/AI2D/sprites/emberfall-volcano-polish-v2-20261029`.
- Synfig: `C:/AI2D/animations/emberfall-volcano-polish-20261029`.
- Eksport roboczy: `C:/AI2D/pipeline/volcano-polish-20261029/runtime`.
- Kopie poprzednich grafik/kodu/ZIP: `art/pipeline/backups/volcano-polish-20261029`.
- `tools/media/polish_volcano_art.py` tworzy świeże KRA/PNG, nazwane warstwy,
  odczytuje stan i preview Krity. `--only` pozwala wznowić brakujące zasoby.
- `tools/media/polish_volcano_motion.py` tworzy i renderuje 14 klipów postaci
  oraz dwa efekty. Nagłówek gry pochodzi z dokładnie zapisanych waypointów SIF.
- `tools/media/verify_volcano_polish.py` weryfikuje natywne źródła, CRC,
  przezroczystość, kierunki, komórki, każdą klatkę i zgodność eksportów.

Używaj runtime Python z AGENTS.md oraz rzeczywistego `pwsh`. Każde z tych
wejść wymaga `--no-audio`. Renderowanie Synfig: dwa wątki, sekwencyjnie.
Pillow służy wyłącznie do skalowania, eksportowania części, pakowania atlasów,
masek technicznych i kontroli obrazu. Rysowanie odbywa się w Kricie.

## Zapisy i wydajność

Format v10 dodaje rewizję scenografii. Starsze zapisy v1–v9 są czytane.
W odwiedzonej Kalderze aktualizuje się tylko dekoracje. Ekwipunek, potwory,
łup w ciałach/skrzyniach, zużyte kapliczki, oba RNG i postęp fabuły pozostają.
Jeżeli nowa skała zajęłaby zapisane miejsce gracza, zostaje pominięta.
Prawdziwy fixture v9 powstał przed zmianą schematu; test porównuje wszystkie
pozostałe bajty starego protokołu, a nie tylko wybrane wartości.

Teren wypiekany raz przy inicjalizacji. Atlasy, maski i klatki efektów
wczytywane raz. Bez malowania, generowania map ani I/O w pętli gry.
Rysowane są widoczne fragmenty lawy. Pojedynczy atlas nie przekracza 2048².
Ukryty test mierzy czas ukończonego renderowania; nie mierzy prezentacji
obrazu i nie potwierdza stałych 60 FPS na wyświetlaczu.

## Wynik odbioru

`QA.json`: PASS. 194277 kontroli rozgrywki i animacji, 4086 kontroli
zapisu/odczytu, 1200 kontroli źródeł i eksportów. 16 poprawnych masterów KRA,
16 projektów SIF, 840 klatek postaci oraz 96 klatek lawy/oparów.

Końcowy renderer gracza: 548 kontroli, 224 widoki postaci w ośmiu kierunkach.
Sprawdzono reprezentatywne pozy wszystkich siedmiu zestawów oraz widoki
przy bramie, mostach, skupiskach obsydianu/fumaroli i bossie.
Tryb DEV: 257 kontroli interfejsu, wszystkie 19 miejsc i trzy starty Kaldery.

Oba końcowe ZIP-y wypakowano i rzeczywiście uruchomiono. Zweryfikowano
135 zasobów w każdej paczce, 180 klatek zwykłego startu gracza i 540 klatek
startów DEV oraz ponowne kontrole renderera/UI. Bez KRA/SIF i helperów
w ZIP-ach; preferencje audio pozostały identyczne. Wszystkie okna testowe
pozostały ukryte, audio nie zostało otwarte.

Końcowy koszt ukończonego renderowania w ukrytym oknie: średnio 6,30 ms,
P95 8,32 ms. To nie jest pomiar wyświetlanych FPS ani foreground frame pacing.
Hashe plików EXE/ZIP i pełne wyniki: `QA.json`. Bieżące podglądy z wypakowanej
gry są w `previews/`; `actors-review.png` porównuje próbki bezczynności,
chodu, ataku i ciała dla wszystkich gatunków.

# Bestiariusz — materiały Krita i ruch Synfig

Zakres: 12 istniejących gatunków lasu i pustyni. Żywe postacie mają obłe
sylwetki, anatomiczne stawy, łapy, pysk i zęby; szkielety zakrzywione żebra,
drzewiec korę i liście. Poprawiono także materiały ciał pozostających po walce.
Kolory, rozmiary kolizji, statystyki, łupy i mechaniki walki pozostają zgodne
z dotychczasową grą.

## Edytowalne źródła

- `creature-paints.kra`: 16 nazwanych warstw narysowanych natywnymi narzędziami
  Krita MCP 0.2.0 / Krita 5.3.4.1. Oryginalne detale, bez obcych zasobów.
- `assets/sprites/creature-paints.png`: przezroczysty atlas 512×512, RGBA8,
  1 MiB pamięci tekstury. Obejmuje sierść, kość, kamień, korę, łuski, chitynę,
  tkaninę i liście. Dodatkowe warianty rysów twarzy zachowano jako źródła.
- `motion/*.sif` i `motion/parts`: sześć natywnych projektów Synfig 1.4.5.
  To edytowalne **rigi kontrolne** kontaktu kończyn i balansu ciała;
  podgląd 2D wykorzystuje fragmenty atlasu Krita. Końcową anatomię rysuje
  istniejący renderer gry, korzystając z dokładnie tych samych krzywych.
- `src/generated/creature_motion_data.hpp`: 366 próbek wyeksportowanych
  z zapisanych waypointów SIF; płynna interpolacja w runtime. Rytm zależy od
  pokonanej odległości. Osobne cykle dla żelu, czworonogów, humanoidów,
  pajęczaków, węży i żywiołaków; przeciwnicy zachowują przygotowanie ciosu.

Oryginały produkcyjne i logi MCP:
`C:\AI2D\sprites\emberfall_creatures_20261021_v1`.
Natywne rendery 360 PNG / 60 Hz:
`C:\AI2D\animations\emberfall_creatures_20261021_v1`.
Nie użyto usług chmurowych, płatnych narzędzi ani modelowania 3D.

## Ponowna produkcja

Użyj nowego katalogu wyjściowego i interpretera z AGENTS.md:

```powershell
& $mediaPython tools/media/create_creature_paints.py --root C:\AI2D\sprites\NOWY_ZESTAW --no-audio
& $mediaPython tools/media/creature_synfig.py --root C:\AI2D\animations\NOWE_RIGI --parts C:\AI2D\sprites\NOWY_ZESTAW\parts --header src/generated/creature_motion_data.hpp --render --no-audio
pwsh -NoProfile -File tests/creature_qa.ps1
& $mediaPython tools/media/verify_creatures.py --project C:\AI2D\animations\NOWE_RIGI --qa dist/qa-creatures/runtime --no-audio
```

PNG należy skopiować do assets/sprites po kontroli, a wersję cache ciał
w corpse_art.hpp zwiększyć po zmianie ich wyglądu. Obecny atlas cache to
`assets/generated/corpses-creatures-v48.png`; poprzedni pozostaje zachowany.
Kopie plików sprzed pracy są w `dist/qa-creatures/before`.

## Wydajność i kontrola

Atlas wczytuje się raz w makeArt; brak odczytów plików i generowania zasobów
w pętli klatek. Drobne elementy używają mniejszej liczby segmentów w skali
świata, z zachowaniem gładkiego oświetlenia; zbliżenia mają pełen detal.
Geometrię współdzielą cień, obraz i wskazywanie kursorem. Rysowanie wysyła
gotowe bufory trójkątów zamiast przełączać stan GPU dla każdej powierzchni.
Bufory są przygotowane przy ładowaniu sceny (49 152 wierzchołki; ok. 2,25 MiB
łącznie kopii CPU i GPU), a dane strumieniowane bez oczekiwania na poprzedni
rysunek. Scena przypisuje postaciom osobne zakresy głębi zgodne z kolejnością
obiektów: nie trzeba czyścić bufora całego świata dla każdej kolejnej bestii.
Samodzielne podglądy zachowują czyszczenie prostokąta postaci.

Test `creature_qa` sprawdza wszystkie gatunki w 8 kierunkach i 3 pozach,
60 klatek wspólnego podglądu ruchu, zgodność kursora z widocznym ciałem,
cienie, ciała i rzeczywiste przenoszenie łupu. Porównuje też bohatera z czterema
obrazami sprzed zmian, aby zabezpieczyć jego chód, bieg i miecz.
`verify_creatures.py` porównuje każdy waypoint z kodem, odczytuje wszystkie
PNG i KRA, sprawdza domknięcie cykli i widoczny ruch każdego gatunku.
`creature_package_qa.ps1` ponawia test na świeżo rozpakowanym ZIP-ie,
sprawdza hashe zasobów i uruchomienie wersji gracza bez trybu deweloperskiego.

Wyniki i podglądy: `dist/qa-creatures/runtime`, benchmarki lasu/pustyni
i `package-verification.json` w `dist/qa-creatures`. Wszystkie uruchomienia
przez agenta wymagają `--no-audio`. Render Synfig 60 Hz sam w sobie nie
potwierdza FPS gry.

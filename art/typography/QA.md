# Wynik — PASS, 26.10.2026

Zmiana obejmuje prawdziwe odmiany Lato Bold 700 i Cinzel Bold 700 oraz
minimum 15 dla drobnego tekstu interfejsu. Oryginały fontów zachowano.

- Renderer wersji gracza: **241 kontroli, 0 błędów**.
- **57 rzeczywistych widoków** przy 1152×720, 1440×900 i 1728×1080:
  menu główne, opcje, twórcy, wczytywanie, HUD lasu, statystyki i umiejętności
  z punktami i bez, księga czarów, opisy przedmiotów, pomoc, pauza, dźwięk,
  śmierć, dialog Eldrina, handel z opisem oraz nazwy w wieży.
- **552 istniejące kontrole UI**: opisy/rarity 224, handel 147,
  statystyki 22, umiejętności 90, walka/leczenie 69. Wszystkie PASS.
- **4074 niezależne kontrole** plików fontów, obrazów i 3921 faktycznych
  wywołań rysowania tekstu. Brak wyjścia napisów poza ekran; wszystkie opisy
  przedmiotów i pełne kwestie dialogowe mieszczą się w swoich regionach.
- Pokrycie pikseli tekstu wzrosło o 41–49% dla dotychczasowego większego
  Lato Regular; nagłówków o 102–171%; drobnego tekstu o 15–30%.
  Pomiar obejmuje to samo zdanie z polskimi znakami przy trzech skalach.
- Żaden z 57 widoków nie doczytuje atlasu podczas renderowania.
  Nie dodano nakładanych kopii liter ani obrysu.

Sprawdzono wizualnie menu główne, przydzielanie punktów statystyk
i umiejętności, pomoc, sklep oraz dialog przy 1152×720.
[Przykładowy widok](preview.png).

## Wydajność

Test z ukończeniem pracy GPU, 60 klatek rozgrzewki i 180 próbek na scenę:

| Scena | Średnio (ms) | P95 (ms) |
| --- | ---: | ---: |
| Fontanna many | 7,90 | 9,25 |
| Fontanna zdrowia | 8,36 | 9,29 |
| Strażnik | 12,86 | 14,04 |
| Szczątki Wydmy | 7,17 | 7,97 |
| Statystyki | 3,91 | 4,08 |

Wszystkie P95 są poniżej 16,67 ms. To czas renderowania poza ekranem,
bez pomiaru synchronizacji prezentacji i bez gwarancji stałych FPS.
Rzeczywisty start EXE sprawdzono osobno przez 180 klatek.

## Końcowa paczka

`Emberfall-Windows-x64.zip`: **15 416 815 B**.

SHA-256 ZIP:
`b5e1c177bf9bfb46880d2b2875588bf4fbe9776ad8a4ff9832b1c8afc5657e4c`

SHA-256 EXE wersji gracza:
`1092013002da0a895d69136b4ca87a27ad991f0dee5e2f0a17ebd3db1d4f5915`

Świeże rozpakowanie:
`dist/qa-typography-package-20261026-164110/Emberfall`.
**112 identycznych zasobów**, rzeczywisty start EXE: **180 klatek**,
brak DEV i otwartego urządzenia audio. Powtórzony pełny test typografii
oraz niezależna walidacja na rozpakowanych zasobach: PASS.
Preferencje audio identyczne przed i po testach. ZIP nie zawiera źródeł
natywnych, zapisów ani pomocników testowych; pomocnik jest kopiowany
dopiero do wyodrębnionego katalogu QA.

## Materiał kontrolny

- `dist/qa-typography-20261026-v2/checks`: ujęcia, granice napisów,
  porównanie pokrycia i wyniki renderera.
- `dist/qa-typography-20261026-v2/typography-render-timing.json`:
  pomiary z ukończeniem pracy GPU.
- `dist/qa-typography-package-20261026-164110/package-verification.json`:
  wynik weryfikacji paczki.
- `art/typography/QA.json`: zbiorczy manifest, hashe i odnośniki do wyników.

Wszystkie uruchomienia gry i testów wykonano z `--no-audio`. Nie zmieniono
preferencji użytkownika, zapisów gry, mechanik ani wcześniejszych zasobów.

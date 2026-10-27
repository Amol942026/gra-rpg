# Synchronizacja chodu Kamiennego Strażnika — 27.10.2026

Poprzednia faza kroków rosła o 0,38 cyklu na pole — tak jak dla ogólnego
typu animacji — przy krótkim wykroku 7 i powiększeniu bossa 1,38. Stopy
cofały się względem ciała znacznie wolniej, niż ciało przesuwało się po ziemi.

Teraz `guardian_gait::advance` liczy fazę z **rzeczywistego przemieszczenia
po kolizjach**, rozciągnięcia osi izometrycznej, skali postaci i 64% kontaktu
zapisanych w istniejącym `art/creatures/motion/humanoid.sif`. Wykrok wynosi
18 jednostek lokalnych. Daje to około 1,84–1,92× szybszy cykl przy tej samej
prędkości świata, z pełnym dopasowaniem przesunięcia stóp w fazie podparcia.

Kolana w chodzie mają dwa stałe odcinki 17,5 i zginają się do przodu.
Przejście do tego rozwiązania używa istniejącej płynnej mieszanki chodu;
spoczynek oraz pochylenie przy rzucie zachowują wcześniejsze ustawienia.
Kierunek chodu odpowiada rzeczywistemu wektorowi ruchu. Patrol, pościg
i spowolnienie korzystają z jednej kalibracji. Brak przemieszczenia
zatrzymuje fazę kroków. Statystyki, prędkość w świecie, kolizje,
parametry głazu i zapis w formacie 7 pozostają zachowane.

Nie dodano bitmap, shaderów, nowych klatek runtime ani operacji plikowych
do pętli gry. Wykorzystywana jest istniejąca natywna krzywa Synfig;
jej plik i eksport C++ pozostają identyczne z zatwierdzonymi źródłami.

## Testy

- **108 558 kontroli logiki i animacji — PASS**, w tym 621 nowych:
  16 kierunków, 30/60/120 Hz, trzy prędkości, stałość kości,
  zatrzymanie oraz rzeczywiste tyknięcia patrolu i pościgu.
- **1891 kontroli zapisu i odczytu — PASS**, łącznie z wcześniejszymi formatami.
- Renderer wersji gracza: **1553 kontrole — PASS**.
- **192 pozy** w ośmiu kierunkach oraz **360 klatek** rzeczywistego ruchu:
  pościg, patrol, zatrzymanie i wznowienie.
- **287 pomiarów podparcia** z projekcji używanej przez renderer:
  największe przesunięcie podpartej stopy **0,000214 px** przy powiększeniu 2,2.
  Niezależny odczyt zaokrąglonego CSV: maks. **0,001 px**.
- **856 niezależnych kontroli — PASS**: oryginalny SIF, identyczny eksport
  krzywej, pomiary stóp, wszystkie 552 kompletne obrazy i rzeczywista zmiana póz.
- Rzeczywisty rzut głazem nadal dochodzi do wypuszczenia; podczas zamachu
  i powrotu nogi nie wykonują cyklu chodzenia w miejscu.

Podglądy i wyniki: `dist/qa-guardian-gait-20261027-v3/checks`.
`chase-preview.webp` to złożenie rzeczywistych klatek na nieruchomej siatce,
umożliwiającej obserwację kontaktu stóp. Brak dodatkowego audio.
[Podgląd ruchu](chase.webp), [Strażnik w grze](preview.png).

## Końcowa paczka — PASS

Nowy ZIP rozpakowano do
`dist/qa-guardian-gait-package-20261027-123309/Emberfall`.
112 zasobów ma identyczne hashe jak źródła. Rzeczywisty EXE przeszedł
180 klatek startu, a renderer powtórzył wszystkie 1553 kontrole i 552 klatki
na rozpakowanych zasobach. Niezależne 856 kontroli również przeszło.
Brak trybu DEV, otwartego audio i zmian preferencji. ZIP nie zawiera
źródeł natywnych, zapisów ani pomocników; pomocnik jest dodawany dopiero
do wyodrębnionego katalogu testowego.

SHA-256 ZIP:
`9546d291614161d52dc737225fdf9e3409a2204f4dbe4005313d005f2dd58421`.

SHA-256 EXE wersji gracza:
`d1eeeb042e77eb36ef84c0080702d8f8e43cae4dd0a6ee5a77982e547fc07da9`.

Zbiorcze wyniki i hashe źródeł: `QA.json`. Pełne wyniki rozpakowania:
`dist/qa-guardian-gait-package-20261027-123309/package-verification.json`.

Procedury:

    pwsh -NoProfile -File build.ps1 -Test
    pwsh -NoProfile -File tests/guardian_gait_qa.ps1 -NoAudio
    python tools/media/verify_guardian_gait.py --qa <katalog-checks> --no-audio --preview
    pwsh -NoProfile -File package.ps1
    pwsh -NoProfile -File tests/guardian_gait_package_qa.ps1 -NoAudio -HelperRoot <katalog-testu>

Każdy start i test gry używa `--no-audio`, bez zmiany ustawień audio użytkownika.
Poprzednia paczka i zmieniane źródła są w
`art/pipeline/backups/guardian-gait-20261027-122105`.

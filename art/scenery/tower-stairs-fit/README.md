# Schody wewnątrz obrysu wieży — 28.10.2026

Zachowano istniejącą grafikę kamiennych schodów i jej skalę. Przesunięto
schody w górę do (12; 16,5), a schody w dół do (14,5; 14,2) na wszystkich
pięciu piętrach. Cała widoczna sylwetka, również górne balustrady, mieści
się w elipsie posadzki. Nie powstały nowe ilustracje, modele ani tekstury.

Kolizja obejmuje pełny prostokątny obrys stopni, spocznika, szybu, czap
słupków i balustrad. Jest powiększona o promień stóp bohatera 0,3 pola,
z zaokrąglonymi narożnikami. Ruch w wieży jest rozwiązywany w odcinkach
do 0,08 pola, również przy biegu, przewrocie i dużej zmianie pozycji.
Na powierzchni schodów nie można chodzić; przejście E / LPM / PPM odbywa
się z wolnego podejścia. Po zmianie piętra bohater staje przed drugim biegiem.

Zapis v7 zachowuje swój format i czyta poprzednie wersje. Jeśli starszy zapis
umieszcza gracza w przesuniętej konstrukcji, menu wczytywania szuka najbliższego
wolnego miejsca, zachowując piętro, postęp i przedmioty. Zapisany plik nie jest
nadpisywany. Prawidłowa pozycja, także na balkonie, pozostaje dokładnie ta sama.

Renderer nadal używa dwóch istniejących buforów schodów, masek i cieni.
Obrys geometrii jest zapisywany do kontroli raz przy przygotowaniu buforów;
nie powstają nowe zasoby ani geometria w pętli wyświetlania.

## Testy

- 110 661 kontroli logiki/animacji i 3403 kontroli zapisów — PASS.
- 17 920 kontroli renderera i mechanik — PASS. Osiem biegów na pięciu
  piętrach, 4320 wierzchołków konstrukcji, 30 podejść z różnych stron
  i 8100 klatek chodu, biegu oraz przewrotu.
- Wszystkie 23 obrazy: 15 widoków pięter w skalach 80/100/120% i osiem
  kontaktów z konstrukcją. Sprawdzono dotychczasowe E, PPM, wybieranie myszą,
  receptury, odnowę, lunetę, schody i rozdzielenie łupu między piętrami.
- 46 niezależnych kontroli obrazów i geometrii — PASS. Najdalszy wierzchołek
  ma promień 6,278957 pola; po dodaniu stóp pozostaje wewnątrz mapy 7,2.
  Maksimum równania elipsy dla rzeczywistych pikseli schodów: 0,912304 < 1.
- Rzeczywiste menu wczytało pięć zapisów z kolidującą pozycją i bezpiecznie
  przeniosło postać. Zachowano poprawny zapis pozycji na balkonie.

Testy renderowania mierzą ukończoną pracę GPU, bez prezentacji obrazu.
W tym przebiegu p95 nie mieścił się w budżecie 16,67 ms na wszystkich
piętrach. Nie deklarujemy potwierdzenia 60 FPS; surowe czasy są w raporcie.
Ta poprawka nie dodaje wywołań rysowania ani tekstur do normalnej sceny.

## Powtarzanie

    pwsh -NoProfile -File tests/tower_stairs_fit_qa.ps1 -NoAudio
    python tools/media/verify_tower_stairs_fit.py --qa <checks> --no-audio
    pwsh -NoProfile -File tests/tower_stairs_fit_package_qa.ps1 -NoAudio -HelperRoot <folder-testu>

Źródła: `src/tower_layout.hpp`, `src/tower_system.hpp`, `src/game.hpp`
i `src/game_menu.hpp`. Kontrolowany renderer: `src/tower_stairs_art.hpp`.
Wcześniejsze źródła i ZIP zachowano w
`art/pipeline/backups/tower-stairs-20261028-110640`.
Wszystkie uruchomienia i diagnostyka wymagają `--no-audio`.

## Końcowa paczka

Świeże rozpakowanie `dist/qa-tower-stairs-fit-package-20261028-112226`:
115 zasobów z identycznymi hashami i 180 klatek uruchomienia rzeczywistego
EXE gracza — PASS. Ponowiono 17 920 + 46 kontroli geometrii i mechanik.
Brak DEV, źródeł natywnych, otwartego urządzenia audio i zmiany preferencji.
Podgląd `preview.png` pochodzi z tego testu. Pełny wynik jest w `QA.json`.

ZIP: `Emberfall-Windows-x64.zip`, 15 617 573 bajty.
SHA-256: `d7c5579d95f861ec8a2fe5da1155db3289c2c2e8f9f62b5a375227f0a69d222a`.
EXE SHA-256: `12638c45cd84b6d643cfcc0995902e6d0afdd94730081f53568e377035fda3a0`.
P95 ukończonego renderowania GPU w teście paczki: 77,48 / 30,00 / 24,57 /
30,94 / 22,32 ms dla pięter 1–5. Test funkcjonalny przeszedł, ale pomiar
wydajności nie potwierdził budżetu 60 Hz. Nie mierzono prezentacji obrazu.

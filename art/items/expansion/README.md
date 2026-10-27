# Wyprost chodu Strażnika i 36 nowych przedmiotów — 27.10.2026

## Nogi Strażnika

Wyprost wynika z fizycznego zasięgu dwóch odcinków nogi po 17,5 jednostki.
Miednica wznosi się nad podpartą stopą, aż noga staje się prosta; przy dwóch
stopach na ziemi wysokość ogranicza bardziej wysunięta noga. Stopa przenoszona
ma odpowiednie uniesienie, żeby kończyna nie była rozciągana. Istniejąca
krzywa Synfig `art/creatures/motion/humanoid.sif` nadal wyznacza wykrok,
podparcie i fazę. Kalibracja do drogi i prędkość w świecie pozostają zachowane.

Wysokość miednicy i rozwiązanie kolan wygaszają się przez istniejące
`moveBlend`, także przy rozpoczęciu rzutu głazem. Spoczynek i końcowa pozycja
zamachu zachowują poprzednie ustawienie. Nie dodano nowego modelu ani tekstury.

## Nowy łup

Katalog `catalog.json` dodaje **36 elementów wyposażenia**, po trzy do każdej
z 12 bestii/bossów. Łącznie jest **75 elementów sprzętu**, z czego 64 to
tematyczne warianty. Obecne identyfikatory zostały zachowane; nowe to 63–98.
Występują miecze, łuki, topory, sztylety, hełmy, zbroje, płaszcze, tarcze,
buty, pierścienie i amulety. Wszystkie współpracują z obecnymi slotami,
umiejętnościami, kołem broni, sprzedażą, opisami i zapisem.

Szansa każdego przedmiotu wynika nadal wyłącznie z jego rzadkości:

| Rzadkość | Szansa na każdą pozycję |
| --- | ---: |
| Pospolity | 60% |
| Rzadki | 15% |
| Bardzo rzadki | 4% |
| Niezwykle rzadki | 1% |

Losowania są niezależne i wykonywane tylko przy śmierci. Liczba pozycji
gatunku nie zmienia szans pojedynczego przedmiotu. Otwarcie ciała nie losuje
ponownie. Fabularne fragmenty bossów nadal są gwarantowane i pozostają
w piątym polu. Największa tabela ma 14 pozycji przy pojemności ciała 15.
Istniejące ciała i stare zapisy zachowują wcześniej wylosowany łup; nowe
przedmioty pojawiają się przy kolejnych zabitych bestiach.

Pełne tabele, sloty, bonusy i opisy: `LOOT.md` w katalogu głównym projektu.

## Materiały Krity

Każdy z nowych przedmiotów ma własną paletę materiałów i zdobienie, narysowane
w natywnej Kricie przez istniejące lokalne MCP. Master:
`C:\AI2D\sprites\emberfall_loot_materials_20261027_v1.kra` — 36 nazwanych
warstw oraz eksport `emberfall_loot_materials_20261027_v1.png`.
Użyto własnych motywów i kolorów, bez zewnętrznych grafik ani płatnych usług.
Można rozpowszechniać te oryginalne materiały razem z grą komercyjną.

Palety i pokrycie zdobień są odczytane z faktycznego PNG i dopisane do
`src/generated/equipment_paints.hpp`. Wszystkie poprzednie palety, kreski
i indeksy zachowano dokładnie. Runtime nie ładuje nowego atlasu ani KRA;
używa małych tablic materiałów i istniejących kształtów ekwipunku.
Źródła i hashe: `provenance.json`. Wygląd w grze jest sprawdzany w świecie
oraz portrecie, po założeniu przez rzeczywiste sterowanie ekwipunkiem.

## Procedury i testy

`tools/media/expand_beast_gear.py` zastosował katalog do kodu i tabel łupu
(odrzuca podwójne dopisywanie identyfikatorów). `create_expansion_paints.py`
tworzy warstwy Krity i kompaktowy eksport; nie zastępuje istniejących masterów.
`update_expansion_loot_doc.py` rozszerza opis gatunków i katalog w `LOOT.md`.

    pwsh -NoProfile -File build.ps1 -Test
    pwsh -NoProfile -File tests/expansion_qa.ps1 -NoAudio
    python tools/media/verify_expanded_loot.py --qa <checks> --no-audio
    python tools/media/verify_guardian_gait.py --qa <checks> --no-audio --preview
    pwsh -NoProfile -File package.ps1
    pwsh -NoProfile -File tests/guardian_gait_package_qa.ps1 -NoAudio -HelperRoot <test> -ExpandedLoot

Python: środowisko wskazane w AGENTS.md. Test wykorzystuje 192 pozy w ośmiu
kierunkach, 360 klatek ruchu, 240 rzeczywistych faz wyprostu oraz wszystkie
36 nowych przedmiotów. Dotychczasowe testy obejmują statystycznie po 30 000
zgonów każdego gatunku, wszystkie sloty, zachowanie przedmiotów i zgodność
starszych zapisów. Wszystkie starty i diagnostyka są bez dźwięku.

Wyniki: `QA.json` i końcowy dopisek poniżej. Kopia wcześniejszych źródeł
i paczki: `art/pipeline/backups/guardian-extension-loot-20261027-133708`.

## Końcowy wynik — PASS

- Logika i animacje: **110 661 kontroli**.
- Zapisy i odczyty, w tym wszystkie nowe identyfikatory i stare formaty:
  **3403 kontrole**.
- Rzeczywisty renderer wersji gracza: **1844 kontrole**; 192 pozy w ośmiu
  kierunkach, 360 klatek ruchu, 36 nowych rzeczy w świecie i portrecie,
  rzeczywiste zabicie bestii i 2× LPM do podnoszenia oraz zakładania.
- Cały katalog: **332 kontrole opisów i rzadkości**, **219 handlu**.
- Niezależnie: **856 kontroli ruchu i klatek** oraz **438 kontroli KRA,
  palet, wyglądu, identyfikatorów i docelowego wyprostu**. W 240 fazach
  zarejestrowanych z patrolu i pościgu geometria nogi podporowej osiąga
  **179,9997° lub więcej**. Kontakt stopy zachowany, maks. dryf 0,000214 px
  z obliczeń renderera / 0,001 px po odczytaniu zaokrąglonego CSV.
- Ponowne wypieczenie materiałów z zapisanego PNG daje ten sam hash nagłówka;
  wszystkie wcześniejsze palety, kreski i indeksy zachowane dokładnie.

Zrzuty i logi: `dist/qa-expansion-20261027-v2/checks`.
[Wyprost w chodzie](guardian-straight.png), [podgląd chodu](guardian-walk.webp),
[nowe przedmioty — 1](new-items-1.png), [2](new-items-2.png), [3](new-items-3.png).

Końcowy ZIP sprawdzono po świeżym rozpakowaniu do
`dist/qa-guardian-gait-package-20261027-140410/Emberfall`:
**112 zgodnych zasobów**, 180 klatek startu rzeczywistego EXE oraz
powtórzone wszystkie testy rozszerzenia i wyprostu — PASS. Brak trybu DEV,
otwartego urządzenia audio i zmian preferencji gracza. W ZIP-ie nie ma KRA,
SIF, źródeł ani testowych EXE. Pomocnik jest dodawany dopiero do katalogu QA.

SHA-256 ZIP:
`a2845557924588718efd84a4dbef4043352910308f53677b9ad0e3385cc1bc3e`.

SHA-256 EXE wersji gracza:
`2cb3f3d0d77eec3dd3495c447c9308311ba0ad698f39a3ad6742e1a186a58cb5`.

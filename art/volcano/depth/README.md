# Kaldera — zagłębione koryta lawy i zamach Wojownika

Ukończenie prac: 04.11.2026. Grafika 2D jest własna, wykonana lokalnie w
Kricie przez istniejący MCP 0.2.0. Animacje powstały i zostały wyrenderowane
w Synfig Studio 1.4.5. Nie użyto modeli 3D, zewnętrznych grafik, płatnych
usług ani przesyłania projektu do chmury.

## Wynik

- Bazaltowy Wojownik: cofnięcie miecza, skręt tułowia, podparcie tarczą,
  szerokie cięcie i powrót do gardy. Łuk samego ramienia wynosi 144 stopnie;
  tułów wykonuje dodatkowy ruch. Osiem kierunków zachowuje dotychczasowe
  części postaci. Punkty barków podążają za obrotem tułowia. Trafianie,
  obrys pod kursorem i cień korzystają z tych samych przekształceń.
- Obie rzeki płyną 38 pikseli poniżej gruntu. Skalna ściana dalszego brzegu,
  nakładany bliższy brzeg, rumowisko i światło odbite od lawy pokazują głębokość.
  Kanały zaczynają się 16 pól przed mapą i kończą 16 pól za nią. Podłoże
  pod kamerą również wychodzi poza granice gry. Mosty zachowują przejście.
- Nowe, nieregularne podłoże: zastygłe potoki, pęknięcia, osady popiołu i
  drobne okruchy. Szesnaście odmian głównej scenografii ma połamane formacje
  bazaltowe, zwęglone drzewa, osady siarki i fasetowany obsydian.
- Lawa zawiera większe fragmenty ciemnej skorupy i szerokie gorące nurty.
  Synfig przesuwa powierzchnię o pełny kafel w zamkniętej pętli.

## Źródła i odtwarzanie

`masters/` zawiera sześć finalnych KRA, PNG i potwierdzenia zapisu.
`motion/` zawiera osiem projektów ataku oraz projekt lawy z częściami PNG.
Pełne sekwencje 480 klatek Wojownika i 60 klatek lawy pozostają lokalnie w
`C:\AI2D\animations\emberfall-caldera-final-20261104`.

Etapy produkcji:

1. `tools/media/caldera_depth_art.py`: natywna scenografia i makro podłoża.
   Źródła: `C:\AI2D\sprites\emberfall-caldera-depth-20261030`.
2. `tools/media/caldera_natural_surfaces.py`: finalne podłoże, brzegi,
   ściana i lawa po ocenie pierwszego ujęcia w grze. Źródła:
   `C:\AI2D\sprites\emberfall-caldera-natural-20261104`.
3. `tools/media/caldera_depth_motion.py`: osiem natywnych SIF i kompilacja
   zapisanych krzywych do `src/generated/basalt_swing.hpp`.
4. `volcano_river_motion.flowing_lava`: finalny render nowej powierzchni
   do atlasu `assets/volcano/lava-flow.png`. Zachowany projekt i komplet klatek
   są w finalnym katalogu animacji; wcześniejszy render pozostaje archiwalny.
5. `tools/media/verify_caldera_depth.py`: niezależny odczyt KRA/SIF/PNG,
   porównanie rzeczywistych punktów Synfig z nagłówkiem C++, kontrola alfa,
   marginesów miecza, ruchu lawy i budżetów tekstur. Wymaga `--no-audio`.
6. `tests/volcano_qa.ps1 -NoAudio`: prawdziwy renderer gracza, zachowane
   interakcje i nowe ujęcia koryt oraz zamachu. `tests/volcano_package_qa.ps1`
   sprawdza finalny wypakowany ZIP, pliki i ukryte uruchomienie.

Dokumenty Krita po zapisie celowo pozostają otwarte; nie wywołujemy zamykania
edytowanego KRA. Receipt zapisuje ID zachowanego dokumentu. Każda kolejna
produkcja używa nowego katalogu, a wznowienie sprawdza zapis i SHA-256 przed
ewentualnym powtórzeniem pracy. Archiwalne i odrzucone powierzchnie pozostają
w katalogach roboczych, nie są źródłem finalnych plików runtime.

## Odbiór i wydajność

Wyniki: `asset-verification.json`, `previews/summary.json` i końcowy `QA.json`.
PASS: 790 kontroli źródeł i eksportów, 708 prawdziwego renderera,
194309 logiki i animacji oraz 4090 zapisu. Oba finalne ZIP-y wypakowano
i sprawdzono po 139 zasobów. Gracz: 180 klatek uruchomienia i 708 kontroli
renderera/interakcji; DEV: trzy punkty startowe Kaldery, łącznie 540 klatek.
Ustawienia audio pozostały niezmienione. Ukończony render paczki gracza miał
średnią 4,79 ms i p95 5,18 ms w sześciu widokach.
`previews/` zawiera ujęcia z rzeczywistego renderera, planszę ośmiu kierunków,
animację lawy i spowolniony podgląd zamachu. Nie są to osobne makiety grafiki.

Ściana wymaga dodatkowej tekstury 512 × 256 RGBA8 (0,5 MiB). Gotowe
czworokąty powstają przy wczytywaniu; bliższe ściany są rysowane jednym
zgrupowanym przebiegiem. Nie dodano kolejnego bufora całej mapy, generowania
grafiki ani odczytu plików w pętli klatki. Pozostaje interpolacja gotowych
klatek lawy. Atlasy mieszczą się w limicie 2048 pikseli na oś.

Wszystkie testy działały bez audio i w ukrytych oknach. Pomiary oznaczają czas
ukończenia renderowania z synchronizacją GPU; nie mierzą prezentacji okna,
V-sync ani foreground FPS. Nie dowodzą stałych 60 FPS podczas normalnej gry.

Kopie poprzednich zasobów i paczek:
`art/pipeline/backups/caldera-depth-20261030` oraz
`art/pipeline/backups/caldera-depth-20261104`. Format zapisu pozostaje v10,
rewizja scenografii 3. Nie wymieniano grafiki pozostałych gatunków ani fabuły.

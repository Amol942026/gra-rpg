# Bezpieczne zatrzymanie na prośbę użytkownika — 22.10.2026

**ZAKOŃCZONE po wznowieniu22.10.2026.** Wszystkie etapy wykonano,
zbudowano i sprawdzono nową paczkę. Bieżący wynik:
`art/menu/FOREST_QA.md` oraz `art/menu/FOREST_QA.json`.
Poniższy zapis opisuje historyczny punkt pauzy, a nie prace oczekujące.

## Aktualne polecenie

1. Trochę zmniejszyć liczbę grzybów w lesie.
2. Dopasować rytm stóp entów do ich rzeczywistego przemieszczania.
3. Upiększyć menu startowe i dodać animowane tło.

Użytkownik poprosił o chwilowe przerwanie. Nie kontynuować produkcji,
kompilacji ani testów do kolejnego polecenia wznowienia.

## Zapisane i wykonane

- Przeczytano AGENTS.md, ASSET_PIPELINE.md, STYLE.md, REPORT.md oraz
  dokumentację art/creatures i art/menu. Wszystkie uruchomienia gry i QA
  muszą nadal przekazywać --no-audio. Bez płatnych usług i publikacji.
- Zastano niezacommitowane, ukończone zmiany poprzedniego menu i zapisu gry.
  Zachować je. Nie resetować repozytorium ani jego binariów.
- Kopia plików sprzed tego zadania (w tym działającego ZIP-a):
  art/pipeline/backups/forest-menu-20261022.
- src/game.hpp: MushroomCount 64 -> 48 (25% mniej). Dodano
  limitForestForage(), który redukuje stare dzikie rośliny o ID >48,
  zachowując zebrane rośliny, ekwipunek i nasiona. Obejmuje aktywny las
  i forestState, gdy zapis powstał na pustyni/w wieży.
- src/game_menu.hpp: odczyt wywołuje limitForestForage() przed podmianą sesji.
- src/treant_gait.hpp: długość kroku 22 zamiast 7 jednostek lokalnych,
  faza liczona z rzeczywistego izometrycznego dystansu, skala 1.18,
  podparcie 70% z istniejącej krzywej Synfig; dokładne ustawienie względem
  kierunku ruchu oraz IK kolan o stałej długości kończyn.
- src/game.hpp i src/forest_creatures.hpp korzystają z tej korekty wyłącznie
  dla entów (typ7 / species8). Pozostałe gatunki i bohater bez zmian ruchu.
- tests/treant_tests.hpp dodano i podłączono do game_tests.cpp; rozszerzono
  mushroom_tests.hpp o migrację starszych zapisów, uaktualniono mushroom_qa.
- Rzeczywiście skompilowano i wykonano testy logiki:
  **PASS: 87555 gameplay and animation checks**, z --no-audio.
  Testy kontaktu stóp obejmują 8 kierunków, patrol/pościg, 30/60/120 FPS,
  błąd kontaktu <0.003 px w podparciu i stałą długość obu części nóg.

## Nowe materiały — zakończone rzeczywiste operacje

- Ilustracja wykonana natywnie przez istniejące Krita MCP w Kricie 5.3.4.1:
  C:\AI2D\sprites\emberfall_menu_20261022_v2.
  forest-gate.kra ma warstwy nieba, dalekiego lasu, podłoża, portalu,
  kamiennego łuku, pierwszego planu, mgły i światła. poster.png oraz
  background.png / foreground.png / mist.png / glow.png sprawdzone.
  Wszystkie PNG 1440x900 RGBA. Tło A=255, warstwy przezroczyste poprawne;
  mist maks. alfa43, glow31. Wszystko oryginalne, bez zewnętrznych zasobów.
  krita_state przed edycją i krita_preview po edycji wykonane.
  Własny dokument został wyeksportowany i zamknięty bez dotykania cudzych.
- Skrypt: tools/media/create_menu_forest.py. Obsługuje --resume według
  receiptów ukończonych operacji; nie ponawia mutacji. Wcześniejsze błędy
  kształtu poza kadrem i DRAW_NO_CHANGE naprawione. Nie uruchamiać ponownie
  na zakończonym katalogu; rezultat ma provenance.json ze statusem PASS.
- Synfig 1.4.5 rzeczywiście wyrenderował **240 PNG**, 320x200, 20Hz,
  pełna pętla12s, dwa wątki. Master i rendery:
  C:\AI2D\animations\emberfall_menu_20261022_v2.
  Podkatalog renders/forest-menu_20261022_162320_212_cce148.
  Projekt forest-menu.sif; przesuwanie planów, mgły i pulsowanie portalu.
  tools/media/menu_synfig.py eksportuje odczytane z ZAPISANEGO SIF krzywe.
  src/generated/menu_motion_data.hpp ma 241 próbek i interpolację runtime.
  Projekt SHA256: a74a97fa68a90ee40e553e3071bcfe5720fc66eb454870e304bb26a602b908d5.
  Header SHA256: 02aef7df3bc40024bb2a6432a6bdc27c5a6e23ac0aca8a1b933df63b6479e841.
- Cztery runtime PNG skopiowano do assets/menu. Master/provenance do
  art/menu/forest, SIF z częściami i receiptem do art/menu/motion.

## Podłączenie menu — zapisane, jeszcze nie sprawdzone graficznie

- src/menu_scene.hpp: cztery tekstury wczytywane raz, paralaksa/mgła/blask
  według krzywych Synfig, ograniczone48 drobin. Pętla12s. Ok.19.8MiB RGBA8,
  każda tekstura <2048. Awaryjnie używa istniejącego obserwatorium.
- makeArt() wczytuje menu_scene, a normalne zamknięcie je zwalnia.
- visual_qa.hpp rysuje nowe tło w gałęzi welcome, zachowując poprawną alfę.
- game_menu.hpp: nowe pozycje przycisków startowych, numerowane wiersze
  i subtelne zaznaczenie, większe logo, czytelniejsza kompozycja z portalem.
- typography.hpp: preloading rozmiarów23 i92, bez tworzenia fontów w klatce.
- main.cpp: deweloperski --menu-benchmark (900 klatek bez kosztownego PNG)
  zostawia menu aktywne i animuje menuClock; raport menu-benchmark.txt.
  Zwykły player nie obsługuje tej flagi. Pozostałe benchmarki bez zmian.
- Pełna gra ani helper menu NIE zostały jeszcze zbudowane po tych zmianach.
  dist/Emberfall.exe, dist/player/Emberfall.exe i główny ZIP pozostają
  ostatnią działającą wersją poprzedniego zadania. Nowy jest tylko game_tests.exe.

## Następne kroki po wznowieniu

1. Rozszerzyć tests/menu_qa.cpp: sprawdzić menu_scene::ready(), 60 klatek
   rzeczywistego menu przez12s i koniec pętli, stabilne ID tekstur/fontów,
   odczyt starszego zapisu z64 roślinami ->48 bez utraty zebranych przedmiotów.
   Dodać sekwencję enta poruszającego się nad nieruchomą siatką/podłożem,
   korzystając z game.update i rzeczywistego actor_art::draw(species8).
   Dwie próby apply_patch zakończyły się odmową dopasowania (długie linie);
   **żadna z planowanych zmian menu_qa.cpp nie została jeszcze zastosowana**.
   Użyć mniejszych, dokładnych fragmentów lub kontrolowanej edycji Python.
2. Zbudować build.ps1 -Test, aby zsynchronizować też assets/menu do dist.
   Następnie tests/menu_qa.ps1 z NOWYM OutputName, wszystko --no-audio.
   Zobaczyć prawdziwe menu i ruch enta; poprawić jeśli wymagają dopracowania.
3. Niezależnie porównać krzywe SIF z headerem, odczytać240 natywnych PNG,
   wszystkie runtime PNG i KRA. Obecny tools/media/verify_menu.py sprawdza
   stary zestaw19 screenów; dodatkowe klatki trzymać w osobnych podfolderach.
   Można dopisać osobny verify_forest_menu.py oraz podgląd GIF z renderów.
4. Rozszerzyć tests/menu_benchmark.ps1 o --menu-benchmark oraz odczyt
   menu-benchmark.txt i oczekiwanie Main menu open:1. Dotychczasowa wersja
   bada tylko forest/desert. Zmierzyć wydajność animowanego menu i lasu.
5. Nowe build/package, świeże rozpakowanie i menu_package_qa.ps1 -NoAudio.
   Zweryfikować wszystkie zasoby i aktualny ZIP, nie stare raporty.
6. Zaktualizować art/menu/README.md i QA, źródła/licencje/provenance,
   README główne, opis grzybów i poprawki enta. Nie zastępować starych QA
   nowymi wynikami przed ich uzyskaniem. Stary QA opisuje poprzednie menu.
7. Zachować osobny raport końcowy, nowy podgląd i ZIP. Nie commitować,
   nie publikować, nie instalować nowych narzędzi, nie odtwarzać dźwięku.

## Narzędzia i ostrożność

Python: C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe.
PowerShell7: pwsh. Kompilator: tools/w64devkit/bin/g++.exe (katalog w PATH).
Wszystkie procesy wykonujące nasze skrypty zakończyły się przed pauzą;
Krita/Synfig nie mają niedokończonego eksportu. Nie zamykać cudzych aplikacji.
C:\AI2D wymaga zatwierdzonego polecenia poza sandboxem; użytkownik wcześniej
upoważnił do lokalnej produkcji. Nie korzystać z płatnego imagegen ani chmury.

Pamięć użyta w tym zadaniu: MEMORY.md:207 (utrzymywany checkout i ponowna
weryfikacja release), 230 (cichy niezależny test rozpakowanego playera),
rollout01a0d7c9-3dc4-70e3-8bcf-46dfbef88f72. Nie modyfikowano pamięci.

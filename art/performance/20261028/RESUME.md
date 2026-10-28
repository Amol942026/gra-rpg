# Bezpieczna pauza — 28.10.2026, 13:01 UTC

Użytkownik polecił przerwać pracę i kontynuować po jego wiadomości.
Nie uruchamiać dalszych testów ani prac do czasu wznowienia. Wszystkie nasze
procesy gry, benchmarków i kompilacji zakończyły się. Nie ma aktywnego renderu
ani testu. Ostatni benchmark ukończył 900 klatek przed pauzą.

## Zadanie

Optymalizacja Emberfall do możliwie stałych 60 FPS na Ryzen 5 7530U / 16 GB /
zintegrowanym Radeonie; użytkownik zgłaszał 45–50 FPS na pustyni.
Katalog: `C:\Users\Amadeusz\Documents\GitHub\gra-rpg`.
Każde uruchomienie gry i diagnostyki MUSI mieć `--no-audio`.
Nie zmieniać preferencji audio ani ustawień Windows, nie zamykać aplikacji
użytkownika. Nie publikować, nie commitować ani nie pushować.

## Stan kodu

- Bohater i obrysy używają preładowanego bufora GPU; rysowanie cieni, modeli
  i wybieranie kursorem współdzieli aktualny model. Bufory geometrii są
  ponownie używane, z limitem 64 zatrzymanych modeli.
- Portret wyposażenia jest buforowany; gotowe piksele RGBA kopiowane bez
  ponownego mieszania alfy. `src/portrait_cache.hpp`.
- Statyczne mapy i warstwy kompozycji mają bufory bez nieużywanej głębi.
  `src/render_targets.hpp`. Aktywna scena i postacie zachowują głębię.
- Tło pustyni pomija całkowicie zasłonięte warstwy i buforuje nieruchomą
  kamerę. Nie zatrzymuje animacji wody/pyłu.
- Test obwiedni odrzuca odległe przeszkody przed wcześniejszym dokładnym
  testem kołowym w `game.hpp` i `enemy_ai.hpp`. Zachowuje wyniki kolizji.
- Domyślna prezentacja w `src/main.cpp` to DXGI (`openGL=false`). Istnieje
  `--opengl`, automatyczny fallback oraz weryfikacja rzeczywistych pikseli
  GPU. NIE zmodyfikowano finalnie `sync_present.cpp/.hpp` — eksperymenty
  z kolejką, waitable swapchain i triple buffering wycofano do stanu backupu.
  Nie zmieniono też `display_ui.hpp` ani trybu borderless.
- `visual_qa.hpp` rysuje LAS I PUSTYNIĘ przy skali ekranu >1.001 do osobnego
  bufora świata 945 × 810 (skala .9). HUD, napisy i etykiety są natywne.
  To świadomy kompromis ostrości świata, około 44% mniej pikseli przy 1080p.
  `--native-world` przywraca pełną rozdzielczość. Wieża/menu/chamber bez zmian.
- Zmiany schodów z poprzedniego zadania są częścią zastanego dirty checkout.
  Należy je zachować, nie resetować całego repozytorium.

## Zakończone testy

- `pwsh -NoProfile -File build.ps1 -Test`: 158661 kontroli logiki/animacji
  (w tym 48000 porównań starego i nowego algorytmu kolizji), 3403 zapisów — PASS.
- Aktualny renderer obu otwartych poziomów:
  `dist/qa-render-optimization-20261028-fields/checks/summary.json` —
  803 kontroli, 0 błędów, 219 par postaci/przeciwników, 75 odmian sprzętu,
  396 prób kursora, identyczne piksele HUD. Audio nieotwarte.
  Wszystkie 219 par modeli GPU vs dawny renderer mają 0 zmienionych pikseli.
- `dist/qa-performance60-default-20261028/qa-v19/refresh-tests.txt`:
  193 kontroli prezentacji i F11, 0 błędów, rzeczywista ścieżka DXGI.
  Ten przebieg poprzedzał rozszerzenie bufora .9 z pustyni również na las;
  presenter nie zmieniał się, ale końcowy test można powtórzyć po wznowieniu.
- Kompilacja aktualnego developer EXE, player EXE, pakowanie oraz końcowy
  helper FPS zakończone. `dist/qa-performance60-release-20261028/performance60_qa.exe`
  jest z aktualnych źródeł, z manifestem DPI i flagą HIGHDPI. Nie uruchomiono
  jeszcze tego helpera po ostatniej kompilacji.

## Rzeczywiste pomiary — nie obiecywać bezwarunkowych 60 FPS

- Przed zmianami, 900 klatek / 9 obszarów pustyni / walka:
  `dist/qa-performance60-actual-before-20261028/desert-benchmark-v43.txt`:
  46.1731 FPS, p95 28.3627 ms, 219 klatek >25 ms. Okno aktywowano ręcznie
  przez Computer Use. EXE sprzed optymalizacji zachowane w tym folderze.
- Zmodyfikowana gra, DXGI i bufor .9 na pustyni:
  `dist/qa-performance60-actual-dxgi-20261028/desert-benchmark-v43.txt`:
  59.9115 FPS, p95 17.7206 ms, p99 18.5717 ms, max 23.1743 ms, 0 >25 ms,
  900 zsynchronizowanych prezentacji, brak occlusion, audio nieotwarte.
- Powtórzenie domyślnym startem:
  `dist/qa-performance60-default-20261028/desert-benchmark-v43.txt`:
  58.4539 FPS, p95 19.1026 ms, 7 >25 ms. W raporcie końcowym należy ujawnić
  wahania, nie wybierać wyłącznie najlepszego przebiegu.
- Wieża w tym samym folderze: 59.543 FPS, p95 18.1858 ms, 1 >25 ms.
- Las nadal ma problem, także po włączeniu bufora .9:
  `dist/qa-performance60-forest-final-20261028/benchmark-v43-fullscreen.txt`:
  DXGI 47.536 FPS, p95 23.8534 ms; aktywowano jego okno przez Computer Use.
- Ostatni test, ukończony przed pauzą:
  `dist/qa-performance60-forest-opengl-20261028/benchmark-v43-fullscreen.txt`:
  OpenGL 49.7924 FPS, p95 25.3016 ms, 54 >25 ms. Audio nieotwarte.
  Zmiana backendu sama NIE rozwiązuje więc problemu lasu. Nie wdrażać
  automatycznej zmiany backendu na podstawie wcześniejszej hipotezy.
- Prawdopodobny kolejny krok: rzeczywiście sprofilować symulację/AI i budowę
  rigów/cieni lasu. `--profile --no-audio` daje etapy, lecz ma bariery glFinish
  i NIE służy do deklarowania FPS. Nowy helper ma zapytania GPU bez blokad.
  Najpierw zmierzyć koszt, nie wykonywać kolejnych przypadkowych zmian jakości.

## Pakiet — istnieje, lecz końcowy test wypakowania JESZCZE NIEZROBIONY

`Emberfall-Windows-x64.zip`, 15624436 bajtów,
SHA-256 `97c42e0c39a1b8121929fc9d6d4f67eab619acf47f8e126832e1e5b75c6356f2`.
Zawiera aktualną wersję z buforem .9 w lesie i pustyni oraz domyślnym DXGI.
Nie przedstawiać go jako całkowicie zweryfikowanej finalnej paczki.

Po dalszych poprawkach odbudować i przepakować. Przy niezmienionym kodzie:

```powershell
pwsh -NoProfile -File tests/performance60_package_qa.ps1 -NoAudio -RendererHelperRoot dist/qa-render-optimization-20261028-fields -PerformanceHelperRoot dist/qa-performance60-release-20261028 -Present -Dxgi
```

Skrypt sprawdza 115 zasobów hashami, 180 klatek startu wypakowanej wersji
gracza, zachowanie ustawień audio, brak DEV i źródeł w ZIP, ponawia 803
kontrole renderera i 7 widoków FPS. Gdy zacznie się `performance60_qa.exe`,
aktywować wyłącznie jego okno przez Computer Use; JSON rejestruje focus.
Nie kompilować ani nie wykonywać innych ciężkich zadań równolegle do pomiaru.

## Pliki i dokumentacja

- `art/performance/20261028/README.md` jest raportem ROBOCZYM: ma tabelę
  najlepszego wyniku pustyni. Przed oddaniem uzupełnić o powtórzenia, stan
  lasu, rzeczywistą walidację pakietu i ograniczenia. Nie zakładać, że raport
  oznacza zakończenie zadania.
- W tym folderze są kopie wyników pustyni, 803 kontroli, obrazy native vs
  balanced i 193 kontroli prezentacji.
- Nowe skrypty: `tests/performance60_qa.cpp/.ps1`,
  `tests/render_optimization_qa.cpp/.ps1`, `tests/performance60_package_qa.ps1`,
  `tests/performance_collision_tests.hpp`.
- README.md i instrukcja w package.ps1 opisują DXGI, bufor świata i flagi.
- `.gitignore` pomija tylko nowe lokalne `dist/qa-performance60-*` i
  `dist/qa-render-optimization-*`, żeby nie dodawać kopii wszystkich zasobów.
- Backup początkowy: `art/pipeline/backups/performance-20261028-122142`.
  Zawiera oryginalny ZIP i źródła (ze wcześniejszą naprawą schodów).
  Nie odtwarzać całego backupu ani resetować innych zmian bez potrzeby.

## Narzędzia i wznowienie

PowerShell 7: `pwsh -NoProfile -File ...`. Compiler w
`tools/w64devkit/bin/g++.exe`, raylib 5.5 w external. Interpreter:
`C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`.
Do GUI zastosowano skill computer-use, `@oai/sky` przez node_repl.
Okno FPS ma tytuł `Emberfall 60 FPS / silent QA --no-audio`.
Nie używać zrzutów ekranu podczas mierzonej pętli, bo ją zakłócają.

Pamięć była użyta: MEMORY.md:57–61 i 70–71; rollout
`01a0d7c9-3dc4-70e3-8bcf-46dfbef88f72`. Końcowa odpowiedź ma mieć wymagany
blok cytowania pamięci. Nie aktualizować samej pamięci bez nowego polecenia.

Wznowić od diagnozy pozostałego problemu FPS lasu i finalnej walidacji
pakietu. Nie powtarzać ukończonej konfiguracji narzędzi ani wcześniejszych
prac artystycznych. Do momentu polecenia wznowienia pozostać w pauzie.

# Leśna roślinność — Krita

## Drzewa — 23.10.2026

Trzy oryginalne, warstwowe ilustracje zastępują proceduralne grafiki drzew:
świerk (`type=0`), brzoza (`type=4`) i dąb (`type=5`). Mają drobne igły lub
liście, nieregularne korony, rozwidlenia, korę, mech i korzenie. Ciepłe światło
z lewej i ciemniejsze dolne gałęzie są namalowane w PNG. Nie potrzeba nowego
shadera, modelowania 3D ani map materiałowych.

- Źródła: `C:\AI2D\sprites\emberfall_forest_trees_20261023_v3`.
- Kopie masterów KRA i pochodzenie: `art/scenery/trees/`.
- Eksporty runtime: `assets/sprites/forest-{spruce,birch,oak}.png`, 540×690 RGBA.
- Kotwica: (270,624) w masterze, (90,208) w logicznych współrzędnych gry.
- `forest_tree_art.hpp` wczytuje źródła w `makeArt`, kopiuje proste RGBA do
  trzech buforów 360×460 i zwalnia źródła. Wiatr, sortowanie, zanik przed
  bohaterem, cienie i maski kursora używają tych samych buforów i kotwicy.
- Bufory drzew: 1 987 200 bajtów RGBA8; dodatkowa pamięć GPU buforów i obrysów
  wobec wersji 1×: 2 980 800 bajtów (2,84 MiB). Maski CPU: dodatkowe 372 600
  bajtów. Bez generowania grafiki lub otwierania plików w klatce gry.
- Dotychczasowe definicje proceduralne są zachowane jako fallback przy braku
  PNG. Położenia drzew, kolizje, zapis wyprawy i zasady gry są zachowane.

Ponowna produkcja do **nowych** katalogów:

```powershell
$py = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/create_forest_trees.py --root C:\AI2D\sprites\trees_base_new --no-audio
& $py tools/media/refine_forest_trees.py --source C:\AI2D\sprites\trees_base_new --root C:\AI2D\sprites\trees_finished_new --no-audio
pwsh -NoProfile -File tests/forest_trees_qa.ps1 -NoAudio
```

Pierwszy skrypt pozwala wznowić przerwane rysowanie przez `--resume`, jeżeli
jego własny dokument nadal jest otwarty. Odtwarza wyniki zapisanych poleceń,
pomija wykonane mutacje i sprawdza stan dokumentu. Nie ponawia w ciemno
mutacji po przerwanym połączeniu. Wyłącznie zgłoszone `DRAW_NO_CHANGE`
(liść o identycznym kolorze całkowicie wewnątrz istniejącej plamy) jest
traktowane jako dopuszczalny brak zmiany. Podgląd końcowy używa natywnego
rozmiaru dokumentu.

Kontrole źródeł i rzeczywistych obrazów GPU:
`tools/media/verify_forest_trees.py --qa <folder-wynikow> --no-audio`.
Raport końcowy: [TREES_QA.md](TREES_QA.md).

## Leśny krzew — 20.10.2026

`forest-bush.kra` to oryginalny master z siedmioma nazwanymi warstwami:
cień kontaktowy, ściółka, zdrewniałe pędy, podszyt, liście tylne, liście
przednie i refleksy. Grafika ma nieregularną, szeroką koronę z pojedynczymi
liśćmi, zamiast odrębnych okrągłych kęp. Pędy wyrastają z kotwicy podłoża.
Nie użyto zasobów zewnętrznych ani płatnych usług.

- Master i eksport: `C:\AI2D\sprites\emberfall_forest_bush_20261020_v1`.
- Dostarczany PNG: `assets/sprites/forest-bush.png`, 360×460 RGBA.
- Kotwica korzeni: (180,416) w źródle, czyli (90,208) po eksporcie 0,5×.
  Jest wspólna z sortowaniem, maską kursora i rzutowaniem cienia.
- Rzeczywiste granice alfa: [61,294,312,426]. Nieprzezroczyste pędy i ściółka
  przechodzą przez kotwicę. Półprzezroczysty cień nie trafia do maski kursora.
- `forest_bush_art.hpp` wczytuje źródło tylko podczas `makeArt`, wypala je
  w istniejącym `propArt[2]` w rozdzielczości 2×, po czym zwalnia źródło.
  W klatce gry pozostaje jeden zwykły sprite i istniejący cień otoczenia.
- Tekstura krzewu zajmuje 662400 bajtów RGBA8. Wzrost względem starego
  bufora 180×230: 496800 bajtów (ok. 0,47 MiB). Bez generowania grafiki i I/O
  w czasie klatki.

Ponowna produkcja natywnym Krita MCP do nowego katalogu:

```powershell
$py = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/create_forest_bush.py --root C:\AI2D\sprites\forest_bush_nowy --logs C:\AI2D\pipeline\forest_bush_nowy --no-audio
```

Sumy i UUID warstw: `forest-bush-provenance.json`.
Testy rzeczywistego renderera: `tests/face_bush_qa.ps1`.

# Malowanie Żużlowego Pełzacza

Pięć [ilustracji źródłowych](../references/sunburst-turnaround.png) wyznacza pysk z kościanym dziobem i bursztynowym okiem, układ bazaltowych płyt oraz szerokie łapy z pazurami. Wymiary poprzedniej postaci służą do ustawienia stóp i obrysu w grze. Własne piksele nowego projektu składają się na żywe ciało i wystygły pancerz po śmierci.

[Projekt Krity](../pelzacz.kra) zawiera 32 widoki ułożone w czterech rzędach. Komórka ma 160 × 160 px. Ogon, korpus z głową i cztery łapy można edytować osobno. Nazwa warstwy wskazuje także głębokość części w obrocie. Przy widoku od tyłu segmentowany ogon wychodzi przed tułów. Krótkie refleksy w bursztynowych oczach namalowano w Kricie na pośrednich ujęciach od przodu.

**Odtworzenie obrotu.** Polecenia wykonuj z głównego katalogu gry, w PowerShellu 7. Parametr `$work` wskazuje świeży katalog roboczy w `C:\AI2D`. Interpreter pochodzi z lokalnego środowiska ComfyUI.

```powershell
$py = "$env:LOCALAPPDATA/Comfy-Desktop/ComfyUI-Installs/Emberfall/standalone-env/python.exe"
$source = 'art/volcano/slag-crawler/references/sunburst-turnaround.png'
$marks = 'art/volcano/slag-crawler/paint-markers.json'
& $py tools/media/slag_crawler_paint.py $source $marks $work --prepare-film --no-audio
```

Wczytaj zapisany przez skrypt `crawler-turn-materials.png` do lokalnego ComfyUI jako animowany PNG. Zastosuj [graf FILM](film-workflow.json) ze ścieżką wejścia odpowiadającą bieżącemu zadaniu. Przebieg ma zwrócić dokładnie 33 obrazy PNG z prefiksem `turn_`. Ostatnia klatka zamyka pętlę. Wagi i model działają tylko podczas przygotowania ilustracji. Ujęcia pośrednie wykorzystują ruch pigmentu pięciu ruchomych części, natomiast położenie oka, dzioba, kła i krawędzi płyt śledzą punkty na malowanych kluczach.

```powershell
& $py tools/media/slag_crawler_paint.py $source $marks $work --film "$work/film" --output "$work/atlas" --header "$work/slag_crawler_turn_atlas.hpp" --no-audio
& $py tools/media/slag_crawler_krita.py assemble "$work/cells" "$work/master" --no-audio
```

**Wprowadzenie poprawek z KRA do gry.** Zapisz edytowany master pod nową nazwą w `C:\AI2D\sprites`. Eksport otwiera go w Kricie i odczytuje zapisane warstwy. Można powtórzyć pakowanie bez kolejnej generacji w ComfyUI.

```powershell
& $py tools/media/slag_crawler_krita.py export $editedKra "$work/native-export" --no-audio
& $py tools/media/slag_crawler_paint.py $source $marks "$work/final" --edited-cells "$work/native-export/cells" --output "$work/final/atlas" --header "$work/final/slag_crawler_turn_atlas.hpp" --no-audio
```

Po kopii zapasowej plik `parts-0.png` trafia do `assets/volcano/crawler-turn`, a tabela współrzędnych do `src/generated`. Arkusz ośmiu kluczy `slag-crawler.png` trafia do `assets/volcano`, a `slag-crawler-relief.png` do `assets/volcano/corpses/slag-crawler.png`. Ostatni plik przechowuje zmiany pigmentu i wypukłość zwłok. Po zmianie wycięć przebuduj program razem ze zaktualizowanym nagłówkiem. [Parametry, przebieg lokalny i sumy SHA-256](provenance.json)

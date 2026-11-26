# Rysunki kierunkowe przeciwników Mokradeł

## Pojedynczy rysunek na każdy z 512 kierunków

Renderer wybiera jeden kompletny rysunek najbliższy kierunkowi postaci. Próbkowanie koloru, obrys po najechaniu kursorem, cień i wskazanie postaci korzystają z tej samej prostokątnej części atlasu. Niezależna siatka o 128 kierunkach interpoluje położenia stawów i fazę chodu, ataku, trafienia lub upadku.

Materiały wytnij z rysunków źródłowych za pomocą `marsh_compass_paints.py`. W Blenderze uruchom `marsh_compass_blender.run(repo, katalog_projektu, katalog_eksportu, direct=True)`. Edytowalny plik `Mokradła-512.blend` przechowuje pełny obrót. Źródłem klatek do pakowania jest `frames` w katalogu eksportu. Wraz z nimi zachowaj odpowiadający im plik `landmarks.json` projektu Blendera do wypieku siatek ruchu.

Uruchom `tools/media/marsh_view_pack.py --frames KATALOG_KLATEK --out KATALOG_ATLASÓW` w interpreterze potoku. Wejściem jest 512 pełnych klatek RGBA w każdym z katalogów `0`–`3`, od `000.png` do `511.png`. Skrypt zapisuje kadrowane atlasy `views-*.png`, indeksy `views-*.bin`, manifest oraz osiem regularnych arkuszy z co czwartą klatką dla edytowalnych projektów akcji. Indeksy zachowują przesunięcie względem kotwicy stóp. Każdy wycinek ma własny przezroczysty margines. Podczas odbioru porównaj widoczne piksele z plikami źródłowymi oraz sprawdź zmianę kierunku podczas pełnej animacji w grze.

## Źródła ruchu i wcześniejszy obrót w 128 kierunkach

Ropucha torfowa, Pijawka denna, Błędnik trzcinowy i Dzwonnik Topieli mają osobne rysunki co 2,8125°. Każdy z obrotów powstaje w Blenderze z płaskich, teksturowanych części. Pysk, łapy, grzbiet, pancerz, wici oraz dzwon mają własne położenia i kolejność przysłaniania. Kąt 0° oznacza kierunek w prawo, 90° w stronę dolnej krawędzi ekranu. Pozycje są liczone na płaszczyźnie gruntu przed skróceniem izometrycznym.

1. Skrypt `tools/media/marsh_compass_paints.py --repo REPO --output KATALOG_ROBOCZY/parts` wycina warstwy z malowanych źródeł tego katalogu. Położenia części we wszystkich kierunkach definiuje `tools/media/marsh_compass_rig.py`.
2. Przez lokalny MCP Blendera załaduj `tools/media/marsh_compass_blender.py` i uruchom `run(REPO, KATALOG_ROBOCZY, KATALOG_ATLASÓW)`. Pełny eksport zapisuje cztery edytowalne sceny w pliku BLEND oraz po dwie strony PNG na gatunek. Strony są ułożone w siatki 8 × 8, kolejne kierunki biegną w wierszach. Zachowaj znaczniki podparcia, położenie głowy i źródła światła oraz dane ruchu powierzchni razem ze źródłem.
3. Skrypt `tools/media/marsh_direction_motion.py --input KATALOG_ATLASÓW --source KATALOG_ROBOCZY --out KATALOG_ATLASÓW` zapisuje kształt chodu, ataku, trafienia i upadku dla każdej pozy kierunkowej. W tym środowisku zależności NumPy, Pillow i SciPy są dostępne w `C:/AI2D/Tools/BiRefNet/.venv/Scripts/python.exe`. Komplet stron koloru, danych deformacji i podparcia trafia do `assets/media/mokradla-obrot`. Aby otworzyć klipy do edycji w Blenderze, załaduj `tools/media/marsh_motion_blender.py` i wywołaj `save_actions(KATALOG_ATLASÓW, ŚCIEŻKA_BLEND)`.
4. Testuj pełny obrót najpierw przy zatrzymanej fazie kroku, a następnie podczas chodu w ukrytej grze z `--no-audio --background-test`. Obserwuj pysk ropuchy, pancerz pijawki, latarnię Błędnika oraz dzwon bossa z przodu, z profilu i od tyłu. Sprawdź także przejście 127 → 0, kontakt łap z podłożem i zatrzymanie kierunku po upadku.

W spoczynkowym obrocie ustaw rozmiar malowanej części raz. Poszczególne warstwy obracają się wokół stawów i zasłaniają się zgodnie z głębokością. Eksporter Blendera sprawdza we wszystkich 128 kierunkach, czy po odjęciu przesunięcia i obrotu punkty rysunku zachowują położenie z dokładnością do 0,02 piksela. Osobno sprawdź ruch podczas chodu. Wtedy przestrzeń pomiędzy segmentami może się zginać, a odległości między detalami jednego pancerza lub dzwonu powinny być stabilne.

Warstwa ma stałą głębokość na całej powierzchni rysunku. Gdy część wychodzi zza tułowia, dolna warstwa zachowuje jej zewnętrzny obrys, a ruchoma przesłona UV odsłania ten sam rysunek z przodu. W masterze Blendera obie warstwy współdzielą siatkę i klucze pozy. Przy odbudowie atlasów zachowaj pliki `visibility-*.npz` z wypieku. Na ich podstawie skrypt ruchu zapisuje po dwie tekstury `cover` na gatunek. Trzy kanały rejestrują widoczność pigmentu przy jednej czwartej, połowie i trzech czwartych odstępu między ujęciami. Podglądy referencyjne z Blendera są w katalogu `visibility-reference` obok pliku BLEND.

## Wcześniejsze źródła ośmiokierunkowe

Numery 0–3 oznaczają kolejno Ropuchę torfową, Pijawkę denną, Błędnika trzcinowego i Dzwonnika Topieli. Pliki RGBA zawierają widok od tyłu (`back`), tylny skos (`backquarter`), lewy profil (`side`) i przód (`front`). Przedni skos pochodzi z oryginalnego arkusza gry w `assets/media/mokradla/atlas.png`.

Nowe rysunki powstały w narzędziu imagegen na podstawie tych samych czterech ilustracji z Emberfall. Warstwowy plik roboczy Krity znajduje się w `C:/AI2D/sprites/mokradla-kierunki-20261126/Mokradla-obroty.kra`.

W Blenderze załaduj moduł `tools/media/marsh_turn_blender.py`. Wywołaj `job = start(repo, repo / "art/creatures/marsh-turn", katalog_roboczy)`, potem `bake_views(job, range(5))`. Funkcja `finish(job)` zapisuje projekt z edytowalnymi pozami i teksturami.

1. Żeby wypalić wersję do gry, w Blenderze z aktywnym kontekstem GPU uruchom `tools/media/marsh_turn_bake.py` i wywołaj `bake(plik_blend, katalog_klatek)`. Każdy piksel jest próbkowany w tych samych 16 miejscach. Arkusze RGBA i położenia wierzchołków trafiają do `katalog_klatek`.
2. Uruchom `tools/media/marsh_turn_pack.py --source KATALOG_KLATEK --root REPO --report KATALOG_RAPORTU --layout-from POPRZEDNI_TURN_IMPORT_RECEIPT_JSON`. Narzędzie zachowuje skalę, oblicza podpory i położenie światła oraz przenosi geometrię do współrzędnych gotowych klatek.
3. Uruchom `tools/media/marsh_turn_warp.py --root REPO --geometry KATALOG_RAPORTU/packed-geometry.npz --output REPO/assets/media/mokradla-ruch`. Cztery mapy RGBA zawierają przesunięcia odpowiadających sobie części ciała, przebieg ich odsłaniania i korektę oświetlenia. Każdy zestaw map ma manifest z sumami odpowiadających mu atlasów koloru.
4. Uruchom `python tools/media/marsh_turn_contact.py --root REPO --stage REPO/dist/qa-local/marsh-contact`. Narzędzie oblicza podparcie widocznej sylwetki pomiędzy kierunkami i zapisuje `turn-contact.bin` oraz jego manifest. Tych punktów używają również cienie i wybór postaci kursorem.

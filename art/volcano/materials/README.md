# Kaldera Popiołu — przeciwnicy i materiały otoczenia

**Nowsze podłoże:** [portal i ciągły teren](../gateway/README.md) zastępują
opisane poniżej `ground.png` i `macro.png`. Przy odtwarzaniu tej produkcji
zachowaj te dwa aktualne eksporty z `gateway`; profil AO dla sprite'ów
tworzył ciemne brzegi na powtarzanym gruncie. Przeciwnicy, dekoracje,
ściany koryt i ich mapy pozostają aktualne. Wyniki poniżej są historyczne.

04.11.2026. Finalna grafika pochodzi z natywnej Krity 5.3.4.1 i lokalnego
Laigtera 1.14.0. Wszystkie ilustracje są własnymi materiałami projektu.
Nie użyto płatnych usług, generowania AI, zewnętrznych grafik ani modeli 3D.

## Co zostało poprawione

- Sześć gatunków oraz Varkhor: pełzacz z płytami bazaltu, łuskowana
  salamandra, chochlik z twarzą i płonącą dłonią, upiór w postrzępionym
  kapturze, wojownik z hełmem/tarczą/ostrzem oraz drak z błonami skrzydeł.
  Wykorzystano szczegółowe, natywne źródła projektu i dodano nowe warstwy
  materiałów, krawędzi, pęknięć, łusek, oczu i żaru dla wszystkich kierunków.
- 32 warianty otoczenia: bazaltowe wychodnie, obsydian, zwęglone drzewa,
  pnie, głazy, siarkowe kratery i monolity. Powierzchnie mają nowe odpryski,
  osady, słoje i światło odbite od lawy. Podłoże i ściany koryt dostały
  porowatą fakturę; duże pola popiołu i zastygłych potoków urozmaicają teren.
- Laigter wygenerował 48 map: normal, specular, occlusion i parallax dla
  12 zestawów. Normal/specular/occlusion uczestniczą w wypalaniu oświetlenia
  do finalnych PNG. Parallax pozostaje źródłem do przyszłego użycia.
- Żar oczu, serc i szczelin zachowuje jasność po wypaleniu światła. Alfa
  każdego finalnego sprite'a jest identyczna z jego nowym albedo. Obrysy
  dekoracji przeliczono z aktualnej alfy. Pozostają właściwe kotwice stóp,
  osiem kierunków i sześć części używanych przez zapisane animacje.
- Podczas przygotowania zamachu miecz wojownika jest rysowany za hełmem,
  a podczas cięcia przed ciałem. Tarcza zachowuje przednią warstwę.

Nie zmieniano kluczy animacji, statystyk, mechanik, rozmieszczenia spotkań,
stanu skrzyń/kapliczek ani fabuły. Zapis pozostaje v10, scenografia w rewizji 3.

## Źródła i odtwarzanie

`masters/`: 12 edytowalnych KRA, ich PNG i potwierdzenia natywnych zapisów.
Nowe detale postaci są na warstwach `Caldera material / <część>`.
`cutouts/`: 42 gotowe części kierunku SE dla istniejących rigów Synfig;
wszystkie kierunki pozostają w pełnych atlasach i KRA.
`maps/`: preset, indeks map i receptury wypalania z hashami.
`previews/`: rzeczywiste ujęcia renderera, pozycje w ośmiu kierunkach,
porównanie przeciwników oraz podgląd map.

Pełne lokalne materiały i logi:

- `C:\AI2D\sprites\emberfall-caldera-materials-20261104`
- `C:\AI2D\maps\emberfall-caldera-materials-height-20261104`
- `C:\AI2D\pipeline\caldera-materials-20261104\runtime-final`
- kopie wcześniejszych grafik, paczek i renderera:
  `C:\AI2D\pipeline\backups\caldera-materials-20261104`

Kolejność powtarzalnej produkcji (Python wskazany w AGENTS.md):

1. `tools/media/caldera_materials_krita.py --root <nowy-katalog-w-sprites> --no-audio`.
   Działa przez istniejący, przypięty Krita MCP, z kopią wejściowego KRA,
   stanem przed edycją, jawnymi ID dokumentów/warstw oraz natywnym preview.
2. `tools/media/caldera_materials_bake.py --paints <katalog-powyżej> --maps <nowy-katalog-map> --runtime <nowy-katalog-wyników> --preset tools/media/presets/caldera-heightmap.presets --no-audio`.
   Wywołuje rzeczywisty Laigter przez `Generate-LaigterMaps.ps1`, w trybie
   bez GUI, na prywatnych kopiach PNG. Sprawdza hash oryginału i eksportu.
3. `tools/media/verify_caldera_materials.py --paints <katalog-sprites> --runtime <katalog-wyników> --previous <kopia-poprzednich-PNG> --output <nowy-raport.json> --no-audio`.
4. Po kontroli: przenieś tylko 12 końcowych PNG i dwa obrysy do
   `assets/volcano`, z kopią i sprawdzeniem hashy. Nie importuj plików
   `*-lighting.png`, map materiałowych ani masterów do paczki gracza.
5. `tests/volcano_qa.ps1 -NoAudio`; po przygotowaniu paczki:
   `tests/volcano_package_qa.ps1 -NoAudio -HelperRoot <katalog-helpera>`.

Wznowienie weryfikuje gotowe pliki zamiast ponawiania edycji. Istniejący KRA
bez potwierdzenia zapisu wymaga inspekcji. Zmiana presetu wymaga świeżego
katalogu wypalania. Zapisane dokumenty Krita celowo pozostają otwarte;
nie używamy zamykania edytowanego KRA jako automatycznego sprzątania.

## Profil Laigtera i kontrola jakości

Domyślny tryb binarny dawał płaski parallax na ciemnych sprite'ach. Finalny
profil używa natywnego trybu HeightMap. Zweryfikowano pełne dekodowanie
wszystkich map, niezerową zmienność na widocznych obszarach, szarość map
skalarnych i długość wszystkich widocznych wektorów normalnych. Zachowano
również wcześniejsze próby w osobnym katalogu roboczym.

Format presetu sprawdzono w [oficjalnym kodzie Laigtera](https://github.com/azagaya/laigter/blob/master/gui/presets_manager.cpp),
a wybór trybu w [dokumentacji parallax](https://github.com/azagaya/laigter-docs/blob/master/docs/source/HowTo/parallax-map-generation.rst).
Każde wywołanie zostało sprawdzone na zainstalowanej wersji 1.14.0.

Wyniki są w `asset-verification.json`, `previews/summary.json` i `QA.json`.
PASS: 811 kontroli natywnych źródeł, map i eksportów; 708 renderera,
w tym 224 ujęcia siedmiu gatunków w ośmiu kierunkach oraz pełny zamach
wojownika. Wypakowana paczka gracza: 139 zasobów, 180 klatek startu i pełne
708 kontroli. Paczka DEV: 139 zasobów i 540 klatek startu w trzech lokacjach
Kaldery. Zachowano preferencje audio. Ukończony render paczki gracza:
średnia 4,41 ms / p95 4,84 ms w sześciu widokach.
Kontrola natywna obejmuje CRC KRA, zgodność z eksportem, 336 części w rigach,
różne kierunki, 32 warianty otoczenia, mapy, receptury i końcowe PNG.

Rozmiary wszystkich zastępowanych atlasów są takie same. Dla 12 głównych
tekstur alokacja RGBA8 przed i po wynosi 82 014 208 bajtów. Nie dodano
tekstur, shaderów materiałowych, przebiegów ani odczytu map w pętli gry.
Wszystkie testy gry działają z `--no-audio --background-test` w ukryciu.
Czas ukończenia renderowania nie dowodzi foreground FPS ani płynności V-sync.

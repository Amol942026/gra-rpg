# Lokalne środowisko tworzenia zasobów Emberfall

Aktualna Kaldera: [płynąca lawa, dodatkowe otoczenie i detale](../volcano/river-detail/README.md).
Produkcja natywna: `volcano_river_detail.py` (Krita), `volcano_river_motion.py`
(Synfig); niezależny odbiór: `verify_volcano_river_detail.py`.
Pierwsza wersja i mechaniki: [Kaldera Popiołu](../volcano/README.md).
`tests/volcano_qa.ps1 -NoAudio` oraz
`tests/volcano_package_qa.ps1 -NoAudio -HelperRoot <katalog helpera>`.
Serializator zapisuje v10, zachowując odczyt v1–v9; rewizja scenografii 3.

Optymalizacja renderera, testy ukryte i rozdzielczość świata/HUD:
[raport 28.10.2026](../performance/20261028/README.md).
`--no-audio --background-test` ukrywa rzeczywisty EXE; helpery renderera
używają ukrytych okien. Nie przywracać ich bez polecenia użytkownika.

Dopasowanie istniejących schodów, kolizje i bezpieczne wczytywanie pozycji:
[art/scenery/tower-stairs-fit/README.md](../scenery/tower-stairs-fit/README.md).
`tests/tower_stairs_fit_qa.ps1 -NoAudio`, `verify_tower_stairs_fit.py` oraz
`tests/tower_stairs_fit_package_qa.ps1 -NoAudio` sprawdzają obrys i mechaniki.

Balkon obserwatorium i Dolina Świtu: [art/scenery/tower-balcony/README.md](../scenery/tower-balcony/README.md).
`create_tower_balcony.py` rysuje trzy warstwowe źródła Krita; kontrola:
`tests/tower_balcony_qa.ps1 -NoAudio`, `verify_tower_balcony.py` i
`tests/tower_balcony_package_qa.ps1 -NoAudio`.

Zachodni start i rzut Kamiennego Strażnika:
[art/gameplay/guardian/README.md](../gameplay/guardian/README.md).
`create_guardian_stone.py` (Krita), `guardian_throw_synfig.py` (Synfig),
`tests/guardian_qa.ps1 -NoAudio` i `guardian_package_qa.ps1 -NoAudio`.
Serializator zapisuje format 10 (szarże dzików, trzecią krainę i rewizję scenografii) i czyta formaty 1–9.
Szarża korzysta z istniejących póz dzika, chodu zależnego od drogi i lokalnych
efektów. Testy: `tests/boar_charge_qa.ps1 -NoAudio` oraz
`tests/boar_charge_package_qa.ps1 -NoAudio -HelperRoot <katalog helpera>`.
Rozpad głazu: `guardian_fragments.py` (Krita) i `guardian_shatter_synfig.py`
(Synfig), źródła w `art/gameplay/guardian/shatter/`. Weryfikator
`verify_guardian_shatter.py` działa w ramach tych samych testów gry i ZIP-a.
Upadek i wstawanie: `hero_backfall_synfig.py` (Synfig), master w
`art/hero/knockdown`, deformacja istniejącego bohatera w `hero_knockdown.hpp`.
`verify_hero_knockdown.py` sprawdza źródło, eksport, 72 ujęcia pozy i wszystkie
wariantów wyposażenia. Nie dodaje tekstur runtime ani modeli.

Leśne drzewa: [art/scenery/README.md](../scenery/README.md),
`tools/media/create_forest_trees.py` i `refine_forest_trees.py` (natywne Krita
MCP, edytowalne KRA i przezroczyste PNG). Rzeczywisty renderer i rozpakowaną
paczkę kontrolują `tests/forest_trees_qa.ps1` i `forest_trees_package_qa.ps1`
z `-NoAudio`; niezależny odczyt KRA/PNG: `verify_forest_trees.py`.

Animowane menu leśne: [art/menu/README.md](../menu/README.md),
`tools/media/create_menu_forest.py` (Krita) i `tools/media/menu_synfig.py`.
Aktualne wyniki grzybów, chodu entów i menu: [FOREST_QA.md](../menu/FOREST_QA.md).

Bestiariusz (12 gatunków): nowe warstwy Krita, sześć rigów kontrolnych Synfig,
eksport krzywych, integracja renderera i test wersji gracza są opisane w
[art/creatures/README.md](../creatures/README.md). Wejścia produkcyjne:
`tools/media/create_creature_paints.py`, `tools/media/creature_synfig.py`;
kontrole: `tests/creature_qa.ps1`, `tools/media/verify_creatures.py`,
`tests/creature_package_qa.ps1`. Wszystkie wymagają pracy bez dźwięku.

Konfiguracja dla Windows, C++17/raylib 5.5 i C:\AI2D. Produkcja odbywa się
lokalnie, bez płatnych API i wysyłania projektu do usług. Zasady wyglądu,
animacji, audio i wydajności są w STYLE.md. Raport końcowy: REPORT.md.

## Jedno wejście do automatyzacji

Z katalogu repozytorium, w PowerShell 7:

    & '.\tools\media\Run-MediaPipeline.ps1' -Action doctor -NoAudio
    & '.\tools\media\Run-MediaPipeline.ps1' -Action demo -NoAudio

Równoważne wejście Python:

    $mediaPython = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
    & $mediaPython 'tools\media\media_pipeline.py' --no-audio doctor

doctor sprawdza rzeczywiste połączenia i wersje; demo tworzy nowy zestaw
źródeł, eksportów i raportów, a potem importuje go do nowego assets/media/<id>.
Identyfikator i katalog są unikalne. Istniejące pliki wynikowe są odrzucane.
Nie uruchamia ani nie przebudowuje gry. Kompilowany jest tylko mały helper
dekodowania zasobów przez tę samą bibliotekę raylib.

Główny Python ma już Pillow i NumPy. Produkcyjne integracje LMMS, Csound oraz
Audacity używają biblioteki standardowej; pakowanie i analiza korzystają z tych
dwóch istniejących bibliotek. Nie instalowano dodatkowego serwera MCP.

W Codex zapis do C:\AI2D i dostęp do pulpitu/potoków wymagają zatwierdzonego
wykonania poza izolacją systemu plików. Użytkownik upoważnił do tych operacji
w ramach tego zadania. Brak procesu w sandboxie nie dowodzi zatrzymania aplikacji.

## Programy i integracje

| Program | Sprawdzona wersja | Lokalizacja i metoda |
| --- | --- | --- |
| Krita | 5.3.4.1 | C:\Program Files\Krita (x64)\bin\krita.exe; lokalny krita-codex-mcp 0.2.0 |
| Synfig | 1.4.5 | C:\Program Files\Synfig\bin\synfig.exe; natywne SIF i renderer CLI |
| Laigter | 1.14.0 | C:\AI2D\Tools\Laigter\laigter.exe; CLI bez GUI |
| LMMS | 1.2.2 | C:\Program Files\LMMS\lmms.exe; XML MMP i render CLI |
| Csound | 6.18 | C:\Program Files\Csound6_x64\bin\csound.exe; CSD i render offline |
| Audacity | 3.7.9.0 | C:\Program Files\Audacity\Audacity.exe; mod-script-pipe |

Wersje i SHA-256 binariów są odczytywane w doctor/environment.json danego
przebiegu. Po aktualizacji programu ponów doctor i celowany test produkcji.

Krita MCP jest już skonfigurowane w Codex. tools/media/krita_mcp_client.py
potrafi też użyć tego samego przypiętego serwera przez prawdziwe STDIO MCP:
initialize, tools/list, tools/call, uvx --offline --from krita-codex-mcp==0.2.0.
Bridge działa wyłącznie na 127.0.0.1:17641; dozwolony katalog C:\AI2D.
Sekret pozostaje w istniejącej prywatnej konfiguracji bridge i nie jest logowany.
Przed zmianami odczytuj krita_state, po zmianach krita_preview.
Nowe dokumenty wybieraj po document_id, warstwy po UUID. Nie dotykaj obcych
otwartych dokumentów. Skrypt tworzący sprite zapisuje i zamyka własny dokument.

## Grafika i animacja

1. Rysuj i edytuj warstwy natywnymi narzędziami Krita MCP. Przykład:

       & $mediaPython 'tools\media\create_krita_sprite.py' --name new_crystal --log-directory 'C:\AI2D\pipeline\new_crystal_krita'

2. Zachowaj master KRA i eksport PNG w C:\AI2D\sprites. Eksportuj oddzielne
   części do rigów przez krita_asset(action=export_layer).
3. Mapy i animacja:

       & $mediaPython 'tools\media\media_pipeline.py' --no-audio maps 'C:\AI2D\sprites\new_crystal.png'
       & $mediaPython 'tools\media\media_pipeline.py' --no-audio animate 'C:\AI2D\sprites\new_crystal.png' 'C:\AI2D\animations\new_crystal.sif'

   animate tworzy przenoszalny SIF z kopią PNG obok, renderuje 60 klatek128px
   i pakuje atlas z opisem kotwicy, prostokątów, FPS i pamięci GPU. Jest to
   szablon animacji wycinanki/obiektu; złożone animacje postaci wymagają
   zaprojektowania części i właściwego rigu w SIF. Nie deklaruje automatycznej
   naturalnej animacji dowolnej postaci na podstawie jednej ilustracji.
4. Dla innych projektów Synfig stosuj C:\AI2D\animations\Render-Synfig.ps1;
   respektuje zakres, FPS i rozmiar z SIF/SIFZ, sprawdza klatki i ruch.
5. bake_lighting w asset_tools.py przygotowuje oświetlony PNG na podstawie
   manifestu Laigtera. Obecny silnik zużywa gotowy kolor; mapy nie są
   automatycznie podłączane jako nieistniejący shader.
6. import-pack kopiuje zweryfikowane eksporty do assets/media/<id> i zapisuje
   receipt z pochodzeniem. runtime_atlas.hpp to adapter tekstury i animacji
   DrawTexturePro, gotowy do użycia przy konkretnym zadaniu integracji grafiki.

Obecne proceduralne postacie nie są zastępowane samym skopiowaniem PNG.
Przy poleceniu dodania nowej postaci/obiektu agent podłącza adapter w jej
konkretnym miejscu renderowania i wykonuje ciche QA. Końcowy test środowiska
sprawdza zgodność plików bez przebudowy gry.

Produkcyjny bohater ma już integrację malowanych paneli Krity i dodatkowych
krzywych ruchu Synfiga. Źródła, ponowny eksport i ograniczenia podglądu
wycinanki opisuje `art/hero/README.md`. Aktualna korekta twarzy i gesty miecza:
`art/hero/SWORD_CLEARANCE_QA.md`; wcześniejsza weryfikacja: `art/hero/QA.md`.
Przy kolejnych zmianach zachowuj dynamiczne wyposażenie wszystkich 75
przedmiotów oraz testuj rzeczywisty renderer, nie sam arkusz lub SIF.
Główny chód od 21.10.2026 także pochodzi z Synfiga: `art/hero/walk/walk.sif`,
eksporter `tools/media/walk_synfig.py`, klip `src/generated/hero_walk_data.hpp`.
Procedurę i testy opisują `art/hero/README.md` oraz `art/hero/WALK_QA.md`.

## Muzyka i efekty

    & $mediaPython 'tools\media\media_pipeline.py' --no-audio music new_composition
    & $mediaPython 'tools\media\media_pipeline.py' --no-audio sfx new_sfx_set

Muzyka powstaje przez edycję nut i instrumentów MMP na podstawie
C:\AI2D\audio\music\szablon.mmp. Komenda music zapisuje nową partyturę JSON,
nowy MMP i 30-sekundowy render trzech cykli. Edycja dowolnej nowej kompozycji:
C:\AI2D\audio\music\LMMS-Music.ps1 / lmms_music.py, opis README-LMMS.md.
Nie wykorzystuj rendertracks z tej instalacji; właściwy render działa.

Csound: C:\AI2D\audio\sfx\Csound-Audio.ps1 / csound_generate.py. Presety
electric, weapon_hit, loot i music; parametry seed, pitch, gain, tempo/bars.
Źródła CSD i parametry pozostają edytowalne. Renderer ignoruje CsOptions,
ustawia rtaudio/rtmidi=null i jawne wyjście WAV. LMMS renderuje przez AudioDummy
z osobną konfiguracją; zapisane preferencje użytkownika pozostają bez zmian.

Obróbka:

    & $mediaPython 'tools\media\media_pipeline.py' --no-audio process-audio 'C:\AI2D\audio\scripts\my_recipe.json'
    & $mediaPython 'tools\media\media_pipeline.py' --no-audio loop-music 'C:\AI2D\audio\edited\music.wav' 'C:\AI2D\audio\edited\new_loop.wav'

Receptura obsługuje inputs, opcjonalny project_source AUP3, trim,
track_gain_db, mix, effects, fade_in/out, normalize_db, output i project.
normalize_db to normalizacja szczytowa. loop-music robi pomiar i normalizację
LUFS, zachowuje liczbę próbek, łagodzi szew i sprawdza ponownie zdekodowany OGG.
FFmpeg z Krity ma loudnorm; libvorbis do zapisu OGG jest w FFmpeg Synfiga.
Program sprawdza enkoder przed rozpoczęciem pracy.

### Ograniczenie Audacity 3.7.9 i bezpieczny tryb

Przy zamykaniu niektórych przyciętych projektów program potrafi zakończyć
cały proces. Potwierdzono to rzeczywistym testem; samo oczekiwanie na HWND,
WM_CLOSE i rozłączenie pipe nie rozwiązały wszystkich przypadków.

Produkcja zapisuje nowy AUP3 i WAV, wywołuje release_saved_project(), kończy
połączenie i pozostawia zapisany projekt otwarty. Nie wysyła Close ani nie
zamyka okien użytkownika. close_project() celowo odmawia wykonania.
Rejestr własnych okien: C:\AI2D\audio\scripts\audacity-managed-projects.json.
Limit: osiem zachowanych projektów, filtrowanych według rzeczywistych PID/HWND.

Po osiągnięciu limitu wspólne wejście process-audio używa FFmpeg/Python dla
WAV: trim, gain, miks, Echo, fade i normalizacja. Zapisuje WAV i recepturę,
raport wyraźnie podaje silnik oraz project=null. Nie tworzy pozornego AUP3.
AUP3 i inne efekty wymagają Audacity; nieobsługiwane operacje są odrzucane.
To pozwala wykonywać kolejne zwykłe zadania audio bez ręcznej obsługi aplikacji.

Nie ponawiaj mutacji po niepewnym wyniku transportu. Sprawdź stan i pliki.
Źródła są chronione SHA-256, wyjścia muszą mieć nowe nazwy.

## Import i podłączanie do zdarzeń

    & $mediaPython 'tools\media\media_pipeline.py' --no-audio import-pack 'C:\AI2D\pipeline\my_pack.json'
    & $mediaPython 'tools\media\media_pipeline.py' --no-audio bind-audio 'C:\AI2D\pipeline\events.json'
    & $mediaPython 'tools\media\media_pipeline.py' --no-audio restore-audio 'ABSOLUTNA_SCIEZKA_DO_binding-receipt.json'

bind-audio używa istniejących nazw z src/audio.hpp: np. spell, swing, pickup,
bow-shot; music wskazuje echoes-of-the-ruins.ogg. Zanim atomowo podmieni
plik w repo/assets/audio, sprawdza format i zachowuje kopię wraz z hashami w
art/pipeline/backups/<id>. restore-audio odmawia, jeśli plik zmieniono po
podpięciu. Nowe pliki bez poprzednika są przenoszone do katalogu kopii.

Test demo używa izolowanej kopii assets w art/pipeline/qa. Produkcyjne audio
pozostaje nietknięte. Aby podłączyć zatwierdzone brzmienia także do istniejącego
dist bez kompilacji, można ponownie użyć bind_audio z asset_root=repo/dist/assets;
stosuj tę samą walidację i backup. Publikowanie/pakowanie wymaga osobnego zadania.

Manifest paczki: schema=1, id, files[{source,path,role,sha256}], provenance.
Dozwolone eksporty: PNG, WAV, OGG i JSON. KRA/SIF/MMP/CSD pozostają masterami
w C:\AI2D. Nie pakuj programów i źródeł roboczych do paczki gracza.

## Kontrola i diagnostyka

Przebiegi: C:\AI2D\pipeline\runs/<id>. Każdy etap ma stdout/stderr logi,
manifesty i raporty. report.json oznacza sukces całego demo; failure.json
zawiera tylko ukończone etapy. Kod wyjścia 0 bez plików nie jest sukcesem.

- PNG/KRA: dekodowanie, alfa, rozmiar, integralność ZIP i warstwy.
- Sekwencja: liczba i kolejność PNG, zmiana obrazu, obcięcie, przejście pętli.
- Audio: komplet PCM, długość, kanały, peak, clipping, LUFS/true peak i OGG.
- Gra: niezależny LoadImage/LoadWave przez używany raylib, adapter atlasu,
  test mapowania Cue i odtworzenia backupów, hashe istniejących zasobów i EXE.

Po nieudanym etapie zachowaj pliki. Wznowienie opieraj na zapisanych manifestach
i rzeczywistym stanie aplikacji; nie uruchamiaj w ciemno ponownie efektu.
Nowy pełny test używa nowego ID. Testy uruchomienia gry zawsze --no-audio.

## Źródła integracji

- [Krita MCP](https://github.com/cyyprezz/krita-codex-mcp)
- [Synfig SIF](https://synfig-docs-dev.readthedocs.io/en/latest/common/sif_file_format.html)
- [Laigter](https://github.com/azagaya/laigter)
- [LMMS CLI 1.2.2](https://github.com/LMMS/lmms/blob/v1.2.2/src/core/main.cpp)
- [Csound CLI](https://csound.com/docs/manual/CommandFlags.html)
- [Audacity scripting](https://manual.audacityteam.org/man/scripting.html)
- [Audacity Close crash](https://github.com/audacity/audacity/issues/9705)

## Fontanny pustyni i ponowne tempo animacji (26.10.2026)

`tools/media/desert_fountains.py --source <nowy-katalog-w-C:\AI2D\sprites>
--animation <nowy-katalog-w-C:\AI2D\animations> --no-audio` tworzy natywne
KRA i wyeksportowane części przez istniejący Krita MCP, a następnie natywne
pętle/opróżnianie w Synfigu. Zachowuje logi oraz źródła; dostarcza dwa atlasy
1024×960 i obrazy suchych mis. Szczegóły i bieżący receipt:
`art/gameplay/fountains/`. Oryginalne pliki nie są nadpisywane; wybierz świeże
foldery dla kolejnej produkcji. `src/fountain_art.hpp` ładuje obrazy raz,
a `Game` przechowuje jednorazowe użycie i zegar opróżniania.

Przy zmianie tempa animacji modyfikuj SIF i ponownie wyrenderuj Synfig,
zamiast zmieniać jedynie zegar gry. Rzut: `guardian_throw_synfig.py`, ruch
na plecy: `hero_backfall_synfig.py`. Kontroluj wspólne gniazda obu dłoni,
pocisku, cienia i znaczników oraz migrację starszych faz zapisu.

Test rzeczywistego silnika, handlarza i fontann:
`pwsh -NoProfile -File tests/guardian_qa.ps1 -NoAudio`.
Niezależna kontrola wszystkich źródeł/klatek:
`verify_guardian.py --qa <folder> --previews --no-audio`.
Nowa paczka: `tests/guardian_package_qa.ps1 -NoAudio`.
Benchmark `--desert-benchmark --windowed --no-audio` obejmuje oba nowe źródła.

## Okna rozwoju, zapas strzał i szczątki Żywej Wydmy (26.10.2026)

`tools/media/create_dune_corpse.py` rysuje natywnie w Kricie ośmiokierunkową
warstwową grafikę pokonanego drugiego bossa. Źródła i receipt są w
`art/gameplay/dune-fallen/`. Runtime wczytuje PNG do ośmiu komórek przy starcie,
wykonując premultiplikację alfa. Cień i maska kursora odpowiadają nowym pikselom.
Pozostałe działające ciała i stary atlas pozostają niezmienione.

Regresje logiki: `build.ps1 -Test` (wywołuje testy z --no-audio).
Interfejs, handel, prawdziwa śmierć bossa i zapis:
`pwsh -NoProfile -File tests/balance_qa.ps1 -NoAudio`.
Niezależne obrazy/KRA: `verify_balance.py --qa <katalog-checks> --no-audio`.
Paczka gracza: `tests/balance_package_qa.ps1 -NoAudio`.
Przy zmianie progu PD aktualizuj funkcję experienceForLevel, a fixture testów
ustawiaj względem bieżącego progu, nie liczby 79 z pierwotnego balansu.
Zachowuj poziomy i procent postępu starszych zapisów. Zapas merchantArrows
jest wspólny dla MAX, zwykłego zakupu i dwukliku; odnawia się tylko przy
pierwszej śmierci Żywej Wydmy, nie przy sprzedaży czy otwarciu okna.

Okna statystyk i umiejętności używają PausedWorldCache w visual_qa.hpp.
To kopia nieruchomego świata w natywnej rozdzielczości; nie zmienia toru
rysowania zwykłej rozgrywki ani worldCanvas. openStats/openSkills unieważniają
cache, podobnie jak zmiana czasu, lokacji, pozycji i rozmiaru bufora. Przy
nowych wejściach do tych okien pamiętaj o invalidatePausedWorld(). Benchmark
resource_performance_qa musi respektować produkcyjną pauzę pod menu.

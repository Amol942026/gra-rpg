# Raport konfiguracji — 19.10.2026

**12.11.2026 — pięć włóczni.** Zachowano dostarczone ikony, materiały i
źródła łupu. Pięć projektów Synfig daje osobne przygotowanie i powrót
do gardy, ze wspólną klatką kontaktu. Dla czterech odmian Csound
wyrenderował 24 nowe pliki WAV PCM16 48 kHz stereo. Gra wybiera
osobny bank ruchu i kontaktu dla każdego modelu. Pchnięcie zatrzymuje
się na pierwszym celu lub przeszkodzie, w tym na skrzyni. Zapis v18
zachowuje stan pchnięcia przed kontaktem i podczas powrotu do gardy.

Źródła w `C:/AI2D/animations/emberfall-spear-finish-20261112` i
`C:/AI2D/audio/emberfall-spear-finish-20261112` można odtworzyć przez
`tools/media/spear_synfig.py` i `tools/media/spear_audio.py`.
Opcja `--export-only` przenosi zmienione klucze Synfig do gry.
150 klatek Synfig, 524 756 kontroli rozgrywki, 20 087 zapisu, 532
grafiki i 724 miksera offline. Czas ukrytego renderu i eksportu
PNG w teście gry wyniósł 6,06 s. Wyniki i podgląd są w
`C:/AI2D/pipeline/runs/emberfall-spear-finish-20261112`.

**11.11.2026 — wiatr niezależny od kamery.** Obie warstwy shadera
próbkują rzut współrzędnych mapy, a ich dryf zależy od czasu. Wygląd
pustynnego piasku zachowano. W lesie są smugi podmuchów i niesione listki,
w Kalderze osobne drobiny popiołu. Tekstury dwóch krain dopracowano w Kricie
na bazie wcześniejszych źródeł ComfyUI. Przeszło 30 kontroli renderera,
w tym porównanie tych samych miejsc mapy przy nieruchomym czasie oraz
przesunięciu kamery w obu osiach. Podgląd i raport są w
`C:/AI2D/pipeline/runs/wind-world-20261111`.

**11.11.2026 — wiatr w trzech krainach.** DreamShaper 8 w lokalnym ComfyUI
i trzy warstwowe pliki Krity dostarczyły maski mgiełki, piasku i popiołu.
Shader przesuwa dwie warstwy nad scenerią, ogranicza krycie przy bohaterze
i kończy pracę przed rysowaniem HUD. Wczytuje trzy tekstury 640 × 320
przy starcie. Przeszły 24 kontrole renderera bez dźwięku, w tym zmiana
podmuchów w czasie, pokrycie alfa i zgodność pikseli bocznego HUD.
Źródła KRA są w `C:/AI2D/sprites/region-wind-20261111`.
Graf ComfyUI, raport i zrzuty są w `C:/AI2D/pipeline/runs/region-wind-20261111`.

**11.11.2026 — dopracowanie wieży i dwóch przeciwników.** Źródło odnowy
przesunięto od schodów, Miron stoi na środku pracowni, a podpisy NPC
mają przejrzyste tło. Witraże obserwatorium osadzono w kamiennej ścianie
z otwartym wyjściem na balkon. Rogacz Żaru jest większy o 45%. Zarodnik
kniei ma widok od tyłu oraz poprawione kierunki chodu i ataku. Dwa
źródła KRA i 92 klatki Synfig zapisano lokalnie pod nazwą
`tower-finish-20261111`. Przeszło 49 kontroli obrazu, interakcji i kierunków,
239 handlu oraz 479 129 logiki i animacji. Obie paczki Windows zweryfikowano
po wypakowaniu. Testy grafiki i startu działały w ukrytym oknie bez dźwięku.
Zrzuty i odbiór są w
`C:/AI2D/pipeline/runs/tower-finish-20261111`.

**11.11.2026 — trzy gatunki przeciwników.** Zarodnik kniei, Oko Wydm
i Rogacz Żaru mają po cztery spotkania w swoich krainach, własne łupy
i osobne efekty ataku, trafienia oraz upadku. Lokalny DreamShaper 8
w ComfyUI i trzy projekty KRA dostarczyły wycinanki. Synfig wyrenderował
15 klipów i 138 klatek do trzech atlasów z maskami kursora. ACE-Step 1.5
Turbo w ComfyUI, nagrania Kenney CC0 i Audacity 3.7.9 dały 27 wariantów
PCM16 stereo 48 kHz. Mastery zachowano w katalogach `C:/AI2D/sprites`,
`C:/AI2D/animations` i `C:/AI2D/audio` pod nazwą `region-enemies-20261111`.
Zapis v18 dodaje spotkania do odwiedzonych krain v1–v17, zachowując łupy.
PASS: 479 129 kontroli rozgrywki, 19 034 zapisu, 100 renderera i 624
miksera offline. Każda rozpakowana edycja Windows przeszła 180 ukrytych
klatek bez urządzenia audio. Raporty i podglądy są w
`C:/AI2D/pipeline/runs/region-enemies-20261111`.
[Wagi ACE-Step 1.5](https://huggingface.co/Comfy-Org/ACE-Step-1.5)

**11.11.2026 — wyposażenie Wieży Eldrina.** Odmalowano 25 grafik na pięciu
piętrach, w tym schody, regały, kominek, kocioł, teleskop, witraże i stoisko
Mirona. Lokalny ComfyUI z DreamShaper 8 dodał szczegóły materiałów. Akcenty
rysowane w Kricie zapisano na osobnej warstwie. Zachowano obrysy,
projekcję i maski wskazywania. Usunięto tabliczki pod schodami i stopkę
sklepu. Przeszło 91 kontroli renderera, grafiki i interakcji w ukrytym
oknie bez audio. W obu rozpakowanych edycjach zweryfikowano 312 zasobów
i 180 klatek startowych w ukrytym oknie bez audio.
Pliki KRA i grafy są w `C:/AI2D/sprites/emberfall-tower-20261111`,
a metryki, skróty plików i zrzuty w `C:/AI2D/pipeline/runs/tower-paint-20261111`.

**11.11.2026 — porządek w HUD i zgodność noszonego wyposażenia z ikonami.**
Wprowadzono 17 zmian z listy użytkownika. Pasek PD jest ciemnozielony,
a Atak i Pancerz są pod nim. Obrysy noszonego sprzętu odpowiadają używanym
ikonom, w tym spiczaste tarcze, hełmy z nosalem i topory o łukowym ostrzu.
Sprawdzono 85 przedmiotów w 2040 ujęciach, po osiem kierunków w postoju,
chodzie i biegu. Przeszło 2170 kontroli HUD i renderera oraz 477 595 kontroli
logiki i animacji. Każda paczka Windows przeszła 180 ukrytych klatek
startowych bez otwierania urządzenia audio. Wyniki i zrzuty znajdują się w
`C:/AI2D/pipeline/runs/hud-equipment-consistency-20261111`.

**11.11.2026 — naprawiony natywny MCP Sprite Studio 0.3.3.** Usunięto
zagnieżdżone `block_on`; handler renderuje w `spawn_blocking`. Zbudowano
i zainstalowano lokalny plik release z kopią oryginału. 39 testów rigu,
16 MCP oraz rzeczywiste 12 PNG / powtórny render / błąd FPS / eksport
na zainstalowanym programie: PASS. [Naprawa i odbiór](SPRITE_MAKER_FIX.md).

**11.11.2026 — kroki, zbieranie i Weź wszystko.** Sześć wariantów foley
CC0 zmontowanych w lokalnym ComfyUI i rzeczywistym Audacity (AUP3, 48 kHz).
Kroki zgodne z fazą stóp, atomowe zbieranie całego łupu i zachowane fragmenty
fabularne. 477 595 kontroli logiki, 19 030 zapisu, 534 audio i 7 UI: PASS.
Oba EXE i ZIP-y odświeżone; 287 zasobów i 180 ukrytych, bezgłośnych klatek
startowych każdej rozpakowanej edycji: PASS (bez wniosku o FPS na pierwszym planie).
[Źródła i odbiór](../audio/steps-loot-20261111/README.md).

**11.11.2026 — próba Sprite Studio 0.3.3.** Natywny import, zapis rigu,
analiza i eksport PNG działają. Render przez MCP ujawnił błąd zagnieżdżonego
runtime Tokio; bez klatek z tego renderu i bez podmiany animacji gry.
[Raport próby](SPRITE_MAKER_TRIAL.md).

**11.11.2026 — bardziej pionowe trzymanie broni.** Boczne odchylenie
ograniczone do 6 stopni dzięki chwytowi bliżej ciała i zgięciu łokcia.
Omijanie rękawa zmienia pochylenie do przodu. Kontrola 32 broni i 476 734
kontrole logiki/animacji: PASS. [Źródła i wyniki](../hero/WALK_QA.md).

**10.11.2026 — broń omija ramiona podczas chodu.** Korekta chwytu,
łokcia i orientacji ostrza/jelca dla mieczy, toporów i sztyletów; osobny
chwyt łuku. Wszystkie 32 bronie, 1384 pozycje renderera i 442 174 kontrole
logiki oraz animacji: PASS. Sprawdzono również przejścia do ataku i chowanie
broni. [Źródła i wyniki](../hero/WALK_QA.md).

**10.11.2026 — wymach ramion bohatera w chodzie.** Większy ruch obu
ramion z zachowaniem długości kończyn, zapisany w natywnym Synfigu 1.4.5.
60 klatek silnika Synfig, 120 klatek renderera w ośmiu kierunkach oraz
338 477 kontroli logiki i animacji: PASS. Wykrywanie przenośnego Synfiga
w skryptach działa bez instalatora. [Źródła i wyniki](../hero/WALK_QA.md).

**10.11.2026 — cztery gatunki leśnego bestiariusza.** Osobne projekty FLUX
i 13 warstw Krity, ciągłe mapowanie nowych materiałów i ruch z 444 klatek
Synfig. PASS w 1189 kontrolach renderera i 345 804 kontrolach logiki.
[Źródła i odbiór](../creatures/forest-20261110/README.md).

**10.11.2026 — leśni rozbójnicy.** Wyprostowana poza spoczynkowa,
skórzana kurta i bordowy kaptur z lokalnego FLUX.1 Schnell oraz Krity.
Cztery klipy Synfig i 192 sprawdzone klatki. PASS w 323 kontrolach
renderera, wskazywania kursorem i eksportu ciał oraz w 345 308 kontrolach
logiki i animacji. [Źródła i wyniki](../creatures/bandit-20261110/README.md).

**09.11.2026 — nowy wygląd Wędrowca.** FLUX.1 Schnell i SDXL, 12 warstw
Krity oraz sześć klipów ruchu tkaniny z projektów Synfig, sprawdzonych na
354 klatkach. Atlas 512 × 384, mocowania i bazowe czasy ruchu z istniejącego
rigu. PASS: 452 kontrole renderera, 594 wyposażenia, 340 709 logiki i animacji
oraz 19 047 zapisu. Obie paczki po wypakowaniu przeszły po 180 klatek,
ukryte i bez dźwięku. [Źródła i odbiór](../hero/wanderer-20261109/README.md).

**09.11.2026 — lokalne modele AI:** Piper TTS, ACE-Step 1.5, SDXL,
FLUX.1 Schnell, AnimateDiff, AudioLDM 2 i TangoFlux przeszły próby
generowania. ComfyUI działa w trybie offline i low VRAM, generatory
efektów mają osobne środowisko Pythona. Własna LoRA odłożona.
[Konfiguracja i wyniki prób](AI_SUITE_SETUP.md).

**09.11.2026 — ComfyUI i Krita MCP:** trzy nowe modele w katalogu
współdzielonym, sumy wydawców sprawdzone i trzy renderowania 512 × 512
zakończone powodzeniem na RTX 3060 6 GB. Zainstalowana Krita 5.3.4.1
połączona przez przypięty lokalny MCP 0.2.0. Próba odczytu, rysowania dwóch
warstw, podglądu oraz zapisu KRA i PNG zakończona powodzeniem.
[Konfiguracja i ścieżki do raportu](COMFY_KRITA_SETUP.md).

**09.11.2026 — ataki specjalne Kaldery:** własne warstwy Krita, cztery
tekstury z drugiego przebiegu DreamShaper 8 w lokalnym ComfyUI i ruch 2D
Blendera. Cztery ścieżki efektów, trzy gesty postaci, 12 różnych renderów
podglądu i atlas 4 MiB. PASS: 514 kontroli gry, 109 klatek z ukrytych scen.
[Źródła i odbiór](../volcano/attacks-20261109/README.md).

**09.11.2026 — lekcje Eldrina:** po pierwszym bossie płatna nauka ognia,
lodu i pioruna (600 / 1500 / 3000 złota). Własne 2D Krita i trzy pętle
Synfig, 180 rzeczywistych klatek 60 Hz. Nagrane prawdziwe łuki i podmuch
płomienia; Csound/Audacity, 21 wariantów, źródła CC0 i CC BY 3.0 z atrybucją.
50 Cue / 150 WAV; muzyka i ustawienia zachowane. Zapis v15 czyta v1–v14.
PASS: 234931 logiki/animacji, 10646 zapisu, 534 AudioSystem i 22 natywnego UI.
Praca cicha, testy gry ukryte. [Źródła i końcowy odbiór](../gameplay/eldrin-spells-20261109/README.md).

**09.11.2026 — przeturlanie i pozostałe przedmioty:** trzy warianty Roll
z nagrań tkaniny/skóry/podłoża, po 0,52 s, native Csound i Audacity.
Własne 23 ilustracje 2D w warstwowym KRA i lokalnym ComfyUI, z korektą
monet i sierści po przeglądzie. Drugi preloadowany atlas 1024 × 384,
1,5 MiB RGBA8; dotychczasowe 85 ikon sprzętu bajtowo zachowane.
Zapis v14, 45 Cue / 135 WAV, muzyka i preferencje bez zmian.
[Źródła audio](../audio/ground-roll-20261109/README.md),
[ikony i końcowy odbiór](../items/loot-20261109/README.md).

**Kula ognia, teleport, łuk i ikony HUD — 08.11.2026:** cztery nowe Cue,
12 wariantów Csound/Audacity, 45 Cue / 135 WAV łącznie. Wyrzut ognia oraz
eksplozja osobno; pojawienie po teleportacji słyszalne w odsłonięciu sceny;
fizyczny naciąg łuku zatrzymywany przy puszczeniu/anulowaniu/pauzie.
85 modeli ikon wyrenderowano rzeczywistym Blenderem 5.2.2 LTS, następnie
wykończono lokalnym ComfyUI 0.44 / DreamShaper 8 w ośmiu udanych zadaniach.
Preload jednego atlasu 1536 × 1024, 6 MiB RGBA8; runtime wyłącznie 2D.
PASS: 234864 kontrole logiki, 10244 zapisu i 482 obsługi audio bez sprzętu.
[Audio](../audio/flame-20261108/README.md), [ikony](../equipment/icons-20261108/README.md).

**Korekta audio 08.11.2026:** nagrane foley CC0 (Kenney, artisticdude,
Darsycho), filtracja i montaż Csound, rzeczywisty eksport i AUP3 Audacity.
Przebudowano 28 gestów walki / 84 warianty; cały bank ma 41 Cue / 123 WAV.
Osobne brzmienia typów wrogów, rozbójnika i mocnego ciosu; zwykły cios wroga
brzmi przy uderzeniu. Kanały domyślnie włączone, muzyka 85%, efekty 95%,
master 90%; menu/pauza 65%. Muzyka po korekcie: −16 LUFS (wieża −17),
finalne OGG −2,05 do −1,90 dBTP. Użytkownik jawnie polecił włączyć dźwięk;
lokalne konfiguracje są włączone, późniejsze wybory nadal się zapisują.
PASS: 234852 kontroli logiki/animacji, 10202 zapisu i 435 AudioSystem/
dekodera. Zapis v13 czyta v1–v12; autentyczny fixture v12 zachowuje cały
payload. Praca i testy bez playbacku, gra ukryta. [Źródła i odbiór](../audio/physical-20261108/README.md).
Końcowe paczki PASS: 253 zasoby każda, gracz 180 klatek i 435 kontroli audio,
DEV 3 × 180 klatek krain i osiem kontroli Opcji. Zatwierdzone preferencje
zachowane; końcowe licencje/instrukcje i wszystkie wpisy ZIP zweryfikowano.

**Aktualizacja map 08.11.2026:** wszystkie trzy krainy 72 × 72 → 102 × 102,
czyli 2,00694 × dawna powierzchnia. Przeciwnicy: las 91, pustynia 55,
Kaldera 61; teren: 490 / 487 / 209 obiektów, łącznie 88 wzniesień.
Rozszerzona sieć dróg i cztery nowe przeprawy, przedłużona lawa i urwiska.
Istniejące zasoby 2D są ponownie użyte; bufory podłoża 6270 × 3285 mieszczą
się w limicie 8192 i zajmują około 8,7% więcej pamięci niż poprzednie.
Zapis v12 migruje v1–v11 z zachowaniem pierwotnych mieszkańców, ciał, łupu,
przedmiotów, postępu, pozycji i RNG. PASS: 234833 kontroli logiki/animacji,
10157 zapisu, trzy pomiary projekcji i 184 renderera. Nowe spotkania i dalekie
krańce map osiągalne przez rzeczywiste kolizje. 21 podglądów, cicho i w tle.
Ukończona aktualizacja + render: średnia 8,47 / p95 10,62 ms; bez deklaracji
foreground FPS. [Źródła, przegląd i odbiór paczek](../world-expansion/README.md).
Gotowe EXE i oba ZIP-y sprawdzone po wypakowaniu: po 228 zgodnych zasobów,
gracz — 180 klatek startu i ponownie 184 kontrole rozbudowy, DEV — 3 × 180
klatek krain. Okna ukryte, audio wyłączone argumentem, preferencje zachowane.

**Aktualizacja audio 08.11.2026:** cztery nowe, 32-taktowe kompozycje LMMS
dla lasu/pustyni/Kaldery/wieży, z płynną zmianą regionu. 33 efekty Csound
w trzech wariantach (99 WAV), obejmujące walkę obu stron, skrzynie,
broń/ekwipunek, zbieranie przedmiotów i turlanie oraz zdolności specjalne.
Obróbka Audacity: pogłos muzyki, normalizacja/DC; pięć rzeczywistych AUP3
z kontrolą SQLite i pozostawieniem otwartych projektów, bez Close/playback.
Całość lokalna, bez obcych sampli. Stereo 48 kHz; muzyka Vorbis q5,
−19 LUFS (wieża −20), PCM16 efekty. PASS: 200189 kontroli logiki/animacji,
5162 zapisu i 353 cichego odbioru AudioSystem z rzeczywistym dekoderem raylib
i wyjściem w pamięci. Nie otwierano urządzenia, nie oceniano przez odsłuch.
Mastery, pochodzenie i końcowy odbiór paczek: [oprawa dźwiękowa](../audio/suite-20261108/README.md).
Gotowe EXE i obie paczki ZIP przeszły weryfikację po wypakowaniu: po 228
zgodnych zasobów, gracz — 180 klatek startu i ponowne 353 kontrole audio,
DEV — 3 × 180 klatek krain. Osiem kontroli rzeczywistego menu Opcji — PASS,
ukryte i bez urządzenia. Cztery finalne OGG mają −18,97 do −19,98 LUFS
i −5,12 do −3,73 dBTP. Zapisane wyciszenie obu kanałów użytkownika zachowane.

**Aktualizacja 08.11.2026 — zdolności silnych gatunków:** kobra piaskowa
otrzymała Jadowity wachlarz, skarabeusz miedziany Miedzianą falę,
obsydianowy wojownik Rozłupujący cios, a żarowy drak Żarowy oddech.
Każdy atak ma zapowiedź, stały kierunek i odpoczynek; działa unik oraz osłona.
Efekty 2D i znaczniki korzystają z istniejących postaci i klipów Synfig,
bez nowej produkcji bitmap lub modeli. Zapis v11 zachowuje fazy i pociski,
odczyt v1–v10 pozostaje dostępny. Logika/animacje: 200119 kontroli,
zapis: 5162, renderer: 497 (cztery zdolności, osiem kierunków, 105 obrazów,
cztery spotkania w oryginalnej scenografii) — PASS, cicho i w tle.
Oba ZIP-y po wypakowaniu: zgodne EXE i po 147 zasobów; gracz — 180 klatek
startu i ponownie 497 kontroli zdolności, DEV — 3 × 180 klatek. Audio nie
zostało otwarte, preferencje użytkownika zachowane; bez pomiaru foreground FPS.
[Opis, podgląd i końcowy odbiór paczek](../gameplay/elite-abilities/README.md).

**Aktualizacja 08.11.2026 — 44 formy terenu i ciągłe drogi Kaldery:**
las 14 / pustynia 14 / Kaldera 16, o różnych wysokościach i promieniach.
Łączą się w grzbiety i łagodne siodła. Ścieżka otrzymała nowy, czterowarstwowy
materiał z lokalnej Krity i wspólną maskę całej wygładzonej sieci.
Miękkie brzegi i skrzyżowania nie nakładają osobnych pasów. Poprawione
mieszanie alfa podczas wypieku zachowuje pełne krycie gruntu i usuwa smugi
od odsłanianego tła. Mosty i przepaść zachowane; zapis bez migracji.
PASS: 1584 kontrole wysokości/dróg, 1063 renderera Kaldery i 13 źródeł KRA/PNG.
Logika i animacje: 200013 / zapis: 4513 / obsługa skrzyń: 115 — PASS.
Wszystkie testy ciche i ukryte. Ukończony render: średnia 4,11 / p95 4,85 ms;
nie jest to pomiar foreground FPS. [Źródła, podglądy i odbiór](../terrain/diversity-20261108/README.md).
Gotowe EXE i oba ZIP-y sprawdzono po wypakowaniu: po 147 zgodnych zasobów,
gracz — 180 klatek startu i 1584 kontrole podłoża/dróg, DEV — 3 × 180 klatek.
Audio wyłączone w testach, okna ukryte, preferencje użytkownika zachowane.

**Aktualizacja 08.11.2026 — dostępne wzniesienia trzech krain:** 13 profili
wysokości (las 4 / pustynia 4 / Kaldera 5), z łagodnymi wejściami i zejściami.
Ruch bohatera i przeciwników mierzy odległość po powierzchni. Podłoże,
obiekty, cienie, kamera i odwrotny rzut kursora korzystają z tej samej
wysokości. Istniejące ilustracje są rysowane na podniesionej siatce 2.5D;
bez nowych modeli, tekstur lub migracji zapisów. Wszystkie szczyty mają
dostęp z równiny przy rzeczywistej scenografii i kolizjach. PASS: 564
kontrole wysokości/ruchu/renderera, 1063 regresji Kaldery i 115 obsługi
skrzyń. Testy ukryte i ciche, preferencje audio zachowane.
Ukończony render próby wysokości: średnia 4,06 ms / p95 4,47 ms;
nie jest to pomiar FPS widocznego okna. [Podglądy i odbiór](../terrain/hills/README.md).
Logika: 199995 / zapis: 4513 — PASS. Zaktualizowane EXE oraz oba ZIP-y
przeszły odbiór po wypakowaniu (147 zgodnych zasobów w każdej paczce,
gracz: 180 klatek startu i 564 kontrole renderera, DEV: 3 × 180 klatek).

**Aktualizacja 08.11.2026 — przepaść i skrzynie krain:** kamienne obrzeże
zastąpiono urwiskiem schodzącym w mgłę. Powierzchnia kończy się na brzegu;
tło ma warstwy odległych skał, popiół i żar, a rzeki spływają do otchłani.
Trzy style skrzyń, po trzy stany: drewno i mech / piaskowe złoto / obsydian
i żar. Nowe źródła wykonano lokalnie w oficjalnej przenośnej Kricie 5.3.4,
przez natywne Python/Qt i kritarunner, bez nowego MCP, chmury i obcych grafik.
Zapisane projekty worker zachowuje w tle. PASS: 1063 kontroli renderera,
115 obsługi skrzyń i 31 niezależnych kontroli zasobów. Testy ukryte i ciche.
[Źródła, podglądy i raporty](../volcano/abyss/README.md).

**Aktualizacja 08.11.2026 — granice i teren Kaldery:** ciągłe skalne obrzeże
przy granicy ruchu, przyciemnione zewnętrze i kontur minimapy. Istniejące
ilustracje Krity wypiekane są raz w tym samym buforze; bez nowych zasobów GPU.
Nowe rozmieszczenie ogranicza tłok i puste obszary: minimalny odstęp
1,77 → 4,45 pola, największy pusty promień próbki 13,53 → 4,97 pola.
Zachowano przejścia i interakcje. Rewizja 4 aktualizuje także starsze zapisy,
z zachowaniem pozostałych danych bajt w bajt. PASS: 199995 kontroli logiki,
4513 zapisu i 820 renderera. Testy ciche i ukryte.
[Opis, podglądy i wyniki](../volcano/layout/README.md).

**Aktualizacja 07.11.2026 — jednolite włosy bohatera:** usunięto jasną
plamę nad czołem, dopasowano kolor przednich i tylnych pasm do korony,
zachowując ich delikatny detal. Korekta istniejącego materiału jest wykonywana
raz przy wczytywaniu; źródłowy PNG i KRA są niezmienione. 428 kontroli
renderera i 25 niezależnych kontroli obrazów — PASS. Wszystkie okna ukryte,
bez urządzenia audio. [Porównanie i raport](../hero/hair/README.md).

**Aktualizacja 04.11.2026 — przeciwnicy Kaldery / ComfyUI:** na wyraźne
zamówienie użytkownika i po potwierdzeniu lokalnego lekkiego modelu wykonano
56 rzeczywistych generacji img2img w ComfyUI 0.41.3 / DreamShaper 8.
Nowe malowanie wszystkich sześciu gatunków i Varkhora jest rzutowane na
istniejące części animowane przez Synfig. Osiem kierunków, dokładna alfa,
kotwice i stan rozgrywki są zachowane. Widoczne detale obejmują rysy twarzy,
łuski, błony skrzydeł, pancerze, fałdy szat i żar kamiennych szczelin.
PASS: 728 kontroli źródeł, grafów, wejść i atlasów oraz 787 renderera,
w tym 224 ujęcia chodu / bezruchu / ataku / szczątków. Testy pozostały
ukryte i ciche. Ukończony render: średnia 4,03 ms / p95 5,55 ms;
nie jest to pomiar foreground FPS. Atlasowe RGBA8 nadal 55 050 240 bajtów,
bez dodatkowych tekstur lub odczytu plików w klatce.
Zaktualizowano obie aktywne kopie zasobów i oba ZIP-y; każdy inny wpis
archiwum pozostał identyczny bajtowo, w tym oba pliki EXE. Rzeczywisty
EXE gracza przeszedł 180 klatek startu z zamkniętym audio i ukrytym oknem.
Źródła ComfyUI, oryginały i siedem warstwowych plików OpenRaster są
w `C:\AI2D\sprites\emberfall-caldera-comfy-20261104`. Na tym PC Krity
ani jej bridge nie ma, mimo historycznego opisu; nie deklarujemy zapisów KRA.
Pochodzenie modelu, komercyjne uprawnienia do obrazów, hashe, backupy,
workflow i wyniki: [ComfyUI](../volcano/comfy/README.md),
`../volcano/comfy/QA.json`. Nie użyto chmury ani płatnych węzłów.

**Aktualizacja 04.11.2026 — portal, ciągły grunt i ostrość:** cztery nowe
warstwowe KRA oraz natywny SIF. Kamienna brama ma runy i wirujący rdzeń,
120 klatek / 60 Hz / pętla 2 s. Grunt jest zawiniętą fakturą bez ciemnych
brzegów kafli, a duże pola popiołu mają miękkie przejścia. Poprawiono
podwójne mnożenie gotowej klatki przez alfę; obraz końcowy zachowuje RGB
piksel w piksel w DXGI i OpenGL. Kaldera korzysta z natywnego bufora świata.
Monitor 1920×1080 / 60 Hz, DPI 125%; rzeczywisty V-sync interval 1.
PASS: 293 kontrole źródeł/eksportów/prezentacji, 787 renderera, 194309
logiki i 4090 zapisu. Finalne paczki sprawdzono po wypakowaniu: po 143
zasoby; gracz — 180 klatek i 787 kontroli, DEV — 540 klatek trzech lokacji.
Testy ukryte, bez audio, preferencje zachowane. Ukończony render gracza:
średnia 5,53 ms / p95 6,17 ms; nie jest to pomiar FPS widocznych na ekranie.
Portal dodaje 14,07 MiB tekstur i nie generuje zasobów w klatce.
[Źródła, porównania i raport](../volcano/gateway/README.md),
`../volcano/gateway/QA.json`. Nowsze podłoże zastępuje dwa eksporty
terenowe z poprzedniej produkcji Krita/Laigter.

**Aktualizacja 04.11.2026 — przeciwnicy i otoczenie Krita/Laigter:**
6 gatunków, Varkhor i 32 odmiany scenografii korzystają z bardziej
szczegółowych natywnych ilustracji. Dopracowano 12 KRA; Laigter 1.14.0
wygenerował 48 map z profilem HeightMap. Oświetlenie wypalono w PNG,
z zachowaniem żaru i alfy, bez dodatkowych tekstur/przebiegów w grze.
Dodano osady mineralne, pory, odpryski, łuski i duże pola zastygłych potoków.
Poprawiono też głębokość rysowania miecza za hełmem podczas zamachu.
PASS: 811 kontroli źródeł/map/eksportów i 708 renderera, w tym 224 pozycje
w ośmiu kierunkach. Finalny ZIP gracza wypakowano i sprawdzono: 139 zasobów,
180 klatek startu oraz pełny test 708 kontroli. DEV: 139 zasobów i 540
klatek w trzech punktach Kaldery. Wszystkie testy gry ukryte, bez audio;
preferencje dźwięku pozostały niezmienione. Ukończony render paczki gracza:
średnia 4,41 ms / p95 4,84 ms; nie jest to foreground FPS ani pomiar V-sync.
12 zastępowanych tekstur zajmuje przed i po 82 014 208 bajtów RGBA8.
Źródła, profile, podglądy i wyniki: `../volcano/materials/README.md`
oraz `../volcano/materials/QA.json`. Starsze raporty dotyczą poprzedniej grafiki.

**Aktualizacja 04.11.2026 — koryta lawy, teren i zamach Wojownika:**
ukończono natywne materiały oraz integrację rozpoczętą 30.10. Lawa płynie
poniżej gruntu, między skalnymi ścianami, i wychodzi poza obie granice mapy.
Nowe powierzchnie, brzegi oraz 16 wariantów głównej scenografii pochodzą
z Krity; duży zamach Wojownika w ośmiu kierunkach i nurt lawy — z Synfiga.
Finalne źródła: 6 KRA, 9 SIF, 480 klatek ataku i 60 klatek lawy. PASS:
790 kontroli natywnych źródeł/eksportów, 708 renderera, 194309 logiki
i 4090 zapisu. Zaktualizowano EXE i oba ZIP-y. Wypakowana paczka gracza
przeszła 180 klatek startu i pełne 708 kontroli; DEV — 540 klatek w trzech
punktach Kaldery. Sprawdzono po 139 zasobów każdej paczki, brak natywnych
masterów w ZIP-ach i niezmienione preferencje audio. Wszystkie okna testów
były ukryte, audio wyłączone argumentem. Ukończony render paczki gracza:
średnia 4,79 ms, p95 5,18 ms; nie jest to pomiar foreground FPS ani V-sync.
Ściana dodaje 0,5 MiB tekstury bez kolejnego bufora mapy i bez I/O w klatce.
Źródła, podglądy i raport: `../volcano/depth/README.md`, `../volcano/depth/QA.json`.

**Aktualizacja 29.10.2026 — przepływ lawy i detal Kaldery:** 8 nowych
natywnych KRA, 15 SIF, dodatkowe 16 grafik dekoracji oraz detale wszystkich
sześciu gatunków i Varkhora w ośmiu kierunkach. Lawa faktycznie przesuwa się
o pełny kafel w zamkniętej pętli; renderer interpoluje i grupuje gotowe próbki.
Format zapisu nadal v10, rewizja scenografii 3; prawdziwy fixture v10 dowodzi
zachowania pozostałych pól. PASS: 194309 kontroli logiki/animacji, 4090
zapisów, 1102 źródeł/eksportów, 624 renderera i 257 DEV. Oba końcowe ZIP-y
wypakowano i uruchomiono w ukryciu bez audio, sprawdzono po 137 zasobów
i niezmienione preferencje dźwięku. Źródła, podgląd animacji i pełne wyniki:
`../volcano/river-detail/README.md` oraz `../volcano/river-detail/QA.json`.
Krita jest używana z zachowaniem zapisanych własnych dokumentów: po wykrytej
awarii zamykania KRA wznowienie sprawdza SHA i nie ponawia gotowych edycji.

**Aktualizacja 29.10.2026 — dopracowanie Kaldery:** bieżąca wersja zastępuje
poniższe pierwsze grafiki tej lokacji. 16 natywnych masterów Krita i 16
projektów Synfig, 16 wariantów dekoracji, siedem zestawów postaci w ośmiu
kierunkach, ciągłe podłoże/lawa i skupiska otoczenia z wolnymi szlakami.
Źródła, pochodzenie, migracja v9→v10 i wyniki: `../volcano/polish/README.md`
oraz `../volcano/polish/QA.json`. PASS: 194277 kontroli logiki/animacji,
4086 zapisów, 1200 źródeł/eksportów, 548 renderera i 257 DEV. Oba końcowe
ZIP-y wypakowano i uruchomiono w ukryciu bez audio; zweryfikowano 135
zasobów każdej paczki i niezmienione preferencje audio. Ukryty ukończony
render: średnio 6,30 ms / P95 8,32 ms; prezentacja obrazu nie była mierzona.

**Aktualizacja 29.10.2026 — Kaldera Popiołu:** trzecia grywalna kraina,
sześć nowych gatunków i Varkhor, Serce Kaldery. 11 warstwowych źródeł Krita,
14 klipów Synfig / 840 klatek oraz 12 klatek lawy. Grafika jest wycinanką
2D; bez nowego modelowania 3D i usług zewnętrznych. Dwie skrzynie i dwie
osobne jednorazowe kapliczki, podróż od Eldrina po zabiciu drugiego bossa,
powrót do wieży i trzeci fragment. Zapis v9 odczytuje v1–v8. 192804 kontrole
logiki/animacji i 4078 zapisów — PASS; 1133 kontroli źródeł i 515 integracji
renderera. Testy pozostają ukryte i ciche; ukończony render nie dowodzi
FPS wyświetlania. [Źródła, procedury i końcowe raporty](../volcano/README.md).

**Dalsza optymalizacja 28.10.2026:** bezpośrednie pakowanie wierzchołków,
wspólne próbki wiatru korony/cienia/obrysu, cache parametrów shaderów
i wstępna alokacja cieni. W ukrytym pomiarze A/B wypakowanej paczki średni
czas renderowania sześciu scen spadł o 13,03%. 813 kontroli renderera,
logika, zapisy oraz ZIP — PASS. Ten pomiar nie potwierdza FPS na ekranie.
[Aktualny raport i paczka](../performance/20261028/round2/README.md).

**Aktualizacja 28.10.2026 — optymalizacja wydajności:** bufory GPU modeli
i obrysów, ponowne używanie geometrii i pamięci, cache portretu, odrzucanie
zasłoniętego tła, usunięcie zbędnej głębi statycznych map oraz wspólne obliczenia
wiatru koron i cieni. Pustynia we wcześniejszych pomiarach ekranu: 46,17 →
58,45–59,91 FPS. Końcowe testy, zgodnie z nowym poleceniem, są ukryte i ciche;
ich czasy renderowania nie dowodzą stałych FPS wyświetlania. HUD pozostaje
natywny, świat otwarty przy 1080p korzysta z mniejszego bufora.
[Raport, porównania i ograniczenia](../performance/20261028/README.md).

**Aktualizacja 28.10.2026 — schody wewnątrz mapy i pełna kolizja:**
oba biegi na wszystkich piętrach przesunięto w głąb sali; wszystkie piksele
grafiki i 4320 kontrolowanych wierzchołków mieszczą się w obrysie podłogi.
Kolizja obejmuje stopnie, szyb, spoczniki i balustrady wraz z marginesem stóp.
Ruch w wieży ma podkroki zapobiegające przeskakiwaniu kolizji. Przejścia
prowadzą na wolne podejścia, a wczytywanie bezpiecznie naprawia pozycję
starszego zapisu wewnątrz przesuniętej konstrukcji. Bez zmiany formatu v7.
110 661 kontroli logiki/animacji, 3403 zapisów, 17 920 renderera i mechanik
oraz 46 niezależnych — PASS. 8100 klatek ruchu i 23 widoki bez dźwięku.
Nie dodano tekstur ani geometrii rysowanej w pętli gry. Pomiar GPU w tym
przebiegu przekraczał 16,67 ms; pełnych 60 FPS nie potwierdzono.
[Źródła, testy i wyniki](../scenery/tower-stairs-fit/README.md).
Końcowy ZIP również sprawdzony: 115 zgodnych zasobów, 180 klatek startu,
ponowione 17 920 + 46 kontroli — PASS. Audio zamknięte, preferencje zachowane,
brak DEV i źródeł natywnych. Czasy GPU zapisano bez deklaracji 60 FPS.

**Aktualizacja 27.10.2026 — balkon i panorama Doliny Świtu:** na piątym
piętrze obserwatorium dodano otwarty taras z fizyczną balustradą. Chód i bieg
prowadzą z sali na balkon i z powrotem bez teleportu. Trzy oryginalne,
warstwowe grafiki Krity przedstawiają panoramę gór, lasów i rzeki oraz taras
z balustradami i bluszczem. Minimapę dopasowano do nowego obrysu.
1337 kontroli ruchu, zapisu, warstw i dotychczasowych pięter — PASS;
97 niezależnych kontroli KRA, PNG, 12 widoków i minimapy — PASS.
P95 renderowania z ukończeniem GPU: 11,60 ms w obserwatorium i 11,77 ms
na balkonie, poniżej 16,67 ms. To pomiar renderowania, bez prezentacji FPS.
Wszystko bez dźwięku. [Źródła i procedury](../scenery/tower-balcony/README.md).
Pełna regresja: 110 661 kontroli logiki/animacji i 3403 zapisów — PASS.
Końcowy ZIP: 115 zgodnych zasobów, 180 klatek startu; po rozpakowaniu
ponownie 1337 + 97 kontroli — PASS. P95 renderowania paczki 13,37 / 13,75 ms.
Brak audio, DEV, źródeł natywnych i zmiany preferencji gracza.

**Aktualizacja 27.10.2026 — dopasowanie pochwy i przydział atrybutów:**
pochwa ma nowy wlot przy biodrze/pasie oraz krótsze mocowania. Nowe natywne
krzywe Synfig dopasowują chwyt i wsuwanie; 72 klatki oraz dokładny eksport C++
sprawdzone. W P dodano DODAJ 5 i opisy przy kursorze dla wszystkich sześciu
atrybutów, z działaniem, podglądem wartości i efektem pięciu punktów.
Zatwierdzanie, COFNIJ i blokada nauki zdolności pozostają zachowane.
110 661 kontroli logiki, 301 renderera, 233 niezależne — PASS. 485 856 póz
i 201 600 rzeczywistych klatek odwracania gestu: zero przecięć głowy i pancerza.
Sprawdzono 18 podpowiedzi w trzech skalach oraz 32 ujęcia postaci.
Końcowy ZIP również PASS po świeżym rozpakowaniu: 112 zgodnych zasobów,
180 klatek startu, ponowione 301 kontroli renderera i 233 niezależne.
Urządzenie audio zamknięte, preferencje zachowane, brak DEV i źródeł natywnych.
Źródła i procedury: [pochwa i atrybuty](../hero/sheath-fit/README.md).

**Aktualizacja 27.10.2026 — pełny wyprost kolan i dodatkowy łup:** miednica
Strażnika dopasowuje wysokość do nogi podporowej; stałe długości kończyn,
pełny wyprost i kontakt stopy są zachowane przez całą pętlę. Przejście do
zamachu wygasza korektę płynnie. Dodano 36 przedmiotów — po trzy na każdy
gatunek — według niezmiennych szans 60% / 15% / 4% / 1%. Łącznie 75 elementów
wyposażenia. Własne palety i zdobienia powstały w 36 warstwach Krity;
poprzednie materiały i identyfikatory zachowano. Bez nowych tekstur runtime.
110 661 kontroli logiki, 3403 zapisów, 1844 renderera, 856 kontroli ruchu
i 438 materiałów/wyprostu/ekwipunku — PASS. Sprawdzono 552 klatki ruchu,
240 faz wyprostu (minimalny kąt 179,9997°), każdą nową rzecz w świecie,
portrecie, rzeczywistym dropie, podwójnym LPM i podpowiedziach. Bez dźwięku.
Źródła i wyniki: [rozszerzenie łupu](../items/expansion/README.md).
Końcowy ZIP również PASS po świeżym rozpakowaniu: 112 zgodnych zasobów,
180 klatek startu, ponowione 1844 kontrole renderera i 856 + 438 kontroli
niezależnych. Brak audio, DEV i zmian preferencji gracza.

**Aktualizacja 27.10.2026 — chód Kamiennego Strażnika:** cykl jest kalibrowany
do drogi po kolizjach, projekcji i skali bossa, na istniejącej krzywej kontaktu
Synfig. Wykrok 18 i dopasowane kolana; tempo automatycznie odpowiada patrolowi,
pościgowi i zatrzymaniu. Prędkość w świecie i walka zachowane. 108 558 kontroli
logiki, 1553 renderera i 856 niezależnych — PASS. Sprawdzono 552 klatki
i 287 kontaktów stóp; maks. ruch stopy w podparciu 0,000214 px przy skali 2,2.
Wszystko bez dźwięku. [Wynik i procedury](../gameplay/guardian/gait/QA.md).
Końcowy ZIP sprawdzony po rozpakowaniu: 112 zgodnych zasobów, 180 klatek
rzeczywistego startu i powtórzony pełny test chodu — PASS. Preferencje
audio niezmienione, brak DEV i źródeł natywnych w paczce.

**Aktualizacja 26.10.2026 — czytelniejsze czcionki:** cały tekst interfejsu
i dialogów używa Lato Bold 700; nagłówki — prawdziwego Cinzel Bold 700,
lokalnie utrwalonego z dołączonego fontu zmiennego na licencji OFL.
Minimum drobnego tekstu interfejsu wzrosło z 14 do 15. Zachowano natywną
rasteryzację i siatkę pikseli; brak dodatkowych kopii liter lub rozmycia.
241 kontroli renderera, 57 widoków w trzech skalach, 552 istniejące kontrole
UI oraz 4074 niezależne kontrole fontów/zrzutów — PASS. Żaden z tych widoków
nie tworzy atlasu podczas rysowania. P95 renderowania z ukończeniem GPU
w pięciu scenach: 4,08–14,04 ms; poniżej 16,67 ms, bez pomiaru prezentacji.
Źródła, procedury i raport końcowej paczki: [typografia](../typography/README.md).
Świeże rozpakowanie ZIP-a: 112 zgodnych zasobów, 180 klatek startu EXE,
powtórzone 57 widoków i wszystkie kontrole typografii — PASS. Brak audio,
trybu DEV i zmian preferencji. [Wyniki i hashe](../typography/QA.md).

Końcowa paczka 26.10 (okna rozwoju/PD/strzały/szczątki): PASS po rozpakowaniu,
111 zgodnych zasobów i 180 klatek startu. Brak audio, trybu DEV i zmian
ustawień gracza. Wyniki i hashe: [QA.md](../gameplay/dune-fallen/QA.md).


Okna P / U przechowują nieruchomy świat w natywnej rozdzielczości na czas
przeglądania i przydzielania punktów. Bieżąca rozgrywka nadal jest rysowana
bez pośredniego skalowania. Cache jest odświeżany przy otwarciu, zmianie
rozmiaru, lokacji, pozycji lub zegara symulacji; HUD i akcje pozostają bieżące.
P95 renderowania z ukończeniem GPU: statystyki 4,4362 ms, szczątki Wydmy
7,8336 ms, Strażnik 13,0342 ms; wszystkie pięć scen poniżej 16,6667 ms.
To test renderowania, nie pełnej prezentacji FPS w tle.


**Aktualizacja 26.10.2026 — okna rozwoju, szczątki Wydmy, strzały i PD:**
Usunięto przyciski +/− z obu okien; tekstowe DODAJ / COFNIJ są widoczne tylko
przy dostępnej operacji. P: statystyki, U: umiejętności. Umiejętności nadal
wymagają przydziału i zatwierdzenia. Nowe szczątki drugiego bossa namalowano
w Kricie w ośmiu kierunkach, z maską kursora i cieniem z nowych pikseli.
Miron sprzedaje łącznie 20 strzał; pierwsza śmierć Wydmy przywraca zapas do 20.
Zapis v7 przechowuje zapas. Progi PD: 100L + 2L(L−1); starsze zapisy
zachowują poziom, rozdane punkty i procent postępu. Nagrody bestii bez zmian.
107 937 kontroli logiki i 1891 zapisów — PASS. 49 nowych kontroli UI,
22 wcześniejsze statystyk, 90 umiejętności i 48 niezależnych źródeł/obrazów — PASS.
Źródła i procedury: `art/gameplay/dune-fallen/README.md`.


Końcowy pakiet 26.10: wszystkie funkcje sprawdzone też po rozpakowaniu ZIP
(110 zgodnych zasobów, 180 klatek startu, bez audio i zmian preferencji).
Renderowanie z ukończeniem GPU: p95 10,28 ms / 11,36 ms dla fontann,
14,03 ms dla Strażnika; poniżej 16,67 ms. Pełnego FPS prezentacji w tle
nie potwierdzono: OpenGL/DXGI blokowały się w sterowniku AMD. Nie zmieniano
V-sync ani konfiguracji gracza. Szczegóły: `art/gameplay/fountains/QA.json`.


**Aktualizacja 26.10.2026 — tempo Strażnika, handel i fontanny:** zamach 1,5 s,
odnowienie 3,5 s, lot 11 pól/s; powrót kontroli po upadku 1,85 s. Cel rzutu
śledzi aktualną pozycję do wypuszczenia; w locie pozostaje ustalony. Nowe
źródła Krity/Synfiga: dwie jednorazowe fontanny pustyni, pełna mana lub HP,
24 klatki płynącej wody i 24 opróżniania na kolor. Tekstury są preładowane,
zużycie i faza opróżniania zapisane w v6; migracja v1–v5 zachowana. U Mirona
2× LPM kupuje lub sprzedaje jedną sztukę. MAX nadal handluje całym stosem.
104 688 kontroli logiki i 1723 zapisu — PASS.
Sprawdzone w rzeczywistym rendererze: 1163 kontrole, 4561 niezależnych
kontroli źródeł i obrazów, 96 natywnych klatek fontann i 96 ujęć z gry.
Testy bez dźwięku. [Fontanny i źródła](../gameplay/fountains/README.md),
[aktualne zachowanie Strażnika](../gameplay/guardian/QA.md).


**Aktualizacja 26.10.2026 — lot na plecy i spokojne wstawanie:** zastąpiono
poprzedni szybki upadek sekwencją 2,8 s z oderwaniem obu stóp, lądowaniem
plecami, odpoczynkiem twarzą do góry i podnoszeniem przez bok oraz kolano.
W Synfigu zapisano osobne tory kontaktów dłoni/stóp, 169 próbek i 168
rzeczywistych renderów. Kierunek patrzenia jest ustalony przed lotem.
112 655 kontroli logiki, 1552 zapisu, 1157 renderera i 5526 źródeł/obrazów
przeszło. Nowy format v5 przelicza fazę starszych zapisów; rzeczywiste pliki
v1–v4 działają. 39 elementów eq, cienie i kolizje zachowane; bez nowych tekstur.
Master: `C:\AI2D\animations\emberfall_backfall_20261026_v3`.
[Aktualny raport i zbliżenie ruchu](../gameplay/guardian/QA.md). Testy bez dźwięku.

**Aktualizacja 26.10.2026 — przewrócenie bohatera:** odrzut głazu prowadzi
do pełnej sekwencji utraty równowagi, upadku, chwili leżenia, podparcia rąk
i wstawania (1,2 s). Nowy natywny SIF z 72 renderami i 73 próbkami istniejącego
rigu, bez nowych modeli i tekstur. Cień korzysta z tej samej pozy, wszystkie
39 przedmiotów zachowują wygląd. Kolizje i chroniony unik pozostają aktywne.
Zapis v4 zachowuje fazę; rzeczywiste pliki v1–v3 nadal działają. 99 594
kontrole logiki, 1510 zapisu, 968 renderera i 3653 źródeł/obrazów — PASS.
Master: `C:\AI2D\animations\emberfall_knockdown_20261026_v2`.
[Aktualny raport i podgląd](../gameplay/guardian/QA.md). Testy bez dźwięku.

**Aktualizacja 23.10.2026 — odrzut od uderzenia głazu:** dodano radialny
impuls w promieniu 1,6 pola, silniejszy blisko środka, z wyhamowaniem przez
0,24 s. Uwzględnia osłony, drzewa, skrzynie, granice mapy i chronioną fazę
uniku. Pauza zatrzymuje odrzut, format zapisu v3 zachowuje jego stan;
rzeczywiste pliki v1 i v2 nadal się wczytują. 89 517 kontroli logiki,
1382 zapisu, 667 renderera i 2085 źródeł/obrazów — PASS. Weryfikacja
rozpakowanej paczki i podgląd: [aktualny raport](../gameplay/guardian/QA.md).
Nie dodano tekstur, audio ani zależności; testy pozostają bez dźwięku.

**Aktualizacja 23.10.2026 — rozpad głazu po uderzeniu:** sześć oryginalnych
fragmentów Krity z atlasu 288×192 rozlatuje się, obraca, odbija i osiada
zgodnie z animacją Synfig (1,2 s, 72 rendery, 438 próbek). Osobne cienie
i sortowanie, drobny gruz i pył; brak dodatkowych obrażeń. Zapis v2 zachowuje
fazę rozpadu, pauza zamraża ruch. 89 309 kontroli logiki, 1295 zapisu,
662 renderera i 2083 źródeł/obrazów — PASS. Dwie ostatnie grupy ponowiono
na świeżo rozpakowanym ZIP; zgodne hashe 106 zasobów, 180 klatek startu gracza.
Wszystkie uruchomienia bez dźwięku, ustawienia audio zachowane. Źródła:
`C:\AI2D\sprites\emberfall_guardian_fragments_20261023` oraz
`C:\AI2D\animations\emberfall_guardian_shatter_20261023`.
[Raport i podgląd](../gameplay/guardian/QA.md).

**Aktualizacja 23.10.2026 — kierunek zgięcia łokci Strażnika:** poprawiono
wybór strony zgięcia obu ramion w natywnym Synfigu oraz ustawienie barków,
aby wyeliminować odwrócone łokcie i blokowanie stawu przy unoszeniu głazu.
Kontrole obejmują cały gest i powrót, także interpolowane podklatki.
89 279 kontroli logiki i animacji, 1252 zapisu, 473 renderera oraz
867 źródeł i obrazów przeszło; ostatnie dwie grupy także po rozpakowaniu ZIP.
Źródło: `C:\AI2D\animations\emberfall_guardian_elbows_20261023_v2`.
Aktualny raport i podgląd: [QA rzutu](../gameplay/guardian/QA.md).

**Aktualizacja 23.10.2026 — oburęczny rzut dużym głazem:** Strażnik przykuca,
wyrywa skałę z ziemią i odłamkami, unosi ją przed twarzą, potem nad głowę
i rzuca obiema rękami. Nowa grafika Krity 192×192, 168 renderów Synfig,
169 próbek dziesięciu kontrolerów, 2,2 s przygotowania i 0,6 s dokończenia.
Wspólny środkowy punkt głazu i oba boczne chwyty, prześwit przy głowie,
cień oraz pasek zdrowia nad skałą. 87 933 kontrole logiki, 1252 zapisu oraz
425 + 513 kontroli renderera i źródeł — także po świeżym rozpakowaniu ZIP.
Pomiar 900 klatek lasu: 59,49 FPS, bez gwarancji stałych 60 FPS.
Aktualne wyniki i paczka:
[raport oburęcznego rzutu](../gameplay/guardian/QA.md).

**Aktualizacja 23.10.2026 — zachodni start i boss:** nowa wyprawa zaczyna
się na skrajnym zachodzie, ponad 67 pól od Strażnika. Pierwszy boss otrzymał
czytelny rzut kamieniem: grafika Krity, sterowanie ręką z Synfiga, oznaczony
cel, lot po łuku, osłony i unik. Format zapisu 2 zachowuje rzut i pociski,
a format 1 nadal działa. 87 595 testów logiki, 1252 zapisu, 35 prologu oraz
204 + 264 kontroli obrazu/źródeł przeszło; te ostatnie ponowiono po rozpakowaniu
nowego ZIP. [Raport i źródła](../gameplay/guardian/QA.md). Las: 55,03 FPS
w cichym teście 900 klatek; nie jest to potwierdzenie stałych 60 FPS.

**Aktualizacja 23.10.2026 — drzewa leśne:** trzy oryginalne, warstwowe grafiki
Krity (świerk, brzoza, dąb), bufory 2× i istniejący wiatr, cienie oraz maski
kursora. Weryfikacja 206 kontroli renderera i 121 kontroli źródeł/obrazów
przeszła także po rozpakowaniu nowego ZIP. [Źródła i wyniki](../scenery/TREES_QA.md).
Porównanie wydajności lasu: 54,40 FPS przed i 54,31 FPS po; pomiar nie oznacza
stałych 60 FPS. Wszystkie testy były bez dźwięku.

**Aktualizacja22.10.2026 — leśne menu i enty:** dodano własną warstwową
ilustrację Krity i12s animacji Synfig, zsynchronizowano chód entów oraz
zmniejszono liczbę grzybów do48. Rzeczywiste testy i nowy rozpakowany ZIP
przeszły kontrolę: [FOREST_QA.md](../menu/FOREST_QA.md).

**Aktualizacja 21.10.2026 — bestiariusz:** do gry i ZIP-a gracza podłączono
poprawione grafiki i ruch wszystkich 12 gatunków lasu i pustyni, z użyciem
Krity i Synfiga. [Aktualne wyniki testów i FPS](../creatures/QA.md),
[źródła i procedury](../creatures/README.md). Poniższa część opisuje
historyczny test konfiguracji środowiska z 19.10.

**Proces produkcji i importu działa.** Wszystkie sześć narzędzi sprawdzono
rzeczywistymi operacjami. Audacity ma opisane poniżej obejście awarii zamykania.
Gra nie była przebudowywana ani uruchamiana. Nie odtwarzano dźwięku, nie użyto
płatnych usług i nie wysłano projektu na serwery.

## Narzędzia

| Program | Wersja | Rzeczywiście sprawdzone |
| --- | --- | --- |
| Krita | 5.3.4.1 | istniejący MCP 0.2.0, state/preview, dokument, natywne rysowanie, warstwy, eksport KRA/PNG |
| Synfig | 1.4.5 | edytowalny SIF z ilustracją z Krity, interpolacja, 60 różnych PNG |
| Laigter | 1.14.0 | cztery mapy z nowego sprite'a, kontrola kanałów i wektorów |
| LMMS | 1.2.2 | nowa kompozycja MMP na bazie szablon.mmp, pięć instrumentów, render WAV |
| Csound | 6.18 | trzy nowe CSD i WAV: electric, weapon_hit, loot |
| Audacity | 3.7.9.0 | import, trim, efekty, normalizacja, cztery eksporty WAV i cztery AUP3 |

Krita używa już zainstalowanego, przypiętego serwera przez prawdziwe STDIO MCP.
Nie instalowano nowego MCP. Bridge działa tylko lokalnie; sekret pozostaje
w prywatnej konfiguracji i nie jest logowany. Pozostałe programy korzystają
z CLI, natywnych formatów i skryptów Python/PowerShell. Dostępne biblioteki
i narzędzia wystarczyły; nie instalowano dodatkowych usług.

FFmpeg z Krity dostarcza loudnorm; FFmpeg Synfiga ma libvorbis. Wykonano
rzeczywisty awaryjny render FFmpeg/Python z trim/gain/mix/Echo/fade/normalize.

## Pliki testowe i parametry

Zaimportowany pakiet:
[assets/media/media_20261019_175901_828feb](../../assets/media/media_20261019_175901_828feb).
Dziesięć plików runtime ma sprawdzone sumy i receipt z pochodzeniem.

- [Sprite](../../assets/media/media_20261019_175901_828feb/sprites/crystal.png):
  256×256 RGBA, przezroczyste tło. Master KRA ma sześć narysowanych warstw
  i przezroczystą bazę: C:\AI2D\sprites\media_20261019_175901_828feb.kra.
- Mapy normal/specular/occlusion/parallax: 256×256. Niezależna kontrola obejmuje
  wszystkie piksele i wektory normalnych. Jest także oświetlony wariant PNG.
- [Atlas](../../assets/media/media_20261019_175901_828feb/sprites/crystal_atlas.png):
  60 różnych klatek128×128, 60 Hz, pętla1s, atlas1056×1056,
  4 460 544 bajty RGBA8 (ok. 4,25 MiB). Stała sylwetka i kotwica, bez obcięcia.
  Obejrzano sprite i przekrój klatek animacji.
- [Muzyka OGG](../../assets/media/media_20261019_175901_828feb/audio/crystal_sanctum_loop.ogg)
  i master WAV: 10,000s, stereo48k, 480 000 ramek także po dekodowaniu OGG,
  -18,00 LUFS / -6,97 dBTP. Cztery takty D-moll przy96BPM; harfa, flet, pad,
  bas i bęben. Render trzech cykli, wycięcie środkowego i kontrola szwu pętli.
- [Zaklęcie](../../assets/media/media_20261019_175901_828feb/audio/spell.wav):2,4s;
  [uderzenie](../../assets/media/media_20261019_175901_828feb/audio/swing.wav):1,6s;
  [zdobycie przedmiotu](../../assets/media/media_20261019_175901_828feb/audio/pickup.wav):2,8s.
  Stereo48k PCM16, szczyt ok. -2dBFS, zero przesterowanych próbek.

KRA/SIF/MMP/CSD, partytury i parametry pozostają w C:\AI2D. Nie użyto
zewnętrznych sampli ani obrazów. Pochodzenie identyfikuje własne natywne
rysowanie i syntezę oraz dostarczony przez użytkownika szablon MMP.

## Wyniki testów

- [Pełny raport](C:/AI2D/pipeline/runs/media_20261019_175901_828feb/report.json): PASS.
  Po produkcji wznowiono etap QA po poprawieniu ścieżki assemblera;
  wykorzystano zapisane zasoby bez ponownego nakładania efektów.
- [15 niezależnych kontroli](C:/AI2D/pipeline/acceptance_c6301d9576/acceptance-report.json): PASS.
  Integracje, hashe, alfa, mapy, wszystkie klatki, pamięć atlasu, WAV/OGG,
  cztery AUP3, blokada Close, odmowa nadpisania/złego hasha/wyjścia poza katalog/
  nieznanego Cue, fallback i odmowa restore po zmianie docelowego pliku.
- [Cztery testy zachowania zapisanych projektów](C:/AI2D/pipeline/close_recovery_412b03e1a6/report.json): PASS.
  Zapisane projekty pozostają otwarte, istniejące okna są zachowane.
- Osobny helper z raylib5.5: **3PNG i 5plików audio — PASS**.
  LoadImage/LoadWave bez urządzenia audio i bez okna gry.
- Podpięcie music/spell/swing/pickup, backup i przywrócenie czterech plików
  w izolowanej kopii assets: PASS. Produkcyjne audio i EXE zachowały hashe
  snapshotu etapu integracji. Nie wykonywano kompilacji gry.

## Ograniczenia i sposób dalszej pracy

**Audacity 3.7.9:** zamykanie części przyciętych projektów potrafi zakończyć
cały program. Odtworzono błąd; zmiana pipe Close na zwykłe zamknięcie okna
nie rozwiązała wszystkich przypadków. Produkcja zapisuje AUP3/WAV i wywołuje
release_saved_project(), pozostawiając zapisane okno. Native Close jest
zablokowane. Po ośmiu takich projektach wspólny proces automatycznie używa
FFmpeg/Python dla obsługiwanych receptur WAV. Ten wariant zapisuje recepturę,
nie pozorny AUP3. AUP3 i inne efekty wymagają dostępnego budżetu Audacity.

**Silnik gry:** obecne postacie są proceduralne. Pakiet jest zaimportowany
i zgodny z dekoderami, a runtime_atlas.hpp dostarcza adapter renderowania.
Podłączenie konkretnej nowej postaci wymaga użycia adaptera w jej rendererze
podczas zadania dotyczącego tej grafiki. Test środowiska nie zmienia rozgrywki.
Mapy nie mają obecnie shadera runtime; oświetlenie wypieka się do PNG,
a parallax pozostaje w źródłach.

**Ocena:** test techniczny nie zastępuje odsłuchu i dopracowania artystycznego.
Eksport60Hz nie jest pomiarem FPS całej gry. Przykład sprawdza narzędzia,
nie stanowi finalnego zestawu artystycznego całej gry.

Przy kolejnych poleceniach agent dobiera potrzebne etapy, przygotowuje master,
animację lub kompozycję, eksportuje, sprawdza i podłącza zasób do wskazanej
mechaniki z backupem. Konfiguracji i ręcznej obsługi sześciu aplikacji nie
trzeba powtarzać. Nie publikowano ani nie commitowano zmian.

[Instrukcja](ASSET_PIPELINE.md), [styl i budżety](STYLE.md),
[AGENTS.md](../../AGENTS.md), [wejście PowerShell](../../tools/media/Run-MediaPipeline.ps1).
Lista skryptów, hashe i aktualne raporty: latest.json w tym katalogu.

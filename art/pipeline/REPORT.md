# Raport konfiguracji — 19.10.2026

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

# Menu i kronika wyprawy — 22.10.2026

`src/game_menu.hpp` obsługuje menu startowe, pauzę, opcje, twórców,
trzy sloty i potwierdzenia. `welcome` oznacza menu przed prologiem,
`paused` pauzę bieżącej sesji; `MenuPage` wybiera podstronę. Wszystkie
akcje myszy i klawiatury trafiają do tego samego dyspozytora.
Escape jest obsługiwany przed wejściem do gry, ekwipunku i scenek.
Świat i zegary scenek pozostają zatrzymane. Wczytanie kończy się pauzą,
co zapobiega przypadkowemu atakowi i pozwala przeczytać wynik odczytu.

Menu wykorzystuje nową ilustrację leśnej Bramy Świtu z Krity oraz natywne
elementy UI. Master `forest/forest-gate.kra` zachowuje osobne warstwy nieba,
dalekiego lasu, gruntu, portalu, kamiennego łuku, pierwszego planu, mgły
i światła. Są to własne rysunki wykonane przez istniejące Krita MCP,
bez zewnętrznych obrazów i modeli. Pochodzenie: `forest/provenance.json`.
Kolory i typografia są zgodne z `art/pipeline/STYLE.md`.
Fonty i zasoby świata wczytywane są przed grą;
odczyt metadanych slotów następuje tylko przy wejściu na listę lub po zapisie.
W zwykłej klatce gry menu wykonuje jedynie sprawdzenie flagi.
W menu i pauzie kompozycja zachowuje nieprzezroczysty bufor końcowy:
alfa jest mieszana oddzielnie od koloru. Obraz nie ciemnieje powtórnie
przy prezentacji tekstury na ekranie. Nie wymaga to dodatkowej kopii obrazu.

## Animowane tło

`motion/forest-menu.sif` to natywny projekt Synfig 1.4.5 z pętlą12s.
Ma 240 klatek przy20Hz; `src/generated/menu_motion_data.hpp` zawiera241
odczytanych z zapisanych waypointów próbek. Gra interpoluje je zgodnie
z własnym odświeżaniem (docelowo60Hz). Podgląd renderera Synfig nie jest
pomiarem FPS gry. Zachowano 240 rzeczywistych natywnych PNG w
`C:\AI2D\animations\emberfall_menu_20261022_v2\renders`.

`src/menu_scene.hpp` wczytuje raz cztery PNG1440×900 z `assets/menu`.
Paralaksa, mgła i blask wykorzystują gotowe warstwy; dodatkowo unoszą się
48 ograniczone liczbowo drobiny. Łączny koszt RGBA8:20 736 000 bajtów
(ok.19,8MiB); każda tekstura mieści się w2048px. Napisy i przyciski nie
podążają za tłem. Nie ma odczytu plików ani tworzenia tekstur w pętli menu.

Produkcja lokalna (nowe, nieistniejące katalogi; Python z AGENTS.md):

```powershell
& $mediaPython tools/media/create_menu_forest.py --root C:\AI2D\sprites\NOWE_MENU --no-audio
& $mediaPython tools/media/menu_synfig.py --root C:\AI2D\animations\NOWE_MENU --art C:\AI2D\sprites\NOWE_MENU --header src/generated/menu_motion_data.hpp --render --no-audio
```

Master produkcyjny: `C:\AI2D\sprites\emberfall_menu_20261022_v2`.
Nie ponawiaj skryptu na ukończonym dokumencie; `--resume` służy wyłącznie
wznowieniu przerwanego własnego rysunku na podstawie jego receiptów.

`src/save_game.hpp` definiuje format EFS v1: jawne pola little endian,
identyfikator, wersja, długość i suma FNV-1a 64 bit. To kontrola błędów,
nie podpis kryptograficzny. Nie zapisuje pamięci struktur ani wskaźników.
Wszystkie dane `Game`, oba generatory MT19937, `IntroSequence`
i `TowerSequence` są zachowywane, w tym stan poza aktywną lokacją.
Wczytywanie waliduje rozmiary i zakresy przed podmianą sesji. Przy zmianie
schematu należy podnieść wersję i dodać migrację lub jawnie odmówić odczytu.

Zapisy użytkownika: `%LOCALAPPDATA%\Emberfall\saves\slot-{1,2,3}.efs`.
Ścieżki używają szerokiego Win32 API, obsługując polskie nazwy kont.
Zapis do `.tmp` jest sprawdzany, zamykany i przenoszony przez
`MoveFileExW(REPLACE_EXISTING | WRITE_THROUGH)`. Poprzedni poprawny plik
pozostaje w `.bak`; uszkodzony plik nie zastąpi poprawnej kopii.
Nieudany zapis/odczyt pokazuje komunikat i zachowuje bieżącą sesję.
Wyjście oraz zastąpienie aktywnej sesji wymagają potwierdzenia.
Nie ma autozapisu ani zapisu przy zamknięciu okna systemowym przyciskiem X.

Punkty umiejętności/statystyk są zachowane. Roboczy, niezatwierdzony
przydział jest odrzucany przy wejściu do pauzy. Łuk przestaje być napinany.
Kolejka dźwięków jest czyszczona przy wczytaniu, aby nie powtórzyć dźwięków
z klatki zapisu. Preferencje audio są oddzielne od stanu wyprawy.

## Powtarzanie kontroli

- `pwsh -NoProfile -File build.ps1 -Test`: mechanika i niezależny
  `tests/save_game_tests.cpp`; zapis do świeżego katalogu w `dist`.
- `pwsh -NoProfile -File tests/menu_qa.ps1`: rzeczywisty renderer wersji
  gracza, akcje menu, trzy sloty, opcje, scenki, stan świata i obrazy
  w skalach 80/100/120%. Sprawdza też wejście przez callbacki raylib
  (Enter, Escape i strzałki), a nie tylko dyspozytor menu. Nie steruje
  fizyczną klawiaturą. Własny katalog zapisów, bez plików gracza.
- `pwsh -NoProfile -File tests/menu_package_qa.ps1 -NoAudio`: świeże rozpakowanie,
  porównanie EXE i każdego assetu, kontrola startu i menu na plikach ZIP-a.
  Niezależnie dekoduje PNG przez `tools/media/verify_menu.py` i porównuje
  SIF/header, KRA, 240 natywnych PNG, 60 klatek tła oraz120 enta przez
  `tools/media/verify_forest_menu.py`. Ten drugi skrypt z `--previews`
  zapisuje GIF-y wyłącznie z gotowych zrzutów rzeczywistego renderera.
- `pwsh -NoProfile -File tests/menu_benchmark.ps1 -NoAudio`: po 900 klatek
  animowanego menu oraz faktycznej rozgrywki w lesie i na pustyni.
  Deweloperski `--menu-benchmark --no-audio` mierzy tło bez eksportu PNG.
  Diagnostyka startu ZIP-a wykonuje dodatkowo kosztowny zrzut PNG w klatce
  119; jego czas zawiera się w średniej krótkiego testu startowego i nie
  jest miarą normalnego renderowania menu.

Wszystkie uruchomienia diagnostyczne przekazują `--no-audio`.
Nie otwierają urządzenia audio, nie odtwarzają dźwięków i nie zapisują
ustawień gracza. Pierwszy statyczny wariant: `QA.md`.
Aktualny wariant leśny, grzyby i enty: `FOREST_QA.md`.
Oryginały zmienianych plików i poprzedni ZIP:
`art/pipeline/backups/menu-20261022` (lokalna kopia, poza paczką gracza).

# Lokalna poprawka Sprite Studio 0.3.3 — 11.11.2026

Poprawka dotyczy rzeczywistego błędu `Cannot start a runtime from within a
runtime` w natywnym narzędziu MCP `render_rig_animation`. Bazą jest oficjalne
źródło MIT `JohnKinyanjui/sprite-maker`, tag `v0.3.3`, commit
`9de73a6136a1a045af2e8769349fbadc17dbef69`. To lokalna kompilacja z poprawką,
nie nowe oficjalne wydanie autora.

## Zmiana

- `src-tauri/src/rig/commands.rs`: synchroniczna funkcja renderująca wywołuje
  bezpośrednio synchroniczny renderer. Usunięto zagnieżdżone `block_on`.
- `src-tauri/src/mcp/schema.rs`: handler MCP jest asynchroniczny i kieruje
  renderowanie, zapis PNG oraz rejestrację animacji do `spawn_blocking`.
  Błędy wejścia zachowują dotychczasowy format odpowiedzi; błąd workera ma
  kod `render_failed`.
- Regresja w `src-tauri/src/rig/tests.rs` wykonuje pełny render z aktywnego
  jednowątkowego i wielowątkowego Tokio. Sprawdza PNG, zmianę pozy i identyczne
  bajty niezależnie od rodzaju runtime.
- Flaga manifestu MSVC w `build.rs` jest teraz ograniczona do MSVC, dzięki
  czemu lokalna kompilacja GNU nie przekazuje opcji innego linkera.

Pristine źródło pozostało w `C:/AI2D/Tools/SpriteStudio-v0.3.3/source`.
Zmiany są w osobnym worktree `patched-source`, na gałęzi
`codex/fix-mcp-rig-render`. Odtwarzalny patch:
`tools/media/sprite-studio-mcp-runtime.patch`; `git apply --check` względem
niezmienionego tagu zakończył się sukcesem. Nie zmieniano Cargo.lock.

## Kompilator i odtwarzanie

Lokalny Rust GNU **1.99.1** pochodzi z oficjalnego `static.rust-lang.org`.
`CARGO_HOME` i `RUSTUP_HOME` są pod `C:/AI2D/Tools/Rust`; instalacja nie
zmieniła globalnego PATH. Kompilacja używa istniejącego w64devkit, trzech
zadań i zablokowanego zestawu zależności `--locked`. Nie instalowano MSVC
ani nie zmieniano plików Windows.

`tools/media/Build-SpriteStudioMcp.ps1` ustawia i przywraca środowisko procesu.
Używa bibliotek GNU dołączonych przez Rust. Brakującą bibliotekę importową
KtmW32 tworzy `dlltool` z tabeli eksportów lokalnego systemowego DLL,
wyłącznie w prywatnym folderze `gnu-support`. Nie kopiuje ani nie zmienia
samego systemowego DLL. Dla wygenerowanego testu biblioteki skrypt
`embed_sprite_test_manifest.py` osadza Common Controls v6 przez natywne API
zasobów Windows, tak jak Tauri robi dla pliku aplikacji. Bez tego oryginalny
test GNU nie startował (`STATUS_ENTRYPOINT_NOT_FOUND`).

Budowany jest tylko plik MCP. `frontendDist=null` dotyczy tej kompilacji
bez okna; zainstalowany interfejs GUI nie jest przebudowywany.

```powershell
pwsh -NoProfile -File tools/media/Build-SpriteStudioMcp.ps1 -Test
pwsh -NoProfile -File tools/media/Build-SpriteStudioMcp.ps1 -Profile release
```

## Weryfikacja

**Zainstalowano i sprawdzono wersję końcową.** Zoptymalizowany plik MCP
(28 253 696 bajtów) podmieniono w
`C:/AI2D/Tools/SpriteStudio-v0.3.3/extracted/PFiles/Sprite Studio`.
Obok znajduje się wymagany przez kompilację GNU `WebView2Loader.dll` z
przypiętego pakietu `webview2-com-sys 0.38.2` oraz licencja MIT programu.
Oryginalny MCP jest zachowany i sprawdzony SHA-256 w
`C:/AI2D/pipeline/runs/sprite-maker-fix-20261111/backup/sprite-studio-mcp.exe`.
Hash istniejącego GUI pozostał identyczny. Żaden proces Sprite Makera nie
wymagał zamykania do podmiany. Instalację i hashe opisuje `installation.json`
oraz lokalny `Emberfall-MCP-runtime-fix.json` obok programu.

**39 testów rigu + 16 testów MCP: PASS**, w tym regresja pełnego renderu
wewnątrz aktywnego Tokio. Pozostałe ostrzeżenia kompilatora o nieużywanych
elementach pochodzą z bazowego źródła.

Rzeczywisty proces MCP w wersji debug przeszedł izolowaną próbę:
48 narzędzi, import bohatera, zapis rigu, 12 PNG 320 × 360, osiem różnych
pozycji, powtórny render identyczny bajtowo, obsłużenie błędnego FPS,
kolejny poprawny render oraz eksport arkusza PNG z JSON. Proces zakończył
się kodem 0, bez panic. Render pierwszego zestawu trwał około 1,65 s;
to czas ukończonego zadania, nie wynik FPS gry.

Taką samą próbę przeszedł plik release przed instalacją i ponownie plik
z docelowego folderu po podmianie. Obie próby dały PASS: 12 prawidłowych
PNG, osiem różnych pozycji, powtarzalne bajty, odzyskanie po niepoprawnym
FPS, eksport PNG/JSON oraz zakończenie kodem 0 bez panic. Końcowy render
trwał około 0,46 s. Potwierdzenie: `installed-smoke/report.json` i
`SPRITE_MAKER_FIX_QA.json`.

Raporty i stderr: `C:/AI2D/pipeline/runs/sprite-maker-fix-20261111`.
Próby używają własnej bazy przez środowisko APPDATA procesu i osobnego
workspace. Nie otwierają okna, nie odtwarzają dźwięku i nie uruchamiają
dostawcy AI. Projekt gry i jego zasoby nie są wysyłane poza komputer.

Skrypt powtarzający próbę: `tools/media/sprite_studio_smoke.py --no-audio`
z argumentami `--exe <poprawiony-exe>` i `--output <nowy-katalog>`.

To odbiór naprawy renderera. Próbny rig nie jest zatwierdzoną nową animacją
bohatera. Diagnostyczny pasek 3840 × 360 nie został zaimportowany jako atlas
gry; produkcyjny limit 2048 i dotychczasowe animacje Emberfall pozostają.

Źródła techniczne: [oficjalny projekt](https://github.com/JohnKinyanjui/sprite-maker),
[Tokio — blokowanie pracy CPU](https://docs.rs/tokio/latest/tokio/task/),
[manifesty Windows](https://learn.microsoft.com/en-us/windows/win32/sbscs/application-manifests).

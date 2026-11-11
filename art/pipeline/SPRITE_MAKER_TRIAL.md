# Próba Sprite Maker / Sprite Studio — 11.11.2026

Aktualizacja: ujawniony niżej błąd został później naprawiony lokalnie;
zainstalowana poprawka przeszła rzeczywisty render i eksport.
[Naprawa i aktualny odbiór](SPRITE_MAKER_FIX.md). Poniżej zachowano wynik
próby oficjalnego, niezmodyfikowanego wydania.

Sprawdzono oficjalne **Sprite Studio 0.3.3**, źródło
`JohnKinyanjui/sprite-maker`, commit `9de73a6136a1a045af2e8769349fbadc17dbef69`.
Licencja MIT znajduje się w pobranym źródle. Pakiet Windows pobrano z
oficjalnego wydania i sprawdzono względem opublikowanego SHA-256:
`529fbd32e713029fe534bbd79b6952ed383138f6993c0245a4105e90b9a64ff5`.

MSI rozpakowano administracyjnie do
`C:/AI2D/Tools/SpriteStudio-v0.3.3/extracted`. Nie wyłączano ochrony Windows.
Uruchomiono dostarczony `sprite-studio-mcp.exe`, bez okna GUI i usług AI.
Nie zmieniano konfiguracji MCP Codexa ani istniejących animacji gry.

## Sprawdzone działanie

- Inicjalizacja natywnego MCP i odczyt listy narzędzi.
- Osobny projekt w
  `C:/AI2D/pipeline/runs/emberfall-loot-foley-20261111/sprite-trial/workspace`.
- Import przez natywny indeks zasobów: przezroczysty render bohatera
  Emberfall 320 × 360, wykonany przez aktualny renderer gry.
- Zapis natywnego rigu: punkty stawów z rzeczywistego szkieletu, 12 kości
  oraz 12 faz próbnego chodu. Analiza dopasowania i walidacja ruchu.
- Eksport PNG i metadanych oraz zapis wzorca skali/baseline/pivotu.

Automatyczna analiza uzyskała dopasowanie kapsuł około 0,707; rozpoznanie
kierunku było niepewne. Nie uznajemy tych wartości za ocenę jakości grafiki
ani potwierdzenie pełnej obsługi naszego widoku izometrycznego.

## Błąd renderowania w automatyzacji

Pierwszy szkic został prawidłowo odrzucony jako `missing_body_motion`:
ruszały się kończyny, lecz tułów nie przekraczał progu widocznego ruchu.
Po dodaniu rotacji i kompresji tułowia walidacja przeszła dalej, ale
`render_rig_animation` zakończył się panic:

`Cannot start a runtime from within a runtime`

Wskazane miejsce: `src/rig/commands.rs:267`. Synchroniczny handler MCP
wywołuje tam `tauri::async_runtime::block_on` wewnątrz aktywnego runtime
Tokio. Nie uzyskano klatek z tego natywnego renderu; nie deklarujemy
sprawnego generowania animacji ani lepszej jakości od naszego Synfiga.
GUI nie było przedmiotem tej próby, więc nie rozszerzamy wyniku na jego
osobną ścieżkę renderowania. Nie przebudowywano ani nie poprawiano programu.

Raport i zachowany rzeczywisty stderr są w katalogu próby:
`mcp-render-bug.json`, `render-panic-stderr.txt`, `rig-input.json`,
`rig-corrected.json` oraz `report.json`. Późniejsza osobna sesja eksportu
nie wygenerowała stderr; jej pusty `trial-stderr.log` nie jest logiem panic.
Nie wysłano projektu do żadnej usługi ani nie zgłoszono issue w GitHubie.

Na teraz narzędzie nadaje się do dalszych prób importu, skali i eksportu,
ale jego sprawdzona wersja MCP wymaga poprawki przed automatycznym
renderowaniem animacji Emberfall. Produkcyjny ruch nadal powstaje w Synfigu.

Źródło: https://github.com/JohnKinyanjui/sprite-maker/releases/tag/v0.3.3

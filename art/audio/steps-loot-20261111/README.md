# Kroki i zbieranie łupu — 11.11.2026

Zastąpiono sześć plików istniejących zdarzeń `step` i `pickup`, po trzy
warianty. Kroki wykorzystują nagrane podłoże, kontakt podeszwy i cichy ruch
ubrania. Zbieranie to ruch skóry/tkaniny oraz delikatny kontakt drobnych
przedmiotów. Nie użyto tonalnej syntezy ani melodyjnego potwierdzenia.

Materiały: lokalne, zachowane nagrania Kenney **Impact Sounds** oraz
**RPG Audio**, CC0 1.0. Licencje są już w `assets/licenses/audio-foley-CC0.txt`.
SHA-256 każdego wejścia, przycięcie i pochodzenie warstw zapisano w
`C:/AI2D/audio/emberfall-steps-loot-20261111/sources.json`.

Rzeczywisty ComfyUI wykonał graf 96 natywnych węzłów: wczytanie nagrań,
poziomy, przesunięcia czasowe, warstwowanie, korekcję i zapis sześciu FLAC.
Graf pracował lokalnie na CPU, bez modeli generatywnych i płatnych API.
Wejściowe nagrania wybrano celowo dla realistycznego foley. Historia zadania
`9c52adc9-b7d4-474f-a8a2-1e96a6eec115` i graf są w katalogu masterów.

Audacity 3.7.9 przetworzyło sześć osobnych regionów w jednym własnym
projekcie: normalizacja, usunięcie DC i wygaszenie końców. Końcowy eksport
to stereo PCM16, **48 kHz**. Zachowano rzeczywisty AUP3; kopię utworzono
przez SQLite Backup z uwzględnieniem aktywnego WAL i sprawdzono
`PRAGMA integrity_check = ok`. Projekt pozostaje zapisany i otwarty;
nie wysłano Close ani Play. Historyczne `C:/AI2D/audio/scripts` na tym PC
nie istnieje — użyto istniejącego skonfigurowanego MCP, bez instalowania
nowego serwera. Log poleceń: `audacity-commands.json`.

Kroki trwają 0,30 s, szczyt −3 dBFS; zbieranie 0,42 s, szczyt −5 dBFS.
Wszystkie próbki sprawdzono bez odsłuchu: brak clippingu, prawidłowe kanały
i częstotliwość, mały offset DC, różne warianty i wygaszone brzegi.
Poziomy miksera i zapisane preferencje użytkownika pozostają bez zmian.
Bank nadal ma 50 zdarzeń / 150 WAV oraz cztery utwory muzyczne.

Zegar kroków korzysta teraz z dwóch kontaktów stóp w cyklu animacji.
Poprzedni próg odległości odtwarzał dodatkowe kroki między kontaktami.
Zachowano pole `stepDistance` dla zgodności istniejących zapisów; format
zapisu pozostaje v15. Sprawdzono chód i bieg przy 30/60/120 aktualizacjach
na sekundę, zatrzymanie, blokadę ruchu oraz brak kroków w przeturlaniu.

## Weź wszystko

Przycisk jest przy kafelkach otwartej skrzyni lub ciała. Zbiera cały łup,
również z innych stron panelu, łącząc zgodne stosy do 99 i wykorzystując
wolne miejsca. Najpierw powstaje plan całej operacji. Jeżeli choć jedna
rzecz się nie mieści, nic nie zostaje przeniesione i pojawia się błąd.
Fragmenty Serca Świtu uruchamiają dotychczasowy powrót do Eldrina dopiero
po przeniesieniu całego łupu. Jeden przycisk emituje jedno zdarzenie Pickup.

Weryfikacja: 477 595 kontroli logiki i animacji, 19 030 zapisu,
534 dekodera i miksera offline, 7 rzeczywistych kontroli UI: PASS.
Okna testowe były ukryte, bez urządzenia audio. Raporty i obrazy:
`C:/AI2D/pipeline/runs/emberfall-loot-foley-20261111/qa`.

Źródła: `tools/media/steps_loot_foley.py`, `tests/loot_all_tests.hpp`,
`tests/loot_all_qa.cpp`. Odtworzenie UI:
`pwsh -NoProfile -File tests/loot_all_qa.ps1 -NoAudio`.
Masterów ani narzędzi produkcyjnych nie dołącza się do gry.

Oba EXE i archiwa Windows przebudowano. Każde rozpakowane archiwum ma
287 zasobów zgodnych SHA-256 ze źródłem i przeszło 180 klatek startowych
z `--no-audio --background-test`, w ukrytym oknie. To kontrola uruchomienia,
nie pomiar płynności na pierwszym planie. Wyniki i hashe: `QA.json`.

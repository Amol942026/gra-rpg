# Lekcje żywiołów u Eldrina — 09.11.2026

Bieżący kanon gry: po pokonaniu Kamiennego Strażnika bohater wraca do
Eldrina z pierwszą częścią Serca Świtu. Mag chwali jego dokonanie i oprócz
podróży oferuje naukę trzech zaklęć. Zapłata pokrywa runy i składniki.
Nie wymaga dodatkowej części artefaktu ani pokonania kolejnego strażnika.

Zmieniona końcowa kwestia pierwszego powrotu jest w `src/tower_story.hpp`;
wybór lekcji działa w istniejącym menu NPC. Zachowano charakter fantasy,
pozostałe rozmowy, bramy zadań i 12 rozmów / 57 kwestii.

Zgodnie z wcześniejszą instrukcją użytkownika aktualne dialogi w grze są
źródłem obowiązującej treści. `source-snapshot/` pochodzi z narzędzia
`story_sync.py extract`; manifest jawnie oznacza brak importu natywnego.
Nie jest to eksport articy. Aktualny `articy_cli.py status` nie znalazł
aktywnego mostu/projektu Emberfall, więc nie zapisano zmian w articy.
Dostępny osobny most Wichrogród nie dotyczy tego projektu i nie był używany.
Przy późniejszej synchronizacji należy odczytać rzeczywistą rewizję i GUID
Emberfall, zachować natywne zmiany i dopiero rozwiązać ewentualne konflikty.

Implementacja i odbiór: `art/gameplay/eldrin-spells-20261109/README.md`.

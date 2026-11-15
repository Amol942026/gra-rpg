# Kaldera w trybie deweloperskim — 29.10.2026

Menu F3 ma 19 miejsc w trzech kolumnach. Nowa prawa kolumna:

| ID CLI | Przycisk | Start |
| --- | --- | --- |
| `volcano` | Kaldera — Brama Popiołu | bezpieczne wejście do krainy |
| `volcano-forges` | Obsydianowe Kuźnie | wschodni szlak (60;43) |
| `volcano-boss` | Boss — Varkhor | przy arenie (56;15) |

Aliasy: `kaldera`, `caldera`, `varkhor`. Stare ID i pozycje na liście są
zachowane. Nazwy i opisy mieszczą się w kartach przy skalach 80%, 100%, 120%.

`NOWA SESJA TESTOWA` tworzy nową postać i żywego trzeciego bossa.
`PRZENIEŚ OBECNĄ POSTAĆ` zachowuje sprzęt, punkty, wyższe rangi i stan
przeciwników. Obie akcje ustawiają poziom 100 i odblokowują wszystkie czary
oraz umiejętności. Plecak jest uzupełniany do 100 strzał i po 50 mikstur
zdrowia i many. Przedmioty z pełnego plecaka, które ustępują miejsca zapasom,
można podnieść obok miejsca przybycia.

Obie akcje wybierają wolne miejsce poza lawą i ciałami
bestii, odnawiają HP/manę i anulują pociski, erupcje, odrzut, upadek,
piaskową pułapkę oraz otwarte okna łupu. Zdobyte niewydane punkty zostają.

Sesja DEV w kalderze ustawia licznik wcześniejszych części na co najmniej 2
i flagę ukończenia drugiego bossa, aby działała podróż powrotna u Eldrina.
Varkhor pozostaje żywy w nowej wyprawie.

Uruchamianie:

- W projekcie: `Uruchom.bat`, następnie F3 i zakładka **Kaldera Popiołu**.
- Szybko: `Uruchom.bat --start=volcano` albo `Uruchom.bat --start=volcano-boss`.
- Oddzielna paczka: `Emberfall-DEV-Windows-x64.zip`. Po wypakowaniu
  uruchom `Uruchom.bat`. Z `--dev` otworzy od razu wybór lokacji.

Produkcja paczki: `pwsh -NoProfile -File package-dev.ps1`.
Używa kompilacji deweloperskiej i zachowuje kopię poprzedniego archiwum DEV.
Zwykły `Emberfall-Windows-x64.zip` pozostaje wydaniem dla gracza.

Odbiór postaci DEV z 09.11.2026 obejmuje 234935 kontroli logiki/animacji,
10646 zapisu, 260 kontroli interfejsu Kaldery i 64 wbudowane kontrole menu.
`tests/developer_volcano_package_qa.ps1 -NoAudio -HelperRoot <helper>`
sprawdza poziom, zapasy i odblokowania podczas trzech startów po wypakowaniu.

Wszystkie testy mają `--no-audio --background-test` i ukryte okna; nie
przywracają fokusu. Nie zmieniają zapisanych preferencji dźwięku. Testy te
nie stanowią pomiaru FPS na ekranie. Poprzednie pliki zachowano w
`art/pipeline/backups/developer-volcano-20261029`.

Podgląd pierwotnego menu z 29.10.2026 znajduje się w `menu.png`.

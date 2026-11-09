# Spójność dialogów — bieżąca gra, 08.11.2026

Źródłem prawdy są aktualne pliki gry, zgodnie z wyraźnym poleceniem
użytkownika. Przegląd objął prolog, wszystkie powroty i rozmowy z Eldrinem,
opisy podróży, kroniki, obserwatorium, projekcję Serca Świtu i dziennik.
Treść articy z 30.10.2026 jest historyczna i nie została użyta do nadpisania gry.
W tym zadaniu nie zmieniono natywnego projektu articy. `current-game-final/`
jest końcowym, oznaczonym zrzutem źródeł gry, a nie eksportem natywnego MDK.
`current-game/` zachowuje wcześniejszy etap tej samej korekty.

## Ustalone fakty opowieści

- Moc Serca Świtu stworzyła Pieczęć Popiołu oddzielającą świat od Otchłani.
  Serce podzielono dawno temu. Dzisiejsze słabnięcie Pieczęci nie jest
  ponownym rozpadem artefaktu. Do odnowienia ochrony potrzeba wszystkich pięciu części.
- Eldrin podtrzymuje Pieczęć zaklęciami i kieruje wyprawą. Mówi spokojnie
  i konkretnie. Wędrowiec jest odważny, praktyczny, z oszczędnym humorem.
  Ich znajomość stopniowo staje się bliższa.
- Kolejność znanych strażników: Kamienny Strażnik w północnych ruinach
  Wielkiej Kniei, Żywa Wydma w południowo-wschodnim Morzu Piasków,
  Varkhor w północno-wschodniej Kalderze Popiołu.
- Odłamki pozostają przy bohaterze. Pięć znaków wieży to ich magiczne
  odbicia. Rozmowy nie opisują przedwczesnego złożenia Serca ani zużycia części.
- Po trzeciej części Eldrin szuka śladów dwóch pozostałych strażników.
  Nie obiecuje natychmiastowego przejścia do nieistniejącej w grze krainy.
  Obecne bramy pozostają dostępne.

## Zmienione zachowanie

12 rozmów / 57 wypowiedzi. Wszystkie są w [DIALOGI.md](DIALOGI.md).
Prolog zachowuje końcowe „Ruszaj więc” i dotychczasową teleportację.
Powrót po zwycięstwie oraz późniejsza rozmowa mają osobne teksty.
Po trzeciej części NPC nie odtwarza już ponownie wypowiedzi bohatera
o świeżym zwycięstwie nad Varkhorem. Jedna funkcja wybiera zarówno tekst,
jak i długość rozmowy, eliminując rozbieżność między tymi dwoma decyzjami.

Nowe warianty obejmują odłamki pozostawione przy Żywej Wydmie i Varkhorze,
również odebranie części Varkhora przed zabraniem drugiej części.
Pokonanie strażnika nie oznacza w dialogu automatycznie zebrania jego łupu.
Read-only zapytania sprawdzają także zapisane, nieaktywne krainy.
Licznik prezentacji i złote odbicia pomijają część nadal znajdującą się
przy zwłokach. Dawny licznik etapów i warunki podróży pozostają bez zmian.
Można nadal wyruszyć do Kaldery po pokonaniu Żywej Wydmy i wrócić po jej
odłamek później. Nie dodano pól do zapisu ani migracji.

Kroniki i opisy podróży mówią o aktualnym stanie strażników. Obserwatorium
nie szuka już drugiego strażnika po odzyskaniu trzech części. Nazwa pierwszej
krainy odpowiada mapie: Wielka Knieja. Poprawiono także kontekst pauzy
w Kalderze i nazwę krainy w przeglądzie zapisów.

## Odbiór

- 200040 kontroli logiki i animacji oraz 4513 zapisu/odczytu — PASS.
  Nowe kontrole przechodzą rzeczywistą trasę: pierwszy odłamek → pokonanie
  Żywej Wydmy bez zabierania łupu → Kaldera → powrót po pominięty odłamek.
- `tests/dialogue_qa.ps1 -NoAudio`: 157 kontroli rzeczywistego renderera,
  12 rozmów, wszystkie 57 wypowiedzi, wybory podróży i pozostania w wieży,
  brak aktywności po utracie fokusu, kroniki i skale okna — PASS.
- `tools/narrative/test_articy_bridge.py`: 32 kontrole offline — PASS.
  Mapping źródeł obejmuje 12 aktualnych tablic. Stare niepełne eksporty
  są odrzucane. Te kontrole nie oznaczają połączenia z natywnym articy.

Okna testów są ukryte; każdy test gry działa bez audio i bez zmiany
preferencji dźwięku użytkownika. Nie jest to pomiar foreground FPS.
Źródła i obie paczki sprzed zmiany: `art/pipeline/backups/dialogue-coherence-20261108`.
Raport renderera i logi: `renderer-QA.json` / `renderer-checks.txt`.

Oba EXE i oba ZIP-y przebudowano. Po wypakowaniu potwierdzono zgodność
147 zasobów w każdej paczce oraz hashe właściwych EXE. Rzeczywisty EXE
gracza przeszedł 180 klatek startu; DEV po 180 klatek trzech krain.
Helper wydania gracza ponownie wyrenderował wszystkie 57 wypowiedzi
i przeszedł 157 kontroli na zasobach z ZIP-a. Preferencje audio nie zmieniły
się również w lokalnym `dist`. Wyniki: `validation.json` i `package-verification.json`.

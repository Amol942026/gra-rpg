# Głośniejszy miks i fizyczne odgłosy walki — 08.11.2026

Nowe ataki wykorzystują krótkie warstwy foley, zamiast tonalnych przebiegów
oscylatorów z pierwszego zestawu. 28 efektów w 84 wariantach przebudowano:
świst miecza i mocnego ciosu, rozbójnik z bronią, kości, chityna, drewno,
pancerz, syczenie węża, warczenie bestii i gadów, obrażenia, upadki, pocisk
łuku, powietrze/magia, jad, fala, rozłupanie i oddech ognia. Cały bank ma
teraz **41 zdarzeń × trzy warianty = 123 WAV**, plus cztery OGG muzyki.

Materiały wejściowe:

- [Kenney — Impact Sounds](https://kenney.nl/assets/impact-sounds), CC0: uderzenia miękkie, drewno, metal, płyty i materiał skalny.
- [Kenney — RPG Audio](https://kenney.nl/assets/rpg-audio), CC0: tkaniny, skóra, skrzypienie i ruch elementów broni.
- [artisticdude — Swishes Sound Pack](https://opengameart.org/content/swishes-sound-pack), CC0: nagrane ruchy patyków i wieszaka, opisane przez autora.
- [Darsycho — Monster snarls](https://opengameart.org/content/monster-snarls), CC0: krótkie fragmenty `monster-snarl-attack.ogg`.

Źródłowe ZIP/OGG, strony z deklaracjami licencji, notices i SHA-256 są
zachowane w `C:/AI2D/audio/emberfall-physical-20261108/sources`. Pochodzenie
i warunki dołączono do `assets/licenses/audio-foley-CC0.txt`. Nie wysyłano
projektu do usług i nie używano płatnych API. Oryginalne kompozycje LMMS
pozostają te same; aktualizacja zmienia ich poziom, nie nuty i instrumenty.

Produkcja: dekodowanie zmiennoprzecinkowe, odcięcie niepotrzebnego dołu,
przycięcie nagrań do początku gestu, wyrównanie warstw, filtry i montaż w
rzeczywistym Csound 6.18.1. Powietrze, syk i ogień mają nieintonowany szum.
CSD nie zawiera oscylatorów tonalnych, generowania nut ani cyfrowych glissand.
Następnie rzeczywista normalizacja/DC i eksport w Audacity 3.7.9, zapis AUP3
oraz kontrola SQLite. Projekt pozostaje zapisany i zminimalizowany; nie
wysyłano Close ani Play. Oryginały zachowano. Każdy eksport to stereo
48 kHz PCM16, z krótkimi zabezpieczeniami brzegów i szczytem około −2,2 dBFS.

Zwykłe ataki wrogów emitują gest w momencie przejścia przygotowania w cios.
Trafienie i obrażenia pozostają osobne; chybienie nie gra dźwięku trafienia.
Rozbójnik korzysta z gestu broni, szkielet z kości, kobra z syku, drzewo
z drewna, wojownik z pancerza, gady i bestie z własnych grup. Mocny cios
bohatera ma osobny cięższy świst. Szarża i rzut głazem nie używają już
zwykłego świstu miecza. Mechanika obrażeń, czasy animacji i RNG nie zmieniają się.

Domyślne poziomy: muzyka **42 → 85%**, efekty **72 → 95%**, główny poziom
**72 → 90%**. Menu/pauza zostawia **65%** muzyki zamiast 42%. Oba kanały są
domyślnie włączone. Na wyraźne polecenie użytkownika ustawiono również
włączone lokalne konfiguracje obu EXE, zachowując suwaki ustawione wyżej.
Kolejne własne zmiany w Opcjach nadal są respektowane i zapisywane.

Muzyka została ponownie przygotowana z nieskompresowanych masterów LMMS/
Audacity: −16 LUFS dla krain, −17 LUFS dla wieży (około +3 dB). Finalne
OGG q5 mają −16,00 do −16,98 LUFS i −2,05 do −1,90 dBTP. Pętle zachowują
dokładną liczbę próbek po pełnym dekodowaniu. Miks zachowuje limiter szczytów.

Nowe Cue są dopisane za poprzednimi identyfikatorami. Zapis **v13** wyraźnie
oznacza rozszerzony bank; odczyt v1–v12 działa. Prawdziwy fixture v12,
utworzony kompilacją kopii poprzedniego kodu, odtwarza bajtowo cały payload,
w tym pozycję, ekwipunek, ciała, zasoby i RNG. Rozbudowane mapy pozostają
w rewizji 1, z Kalderą w rewizji scenografii 5.

Kod: `tools/media/refine_combat_audio.py`. Mastery, CSD, receptury warstw,
AUP3, logi, wyniki pomiarów i backup SQLite: `C:/AI2D/audio/emberfall-physical-20261108`.
Backup zasobów i poprzednich EXE/ZIP: `art/pipeline/backups/audio-refinement-20261108`.
Atomowe podpięcie wszystkich wariantów i czterech ścieżek ma osobny receipt.

PASS: 234852 kontrole logiki/animacji, 10202 zapisów i 435 rzeczywistego
AudioSystem z wyjściem w pamięci i dekoderem raylib. Sprawdzono 123 WAV,
cztery pełne OGG, nowe domyślne poziomy, niezależne wyciszenie, przejścia,
ducki menu/pauzy, wybór wariantów i zwalnianie głosów. Testy nie otwierają
urządzenia, nie tworzą widocznych okien i nie zmieniają zatwierdzonych
ustawień użytkownika. Nie wykonywano odsłuchu artystycznego na głośnikach.
Końcowy odbiór i paczki: `QA.json`.

Gotowe EXE i oba ZIP-y przeszły odbiór po wypakowaniu: po 253 zgodne zasoby,
gracz — 180 klatek startu oraz ponownie 435 kontroli audio, DEV — 3 × 180
klatek lasu, pustyni i Kaldery. Osiem dodatkowych kontroli natywnego EXE
potwierdza przełączniki Opcji w menu początkowym i pauzie. Okna były ukryte,
urządzenie audio zamknięte, zatwierdzone ustawienia pozostały niezmienione.
Po końcowej korekcie licencji i instrukcji ZIP sprawdzono hashe wszystkich
wpisów; EXE i zasoby zachowały identyczne bajty. Końcowe SHA-256 i wyniki
są w `package-verification.json`; pochodzenie i mastery w `provenance.json`.

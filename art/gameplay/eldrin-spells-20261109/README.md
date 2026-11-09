# Lekcje Eldrina i trzy żywioły — 09.11.2026

Po pokonaniu Kamiennego Strażnika Eldrin w swojej wieży oferuje wybór
„Naucz mnie władać żywiołami”. Każde zaklęcie, również wcześniejsza Kula
ognia, wymaga jednorazowego zakupu. K wybiera poznany czar, C go rzuca.

| Zaklęcie | Złoto | Mana | Bazowe obrażenia | Odnowienie | Zasięg |
| --- | ---: | ---: | ---: | ---: | ---: |
| Kula ognia | 600 | 10 | 28 | 0,8 s | 8 pól |
| Sopel lodu | 1500 | 20 | 48 | 1,8 s | 9 pól |
| Kulisty piorun | 3000 | 35 | 84 | 3,5 s | 9 pól |

Moc magiczna zwiększa obrażenia wszystkich trzech czarów. Piorun ma też
największy obszar trafienia. Przełączenie zaklęcia nie skraca trwającego
odnowienia. Lecący pocisk zachowuje własny typ, obrażenia, prędkość i efekt
trafienia niezależnie od późniejszej zmiany czaru.

Zakup sprawdza pokonanie pierwszego bossa, obecność przy Eldrinie na jego
piętrze, dostępność postaci i łączną ilość złota w sakwie/plecaku. Ponowny
wybór poznanego zaklęcia tylko je zakłada; nie zabiera złota. Brak środków
nie zmienia stanu postaci. Pełny plecak nie blokuje nauki.

## Grafika i animacja

Własny warstwowy master Krita: `C:/AI2D/sprites/emberfall-spells-20261109/krita/`.
Atlas `assets/sprites/spells.png` ma 768 × 768 RGBA: trzy ikony, trzy rdzenie
pocisków i trzy trafienia. Wczytywany raz; 2 359 296 bajtów RGBA8.
Płomień, lodowy grot i kula burzy mają różne sylwetki, kolory i ślady.

Trzy rzeczywiste projekty Synfig 1.4.5 i 180 wyrenderowanych PNG (60 Hz,
dwa wątki) są w `C:/AI2D/sprites/emberfall-spells-20261109/animation-v2/`.
Zapisane ścieżki skali, obrotu i alfa są kompilowane do
`src/generated/spell_motion.hpp` i używane w grze. Wirtualny koniec pętli
równa się początkowi. Eksport 60 Hz nie jest pomiarem FPS gry.
Narzędzie Synfig odtworzono z oficjalnego przenośnego wydania, lokalnie.

## Dźwięk

Naciąg łuku oparto na rzeczywistych nagraniach English Longbow i Scythian
Recurve (Ben Jaszczak, Brian Nelson / StillNorthMedia, CC0); usunięto stare
brzmienie skrzypienia. Wyrzut ognia jest krótkim podmuchem z nagrania
„Torch Fire Spell” (spookymodem, CC BY 3.0). Lodowi i piorunowi odpowiadają
osobne rzucenia/trafienia z „Ice & Electricity Magic” (Iwan „qubodup”
Gabovitch, CC BY 3.0). Zakup lekcji ma osobne potwierdzenie.

21 wariantów zmontowano rzeczywistym Csound 6.18.1 i znormalizowano
w rzeczywistym Audacity 3.7.9. Stereo PCM16 48 kHz, szczyt −2,8 dBFS,
bez przesterowanych próbek. Master reel 22,31 s; zachowano CSD, recepty,
źródła, hashe i poprawny AUP3. Własny zapisany projekt Audacity pozostaje
zminimalizowany: bez Close, Play, Record i Scrub.

Mastery: `C:/AI2D/audio/emberfall-spells-20261109/`.
Pełna atrybucja: `assets/licenses/audio-elements-credits.txt`.
Atomowy binder zastąpił sześć WAV łuku/ognia i dodał 15 WAV nowych Cue.
Bank: 50 Cue / 150 WAV, cztery istniejące OGG. Muzyka i preferencje audio
zostały zachowane. Praca i kontrole odbyły się bez odtwarzania audio.

## Zapis i dialogi

Format v15 przechowuje poznane czary, wybrany czar, długość odnowienia,
typ i wiek pocisków oraz typ eksplozji. Odczyt v1–v14 pozostaje dostępny.
Starszy zapis zachowuje postęp, złoto, ekwipunek i lecące pociski; czarów
trzeba nauczyć się u Eldrina zgodnie z nową zasadą. Nie przyznajemy darmowej
Kuli ognia przy migracji.

Aktualna kwestia Eldrina wspomina o płatnych lekcjach i składnikach.
Istniejące dialogi gry pozostają źródłem bieżącego kanonu (12 rozmów,
57 kwestii). `narrative/eldrin-lessons-20261109/source-snapshot/` jest
snapshotem C++, nie eksportem articy. Natywny most Emberfall jest obecnie
niedostępny; nie zmieniano innego projektu Wichrogród ani nie deklarowano
zapisu w articy. Szczegóły: `narrative/eldrin-lessons-20261109/README.md`.

## Odbiór

- PASS: 234931 kontroli logiki/animacji, w tym zakup i blokada darmowych czarów.
- PASS: 10646 kontroli zapisu, autentyczny fixture v14 i odczyt v1–v14.
- PASS: 534 kontrole AudioSystem, rzeczywiste dekodowanie 150 WAV / czterech OGG.
- PASS: 22 kontrole natywnego interfejsu i renderera: rozmowa z Eldrinem,
  brak złota, wszystkie zakupy, ponowny wybór, zapis i trafienia trzech czarów.
- PASS: 180 rzeczywistych klatek Synfig; kontrola próbek, eksportu i AUP3 Audacity.
- PASS: oba ZIP-y po wypakowaniu, po 286 zasobów; wersja gracza uruchamia
  menu, DEV uruchamia las, pustynię i Kalderę. Wszystkie 154 pliki dźwięku
  są zgodne w projekcie, lokalnych kompilacjach i obu paczkach. Zachowano
  lokalne preferencje oraz dołączono końcowe licencje i instrukcję gracza.

Testy graficzne są ukryte i używają `--no-audio --background-test`.
Nie inicjują urządzenia audio. Szczegóły końcowych paczek i hashe: `QA.json`.
Natywne mastery, narzędzia i modele pozostają poza paczkami gracza.

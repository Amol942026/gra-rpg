# Animacje bohatera

Projekt hero_motion.blend zawiera armaturę 17 kości, sześć klipów i model kontrolny do podglądu. C++ podpina do kości szczegółowy model bohatera i wyposażenia. Ten sam model generuje obraz, cień i maskę wskazywania.

Odtworzenie danych w przetestowanym Blenderze 4.5.7 LTS, z głównego folderu projektu:

    blender --background --factory-startup -noaudio --threads 4 --python art/animation/build_hero_motion.py -- --no-audio

Skrypt zapisuje hero_motion.blend, bake-report.json oraz src/generated/hero_motion_data.hpp. Dane są kompilowane z grą; Blender nie jest zależnością gry ani paczki dla gracza. Animacje w pliku źródłowym mają znormalizowaną oś 1–121; nominalny czas jest zapisany jako właściwość duration_seconds każdej akcji. Gra dopasowuje tempo chodu i biegu do przebytej drogi.

Osie: X w prawo postaci, Y przed postacią, Z w górę. Root steruje ciężarem i obrotem, Pelvis/Spine/Head przenoszą ruch tułowia, a pozostałe kości sterują kończynami i bronią. Krzywe kontrolne Blendera są interpolowane Bézierem, kończyny rozwiązane IK, a eksport zapisuje transformacje względem rodzica. W grze FK oraz interpolacja kwaternionów zachowują sztywność odcinków.

Kontrole po zmianie: build.ps1 -Player -Test; grafika --graphics-qa --no-audio; zamach --swing-qa --no-audio; cienie --shadow-qa --no-audio; synchronizacja --motion-qa --no-audio. Nie uruchamiać gry/testów bez --no-audio podczas pracy nad projektem.

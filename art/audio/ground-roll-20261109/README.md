# Przeturlanie po ziemi

Istniejące zdarzenie `Roll` ma trzy nowe warianty, po 0,52 s. Warstwy
z nagrań CC0 Kenney (RPG Audio / Impact Sounds): szelest tkaniny, skóra,
ziemisty kontakt podłoża i ciche, miękkie przyziemienie. W montażu nie ma
tonalnej syntezy, elektronicznych zjazdów wysokości ani muzycznego motywu.
Odgłos zaczyna się podczas skulania, tarcie rozwija się przez obrót, a
kontakt barku/biodra wypada pod koniec istniejącej animacji 0,4 s.

Rzeczywisty Csound 6.18.1 połączył nagrania, a Audacity 3.7.9 usunął DC
i znormalizował master. Eksport: stereo PCM16 48 kHz, trzy warianty
`roll.wav`, `roll-v2.wav`, `roll-v3.wav`, peak około −3 dBFS bez clippingu.
Zachowano CSD, próbki wejściowe, SHA-256, AUP3 i poprawną kopię SQLite.
Zapisany projekt pozostaje zminimalizowany; nie używano Close ani playbacku.

Cue, czasy ruchu, uniki, RNG i zapis v14 nie zmieniają się. Bank nadal ma
45 zdarzeń / 135 WAV oraz cztery niezmienione ścieżki muzyczne. Domyślne
włączenie kanałów, głośność i zapisane preferencje użytkownika są zachowane.
Nowy zestaw zastępuje wyłącznie trzy pliki przeturlania przez atomowy binder.

Mastery: `C:/AI2D/audio/emberfall-roll-20261109`; produkcja:
`tools/media/ground_roll_audio.py`. Oryginały i poprzednie EXE/ZIP:
`art/pipeline/backups/roll-loot-20261109`. Licencje materiałów:
`assets/licenses/audio-foley-CC0.txt`. Odbiór w pamięci bez urządzenia audio:
`tests/audio_suite_qa.ps1 -NoAudio`. Techniczna kontrola ciszy i parametrów
nie jest odsłuchem artystycznym. Końcowy odbiór: `QA.json`.

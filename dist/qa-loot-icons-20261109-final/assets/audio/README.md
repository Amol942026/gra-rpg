# Dźwięk Emberfall

Aktualizacja 08.11.2026: głośniejszy miks, kanały domyślnie włączone,
muzyka 85%, efekty 95%, główny poziom 90%. Menu/pauza zachowuje 65% muzyki.

Cztery oryginalne kompozycje LMMS 1.2.2, po 32 takty, ze wspólnym motywem
Serca Świtu: „Wielka Knieja” (72 BPM, 106,67 s), „Szlaki Żywej Wydmy”
(76 BPM, 101,05 s), „Pieśń Kaldery” (88 BPM, 87,27 s) i „Wieża Eldrina”
(66 BPM, 116,36 s). Flet, harfa, smyczki, wiolonczela, bęben ramowy
i dzwonki są syntezą natywnych instrumentów LMMS, bez sampli.

45 efektów, każdy w trzech wariantach (135 WAV): ataki gracza
i przeciwników, trafienia zależne od materiału, obrażenia i śmierć bohatera,
turlanie, skrzynia, zakładanie broni i sprzętu, łup, kroki, czary i interfejs.
Własne brzmienia mają też wachlarz kobry, fala skarabeusza, cios wojownika
i oddech draka. Warianty nie zużywają losowości walki ani łupu.

Kula ognia ma osobny wyrzut płomienia i eksplozję. Pojawieniu z teleportu
towarzyszy odgłos materializacji, także podczas odsłonięcia sceny prologu.
Naciąganie łuku uruchamia skrzypienie drewna/cięciwy i jest uciszane po
puszczeniu, anulowaniu oraz pauzie. Cztery nowe Cue / 12 wariantów:
`art/audio/flame-20261108/README.md`; bez tonalnych oscylatorów.

28 efektów walki w 84 wariantach korzysta z nagranych świstów, uderzeń,
tarcia materiałów i odgłosów bestii: Kenney, artisticdude i Darsycho, CC0.
Warstwy złożono w Csound i obrobiono w Audacity, z dodatkiem nieintonowanego
szumu powietrza, syku i ognia. Bez tonalnych oscylatorów w nowej walce.
Szkielet, chityna, drewno, pancerz, rozbójnik i gady mają osobne gesty;
mocny cios bohatera ma cięższy świst. Zwykłe ataki wrogów brzmią w chwili
ciosu, po zakończeniu przygotowania. Muzyka i pozostałe efekty zachowują
oryginalne źródła Emberfall. Można rozprowadzać z grą; pełne pochodzenie:
`assets/licenses/audio-foley-CC0.txt`.
Rzeczywista obróbka w Audacity 3.7.9: zachowane projekty AUP3,
usunięcie DC i normalizacja, pogłos muzyczny; nowa walka ma osobny zapis AUP3.
Wydanie: stereo 48 kHz,
PCM16 dla SFX i OGG Vorbis q5 dla muzyki. Pętle wycięto ze środkowego
z trzech wyrenderowanych cykli. Muzyka: −16 LUFS, wieża −17 LUFS.

Mastery MMP/CSD, WAV i AUP3: `C:/AI2D/audio/emberfall-suite-20261108`.
Aktualne efekty i głośniejsze mastery: `C:/AI2D/audio/emberfall-physical-20261108`.
Kod: `tools/media/emberfall_audio_suite.py`, `tools/media/refine_combat_audio.py`;
manifest: `production.json`.
`audio/create_audio.py` opisuje historyczną wersję. Narzędzia i źródła
nie są potrzebne ani dołączane do wydania gry.

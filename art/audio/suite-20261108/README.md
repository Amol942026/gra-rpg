# Oprawa dźwiękowa Emberfall — 08.11.2026

Cztery kompozycje LMMS, 33 efekty Csound, po trzy autorskie warianty, oraz
rzeczywista obróbka w Audacity. Wykonane lokalnie, bez płatnych usług,
zewnętrznych sampli i odtwarzania na głośnikach. Zestaw jest podpięty w grze.

| Muzyka | Tempo | Długość pętli | Charakter |
| --- | --- | --- | --- |
| Serce Świtu — Wielka Knieja | 72 BPM | 106,67 s | Flet, harfa, spokojne smyczki |
| Szlaki Żywej Wydmy | 76 BPM | 101,05 s | Modalna melodia i pustynne ostinato |
| Pieśń Kaldery | 88 BPM | 87,27 s | Ciemniejsze harmonie i mocniejszy bęben |
| Wieża Eldrina | 66 BPM | 116,36 s | Dzwonki, rzadsza faktura i dłuższy pogłos |

Każdy utwór ma 32 takty i własną aranżację, ze wspólnym motywem Serca Świtu.
Wszystkie instrumenty są syntezą natywnych TripleOscillator/Kicker, bez sampli.
Nazwa instrumentu określa jego rolę i charakter, nie nagranie akustyczne.
LMMS wyrenderował trzy cykle; po obróbce w Audacity pobrano cykl środkowy,
z zachowanymi ogonami. PCM master: stereo 48 kHz / 16 bitów. Wydanie:
Vorbis q5. Cel: −19 LUFS dla krain, −20 LUFS dla wieży, z zapasem true peak.

Efekty obejmują wszystkie dotychczasowe zdarzenia i nowe: otwieranie skrzyni,
dobycie/założenie broni, trafienie chityny i istoty żarowej, przygotowanie
ataku bestii, kamiennego przeciwnika i maga oraz cztery zdolności specjalne.
Przy trafieniu gatunek określa materiał; szlam, ciało, kość, kamień, chityna
i żar mają różne brzmienia. Obrażenia bohatera i jego śmierć pozostają osobne.
Turlanie, podniesienie łupu, zakładanie sprzętu i otwieranie skrzyni emitują
efekt po udanej czynności. Odrzucone akcje nie odtwarzają dźwięku sukcesu.

Trzy warianty każdego efektu są przygotowane w Csound, w osobnych plikach.
Silnik ładuje je raz i przeplata w ograniczonych bankach trzech głosów;
subtelna zmiana wysokości używa własnego licznika, bez RNG walki i łupu.
Cztery strumienie muzyczne są przygotowane przy starcie; zmiana krainy
nie otwiera plików i płynnie wygasza poprzedni utwór. Głośność, wyłączenie
muzyki/efektów, pauza i utrata aktywności obejmują cały miks.

Mastery: `C:/AI2D/audio/emberfall-suite-20261108`: cztery MMP, `effects.csd`,
partytury i definicje, renderowane WAV, pięć AUP3, logi i receptury.
Programy na tym PC są przenośne w `AppData/Local/Programs/CodexAudio`.
Historyczny klient `C:/AI2D/audio/scripts` nie istnieje na tym komputerze;
użyto istniejących, lokalnych narzędzi MCP. Nie instalowano nowego serwera.
Audacity zapisuje i pozostawia pięć projektów otwartych — nie wysyłano
Close, Play, Record ani Scrub. Ich kopie wykonane przez SQLite backup mają
`quick_check=ok`, zachowując obejście awarii zamykania 3.7.9.

Kod przygotowania/odbioru: `tools/media/emberfall_audio_suite.py`.
Własne ustawienia syntezy i nuty są w MMP/CSD. Dalsze wykończenie obejmuje
usunięcie DC każdego krótkiego efektu i 3 ms zabezpieczenia brzegów.
Pętle sprawdzono po pełnym dekodowaniu Vorbis; nie zmieniają liczby próbek.
Oryginały runtime i EXE/ZIP sprzed zmian: `art/pipeline/backups/audio-suite-20261108`;
osobny atomowy backup podpięcia bazowych plików wskazuje `binding.json`.

Odbiór obejmuje testy zdarzeń gry, zapisów, limiter i rzeczywiste dekodery
raylib, a także wykonanie prawdziwego AudioSystem z wyjściem zastąpionym
pamięcią w teście. Ten test sprawdza ładowanie, wybór wariantów, przejścia,
wyciszenie, focus, pauzę i zwalnianie zasobów bez otwierania urządzenia.
Wyniki końcowe: `QA.json`. Nie wykonywano odsłuchu artystycznego; techniczna
walidacja nie jest oceną brzmienia ze słuchu. Ustawienia audio gracza zachowane.

PASS: 200189 kontroli logiki/animacji, 5162 zapisów, 353 dekodera i wyjścia
w pamięci oraz osiem kontroli prawdziwego menu Opcji w ukrytym DEV EXE
z `--no-audio --background-test`. Obie paczki po wypakowaniu: zgodne EXE
i po 228 zasobów. Wersja gracza: 180 klatek startu i ponowne 353 kontrole
audio z zasobami z ZIP-a; DEV: 3 × 180 klatek krain. Cztery dostarczone
OGG: −18,97 do −19,98 LUFS, true peak od −5,12 do −3,73 dBTP, bez zmian
liczby próbek po dekodowaniu. Wszystkie 103 pliki audio mają zgodne hashe
w źródle, obu aktywnych kopiach i paczkach. Masterów nie dołączono do ZIP-ów.

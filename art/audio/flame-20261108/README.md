# Płomień, pojawienie z teleportu i naciąganie łuku

Cztery nowe, niezależne zdarzenia w trzech wariantach: `fireball-cast`,
`fireball-impact`, `teleport-arrival`, `bow-draw`. Bank zawiera teraz 45 Cue,
135 stereo WAV PCM16 48 kHz i cztery niezmienione kompozycje OGG. Kula
ognia wykorzystuje szerokopasmowy wyrzut płomienia oraz oddzielną, głębszą
eksplozję w miejscu trafienia, także w teren lub po wygaśnięciu pocisku.
Nie używa dawnego wspólnego `Spell`, nut ani tonalnych oscylatorów.

Pojawienie w lesie po prologu, powrót do wieży oraz podróże do pustyni
i Kaldery mają jeden odgłos materializacji w miejscu docelowym. Zdarzenie
przechodzi przez blokadę sterowania podczas odsłonięcia sceny; pauza,
brak focusu i wyłączony kanał efektów nadal zapewniają ciszę. Nie jest
zapętlane ani odkładane do późniejszego odtworzenia.

Udane rozpoczęcie naciągu uruchamia fizyczne skrzypienie drewna i cięciwy.
Brak strzał i powtórne przytrzymanie nie uruchamiają kolejnego efektu.
Po puszczeniu, anulowaniu, uniku, zmianie broni lub wejściu do menu
aktywny naciąg jest zatrzymywany. Dźwięk wypuszczenia strzały pozostaje osobny.

Warstwy CC0 z zachowanych źródeł poprzedniej produkcji (Kenney,
artisticdude) połączono z nieintonowanym szumem i pogłosem w rzeczywistym
Csound 6.18.1. Normalizację/DC, eksport i master AUP3 wykonano w Audacity
3.7.9. Finalny eksport sprawdzono przy 48 kHz; pośredni 44,1 kHz zachowano
wyłącznie jako etap roboczy. Dwanaście finalnych plików ma peak około
−2,5 dBFS, bez obciętych próbek. Master i kopia SQLite są poprawne, zapisane
okno pozostaje zminimalizowane. Nie wykonywano Close ani playbacku.

Źródła, CSD, receptura wycinków, finalne WAV, raporty i AUP3:
`C:/AI2D/audio/emberfall-flame-20261108`. Produkcja: `tools/media/flame_portal_audio.py`.
Pochodzenie sampli: `assets/licenses/audio-foley-CC0.txt`. Backup poprzedniej
wersji: `art/pipeline/backups/flame-teleport-icons-20261108`; podpięcie WAV
ma dodatkowy atomowy receipt. Domyślne poziomy i preferencje użytkownika
z poprzedniej aktualizacji pozostają zachowane.

Zapis v14 dopisuje Cue 41–44 po dotychczasowych 0–40. Odczyt v1–v13
pozostaje dostępny; pola payloadu nie zmieniły się. PASS: 234864 kontrole
logiki/animacji, 10244 zapisu, 482 cichego odbioru AudioSystem z prawdziwym
dekoderem raylib i wyjściem w pamięci. Odbiór nie otwiera urządzenia audio.
Nie wykonywano odsłuchu artystycznego na głośnikach. Końcowe wyniki: `QA.json`.

# Strażnik: szybszy rzut i podnoszenie — 26.10.2026

Zachowano duży głaz wyrywany z ziemi i rzucany obiema rękami znad głowy,
anatomiczne zgięcie łokci, rozpad na sześć odłamków, kolizje oraz upadek
na plecy z podparciem dłonią i przejściem przez kolano.

| Parametr | Poprzednio | Teraz |
| --- | --- | --- |
| Pierwsze przygotowanie po wejściu w zasięg | 0,7 s | 0,5 s |
| Zamach / podnoszenie głazu | 2,2 s | 1,5 s |
| Zakończenie ruchu rąk | 0,6 s | 0,4 s |
| Odnowienie rzutu od wypuszczenia | 4,5 s | 3,5 s |
| Lot kamienia | 7 pól/s | 11 pól/s |
| Minimalny czas lotu | 0,25 s | 0,20 s |
| Upadek i wstawanie | 2,8 s | 1,85 s |

Przyczyna błędnego celowania: pozycję kopiowano na początku zamachu i przez
cały zamach jej nie aktualizowano. Obecnie Strażnik śledzi bieżącą pozycję
bohatera aż do wypuszczenia. Dotyczy to również przesunięcia po poprzednim
uderzeniu. Punkt trafienia jest zamykany dopiero przy wypuszczeniu głazu.
Potem tor nie jest korygowany, więc przewrót i zejście z toru nadal pozwalają
uniknąć uderzenia. Znacznik na ziemi odpowiada temu samemu punktowi.

Rzut ponownie zapisano i wyrenderowano w Synfigu: 114 klatek, 115 próbek.
Upadek: 111 klatek, 112 próbek × 17 kości. Plecy lądują nadal w 0,45 s,
obie stopy są oderwane podczas lotu; odpoczynek na plecach 0,66–0,80 s,
podparcie 1,07 s, kolano 1,24 s, stanie 1,75 s, sterowanie 1,85 s.
Rzeczywiste starcie: 111 klatek blokady przy 60 Hz, 1,35002 pola przesunięcia.
Przyspieszono również odrywanie ziemi, aby odpowiadało nowemu tempu rąk.

Bez zmian: 24 bazowych obrażeń fizycznych, promień obrażeń 0,72 pola,
fala odrzutu do 1,6 pola, fizyczny ślizg 0,48 s, osłony i kolizje,
chroniony przewrót, anulowanie przy śmierci i zmianie lokacji. Źródło obu
chwytów i pocisku jest wspólne. 39 wariantów wyposażenia korzysta z tej samej
pozy i cienia. Punkt startowy nowej gry pozostaje na zachodnim krańcu lasu.

Zapis v6 przelicza fazy zamachu/odnowienia z v2–v5 i upadku z v4–v5,
przyjmuje też rzeczywiste pliki v1 i v3. Dochodzą stany dwóch jednorazowych
fontann; ich grafiki, animacje oraz handel dwuklikiem mają osobne testy.

Testy: `tests/guardian_qa.ps1 -NoAudio` i
`tools/media/verify_guardian.py --qa <folder> --previews --no-audio`.
Wykonano 1163 kontrole renderera/interfejsu oraz 4561 niezależnych kontroli
źródeł i klatek. Są wśród nich wszystkie pozycje w ośmiu kierunkach,
39 elementów eq, rzeczywiste trafienie, pauza, zapis, uniki, oba panele
handlarza i oba źródła pustyni. Szczegóły maszynowe: `QA.json`.

Źródła: `motion/guardian-throw.sif`, `../../hero/knockdown/knockdown.sif`.
Mastery: `C:\AI2D\animations\emberfall_guardian_fast_20261026` oraz
`C:\AI2D\animations\emberfall_backfall_fast_20261026`.
Wszystkie uruchomienia korzystały z `--no-audio`.

Końcowa weryfikacja: 104 688 kontroli logiki, 1723 zapisu, 1163
renderera/interfejsu oraz 4561 źródeł i obrazów — PASS. Paczka gracza została
rozpakowana i przetestowana ponownie; 110 zasobów ma zgodne hashe, 180 klatek
uruchomienia bez trybu DEV, bez otwierania audio i bez zmiany preferencji.

Pomiar samego renderowania po ukończeniu GPU (bez prezentacji okna):
p95 fontanny many 10,2789 ms, HP 11,3586 ms, Strażnika 14,0349 ms.
Budżet 60 Hz wynosi 16,6667 ms. To zapas dla renderowania, nie potwierdzenie
60 FPS wyświetlania. Pełne przebiegi w tle w OpenGL i DXGI przerwano po
stwierdzeniu blokowania w prezentacji sterownika AMD (atio6axx.dll / atidxx64.dll).
Nie zmieniano V-sync, sterownika ani globalnych ustawień grafiki.
Powtarzalny pomiar ukrytego renderowania: `tests/resource_performance_qa.ps1
-NoAudio -GameRoot <katalog-z-assets>` (jedno polecenie).

ZIP SHA-256: `f005bd24749f6f7376bd8e1ce42d5574eff1b8bf21c479a339a9517b31979bb1`.
EXE SHA-256: `4e668262b6d3cde63859dde230ecebedcf28b969ccb7b25153f6224616a0da87`.

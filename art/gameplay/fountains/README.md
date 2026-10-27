# Jednorazowe fontanny pustyni — 26.10.2026

Na Morzu Piasków są dwa stałe źródła: niebieskie przy (25, 22), odnawiające
całą manę, oraz czerwone przy (50, 37), odnawiające całe HP. Ich nazwy to
Źródło Błękitnej Pieczęci i Źródło Szkarłatnego Świtu. Każde ma jedno użycie.
E / LPM / PPM działa w zasięgu 2,5 pola; pełny zasób nie zużywa źródła.

Woda płynie i faluje w pętli. Użycie od razu przywraca zasób, następnie woda
opada przez sekundę i znika. Sucha misa zachowuje kolor kryształu. Zużycie
oraz środkowa faza opróżniania są zapisywane w formacie v6. Zmiana lokacji
nie odnawia fontann. Starsze zapisy v1–v5 otrzymują oba nieużywane źródła;
usuwa się jedynie dekoracje zachodzące na nowe miejsca.

Grafika została faktycznie narysowana przez istniejący Krita MCP: osobne
warstwy kamienia, tafli, kręgów, strumieni i kropli. Źródła KRA są w folderach
mana i health. Oryginalne części PNG zostały następnie animowane w Synfigu,
po 24 klatki pętli i 24 opróżniania dla każdego koloru. Są tu również natywne
SIF i niezmienione wyeksportowane warstwy. Wyłącznie darmowe narzędzia lokalne,
oryginalne kształty; brak zewnętrznych materiałów i modeli 3D.

Runtime: dwa atlasy RGBA 1024×960, suche obrazy 128×160, wspólna kotwica
stóp (64, 148). Łącznie 8 028 160 bajtów RGBA8 (~7,66 MiB). Tekstury ładowane
raz. Brak generowania grafiki i odczytu plików w pętli klatki. Woda 24 Hz
jest niezależna od odświeżania gry 60 Hz. Fontanny uczestniczą w sortowaniu
głębi, cieniach, wyborze kursorem, kolizjach i omijaniu przez potwory.

Produkcja: `tools/media/desert_fountains.py --source <nowy-katalog-w-sprites>
--animation <nowy-katalog-w-animations> --no-audio` (jedno polecenie).
Master grafiki: `C:\AI2D\sprites\emberfall_fountains_20261026`.
Master animacji: `C:\AI2D\animations\emberfall_fountains_20261026`.
Do paczki gracza trafiają tylko PNG w assets/sprites; KRA/SIF pozostają źródłami.

Test silnika i interfejsu: `tests/guardian_qa.ps1 -NoAudio` obejmuje
`tests/fountains_trade_qa.hpp`. Niezależna kontrola dekoduje obydwa KRA,
wszystkie 96 renderów Synfig i porównuje każdą klatkę z atlasem, mierzy
obecność kolorowej wody i jej zniknięcie, dekoduje 96 ujęć z gry.
`tools/media/verify_guardian.py --qa <folder> --previews --no-audio`
obejmuje również `verify_fountains.py`. Używane są prywatne katalogi zapisów;
nie dotykamy zapisów i ustawień dźwięku gracza.

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

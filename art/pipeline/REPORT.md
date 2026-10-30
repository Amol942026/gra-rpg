# Raport konfiguracji — 19.10.2026

**Aktualizacja 29.10.2026 — przepływ lawy i detal Kaldery:** 8 nowych
natywnych KRA, 15 SIF, dodatkowe 16 grafik dekoracji oraz detale wszystkich
sześciu gatunków i Varkhora w ośmiu kierunkach. Lawa faktycznie przesuwa się
o pełny kafel w zamkniętej pętli; renderer interpoluje i grupuje gotowe próbki.
Format zapisu nadal v10, rewizja scenografii 3; prawdziwy fixture v10 dowodzi
zachowania pozostałych pól. PASS: 194309 kontroli logiki/animacji, 4090
zapisów, 1102 źródeł/eksportów, 624 renderera i 257 DEV. Oba końcowe ZIP-y
wypakowano i uruchomiono w ukryciu bez audio, sprawdzono po 137 zasobów
i niezmienione preferencje dźwięku. Źródła, podgląd animacji i pełne wyniki:
`../volcano/river-detail/README.md` oraz `../volcano/river-detail/QA.json`.
Krita jest używana z zachowaniem zapisanych własnych dokumentów: po wykrytej
awarii zamykania KRA wznowienie sprawdza SHA i nie ponawia gotowych edycji.

**Aktualizacja 29.10.2026 — dopracowanie Kaldery:** bieżąca wersja zastępuje
poniższe pierwsze grafiki tej lokacji. 16 natywnych masterów Krita i 16
projektów Synfig, 16 wariantów dekoracji, siedem zestawów postaci w ośmiu
kierunkach, ciągłe podłoże/lawa i skupiska otoczenia z wolnymi szlakami.
Źródła, pochodzenie, migracja v9→v10 i wyniki: `../volcano/polish/README.md`
oraz `../volcano/polish/QA.json`. PASS: 194277 kontroli logiki/animacji,
4086 zapisów, 1200 źródeł/eksportów, 548 renderera i 257 DEV. Oba końcowe
ZIP-y wypakowano i uruchomiono w ukryciu bez audio; zweryfikowano 135
zasobów każdej paczki i niezmienione preferencje audio. Ukryty ukończony
render: średnio 6,30 ms / P95 8,32 ms; prezentacja obrazu nie była mierzona.

**Aktualizacja 29.10.2026 — Kaldera Popiołu:** trzecia grywalna kraina,
sześć nowych gatunków i Varkhor, Serce Kaldery. 11 warstwowych źródeł Krita,
14 klipów Synfig / 840 klatek oraz 12 klatek lawy. Grafika jest wycinanką
2D; bez nowego modelowania 3D i usług zewnętrznych. Dwie skrzynie i dwie
osobne jednorazowe kapliczki, podróż od Eldrina po zabiciu drugiego bossa,
powrót do wieży i trzeci fragment. Zapis v9 odczytuje v1–v8. 192804 kontrole
logiki/animacji i 4078 zapisów — PASS; 1133 kontroli źródeł i 515 integracji
renderera. Testy pozostają ukryte i ciche; ukończony render nie dowodzi
FPS wyświetlania. [Źródła, procedury i końcowe raporty](../volcano/README.md).

**Dalsza optymalizacja 28.10.2026:** bezpośrednie pakowanie wierzchołków,
wspólne próbki wiatru korony/cienia/obrysu, cache parametrów shaderów
i wstępna alokacja cieni. W ukrytym pomiarze A/B wypakowanej paczki średni
czas renderowania sześciu scen spadł o 13,03%. 813 kontroli renderera,
logika, zapisy oraz ZIP — PASS. Ten pomiar nie potwierdza FPS na ekranie.
[Aktualny raport i paczka](../performance/20261028/round2/README.md).

**Aktualizacja 28.10.2026 — optymalizacja wydajności:** bufory GPU modeli
i obrysów, ponowne używanie geometrii i pamięci, cache portretu, odrzucanie
zasłoniętego tła, usunięcie zbędnej głębi statycznych map oraz wspólne obliczenia
wiatru koron i cieni. Pustynia we wcześniejszych pomiarach ekranu: 46,17 →
58,45–59,91 FPS. Końcowe testy, zgodnie z nowym poleceniem, są ukryte i ciche;
ich czasy renderowania nie dowodzą stałych FPS wyświetlania. HUD pozostaje
natywny, świat otwarty przy 1080p korzysta z mniejszego bufora.
[Raport, porównania i ograniczenia](../performance/20261028/README.md).

**Aktualizacja 28.10.2026 — schody wewnątrz mapy i pełna kolizja:**
oba biegi na wszystkich piętrach przesunięto w głąb sali; wszystkie piksele
grafiki i 4320 kontrolowanych wierzchołków mieszczą się w obrysie podłogi.
Kolizja obejmuje stopnie, szyb, spoczniki i balustrady wraz z marginesem stóp.
Ruch w wieży ma podkroki zapobiegające przeskakiwaniu kolizji. Przejścia
prowadzą na wolne podejścia, a wczytywanie bezpiecznie naprawia pozycję
starszego zapisu wewnątrz przesuniętej konstrukcji. Bez zmiany formatu v7.
110 661 kontroli logiki/animacji, 3403 zapisów, 17 920 renderera i mechanik
oraz 46 niezależnych — PASS. 8100 klatek ruchu i 23 widoki bez dźwięku.
Nie dodano tekstur ani geometrii rysowanej w pętli gry. Pomiar GPU w tym
przebiegu przekraczał 16,67 ms; pełnych 60 FPS nie potwierdzono.
[Źródła, testy i wyniki](../scenery/tower-stairs-fit/README.md).
Końcowy ZIP również sprawdzony: 115 zgodnych zasobów, 180 klatek startu,
ponowione 17 920 + 46 kontroli — PASS. Audio zamknięte, preferencje zachowane,
brak DEV i źródeł natywnych. Czasy GPU zapisano bez deklaracji 60 FPS.

**Aktualizacja 27.10.2026 — balkon i panorama Doliny Świtu:** na piątym
piętrze obserwatorium dodano otwarty taras z fizyczną balustradą. Chód i bieg
prowadzą z sali na balkon i z powrotem bez teleportu. Trzy oryginalne,
warstwowe grafiki Krity przedstawiają panoramę gór, lasów i rzeki oraz taras
z balustradami i bluszczem. Minimapę dopasowano do nowego obrysu.
1337 kontroli ruchu, zapisu, warstw i dotychczasowych pięter — PASS;
97 niezależnych kontroli KRA, PNG, 12 widoków i minimapy — PASS.
P95 renderowania z ukończeniem GPU: 11,60 ms w obserwatorium i 11,77 ms
na balkonie, poniżej 16,67 ms. To pomiar renderowania, bez prezentacji FPS.
Wszystko bez dźwięku. [Źródła i procedury](../scenery/tower-balcony/README.md).
Pełna regresja: 110 661 kontroli logiki/animacji i 3403 zapisów — PASS.
Końcowy ZIP: 115 zgodnych zasobów, 180 klatek startu; po rozpakowaniu
ponownie 1337 + 97 kontroli — PASS. P95 renderowania paczki 13,37 / 13,75 ms.
Brak audio, DEV, źródeł natywnych i zmiany preferencji gracza.

**Aktualizacja 27.10.2026 — dopasowanie pochwy i przydział atrybutów:**
pochwa ma nowy wlot przy biodrze/pasie oraz krótsze mocowania. Nowe natywne
krzywe Synfig dopasowują chwyt i wsuwanie; 72 klatki oraz dokładny eksport C++
sprawdzone. W P dodano DODAJ 5 i opisy przy kursorze dla wszystkich sześciu
atrybutów, z działaniem, podglądem wartości i efektem pięciu punktów.
Zatwierdzanie, COFNIJ i blokada nauki zdolności pozostają zachowane.
110 661 kontroli logiki, 301 renderera, 233 niezależne — PASS. 485 856 póz
i 201 600 rzeczywistych klatek odwracania gestu: zero przecięć głowy i pancerza.
Sprawdzono 18 podpowiedzi w trzech skalach oraz 32 ujęcia postaci.
Końcowy ZIP również PASS po świeżym rozpakowaniu: 112 zgodnych zasobów,
180 klatek startu, ponowione 301 kontroli renderera i 233 niezależne.
Urządzenie audio zamknięte, preferencje zachowane, brak DEV i źródeł natywnych.
Źródła i procedury: [pochwa i atrybuty](../hero/sheath-fit/README.md).

**Aktualizacja 27.10.2026 — pełny wyprost kolan i dodatkowy łup:** miednica
Strażnika dopasowuje wysokość do nogi podporowej; stałe długości kończyn,
pełny wyprost i kontakt stopy są zachowane przez całą pętlę. Przejście do
zamachu wygasza korektę płynnie. Dodano 36 przedmiotów — po trzy na każdy
gatunek — według niezmiennych szans 60% / 15% / 4% / 1%. Łącznie 75 elementów
wyposażenia. Własne palety i zdobienia powstały w 36 warstwach Krity;
poprzednie materiały i identyfikatory zachowano. Bez nowych tekstur runtime.
110 661 kontroli logiki, 3403 zapisów, 1844 renderera, 856 kontroli ruchu
i 438 materiałów/wyprostu/ekwipunku — PASS. Sprawdzono 552 klatki ruchu,
240 faz wyprostu (minimalny kąt 179,9997°), każdą nową rzecz w świecie,
portrecie, rzeczywistym dropie, podwójnym LPM i podpowiedziach. Bez dźwięku.
Źródła i wyniki: [rozszerzenie łupu](../items/expansion/README.md).
Końcowy ZIP również PASS po świeżym rozpakowaniu: 112 zgodnych zasobów,
180 klatek startu, ponowione 1844 kontrole renderera i 856 + 438 kontroli
niezależnych. Brak audio, DEV i zmian preferencji gracza.

**Aktualizacja 27.10.2026 — chód Kamiennego Strażnika:** cykl jest kalibrowany
do drogi po kolizjach, projekcji i skali bossa, na istniejącej krzywej kontaktu
Synfig. Wykrok 18 i dopasowane kolana; tempo automatycznie odpowiada patrolowi,
pościgowi i zatrzymaniu. Prędkość w świecie i walka zachowane. 108 558 kontroli
logiki, 1553 renderera i 856 niezależnych — PASS. Sprawdzono 552 klatki
i 287 kontaktów stóp; maks. ruch stopy w podparciu 0,000214 px przy skali 2,2.
Wszystko bez dźwięku. [Wynik i procedury](../gameplay/guardian/gait/QA.md).
Końcowy ZIP sprawdzony po rozpakowaniu: 112 zgodnych zasobów, 180 klatek
rzeczywistego startu i powtórzony pełny test chodu — PASS. Preferencje
audio niezmienione, brak DEV i źródeł natywnych w paczce.

**Aktualizacja 26.10.2026 — czytelniejsze czcionki:** cały tekst interfejsu
i dialogów używa Lato Bold 700; nagłówki — prawdziwego Cinzel Bold 700,
lokalnie utrwalonego z dołączonego fontu zmiennego na licencji OFL.
Minimum drobnego tekstu interfejsu wzrosło z 14 do 15. Zachowano natywną
rasteryzację i siatkę pikseli; brak dodatkowych kopii liter lub rozmycia.
241 kontroli renderera, 57 widoków w trzech skalach, 552 istniejące kontrole
UI oraz 4074 niezależne kontrole fontów/zrzutów — PASS. Żaden z tych widoków
nie tworzy atlasu podczas rysowania. P95 renderowania z ukończeniem GPU
w pięciu scenach: 4,08–14,04 ms; poniżej 16,67 ms, bez pomiaru prezentacji.
Źródła, procedury i raport końcowej paczki: [typografia](../typography/README.md).
Świeże rozpakowanie ZIP-a: 112 zgodnych zasobów, 180 klatek startu EXE,
powtórzone 57 widoków i wszystkie kontrole typografii — PASS. Brak audio,
trybu DEV i zmian preferencji. [Wyniki i hashe](../typography/QA.md).

Końcowa paczka 26.10 (okna rozwoju/PD/strzały/szczątki): PASS po rozpakowaniu,
111 zgodnych zasobów i 180 klatek startu. Brak audio, trybu DEV i zmian
ustawień gracza. Wyniki i hashe: [QA.md](../gameplay/dune-fallen/QA.md).


Okna P / U przechowują nieruchomy świat w natywnej rozdzielczości na czas
przeglądania i przydzielania punktów. Bieżąca rozgrywka nadal jest rysowana
bez pośredniego skalowania. Cache jest odświeżany przy otwarciu, zmianie
rozmiaru, lokacji, pozycji lub zegara symulacji; HUD i akcje pozostają bieżące.
P95 renderowania z ukończeniem GPU: statystyki 4,4362 ms, szczątki Wydmy
7,8336 ms, Strażnik 13,0342 ms; wszystkie pięć scen poniżej 16,6667 ms.
To test renderowania, nie pełnej prezentacji FPS w tle.


**Aktualizacja 26.10.2026 — okna rozwoju, szczątki Wydmy, strzały i PD:**
Usunięto przyciski +/− z obu okien; tekstowe DODAJ / COFNIJ są widoczne tylko
przy dostępnej operacji. P: statystyki, U: umiejętności. Umiejętności nadal
wymagają przydziału i zatwierdzenia. Nowe szczątki drugiego bossa namalowano
w Kricie w ośmiu kierunkach, z maską kursora i cieniem z nowych pikseli.
Miron sprzedaje łącznie 20 strzał; pierwsza śmierć Wydmy przywraca zapas do 20.
Zapis v7 przechowuje zapas. Progi PD: 100L + 2L(L−1); starsze zapisy
zachowują poziom, rozdane punkty i procent postępu. Nagrody bestii bez zmian.
107 937 kontroli logiki i 1891 zapisów — PASS. 49 nowych kontroli UI,
22 wcześniejsze statystyk, 90 umiejętności i 48 niezależnych źródeł/obrazów — PASS.
Źródła i procedury: `art/gameplay/dune-fallen/README.md`.


Końcowy pakiet 26.10: wszystkie funkcje sprawdzone też po rozpakowaniu ZIP
(110 zgodnych zasobów, 180 klatek startu, bez audio i zmian preferencji).
Renderowanie z ukończeniem GPU: p95 10,28 ms / 11,36 ms dla fontann,
14,03 ms dla Strażnika; poniżej 16,67 ms. Pełnego FPS prezentacji w tle
nie potwierdzono: OpenGL/DXGI blokowały się w sterowniku AMD. Nie zmieniano
V-sync ani konfiguracji gracza. Szczegóły: `art/gameplay/fountains/QA.json`.


**Aktualizacja 26.10.2026 — tempo Strażnika, handel i fontanny:** zamach 1,5 s,
odnowienie 3,5 s, lot 11 pól/s; powrót kontroli po upadku 1,85 s. Cel rzutu
śledzi aktualną pozycję do wypuszczenia; w locie pozostaje ustalony. Nowe
źródła Krity/Synfiga: dwie jednorazowe fontanny pustyni, pełna mana lub HP,
24 klatki płynącej wody i 24 opróżniania na kolor. Tekstury są preładowane,
zużycie i faza opróżniania zapisane w v6; migracja v1–v5 zachowana. U Mirona
2× LPM kupuje lub sprzedaje jedną sztukę. MAX nadal handluje całym stosem.
104 688 kontroli logiki i 1723 zapisu — PASS.
Sprawdzone w rzeczywistym rendererze: 1163 kontrole, 4561 niezależnych
kontroli źródeł i obrazów, 96 natywnych klatek fontann i 96 ujęć z gry.
Testy bez dźwięku. [Fontanny i źródła](../gameplay/fountains/README.md),
[aktualne zachowanie Strażnika](../gameplay/guardian/QA.md).


**Aktualizacja 26.10.2026 — lot na plecy i spokojne wstawanie:** zastąpiono
poprzedni szybki upadek sekwencją 2,8 s z oderwaniem obu stóp, lądowaniem
plecami, odpoczynkiem twarzą do góry i podnoszeniem przez bok oraz kolano.
W Synfigu zapisano osobne tory kontaktów dłoni/stóp, 169 próbek i 168
rzeczywistych renderów. Kierunek patrzenia jest ustalony przed lotem.
112 655 kontroli logiki, 1552 zapisu, 1157 renderera i 5526 źródeł/obrazów
przeszło. Nowy format v5 przelicza fazę starszych zapisów; rzeczywiste pliki
v1–v4 działają. 39 elementów eq, cienie i kolizje zachowane; bez nowych tekstur.
Master: `C:\AI2D\animations\emberfall_backfall_20261026_v3`.
[Aktualny raport i zbliżenie ruchu](../gameplay/guardian/QA.md). Testy bez dźwięku.

**Aktualizacja 26.10.2026 — przewrócenie bohatera:** odrzut głazu prowadzi
do pełnej sekwencji utraty równowagi, upadku, chwili leżenia, podparcia rąk
i wstawania (1,2 s). Nowy natywny SIF z 72 renderami i 73 próbkami istniejącego
rigu, bez nowych modeli i tekstur. Cień korzysta z tej samej pozy, wszystkie
39 przedmiotów zachowują wygląd. Kolizje i chroniony unik pozostają aktywne.
Zapis v4 zachowuje fazę; rzeczywiste pliki v1–v3 nadal działają. 99 594
kontrole logiki, 1510 zapisu, 968 renderera i 3653 źródeł/obrazów — PASS.
Master: `C:\AI2D\animations\emberfall_knockdown_20261026_v2`.
[Aktualny raport i podgląd](../gameplay/guardian/QA.md). Testy bez dźwięku.

**Aktualizacja 23.10.2026 — odrzut od uderzenia głazu:** dodano radialny
impuls w promieniu 1,6 pola, silniejszy blisko środka, z wyhamowaniem przez
0,24 s. Uwzględnia osłony, drzewa, skrzynie, granice mapy i chronioną fazę
uniku. Pauza zatrzymuje odrzut, format zapisu v3 zachowuje jego stan;
rzeczywiste pliki v1 i v2 nadal się wczytują. 89 517 kontroli logiki,
1382 zapisu, 667 renderera i 2085 źródeł/obrazów — PASS. Weryfikacja
rozpakowanej paczki i podgląd: [aktualny raport](../gameplay/guardian/QA.md).
Nie dodano tekstur, audio ani zależności; testy pozostają bez dźwięku.

**Aktualizacja 23.10.2026 — rozpad głazu po uderzeniu:** sześć oryginalnych
fragmentów Krity z atlasu 288×192 rozlatuje się, obraca, odbija i osiada
zgodnie z animacją Synfig (1,2 s, 72 rendery, 438 próbek). Osobne cienie
i sortowanie, drobny gruz i pył; brak dodatkowych obrażeń. Zapis v2 zachowuje
fazę rozpadu, pauza zamraża ruch. 89 309 kontroli logiki, 1295 zapisu,
662 renderera i 2083 źródeł/obrazów — PASS. Dwie ostatnie grupy ponowiono
na świeżo rozpakowanym ZIP; zgodne hashe 106 zasobów, 180 klatek startu gracza.
Wszystkie uruchomienia bez dźwięku, ustawienia audio zachowane. Źródła:
`C:\AI2D\sprites\emberfall_guardian_fragments_20261023` oraz
`C:\AI2D\animations\emberfall_guardian_shatter_20261023`.
[Raport i podgląd](../gameplay/guardian/QA.md).

**Aktualizacja 23.10.2026 — kierunek zgięcia łokci Strażnika:** poprawiono
wybór strony zgięcia obu ramion w natywnym Synfigu oraz ustawienie barków,
aby wyeliminować odwrócone łokcie i blokowanie stawu przy unoszeniu głazu.
Kontrole obejmują cały gest i powrót, także interpolowane podklatki.
89 279 kontroli logiki i animacji, 1252 zapisu, 473 renderera oraz
867 źródeł i obrazów przeszło; ostatnie dwie grupy także po rozpakowaniu ZIP.
Źródło: `C:\AI2D\animations\emberfall_guardian_elbows_20261023_v2`.
Aktualny raport i podgląd: [QA rzutu](../gameplay/guardian/QA.md).

**Aktualizacja 23.10.2026 — oburęczny rzut dużym głazem:** Strażnik przykuca,
wyrywa skałę z ziemią i odłamkami, unosi ją przed twarzą, potem nad głowę
i rzuca obiema rękami. Nowa grafika Krity 192×192, 168 renderów Synfig,
169 próbek dziesięciu kontrolerów, 2,2 s przygotowania i 0,6 s dokończenia.
Wspólny środkowy punkt głazu i oba boczne chwyty, prześwit przy głowie,
cień oraz pasek zdrowia nad skałą. 87 933 kontrole logiki, 1252 zapisu oraz
425 + 513 kontroli renderera i źródeł — także po świeżym rozpakowaniu ZIP.
Pomiar 900 klatek lasu: 59,49 FPS, bez gwarancji stałych 60 FPS.
Aktualne wyniki i paczka:
[raport oburęcznego rzutu](../gameplay/guardian/QA.md).

**Aktualizacja 23.10.2026 — zachodni start i boss:** nowa wyprawa zaczyna
się na skrajnym zachodzie, ponad 67 pól od Strażnika. Pierwszy boss otrzymał
czytelny rzut kamieniem: grafika Krity, sterowanie ręką z Synfiga, oznaczony
cel, lot po łuku, osłony i unik. Format zapisu 2 zachowuje rzut i pociski,
a format 1 nadal działa. 87 595 testów logiki, 1252 zapisu, 35 prologu oraz
204 + 264 kontroli obrazu/źródeł przeszło; te ostatnie ponowiono po rozpakowaniu
nowego ZIP. [Raport i źródła](../gameplay/guardian/QA.md). Las: 55,03 FPS
w cichym teście 900 klatek; nie jest to potwierdzenie stałych 60 FPS.

**Aktualizacja 23.10.2026 — drzewa leśne:** trzy oryginalne, warstwowe grafiki
Krity (świerk, brzoza, dąb), bufory 2× i istniejący wiatr, cienie oraz maski
kursora. Weryfikacja 206 kontroli renderera i 121 kontroli źródeł/obrazów
przeszła także po rozpakowaniu nowego ZIP. [Źródła i wyniki](../scenery/TREES_QA.md).
Porównanie wydajności lasu: 54,40 FPS przed i 54,31 FPS po; pomiar nie oznacza
stałych 60 FPS. Wszystkie testy były bez dźwięku.

**Aktualizacja22.10.2026 — leśne menu i enty:** dodano własną warstwową
ilustrację Krity i12s animacji Synfig, zsynchronizowano chód entów oraz
zmniejszono liczbę grzybów do48. Rzeczywiste testy i nowy rozpakowany ZIP
przeszły kontrolę: [FOREST_QA.md](../menu/FOREST_QA.md).

**Aktualizacja 21.10.2026 — bestiariusz:** do gry i ZIP-a gracza podłączono
poprawione grafiki i ruch wszystkich 12 gatunków lasu i pustyni, z użyciem
Krity i Synfiga. [Aktualne wyniki testów i FPS](../creatures/QA.md),
[źródła i procedury](../creatures/README.md). Poniższa część opisuje
historyczny test konfiguracji środowiska z 19.10.

**Proces produkcji i importu działa.** Wszystkie sześć narzędzi sprawdzono
rzeczywistymi operacjami. Audacity ma opisane poniżej obejście awarii zamykania.
Gra nie była przebudowywana ani uruchamiana. Nie odtwarzano dźwięku, nie użyto
płatnych usług i nie wysłano projektu na serwery.

## Narzędzia

| Program | Wersja | Rzeczywiście sprawdzone |
| --- | --- | --- |
| Krita | 5.3.4.1 | istniejący MCP 0.2.0, state/preview, dokument, natywne rysowanie, warstwy, eksport KRA/PNG |
| Synfig | 1.4.5 | edytowalny SIF z ilustracją z Krity, interpolacja, 60 różnych PNG |
| Laigter | 1.14.0 | cztery mapy z nowego sprite'a, kontrola kanałów i wektorów |
| LMMS | 1.2.2 | nowa kompozycja MMP na bazie szablon.mmp, pięć instrumentów, render WAV |
| Csound | 6.18 | trzy nowe CSD i WAV: electric, weapon_hit, loot |
| Audacity | 3.7.9.0 | import, trim, efekty, normalizacja, cztery eksporty WAV i cztery AUP3 |

Krita używa już zainstalowanego, przypiętego serwera przez prawdziwe STDIO MCP.
Nie instalowano nowego MCP. Bridge działa tylko lokalnie; sekret pozostaje
w prywatnej konfiguracji i nie jest logowany. Pozostałe programy korzystają
z CLI, natywnych formatów i skryptów Python/PowerShell. Dostępne biblioteki
i narzędzia wystarczyły; nie instalowano dodatkowych usług.

FFmpeg z Krity dostarcza loudnorm; FFmpeg Synfiga ma libvorbis. Wykonano
rzeczywisty awaryjny render FFmpeg/Python z trim/gain/mix/Echo/fade/normalize.

## Pliki testowe i parametry

Zaimportowany pakiet:
[assets/media/media_20261019_175901_828feb](../../assets/media/media_20261019_175901_828feb).
Dziesięć plików runtime ma sprawdzone sumy i receipt z pochodzeniem.

- [Sprite](../../assets/media/media_20261019_175901_828feb/sprites/crystal.png):
  256×256 RGBA, przezroczyste tło. Master KRA ma sześć narysowanych warstw
  i przezroczystą bazę: C:\AI2D\sprites\media_20261019_175901_828feb.kra.
- Mapy normal/specular/occlusion/parallax: 256×256. Niezależna kontrola obejmuje
  wszystkie piksele i wektory normalnych. Jest także oświetlony wariant PNG.
- [Atlas](../../assets/media/media_20261019_175901_828feb/sprites/crystal_atlas.png):
  60 różnych klatek128×128, 60 Hz, pętla1s, atlas1056×1056,
  4 460 544 bajty RGBA8 (ok. 4,25 MiB). Stała sylwetka i kotwica, bez obcięcia.
  Obejrzano sprite i przekrój klatek animacji.
- [Muzyka OGG](../../assets/media/media_20261019_175901_828feb/audio/crystal_sanctum_loop.ogg)
  i master WAV: 10,000s, stereo48k, 480 000 ramek także po dekodowaniu OGG,
  -18,00 LUFS / -6,97 dBTP. Cztery takty D-moll przy96BPM; harfa, flet, pad,
  bas i bęben. Render trzech cykli, wycięcie środkowego i kontrola szwu pętli.
- [Zaklęcie](../../assets/media/media_20261019_175901_828feb/audio/spell.wav):2,4s;
  [uderzenie](../../assets/media/media_20261019_175901_828feb/audio/swing.wav):1,6s;
  [zdobycie przedmiotu](../../assets/media/media_20261019_175901_828feb/audio/pickup.wav):2,8s.
  Stereo48k PCM16, szczyt ok. -2dBFS, zero przesterowanych próbek.

KRA/SIF/MMP/CSD, partytury i parametry pozostają w C:\AI2D. Nie użyto
zewnętrznych sampli ani obrazów. Pochodzenie identyfikuje własne natywne
rysowanie i syntezę oraz dostarczony przez użytkownika szablon MMP.

## Wyniki testów

- [Pełny raport](C:/AI2D/pipeline/runs/media_20261019_175901_828feb/report.json): PASS.
  Po produkcji wznowiono etap QA po poprawieniu ścieżki assemblera;
  wykorzystano zapisane zasoby bez ponownego nakładania efektów.
- [15 niezależnych kontroli](C:/AI2D/pipeline/acceptance_c6301d9576/acceptance-report.json): PASS.
  Integracje, hashe, alfa, mapy, wszystkie klatki, pamięć atlasu, WAV/OGG,
  cztery AUP3, blokada Close, odmowa nadpisania/złego hasha/wyjścia poza katalog/
  nieznanego Cue, fallback i odmowa restore po zmianie docelowego pliku.
- [Cztery testy zachowania zapisanych projektów](C:/AI2D/pipeline/close_recovery_412b03e1a6/report.json): PASS.
  Zapisane projekty pozostają otwarte, istniejące okna są zachowane.
- Osobny helper z raylib5.5: **3PNG i 5plików audio — PASS**.
  LoadImage/LoadWave bez urządzenia audio i bez okna gry.
- Podpięcie music/spell/swing/pickup, backup i przywrócenie czterech plików
  w izolowanej kopii assets: PASS. Produkcyjne audio i EXE zachowały hashe
  snapshotu etapu integracji. Nie wykonywano kompilacji gry.

## Ograniczenia i sposób dalszej pracy

**Audacity 3.7.9:** zamykanie części przyciętych projektów potrafi zakończyć
cały program. Odtworzono błąd; zmiana pipe Close na zwykłe zamknięcie okna
nie rozwiązała wszystkich przypadków. Produkcja zapisuje AUP3/WAV i wywołuje
release_saved_project(), pozostawiając zapisane okno. Native Close jest
zablokowane. Po ośmiu takich projektach wspólny proces automatycznie używa
FFmpeg/Python dla obsługiwanych receptur WAV. Ten wariant zapisuje recepturę,
nie pozorny AUP3. AUP3 i inne efekty wymagają dostępnego budżetu Audacity.

**Silnik gry:** obecne postacie są proceduralne. Pakiet jest zaimportowany
i zgodny z dekoderami, a runtime_atlas.hpp dostarcza adapter renderowania.
Podłączenie konkretnej nowej postaci wymaga użycia adaptera w jej rendererze
podczas zadania dotyczącego tej grafiki. Test środowiska nie zmienia rozgrywki.
Mapy nie mają obecnie shadera runtime; oświetlenie wypieka się do PNG,
a parallax pozostaje w źródłach.

**Ocena:** test techniczny nie zastępuje odsłuchu i dopracowania artystycznego.
Eksport60Hz nie jest pomiarem FPS całej gry. Przykład sprawdza narzędzia,
nie stanowi finalnego zestawu artystycznego całej gry.

Przy kolejnych poleceniach agent dobiera potrzebne etapy, przygotowuje master,
animację lub kompozycję, eksportuje, sprawdza i podłącza zasób do wskazanej
mechaniki z backupem. Konfiguracji i ręcznej obsługi sześciu aplikacji nie
trzeba powtarzać. Nie publikowano ani nie commitowano zmian.

[Instrukcja](ASSET_PIPELINE.md), [styl i budżety](STYLE.md),
[AGENTS.md](../../AGENTS.md), [wejście PowerShell](../../tools/media/Run-MediaPipeline.ps1).
Lista skryptów, hashe i aktualne raporty: latest.json w tym katalogu.

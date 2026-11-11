# Broń bliżej ciała i bardziej pionowo — 11.11.2026

Zmniejszono boczne odchylenie ostrzy i łuków do 6 stopni względem tułowia.
Chwyt jest bliżej ciała: minimalna odległość boczna dłoni wynosi 24 jednostki
zamiast 28 dla broni białej i 31 dla łuków. Ręka jest bardziej zgięta,
uniesiona i wysunięta przed żebra. Korekta omijania rękawa zmienia teraz
pochylenie ostrza do przodu, zachowując mały kąt boczny. Jelec nadal omija
przedramię; chowanie i płynne przejście do ataku pozostają zachowane.

Wyniki w `C:\AI2D\pipeline\runs\weapon-upright-20261110` (praca zakończona
11.11): wszystkie 32 bronie przechodzą kontrolę odległości przy 240 fazach,
trzech wagach ruchu i pięciu mieszankach chód/bieg. Dodatkowa regresja
ogranicza boczny kąt do 6,1 stopnia. Logika i animacje: **476 734 kontrole
PASS**. Pełna kontrola głowy/tułowia przy chowaniu i rzeczywistym wejściu
również przechodzi. Źródła sprzed korekty zachowano w `backup`.

`render/render-verification.json`: **138 363 kontrole PASS**, 32 bronie,
1384 pozycje w ośmiu kierunkach, ataki, chowanie i szerokie naramienniki.
Wszystkie obrazy zdekodowano; podgląd jest w
`render/weapon-upright-preview.gif`. Pełna procedura
`tests/weapon_carry_qa.ps1 -NoAudio` została wykonana, z ukrytym oknem
i bez otwierania urządzenia audio.
Zaktualizowano oba EXE oraz obie paczki ZIP. Każda po rozpakowaniu przeszła
180 klatek ukrytego uruchomienia i kontrolę zgodności EXE oraz 287 zasobów.
Zapisane ustawienia dźwięku i dziennik użytkownika pozostały niezmienione.

# Korekta przenikania broni przez ramię — 10.11.2026

Większy wymach ujawnił kolizję pionowo trzymanego miecza z ramieniem
i przedramieniem. Poprzednia kontrola chowania broni obejmowała głowę
oraz napierśnik, lecz nie te części ciała. Nowe testy odtwarzają błąd
na zachowanej kopii kodu, a po korekcie przechodzą dla całego katalogu:
32 miecze, topory, sztylety i łuki, w chodzie, spoczynku i pośrednich
stanach chód/bieg.

`src/hero_carry.hpp` ustala chwyt na zewnątrz ciała, rozwiązuje łokieć
przy zachowanych długościach obu segmentów ręki i odchyla ostrze od
przedramienia. Jelec jest ustawiany w płaszczyźnie, która omija rękaw.
Łuk ma osobny chwyt, sprawdzony także dla najkrótszego i najdłuższego
wariantu. Korekta płynnie zanika przy wejściu w atak i wraca przy jego
zakończeniu. Gest chowania zachowuje istniejące krzywe Synfiga i kontakt
z pochwą. Główny klip chodu, ruch wolnej ręki i nóg pozostały bez zmian.

Wyniki: `C:\AI2D\pipeline\runs\weapon-clearance-20261110`.

- `baseline.json`: test wykrywa przenikanie w poprzednim kodzie;
  w 7680 sprawdzonych pozycjach chodu po korekcie nie wykrywa przecięć.
- `tests/weapon_carry_tests.hpp`: odległości od odcinków ramion oraz
  powiększonych brył naramienników, głowy i tułowia. Kontakt dłoni z chwytem
  jest zamierzony; kontrola przedramienia pomija samą dłoń i mankiet.
  Sprawdza także długości kończyn i zgodność chwytu na granicach ataku.
- Logika i animacje: **442 174 kontroli PASS**.
- Rzeczywisty renderer: **103 803 kontrole PASS**, 1384 pozycje,
  wszystkie 32 bronie w ośmiu kierunkach, dodatkowe szerokie naramienniki,
  początek/koniec ataku oraz 11 etapów chowania. Brak obcięcia obrazu.
- Niezależny test chowania: 485 856 pozycji i 201 600 klatek rzeczywistego
  wejścia; brak przecięć powiększonej głowy i napierśnika. Błąd kontaktu
  ostrza z wlotem pochwy poniżej 0,0001 jednostki.
- Zdekodowano wszystkie 32 arkusze oraz 60 klatek podglądu:
  `qa/weapon-carry-preview.gif`.
- Oba EXE i obie paczki ZIP zaktualizowane. Po wypakowaniu zgodność EXE
  i wszystkich 287 zasobów oraz po 180 klatek ukrytego uruchomienia,
  bez otwierania urządzenia audio: PASS. Zapisane ustawienia dźwięku
  i dziennik uruchomienia użytkownika pozostały identyczne z początkiem pracy.

Reprodukcja pełnej kontroli broni i renderera w nowym katalogu:
`pwsh -NoProfile -File tests/weapon_carry_qa.ps1 -NoAudio`.
Uruchomienia pozostają ukryte i bez dźwięku. Oryginały EXE, ZIP i zmienianych
źródeł zachowano w katalogu `backup` tego przebiegu.

# Większy wymach ramion — 10.11.2026

W natywnym projekcie Synfig zwiększono naprzemienny ruch obu ramion
przód–tył. Zakres dłoni lewej wynosi teraz 25 jednostek rigu (wcześniej 9,2),
a prawej 21 (wcześniej 2,2). Dłonie unoszą się na końcach łuku, zachowując
stałe długości ramion i przedramion. Zmieniono cztery kanały dłoni; krzywe
nóg, tułowia i głowy oraz osobny klip biegu pozostały bez zmian.

Master: `C:\AI2D\animations\emberfall-walk-arms-20261110\walk.sif`.
Kopia źródłowa w repo: `walk/walk.sif`; dane gry: `src/generated/hero_walk_data.hpp`.
Użyto rzeczywistego silnika Synfig 1.4.5 z przenośnego pakietu
`C:\AI2D\Tools\Synfig-1.4.5-spells\portable\bin`. Są tam zarówno
`synfig.exe` (CLI), jak i `synfigstudio.exe` (edytor). Automatyzacja renderuje
w tle. `synfig_cli.py` wykrywa tę lokalizację; opcjonalne nadpisanie ścieżki
to zmienna `EMBERFALL_SYNFIG_EXE`.

Wyniki w `C:\AI2D\pipeline\runs\walk-arms-20261110\qa`:

- 60 poprawnych, różnych klatek natywnego renderu Synfiga.
- Eksport z zapisanego SIF odtwarza nagłówek C++ bajt w bajt.
  Największa różnica kontrolek po odtworzeniu w C++: 0,000059 jednostki.
- 120 klatek rzeczywistego renderera, osiem kierunków, miecz/tarcza
  oraz postać bez broni. Brak obcięcia obrazu i nieprawidłowych kości.
- Kontakt butów i długości kończyn przechodzą kontrolę w całym cyklu.
  Sprawdzono start, zatrzymanie, zmianę kierunku i przejście do biegu.
- 338 477 kontroli logiki i animacji: PASS, w tym chowanie miecza i walka.
- Zaktualizowane EXE oraz obie paczki ZIP: zgodność 287 zasobów i EXE
  po wypakowaniu; po 180 klatek próby uruchomienia, ukrytej i bez dźwięku.

Podgląd: `qa/walk-preview.gif` w powyższym katalogu przebiegu.
Oryginały źródeł, obu EXE i ZIP zachowano w jego podkatalogu `backup`.
Wszystkie testy były bez dźwięku; okno renderera było ukryte.
Są to kontrole poprawności animacji, nie pomiar płynności okna na pierwszym planie.

# Naturalniejszy chód — 21.10.2026 (raport wcześniejszej wersji)

Nowy główny klip chodu powstał z natywnych krzywych Synfiga. Stopy mają
węższe, równoległe tory. Kolana są rozwiązywane w kierunku przodu nogi,
przy stałych długościach uda i goleni. Wysokość bioder rośnie w pojedynczym
podparciu i maleje przy przejęciu ciężaru przez drugą nogę. Ograniczono
skręcanie klatki piersiowej, utrzymano przeciwbalans i stabilne spojrzenie.
Noga powrotna jest unoszona niżej, z płynną zmianą kierunku stopy.

Kontakt uwzględnia rzeczywiste trzy zaokrąglone części buta. Początek kroku
ma uniesiony czubek, środek płaskie podparcie, a koniec wybicie z palców.
Stały ruch stopy w fazie podparcia jest określony w układzie postaci.
Prędkości gry, sterowanie i odległościowy zegar animacji pozostały zachowane.

Klip działa w dotychczasowym rendererze i systemie wyposażenia. Nie zmieniono
grafiki postaci, tekstur ani źródeł pozostałych bazowych klipów. Starsza
dodatkowa rotacja tułowia i ramion nie jest powtórnie dodawana do nowego
chodu; warstwa peleryny pozostała. Przejście do ataku zaczyna się od tej samej
pozy, bez przywracania starego kołysania w pierwszej klatce.

## Źródła i odtwarzanie

- Synfig: `C:\AI2D\animations\emberfall_walk_20261021_v1` — master SIF,
  części Krity, 60 natywnych renderów i manifest.
- Kopia w repo: `art/hero/walk`.
- Generator/eksporter: `tools/media/walk_synfig.py`.
- Runtime: `src/generated/hero_walk_data.hpp`, 121 póz, 17 istniejących kości,
  57 596 bajtów danych klipu; interpolacja w `src/hero_motion.hpp`.
- Podgląd SIF przedstawia kontrolny rig 2D. Końcowy wygląd wyposażenia i ruchu
  oceniamy w rendererze gry. Nie uruchamiano Blendera ani modelowania 3D.
- Kopia stanu sprzed zmian:
  `dist/qa-walk/backup-20261021-134602`.
- Rzeczywisty render poprzedniej wersji: `dist/qa-walk-before`.

Eksport z zapisanego SIF i ponowny render: instrukcja w `README.md`.
Skrypty testowe: `tests/walk_qa.ps1`, `tests/walk_metrics.cpp` oraz
`tools/media/verify_walk.py`. Wszystkie uruchomienia otrzymują `--no-audio`.

## Sprawdzone wyniki

- 87 214 testów logiki i animacji: PASS, w tym kontakt buta z podłożem,
  stała długość obu segmentów nóg, oś kolan, podparcie przez całą pętlę,
  zamknięcie pętli i zgodne rozpoczęcie ataku.
- Test bohatera w rzeczywistym rendererze: 424/424 PASS.
- Chowanie miecza: 485 856 kombinacji pozy oraz 201 600 klatek wejścia
  przy 30/60/120 Hz, bez przecięć powiększonych brył głowy i napierśnika.
- Natywny Synfig: 60 poprawnych PNG; ponowny eksport SIF daje bajtowo
  identyczny nagłówek C++.
- Zapisane punkty kontrolne SIF i faktyczne stawy z C++: maksymalna różnica
  odczytu CSV poniżej 0,00006 jednostki.
- Osiem kierunków × dziesięć póz oraz 120 klatek z trzech stron. Wizualnie
  porównano przód i profil z poprzednim rendererem.

Pomiar dla pełnego chodu, w jednostkach istniejącego rigu:

| Parametr | Przed | Po |
| --- | --- | --- |
| Najniższa część buta względem poziomu podłoża | -3,36564 | -0,00047 |
| Najwyższa z dolnych powierzchni nogi podporowej | -0,14580 | 0,00390 |
| Maksymalny rozstaw kolan | 14,0469 | 12,8286 |
| Maksymalny skręt klatki piersiowej | 6,997° | 0,5° |

Kontakt jest kontrolowany na bryłach zaokrąglonych części buta i oceniany
również na obrazach. Nie oznacza pełnej symulacji kontaktu z dowolną geometrią
terenu ani zmiany prędkości poruszania postaci.

Raporty i podgląd: `dist/qa-walk/asset-verification.json`, `metrics.json`,
`clearance.json`, `unit-tests.txt`, `render-verification.json` i
`walk-preview.gif`.

## Wydanie gracza i wydajność

Ponownie zbudowany ZIP rozpakowano do
`dist/qa-hero-art/package-20261021-140454/Emberfall`. Zweryfikowano EXE,
wszystkie 95 zasobów oraz 180 klatek uruchomienia wydania gracza bez urządzenia
audio i bez trybu deweloperskiego. Native KRA/SIF i inne mastery nie trafiły
do archiwum. Na rozpakowanej kopii ponownie przeszły:

- renderer bohatera 424/424;
- wyposażenie 364/364, pięć poprawnie zapisanych i zdekodowanych PNG;
- gesty miecza 724/724;
- 120 klatek chodu, osiem kierunków, bez obcięcia i błędów zapisu;
- porównanie stawów z SIF i ponowny eksport nagłówka C++.

SHA-256 ZIP:
`e516108188018eb26f054986f38f4710881f323b23f06956cfeb2f39e3b70302`.
Automatyczne ponowienie po aktualnym `tests/hero_package_qa.ps1`:
`pwsh -NoProfile -File tests/walk_package_qa.ps1`.
Inny master można podać parametrem `-SynfigProject`.

Benchmark po przebudowie, 900 klatek, 1440×900, OpenGL/V-sync 60 Hz:
**59,128 fps**, CPU średnio **8,11639 ms**, CPU p95 **9,3382 ms**,
p99 całej klatki **20,3562 ms**. Jedna klatka przekroczyła 25 ms,
maksimum **28,0035 ms**. Jest to wynik bieżącego testu, nie gwarancja
stałych 60 fps we wszystkich scenach. Przygotowanie klipu nie dodaje
generowania animacji ani odczytu zasobów w pętli gry.

Dowody: `dist/qa-walk/package-verification.json`,
`packaged-render-verification.json`, `packaged-equipment-tests.txt`,
`packaged-stow-tests.txt`, `benchmark.txt` oraz
`dist/qa-hero-art/package-verification.json`.

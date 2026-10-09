# Weryfikacja prototypu

## Wersja 0.23 — koło broni, łuk i strzały

Sprawdzone 9 października 2026 w `C:\Users\Amadeusz\Documents\GitHub\gra-rpg`. Wszystkie uruchomienia gry, testów graficznych i benchmarku miały `--no-audio`. Nie otwierano urządzenia audio ani zmieniano preferencji dźwięku.

- Q lub przycisk BROŃ otwiera ośmiopolowe koło i wstrzymuje grę. Miecz i łuk są przypisane na starcie. LPM wybiera broń, przeciąganie z plecaka lub miejsca broni dodaje skrót, a PPM usuwa sam skrót. Skróty wyszukują faktycznie posiadany rodzaj i poziom rzadkości, więc przełożenie przedmiotu w plecaku ich nie psuje. Porzucona broń nie może zostać skopiowana przez koło. Przełączanie działa przy pełnym plecaku, wymieniając wybraną broń z dotychczasową.
- Łuk naciąga się przez przytrzymanie LPM, a puszczenie wypuszcza jedną strzałę. Start: **20 strzał**, zliczanych ze stosów w plecaku. Naciąg do **1,4 s** daje **2–12 pól zasięgu**, **12–20 pól/s** i **22–34 bazowe obrażenia** przed obroną (miecz: 14). Przerwanie przez menu, zmianę broni, unik, utratę aktywności okna lub puszczenie nad HUD nie zużywa strzały. Brak amunicji uniemożliwia strzelanie.
- Kolizja bada odcinek całego ruchu strzały, aby szybki pocisk nie przeskakiwał celu. Trafia pierwszego żywego wroga i stosuje obronę fizyczną. Przeszkody i granice zatrzymują pocisk. Zabójstwo korzysta z dotychczasowych ciał, losowania łupu, doświadczenia i pauzy na awans.
- Dodano animowany łuk w obu rękach, kołczan na plecach, widoczne strzały zależnie od amunicji, cięciwę i pozę naciągu w ośmiu kierunkach. Celownik, pierścień naciągu, kierunek, zasięg oraz licznik amunicji renderują się przy aktualnym dopasowaniu ekranu. Nowy atlas `items-v23.png` mieści 29 ikon.
- **1058/1058 testów logiki i animacji**. Nowe testy obejmują rzeczywiste przechowywanie broni, pełny plecak, porzucanie i odzyskiwanie, brak duplikacji, amunicję i stosy, naciąg oraz zasięg, anulowanie, odnowienie, brak ataku mieczem z łuku, niezależność ruchu pocisku od częstotliwości aktualizacji, najbliższy cel, przeszkody, granicę mapy, obrażenia fizyczne i awans po zabójstwie.
- **65/65 sprawdzeń łuku i koła**: rzeczywiste funkcje wskazywania i przeciągania, neutralne kliknięcie licznika amunicji i plecaka, anulowanie puszczenia nad HUD, trzy skale interfejsu oraz 24 pozy łuku w ośmiu kierunkach z porównaniem niezależnego obrazu GPU i geometrii kursora. Obejrzano koło, przypisywanie, naciąg, strzałę w locie, pełny arkusz kierunków i pusty kołczan. Raport: `dist/qa-v23/bow-tests.txt`.
- Przeszły także **229 sprawdzeń dotychczasowej grafiki i 29 ikon, 16 grzybów, 41 sterowania, 40 ciał i 82 kursora**. Finalną wersję ponownie sprawdzono w diagnostyce łuku, kursora i sterowania po zmianach w HUD. Pełne `build.ps1 -Test` działa; kopiowanie pomija identyczne zasoby, w tym PNG otwarte w podglądzie.
- Efekt strzału `bow-shot.wav` wytworzono lokalnie, bez odtwarzania: mono, PCM16, **44 100 Hz**, **0,27 s**, szczyt **0,4308**, bez przepełnienia próbek. Generator: `audio/create_bow_sound.py`. Zdarzenie dźwiękowe jest emitowane po skutecznym wypuszczeniu strzały. Rozgrywka użytkownika zachowuje ustawienia głośności.

**900 klatek, pełny ekran 1920 × 1080, monitor 60 Hz:** **60,0542 FPS**, średnio **10,3606 ms CPU**, P99 klatki **17,4037 ms**, maksimum **19,589 ms**, **0 klatek ponad 25 ms**. Scenariusz obejmuje las, kamerę, AI, miecz, przełączenie na łuk, naciąg i strzały, magię, uniki oraz powrót do miecza. Wszystkie 900 prezentacji przeszło przez DXGI / V-sync z interwałem 1, bez zasłonięć i urządzenia audio. Raport: `dist/benchmark-v23-fullscreen.txt`. Wynik dotyczy tego komputera i scenariusza.

Paczka `Emberfall-Windows-x64.zip`: **7 043 194 bajty** (około **7,04 MB**). Wypakowano do osobnego katalogu i potwierdzono SHA256 finalnego EXE oraz wszystkich **78 zasobów**, w tym nowego atlasu i dźwięku łuku. Instrukcja ma wersję 0.23 i opis 20 strzał oraz koła. Wypakowana gra przeszła **65/65 testów łuku i koła oraz 16/16 grzybów** z `--no-audio`. Raport: `dist/package-test-v23.txt`.

## Wersja 0.22 — jadalne grzyby

Sprawdzone 9 października 2026 w docelowym projekcie `C:\Users\Amadeusz\Documents\GitHub\gra-rpg`. Wszystkie uruchomienia odbyły się z `--no-audio`; nie otwierano urządzenia audio i nie zmieniano preferencji dźwięku gracza.

- Każda nowa wyprawa losuje **36 borowików** na przechodnim podłożu lasu, poza ścieżkami, drzewami i startem bohatera. Losowanie roślin ma osobny generator, więc nie zmienia szans łupu z potworów. Dawne dekoracyjne kępki zastąpiono zbieranymi grzybami; zachowano układ pozostałych dekoracji.
- Grzyby wskazuje się i podświetla na mapie. Panel pod kursorem pokazuje nazwę, rodzaj i **10 HP**. LPM przeciąga grzyb z mapy lub panelu W ZASIĘGU do plecaka, bez ataku mieczem. Stosy mieszczą 99 sztuk. Pełny plecak i anulowane lub zbyt dalekie przeciąganie nie usuwają rośliny.
- **PPM na grzybie w plecaku** leczy o 10 HP do aktualnego maksimum i usuwa jedną sztukę. Pełne HP pozostawia jedzenie, ostatnia sztuka czyści slot, a używanie podlega dotychczasowemu odnowieniu 0,65 s. Zwiększone HP z Wytrzymałości jest uwzględniane.
- **1022/1022 sprawdzeń logiki i animacji**, w tym miejsca roślin dla 16 ziaren, powtarzalność oraz zmiana pozycji, zachowanie losowania łupu, zakres zbierania, pełny plecak i stos, brak duplikowania, leczenie, odnowienie i maksimum HP.
- **16/16 nowych sprawdzeń interakcji** poprzez te same funkcje wskazywania i przeciągania co mysz. Raport: `dist/qa-v22/mushroom-tests.txt`. Obejrzano eksporty naturalnego lasu i borowików w plecaku z opisem użycia. Nowy atlas `items-v22.png` ma trzy wiersze dla 27 ikon.
- Przeszły także **227 sprawdzeń grafiki, 25 lasu, 41 sterowania, 40 ciał i 82 wskazywania kursorem**. Wszystkie raporty mają zero błędów. Gra i testy zostały skompilowane C++17 z dotychczasowymi bibliotekami. Przy kopiowaniu zasobów pominięto pliki o identycznej zawartości, aby nie nadpisywać PNG trzymanego przez podgląd; sprawdzono SHA256 całego zestawu zasobów źródłowych i w dystrybucji.

**900 klatek, pełny ekran 1920 × 1080, monitor 60 Hz:** **60,0545 FPS**, średnio **10,0545 ms CPU**, P99 klatki **17,0073 ms**, maksimum **17,4981 ms**, **0 klatek ponad 25 ms**. Wszystkie 900 prezentacji użyło DXGI / V-sync z interwałem 1, bez zasłonięć i otwarcia urządzenia audio. Raport: `dist/benchmark-v22-fullscreen.txt`. Wynik dotyczy tego komputera i scenariusza diagnostycznego.

Paczka `Emberfall-Windows-x64.zip`: **6 994 353 bajty** (około **6,99 MB**). Potwierdzono SHA256 finalnego EXE i wszystkich **76 zasobów** oraz instrukcję 0.22 z opisem borowików. Wypakowana gra przeszła **16/16 testów grzybów** z `--no-audio`. Raporty: `dist/package-test-v22.txt`, `dist/qa-v22/packaged-mushroom-tests.txt`.

## Wersja 0.21 — odświeżenie grafiki

Sprawdzone 9 października 2026. Każde uruchomienie gry, testu graficznego i benchmarku odbyło się z `--no-audio`, bez otwierania urządzenia audio i bez zmiany preferencji gracza.

- Przebudowano obecne sylwetki bohatera i wszystkich ośmiu gatunków przeciwników: obłe bryły, cieniowanie, detale futra, pancerza, drewna, zbroi, tarczy i płaszczy. Ciała używają tych samych nowych brył. Osiem kierunków, chód, bieg, atak i przewrót są zachowane. Górną granicę wskazywania ożywionego drzewa dopasowano do jego korony; parametry walki pozostały bez zmian.
- Wszystkie siedem dekoracji otrzymało nowe tekstury: korony z drobnymi liśćmi, pnie, korzenie, mech, pęknięcia, jagody, grzyby i zdobienia kolumn. Teren ma trawę, drobne kamienie, kwiaty i gałązki oraz mniej wyraźną siatkę. Wszystkie 26 ikon mają dodatkowe detale materiałów i miękkie cienie. Ogień ma iskry i dym.
- Detale terenu i dekoracji są wypiekane do tekstur raz przy starcie. Animowane sylwetki odrzucają tylne powierzchnie zamkniętych brył. Obrysy oraz wskazywanie korzystają z tej samej aktualnej geometrii. Nie wprowadzono odczytu GPU ani blokujących barier do zwykłych klatek rozgrywki.
- **960/960 sprawdzeń logiki i animacji**. **226/226 nowych sprawdzeń grafiki**: obraz GPU wszystkich dziewięciu sylwetek w ośmiu kierunkach, bohater podczas cięcia oraz dwóch faz przewrotu w ośmiu kierunkach, kompletność części, zgodność wnętrza sylwetki i pustych miejsc z geometrią wskazywania, brak ucięcia siedmiu dekoracji i 26 ikon. Obejrzano arkusz wszystkich kierunków i końcowy obraz gry. Raport: `dist/qa-v21/graphics-tests.txt`; obrazy: `dist/qa-v21/all-actors-8-directions.png`, `dist/screenshots/emberfall-v21.png`.
- Przeszły także **25 testów nowego lasu i gatunkowego łupu, 40 ciał, 38 środowiska i F11, 82 kursora, 193 synchronizacji, 11 ruchu oraz 27 prezentacji**. Ciała otwierają się PPM, łup przeciąga się do plecaka, korony poruszają się na wietrze, a mysz odpowiada widocznemu obrazowi. Arkusze zamachu i przewrotu wyeksportowano dotychczasowymi narzędziami QA.

**1800 klatek, pełny ekran 1920 × 1080, monitor 60 Hz:** **60,0547 FPS**, średnio 10,0246 ms CPU, P99 klatki **17,0236 ms**, maksimum po rozgrzewce **17,7079 ms**, **0 klatek ponad 25 ms**. DXGI przyjął wszystkie 1800 prezentacji z interwałem 1, bez zasłoniętych prezentacji; pominięto 1800 natywnych przełączeń OpenGL. Scenariusz zawiera przemieszczanie kamery przez las, AI, walkę, magię i uniki. Raport: `dist/benchmark-v21-long.txt`. Wynik dotyczy tego komputera i scenariusza. Eksport PNG wydłuża klatki diagnostyczne, więc liczniki FPS na arkuszach QA nie są pomiarem płynności.

Grafiki są nadal tworzone lokalnie w kodzie projektu. Nie wymagają nowych zależności ani pobierania zasobów przy uruchamianiu. Aktualne eksporty: `assets/generated/terrain-v21.png`, `characters-v21.png`, `items-v21.png`, `prop-v21-0.png`–`prop-v21-6.png`.

Paczka `Emberfall-Windows-x64.zip`: **6 974 500 bajtów**, około **6,97 MB**. Wypakowano do nowego katalogu, potwierdzono SHA256 finalnego EXE i wszystkich **75 zasobów** oraz instrukcję wersji 0.21. Wypakowana gra przeszła **226/226 sprawdzeń grafiki i 25/25 lasu oraz łupu** z `--no-audio`. Obejrzano rzeczywisty końcowy obraz z wypakowanej kopii: `dist/qa-v21/packaged-forest.png`. Raport: `dist/package-test-v21.txt`.

## Wersja 0.20 — większy las, nowe gatunki i drop rate

Sprawdzone 8 października 2026. Wszystkie uruchomienia gry odbyły się z `--no-audio`, bez otwierania urządzenia dźwiękowego i zmiany zapisanych preferencji gracza.

- Mapa **48 × 48**, pięć nazwanych obszarów i **46 przeciwników z ośmiu gatunków**. Potwierdzono obecność wszystkich gatunków i nowych dekoracji, poprawne miejsca respawnu, rozdzielenie ciał, rozszerzone granice oraz pustą ziemię na starcie. Dodane brzozy i dęby korzystają z istniejącej animacji wiatru oraz masek wskazywania.
- **15 nowych rodzajów przedmiotów**: 12 materiałów i trzy elementy sprzętu. Każdy gatunek ma pięć niezależnych reguł łupu z własną szansą, ilością i rzadkością. **240 000 testowych losowań** (30 000 na gatunek) sprawdziło wszystkie 40 reguł: zaobserwowane szanse mieszczą się w tolerancji pięciu odchyleń standardowych, minimum 0,4 punktu procentowego. Przetestowano też jednoczesny drop kła/sierści, skrajne szanse 0/100%, zgodność ilości i rzadkości oraz niezależność od patrolowania. Normalna wyprawa używa losowego ziarna; testy stałego.
- **960/960 sprawdzeń logiki i animacji**: dotychczasowa walka, awans, magia, uniki, kolizje i ekwipunek oraz nowa mapa, osiem gatunków, dropy, stosy materiałów z resztą pozostawioną w ciele, odrzucanie materiałów jako sprzętu i premie trzech nowych elementów wyposażenia.
- **25/25 nowych sprawdzeń grafiki i interakcji**: pięć rzeczywistych ujęć lasu, osiem kierunków nowych przeciwników, wskazywanie ich kształtu, właściwe HP/statystyki/szanse w HUD, cztery nowe ciała otwierane PPM, przeciąganie materiału do ostatniego pola plecaka, animowane drzewa i szanse kła 75% / sierści 55% w podpowiedziach. Obejrzano rzeczywisty obraz GPU oraz arkusz nowych potworów. Raport i obrazy: `dist/qa-v20/`.
- Przeszły także **82 sprawdzenia kursora, 40 ciał, 64 inspekcji, 13 patrolu, 36 optymalizacji, 11 ruchu, 193 synchronizacji, 38 środowiska i 27 prezentacji**. Wszystkie bez błędów. Potwierdzono kompletne HUD, mapowanie myszy i F11 po powiększeniu mapy. Testy są wykonywane przez funkcje aplikacji i callback wejścia, bez wstrzykiwania fizycznych zdarzeń do Windows.

Pomiary na tym komputerze, **1920 × 1080, pełny ekran, monitor 60 Hz**, kamera przemieszczająca się przez rozbudowany las, AI, walka, magia, przewroty i podświetlanie:

| Pomiar | 900 klatek | 1800 klatek |
|---|---:|---:|
| Średni FPS | 60,0543 | 60,0546 |
| CPU średnio | 9,42337 ms | 9,49879 ms |
| P99 czasu klatki | 17,0545 ms | 17,1333 ms |
| Maksymalny czas klatki po rozgrzewce | 17,4237 ms | 19,0813 ms |
| Klatki powyżej 25 ms | 0 | 0 |
| Prezentacje przyjęte przez DXGI | 900 | 1800 |
| Prezentacje zasłonięte | 0 | 0 |

Raporty: `dist/benchmark-v20-900.txt`, `dist/benchmark-v20-long.txt`. Interwał prezentacji wynosi 1; na każdą klatkę przypada jedno pominięte przełączenie OpenGL. Wyniki dotyczą tego komputera i scenariusza, nie gwarantują tej samej wydajności na każdym sprzęcie. Zapis PNG w testach graficznych wydłuża klatkę, dlatego liczniki na eksportowanych obrazach nie są benchmarkiem.

Pełne szanse i ilości są opisane w `LOOT.md`, dołączonym także do przenośnej paczki. Paczka `Emberfall-Windows-x64.zip` ma **5 320 277 bajtów**, około **5,32 MB**. Wypakowano ją do nowego katalogu i potwierdzono SHA256 EXE, wszystkich **65 zasobów** i dokumentacji łupu oraz aktualność instrukcji 0.20. Wypakowana gra przeszła **25/25 testów lasu**, w tym otwieranie nowych ciał, podpowiedzi drop rate i przeciąganie łupu, bez urządzenia audio. Obejrzano kompletny obraz z tej kopii: `dist/qa-v20/packaged-fullscreen.png`. Wynik kontroli paczki: `dist/package-test-v20.txt`.

## Wersja 0.19 — synchronizowane wyświetlanie klatek

Sprawdzone 5 października 2026. Wszystkie uruchomienia odbyły się z `--no-audio`, bez urządzenia dźwiękowego i bez zmieniania preferencji gracza.

- Widoczne okno korzysta teraz z DXGI flip z dodatnim interwałem `Present`, bez zezwolenia na tearing. Wcześniejszy bufor OpenGL przełączał się z interwałem 0 i oczekiwał na `DwmFlush`; sam równy licznik FPS nie potwierdzał synchronizacji tego przełączenia. Zachowanie interwału opisuje [dokumentacja Microsoft Present](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present).
- Raylib renderuje do powierzchni współdzielonej z DirectX w pamięci GPU. Końcowy przebieg DirectX rysuje z niej całą klatkę do jawnie podpiętego widoku bufora flip. Nie ma kopiowania klatek przez CPU. Natywne przełączenie bufora OpenGL jest pomijane, a `EndDrawing` nadal obsługuje czas i wejście. Gdy Windows zgłasza zasłonięte okno, tylko wtedy używane jest zastępcze oczekiwanie kompozytora. Brak obsługi współdzielenia lub błąd inicjalizacji przywraca V-sync OpenGL.
- Współdzielona powierzchnia jest oddzielona od buforów flip. Próba bezpośredniej rejestracji bufora flip uniemożliwiała `ResizeBuffers` po F11 na tym sterowniku. Przy zmianie rozmiaru zwalniane są widoki i rejestracje, a współdzielenie jest odtwarzane. Początkowy wariant zwykłej kopii do bufora flip dawał niekompletny obraz mimo poprawnych znaczników narożników; zastąpiono go końcowym rysowaniem DirectX. Jakość wygładzania świata pozostała bez zmian.
- **193/193 nowych sprawdzeń**: 32 aktualne klatki podczas biegu/przewrotu, porównanie wszystkich pikseli świata i HUD z obrazem docelowym (z uwzględnieniem kompozycji alfa na czarnym tle, tolerancja 1/255), brak czarnej/płaskiej sceny, cztery narożniki, orientacja, pełny ekran, dwa rozmiary okna i powrót F11, dodatni interwał oraz jedna prezentacja DXGI na jedno pominięte przełączenie OpenGL. Różnice całego obrazu: **0 pikseli ponad tolerancję we wszystkich 32 klatkach**. F12 i diagnostyka odczytują rzeczywisty docelowy bufor DXGI, a nie tylko powierzchnię wejściową.
- Ponownie przeszły **529 testów logiki, 82 kursora, 36 optymalizacji, 11 ruchu, 38 środowiska/F11 i 27 prezentacji**. Razem: **916 sprawdzeń bez błędów**. Bieg/przewrót: **60,0383 klatek/s**, średnia 16,656 ms, P95 17,2698 ms; dystanse i sterowanie zachowane.
- **1800 klatek, pełny ekran 1920 × 1080: 60,0541 klatek/s**, P99 17,4992 ms, maksimum 17,7869 ms, **0 klatek ponad 25 ms** w 1739 zmierzonych po rozgrzewce; CPU średnio 7,35938 ms. **900 klatek w oknie: 59,9856 klatek/s**, P99 17,3921 ms, maksimum 34,0661 ms, jedna klatka ponad 25 ms; CPU średnio 7,82775 ms. Scenariusze zawierają kamerę, AI, walkę, magię, uniki i podświetlanie.

**Ograniczenie pomiarów:** Windows zgłaszał okna diagnostyczne jako zasłonięte (`DXGI_STATUS_OCCLUDED`). Raporty oddzielają takie próby od przyjętych prezentacji: w tych pomiarach liczba przyjętych prezentacji wynosi 0. Wyniki mierzą rytm aktualizacji i koszt buforów przy zastępczym oczekiwaniu kompozytora. Potwierdzono cały obraz w docelowym buforze GPU i dodatni interwał wywołania API; nie potwierdzono fizycznego odświeżenia ani wizualnego zniknięcia falowania podczas gry na aktywnym ekranie.

Raporty: `dist/qa-v19/refresh-tests.txt`, `dist/benchmark-v19-long.txt`, `dist/benchmark-v19-large.txt`. Pozostałe sprawdzenia zachowują katalogi `qa-v14`, `qa-v15`, `qa-v16`, `qa-v17` i `qa-v18`.

Paczka `Emberfall-Windows-x64.zip`: **4 719 077 bajtów**, około 4,72 MB. Wypakowano do nowego katalogu, zweryfikowano SHA256 EXE i 59 zasobów oraz instrukcję wersji 0.19. Wypakowana gra przeszła **193/193 sprawdzeń pełnego obrazu, zmiany trybu i synchronizacji**, z `--no-audio`. Obejrzano kompletny docelowy obraz GPU: `dist/qa-v19/packaged-fullscreen.png`. Raport: `dist/package-test-v19.txt`.

## Wersja 0.18 — optymalizacja renderowania i krótkich przycięć

Sprawdzone 5 października 2026. Każde uruchomienie gry odbyło się z `--no-audio`, bez urządzenia dźwiękowego i bez zmiany preferencji gracza.

- Dodano profilowanie sześciu etapów klatki. Największy koszt miały woda i renderowanie świata; symulacja oraz wskazywanie zajmowały po około 0,02–0,03 ms. Diagnostyka czeka na ukończenie pracy GPU przy granicach etapów, dlatego jej wyniki opisują porównanie kosztów, a nie normalną płynność gry. Zwykła gra nie używa tych barier.
- Bufor świata przy 1080p zmniejszono z **2521 × 2161** do **1890 × 1620**: około **44% mniej pikseli**, nadal z nadpróbkowaniem 1,5× na oś. HUD zachowuje natywną rozdzielczość, a teren filtrację ułamkowych przesunięć. Usunięto MSAA końcowego okna: ten etap rysuje już gotowy obraz z buforów i nie poprawiał wygładzenia geometrii wewnątrz nich.
- Obliczanie fal ograniczono do komórek w widocznym obszarze z marginesem na ruch i szerokość fali. Niewidoczne komórki są odrzucane przed trygonometrią. Krzywe współdzielą wartości sinusa i wierzchołki między segmentami. Rysowanie wielokątów postaci przestało tworzyć tymczasowy wektor dla każdej ściany.
- Czcionki wszystkich typowych ekranów są przygotowane przed aktywnym renderowaniem. Pierwszy atak, podpowiedź, inspekcja i otwarcie menu nie tworzą już nowych atlasów. Historia czasów klatek jest gromadzona tylko w trybie diagnostycznym; wcześniej rosła również przez całą zwykłą rozgrywkę.
- Porównanie tego samego 600-klatkowego scenariusza profilowania: **woda 6,903 → 3,71263 ms (około 46% mniej)**, **świat i postacie 5,09103 → 3,70512 ms (około 27% mniej)**, **prezentacja 2,26406 → 1,60429 ms**. Wyniki dotyczą tego komputera i zawierają synchronizację diagnostyczną GPU. Raporty: `dist/performance-v18-profile-before.txt` i `dist/performance-v18-profile-final.txt`.
- **36/36 nowych sprawdzeń**: zgodność widocznej wody z niezależnym algorytmem wersji 0.17 w 24 ujęciach przy wszystkich brzegach i narożnikach (maksymalna różnica kanału koloru 1/255), dziesięć scenariuszy pierwszego użycia tekstu bez nowych atlasów oraz cicha diagnostyka. Ponownie przeszły **529/529 testów logiki**, **82/82 kursora**, **11/11 płynności ruchu**, **38/38 środowiska i przełączania ekranu**, **27/27 końcowej prezentacji** — razem **723 sprawdzenia bez błędów**. Obejrzano końcowy obraz `dist/qa-v18/optimized-fullscreen.png`; kompletne HUD, tekst, wygładzone postacie i animacje pozostają czytelne. Licznik FPS na tym obrazie pochodzi z diagnostyki; właściwe pomiary są poniżej.
- **900 klatek, pełny ekran 1920 × 1080: 60,0555 FPS**. CPU średnio **4,79724 ms**, P95 5,3042 ms; P99 klatki 17,5279 ms, maksimum 18,0937 ms, **0 klatek ponad 25 ms**. Punkt odniesienia przed zmianą: 59,9848 FPS, CPU średnio 5,11776 ms / P95 5,6083 ms. Raporty: `dist/performance-v18-baseline.txt`, `dist/performance-v18-optimized.txt`.
- **Dłuższy test: 1800 klatek, 59,8813 FPS**, CPU średnio 4,71626 ms; P99 klatki 19,7213 ms, maksimum 36,1782 ms. W 1739 zmierzonych klatkach po rozgrzewce wystąpiło **5 klatek ponad 25 ms**. **Okno: 59,986 FPS**, P99 18,2562 ms i jedna klatka ponad 25 ms. Nie odtworzono dłuższego spadku do około 40 FPS; pomiary nie dowodzą całkowitego usunięcia pojedynczych przycięć. Monitor 60 Hz, synchronizacja DWM, ruch kamery, AI, walka, magia, przewroty i podświetlanie. Raporty: `dist/performance-v18-long.txt`, `dist/benchmark-v18-large.txt`.

Raport nowych testów: `dist/qa-v18/performance-tests.txt`. Pozostałe testy zachowują dotychczasowe katalogi `qa-v14`, `qa-v15`, `qa-v16` i `qa-v17`.

Paczka `Emberfall-Windows-x64.zip`: **4 708 993 bajty**, około 4,71 MB. Sprawdzono SHA256 EXE i 59 zasobów oraz instrukcję wersji 0.18. Wypakowana gra przeszła 180-klatkowy test uruchomienia: domyślny pełny ekran 1920 × 1080, synchronizacja DWM i zamknięte urządzenie audio. Raport: `dist/package-test-v18.txt`; obraz końcowy: `dist/qa-v18/packaged-fullscreen.png`. Ten test zawiera jednorazowy odczyt GPU i zapis PNG (około 200 ms), więc jego średni FPS nie jest pomiarem płynności; właściwe benchmarki podano powyżej.

## Wersja 0.17 — zgodność kursora z obrazem

Sprawdzone 2 października 2026. Wszystkie uruchomienia gry odbyły się z `--no-audio`, bez otwierania urządzenia dźwiękowego i bez zmieniania ustawień gracza.

- Odtworzono błąd w rzeczywistej ścieżce wejścia raylib: współrzędne klienta **1746,658** były zwracane jako **1396,8; 561,327**. Biblioteka zachowywała skalę myszy wyliczoną podczas tworzenia okna, podczas gdy końcowy obraz korzystał już z własnego dopasowania do ekranu. Potwierdzają to [inicjalizacja DPI w raylib 5.5](https://github.com/raysan5/raylib/blob/5.5/src/platforms/rcore_desktop_glfw.c#L1424) i [przeliczenie GetMousePosition](https://github.com/raysan5/raylib/blob/5.5/src/rcore.c#L3128).
- `refreshDisplayLayout()` zeruje dodatkowe przesunięcie i ustawia skalę wejścia biblioteki na 1 przed odczytem. Jedynym przeliczeniem do gry pozostaje odwrotność aktualnego wyświetlania. Po poprawce odczyt wynosi dokładnie **1746,658**; mechanizm działa również po zmianie rozmiaru i F11.
- **82/82 nowych sprawdzeń**: pełny ekran 1920 × 1080, okna 1152 × 720, 1280 × 720 i 1600 × 900, ponowny pełny ekran, odziedziczone skale myszy odpowiadające 100/125/150/200%. Obejmują wszystkie 25 pól plecaka i sześć slotów sprzętu, przeciąganie z prawego dolnego pola, potwora i informacje HUD, celowanie, otwieranie ciała PPM, przeciąganie jego łupu do prawej kolumny, teren, przycisk pełnego ekranu i czarne pasy.
- Test podaje współrzędne do rzeczywistego callbacku raylib/GLFW, następnie korzysta z produkcyjnego `GetMousePosition()` oraz funkcji interakcji. Nie przesuwa systemowego kursora i nie wysyła zdarzeń do innych aplikacji. Zmiany skali są symulowane w bibliotece, bez zmiany DPI Windows. Poprzednie testy mapowania pomijały odczyt pozycji z biblioteki, dlatego nie wykryły tego błędu.
- Ponownie przeszły testy: **41/41 sterowania**, **40/40 ciał i łupu**, **38/38 środowiska i trybów ekranu**, **27/27 prezentacji**, **11/11 płynności ruchu**. W tym zadaniu wykonano również **529/529 testów logiki**. Razem **768 sprawdzeń bez błędów** po naprawie. Bieg i przewrót: **60,0518 FPS**, średnia klatka 16,6523 ms, P95 17,4719 ms, monitor 60 Hz i synchronizacja DWM.

Raport odtworzenia: `dist/qa-v17/pointer-before-fix.txt`. Raport naprawy: `dist/qa-v17/pointer-tests.txt`. Pozostałe scenariusze zapisują raporty w dotychczasowych katalogach `qa-v13`, `qa-v14`, `qa-v15` i `qa-v16`.

Paczka `Emberfall-Windows-x64.zip`: **4 703 749 bajtów**, około 4,70 MB. Sprawdzono SHA256 pliku EXE i wszystkich 59 zasobów oraz instrukcję wersji 0.17. Wypakowana gra przeszła **82/82 testów kursora** z parametrem `--no-audio`. Raport: `dist/package-test-v17.txt`.

## Wersja 0.16 — domyślny pełny ekran i synchronizacja ruchu

Sprawdzone 2 października 2026. Wszystkie uruchomienia aplikacji i narzędzi diagnostycznych w tej zmianie używały **`--no-audio`**. Raporty potwierdzają brak otwartego urządzenia dźwiękowego; ustawienia gracza nie były zmieniane.

- Domyślny start na pełnym ekranie bez ramek, z zachowanym dopasowaniem całego HUD. F11 przywraca okno; `--windowed` wybiera start w oknie. Test nowego ruchu sprawdza stan okna bez przekazywania `--fullscreen`, a osobny benchmark potwierdza `--windowed`.
- Prezentacja jest zsynchronizowana z odświeżaniem ekranu. Na tym komputerze bezpośrednie oczekiwanie na V-sync OpenGL obniżało tempo nawet prostej sceny do około 33 FPS. Synchronizacja z kompozytorem Windows po wysłaniu klatki utrzymuje około 60 FPS, bez drugiego ogranicznika programowego. Domyślny pełny ekran i okno używają tej ścieżki; przy braku kompozytora zostaje V-sync OpenGL. Manifest deklaruje obsługiwane wersje Windows, co zapewnia poprawną informację o kompozytorze z `DwmIsCompositionEnabled`. Zachowanie API sprawdzono w dokumentacji [DwmFlush](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmflush) i [DwmIsCompositionEnabled](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmiscompositionenabled).
- Teren i dekoracje są filtrowane przy przesunięciach o ułamki piksela. Świat ma osobny bufor z wygładzaniem krawędzi (około 2× w każdej osi przy 1080p, ograniczony na większych monitorach). HUD i napisy pozostają w rozdzielczości natywnej. Pozę chodu oblicza się z ciągłej fazy animacji, a cienie i ostrzeżenia zachowują dokładne położenia. Geometria wskazywania korzysta z aktualnej pozy, a przeliczenie mysz–świat zachowuje zgodność z kamerą.
- **11/11 nowych sprawdzeń ruchu**: domyślny pełny ekran, wyłączone urządzenie audio, rytm odświeżania, rzeczywisty bieg i przewrót wzdłuż ścieżki, zachowane szybkości i dystans, stabilne czasy prezentacji, bufor wygładzania, ruch terenu o 0,12 piksela oraz płynna zmiana pozy wewnątrz dawnego kroku animacji. Dawne filtrowanie zatrzymywało obraz terenu w 8 z 11 porównywanych par klatek; nowe aktualizuje go przy każdym kroku.
- Test biegu i przewrotu: **60,0526 FPS**, średnio 16,6521 ms na klatkę, P95 17,1247 ms. Obejrzano cały końcowy obraz z wygładzonym światem i ostrym HUD: `dist/qa-v16/fullscreen-motion.png`.
- **Benchmark pełnego ekranu: 900 klatek, 59,9833 FPS**, P95 klatki 17,2595 ms; CPU średnio 4,90912 ms / P95 5,3964 ms. **Okno: 59,9865 FPS**, P95 klatki 17,14 ms; CPU średnio 4,92811 ms / P95 5,4129 ms. Monitor 1920 × 1080 / 60 Hz, AMD Radeon Graphics, zegar prezentacji DWM; urządzenie audio zamknięte. Aktywne ruch kamery, animacje, AI, kolizje, walka, pociski i przewroty. Wyniki dotyczą tego komputera; sterowniki i ekran wpływają na prezentację.
- **529/529 testów logiki**, **13/13 ekwipunku**, **24/24 wskazywania**, **64/64 inspekcji**, **12/12 tłumu**, **21/21 awansu i czaru**, **41/41 sterowania**, **40/40 ciał i łupu**, **13/13 startu i patrolu**, **38/38 środowiska i przełączania ekranu**, **27/27 prezentacji**. Razem z nowymi testami ruchu: **833 sprawdzenia bez błędów**. Testy używają rzeczywistych funkcji gry i renderera, bez wstrzykiwania fizycznych zdarzeń wejściowych.

Raporty: `dist/qa-v16/motion-tests.txt`, `dist/benchmark-v16-fullscreen.txt`, `dist/benchmark-v16-large.txt`, `dist/interaction-test-v16.txt`. Pozostałe scenariusze nadal zapisują się w katalogach `qa-v04`, `qa-v13`, `qa-v14`, `qa-v15`. Diagnostyka porównawcza synchronizacji: `tests/swap_probe.cpp` i `dist/swap-probe.txt`.

Paczka `Emberfall-Windows-x64.zip`: **4 700 167 bajtów**, około 4,70 MB. Zweryfikowano SHA256 pliku EXE i 59 zasobów oraz instrukcję wersji 0.16. Wypakowana gra przeszła 180-klatkowy test z samymi `--smoke --no-audio`: pełny ekran 1920 × 1080 włączył się domyślnie, działał zegar DWM, a urządzenie audio pozostało zamknięte. Obejrzany zrzut całego HUD: `dist/qa-v16/packaged-fullscreen.png`. Raport: `dist/package-test-v16.txt`. Krótki test uruchomienia obejmuje koszt odczytu GPU i zapisu PNG; pomiary płynności bez zapisu obrazów znajdują się w benchmarkach powyżej.

## Wersja 0.15 — pełny ekran bez przycinania i płynne animacje

Sprawdzone 2 października 2026:

- Odtworzono błąd na monitorze **1920 × 1080 przy skali Windows 125%**. Dawna ścieżka prezentacji nakładała na gotowy obraz dodatkową transformację odziedziczoną z `BeginDrawing()`. Zrzut rzeczywistego bufora okna (`dist/qa-v15/legacy-native-fullscreen.png`) pokazuje ucięty prawy HUD i dolny pasek. Poprzednie testy wersji 0.14 weryfikowały bufor gry przed tym etapem i nie wykrywały przycięcia.
- Końcowa prezentacja ustawia jawnie obszar renderowania i projekcję w pikselach bufora ekranu oraz zeruje odziedziczone skalowanie. Przeliczenie pozycji myszy korzysta z osobnego prostokąta w jednostkach okna. Zachowane są proporcje, natywna ostrość tekstu i całe pole widzenia; F12 zapisuje teraz obraz po końcowej prezentacji. Mechanizm skalowania `BeginDrawing()` sprawdzono również w [kodzie raylib 5.5](https://github.com/raysan5/raylib/blob/5.5/src/rcore.c#L798).
- **27/27 nowych sprawdzeń**: cztery narożniki faktycznego bufora ekranu przy pełnym ekranie 1920 × 1080 i oknach 1152 × 720, 1280 × 720, 1600 × 900; transformacje wejściowe odpowiadające DPI 100/125/150/200%; wszystkie 25 pól plecaka w każdym rozmiarze; ciągłość animacji. Testy transformacji nie zmieniają ustawień skalowania Windows. Obejrzano stare i poprawione wyświetlenie, a także wygładzone drzewa i wodę.
- Drzewa używają filtrowania dwuliniowego i dokładniejszej siatki ugięcia, a podmuchy mają łagodniejszy rytm. Fale składają się z gładkich krzywych z przezroczystymi brzegami, bez zaokrąglania położenia do całych pikseli. W próbce 45 kolejnych klatek przy 60 FPS oba efekty zmieniają się w każdej klatce: maksymalny skok pokrycia krawędzi drzewa to 29/255, a zielonego kanału wody 2/255. Wskazywanie drzewa i kontur pozostają zgodne z jego aktualnym kształtem.
- Ponownie przeszło **38/38 testów środowiska i przełączania ekranu**, **529/529 testów logiki**, **24/24 wskazywania**, **64/64 inspekcji**, **41/41 sterowania**, **40/40 ciał i łupu**, **21/21 awansu i czaru**, **12/12 tłumu**, **13/13 startu i patroli**, **13/13 ekwipunku**. Razem z nową diagnostyką: **822 sprawdzenia bez błędów**. Testy używają funkcji gry i rzeczywistych API renderowania/okna; nie wstrzykują fizycznych zdarzeń klawiatury ani myszy.
- **Benchmark pełnego ekranu: 900 klatek, 59,9247 FPS**, P95 klatki 16,8341 ms; CPU średnio 4,68414 ms / P95 5,2701 ms. Monitor 1920 × 1080, bufor 1728 × 1080. **Okno: 59,909 FPS**, P95 klatki 16,8796 ms; CPU średnio 4,78264 ms / P95 5,6163 ms. Wiatr, walka, patrole, kolizje, pociski, uniki, wskazywanie, muzyka i efekty są aktywne. Wyniki dotyczą tego komputera.

Raporty: `dist/qa-v15/presentation-tests.txt`, `dist/benchmark-v15-fullscreen.txt`, `dist/benchmark-v15-large.txt`, `dist/interaction-test-v15.txt`. Obraz końcowy: `dist/qa-v15/presented-mode-0.png`. Pozostała diagnostyka zachowuje dotychczasowe katalogi `qa-v04`, `qa-v13` i `qa-v14`.

Paczka `Emberfall-Windows-x64.zip` ma **4 693 997 bajtów** (około 4,69 MB). Sprawdzone zgodność SHA256 pliku EXE i 59 zasobów oraz instrukcja wersji 0.15. Wypakowana gra wykonała 180 klatek na pełnym ekranie 1920 × 1080 i zapisała faktycznie prezentowany obraz: `dist/qa-v15/packaged-fullscreen.png`. Zrzut obejmuje cały HUD, dolny pasek i pokazuje 60 FPS. Średnia krótkiego testu uruchomienia (55,52 FPS) obejmuje jednorazowy odczyt GPU i zapis PNG; pomiar wydajności bez zapisu obrazów podano wyżej. Raport pakowania: `dist/package-test-v15.txt`.

## Wersja 0.14 — animowany wiatr i pełny ekran

Sprawdzone 2 października 2026:

- Korony drzew są deformowane pod wpływem podmuchów, z różną fazą dla każdego drzewa. Dolna część pnia i cień pozostają nieruchome. Kontur podświetlenia korzysta z tej samej siatki, a wskazywanie odwraca jej aktualną deformację. Porównanie maski z pikselami rzeczywiście narysowanymi przez GPU obejmuje cztery momenty animacji w dwóch skalach; niezgodność najwyżej 2 piksele na 4680 próbek.
- Fale przesuwają się po wodzie pod lądem i poza brzegami mapy. Podmuchy zmieniają ich intensywność; ruch jest powiązany z czasem symulacji, więc pauza go zatrzymuje. Porównanie dwóch klatek wykazało 2965 zmienionych pikseli wody, bez malowania fal na lądzie.
- F11 i stale widoczny przycisk PEŁNY EKRAN / OKNO uruchamiają rzeczywisty tryb bez ramek w natywnej rozdzielczości monitora. Powrót przywraca rozmiar, położenie i poprzednią maksymalizację okna. Menu powitalne, pauza i obowiązkowe rozdawanie statystyk zachowują stan. Przeciąganie przedmiotu lub suwaka jest anulowane przy zmianie trybu. Sprawdzone proporcje obrazu i mapowanie wszystkich 25 pól plecaka.
- **38/38 nowych sprawdzeń animacji, wskazywania i trybu ekranu**, **529/529 sprawdzeń logiki**, **24/24 wskazywania**, **64/64 inspekcji**, **41/41 sterowania**, **40/40 ciał i łupu**, **21/21 awansu i czaru**, **12/12 tłumu**, **13/13 pustego startu i patrolu**, **13/13 ekwipunku** — razem **795**. Diagnostyka używa funkcji aplikacji i rzeczywistych API okna raylib; nie wstrzykuje fizycznych zdarzeń myszy i klawiatury Windows.
- Obejrzane obrazy kołyszących się drzew z dopasowanym konturem i HUD inspekcji oraz interfejs na pełnym ekranie. Eksporty: `dist/qa-v14/wind-trees-1.png`, `fullscreen-game.png`, `fullscreen-welcome.png` i dwie klatki wody.
- **Benchmark pełnego ekranu: 900 klatek, 59,9874 FPS**, P95 klatki 16,7041 ms; CPU średnio 2,22731 ms / P95 2,6279 ms. Monitor 1920 × 1080, bufor 1728 × 1080. **Tryb okna: 59,978 FPS**, P95 klatki 16,7182 ms; CPU średnio 2,26944 ms / P95 2,7191 ms, bufor 1688 × 1055. Oba scenariusze obejmują wiatr, patrole, kolizje, walkę, pociski, uniki, wskazywanie oraz muzykę i efekty. Wyniki dotyczą tego komputera.

Raporty: `dist/qa-v14/environment-display-tests.txt`, `dist/interaction-test-v14.txt`, `dist/benchmark-v14-fullscreen.txt`, `dist/benchmark-v14-large.txt`; ponownie uruchomione raporty pozostałych funkcji są w `dist/qa-v13` oraz `dist/qa-v04/hover-tests.txt`.

Paczka `Emberfall-Windows-x64.zip`: **4 688 969 bajtów** (około 4,69 MB). Wypakowanie do nowego katalogu, zgodność SHA256 pliku EXE i wszystkich 59 zasobów, instrukcja wersji 0.14 z F11 oraz **38/38 sprawdzeń wiatru i pełnego ekranu uruchomionych z wypakowanej gry** zakończyły się pomyślnie. Raport: `dist/package-test-v14.txt`.

## Wersja 0.13 — pusty start i patrole potworów

Sprawdzone 2 października 2026:

- Konstruktor nowej gry nie tworzy żadnych przedmiotów na ziemi. Początkowe wyposażenie i mikstury pozostają w ekwipunku. Komunikat powitalny kieruje do łupu z ciał. Restart również zaczyna się bez łupu na mapie. Testy przeciągania tworzą swoje przedmioty dopiero w scenariuszu diagnostycznym.
- Każdy żywy potwór, który nie widzi bohatera, wybiera punkty do 2,5 pola od swojego miejsca respawnu, chodzi z 60% szybkości pościgu i robi przerwy 0,65–2,05 s. Początkowe przerwy są rozłożone w czasie. Punkty i odcinki ruchu mają prześwit na cały rozmiar potwora; trasy nie przechodzą przez przeszkody ani poza mapę.
- Widoczność wymaga odległości poniżej 6,5 pola i odsłoniętego odcinka do bohatera. Drzewa, skały, krzewy i kolumny zasłaniają widok. Wykrycie przerywa patrol i rozpoczyna pościg. Po utracie widoczności przeciwnik wraca do domu; przeszkody na drodze powrotnej omija po trasie na siatce mapy. Ochrona przed utknięciem wybiera nowy cel po zablokowaniu przez tłum. Przygotowany atak również sprawdza widoczność w momencie trafienia.
- Ruch uruchamia istniejące osiem kierunków animacji i kolizje potworów. Pauza i obowiązkowy awans zatrzymują cele, ruch i czas przerw. Martwe ciała nie patrolują. Sekwencja wyboru trasy jest niezależna od generatora łupu.
- **529/529 sprawdzeń logiki i animacji**, w tym 48 nowych sprawdzeń AI: minuta patrolu każdego gatunku, okolica respawnu, 30/60/120 FPS, blokowanie widoku, powrót każdego gatunku wokół przeszkody, pauza, nieruchome ciała, niezależny łup oraz 30 sekund na pełnej mapie z kontrolą kolizji.
- **13/13 sprawdzeń pustego startu i patroli w aplikacji**, **40/40 ciał i lootu**, **41/41 sterowania**, **64/64 inspekcji**, **21/21 awansu i czaru**, **12/12 tłumu**, **13/13 ekwipunku** — razem z logiką **733**. Diagnostyka używa funkcji aplikacji, bez wstrzykiwania fizycznych zdarzeń myszy i klawiatury Windows. Wskazywanie animowanego potwora sprawdzane jest na faktycznie widocznym fragmencie aktualnej pozy.
- Obejrzane `dist/qa-v13/startup-no-loot.png` oraz podglądy patrolującego szlamu i strażnika z poprawnym HUD inspekcji. Wyeksportowane wszystkie cztery gatunki oraz zwykła mapa po pięciu sekundach symulacji.
- **Benchmark 900 klatek: 59,9675 FPS.** P95 klatki 16,7758 ms; CPU średnio 1,83141 ms i P95 2,2727 ms. Okno 1920 × 1080, bufor 1688 × 1055, patrole, pościg, kolizje, walka, pociski, uniki, podświetlanie oraz muzyka i efekty. Wynik dotyczy tego komputera i scenariusza.

Raporty: `dist/qa-v13/patrol-tests.txt`, `corpse-tests.txt`, `controls-tests.txt`, `inspection-tests.txt`, `progression-tests.txt`, `crowd-tests.txt`, `dist/interaction-test-v13.txt` i `dist/benchmark-v13-large.txt`. Starsze raporty dokumentują poprzednie wersje.

## Wersja 0.12 — ciała, łup w HUD i czar pod C

Sprawdzone 1 października 2026:

- Zabójstwo mieczem lub Kulą ognia zostawia nieruchome ciało na pozycji potwora. Monety, losowy przedmiot oraz dwa legendarne przedmioty strażnika są przechowywane w ciele; śmierć nie tworzy przedmiotów na ziemi. Puste ciała pozostają na mapie i nie blokują ruchu. Ponowne trafienie lub otwarcie nie odtwarza nagród ani XP.
- Cztery statyczne grafiki ciał z istniejących geometrycznych modeli: spłaszczony szlam, poległy wilk, leżący szkielet i rozpadły strażnik. Wskazywanie używa tych samych wielokątów co rysowanie i kolejności obiektów, z uwzględnieniem zasłaniających drzew. Najazd pokazuje nazwę ciała oraz instrukcję otwarcia.
- PPM otwiera pięć pól łupu w prawym HUD, z nazwą potwora i przyciskiem X. Źródło każdego przedmiotu jest powiązane ze stabilnym ID ciała. LPM przeciąga do plecaka; dopiero po zebraniu można przedmiot założyć, użyć lub odłożyć. Inny zajęty slot, pełny stos lub nieprawidłowy cel nie usuwa przedmiotu. Nadmiar częściowego stosu zostaje w ciele. Podświetlenie slotu wskazuje poprawne i niepoprawne cele.
- Zamykanie, ponowne otwieranie i przełączanie ciał zachowuje zawartość. Otwarcie i transfer wymagają odległości do 2,5 pola, a odejście zamyka panel. Pauza, obowiązkowy awans i śmierć blokują zbieranie. Przeciąganie blokuje rzucanie czaru i zmianę ciała.
- C wywołuje pojedynczy wybrany czar z dotychczasowym celowaniem, kosztem 10 many, zasięgiem i odnowieniem. PPM nad żywym potworem nie rzuca czaru. K lub kafelek czaru otwiera księgę; pozostałe sterowanie zachowano. Pasek akcji, pomoc, księga, podpowiedzi i instrukcja ZIP pokazują te przypisania.
- **480/480 sprawdzeń logiki i animacji**, **40/40 ciał oraz lootu w HUD**, **41/41 sterowania**, **64/64 inspekcji**, **21/21 awansu i czaru**, **12/12 ataku obszarowego i tłumu**, **12/12 ekwipunku** — łącznie **670**. Diagnostyka wywołuje funkcje aplikacji, bez wstrzykiwania fizycznych zdarzeń wejściowych Windows.
- Obejrzane ciała czterech gatunków, panel łupu strażnika w skali 0,8, pusty panel i nowa pomoc. Podglądy w trzech skalach: `dist/qa-v12/corpse-panel-80.png`, `corpse-panel-100.png`, `corpse-panel-120.png`.
- **Benchmark 900 klatek: 59,9457 FPS.** P95 klatki 16,7944 ms, CPU średnio 1,82098 ms i P95 2,2683 ms. Okno 1920 × 1080, bufor 1688 × 1055; muzyka, efekty, walka, pociski, ciała, kolizje, uniki i podświetlanie. Wynik dotyczy tego komputera i scenariusza.

Raporty: `dist/qa-v12/corpse-tests.txt`, `controls-tests.txt`, `inspection-tests.txt`, `progression-tests.txt`, `crowd-tests.txt`, `dist/interaction-test-v12.txt` oraz `dist/benchmark-v12-large.txt`. Starsze raporty dokumentują wcześniejsze wersje.

## Wersja 0.11 — Spacja, Shift, LPM i PPM

Sprawdzone 1 października 2026:

- Spacja korzysta z pojedynczego zdarzenia naciśnięcia i uruchamia istniejący przewrót. Oba Shifty sprawdzane są jako przytrzymane i zwiększają prędkość chodu o 50%. Puszczenie klawisza przywraca zwykłą prędkość. Kierunki ukośne są normalizowane; buty zwiększają prędkość zarówno chodu, jak i biegu. Przewrót zachowuje czas, dystans, kolizje i ochronę.
- LPM na mapie wykonuje pojedynczy zamach obszarowy bez wyboru celu. PPM używa wybranego czaru, celując w potwora lub pozycję kursora. R nie rzuca już czaru. Wejście pobiera zdarzenia naciśnięcia myszy, więc trzymanie przycisku nie powtarza ataku.
- Kliknięcia przedmiotów, pustych pól ekwipunku, przycisków HUD, minimapy i menu nie wykonują ataków. LPM zachowuje przeciąganie łupu. PPM na miksturze w panelu ekwipunku lub łupu zachowuje wypicie, a nad mapą rzuca czar. Przeciąganie blokuje oba ataki myszą. Pomoc, pasek akcji, księga czarów i kontekstowe podpowiedzi odpowiadają nowym przypisaniom.
- **427/427 sprawdzeń logiki i animacji**, **41/41 sterowania i routingu kliknięć**, **64/64 inspekcji**, **21/21 awansu i czaru**, **12/12 ataku obszarowego i tłumu**, **12/12 ekwipunku** — łącznie **577**. Testy biegu obejmują 30/60/120 FPS, buty, ruch ukośny, puszczenie Shifta, kolizje oraz niezmieniony przewrót. Diagnostyka używa funkcji aplikacji, bez wstrzykiwania fizycznych zdarzeń wejściowych Windows.
- Obejrzane HUD i pomoc w skali 0,8 oraz nowa księga czarów. Eksporty dla trzech skal są w `dist/qa-v11/`.
- **Benchmark 900 klatek: 59,9481 FPS.** P95 klatki 16,8348 ms; CPU średnio 1,98367 ms i P95 2,468 ms. Okno 1920 × 1080, bufor 1688 × 1055, muzyka, efekty, walka, pociski, uniki i podświetlanie. Wynik dotyczy tego komputera oraz tego scenariusza.

Raporty: `dist/qa-v11/controls-tests.txt`, `inspection-tests.txt`, `progression-tests.txt`, `crowd-tests.txt`, `dist/interaction-test-v11.txt`, `dist/benchmark-v11-large.txt`. Starsze raporty dokumentują poprzednie wersje.

## Wersja 0.10 — awans, punkty statystyk i Kula ognia

Sprawdzone 1 października 2026:

- Każdy awans daje 5 punktów i automatycznie otwiera menu. Symulacja zatrzymuje bohatera, potwory, lecące pociski, regenerację i odnowienia. Podział jest podglądem do momentu zatwierdzenia; nie można wydać za dużo, użyć ujemnych wartości ani ominąć punktów. Kilka awansów w jednym zamachu sumuje punkty.
- Sześć statystyk: Siła +2 obrażenia fizyczne; Moc magiczna +3 magiczne; Obrona i Obrona magiczna +1 odpowiedniego pancerza; Wytrzymałość +10 maks. HP; Mana +5 maks. many. Fizyczna i magiczna obrona oddzielnie zmniejszają obrażenia. Strażnik używa ataku magicznego, oznaczonego w HUD i niebieskim ostrzeżeniem.
- Mana startuje na 100 i regeneruje dokładnie 1 punkt na 5 sekund aktywnej gry. Sprawdzone granice czasu przy 30/60/120 FPS, limit pojemności i brak magazynowania czasu regeneracji przy pełnej manie.
- Księga pod C lub kafelkiem czaru umożliwia wybór Kuli ognia. R celuje w kursora, z uwzględnieniem faktycznej pozycji potwora pod jego grafiką. Pocisk kosztuje 10 many, odnawia się przez 0,8 s, leci do 8 pól i wybucha przy kontakcie. Podstawowe obrażenia magiczne 28 wobec 14 miecza. Kolizje są sprawdzane na całym odcinku ruchu.
- **388/388 sprawdzeń logiki i animacji**, **21/21 awansu, menu, wyboru czaru i celowania**, **60/60 inspekcji HUD**, **12/12 interakcji ekwipunku** — łącznie **481**. Osobny test menu z aktywnym urządzeniem audio potwierdził fanfarę awansu podczas pauzy. Diagnostyka używa funkcji aplikacji, bez fizycznego wstrzykiwania zdarzeń myszy lub klawiatury.
- Obejrzane menu awansu w skali 0,8, księga czarów, pomoc oraz lot i trafienie pociskiem. Eksporty: `dist/qa-v10/`. Animacja przygotowania ataku ma ograniczony zakres, dzięki czemu długie przygotowanie używane w diagnostyce nie deformuje postaci.
- **Benchmark 900 klatek: 59,957 FPS.** P95 klatki 16,7679 ms, CPU średnio 1,93863 ms i P95 2,4159 ms. Okno 1920 × 1080, bufor 1688 × 1055. Muzyka, efekty, walka mieczem i pociskami, kolizje, uniki oraz podświetlanie. Benchmark automatycznie zatwierdza punkty, aby kontynuować pomiar po awansie.

Raporty: `dist/qa-v10/progression-tests.txt`, `dist/qa-v10/inspection-tests.txt`, `dist/interaction-test-v10.txt`, `dist/benchmark-v10-large.txt`. Wynik FPS dotyczy tego komputera i opisanego scenariusza. Starsze raporty pozostają dokumentacją poprzednich wersji.

## Wersja 0.9 — przełączniki MUZYKA i EFEKTY w HUD

Sprawdzone 29 września 2026:

- Dwa niezależne przełączniki stale widoczne obok paska akcji, ze stanami WŁ. / WYŁ. Działają w grze, podczas pauzy i nad oknami pomocy oraz głośności. Przycisk GŁOŚNOŚĆ · F2 zachowuje dostęp do suwaków.
- Wyłączenie muzyki natychmiast zeruje jej głośność, zachowując efekty. Wyłączenie efektów zatrzymuje trwające odgłosy i odrzuca kolejne zdarzenia bez ich późniejszego odtwarzania. Przełączniki zachowują ustawienia suwaków; oba stany są zapisywane razem z głośnością i wyciszeniem M. Stare pliki z trzema polami nadal się wczytują.
- **52/52 sprawdzenia audio i HUD**, w tym pomiar rzeczywistej ciszy w mikserze po wyłączeniu obu kanałów, osobne kanały, zapis ustawień, starszy format i działanie po wspólnym wyciszeniu M. **11/11 sprawdzeń bez urządzenia audio**, **12/12 interakcji ekwipunku**, **323/323 testy logiki i animacji**.
- Obejrzane podglądy przycisków przy szerokości 1152 px oraz podczas pauzy. Eksporty w trzech skalach: `dist/qa-v09/hud-80.png`, `hud-100.png`, `hud-120.png`; dodatkowo `paused-hud.png` i `audio-settings.png`.
- Benchmark z testem interakcji i zapisem zrzutu: 900 klatek, **59,4638 FPS**, P95 klatki **16,7863 ms**. CPU średnio 1,91148 ms, P95 2,1731 ms. Okno 1920 × 1080, bufor 1688 × 1055; muzyka, efekty, walka, uniki i podświetlanie aktywne.

Raporty: `dist/qa-v09/audio-tests.txt`, `dist/qa-v09/audio-disabled-tests.txt`, `dist/interaction-test-v09.txt`, `dist/benchmark-v09-large.txt`. Testy HUD używają wspólnej funkcji obsługi kliknięć aplikacji, bez wstrzykiwania zdarzeń myszy systemu Windows. Diagnostyka nie zapisuje ustawień gracza.

## Wersja 0.8 — atak obszarowy i kolizje przeciwników

- Usunięto trwały wybór celu, kliknięcie wybierające przeciwnika i złoty pierścień zaznaczenia. Najazd kursorem nadal pokazuje obrys i aktualne informacje w HUD. Kliknięcie potwora nie obraca postaci i nie uruchamia zamachu.
- Jedno naciśnięcie spacji trafia raz każdego żywego potwora w promieniu 2,1 pola, w każdym kierunku. Obrona, łup i doświadczenie naliczane są indywidualnie. Obrażenia zamachu są ustalane przed trafieniami, więc awans po zabiciu pierwszego potwora nie zmienia ich dla reszty grupy.
- Kolizje uwzględniają rozmiar całego ciała, w tym pysk i ogon wilka oraz szerszego strażnika. Wspólne rozwiązywanie kontaktów rozdziela tłum, a tłumiony impuls daje lekkie odbicie. Krótkie kroki ruchu zapobiegają przechodzeniu przez siebie. Sprawdzane są także przeszkody i granice mapy.
- **323/323 sprawdzenia logiki**, **12/12 interakcji ekwipunku**, **60/60 inspekcji HUD i akcji myszy**, **12/12 scen ataku i tłumu** — łącznie **407**. Zderzenia czołowe sprawdzone przy 20/30/60/120 FPS, a 12-sekundowe symulacje 14 potworów przy 30/60/120 FPS, w otwartym terenie i przy przeszkodach. Testy obejmują również dokładnie pokrywające się pozycje początkowe i ignorowanie zwłok.
- Obejrzane obrazy zamachu trafiającego sześciu przeciwników oraz tłumu po sześciu sekundach pogoni: `dist/qa-v08/area-swing.png` i `dist/qa-v08/crowd.png`.
- **Benchmark 900 klatek: 59,9384 FPS.** P95 klatki 16,8154 ms, CPU średnio 1,86417 ms i P95 2,3102 ms. Okno 1920 × 1080, bufor 1688 × 1055. Scenariusz zawiera walkę, muzykę, efekty, uniki i wskazywanie.
- Skrypty kompilacji i pakowania działają także w systemowym Windows PowerShell 5.1 (kodowanie UTF-8 z BOM i jawne załadowanie obu zestawów kompresji .NET).

Raporty: `dist/interaction-test-v08.txt`, `dist/qa-v08/inspection-tests.txt`, `dist/qa-v08/crowd-tests.txt`, `dist/benchmark-v08-large.txt`. Testy interakcji wywołują funkcje aplikacji, bez wstrzykiwania fizycznych zdarzeń klawiatury i myszy. Pomiar wydajności dotyczy tego komputera i opisanego scenariusza.

## Wersja 0.7 — miecz wysunięty przed postać

- Wysunięta ręka, obrócone ostrze i płynne cięcie od prawej do lewej w ośmiu kierunkach. Ślad ruchu wynika z kolejnych pozycji ostrza w izometrii; usunięty dawny okrąg przy tułowiu. Wykrywanie kształtu bohatera obejmuje nową pozycję broni.
- Zasięg broni zwiększony z 1,8 do 2,1 jednostki mapy. Pojedyncze naciśnięcie spacji, czas animacji i odnowienie pozostają bez zmian.
- **278/278 sprawdzeń logiki**, **12/12 interakcji ekwipunku**, **24/24 wskazywania**. Dodatkowe przypadki potwierdzają trafienie celu oddalonego o 2,06 i brak trafienia przy 2,2 we wszystkich ośmiu kierunkach.
- Obejrzany arkusz 8 kierunków × 8 faz zamachu oraz scena trafienia wysuniętym mieczem: `dist/qa-v07/`. Arkusz PNG ma większe komórki, aby ostrze nie było obcinane.
- **Benchmark 900 klatek: 59,9262 FPS.** P95 klatki: 16,8333 ms. CPU średnio 1,90911 ms, P95 2,3771 ms. Okno 1920 × 1080, bufor 1688 × 1055; działające muzyka, efekty i walka.

Raporty: `dist/interaction-test-v07.txt`, `dist/qa-v04/hover-tests.txt`, `dist/benchmark-v07-large.txt`. Wynik wydajności dotyczy tego komputera i scenariusza. Testy interakcji korzystają z funkcji aplikacji, bez wstrzykiwania zdarzeń klawiatury i myszy.

## Wersja 0.6 — atak na naciśnięcie i inspekcja HUD

Sprawdzone 28 września 2026:

- Sterowanie bronią korzysta wyłącznie z `IsKeyPressed(KEY_SPACE)`. Usunięto automatyczne atakowanie wybranego celu i atak z kliknięcia potwora lub kafelka miecza. Czar R pozostaje osobną umiejętnością.
- **262/262 sprawdzenia logiki**, **12/12 interakcji ekwipunku**, **24/24 wskazywania**, **60/60 inspekcji HUD i akcji myszy** — łącznie 358.
- Sprawdzono brak ponawiania zamachu po pojedynczym naciśnięciu i po upływie odnowienia, drugi zamach po nowym naciśnięciu, wybór celu bez ataku oraz zgodność obrażeń z prezentowanym atakiem i obroną czterech rodzajów przeciwników.
- Sekcja POD KURSOREM pokazuje bieżące HP i statystyki potworów, nazwę i rodzaj dekoracji oraz informacje o łupie. Wskazanie korzysta z istniejących masek i kolejności zasłaniania. Martwy/usunięty cel oraz kursor nad panelem nie pozostawiają starych danych.
- Obejrzane podglądy strażnika, sosny i pomocy. Sprawdzone wszystkie 25 pól plecaka oraz przeciąganie przedmiotów po zmianie układu prawego panelu.
- **Benchmark 900 klatek z muzyką, efektami, walką i podświetleniem: 59,9531 FPS.** P95 klatki: 16,7987 ms. Przygotowanie na CPU: średnio 1,92361 ms, P95 2,2917 ms. Okno 1920 × 1080, bufor 1688 × 1055.

Raporty: `dist/qa-v06/inspection-tests.txt`, `dist/interaction-test-v06.txt`, `dist/qa-v04/hover-tests.txt`, `dist/benchmark-v06-large.txt`. Podglądy: `dist/qa-v06/`. Testy uruchamiają logikę gry i te same funkcje wyboru celu/akcji myszy co normalne sterowanie; nie wstrzykują fizycznych zdarzeń klawiatury ani myszy. Wynik FPS dotyczy tego komputera i scenariusza.

## Wersja 0.5 — muzyka i efekty dźwiękowe

Sprawdzone 28 września 2026:

- **250/250 sprawdzeń logiki i animacji**, **12/12 interakcji ekwipunku**, **24/24 wskazywania**, **35/35 odtwarzania audio**, **2/2 pracy bez audio** — łącznie 323.
- Muzyka „Echoes of the Ruins”: 101,05 s, stereo 32 kHz Ogg Vorbis, 32 takty przy 76 BPM; plik 818 130 bajtów. 21 efektów WAV mono 32 kHz / 16 bitów. Całość powstała lokalnie metodą syntezy, bez zewnętrznych sampli.
- Sprawdzone zdarzenia: rzeczywiste kroki, atak bez trafienia, cztery rodzaje trafień przeciwników, upadek, śmierć i obrażenia bohatera, udany unik, czar, mikstury, ekwipunek, awans i zwycięstwo. Zablokowane lub nieudane akcje nie odtwarzają dźwięku sukcesu.
- Test urządzenia audio potwierdził załadowanie wszystkich efektów, rzeczywisty niezerowy sygnał w mikserze, zapętlenie muzyki przez granicę strumienia, wyciszanie, wstrzymanie/wznowienie po zmianie aktywności okna oraz zapis/odczyt głośności.
- Przy jednoczesnym odtworzeniu wszystkich efektów z maksymalną głośnością szczyt sygnału wyniósł **0,959925**, poniżej przesterowania cyfrowego. Łagodne ograniczenie szczytów działa na sumie sygnałów.
- Obejrzany panel F2 z suwakami muzyki i efektów, wyciszeniem M i czytelnymi etykietami; `dist/qa-v05/audio-settings.png`.
- **Benchmark 900 klatek z muzyką, efektami, walką, unikami i podświetleniem: 59,9422 FPS.** P95 klatki: 16,8311 ms. Przygotowanie na CPU: średnio 1,98562 ms, P95 2,4816 ms. Okno 1920 × 1080; natywny bufor gry 1688 × 1055.

Raporty: `dist/qa-v05/audio-tests.txt`, `dist/qa-v05/audio-disabled-tests.txt`, `dist/interaction-test-v05.txt`, `dist/qa-v04/hover-tests.txt`, `dist/benchmark-v05-large.txt`, `audio/signal-report.json`. Test dźwięku mierzy własny mikser aplikacji, bez nagrywania mikrofonu ani innych aplikacji. Testy interakcji nie wstrzykują fizycznych zdarzeń urządzeń wejściowych. Pomiar FPS dotyczy tego komputera i scenariusza.

## Wersja 0.4 — szybszy ruch i unik

- Zwiększona prędkość bohatera (3,8; z butami 4,3) i każdego typu przeciwnika (1,85 / 2,6 / 1,85 / 1,35), o około 23%.
- Przeturlanie na oba klawisze Shift oraz przycisk UNIK: 0,4 s, 2,8 jednostki mapy, 1,2 s odnowienia od rozpoczęcia i 0,28 s ochrony przed trafieniem. Animowany pełny przewrót w ośmiu kierunkach, z podkurczeniem nóg, obrotem ciała i śladem pyłu.
- **217/217 sprawdzeń logiki i animacji**, **12/12 sprawdzeń ekwipunku**, **24/24 sprawdzenia wskazywania** — łącznie 253.
- Nowe testy obejmują prędkości wszystkich postaci, osiem kierunków uniku, kierunek w bezruchu, blokowanie zmiany kierunku, odnowienie, brak kosztu many, blokadę ataku/zaklęcia, śmierć, kolizje z czterema typami dekoracji i brzegiem mapy, ruch przy 30/60/120 FPS oraz ochronę dokładnie w chwili uderzenia i jej wygaśnięcie.
- Obejrzany arkusz 8 × 12 klatek przewrotu, interfejs z przyciskiem UNIK, ekran pomocy i postać podczas przewrotu. Eksporty: `dist/qa-v04/`; arkusz z przezroczystością: `assets/generated/v04/hero-roll.png`.
- **Benchmark 900 klatek z walką, unikami, ruchem kamery i podświetleniem: 59,958 FPS.** P95 klatki: 16,8278 ms. Przygotowanie na CPU: średnio 1,88187 ms, P95 2,3572 ms. Okno 1920 × 1080; natywny bufor gry 1688 × 1055.

Raporty: `dist/benchmark-v04-large.txt`, `dist/interaction-test-v04.txt`, `dist/qa-v04/hover-tests.txt`. Testy wejścia korzystają z funkcji aplikacji; nie wstrzykują zdarzeń klawiatury ani myszy przez Windows. Wynik FPS dotyczy tego komputera i scenariusza.

## Wersja 0.3 — WSAD i wskazywanie obiektów

- Ruch przypisany do W/S/A/D; usunięte wiązania ruchu ze strzałkami. Zmienione etykiety w HUD, pomocy i instrukcji.
- **152/152 sprawdzenia logiki i animacji**, **12/12 sprawdzeń ekwipunku**, **24/24 sprawdzenia wskazywania** — łącznie 188.
- Sprawdzone granice pól, ignorowanie panelu/minimapy/pauzy, maski drzew, skał, krzewów i kolumn, przezroczyste marginesy i cienie, kolejność zasłaniania, martwi przeciwnicy oraz zgodność wskazania z przeciąganiem łupu.
- Obejrzane eksporty podświetlonego pola, drzewa, przeciwnika i łupu w `dist/qa-v03/`.
- **Benchmark 900 klatek z ruchem kamery, walką, wskazywaniem i obrysem: 59,9682 FPS.** P95 klatki: 16,7661 ms. Przygotowanie na CPU: średnio 1,85261 ms, P95 2,3069 ms. Okno 1920 × 1080; natywny bufor gry 1688 × 1055.

Raporty: `dist/benchmark-v03-large.txt`, `dist/interaction-test-v03.txt`, `dist/qa-v03/hover-tests.txt`. Testy działają wewnątrz aplikacji; nie wstrzykują klawiszy ani ruchów fizycznej myszy przez Windows.

## Wersja 0.2 — czytelność i osiem kierunków

- Kompilacja zakończona poprawnie. Plik EXE zawiera manifest `PerMonitorV2`.
- **152/152 sprawdzenia logiki i animacji**: osiem kierunków gracza i każdego typu przeciwnika, zachowanie kierunku w bezruchu, obrót do ataku, zatrzymanie animacji przy przeszkodzie, wszystkie klatki chodu i niezależność fazy od FPS.
- **12/12 sprawdzeń interakcji ekwipunku** po zmianie sposobu renderowania, również w większym oknie.
- Obejrzane podglądy interfejsu w 1280 × 800, 1440 × 900 i 1728 × 1080. Czcionki są rasteryzowane do rozmiaru fizycznych pikseli zamiast skalowania całego interfejsu z 1440 × 900.
- Obejrzany arkusz pięciu postaci w ośmiu kierunkach. Automatyczne porównanie zawartości PNG potwierdziło **8 różnych klatek w każdym z 40 kierunkowych cykli chodu**.
- Wyeksportowano 15 arkuszy: 5 postaci × bezczynność/chód/atak, każdy po 8 kierunków × 8 klatek.
- **Benchmark 900 klatek, większe okno: średnio 59,9601 FPS.** Czas klatki P95: 16,8038 ms. Czas przygotowania na CPU: średnio 1,79661 ms, P95 2,2739 ms. Okno zgłosiło 1920 × 1080; natywny obszar renderowania gry po uwzględnieniu dostępnego bufora i proporcji miał 1688 × 1055.

Raporty: `dist/benchmark-v02-large.txt`, `dist/interaction-test-v02.txt`. Podglądy: `dist/qa-v02/`. Arkusze: `assets/generated/v02/` oraz `dist/assets/generated/v02/`.

Testy interakcji uruchamiają wspólne handlery interfejsu; nie symulują fizycznej myszy przez system Windows. Wynik wydajności dotyczy tego komputera i scenariusza testowego.

## Archiwum: wersja 0.1

Sprawdzone 25 września 2026 na komputerze roboczym:

- Kompilacja C++17 / GCC z lokalnego w64devkit 2.0.0, optymalizacja `-O2`, statyczne linkowanie raylib 5.5.
- **30/30 testów logiki** w `tests/game_tests.cpp`.
- **12/12 testów interakcji** przez rzeczywiste funkcje hit-testingu i przeciągania interfejsu. Sprawdzone m.in. mapa → plecak → broń, łup → hełm, tarcza z plecaka, odłożenie na ziemię, mikstura i niedozwolone cele.
- Gra uruchamia się, renderuje mapę, sprzęt i polskie napisy; zrzut obejrzany wizualnie po poprawkach.
- **Benchmark 900 klatek, 1440 × 900, ruch kamery i walka: 59,9889 FPS średnio.** P95 całej klatki: 16,7513 ms. Czas przygotowania klatki na CPU: średnio 1,2184 ms, P95 1,5791 ms.
- Krótki test 180 klatek zapisywał zrzut PNG w trakcie pomiaru i osiągnął około 58 FPS; synchroniczny zapis zrzutu powoduje jednorazowy przestój. Zwykła gra nie wykonuje takich zapisów automatycznie.

Raporty: `dist/benchmark.txt`, `dist/interaction-test.txt`, `dist/smoke-test.txt`. Podgląd: `dist/screenshots/emberfall.png`.

Próba automatyzacji fizycznej myszy przez narzędzie pulpitu zakończyła się przekroczeniem czasu oczekiwania na dostęp. Test interakcji w aplikacji sprawdza wspólne handlery i współrzędne, lecz nie testuje wstrzykiwania zdarzeń przez system Windows. Wynik wydajności dotyczy tego komputera i tej sceny, a nie gwarancji dla każdego sprzętu.




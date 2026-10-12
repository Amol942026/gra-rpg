# Emberfall — Echa zapomnianych ruin

Grywalny prototyp RPG 2.5D w C++17 i raylib 5.5, przygotowany dla Windows x64. Autorska izometryczna grafika, leśne ruiny, walka w czasie rzeczywistym i ekwipunek obsługiwany myszą. Nie wymaga instalowania silnika ani dodatkowych DLL.

### Poprawka czarnego ekranu na Windows 11

Przed wybraniem prezentacji DirectX gra sprawdza rzeczywisty piksel przesłany z OpenGL do docelowego bufora. Niektóre sterowniki Intel UHD przyjmują wywołania współdzielenia GPU, lecz zwracają czarny obraz. Gdy kontrola wykryje tę sytuację, gra automatycznie przechodzi na OpenGL z V-sync. Sprawdzenie jest powtarzane po zmianie rozmiaru bufora; nie wymaga zmiany ustawień Windows ani urządzenia audio.

Uruchamiaj grę zwykłym `Uruchom.bat`. Opcjonalne `Uruchom.bat --opengl` wymusza OpenGL, a `--windowed` zaczyna w oknie. Launcher przekazuje parametry do gry. Etapy uruchamiania i używany sterownik są zapisywane w `dist/emberfall-startup.log` (w paczce ZIP: obok EXE). Log jest odświeżany przy każdym starcie. Cichy test `--startup-smoke --no-audio` sprawdza normalny start prologu i zapisuje `startup-smoke.txt` oraz `screenshots/emberfall-startup.png`. Parametr `--no-audio` nie zapisuje ustawień dźwięku.

## Wersja 0.32 — szczegółowe wyposażenie wszystkich pięter wieży

Dopracowano **wszystkie 13 rodzajów obiektów**: runiczne kolumny, stół Eldrina, kulę skupienia, kominek, źródło odnowy, regały z księgami, otwartą kronikę, kocioł, ogród ziół, kamienie pieczęci, lunetę, astrolabium i Serce Świtu. Mają tę samą perspektywę **40:20** i oświetlenie co kamienne schody. Kolumny i postumenty mają profilowane podstawy, rowki i łączenia; kominek murowany łuk, gzyms, drewno i płomienie. Meble mają drewniane nogi, wsporniki, słoje i mosiężne okucia. Na regałach widać kolorowe oprawy, złocone grzbiety i krawędzie stron; otwarte księgi mają dwie strony, tekst oraz wstążkę.

W alchemii stoły zawierają moździerz, tłuczek, fiolki, notatki i księgę. Metalowy kocioł ma obręcz, uchwyty, nity i trzy nogi, a skrzynie ziół różne liście i kwiaty. Luneta ma trójnóg, segmenty tubusu, pierścienie i szkło; astrolabium mosiężne kręgi z podziałką i orbitujące kule. Kamienne okna mają łuk z osobnych bloków, parapet, szprosy i rombowy wzór szyb.

Płomienie, woda, para, zioła oraz efekty magiczne nadal się poruszają. Modele przygotowuje się raz w **podwójnej rozdzielczości** i przechowuje na GPU. Najazd myszą używa maski rzeczywistej sylwetki, także dla animowanych części; dodaje złoty obrys i właściwe informacje w HUD. Księgi, alchemia, źródło, pieczęcie i luneta zachowują obsługę **E / LPM / PPM**.

Cichy test `--tower-art-qa --no-audio` sprawdza gotowe piksele GPU, brak ucięcia obiektów, zgodność wskazywania myszą z animacją, HUD i pełny ekran. Galerie z bohaterem dla porównania skali: `dist/qa-v32/furniture-detail-1.png` i `furniture-detail-2.png`; widoki pięter w skalach 80%, 100% i 120% oraz raport: `dist/qa-v32/`.

## Wersja 0.31 — pełnowymiarowe kamienne schody w wieży

Wieża ma teraz schody dopasowane do skali bohatera i izometrycznych osi posadzki. **Schody w górę** to wysoki bieg z **14 stopniami**, spocznikiem, murowanymi bokami, słupkami i kamiennymi poręczami. Stopnie mają osobne podstopnice, jasne krawędzie, fugi i ślady zużycia. **Schody w dół** są otwartym szybem w podłodze: stopnie schodzą poniżej posadzki i znikają w cieniu, a otwór otacza kamienna balustrada z wolnym wejściem.

Kolumnę odsunięto, aby nie zasłaniała biegu. Podpisy obu schodów pozostają czytelne. Mysz wskazuje rzeczywistą sylwetkę kamiennej konstrukcji; najazd dodaje obrys i pokazuje cel w HUD. Kolizje chronią przed przechodzeniem przez podwyższony bieg, poręcze i otwarty szyb. Podejdź do wejścia i użyj **E / LPM / PPM**. Po zmianie piętra postać pojawia się przy bezpiecznym wyjściu. Na parterze nie ma zejścia w dół, na ostatnim piętrze nie ma dalszych schodów w górę.

Grafika schodów jest przygotowywana raz w podwójnej rozdzielczości, a później rysowana z pamięci GPU. Test `--stairs-qa --no-audio` sprawdza proporcje, brak uciętych fragmentów, zgodność grafiki ze wskazywaniem myszą i wejście na piętra. Podgląd z bohaterem obok obu konstrukcji: `dist/qa-v31/stairs-detail.png`. Widoki wieży i raport: `dist/qa-v31/`.

## Wersja 0.30 — krytyczne trafienia i Leczenie

Każdy udany atak bohatera ma **5% szansy na trafienie krytyczne**: zwykły zamach mieczem, strzała z łuku, Kula ognia, Mocne cięcie i Przeszywająca strzała. Krytyk zadaje **150% więcej obrażeń**, czyli **250% zwykłych obrażeń tego ataku po uwzględnieniu obrony przeciwnika**. Losowanie odbywa się raz na zamach lub wypuszczony pocisk; wszyscy trafieni obszarowo lub przeszyci przeciwnicy dzielą jego wynik. Naciąganie, anulowanie oraz zablokowana akcja nie losują krytyka. Losowania ataków nie zmieniają generatora losowego łupu.

Zwykłe obrażenia mają większe jasne cyfry i ciemny obrys. Krytyk pokazuje **duże złote cyfry, napis KRYTYK! i ciemne tło** przez dłuższy czas. Wyniki unoszą się ponad sylwetką danego przeciwnika, również dużego Strażnika.

**Leczenie** można poznać za **1 punkt umiejętności** po rozdaniu statystyk podczas awansu. **V** lub zielony kafelek **LECZENIE** w HUD przywraca **30 HP za 20 many**, do maksymalnego zdrowia. Działa z każdą bronią i w wieży. Zielone pierścienie i krzyżyki pokazują efekt, a napis podaje rzeczywiście odzyskane HP. Przy pełnym zdrowiu lub niewystarczającej manie akcja nie pobiera kosztu. Odnowienie: **1 s**. Leczenie ma jedną rangę; kolejne punkty nadal można wydawać na dwie umiejętności broni. W normalnej nowej wyprawie Leczenie trzeba poznać; zestaw deweloperski zawiera wszystkie trzy umiejętności.

Cichy test `--combat-qa --no-audio` sprawdza oba etapy awansu, naukę Leczenia, używanie z mieczem i łukiem oraz w wieży, koszty i wszystkie pięć rodzajów krytycznych ataków. Menu i efekty są renderowane w skalach 80%, 100% i 120%; raporty oraz podglądy: `dist/qa-v30/`. Testy modelu: `tests/critical_healing_tests.hpp`.

## Wersja 0.29 — tryb deweloperski i szybki start w lokacji

Naciśnij **F3** albo kliknij **DEV · F3**. Wybór działa także podczas prologu, scenki powrotu, pauzy, rozdawania punktów i po śmierci. Dostępnych jest **13 miejsc startu**: osiem punktów w Lesie Mchu (w tym okolice bossa) oraz każde z pięciu pięter wieży. Zaznacz miejsce, następnie wybierz:

- **NOWA SESJA TESTOWA** — resetuje wyprawę, postać i przeciwników, pomija prolog i uruchamia wybraną lokację. ENTER robi to samo.
- **PRZENIEŚ OBECNĄ POSTAĆ** — zachowuje plecak, założony sprzęt, poziom, XP, statystyki i istniejące rangi. Pozostawione przedmioty i ciała zostają w swojej lokacji; można wrócić po nie.

Każdy start odnawia HP i manę oraz usuwa stary naciąg, unik, pociski, efekty i otwarte menu łupu. Bohater pojawia się w najbliższym bezpiecznym miejscu, poza przeszkodami i ciałami żywych przeciwników. Wieża jest w sesji testowej odblokowana bez walki z bossem; powrót do lasu z zachowaną postacią zachowuje również postęp zadania. **Nowa sesja przy bossie** pozwala ponownie testować zabicie Strażnika, łup i powrót do Eldrina.

Opcjonalny **ZESTAW TESTOWY** uzupełnia do **100 strzał, po 15 borowików i esencji lasu oraz po 10 mikstur zdrowia i many**; odblokowuje wszystkie trzy umiejętności przynajmniej na randze 1. Nie obniża wyższych rang i nie nadpisuje przedmiotów. Powtórne przenoszenie nie powiększa kompletnego zestawu. Przy pełnym plecaku wyświetla informację o ograniczeniu zestawu. Odznacz opcję, aby testować bez dodatkowych przedmiotów i umiejętności.

**ESC, F3 lub X** zamyka wybór i wraca do poprzedniego stanu bez resetowania wyprawy. Otwarta lista zatrzymuje symulację. Po uruchomieniu testu HUD pokazuje **SESJA TESTOWA**. Po śmierci **ENTER** rozpoczyna ponownie test w ostatnio wybranej lokacji. **NOWA ZWYKŁA GRA — PROLOG** kończy tryb testowy i resetuje wyprawę do normalnego startu. Zwykłe uruchomienie nadal zaczyna się prologiem i nie dostaje zestawu testowego.

W folderze projektu i wypakowanej grze są trzy wygodne launchery:

- `Tryb_deweloperski.bat` — od razu otwiera wybór lokacji.
- `Test_wiezy.bat` — zaczyna na pierwszym piętrze; F3 wybiera kolejne.
- `Test_bossa.bat` — zaczyna przy Kamiennym Strażniku.

Bezpośredni start z parametrami (Windows CMD, folder projektu lub wypakowana paczka):

```bat
Uruchom.bat --start tower-3
Uruchom.bat --start tower-5
Uruchom.bat --start ruins --no-dev-kit
Uruchom.bat --dev
```

Identyfikatory lokacji: `forest`, `ruins` (także `boss`), `birches`, `grove`, `heart`, `trail`, `wolves`, `spiders`, `tower-1` do `tower-5` (`tower` oznacza pierwsze piętro). Obsługiwane jest również `--start=tower-3`. Błędna lub brakująca lokacja otwiera listę z komunikatem. `--no-dev-kit` wyłącza domyślny zestaw testowy. Pozostałe parametry, np. `--windowed`, `--opengl`, `--no-audio`, nadal działają.

`--dev-qa --no-audio` sprawdza rzeczywiste menu i jego interakcje; raport i obrazy trafiają do `dist/qa-v29/`. `--dev-smoke --no-audio` kończy uruchomienie testowe po 180 klatkach, zapisuje faktyczny obraz i stan startu w `dist/dev-startup-v29.txt`. Testy mechaniki i parametrów są w `tests/developer_tests.hpp`.

## Wersja 0.28 — pięć pięter wieży Eldrina

Po dotychczasowym pokonaniu Strażnika i zebraniu części Serca Świtu wracasz na **pierwsze z pięciu pięter**. Wieża jest bezpieczna, a Eldrin nadal rozmawia w sali przybycia. Schody łączą sąsiednie piętra: podejdź na 2,3 pola i użyj **E, LPM lub PPM**. Złote schody prowadzą w górę, niebieskie w dół; HUD i minimapa pokazują aktualne piętro.

1. **Sala przybycia:** kominek z ruchomymi płomieniami, dywan, stół, kryształowe kolumny, Eldrin i źródło odnowy. Źródło przywraca całe HP i manę.
2. **Biblioteka:** regały z różnymi księgami, pulpit i kroniki z trzema rozdziałami o Pieczęci Popiołu, pierwszym strażniku i wieży. Kliknij regał lub otwartą kronikę.
3. **Pracownia alchemiczna:** parujący kocioł, zioła i stoły. **3 borowiki → 1 mikstura zdrowia (+50 HP)**; **3 esencje lasu → 1 mikstura many (+35 many)**. Mikstury trafiają do plecaka. Brak składników lub miejsca nie zużywa materiałów; stosy respektują limit 99 sztuk.
4. **Sala pieczęci:** trzy niezależne kamienie runiczne i magiczne odbicie Serca Świtu. Dotykaj kamieni, aby zapalić ich światło. Gdy świecą wszystkie trzy, promienie stabilizują odbicie. Zdobyty fragment pozostaje w plecaku; projekcja go nie kopiuje ani nie zużywa.
5. **Obserwatorium:** luneta, ruchome astrolabium, gwiazdy w oknach i trzy mapy nieba do przeglądania. Jedna zapowiada poszukiwania kolejnego strażnika.

Księgi, alchemia i mapy nieba otwierają okna z zatrzymaną grą; **ESC, ENTER lub X** zamyka okno. Schody zachowują sprzęt, materiały, statystyki, umiejętności i aktywowane runy. Porzucone przedmioty pozostają na konkretnym piętrze i można je odzyskać po powrocie. Wyposażenie blokuje ruch i unik. Statyczne podłogi i ornamenty są przygotowywane raz jako tekstury GPU; ogień, para, zioła, runy i astrolabium pozostają animowane.

Cichy test `--tower-floors-qa --no-audio` zapisuje pięć pomieszczeń, księgę, receptury, pieczęci i mapę nieba do `dist/qa-v28/`. `--tower-benchmark --no-audio` mierzy 900 klatek z przemieszczaniem kamery przez wszystkie piętra; raport: `dist/tower-benchmark-v28.txt`. Powrót po bossie nadal sprawdza `--tower-qa --no-audio`.

## Wersja 0.27 — umiejętności i drugi etap awansu

Każdy zdobyty poziom daje **5 punktów statystyk oraz 1 punkt umiejętności**. Po zatwierdzeniu statystyk automatycznie otwiera się okno umiejętności. Gra pozostaje zatrzymana do rozdania i zatwierdzenia wszystkich punktów. Kliknij kartę lub przycisk +, a przyciskiem - popraw wybór. Pierwszy punkt odblokowuje umiejętność; kolejne zwiększają jej rangę. Awans o kilka poziomów jednocześnie zachowuje wszystkie należne punkty.

- **Mocne cięcie — Z albo pierwszy nowy kafelek HUD.** Działa wyłącznie z mieczem i kosztuje **10 many**. Ranga 1 zadaje **150% obrażeń zwykłego ataku**; każda kolejna dodaje 10 punktów procentowych tego mnożnika. Trafia przeciwników w dotychczasowym zasięgu miecza i uwzględnia obronę fizyczną. Ma dłuższy, szerszy zamach z pomarańczową smugą i odnowienie **1,2 s**. Łuk, topór i sztylet blokują tę umiejętność.
- **Przeszywająca strzała — X albo drugi nowy kafelek HUD.** Działa wyłącznie z łukiem. X przygotowuje następny strzał; następnie **przytrzymaj i puść LPM**, tak jak przy zwykłym strzale. Skuteczne wypuszczenie kosztuje **20 many i 1 strzałę**. Ranga 1 trafia do **3 kolejnych przeciwników**, każda kolejna zwiększa limit o 1. **Obrażenia, zasięg i prędkość są takie same jak dla zwykłej strzały przy tym samym naciągu.** Każdy cel otrzymuje obrażenia tylko raz. Przeszkody i granice mapy nadal zatrzymują pocisk. Ma błękitną cięciwę, smugę, celownik i pierścienie trafień oraz odnowienie **1,5 s**. Ponowne X wyłącza przygotowanie bez kosztu.

**U** lub przycisk **UMIEJĘTNOŚCI** na prawym HUD otwiera podgląd rang. Nieznane umiejętności i umiejętności niewłaściwe dla broni są przygaszone. Najechanie na kafelek wyjaśnia sterowanie, koszt i blokadę. Menu, zmiana broni, unik i anulowanie strzału nie pobierają kosztu przygotowanej strzały. Koszt jest sprawdzany ponownie przed wypuszczeniem. Oba ataki są fizyczne; zabójstwa zachowują dotychczasowy łup, doświadczenie, awans i fabułę. Walka pozostaje zablokowana w wieży Eldrina.

## Wersja 0.26 — pierwsza część Serca Świtu i wieża Eldrina

**Kamienny Strażnik** w północnych ruinach zostawia **jedną część Serca Świtu ze szansą 100%**. Pozostaje ona w piątym polu łupu obok monet, legendarnego miecza, pierścienia i esencji. Pokonanie bossa samo nie uruchamia sceny. Podejdź do ciała, otwórz je PPM i przeciągnij fragment do wolnego pola plecaka. Warto wcześniej zebrać pozostałe nagrody: podniesienie fragmentu rozpoczyna powrót. Przy zajętym polu, zbyt dużej odległości lub nierozdanych punktach awansu fragment zostaje w ciele.

Po zebraniu części świat zatrzymuje się, **ekran ciemnieje**, a bohater wraca do obserwatorium Eldrina. Ośmiowypowiedziowa scenka pokazuje powrót z niebieskim efektem teleportacji, sukces Wędrowca, pochwałę maga i zapowiedź teleportu do drugiej krainy, gdy bohater będzie gotowy. Enter / Spacja / LPM odsłaniają tekst i przechodzą dalej; Esc pomija rozmowę, zachowując przejście do wieży. Utrata aktywności okna wstrzymuje scenę, F11 nadal działa.

Po rozmowie **wieża maga staje się grywalną lokacją**: kamienna sala z oknami, kolumnami, księgą, magiczną kulą i kręgiem teleportacji. Można chodzić, biegać, robić uniki oraz korzystać z ekwipunku. Ściany, meble i Eldrin blokują ruch; przeciwnicy z lasu pozostają tam i nie symulują się w schronieniu. Poziom, statystyki, HP, mana i zebrane wyposażenie są zachowane. Fragment pozostaje przedmiotem zadania w plecaku i nie można go porzucić ani zużyć. Dziennik pokazuje **1 / 5 części**.

**Eldrin jest przyjaznym NPC.** Podejdź na maksymalnie 2,7 pola i naciśnij **E** lub kliknij maga **LPM / PPM**, aby porozmawiać ponownie. Dialog kończy się powrotem do swobodnego ruchu; nie resetuje postaci ani nie powtarza zdobycia fragmentu. Druga kraina pozostaje zapowiedziana fabularnie — ten etap kończy się pobytem w wieży.

Cichy test `--tower-qa --no-audio` sprawdza faktyczne przeciąganie fragmentu, ściemnienie, dialog powrotu, ruch w wieży, ponowne rozmowy z NPC i zmianę rozmiaru okna. Raport, transkrypcja i obrazy trafiają do `dist/qa-v26/`.

## Wersja 0.25 — większy Las Mchu i luźniejsze starcia

Las ma teraz **72 × 72 pola**, czyli **2,25 raza większy obszar** niż wcześniej. Rozbudowane trakty łączą Ruiny Mchu, Wilczą knieję, Brzozową polanę, Pajęcze ostępy i Serce Kniei z **Zacisznym zagajnikiem**, **Starym traktem** oraz kamiennym **Zapomnianym kręgiem**. Polany mają więcej kwiatów, brzozy skupiają się na zachodzie, sosny we wschodniej kniei, a dęby w głębokim lesie.

Na mapie pozostaje **46 wrogów z ośmiu gatunków**. Ich miejsca startowe dzieli co najmniej **8 pól**, a patrole odbywają się wokół własnych miejsc. W okolicy przesieki, do której teleportuje Eldrin, przeciwnicy nie zaczynają samoczynnie pościgu. Strażnik nadal czeka w północnych ruinach, zgodnie z prologiem. W większym lesie rosną **64 borowiki**. Minimapę dopasowano do nowych dróg i polan.

Cichy test `--forest-qa --no-audio` zapisuje widoki polan, odległych obszarów, całej mapy i raport w `dist/qa-v25/`. Podgląd układu lasu: `dist/qa-v25/forest-overview.png`.

## Wersja 0.24 — prolog i Serce Świtu

Gra zaczyna się sceną w zrujnowanym obserwatorium. **Mag Eldrin w fioletowych szatach z laską** rozmawia z Wędrowcem o pękającej Pieczęci Popiołu i nadchodzącej inwazji demonów. Świat może ocalić **Serce Świtu**, rozbite na **pięć części**, z których każdej broni osobny strażnik. Pierwszy trop prowadzi do Strażnika ruin w północnej części istniejącego Lasu Mchu. Dziennik wyprawy przypomina ten cel.

Prolog ma 12 wypowiedzi obu postaci, stopniowo pojawiający się polski tekst, animowane postacie, pięć świetlistych odłamków i magiczny portal. Po dokładnych słowach **„Ruszaj więc”** mag rzuca zaklęcie, bohater zostaje przeniesiony do dotychczasowego punktu startowego lasu, a sterowanie wraca po wygaszeniu efektu. Scena odtwarza się również na początku nowej wyprawy po śmierci.

Eldrin ma zaokrągloną sylwetkę, fałdy i złote hafty szat, szczegółową twarz i falującą brodę oraz ozdobną laskę z kryształem. Obserwatorium zdobią kamienny łuk, kolumny z pnączami, grawerowany podest i świece. Przyciski **DALEJ / POKAŻ TEKST** mają taki sam odstęp od prawej ramki jak tekst od lewej. Przybycie do lasu otacza bohatera **niebieskim słupem magii**, z pierścieniami światła i unoszącymi się iskrami; błysk szybko ustępuje, a słup wygasa przez dwie sekundy.

**Wędrowiec ma nowy, zaokrąglony model**: kaptur z widoczną twarzą i włosami, profilowany napierśnik i naramienniki, dłonie, skórzane buty, sprzączki oraz płaszcz z falującymi fałdami i lamówką. Ten sam model pojawia się w lesie, prologu i portrecie. Osiem kierunków, chód, zamach mieczem, naciąganie łuku, kołczan, tarcza, hełm i przewrót korzystają z aktualnej sylwetki. Zbliżenia mają dodatkowe wygładzenie brył. Podgląd postaci i animacji: `dist/qa-v24/hero/poses-8-directions.png`.

Dialog przewija się sam, pozostawiając czas na lekturę. **Enter, Spacja lub LPM** najpierw pokazują resztę bieżącej wypowiedzi, a kolejne naciśnięcie przechodzi dalej. **Esc / POMIŃ** pomija rozmowę, zachowując teleportację. **F11** działa w trakcie sceny. Utrata aktywności okna zatrzymuje scenę; ruch, walka, ekwipunek oraz przeciwnicy są zablokowani do końca przejścia. Obecny las pozostaje pierwszym etapem fabuły; pozostałe krainy i strażnicy są na razie zapowiedziani w dialogu.

Cichy test `--intro-qa --no-audio` zapisuje raport, pełną rozmowę i obrazy do `dist/qa-v24/intro/`. `--startup-smoke --no-audio` sprawdza teraz rzeczywisty start prologu. Testy mechaniki i benchmarki zaczynają się od razu w lesie.

## Wersja 0.23 — koło broni i łuk

**Q** lub przycisk **BROŃ · Q** otwiera ośmiopolowe koło i wstrzymuje rozgrywkę. Na początku ma przypisany miecz oraz łuk; oba są rzeczywistymi przedmiotami — miecz w ręku, łuk w plecaku. **LPM na polu koła** zakłada broń i zamyka menu. Poprzednia broń trafia w miejsce wybranej w plecaku, więc przełączanie działa także przy pełnym plecaku.

Aby dodać broń, otwórz Q i **przeciągnij ją z plecaka lub pola BROŃ na sektor koła**. Możesz też otworzyć Q w trakcie przeciągania. Przypisanie tworzy skrót, bez kopiowania lub usuwania przedmiotu. Obsługuje miecze, łuki, topory i sztylety, także ich poziomy rzadkości. Przenoszenie przedmiotu wewnątrz plecaka nie psuje skrótu. Porzucona broń jest oznaczona BRAK i nie można jej wybrać do czasu odzyskania. **PPM na sektorze** usuwa sam skrót. Q lub ESC zamyka koło.

Po wybraniu łuku **przytrzymaj LPM na świecie**, aby naciągać cięciwę, i **puść LPM**, aby wypuścić jedną strzałę. Celowanie działa w stronę kursora, a po najechaniu na potwora kieruje strzał w jego pozycję, bez trwałego zaznaczania celu. Celownik ma pierścień naciągu i zaznacza przewidywany kierunek oraz maksymalny zasięg. HUD pokazuje liczbę strzał, procent naciągu i zasięg. Ruch podczas naciągania jest wolniejszy; unik przerywa celowanie.

Na początku jest **20 strzał w plecaku**. Każdy wypuszczony strzał zużywa jedną sztukę. Strzały łączą się w stosy do 99, a łuk korzysta ze wszystkich stosów w plecaku. Brak amunicji blokuje strzał. Użycie menu, zmiana broni, unik, utrata aktywności okna oraz puszczenie LPM nad HUD przerywają celowanie bez zużycia strzały. Kliknięcia ekwipunku zachowują przeciąganie i nie rozpoczynają strzału.

Naciąg rośnie do maksimum przez **1,4 s**. Zasięg wynosi **2–12 pól**, prędkość strzały **12–20 pól/s**, a bazowe obrażenia początkowego łuku **22–34**, przed obroną przeciwnika i dodatkami z Siły lub biżuterii. Początkowy miecz zadaje 14. Strzała trafia pierwszy napotkany żywy cel; drzewa, inne przeszkody i krawędź mapy ją zatrzymują. Nie przebija celów i nie zadaje obrażeń obszarowych. Trafienie jest fizyczne i uwzględnia obronę. Zabójstwa zachowują ciała z łupem, doświadczenie i pauzę przy awansie. Odnowienie strzału wynosi 0,38 s.

Bohater ma łuk w rękach oraz kołczan na plecach, z widocznymi strzałami dopóki ma amunicję. Ręce i cięciwa animują naciąg we wszystkich ośmiu kierunkach. Łuk zajmuje obie ręce w grafice. Miecz zachowuje jeden obszarowy zamach na kliknięcie LPM.

## Wersja 0.22 — zbieranie grzybów

Na początku każdej nowej wyprawy **36 borowików** rośnie w losowych miejscach lasu, na wolnym podłożu poza ścieżkami. Nie blokują ruchu. Podejdź na maksymalnie **2,5 pola** i przeciągnij grzyb z mapy lub panelu **W ZASIĘGU** do plecaka. Grzyby łączą się w stosy do 99 sztuk. Podświetlenie i panel pod kursorem pokazują nazwę, rodzaj oraz leczenie.

**PPM na borowiku w plecaku** zjada jedną sztukę i odnawia **10 HP**, do aktualnego maksimum zdrowia. Przy pełnym HP grzyb nie jest zużywany. Obowiązuje dotychczasowe odnowienie używania przedmiotów (0,65 s). Pełny plecak, niepasujący stos, anulowane przeciąganie lub odejście z zasięgu pozostawiają grzyb w lesie. Zebrane grzyby nie odrastają podczas tej wyprawy; nowa wyprawa losuje nowy zestaw. Rosnące grzyby zastępują wcześniejsze dekoracyjne kępki; początkowych rozrzuconych przedmiotów sprzętu nadal nie ma.

## Wersja 0.21 — bardziej szczegółowa grafika

Odświeżono obecną grafikę, zachowując izometryczny widok i paletę Emberfall. Postacie i potwory mają obłe, cieniowane bryły oraz więcej detali: futro i zęby zwierząt, pancerz pająków, zdobienia zbroi i tarczy, szwy płaszczy, korę i liście ożywionych drzew. Grafiki ciał odpowiadają nowym sylwetkom. Bohater nadal ma osiem kierunków animacji, wysunięte cięcie mieczem i przewrót.

Sosny, brzozy i dęby mają gęstsze korony, drobne liście, gałęzie, korzenie i fakturę pni. Skały, krzewy, grzyby i kolumny otrzymały mech, pęknięcia, jagody i dodatkowe światło. Teren ma mniej widoczną siatkę oraz trawę, kamienie, kwiaty, gałązki i ślady na ścieżkach. Ikony przedmiotów dostały detale materiałów, odbicia szkła i metalu oraz miękkie cienie. Kula ognia zostawia iskry, a wybuch ma drobne obłoki dymu.

Drobne szczegóły lasu są renderowane do tekstur przy starcie. Animowane postacie odrzucają niewidoczne tylne powierzchnie zamkniętych brył, aby ograniczyć koszt nowych detali. Wskazywanie myszy korzysta z aktualnych kształtów; falowanie drzew, dopasowanie pełnego ekranu, loot i sterowanie pozostają zachowane.

## Wersja 0.20 — rozbudowana knieja i łup gatunkowy

Mapa urosła z **30 × 30 do 48 × 48 pól**. Nowe ścieżki prowadzą do Wilczej kniei, Brzozowej polany, Pajęczych ostępów i Serca Kniei. Las jest gęstszy i bardziej zróżnicowany: oprócz sosen są kołyszące się brzozy i dęby, grzyby oraz odmienne podłoże. Drzewa przed bohaterem stają się półprzezroczyste, aby go nie zasłaniać. Minimapę, granice ruchu, wodę i kolizje dopasowano do większej mapy.

Na mapie jest **46 przeciwników, 8 gatunków**. Do szlamów, wilków, szkieletów i strażnika dołączyły **dziki, leśne pająki, leśni rozbójnicy i ożywione drzewa**. Mają własne grafiki, osiem kierunków animacji, ciała, statystyki i zachowanie oparte na patrolu oraz pościgu. Jad pająków zadaje obrażenia magiczne. Informacje i główne szanse łupu są widoczne po najechaniu na przeciwnika.

Dodano **12 materiałów**: kieł i sierść wilka, kieł i skórę dzika, pajęczą nić, woreczek jadu, fragment kości, śluz szlama, prastarą korę, esencję kniei, odznakę rozbójnika oraz kamień runiczny. Nowy sprzęt to **sztylet, płaszcz i amulet** — razem 15 nowych rodzajów przedmiotów. Materiały mają własne ikony i opisy, łączą się w stosy do 99 sztuk i na razie służą jako trofea.

Każdy gatunek ma własną tabelę dropów. Pozycje losują się niezależnie: np. **kieł wilka 75%, sierść wilka 55%**, więc oba mogą pojawić się z jednego zabójstwa. Łup pozostaje w ciele otwieranym PPM i jest przeciągany do plecaka. Podpowiedź przedmiotu w ciele pokazuje jego szansę. Pełna tabela: [LOOT.md](LOOT.md).

## Wersja 0.19 — synchronizacja wyświetlania

Gotowe klatki trafiają teraz do synchronizowanego bufora DirectX (DXGI flip). Wyświetlanie czeka na odświeżenie monitora, a gra korzysta z jednego mechanizmu prezentacji. Usunięto wcześniejsze przełączanie bufora OpenGL bez V-sync w trybie pełnego ekranu. Oczekiwanie kompozytora dotyczy już tylko zasłoniętego okna, które Windows wyłącza z prezentacji.

Renderowanie świata pozostaje w raylib. Obraz jest przekazywany między OpenGL i DirectX w pamięci GPU. Końcowy przebieg DirectX rysuje całą klatkę z tej powierzchni do bufora wyświetlania, bez kopiowania klatek przez procesor. Po zmianie rozmiaru lub F11 bufory są odtwarzane wraz z aktualnym dopasowaniem HUD i kursora. Przy braku obsługi współdzielenia GPU gra automatycznie korzysta z V-sync OpenGL.

## Wersja 0.18 — mniejszy koszt renderowania

Ograniczono obciążenie rysowania świata i animowanej wody: fale poza widocznym obszarem są odrzucane wcześniej, obliczenia ich krzywych są współdzielone, a rysowanie figur postaci nie tworzy zbędnych tymczasowych tablic. Wygładzanie świata używa skali 1,5× na oś zamiast 2×, co przy 1080p oznacza około 44% mniej pikseli w tym buforze. Teren nadal przesuwa się płynnie o ułamki piksela, a HUD pozostaje w natywnej rozdzielczości.

Usunięto dodatkowe wygładzanie gotowej klatki, które nie było potrzebne do prezentacji obrazu z buforów. Czcionki walki, podpowiedzi i menu są przygotowane wcześniej, dzięki czemu ich pierwsze wyświetlenie nie wymaga tworzenia atlasu podczas klatki rozgrywki.

Zwykła rozgrywka nie gromadzi już w pamięci historii czasów każdej klatki. Takie dane są zbierane wyłącznie w testach.

## Wersja 0.17 — poprawiona pozycja kursora

Naprawiono rozbieżność między widocznym kursorem a punktem wskazywanym w grze przy skalowaniu Windows. Dotyczy to mapy, podświetlania, celowania, przycisków HUD i przeciągania przedmiotów. Pozycja myszy jest przeliczana zgodnie z aktualnie wyświetlanym obrazem, również po zmianie rozmiaru okna i przełączeniu F11.

## Wersja 0.16 — domyślny pełny ekran i płynny ruch

Gra uruchamia się domyślnie na **pełnym ekranie**, w rozdzielczości monitora. **F11** lub **OKNO · F11** przywraca okno. Opcjonalny parametr `--windowed` pozwala zacząć od razu w oknie.

W wersji 0.16 dodano synchronizację z kompozytorem pulpitu. Wersja 0.19 zastępuje ją synchronizowaną prezentacją DXGI dla widocznego okna, z V-sync OpenGL jako ścieżką zastępczą. Gra unika podwójnego oczekiwania na odświeżenie. Na monitorze 60 Hz celem jest 60 FPS; przy wyższych częstotliwościach wybiera całkowitą liczbę odświeżeń na klatkę (np. 120 Hz → 60 FPS, 144 Hz → 72 FPS), aby ruch miał równy rytm.

Przesuwający się teren i dekoracje mają filtrowanie ułamkowych przesunięć. Świat jest dodatkowo wygładzany w osobnym buforze, a HUD i napisy zachowują natywną ostrość. Chód i bieg płynnie zmieniają pozę między wcześniejszymi ośmioma klatkami; kierunki, szybkość, zasięg przewrotu i sterowanie pozostają takie same. Cienie i ostrzeżenia nie przeskakują między całymi pikselami.

Testy podczas prac nad kodem uruchamia się z **`--no-audio`**, bez inicjowania urządzenia dźwiękowego i bez zmiany ustawień dźwięku gracza.

## Wersja 0.15 — dopasowanie pełnego ekranu i płynniejszy wiatr

Naprawiono ucinanie prawej i dolnej części ekranu przy powiększeniu interfejsu w ustawieniach Windows. Cały obraz gry jest dopasowany do dostępnej rozdzielczości, także przy 1920 × 1080 i skali systemowej 125%. Obraz zachowuje proporcje; ewentualne pasy po bokach lub u góry i dołu mieszczą cały HUD bez rozciągania. F11 oraz przycisk w HUD nadal przełączają między oknem i pełnym ekranem, a mysz wskazuje właściwe pola ekwipunku. F12 zapisuje teraz faktycznie wyświetlaną klatkę wraz z dopasowaniem do okna.

Drzewa mają łagodniejszy rytm podmuchów, dokładniejsze uginanie i płynny ruch także o ułamki piksela. Woda ma gładkie, delikatnie zanikające fale; ich krawędzie są wygładzane bez rozmywania napisów HUD. Przejścia jasności i ruch fal są spokojniejsze. Pauza nadal zatrzymuje animacje świata.

## Wersja 0.14 — wiatr i pełny ekran

Drzewa mają animowane korony i gałęzie, które kołyszą się i uginają podczas podmuchów. Pień przy ziemi pozostaje nieruchomy. Każde drzewo ma własną fazę ruchu; podświetlenie i informacje pod kursorem podążają za jego aktualnym kształtem. Na wodzie przesuwają się zmienne, delikatne fale zgodne z kierunkiem wiatru. Animacje zatrzymują się razem ze światem podczas pauzy.

**F11** lub przycisk **PEŁNY EKRAN · F11** u góry HUD przełącza grę na pełny ekran bez ramek, w natywnej rozdzielczości monitora. Ponowne użycie F11 albo przycisku **OKNO · F11** przywraca rozmiar i położenie okna. Przycisk działa także w menu, podczas pauzy i awansu. Proporcje obrazu, czytelność napisów i obsługa ekwipunku myszą są zachowane.

## Wersja 0.13 — patrole i pusty teren na starcie

Nowa wyprawa zaczyna się **bez przedmiotów na ziemi**. Broń, zbroja, mikstury i pozostały sprzęt początkowy zostają w ekwipunku bohatera. Łup z potworów nadal zbiera się z ciał, a przedmioty można ręcznie odkładać na ziemię.

Gdy potwór nie widzi bohatera, **patroluje okolicę swojego miejsca respawnu**, wybierając różne punkty do 2,5 pola od niego i robiąc krótkie przerwy. Porusza się wtedy z 60% szybkości pościgu. Widzi bohatera w zasięgu 6,5 pola, jeśli widoku nie zasłania drzewo, skała, krzew lub kolumna. Wykrycie bohatera natychmiast przerywa patrol i rozpoczyna pościg. Po utracie widoczności potwór wraca do domu, omijając przeszkody, i wznawia patrol.

Patrole używają ośmiu kierunków animacji i istniejących kolizji: potwory nie przenikają przez siebie, teren ani brzegi mapy. Pauza i menu awansu zatrzymują także patrol oraz przerwy. Martwe ciała pozostają nieruchome. Losowanie tras jest niezależne od losowania łupu.

## Ciała potworów i łup w HUD z wersji 0.12

Pokonany potwór zostawia **ciało z łupem w środku**. Przedmioty z zabójstwa nie pojawiają się na ziemi. **PPM na ciele** otwiera jego zawartość w prawym HUD, zamiast listy przedmiotów na ziemi. Panel pokazuje nazwę potwora i pięć pól na łup. **LPM + przeciągnięcie do plecaka** zabiera przedmiot; nie można użyć go ani założyć przed zebraniem. Pełny lub zajęty plecak nie usuwa łupu; przy częściowym łączeniu stosów reszta pozostaje w ciele.

Ciała pozostają na mapie również po opróżnieniu. Można zamknąć okienko przyciskiem **X**, wrócić później albo otworzyć inne ciało. Otwarcie i zabieranie przedmiotów wymaga odległości do **2,5 pola**; odejście zamyka panel. Ciała nie blokują ruchu. Szlam, wilk, szkielet i strażnik mają własną grafikę poległej postaci oraz podświetlenie pod kursorem. Strażnik zachowuje oba legendarne przedmioty w swoim ciele.

**C** rzuca wybrany czar w stronę kursora. **K** lub kliknięcie kafelka czaru otwiera księgę wyboru. **PPM** otwiera ciało i nie rzuca czaru. Spacja, bieg pod Shiftem i pojedynczy zamach LPM pozostają bez zmian. Przedmioty odłożone przez gracza nadal leżą na ziemi i można je przeciągać dotychczasowym sposobem.

## Bieg i sterowanie z wersji 0.11

**Spacja** uruchamia dotychczasowe przeturlanie w kierunku WSAD lub ostatniego ustawienia. **Przytrzymany lewy lub prawy Shift + WSAD** daje bieg o **50% szybszy** od chodu (5,7 pola/s, z butami 6,45). Puszczenie Shifta od razu przywraca chód. Bieg nie zwiększa dystansu przewrotu.

**LPM na mapie** wykonuje jeden zamach mieczem, trafiający wszystkich przeciwników w promieniu 2,1 pola. Każdy zamach wymaga nowego kliknięcia i gotowej broni; przytrzymanie myszy nie powtarza ataku. **C** rzuca wybrany czar w stronę kursora. Klawisz R nie jest już przypisany do rzucania czaru.

LPM na przedmiocie nadal rozpoczyna przeciąganie. Kliknięcia w HUD, minimapę i menu nie uruchamiają walki. PPM na miksturze w plecaku lub w panelu przedmiotów na ziemi nadal ją wypija; na ciele otwiera jego zawartość. Rzucanie czaru i atak są zablokowane podczas przewrotu, pauzy i przeciągania przedmiotu. Pasek akcji, pomoc i księga czarów pokazują nowe przypisania.

## Statystyki, awans i Kula ognia z wersji 0.10

Po każdym awansie świat zatrzymuje się i otwiera menu z **5 punktami do rozdania**. Przyciskami **+ / −** ustalasz podział i widzisz podgląd parametrów. **Zatwierdź i wróć do gry** wydaje punkty jednocześnie oraz wznawia wyprawę. Trzeba rozdać wszystkie punkty; gdy jeden atak daje kilka poziomów, punkty sumują się. Pauza obejmuje potwory, lecące pociski, odnowienia oraz regenerację many. **P** lub przycisk **STATYSTYKI** w prawym HUD pozwala obejrzeć kartę bohatera.

| Statystyka | Korzyść z jednego punktu |
|---|---|
| Siła | +2 obrażenia fizyczne bronią |
| Moc magiczna | +3 obrażenia magiczne czaru |
| Obrona | +1 obrony fizycznej, czyli o 0,45 mniej otrzymanych obrażeń fizycznych |
| Obrona magiczna | +1 obrony magicznej, czyli o 0,45 mniej otrzymanych obrażeń magicznych |
| Wytrzymałość | +10 maksymalnego HP |
| Mana | +5 maksymalnej many |

Początkowo bohater ma **100 HP i 100 many**, a statystyki do rozdzielenia mają wartość 0. Przyrosty HP, many i obrażeń wynikają z przydzielonych punktów oraz sprzętu; dawny automatyczny przyrost parametrów za poziom został zastąpiony tym systemem. Awans nadal odnawia zdrowie i manę. Regeneracja many daje **dokładnie 1 punkt co 5 sekund aktywnej gry**, do aktualnego maksimum.

**K** lub kliknięcie kafelka czaru na pasku akcji otwiera księgę. Na razie dostępna jest **Kula ognia**, wybrana domyślnie. **C** wystrzeliwuje ją w stronę kursora; najazd na potwora celuje w jego pozycję. Koszt to **10 many**, odnowienie **0,8 s**, zasięg lotu **8 pól**. Podstawowe obrażenia wynoszą **28** wobec **14** zwykłego miecza. Moc magiczna rozwija czar niezależnie od siły i broni. Kula ma płonący ślad, zatrzymuje się na przeszkodach i wybucha, raniąc pobliskich przeciwników. Trafienia używają obrony magicznej.

Strażnik ruin zadaje obrażenia magiczne, oznaczone niebieskim ostrzeżeniem; od wersji 0.20 dotyczy to także jadu leśnych pająków. Pozostałe potwory atakują fizycznie. HUD przeciwnika pokazuje typ ataku i obronę magiczną. Fala żaru została zastąpiona Kulą ognia.

## Wersja 0.9 — osobne przełączniki dźwięku w HUD

W prawym dolnym rogu obszaru gry, obok paska akcji, są stale widoczne przyciski **MUZYKA** i **EFEKTY**. Każdy pokazuje stan **WŁ. / WYŁ.** i działa niezależnie, bez otwierania menu. Pozostają widoczne i działają także podczas pauzy, pomocy oraz ustawiania głośności. Wyłączenie efektów ucisza także odgłosy już trwające.

Przełączniki zachowują wartości suwaków głośności. Oba stany zapisują się przy zamykaniu gry; wcześniejsze pliki ustawień nadal działają. **M** pozostaje wspólnym wyciszeniem. Po jego użyciu kliknięcie MUZYKA albo EFEKTY włącza wyłącznie wskazany kanał. Przycisk **GŁOŚNOŚĆ · F2** pod przełącznikami otwiera dotychczasowe suwaki.

## Wersja 0.8 — atak obszarowy i kolizje potworów

**Jeden zamach trafia wszystkich żywych przeciwników w promieniu 2,1 pola wokół bohatera**, także z boku i z tyłu. Każdy otrzymuje obrażenia tylko raz, z uwzględnieniem własnej obrony. Każdy pokonany potwór zostawia ciało z łupem i daje doświadczenie. Broń zachowuje wysuniętą animację z wersji 0.7 oraz odnowienie 0,46 s.

Usunięto wybór celu kliknięciem i stały pierścień zaznaczenia. Bohater macha w kierunku swojego ustawienia. Najazd myszą nadal podświetla obiekt i pokazuje jego informacje w prawym HUD; odsunięcie kursora usuwa wskazanie.

Potwory mają fizyczny rozmiar, odpychają się i lekko odbijają przy zderzeniu. Strażnik ma większy promień i masę. Kolizje obejmują także stojących i przygotowujących atak przeciwników oraz przeszkody i brzegi mapy. Martwe potwory nie blokują drogi. Ruch jest dzielony na krótkie kroki, aby szybkie stworki nie przechodziły przez siebie między klatkami.

## Animacja miecza z wersji 0.7

Bohater wysuwa rękę i prowadzi miecz szerokim cięciem przed sobą. Ostrze przechodzi z prawej na lewą, a ślad cięcia podąża za jego pozycją w perspektywie izometrycznej. Zasięg broni zwiększono z 1,8 do **2,1 pola**. Atak wymaga pojedynczego kliknięcia LPM; jego odnowienie i czas animacji pozostały bez zmian. Nowy zamach działa w ośmiu kierunkach.

## Sterowanie i inspekcja z wersji 0.6

Atak bronią wykonuje się **kliknięciem LPM na mapie**. Każdy zamach wymaga nowego kliknięcia; przytrzymanie przycisku i kliknięcie ikony miecza nie powtarzają ataku. Nadal obowiązuje odnowienie broni (0,46 s) oraz blokada ataku podczas przewrotu. Czar przypisany do C wybiera się w księdze pod K.

Sekcja **POD KURSOREM** na dole prawego HUD pokazuje nazwę, rodzaj, aktualne/maksymalne HP, pasek zdrowia, atak, obronę, szybkość i zasięg trafienia wskazanego przeciwnika. Dla drzewa, skały, krzewu lub kolumny pokazuje nazwę i rodzaj; dla łupu także ilość i rzadkość. Dane znikają po odsunięciu kursora lub śmierci celu. Plecak zachowuje 25 pól, a wyposażenie 6 miejsc.

Statystyki przeciwników są wspólne dla HUD i walki. Szlam ma atak 9 / obronę fizyczną 0, wilk 12 / 1, szkielet 15 / 3, strażnik ruin magiczny atak 25 / obronę fizyczną 8 / obronę magiczną 6. Obrona odpowiedniego rodzaju zmniejsza otrzymane obrażenia o 0,45 punktu na punkt obrony, z minimum 2 obrażeń.

## Dźwięk z wersji 0.5

Gra ma autorską, zapętloną muzykę **„Echoes of the Ruins”** (około 101 sekund, stereo) oraz **21 efektów dźwiękowych**. Słychać kroki, zamachy, odmienne trafienia każdego rodzaju przeciwnika, upadki, obrażenia i śmierć bohatera, przewrót i unik trafienia, rzucanie czaru, mikstury, podnoszenie i odkładanie łupu, zakładanie sprzętu, awans i zwycięstwo. Odległe zdarzenia są cichsze, a ich pozycja na ekranie wpływa na lewy/prawy kanał. Kroki występują tylko podczas faktycznego chodzenia.

**F2** lub przycisk **GŁOŚNOŚĆ** otwiera panel z osobną głośnością muzyki i efektów. **M** wycisza całość lub przywraca wcześniejsze stany kanałów. Ustawienia zapisują się przy zamykaniu w `audio-settings.cfg` obok EXE. Otwarcie panelu pauzuje świat. W menu muzyka cichnie; przełączenie na inne okno wstrzymuje muzykę i odgłosy. Gra działa też bez urządzenia audio. Muzyka jest odczytywana strumieniowo z niewielkiego pliku OGG; Python ani narzędzia do tworzenia muzyki nie są potrzebne do gry.

## Ruch i unik z wersji 0.4

Bohater i wszystkie rodzaje przeciwników poruszają się o około **23% szybciej**. Bohater pokonuje 3,8 jednostki mapy na sekundę, a w butach 4,3; szlam i szkielet 1,85, wilk 2,6, strażnik 1,35.

**Spacja** wykonuje przeturlanie w kierunku trzymanych klawiszy WSAD, a podczas stania — w ostatnim kierunku ustawienia postaci. Pełny przewrót trwa 0,4 s, pokonuje do 2,8 jednostki mapy i odnawia się przez 1,2 s od rozpoczęcia. Pierwsze 0,28 s chroni przed trafieniem; końcówka animacji pozwala przeciwnikowi zadać obrażenia. Złoty krąg pod postacią sygnalizuje ochronę, a napis „UNIK” oznacza uniknięte trafienie. Przeturlanie zatrzymuje się na przeszkodach i granicach mapy. Nie kosztuje many; w trakcie nie można atakować ani rzucać zaklęcia. Kierunek pozostaje ustalony do końca przewrotu. Przycisk **UNIK** na dole także uruchamia tę akcję i pokazuje odnowienie. Każde kolejne przeturlanie wymaga nowego naciśnięcia klawisza.

## Sterowanie i wskazywanie z wersji 0.3

Sterowanie ruchem to teraz **WSAD**: W — góra, S — dół, A — lewo, D — prawo; połączenia klawiszy dają ruch ukośny. Strzałki nie sterują już bohaterem.

Po najechaniu kursorem teren otrzymuje podświetlenie całego pola. Wróg ma ciepły czerwony obrys, podświetlone pole oraz nazwę i zdrowie. Drzewa, skały, krzewy i kolumny otrzymują złoty obrys z nazwą; łup na ziemi jest oznaczony złotym pierścieniem. Wskazywanie korzysta z kształtu grafiki i kolejności rysowania, więc nie wybiera obiektów przez przeszkody ani przez minimapę lub panel interfejsu. Przeciąganie łupu korzysta z tego samego wskazania.

## Grafika i animacja z wersji 0.2

Napisy są renderowane w natywnej rozdzielczości okna, z dopasowaniem do DPI Windows i bez rozciągania gotowego obrazu interfejsu. Małe etykiety mają większy rozmiar, pogrubienie i wyższy kontrast. Okno pomocy używa większych napisów.

Bohater, szlam, wilk, szkielet i strażnik mają po osiem kierunków: E, SE, S, SW, W, NW, N, NE. Każdy kierunek ma animację bezczynności, ośmioklatkowy chód i animację ataku. Ruch uruchamia się na podstawie faktycznej zmiany pozycji, więc postacie nie idą w miejscu po zablokowaniu. Po zatrzymaniu zachowują kierunek. Potwory obracają się ku bohaterowi podczas przygotowania ataku; bohater zachowuje kierunek ustawienia przy zamachu. Animacja zatrzymuje się podczas pauzy i nie zależy od FPS.

## Uruchomienie

Kliknij dwukrotnie **Uruchom.bat** w Eksploratorze Windows albo uruchom **dist/Emberfall.exe**. Gra zaczyna się prologiem; Enter, Spacja lub LPM przyspieszają dialog, a Esc go pomija. Folder `assets` musi pozostać obok pliku EXE. Cały folder `dist` jest przenośną wersją gry.

## Sterowanie

| Przycisk | Działanie |
|---|---|
| W / S / A / D | Ruch: góra / dół / lewo / prawo; możliwe kierunki ukośne |
| Shift (lewy lub prawy, przytrzymany) + WSAD | Bieg o 50% szybszy od chodu |
| Spacja (pojedyncze naciśnięcie) | Przeturlanie w kierunku ruchu lub ustawienia; odnowienie 1,2 s |
| LPM na mapie (pojedyncze kliknięcie) | Jeden zamach; wszyscy przeciwnicy w promieniu 2,1 pola |
| Najazd kursorem na potwora lub obiekt | Informacje w sekcji POD KURSOREM po prawej |
| V / kafelek LECZENIE | Poznane Leczenie: +30 HP za 20 many |
| C | Wystrzelenie wybranego czaru w stronę kursora: Kula ognia za 10 many |
| PPM na ciele potwora | Otwarcie jego łupu w prawym HUD |
| K / kliknięcie kafelka czaru | Księga czarów i wybór czaru |
| P / STATYSTYKI w prawym HUD | Karta statystyk bohatera |
| 1 / 2 | Mikstura zdrowia / many z plecaka |
| PPM na miksturze w plecaku lub panelu przedmiotów na ziemi | Wypicie wskazanej mikstury |
| LPM + przeciągnięcie | Przenoszenie przedmiotów |
| F1 | Pomoc |
| F2 | Ustawienia głośności muzyki i efektów |
| F11 / PEŁNY EKRAN w górnym HUD | Pełny ekran bez ramek / powrót do okna |
| MUZYKA / EFEKTY w HUD | Osobne włączenie lub wyłączenie danego kanału |
| M | Wyciszenie / włączenie całego dźwięku |
| Esc | Pauza / powrót |
| Enter po śmierci | Nowa wyprawa |
| F12 | Zrzut wyświetlonego obrazu do `screenshots/emberfall-v31.png` obok EXE |

Klikalne przyciski akcji na dole pozwalają wybierać czar, pić mikstury i robić unik. Ikona miecza jest wskaźnikiem ataku pod LPM. Utrata aktywności okna pauzuje rozgrywkę.

## Loot i ekwipunek

- PPM na ciele potwora otwiera jego łup w HUD. Przeciągnij przedmioty z panelu do plecaka; potem można je założyć lub użyć. Ciała można ponownie otwierać.
- Podejdź do łupu na maksymalnie 2,5 pola. Przeciągnij przedmiot bezpośrednio z mapy lub z sekcji **Na ziemi** do plecaka albo na pasujące miejsce sprzętu.
- 25 pól plecaka; 6 miejsc wyposażenia: broń, hełm, tarcza, zbroja, buty, pierścień. Ekwipunek znajduje się stale po prawej stronie.
- Przeciąganie na zajęte miejsce zamienia przedmioty. Nieprawidłowe przeniesienie pozostawia przedmiot na swoim miejscu. Mikstury, skarby i materiały łączą się w stosy do 99 sztuk; reszta zostaje w źródle.
- Przeciągnij przedmiot z plecaka na wolną ziemię blisko postaci, by go odłożyć. Można także przesuwać łup na ziemi.
- Jeśli w pobliżu leży więcej niż 5 przedmiotów, przewijaj listę kółkiem myszy lub strzałkami pod nią.
- Najedź na przedmiot, aby sprawdzić jego nazwę, rzadkość i statystyki. Zielone przedmioty są niepospolite, złote — legendarne.
- Broń i pierścień zwiększają atak, pozostały sprzęt pancerz. Buty zwiększają również szybkość ruchu.
- Czerwona mikstura odnawia 50 HP, niebieska 35 many. Pełne zdrowie lub mana nie zużywają mikstury.

## Wyprawa

Mapa ma 30 × 30 pól. Pokonuj szlamy, wilki i szkielety, zdobywaj doświadczenie oraz ulepszaj sprzęt. Silny kamienny strażnik czeka w północnych ruinach (złoty punkt na minimapie). Upuszcza legendarny miecz i pierścień. Zamach fizyczny sygnalizuje czerwony okrąg, magiczny — niebieski; można od niego odejść lub wykonać unik. Awans odnawia zdrowie i manę oraz daje punkty statystyk.

To prototyp jednej lokalnej wyprawy. Nie ma jeszcze zapisu stanu wyprawy, handlarzy, multiplayera ani kolejnych map. Złoto i odłamki są zbieralnym łupem; nie mają jeszcze zastosowania u handlarza. Zamknięcie gry kończy bieżącą wyprawę.

## Kompilacja i testy

W PowerShell w folderze projektu:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 -Test
```

Projekt zawiera lokalny kompilator w `tools/w64devkit` i raylib w `external/raylib-5.5_win64_mingw-w64`. Skrypt użyje systemowego `g++`, jeśli lokalny kompilator nie istnieje. W razie odtwarzania zależności pobierz [w64devkit 2.0.0](https://github.com/skeeto/w64devkit/releases/tag/v2.0.0) i [raylib 5.5 dla MinGW x64](https://github.com/raysan5/raylib/releases/tag/5.5), a następnie wypakuj do tych katalogów. Zależności nie są pobierane podczas uruchamiania gry.

`tests/game_tests.cpp` sprawdza m.in. przenoszenie i zachowanie liczby przedmiotów, zgodność slotów, zasięg łupu, stosy, leczenie, odnowienie umiejętności, obrażenia, śmierć przeciwnika, loot, bossa i niezależność ruchu od liczby klatek.

```powershell
Start-Process .\dist\Emberfall.exe -ArgumentList '--smoke','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--integration','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--benchmark','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--benchmark','--windowed','--large','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--benchmark','--fullscreen','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--environment-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--presentation-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--refresh-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--motion-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--pointer-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--performance-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--benchmark-long','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--profile','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--visual-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--hover-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--roll-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--audio-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--inspection-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--swing-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--crowd-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--progression-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--controls-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--corpse-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--patrol-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--forest-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--graphics-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--mushroom-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--bow-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--skills-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--tower-floors-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--tower-benchmark','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--dev-qa','--no-audio' -WindowStyle Hidden -Wait
Start-Process .\dist\Emberfall.exe -ArgumentList '--start','tower-3','--dev-smoke','--no-audio' -WindowStyle Hidden -Wait
```

Pierwsza komenda renderuje 180 klatek i zapisuje zrzut. Druga wykonuje 13 sprawdzeń interakcji, używając tych samych funkcji wskazywania, rozpoczynania i kończenia przeciągania co mysz; raport zapisuje do `dist/interaction-test-v29.txt`. Nie jest to automatyzacja urządzenia wejściowego systemu Windows. Benchmark uruchamia 900 klatek z ruchem kamery, walką, unikami, podświetlaniem i wiatrem, a `--windowed --large` wybiera okno 1920 × 1080. Raporty mają nazwy `benchmark-v29.txt`, `benchmark-v29-large.txt` i `benchmark-v29-fullscreen.txt`. `--visual-qa` eksportuje arkusze animacji oraz podglądy interfejsu w trzech skalach do `dist/qa-v02`. `--hover-qa` wykonuje 24 sprawdzenia wskazywania i eksportuje podglądy do `dist/qa-v04`. `--roll-qa` zapisuje arkusz przewrotów (8 kierunków × 12 klatek) w `assets/generated/v04/hero-roll.png` obok EXE oraz podglądy w `dist/qa-v04`. Testy w `tests/roll_tests.hpp` sprawdzają szybkość, kierunki uniku, kolizje, ochronę, odnowienie i niezależność od FPS.

Gra synchronizuje klatki z monitorem, celując w 60 FPS na ekranie 60 Hz; osiągana płynność zależy od komputera i obciążenia systemu. Logika korzysta z rzeczywistego czasu między klatkami. Podłoże i podstawowe grafiki dekoracji są renderowane do tekstur przy starcie, a korony drzew są deformowane podczas rysowania każdej klatki. Fale na wodzie są animowane osobno pod lądem.

W wersji 0.25 aktualne raporty wydajności i ekwipunku mają przyrostek `v25`. Eksporty `--hover-qa` i `--roll-qa` pozostają w `dist/qa-v04`. `--audio-qa` sprawdza rzeczywisty mikser, wszystkie efekty, granicę pętli muzycznej, wyciszanie, utratę aktywności okna, sumowanie efektów i zapis ustawień; raporty oraz podglądy panelu i nowych przycisków HUD zapisuje w `dist/qa-v09`. Nie zmienia ustawień gracza. Wariant `--no-audio` sprawdza działanie z pominiętym urządzeniem audio. `audio/create_audio.py` odtwarza zasoby z syntezy przy użyciu NumPy i SoundFile; nie jest uruchamiany przy starcie ani kompilacji gry.

`--environment-qa` porównuje maskę wskazywania drzewa z jego faktycznie wyrenderowanym, poruszającym się kształtem w ośmiu pozycjach i skalach. Sprawdza zmiany pikseli wody i nieruchomy ląd, podświetlenie i HUD, rzeczywiste przejścia okno–pełny ekran, przywrócenie rozmiaru, pozycji i maksymalizacji, zachowanie menu awansu oraz mapowanie wszystkich pól plecaka. Raport i obrazy są w `dist/qa-v14`. `--fullscreen` pozwala też uruchomić grę lub benchmark bezpośrednio na pełnym ekranie.

`--presentation-qa` sprawdza końcowy obraz w buforze ekranu, po przejściu przez rzeczywistą ścieżkę wyświetlania gry. Wykrywa przycięcie znaczników przy wszystkich czterech narożnikach, przy czterech rozmiarach okna i odziedziczonych transformacjach DPI 100/125/150/200% (bez zmiany ustawień Windows). Sprawdza też mapowanie plecaka i płynne zmiany pikseli drzew oraz wody w kolejnych 45 klatkach. Raport i pełne zrzuty okna są w `dist/qa-v15`.

`--motion-qa --no-audio` sprawdza start na pełnym ekranie, synchronizację, faktyczne czasy klatek podczas biegu i przewrotu, odległości ruchu, płynne przewijanie terenu o ułamki piksela i przejścia między pozami chodu. Raport jest w `dist/qa-v16/motion-tests.txt`. Test używa tych samych funkcji ruchu i wyświetlania co gra, bez wstrzykiwania fizycznych zdarzeń wejściowych.

`--pointer-qa --no-audio` sprawdza całą ścieżkę pozycji myszy, od prawdziwego callbacku raylib/GLFW przez skalowanie biblioteki do wskazywania i interakcji. Obejmuje pełny ekran, trzy rozmiary okna i powrót F11, odziedziczone skale 100/125/150/200%, 25 pól plecaka, sprzęt, celowanie, wskazywanie, otwieranie ciała i przeciąganie łupu. Test podaje współrzędne do callbacku wewnątrz aplikacji; nie porusza fizycznego kursora, nie wysyła zdarzeń do Windows ani nie zmienia systemowego DPI. Raport: `dist/qa-v17/pointer-tests.txt`.

`--performance-qa --no-audio` porównuje zoptymalizowaną wodę z algorytmem wersji 0.17 w 24 ujęciach przy brzegach i narożnikach. Sprawdza też pierwsze użycie tekstów walki, inspekcji, ekwipunku i menu bez ładowania nowych atlasów czcionek. Raport: `dist/qa-v18/performance-tests.txt`.

`--benchmark-long --no-audio` wykonuje 1800 klatek i zapisuje również P99 oraz liczbę klatek ponad 25 ms. `--profile --no-audio` mierzy osobno symulację, wskazywanie, wodę, świat, HUD i prezentację. Ten ostatni test używa celowych barier GPU (`glFinish`), które zmieniają przebieg klatki; raport `dist/performance-profile.txt` służy do porównań etapów, a nie oceny rzeczywistego FPS. Zwykła gra nie stosuje tych barier.

Paczka dla znajomych: `powershell -ExecutionPolicy Bypass -File .\package.ps1`. Powstaje `Emberfall-Windows-x64.zip` z folderem gry, instrukcją i zasobami, bez kompilatora, generatora audio i plików testowych.

`--inspection-qa` sprawdza wskazywanie potworów i obiektów, bieżące HP w HUD, atak po kliknięciu potwora i brak ataku po kliknięciu wskaźnika miecza i wszystkie pola plecaka po zmianie układu. Raport i obrazy zapisuje w `dist/qa-v13`. `tests/combat_press_tests.hpp` sprawdza pojedyncze ataki i zgodność statystyk obrony/ataku z walką.

`--swing-qa` eksportuje podgląd cięcia w ośmiu kierunkach oraz scenę trafienia dalej stojącego wilka do `dist/qa-v07`. Arkusz `assets/generated/v07/hero-attack.png` obok EXE ma 8 × 8 klatek po 208 × 160 pikseli (kotwica stóp 104, 135). Eksporter `--visual-qa` także poszerza komórki ataku bohatera do 208 pikseli, aby nie ucinać wysuniętego miecza.

`--crowd-qa` zapisuje testy oraz obrazy ataku obszarowego i tłumu 14 potworów do `dist/qa-v13`. `tests/crowd_combat_tests.hpp` sprawdza zasięg we wszystkich kierunkach, pojedyncze trafienie grupy, łup, awans w trakcie zamachu, odpychanie, brak przenikania przy 20/30/60/120 FPS oraz kolizje tłumu z terenem.

`--controls-qa` sprawdza pojedyncze zamachy LPM, czary C, celowanie, przeciąganie, mikstury, blokady HUD i menu oraz niezmieniony przewrót przy biegu. Raport i podglądy sterowania w trzech skalach są w `dist/qa-v13`. Testy logiki sprawdzają bieg z butami i bez, ukośne kierunki, puszczenie Shifta, kolizje i 30/60/120 FPS. Diagnostyka wywołuje funkcje aplikacji, bez wstrzykiwania fizycznych zdarzeń wejściowych.

`--corpse-qa` sprawdza rzeczywiste zabójstwa, grafiki i wskazywanie czterech rodzajów ciał, otwieranie PPM, przeciąganie do plecaka, częściowe stosy, zamykanie, ponowne otwieranie i zasięg. Obrazy i raport są w `dist/qa-v13`. `tests/corpse_tests.hpp` sprawdza zachowanie łupu, blokady, stabilne ID i nagrody strażnika.

`--patrol-qa` sprawdza pusty start i restart, zachowany ekwipunek, odkładanie przedmiotów oraz animację i wskazywanie czterech rodzajów patrolujących potworów. Raport i podglądy są w `dist/qa-v13`. `tests/patrol_tests.hpp` sprawdza minutę patrolu każdego gatunku, granice okolicy respawnu, 30/60/120 FPS, zasłanianie widoku, powrót wokół przeszkody, pauzę i niezależne losowanie łupu.

## Struktura

`--progression-qa` sprawdza automatyczne otwarcie menu po zabiciu przeciwnika, przyciski +/− i zatwierdzenie, wybór czaru, celowanie oraz trafienie pociskiem. Eksportuje menu w trzech skalach, księgę czarów i obrazy lotu/wybuchu do `dist/qa-v13`. `tests/progression_tests.hpp` sprawdza naliczanie punktów, nieprawidłowe podziały, pauzę całej symulacji, sześć statystyk, regenerację przy różnych FPS oraz koszt, zasięg i kolizje Kuli ognia.

- `src/game.hpp` — model świata, walka i transakcje ekwipunku, bez zależności od grafiki.
- `src/items.hpp`, `src/loot.hpp` — rodzaje przedmiotów, opisy, premie i niezależne tabele szans łupu.
- `src/forest_qa.hpp`, `tests/forest_loot_tests.hpp` — nowy las, osiem gatunków, drop rate, materiały i sprzęt.
- `src/enemy_ai.hpp` — widoczność, patrolowanie i powrót do miejsca respawnu.
- `src/main.cpp` — grafika izometryczna, interfejs, wejście, proceduralne ilustracje i pętla gry.
- `src/scenery_art.hpp` — szczegółowe tekstury terenu, drzew, roślin i ruin.
- `src/graphics_qa.hpp` — porównanie wyrenderowanych sylwetek z geometrią kursora i eksport nowych grafik.
- `src/animation.hpp` — osiem kierunków i cykle animacji zależne od pokonanej odległości.
- `src/actor_art.hpp`, `src/actor_view.hpp` — obracane postacie, kończyny, broń, twarze i animacje.
- `src/typography.hpp` — kroje pisma rasteryzowane do fizycznego rozmiaru napisów w aktualnym oknie.
- `src/visual_qa.hpp` — renderowanie w natywnej rozdzielczości i eksport podglądów.
- `src/world_hover.hpp`, `src/hover_qa.hpp` — wykrywanie wskazanego pola i obiektu, obrysy oraz testy zasłaniania.
- `src/roll_qa.hpp`, `tests/roll_tests.hpp` — podglądy przewrotów i testy mechaniki uniku.
- `src/audio.hpp`, `src/audio_ui.hpp`, `src/sound_events.hpp` — odtwarzanie, mikser, ustawienia i zdarzenia audio.
- `src/audio_qa.hpp`, `tests/audio_events_tests.hpp` — testy odtwarzania i dźwięków zdarzeń.
- `src/entity_info.hpp`, `src/inspection_hud.hpp`, `src/input_actions.hpp` — statystyki, panel informacji i akcje myszy.
- `src/inspection_qa.hpp`, `tests/combat_press_tests.hpp` — testy wskazywania i ręcznego ataku.
- `audio/create_audio.py`, `assets/audio/` — źródło syntezy oraz gotowa muzyka i efekty.
- `resources/Emberfall.manifest` — obsługa DPI dla poszczególnych monitorów Windows.
- `assets/` — kroje pisma z licencjami; `assets/generated/` zawiera eksportowane przy uruchomieniu grafiki przedmiotów i dekoracji.
- `dist/` — gotowa gra, zasoby i dokumentacja.

Grafiki mapy, postaci, przeciwników, przedmiotów i dekoracji są zdefiniowane w kodzie i tworzone lokalnie. Nie korzystają z pobranych sprite’ów ani usług sieciowych.

`--refresh-qa --no-audio` porównuje wszystkie piksele świata i HUD z docelowym buforem GPU podczas ruchu, sprawdza zawartość sceny i cztery narożniki, przełączanie F11, zmianę rozmiaru, dodatni interwał synchronizacji i pojedynczą prezentację na klatkę. Raport: `dist/qa-v19/refresh-tests.txt`. Benchmarki odróżniają prezentacje przyjęte przez DXGI od odrzuconych dla zasłoniętego okna (`Occluded presentations`). Wyniki rytmu pracy zasłoniętego okna nie potwierdzają fizycznego odświeżenia ekranu.

`--forest-qa --no-audio` sprawdza wszystkie nowe obszary, cztery nowe gatunki, ich faktyczną grafikę i wskazywanie, HUD ze statystykami i drop rate, ciała otwierane PPM oraz przeciąganie materiałów do prawego pola plecaka. Eksportuje osiem kierunków nowych potworów, las i podgląd kła/sierści wilka do `dist/qa-v20`. `tests/forest_loot_tests.hpp` sprawdza także wszystkie spawny, granice, stosy i nowy sprzęt; wykonuje 30 000 losowań na gatunek, kontrolując szanse, ilości i niezależność dropów.

`--graphics-qa --no-audio` porównuje niezależnie rasteryzowany obraz GPU z geometrią wskazywania wszystkich dziewięciu sylwetek w ośmiu kierunkach oraz bohatera podczas cięcia i przewrotu. Wykrywa znikające lub ucięte części, błędne wnętrza i puste przestrzenie. Sprawdza również pełne tekstury dekoracji oraz wszystkie 26 ikon. Eksportuje arkusz sylwetek i faktycznie wyświetlany obraz lasu do `dist/qa-v21/`. Aktualne grafiki są eksportowane do `assets/generated/*-v21.png` oraz `prop-v21-*.png` obok EXE.

`--mushroom-qa --no-audio` sprawdza rzeczywiste wskazywanie borowików, informacje w HUD, przeciąganie z mapy i panelu do plecaka, zasięg, łączenie stosów oraz zjadanie PPM. Raport i podglądy: `dist/qa-v22/`. Logika losowania, zachowanie pełnego plecaka i leczenie są dodatkowo sprawdzane w `tests/mushroom_tests.hpp`. Nowy atlas przedmiotów `assets/generated/items-v22.png` mieści 27 ikon na trzech wierszach; grafiki terenu, postaci i dekoracji zachowują eksporty `v21`.

`--bow-qa --no-audio` sprawdza koło, przypisywanie przedmiotów, zmianę broni, naciąganie i puszczanie LPM, neutralne kliknięcia HUD, amunicję, trzy skale renderowania oraz rasteryzację i wskazywanie łuku z kołczanem w ośmiu kierunkach. Raport i podglądy: `dist/qa-v23/`. Testy modelu są w `tests/bow_tests.hpp`. Nowy atlas `assets/generated/items-v23.png` mieści 29 ikon. `audio/create_bow_sound.py` odtwarza efekt strzału z łuku bez jego odtwarzania; testy rozwoju nadal zawsze korzystają z `--no-audio`. Benchmark 900 klatek obejmuje od teraz także zmianę na łuk, naciąganie, strzały i powrót do miecza.

`--skills-qa --no-audio` sprawdza przejście statystyki → umiejętności → gra, przydzielanie i cofanie punktów, blokady broni, aktywację z HUD, naciąg, koszty, przeszywanie trzech wrogów, anulowanie oraz animacje miecza i łuku w ośmiu kierunkach. Podglądy i raport: `dist/qa-v27/`. Logika jest sprawdzana w `tests/skill_tests.hpp`. Benchmark zawiera obie umiejętności i zapisuje liczbę rzeczywistych cięć oraz wypuszczonych przeszywających strzał.

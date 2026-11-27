# Raport konfiguracji — 19.10.2026

**27.11.2026 — obrót Ropuchy i rękawów Dzwonnika z osobnych części 2D**

Ropucha ma korpus o stałej wysokości i cztery łapy osadzone względem niego. W teście 512 kierunków skoki szerokości obrysu korpusu spadły o 73%. Dzwonnik ma dwa osobne rękawy osadzone w barkach. Pomiar bocznego obrysu na wysokości rękawów poprawił się o około 1%. W obu przypadkach zweryfikowano geometrię 6144 poz.

Eksport w Blenderze przez MCP trwał 8,45 s. Przechwycenie 372 klatek z renderera gry trwało 16,18 s. Zaliczono 1651 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-czesci-2d-20261127-023415/Mokradla-czesci-2d.blend) · [Chód i skręt w grze](C:/AI2D/pipeline/runs/mokradla-czesci-2d-20261127-023415/Mokradla-czesci-2d.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-czesci-2d-20261127-023415/report.json)

**27.11.2026 — zgodny skręt dzwonu i stabilniejszy tułów Ropuchy**

Ruch dzwonu jest rzutowany względem przodu i boku Dzwonnika. Krawędź zasłaniania ma stałe położenie w układzie płaszcza. Test porównuje to samo wychylenie z przodu i z profilu. Ropucha korzysta z zamkniętej krzywej środka i szerokości przekroju tułowia. W kontroli 512 kierunków skoki szerokości spadły o około 47%. Pole całej sylwetki zmieniło się od −5,1% do +6,0% względem poprzednich klatek. Kontrola geometrii objęła 6144 pozy sztywnego tułowia.

Eksport w Blenderze przez MCP trwał 8,31 s, a przechwycenie 372 klatek z ukrytego renderera bez dźwięku 16,18 s. Zaliczono 1651 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-zgodny-skret-20261127-022557/Mokradla-zgodny-skret.blend) · [Chód i obrót przy zatrzymanym kroku](C:/AI2D/pipeline/runs/mokradla-zgodny-skret-20261127-022557/Mokradla-zgodny-skret.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-zgodny-skret-20261127-022557/report.json)

**27.11.2026 — barki, łokcie i przesunięcie dzwonu Dzwonnika**

Ramię i przedramię mają własne osie obrotu. Przedramię śledzi łokieć przeniesiony razem z ramieniem. W dwóch półkrokach na wprost punkt na rękawie przesuwa się o 11,64 piksela, a test zachowuje szerokość i wysokość klatki. Wygładzenie krzywej obrysu rękawów zmniejszyło skoki podczas skrętu o około 20%. W próbie ze stałym kierunkiem dzwon przesuwa się przy kroku w bok i w pionie. Osobny tor prowadzi go w bok i wyżej podczas zamachu.

Eksport w Blenderze przez MCP trwał 8,12 s, a przechwycenie 372 klatek z ukrytego renderera bez dźwięku 16,31 s. Zaliczono 1650 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/dzwonnik-barki-i-dzwon-20261127-020454/Dzwonnik-barki-i-dzwon-final.blend) · [Krok i zamach w grze](C:/AI2D/pipeline/runs/dzwonnik-barki-i-dzwon-20261127-020454/Dzwonnik-krok-i-zamach.webp) · [Wyniki](C:/AI2D/pipeline/runs/dzwonnik-barki-i-dzwon-20261127-020454/report.json)

**27.11.2026 — sztywny tułów i osobny ruch dzwonu**

Rig obu przeciwników prowadzi środek tułowia jako sztywną część, a odkształcenie kończyn wygasa w stawach. Eksport sprawdził stałe wymiary tułowia w 12 288 pozach chodu, ataku i reakcji na trafienie. Dopasowanie obrysu do zamkniętej krzywej skrętu zmniejszyło skoki jego szerokości o około 30% u Ropuchy i 51% u Dzwonnika. Dzwon ma osobną teksturę. W silniku kołysze się wokół zawieszenia w rytm kroku, unosi w zamachu i chowa za płaszczem. Kontrole geometrii i ruchu dzwonu są częścią eksportu i testów gry.

Eksport w Blenderze przez MCP trwał 7,46 s, a przechwycenie 372 klatek z ukrytego renderera gry bez dźwięku 16,40 s. Zaliczono 1649 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-sztywny-rig-20261127-015006/Mokradla-sztywny-rig.blend) · [Chód i atak w grze](C:/AI2D/pipeline/runs/mokradla-sztywny-rig-20261127-015006/Mokradla-krok-i-dzwon.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-sztywny-rig-20261127-015006/report.json)

**27.11.2026 — przestrzenny dzwon i dopasowanie tekstury przy skręcie**

Zasięg tekstury korpusu dopasowano do lewej i prawej krawędzi sylwetki. Przed zmianą skrajny obszar tekstury powtarzał się w najgorszym ujęciu na 11,2% widocznych pikseli Ropuchy i 10,3% Dzwonnika. Po zmianie kontrola 1024 klatek wykazała zero widocznych powtórzeń tego rodzaju. Dzwon wypieczono jako osobny kształt 2D z owalnym rantem i ciemnym otworem. Od przodu i z ukosa miał 26–27 pikseli szerokości, a od tyłu był całkowicie zasłonięty. W tej samej próbie skrętu mediana kontrastu detali zmieniła się o około −1% u Ropuchy i −4% u Dzwonnika.

Eksport w Blenderze przez MCP trwał 6,85 s, a przechwycenie 312 klatek w ukrytym rendererze bez dźwięku 13,64 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-dzwon-przestrzen-20261127-013601/Mokradla-przestrzenny-dzwon.blend) · [Chód i skręt w grze](C:/AI2D/pipeline/runs/mokradla-dzwon-przestrzen-20261127-013601/Mokradla-dzwon-chod.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-dzwon-przestrzen-20261127-013601/report.json)

**27.11.2026 — jedna tekstura w każdej klatce skrętu Ropuchy i Dzwonnika**

Kolor klatek pośrednich pochodzi z jednej tekstury powstałej z ośmiu namalowanych widoków. ComfyUI FILM wyznacza dopasowanie obrysu. Dzwon zajmuje jedno miejsce na przedzie tekstury. Dla 1024 klatek odtworzono RGB z samej tekstury z maksymalną różnicą jednego poziomu kwantyzacji. Sprawdzono alfę sylwetek oraz podparcie w chodzie. W przechwyconych klatkach kontrast detali był większy o 64% u Ropuchy i 53% u Dzwonnika. Zmienność względem tego kontrastu wzrosła o 8% i 2%.

Eksport Blendera przez MCP trwał 6,81 s. Przechwycenie 312 klatek w ukrytym rendererze gry bez dźwięku zajęło 13,77 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-jedna-tekstura-20261127-012254/Mokradla-jedna-tekstura-final.blend) · [Skręt w trakcie chodu](C:/AI2D/pipeline/runs/mokradla-jedna-tekstura-20261127-012254/Mokradla-jedna-tekstura.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-jedna-tekstura-20261127-012254/report.json)

**27.11.2026 — czytelniejsza faktura Ropuchy i Dzwonnika w trakcie skrętu**

Szczegóły skóry i kory wypieczono na wspólnej powierzchni z namalowanych widoków. Każdy fragment faktury korzysta głównie z najbliższego ujęcia, a jego kontrast przy samym brzegu sylwetki maleje. W tej samej próbie skrętu w silniku mediana kontrastu drobnych detali wzrosła o 80% u Ropuchy i 93% u Dzwonnika. Zmienność detali względem ich kontrastu wzrosła odpowiednio o 8% i 5%. Sprawdzono alfę 1024 sylwetek i zgodność geometrii chodu z poprzednim eksportem.

Eksport w Blenderze przez MCP trwał 7,13 s, a przechwycenie 312 klatek w ukrytym rendererze gry bez dźwięku 13,55 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-ostry-pigment-20261127-010724/Mokradla-ostry-pigment.blend) · [Chód i skręt w grze](C:/AI2D/pipeline/runs/mokradla-ostry-pigment-20261127-010724/Ostry-chod-Mokradel.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-ostry-pigment-20261127-010724/report.json)

**27.11.2026 — stabilniejsze podparcie Ropuchy i Dzwonnika przy skręcie**

Punkt podparcia w chodzie wyznacza cały obszar łapy lub stopy przy podłożu. W Blenderze dopasowano zamknięte krzywe przebiegu podpór, barków i głowy. Największy ruch punktu podparcia między sąsiednimi pozycjami skrętu spadł z 6,50 do 3,75 piksela u Ropuchy i z 4,75 do 1,69 piksela u Dzwonnika. W porównaniu obrotu z zamrożoną fazą kroku nadal widać drobną zmianę obrysu Ropuchy. Próbę z inną interpolacją rysunków odrzucono po pomiarze obrysu.

Eksport w Blenderze przez MCP trwał 7,14 s. Przechwycenie 312 klatek z ukrytego renderera bez dźwięku zajęło 13,85 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-ciagly-skret-20261127-005424/Mokradla-ciagly-krok.blend) · [Chód w grze](C:/AI2D/pipeline/runs/mokradla-ciagly-skret-20261127-005424/Mokradla-ciagly-krok.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-ciagly-skret-20261127-005424/report.json)

**27.11.2026 — Ropucha torfowa i Dzwonnik Topieli, obrót wspólnej powierzchni**

Lokalny ComfyUI FILM wyznacza przesunięcia w obu kierunkach między istniejącymi ujęciami. Większe formy podążają za tymi przesunięciami, a kolor powierzchni przesuwa się po jednym zamkniętym pasie 2D. Retusz kontrastu jest ograniczony do widocznego oka i dzwonu. W powtórzonej próbie skrętu w silniku skok detali między klatkami spadł o 15% u Ropuchy i 24% u Dzwonnika. Szybko zmienny składnik detalu spadł odpowiednio o 3% i 19%. Faktura w obrębie tułowia jest łagodniejsza.

Eksport w Blenderze przez MCP trwał 6,99 s, a przechwycenie 312 klatek z ukrytego renderera gry bez dźwięku 13,73 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-przeplyw-20261127-003043/Mokradla-wspolna-powierzchnia-retusz.blend) · [Chód i skręt w grze](C:/AI2D/pipeline/runs/mokradla-przeplyw-20261127-003043/Mokradla-chod.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-przeplyw-20261127-003043/report.json)

**27.11.2026 — 24 ujęcia kierunkowe Ropuchy torfowej i Dzwonnika Topieli**

Porównano chód Szakala z pustyni i Rogacza z Kaldery. Ropucha i Dzwonnik zyskali po osiem osobno namalowanych sylwetek z Sunburst, dopasowanych kolorystycznie w ComfyUI i zapisanych jako osobne warstwy w Kricie. Rysunki i ich odbicia zwiększyły liczbę kluczowych widoków z 8 do 24 na postać. Widoki są rozstawione co 15° i zachowują skalę oraz podparcie. W próbie w silniku przy zatrzymanej fazie kroku migotanie drobnych szczegółów na tułowiu zmalało o 21,6% u Ropuchy i o 11,4% u Dzwonnika. Drobna faktura została przy tym złagodzona. [Ropucha w Kricie](C:/AI2D/sprites/mokradla-gestsze-kierunki-20261127-000904/Ropucha-dodatkowe-kierunki.kra) · [Dzwonnik w Kricie](C:/AI2D/sprites/mokradla-gestsze-kierunki-20261127-000904/Dzwonnik-dodatkowe-kierunki.kra)

Eksport 1024 klatek i 16384 póz siatki w Blenderze przez MCP trwał 7,46 s. Przechwycenie 312 klatek w ukrytym rendererze gry bez dźwięku trwało 17,11 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni oraz cichy start Mokradeł. [Projekt Blendera](C:/AI2D/animations/mokradla-gestsze-kierunki-20261127-000904/Mokradla-24-kierunki.blend) · [Podgląd chodu w grze](C:/AI2D/pipeline/runs/mokradla-gestsze-kierunki-20261127-000904/Ropucha-Dzwonnik-chod.webp) · [Raport](C:/AI2D/pipeline/runs/mokradla-gestsze-kierunki-20261127-000904/report.json)

**27.11.2026 — stabilizacja pigmentu Ropuchy i Dzwonnika przy skręcie**

Drobne plamy skóry i kory filtruje się kątowo po zarejestrowaniu odpowiadających sobie miejsc na sylwetce. Największe formy zachowały odpowiednio 95,4% i 95,8% kontrastu. Alfa wszystkich 1024 ustawień kierunkowych i binarne dane ruchu mają identyczne wartości. W rendererze gry, przy zamrożonej fazie kroku, pomiar w obrębie tułowia wykazał spadek szybko zmiennego składnika detalu o 33,8% u Ropuchy i 31,2% u Dzwonnika.

Eksport w Blenderze przez MCP zajął 7,04 s, a przechwycenie 312 klatek z ukrytej gry bez dźwięku 14,54 s. Zaliczono 1646 kontroli grafiki, 1001 kontroli cieni i cichy start Mokradeł. [Projekt w Blenderze](C:/AI2D/animations/mokradla-stabilny-pigment-20261126-235539/Mokradla-stabilny-pigment.blend) · [Bezstratny podgląd z gry](C:/AI2D/pipeline/runs/mokradla-stabilny-pigment-20261126-235539/Ropucha-Dzwonnik-stabilny-obrot.webp) · [Porównanie](C:/AI2D/pipeline/runs/mokradla-stabilny-pigment-20261126-235539/report.json)

**26.11.2026 — płynny krok i skręt Ropuchy oraz Dzwonnika**

Krok interpoluje 16 kluczy jednej siatki według drogi przebytej na ziemi. Podpory i kołysanie sprawdzono osobno przy stałym kierunku. Skręt używa ciągłej krzywej obszaru sylwetki, punktu ciężkości i położenia oka lub dzwonu. Największe zmiany pola obrysu między sąsiednimi z 512 kierunków spadły z 1,85% do 0,66% u Ropuchy i z 1,93% do 0,71% u Dzwonnika. W dwóch niezależnych próbach z silnika wszystkie 60 klatek dla każdej postaci różniło się od siebie.

Finalny eksport w Blenderze przez MCP trwał 6,91 s. Przechwycenie 312 klatek chodu, kontroli kierunku i kontroli kroku z ukrytej gry bez dźwięku zajęło 16,56 s. Test grafiki zaliczył 1646 kontroli, test cieni 1001, a test logiki 588936. Cichy start Mokradeł doszedł do prezentacji gry. [Projekt Blender](C:/AI2D/animations/mokradla-plynny-chod-20261126-234116/Ropucha-i-Dzwonnik-final.blend) · [Skręt podczas chodu](C:/AI2D/pipeline/runs/mokradla-plynny-chod-20261126-234116/Plynny-chod-Ropuchy-i-Dzwonnika.webp) · [Pomiary](C:/AI2D/pipeline/runs/mokradla-plynny-chod-20261126-234116/report.json)

**26.11.2026 — nowe modele Ropuchy torfowej i Dzwonnika Topieli**

Zastąpiono rysunki obu postaci nowymi pełnymi sylwetkami kierunkowymi. Pozowanie do chodu, ataku, trafienia i śmierci prowadzi jedna siatka na sylwetkę. Sunburst przygotował rysunki, ComfyUI dopasował paletę do torfu, a [Ropucha w Kricie](C:/AI2D/sprites/mokradla-nowe-modele-20261126-232659/Ropucha.kra), [Dzwonnik w Kricie](C:/AI2D/sprites/mokradla-nowe-modele-20261126-232659/Dzwonnik.kra) oraz [projekt animacji w Blenderze](C:/AI2D/animations/mokradla-nowe-modele-20261126-232659/Ropucha-i-Dzwonnik-final.blend) zachowują edytowalne źródła.

Sprawdzono 3072 klatki chodu i 16384 pozy siatki obu postaci. Żadna poza nie odwraca trójkątów. Czas renderu w Blenderze wraz z poprawką upadku to 57,15 s. Przechwycenie 168 klatek z ukrytej gry bez dźwięku zajęło 9,72 s. Na finalnych zasobach zaliczono 1645 kontroli grafiki i 1001 kontroli cieni. [Podgląd z gry](C:/AI2D/pipeline/runs/mokradla-nowe-modele-20261126-232659/Ropucha-Dzwonnik-animacje.webp) · [Raport](C:/AI2D/pipeline/runs/mokradla-nowe-modele-20261126-232659/report.json)

**26.11.2026 — obrót tułowia Ropuchy i Dzwonnika**

Obrót odsłania osobno malowany grzbiet, boki i przód tułowia. Skrót sylwetki i położenia głowy, barków oraz bioder korzystają z tego samego kąta. Edytowalne materiały, siatki i klucze chodu zapisano w [projekcie Blendera](C:/AI2D/animations/mokradla-obrot-tulowia-20261126-231701/Mokradla-chod-0-3.blend).

Sprawdzono wszystkie 3072 klatki chodu obu przeciwników oraz ich widoki spoczynkowe. Render w Blenderze trwał 85,80 s, przechwycenie 96 klatek w ukrytym rendererze gry bez dźwięku 4,52 s. Zaliczono 1645 kontroli grafiki i 1001 kontroli cieni. [Obrót podczas chodu](C:/AI2D/pipeline/runs/mokradla-obrot-tulowia-20261126-231701/Ropucha-i-Dzwonnik-obrot.webp) · [Raport](C:/AI2D/pipeline/runs/mokradla-obrot-tulowia-20261126-231701/report.json)

**26.11.2026 — ruch stawów przeciwników Mokradeł podczas skrętu**

Gra wybiera jedną kompletną klatkę chodu z 128 kierunków i 12 faz kroku. W Blenderze osadzono łapy Ropuchy w stawach, poruszono pierścieniami Pijawki wzdłuż jednej osi, wsunięto nasady wici Błędnika pod latarnię i przedłużono obie warstwy rękawa Dzwonnika przy łokciu. Klatka wyznacza kolor, kształt cienia i obrys pod kursorem. Źródła ruchu to [Ropucha i Pijawka](C:/AI2D/animations/mokradla-spojny-ruch-20261126-230218/Mokradla-chod.blend) oraz [Błędnik i Dzwonnik](C:/AI2D/animations/mokradla-spojny-ruch-20261126-230218/Mokradla-chod-2-3.blend).

Przez MCP Blendera wypalono i sprawdzono 6144 końcowe klatki chodu. Render wraz z dodatkową próbą dwóch gatunków trwał 266,48 s. Przechwycenie 96 klatek w ukrytej grze bez dźwięku trwało 5,98 s. Na finalnych atlasach zaliczono 1645 kontroli grafiki i 1001 kontroli cieni. [Próba chodu z gry](C:/AI2D/pipeline/runs/mokradla-spojny-ruch-20261126-230218/Chod-przeciwnikow-Mokradel.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-spojny-ruch-20261126-230218/report.json)

**26.11.2026 — zasłanianie części ciała przeciwników Mokradeł podczas obrotu**

W Blenderze łapy i boki ropuchy zachodzą pod grzbiet, kolejne pierścienie pijawki zasłaniają dalsze segmenty, wici Błędnika chowają się za latarnią, a płaszcz Dzwonnika przykrywa dalsze ramię i dzwon. Każda z 73 malowanych części zachowuje kształt. W klatkach pośrednich gra wybiera kolor widocznej części na podstawie trzech dodatkowych próbek kąta między sąsiednimi ujęciami. Maski są wczytywane do pamięci graficznej razem z atlasami. [Projekt Blendera](C:/AI2D/animations/mokradla-przeslanianie-20261126/Mokradła-128.blend) · [Odbudowa zasobów](../creatures/marsh-turn/README.md)

Przy 512 ustawieniach kierunku test wykrył niezamierzone przecięcie warstw w 425 ujęciach poprzedniej wersji i w żadnym ujęciu po poprawce. Drugi test obejmował 11 relacji anatomicznych, w tym kolejność płaszcza, naramiennika i ramienia. Render kierunków i masek w Blenderze trwał 27,94 s, kompilacja atlasów ruchu i przesłon 24,39 s, a przechwycenie 1720 klatek w ukrytej grze bez dźwięku 89,14 s. [Pomiary zasłaniania](C:/AI2D/pipeline/runs/mokradla-przeslanianie-20261126/layers/result.json) · [Obrót w grze](C:/AI2D/pipeline/runs/mokradla-przeslanianie-20261126/qa/final/mokradla-obrot-w-grze.webp)

**26.11.2026 — proporcje ciał podczas obrotu na Mokradłach**

Każde ze 128 ujęć kierunkowych korzysta ze stałych rozmiarów malowanych części. Stawy prowadzą ich położenie oraz obrót. Eksport Blendera sprawdza kształt wszystkich 73 części. W chodzie pijawki kolejne pierścienie zginają się wzdłuż grzbietu. U ropuchy, Błędnika i Dzwonnika animacja rozdziela ruch łap lub wici od pracy tułowia. [Źródło kierunków](C:/AI2D/animations/mokradla-stabilny-obrot-20261126/Mokradła-128.blend) · [Źródło ruchu](C:/AI2D/animations/mokradla-stabilny-obrot-20261126/Mokradla-stabilne-akcje-final.blend)

Render 512 rysunków w Blenderze trwał 22,84 s. Wypieczenie siatek animacji zajęło 26,80 s, a przechwycenie 1720 klatek w ukrytej grze bez dźwięku 87,31 s. Podczas obrotu w chodzie mediana lokalnej zmiany proporcji w okolicy dzwonu spadła z 14,4% do 4,8%, latarni z 7,7% do 4,4%, pyska ropuchy z 14,8% do 11,3%, a pośrodku pancerza pijawki z 14,7% do 10,0%. Pomiar obrazu obejmuje również odsłanianie części ciała, szczególnie przy przyssawce pijawki. Zęby na rysunku zachowują wzajemny rozstaw. Osobno sprawdzono zgodność wskazań kursora z widocznym obrysem w 568 520 punktach. [Obrót w chodzie](C:/AI2D/pipeline/runs/mokradla-stabilny-obrot-20261126/qa/final/mokradla-obrot-w-grze.webp) · [Pełne pomiary](C:/AI2D/pipeline/runs/mokradla-stabilny-obrot-20261126/qa/final/runtime-stability.json)

**26.11.2026 — 128 kierunków przeciwników Mokradeł**

Pełny obrót Ropuchy torfowej, Pijawki dennej, Błędnika trzcinowego i Dzwonnika Topieli ma po 128 osobnych ujęć co 2,8125°. W projekcie Blendera 73 malowane części mają własne pozycje i kolejność zasłaniania. Podczas skręcania gra zachowuje fazę kroku i płynnie rozpędza i wyhamowuje obrót na płaszczyźnie gruntu. W teście nagłej zmiany celu przy 60 krokach symulacji na sekundę największy obrót w jednym kroku wyniósł 3,63° wobec wcześniejszych 10,63°. Wszystkie cztery łapy ropuchy zachowują tożsamość przy zmianie podparcia.

Wypalenie 512 kierunkowych rysunków zajęło 19,47 s w Blenderze. Siatki chodu, ataku, trafienia i upadku korzystają z 16 kluczy każdej akcji oraz interpolacji w czasie. [Kierunki w Blenderze](C:/AI2D/animations/mokradla-128-20261126/Mokradła-128.blend) · [Edytowalne akcje](C:/AI2D/animations/mokradla-128-20261126/Mokradla-128-akcje.blend) · [Odbudowa materiałów](../creatures/marsh-turn/README.md)

W ukrytym rendererze bez dźwięku przechwycenie chodu z pełnym skrętem, ataku, trafienia i upadku trwało 93,78 s. Zbadano po 256 kątów każdego gatunku w czterech akcjach, a osobno wartości po obu stronach każdego rysunku i połowy odstępu do następnego. W połowie odstępu mediany kontrastu krawędzi dla czterech gatunków wyniosły od 99,1% do 99,8% w porównaniu z sąsiednimi rysunkami. W najtrudniejszym przejściu Dzwonnika miara przeskoku widocznej powierzchni spadła z 7 do 2 px. [Podgląd z gry](C:/AI2D/pipeline/runs/mokradla-pelny-obrot-20261126/qa/accepted/mokradla-obrot-w-grze.webp) · [Wyniki](C:/AI2D/pipeline/runs/mokradla-pelny-obrot-20261126/report.json)

**26.11.2026 — ostrość animacji podczas skręcania na Mokradłach.**

Podczas skrętu gra prowadzi detale między odpowiadającymi sobie punktami dwóch rysunków. Drugi rysunek odsłania się miejscowo, np. kiedy pysk zasłania dalsze oko. Poprzednia wersja nakładała półprzezroczyste sylwetki na całej powierzchni. Ponowne wypalenie 1280 klatek w Blenderze zajęło 15,07 s. Wszystkie 60 porównań wspólnej pozy początkowej chodu, ataku, trafienia i śmierci dało identyczne wartości RGBA.

Przechwycenie 1055 klatek sceny rozgrywki w ukrytym oknie bez dźwięku trwało 56,90 s. W 880 klatkach chodu korekta podparcia nie powodowała pionowych przeskoków tułowia. Przy podporach krawędź ciała znajdowała się najwyżej jeden piksel od cienia. We wszystkich 164 526 porównaniach wybór kursorem zgadzał się z renderowaną sylwetką. [Podgląd](C:/AI2D/pipeline/runs/mokradla-ostry-obrot-20261126/qa/mokradla-ostry-obrot.webp) · [Raport i pomiary](C:/AI2D/pipeline/runs/mokradla-ostry-obrot-20261126/report.json)

**26.11.2026 — płynne skręty i upadki na Mokradłach.**

Ropucha, Pijawka, Błędnik i Dzwonnik mają po pięć malowanych widoków. Trzy boczne widoki mają prawe odpowiedniki. Podczas skrętu gra wyznacza położenie pomiędzy sąsiednimi sylwetkami dla bieżącej fazy chodu, ataku, trafienia lub upadku. Śmierć utrwala kierunek postaci. Ropucha podkurcza łapę i zamyka oczy, pijawka zwija ogon, a Błędnik składa wici i przygasa. Dzwonnik osuwa się na bok ze zgiętymi kończynami.

Blender 5.3 wyrenderował 1280 klatek w 61,86 s. W projekcie ponownie otwarto 20 scen z kluczami kształtu i spakowanymi teksturami. Źródła i sposób odbudowy znajdują się w [katalogu rysunków kierunkowych](../creatures/marsh-turn/README.md). [Projekt Blender](C:/AI2D/animations/mokradla-kierunki-20261126/Mokradla-kierunki.blend) · [Projekt Krita](C:/AI2D/sprites/mokradla-kierunki-20261126/Mokradla-obroty.kra)

W ukrytym rendererze gry, bez dźwięku, przechwycenie trwało 21,01 s. Dla każdego z czterech gatunków i każdej z czterech akcji sprawdzono 128 kątów. Największa zmiana obrysu między sąsiednimi kątami zmalała o 67–92% względem wcześniejszej animacji. W ujęciu bocznym ciała mają po upadku od 38% do 61% więcej wysokości. W 530 klatkach sceny cień stykał się z postacią. [Podgląd obrotu i upadku](C:/AI2D/pipeline/runs/mokradla-kierunki-20261126/qa/mokradla-obrot-i-upadek.webp) · [Pomiary](C:/AI2D/pipeline/runs/mokradla-kierunki-20261126/qa/preview.json)

**26.11.2026 — przeciwnicy Mokradeł Cichych Dzwonów.**

Ropucha torfowa porusza łapami, Pijawka denna wydłuża i skraca tułów, Błędnik trzcinowy ciągnie za sobą pędy, a Dzwonnik Topieli przenosi ciężar między korzeniami. Każdy ma cztery klipy po 16 póz. Chód zależy od przebytej drogi. Zwykły atak i fala dzwonu mają pozę kontaktu, a promień Błędnika zaczyna się w jasnym rdzeniu aktualnej klatki. Ranny przeciwnik reaguje na trafienie, po śmierci pozostaje w ostatniej pozie upadku.

Blender 5.3 wyrenderował 256 póz w 17,13 s. Ponowne otwarcie potwierdziło cztery edytowalne sceny z teksturami spakowanymi w projekcie. Narzędzia `tools/media/marsh_actor_blender.py` i `tools/media/marsh_actor_pack.py` tworzą atlasy oraz punkty podparcia i rdzenia używane przez grę. Cień, obrys pod kursorem i ciało korzystają z tej samej klatki i odbicia. [Projekt Blender](C:/AI2D/animations/mokradla-ruch-20261126-wydanie/Mokradla-bestie.blend)

Przechwycenie sceny rozgrywki w ukrytym oknie bez dźwięku trwało 11,69 s. We wszystkich kadrach czterech akcji każdego gatunku cień stykał się z sylwetką, a jej dół pozostawał w odległości najwyżej jednego piksela od punktu osadzenia. Gra potwierdziła obrażenia i utrzymanie pozy po upadku. [Podgląd animacji](C:/AI2D/pipeline/runs/mokradla-animacje-20261126/qa/mokradla-chod-atak-trafienie-upadek.webp) · [Raport](C:/AI2D/pipeline/runs/mokradla-animacje-20261126/report.json)

**25.11.2026 — szybsze ugryzienie Szakala wydmowego.**

Przygotowanie ugryzienia trwa 0,47 s, a powrót do pozycji bojowej 0,30 s. Łączny czas skrócił się o 110 ms. Przy kroku symulacji 60 Hz trafienie nastąpiło po 29 krokach, w pozie kontaktu szczęk, a powrót zajął 18 kroków. Test potwierdził jednorazowe obrażenia i wyświetlenie wszystkich 15 póz ugryzienia.

Przechwycenie z renderera gry trwało 1,31 s. W ukrytym oknie bez dźwięku sprawdzono też 180 klatek po starcie na pustyni. Wdrożony `dist/Emberfall.exe` ma sumę SHA-256 zgodną z plikiem użytym w teście. [Podgląd](C:/AI2D/pipeline/runs/szakal-szybszy-atak-20261125/szakal-szybszy-atak.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-szybszy-atak-20261125/report.json)

**25.11.2026 — płynniejsze skręcanie Szakala wydmowego.**

Renderer wyznacza widok pośredni z dwóch sąsiednich klatek tej samej fazy chodu. Mapa przesunięć jest wyliczona z odpowiadających sobie punktów płaskiego modelu Blendera. Dzięki temu położenie łap zmienia się razem z obrysem. Zaznaczanie kursorem używa tego samego obrysu.

W czterech próbach największa zmiana maski między kolejnymi klatkami zmalała o 89–90%. Sprawdzono między innymi przejście między stronami atlasu i zawinięcie kierunku przez 360°. Zaznaczanie zgodziło się z odczytem GPU w 66 660 próbkach. Wydobycie geometrii w Blenderze trwało 4,81 s, przechwycenie z gry 1,54 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-plynny-skret-20261125/szakal-plynny-skret.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-plynny-skret-20261125/preview.json)

Test grafiki zaliczył 1487 kontroli w ukrytym oknie i bez dźwięku. Sumy SHA-256 wdrożonego pliku `Emberfall.exe` i mapy ruchu zgadzają się ze sprawdzonymi plikami. [Raport wdrożenia](C:/AI2D/pipeline/runs/szakal-plynny-skret-20261125/report.json)

**25.11.2026 — naturalniejszy chód Szakala wydmowego.**

Punkt podparcia łapy i rozwiązanie stawu są wyznaczane osobno. Przedni łokieć zgina się o maksymalnie 53° w cyklu chodu zamiast 101°. Tylna kończyna ma wyżej ustawiony staw skokowy, a przednie łapy podążają za tylnymi po tej samej stronie z płynniejszym przeniesieniem obciążenia. W 1814 próbkach podparcia współrzędna łapy względem podłoża pozostawała stała wzdłuż kierunku ruchu.

Blender przez MCP wyrenderował 12 atlasów w 57,09 s. Sprawdzono 3072 klatki RGBA i 1152 pozy chodu w 48 kierunkach. Gra potwierdziła obrót podczas chodu, trafienie ugryzieniem i osiadanie ciała po upadku. Przechwycenie scen z gry trwało 6,57 s. Test grafiki zaliczył 1487 kontroli w ukrytym oknie i bez dźwięku. [Podgląd](C:/AI2D/pipeline/runs/szakal-naturalny-chod-20261125/szakal-wydmowy.webp) · [Projekt Blender](C:/AI2D/animations/szakal-naturalny-chod-20261125/Szakal-wydmowy.blend) · [Raport](C:/AI2D/pipeline/runs/szakal-naturalny-chod-20261125/report.json)

**25.11.2026 — model głowy i kaptura Kobry piaskowej.**

Głowa ma zwężający się profil pyska z tarczkami czołowymi, nozdrzami, osobną żuchwą oraz bursztynowymi oczami z pionową źrenicą. Łukowaty kaptur ma ciągły obrys, żebra i dwa odrębne malowania. Nowy atlas 512 × 512 zawiera przód i tył kaptura, tarczki głowy i żuchwę, z ośmiopikselowym marginesem wokół pól.

Przechwycenie 282 klatek z gry trwało 4,75 s. Bezstratny eksport i kontrola zgodności pikseli trwały 14,48 s. Podgląd obejmuje pełzanie, obrót o 360°, wyrzut jadu i upadek. [Podgląd](C:/AI2D/pipeline/runs/kobra-model-20261125/kobra-piaskowa.webp) · [Źródło Krity](C:/AI2D/sprites/cobra-anatomy-20261125/kobra-glowa-kaptur.kra) · [Weryfikacja atlasu](C:/AI2D/pipeline/runs/kobra-model-20261125/compare.json)

Pustynny poziom zaliczył 152 kontrole w ukrytym oknie i bez dźwięku. Wdrożona kompilacja i obie tekstury mają sumy SHA-256 zgodne z testowanymi plikami. [Test pustyni](C:/AI2D/pipeline/runs/kobra-model-20261125/desert-tests.txt) · [Wdrożenie](C:/AI2D/pipeline/runs/kobra-model-20261125/deployment.json)

**25.11.2026 — relief i ostrość łusek Kobry piaskowej.**

Tekstura 512 × 512 ma osobne warstwy cieni między łuskami, refleksów i tarczek brzusznych. W Kricie zagęszczono tarczki w zakresie UV faktycznie widocznym na wydłużonej szyi. Mapy normalnych i odbić z Laigtera wypalono w gotowym kolorze. Tekstura korzysta z mipmap i filtrowania anizotropowego 4×. W próbce szyi z silnika wzrost lokalnego kontrastu między rzędami wyniósł 66%, przy pikselowo identycznej masce wygładzonego obrysu.

Test pustyni zaliczył 152 kontrole w ukrytym oknie i bez dźwięku. Przechwycenie 282 klatek trwało 5,42 s. Bezstratny eksport i kontrola pikseli trwały 15,56 s. [Podgląd](C:/AI2D/pipeline/runs/kobra-relief-20261125/kobra-piaskowa.webp) · [Źródło Krity](C:/AI2D/sprites/cobra-relief-release-20261125/kobra-relief-final.kra) · [Wypalenie światła](C:/AI2D/pipeline/runs/kobra-relief-20261125/relief-release.json) · [Odbiór](C:/AI2D/pipeline/runs/kobra-relief-20261125/deployment.json)

**25.11.2026 — tekstura i wygładzenie obrysu Kobry piaskowej.**

Przywrócono zachodzące na siebie łuski z oryginalnego materiału ComfyUI, z korektą barwy, pasm grzbietu i tarczek brzusznych w Kricie. Głowa i kaptur korzystają z tej samej tekstury 256 × 256. Rozłożenie wzoru uwzględnia wydłużoną szyję, a dziewięć poziomów mipmap ogranicza migotanie łusek przy obrocie. Renderer pobiera cztery próbki na piksel w obszarze kobry. Test brzegów wykazał 140 pikseli o częściowej przezroczystości przy zachowaniu powierzchni sylwetki.

Przechwycenie 282 klatek trwało 6,03 s, a bezstratny eksport z porównaniem pikseli 16,22 s. [Podgląd](C:/AI2D/pipeline/runs/kobra-luski-20261125/kobra-piaskowa.webp) · [Raport przechwycenia](C:/AI2D/pipeline/runs/kobra-luski-20261125/report.json) · [Źródło Krity](C:/AI2D/sprites/cobra-scales-fit-20261125/kobra-skora.kra)

Test pustyni ukończył 152 kontrole w ukrytym oknie i bez dźwięku. Sumy SHA-256 wdrożonej gry i tekstury zgadzają się ze sprawdzoną kompilacją i eksportem. [Test pustyni](C:/AI2D/pipeline/runs/kobra-luski-20261125/desert-tests.txt) · [Wdrożenie](C:/AI2D/pipeline/runs/kobra-luski-20261125/deployment.json)

**25.11.2026 — ostrość i ruch Kobry piaskowej.**

Łuski mają większy, czytelniejszy wzór namalowany w Kricie na bazie dotychczasowego materiału z ComfyUI. Ciągła siatka prowadzi teksturę od szyi do końca ogona. Układ współrzędnych tekstury zachowuje orientację, gdy szyja przechodzi przez pion. Krzywe opracowane w Blenderze ograniczają wychylenie głowy przy pełzaniu i łączą pozycje cofnięcia, wyrzutu jadu i powrotu.

Przechwycenie 282 klatek z renderera gry trwało 4,82 s. Bezstratny eksport i porównanie z oryginalnymi pikselami trwały 15,92 s. Próba obejmowała pełny obrót podczas pełzania, trzy pociski jadu, trafienie oraz upadek i spoczynek. Pozostałe pola atlasu mają te same piksele. [Podgląd](C:/AI2D/pipeline/runs/kobra-ostrosc-20261125/kobra-piaskowa.webp) · [Raport przechwycenia](C:/AI2D/pipeline/runs/kobra-ostrosc-20261125/report.json) · [Krzywe Blendera](C:/AI2D/animations/cobra-motion-20261125/kobra-krzywe.blend) · [Projekt Krity](C:/AI2D/sprites/cobra-skin-20261125/kobra-luski.kra)

Kompilacja zaliczyła 152 kontrole pustynnego poziomu. Wdrożony `dist/Emberfall.exe` ukończył 180 klatek w trybie deweloperskim na pustyni z ukrytym oknem i bez urządzenia audio. [Testy pustyni](C:/AI2D/pipeline/runs/kobra-ostrosc-20261125/desert-tests.txt) · [Wdrożenie](C:/AI2D/pipeline/runs/kobra-ostrosc-20261125/deployment.json)

**24.11.2026 — MCP z Blenderem 5.3.0.**

Zarejestrowano lokalny serwer `blender` w Codex. Osobna instancja Codex App Server odkryła narzędzia, a klient STDIO sprawdził dodatek i odczytał scenę. Przez MCP utworzono płaski emblemat z czterech warstw, dodano klatki kluczowe, zapisano i ponownie otwarto BLEND. Podgląd otrzymano przez `look`. Dwie klatki PNG RGBA potwierdziły ruch o 84 px i zachowanie przezroczystości. Ponowny render po otwarciu projektu trwał 0,60 s i dał piksele identyczne z pierwotnym eksportem. Dodatek działa na `127.0.0.1:9876` z wyłączoną telemetrią. [Próba MCP](C:/AI2D/pipeline/runs/blender-mcp-20261124-172642/report.json) · [Projekt testowy](C:/AI2D/pipeline/runs/blender-mcp-20261124-172642/Test-Blender-MCP.blend)

**24.11.2026 — ciągłe połączenie zadu z tylnymi nogami Szakala wydmowego.**

Dolny zarys miednicy wypełnia płytki prześwit pomiędzy nasadami tylnych nóg. Górna część widocznego uda pobiera malowidło i światło z odpowiadającego punktu na zadzie. Poniżej nasady pozostaje naturalne rozdzielenie nóg. W 312 klatkach od tyłu i pod skosem porównano obszar bezpośrednio przy nasadach. W nowym eksporcie wszystkie 15 wykrytych tam przezroczystych próbek jest wypełnionych. Porównanie 4608 pozycji palców potwierdziło zachowanie przebiegu kroków.

Sprawdzono 3072 klatki i 1152 pozy chodu w rendererze gry. Obie wersje gry przeszły test startu, a test grafiki zaliczył 1487 kontroli. Render w Blenderze przez MCP trwał 41,89 s. Przechwycenie sceny z gry trwało 7,25 s. [Projekt Blender](C:/AI2D/animations/szakal-ciagle-uda-20261124/Szakal-wydmowy.blend) · [Podgląd](C:/AI2D/pipeline/runs/szakal-ciagle-uda-20261124/szakal-ciagle-uda.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-ciagle-uda-20261124/report.json)

**24.11.2026 — mocowanie tylnych nóg i amortyzacja chodu Szakala wydmowego.**

Nasady ud prowadzone są z wnętrza zadu. Sprawdzono 2304 połączenia tylnej nogi z tułowiem w 48 kierunkach i 24 fazach chodu. Najmniejszy obszar, w którym udo zachodzi pod zad, wzrósł z 37,9 do 97,1 px² w rozdzielczości sprite'u. Po postawieniu łapy biodra i barki opadają, a stawy przejmują obciążenie w różnym czasie. Kontrola 2976 póz podparcia potwierdziła zachowanie współrzędnych palców.

Sprawdzono 3072 klatki atlasów, 1152 pozy chodu w rendererze gry, trafienie atakiem i 96 nieruchomych póz po upadku. Obie wersje gry przeszły test startu, a test grafiki zaliczył 1487 kontroli. Render w Blenderze przez MCP trwał 40,87 s. Przechwycenie sceny z gry trwało 7,27 s. [Projekt Blender](C:/AI2D/animations/szakal-osadzenie-lap-20261124/Szakal-wydmowy.blend) · [Podgląd chodu](C:/AI2D/pipeline/runs/szakal-osadzenie-lap-20261124/szakal-osadzenie-lap.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-osadzenie-lap-20261124/report.json)

**24.11.2026 — zaokrąglony model Szakala wydmowego.**

Nowy obrys prowadzi ciągłe krzywizny przez barki, grzbiet, klatkę piersiową i zad. Kark łagodniej łączy się z policzkami. Zaokrąglono zakończenia pyska, uszu i łap oraz obrysy stawów. Światło na przekrojach podkreśla objętość sierści podczas chodu i po upadku.

Sprawdzono 3072 klatki w 48 kierunkach, 1152 pozy chodu w rendererze gry, trafienie atakiem i 96 nieruchomych póz ciała po kontakcie z podłożem. Obie wersje przeszły test startu, a test grafiki zaliczył 1487 kontroli. Render w Blenderze przez MCP trwał 40,81 s. Przechwycenie sceny z gry trwało 7,29 s. [Projekt Blender](C:/AI2D/animations/szakal-zaokraglony-model-20261124/Szakal-wydmowy.blend) · [Podgląd](C:/AI2D/pipeline/runs/szakal-zaokraglony-model-20261124/szakal-zaokraglony-model.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-zaokraglony-model-20261124/report.json)

**24.11.2026 — krótszy upadek i praca szyi oraz łap Szakala wydmowego.**

Kontakt z ziemią następuje po 0,31 s zamiast 0,44 s. Ten sam zegar wyznacza rozpoczęcie spoczynku i wywołanie odgłosu upadku. W trakcie ataku przednie łapy kolejno podwijają się, sięgają do przodu i opierają na podłożu, a tylne wykonują odbicie i wracają pod tułów. Szyja wygina się między nasadą w klatce a głową, która przechyla się podczas wypadu i powrotu. Pozy spoczynkowe zweryfikowano we wszystkich 48 kierunkach i na obu bokach.

Przechwycenie z gry potwierdziło trafienie ugryzieniem, kontakt z ziemią w 19. kroku przy 60 Hz i nieruchome ciało od chwili kontaktu. Sprawdzono 3072 klatki atlasów. Obie kompilacje przeszły cichy test startu, a test grafiki zaliczył 1487 kontroli. Render atlasów w Blenderze przez MCP trwał 22,94 s. Przechwycenie z gry trwało 7,39 s. [Projekt Blender](C:/AI2D/animations/szakal-szybszy-upadek-20261124/Szakal-wydmowy.blend) · [Podgląd](C:/AI2D/pipeline/runs/szakal-szybszy-upadek-20261124/szakal-szybszy-upadek.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-szybszy-upadek-20261124/report.json)

**24.11.2026 — ugryzienie, upadek i bryła ciała Szakala wydmowego.**

W zamachu szakal przenosi ciężar na tylne łapy, po czym wysuwa przednie łapy i głowę. Żuchwa otwiera się i zamyka osobno. W trakcie upadku najpierw uginają się przednie łapy, potem osiada zad i policzek opiera się przy ziemi. Po zetknięciu z podłożem gra wyświetla tę samą końcową klatkę. Klatka piersiowa zachowuje wypukły obrys i ma cień od spodu. Teksturę tułowia nadal stanowi jedno malowidło.

Sprawdzono 3072 klatki atlasów, trafienie ugryzieniem oraz 96 końcowych póz po zetknięciu z ziemią. Obie kompilacje przeszły cichy test startu, a test grafiki zaliczył 1487 kontroli. Render atlasów w Blenderze przez MCP trwał 22,33 s. Przechwycenie z gry trwało 7,30 s. [Projekt Blender](C:/AI2D/animations/szakal-atak-upadek-20261124/wydanie/Szakal-wydmowy.blend) · [Podgląd](C:/AI2D/pipeline/runs/szakal-atak-upadek-20261124/szakal-atak-upadek.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-atak-upadek-20261124/report.json)

**24.11.2026 — osadzenie ogona i praca łap Szakala wydmowego.**

Nasada ogona jest osadzona w zadzie. Końcówka zatacza łagodny łuk i podąża za biodrami z opóźnieniem. Tor łapy ma zgodną prędkość po obu stronach granicy podparcia. Podczas ugryzienia przednie łapy podążają za wypadem, a przy upadku przednie i tylne łapy uginają się w różnym czasie. Gra korzysta z 24 klatek chodu oraz po 11 klatek ugryzienia i upadku w każdym kierunku. Dla tułowia i karku zachowano jeden malunek.

Sprawdzono 2784 klatki atlasów, 1152 pozy chodu z maską GPU, trafienie ugryzieniem i 96 końcowych póz upadku. Test grafiki zaliczył 1487 kontroli, a obie kompilacje przeszły cichy test startu. Render w Blenderze przez MCP trwał 20,57 s. Przechwycenie sceny z gry trwało 7,27 s. [Projekt Blender](C:/AI2D/animations/szakal-miekki-ruch-20261124/Szakal-wydmowy.blend) · [Podgląd](C:/AI2D/pipeline/runs/szakal-miekki-ruch-20261124/szakal-miekki-ruch.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-miekki-ruch-20261124/report.json)

**24.11.2026 — jedna tekstura tułowia Szakala wydmowego.**

Przy skręcie materiał mieszał osobne malowidło boku z malowidłem piersi lub zadu. Tułów i kark mają teraz jeden malunek ze współrzędnymi prowadzonymi po ciągłej krzywej. Klatka zasłania nasadę karku pod każdym kątem. Sprawdzono 768 póz chodu i porównano alfę 2112 klatek z poprzednimi atlasami. Każdy obrys pozostał identyczny. Nowe atlasy są w obu kompilacjach.

Render w Blenderze przez MCP trwał 15,21 s. Przechwycenie sceny z gry trwało 7,28 s w ukrytym oknie i z wyłączonym dźwiękiem. Kompilacje gracza i DEV przeszły test startu. Test grafiki zaliczył 1487 kontroli. [Projekt Blender](C:/AI2D/animations/szakal-jedna-tekstura-20261124/Szakal-wydmowy.blend) · [Skręt w chodzie](C:/AI2D/pipeline/runs/szakal-jedna-tekstura-20261124/szakal-skret.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-jedna-tekstura-20261124/report.json)

**24.11.2026 — skręt Szakala wydmowego z Blendera przez MCP.**

Płaski rig 2D korzysta z malowidła Krity i wspólnego obrysu tułowia. W każdym kierunku nasady łap pozostają pod tym samym obrysem, a widok od przodu i od tyłu zachowuje tę samą grubość tułowia. Gra wybiera gotowe klatki z 48 kierunków, zachowując fazę kroku w trakcie skrętu. Ugryzienie i upadek na obie strony korzystają z tych samych części. Sprawdzono 2112 klatek eksportu, 768 póz chodu z maską z renderera i 96 niezmiennych końcowych póz ciała. W czterech półprzezroczystych próbkach próg kursora obejmuje piksel, który nie rzuca cienia.

Render atlasów trwał 15,48 s. Przechwycenie z gry trwało 7,13 s w ukrytym oknie i z wyłączonym dźwiękiem. Test grafiki zaliczył 1487 kontroli, a kompilacje gracza i DEV przeszły test startu. [Projekt Blender](C:/AI2D/animations/szakal-blender-mcp-20261124/wydanie/Szakal-wydmowy.blend) · [Podgląd](C:/AI2D/pipeline/runs/szakal-blender-mcp-20261124/szakal-blender.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-blender-mcp-20261124/report.json)

**24.11.2026 — praca tułowia szakala podczas chodu.**

Krzywe Synfiga przenoszą obciążenie z podparcia tylnych łap na podparcie przednich. Różnica pionowego położenia łopatek i bioder zmienia się w cyklu chodu w zakresie 4,64 px, wobec 0 px we wcześniejszej animacji. Największa zmiana między próbkami cyklu wyniosła 0,49 px. Kark i nasada ogona podążają za grzbietem. Tekstura klatki i zadu wchodzi w widok na odpowiednim końcu tułowia w miarę skrętu. Kontrola pełnego obrotu wykazała minimum 1,08 px odstępu między obrysami głowy i tułowia. Sprawdzono 224 połączone pozy chodu oraz 32 końcowe pozy po upadku.

Render krzywych Synfiga trwał 6,25 s. Przechwycenie sceny z gry trwało 7,23 s. Obie wersje przeszły test startu. Test grafiki zaliczył 1488 z 1488 kontroli. [Projekt Synfiga](C:/AI2D/animations/szakal-tulow-chod-20261124/ruch-szakala.sif) · [Podgląd](C:/AI2D/pipeline/runs/szakal-tulow-chod-20261124/szakal-tulow-chod.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-tulow-chod-20261124/report.json)

**24.11.2026 — połączenia części szakala podczas skrętu.**

Klatka zasłania dolną część szyi i nasady kończyn. Górne mocowanie szyi jest punktem wspólnym z głową. Każda łapa ma jeden ciągły pas tekstury, a z geometrii tułowia wynika położenie jej nasady. Kontrola pełnego obrotu w pięciu fazach wykazała co najmniej 1,47 px pionowego odstępu między obrysami głowy i tułowia. Szyja zachowała połączenie z oboma obrysami. W 224 maskach z renderera nie znaleziono odłączonych fragmentów o powierzchni co najmniej siedmiu pikseli. Sprawdzono też ugryzienie i 32 końcowe pozy upadku.

Render krzywych Synfiga trwał 6,02 s. Przechwycenie sceny z gry trwało 6,78 s. Obie kompilacje przeszły test startu, a test grafiki zaliczył 1487 z 1487 kontroli. [Master Krity](C:/AI2D/sprites/szakal-spojne-czesci-20261124/Szakal-czesci.kra) · [Podgląd](C:/AI2D/pipeline/runs/szakal-spojne-czesci-20261124/szakal-spojne-czesci.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-spojne-czesci-20261124/report.json)

**24.11.2026 — nowy malowany rig 2D szakala.**

Tułów ma jeden obrys grzbietu i brzucha rzutowany z kąta obrotu. Malowane tekstury z Krity zmieniają kolor w granicach tej samej geometrii. Synfig dostarcza krzywe kroku, podnoszenia łapy, głowy, ogona, ugryzienia i upadku. Cztery nogi mają stałe długości odcinków i wspólne punkty mocowania. W pomiarze co 3° maksymalna zmiana szerokości wyniosła 2,03 px na tylnym półobrocie i 1,65 px na przednim. W próbach nie wystąpiło chwilowe odwrócenie kierunku zmiany szerokości. Sprawdzono 224 pozy z cieniem i maską kursora, trafienie ugryzieniem i 32 pozy końcowe po upadku.

Render 61 klatek Synfiga trwał 6,03 s. Przechwycenie sceny z gry trwało 6,90 s. Obie wersje przeszły test startu. Test grafiki zaliczył 1485 z 1485 kontroli. [Projekt Synfiga](C:/AI2D/animations/szakal-rig-2d-final-20261124/ruch-szakala.sif) · [Master Krity](C:/AI2D/sprites/szakal-rig-2d-20261124/Czesci-szakala-final.kra) · [Podgląd](C:/AI2D/pipeline/runs/szakal-rig-2d-20261124/gotowe/szakal-nowy-rig.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-rig-2d-20261124/gotowe/report.json)

**24.11.2026 — płynna szerokość szakala przy skręcie.**

Pas tułowia jest dopasowany do gładkiej krzywej szerokości na całym obrocie. Gdy biodro i łopatka zaczynają się nakładać, ich odległość przestaje rozciągać tylny obrys. Porównanie renderów wykazało zwężenie pasa tułowia od tyłu z 22,71 do 20,48 px. Największa zmiana w jednym kroku o 3° na tylnym półobrocie spadła z 2,84 do 2,39 px, a wcześniejsze ponowne poszerzenie o 1,11 px zniknęło w sprawdzonych fazach. Test regresji mierzy pełny obrót w spoczynku i w czterech fazach chodu.

Przechwycenie sceny trwało 6,84 s. Obie wersje przeszły test startu. Test grafiki zaliczył 1484 z 1484 kontroli. [Podgląd](C:/AI2D/pipeline/runs/szakal-plynny-tylny-obrys-20261124/odbior/szakal-plynny-tylny-obrys.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-plynny-tylny-obrys-20261124/odbior/report.json) · [Test grafiki](C:/AI2D/pipeline/runs/szakal-plynny-tylny-obrys-20261124/graphics-tests.txt)

**24.11.2026 — proporcje tułowia szakala podczas skrętu.**

W 64 kątach i 29 pozach renderer dopasowuje odcinek od biodra do łopatki do rzutu długości profilu. Głębokość klatki jest mierzona osobno na pionowych przekrojach tułowia. W porównaniu renderów błąd RMS długości spadł z 3,59 do 0,41 px, a głębokości z 3,00 do 0,58 px. W żadnej z 1856 skorygowanych póz przerwa między nasadą łapy a obrysem tułowia nie przekroczyła 2 px. Pomiar maski wskazania wykazał jedną rozbieżność w 135 650 próbkach pokrycia. Sprawdzono granice kierunków, ugryzienie i 32 końcowe pozy ciała.

Przechwycenie sceny trwało 6,68 s. Obie kompilacje przeszły test startu, a test grafiki zaliczył 1480 z 1480 kontroli. [Podgląd](C:/AI2D/pipeline/runs/szakal-proporcje-tulowia-20261123/odbior/szakal-proporcje-tulowia.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-proporcje-tulowia-20261123/odbior/report.json)

**23.11.2026 — stała skala szakala przy zmianie kierunku.**

Kalibracja wyrównuje położenie tułowia, wysokość sylwetki i podparcie łap w tej samej fazie kroku. W 1408 próbach renderera różnica wysokości środka tułowia między kierunkami spadła z 6,64 do 0,01 px. Maksymalne odchylenie pola obrysu od krzywej projekcji spadło z 22,92% do 14,76%. Szerokość bioder od tyłu wynosi średnio 22,69 px podczas chodu. Renderer odczytuje też starszą kalibrację i zachowuje kotwicę martwego ciała.

Przechwycenie sceny trwało 6,66 s. Obie kompilacje przeszły test startu, a test grafiki zaliczył 1480 z 1480 kontroli. [Podgląd](C:/AI2D/pipeline/runs/szakal-stala-skala-20261123/gotowe/szakal-stala-skala.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-stala-skala-20261123/gotowe/report.json)

**23.11.2026 — sylwetka szakala od tyłu.**

Korekta szerokości zwęża tylny obrys i płynnie zanika przy skręcie do profilu. W spoczynku średnia szerokość w pasie bioder od tyłu zmalała z 32,44 do 22,81 px. Podczas chodu różnica wysokości między kierunkami w tej samej fazie kroku wynosiła najwyżej 2 px w 1408 próbach renderera. Sprawdzono maskę wskazania, ugryzienie i 32 końcowe pozy po upadku.

Przechwycenie sceny z gry trwało 7,28 s. Testy startu obu wersji oraz 1480 kontroli grafiki zakończyły się powodzeniem. [Podgląd](C:/AI2D/pipeline/runs/szakal-smuklejszy-tyl-20261123/szakal-smuklejszy-tyl.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-smuklejszy-tyl-20261123/report.json)

**23.11.2026 — rozmiar szakala podczas skrętu.**

Kalibracja 64 kątów i 29 póz wyrównuje wysokość przy zachowaniu skrótu perspektywicznego. Osobne przekształcenia górnej i dolnej części sylwetki ograniczają wpływ wysuwanej łapy na skalę tułowia. W 1408 próbach renderera największa różnica wysokości przy tej samej fazie kroku spadła z 15 do 2 px. Różnica szerokości tułowia między widokiem od przodu i od tyłu spadła z 7 do 2 px. Ponownie sprawdzono granice kierunków, maskę kursora i 32 końcowe pozy ciała.

Przechwycenie sceny z gry trwało 6,81 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-staly-rozmiar-20261123/szakal-staly-rozmiar.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-staly-rozmiar-20261123/report.json)

**23.11.2026 — ciągły skręt Szakala wydmowego.**

Synfig wyeksportował tułów z ogonem osobno od kończyn. Lokalny model FILM z ComfyUI przygotował pola ruchu dla 1184 par póz. Renderer interpoluje tułów po polu ruchu i obrysie, a każdą z czterech łap prowadzi po wspólnych położeniach stawów. W grze kierunek wynika z ciągłego kąta obrotu na mapie. W 224 pozach pośrednich obrys wskazywania odpowiadał renderowi. W 32 parach przy granicach dawnych widoków największa różnica powyżej progu 22/255 objęła 14 pikseli. Po ugryzieniu i upadku sprawdzono wszystkie kierunki i 32 pozycje ciała.

Render Synfiga trwał 20,59 s, wyznaczenie pól FILM 16,10 s, przechwycenie sceny z gry 6,93 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-plynny-skret-20261123/szakal-plynny-skret.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-plynny-skret-20261123/report.json)

**23.11.2026 — 16 kierunków chodu Szakala wydmowego.**

Cztery nowe widoki z GPT Image po retuszu w Kricie dają osiem dodatkowych kierunków z odbiciami. Projekcja kątów i próg przełączania działają w płaszczyźnie mapy. Synfig wyrenderował 296 dodatkowych klatek dla chodu, spoczynku, ugryzienia i upadku. W spoczynku największa różnica szerokości między sąsiednimi widokami spadła z 46 do 25 pikseli atlasu. Sprawdzono 666 połączonych klatek z atlasów, 224 pozy z GPU i 32 końcowe pozy ciała. Gra pokazała wszystkie 16 kierunków w jednym okrążeniu. [Arkusz i polecenie](../desert/new-enemies/jackal-intermediate-prompt.md)

Render Synfiga trwał 17,76 s. Przechwycenie sceny z gry trwało 6,75 s. Obie wersje przeszły test uruchomienia. Test grafiki zakończył się wynikiem 1480/1480. [Podgląd](C:/AI2D/pipeline/runs/szakal-16-kierunkow-20261123/szakal-skret.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-16-kierunkow-20261123/report.json)

**23.11.2026 — klatkowy chód Szakala wydmowego.**

Synfig składa tułów, cztery niezależne łapy i ogon z istniejącej ilustracji Krity. Gra wybiera pojedynczą klatkę w stałej skali. Próg zmiany widoku stabilizuje kierunek podczas skrętu. Zapisano 370 klatek dla pięciu narysowanych kierunków i trzech odbić w grze. Wszystkie klatki mają połączoną sylwetkę. Górny obrys przy chodzie w danym kierunku przesuwa się najwyżej o trzy piksele atlasu. W 112 ujęciach z GPU maska kursora zgadzała się z renderem. Sprawdzono trafienie i unieruchomienie 16 końcowych póz po upadku.

Render Synfiga trwał 21,76 s. Przechwycenie scen z gry trwało 12,20 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-klatki-20261123/szakal-chod.webp) · [Pomiary](C:/AI2D/pipeline/runs/szakal-klatki-20261123/report.json)

**23.11.2026 — anatomia chodu szakala.**

Osobne krzywe Synfiga sterują łokciem i nadgarstkiem przedniej łapy oraz kolanem i stawem skokowym tylnej. Szakal stawia tylną łapę przed przednią po tej samej stronie. Ruch nasad i stawów przenoszą segmenty wspólnej siatki malowidła. Próby chodu objęły osiem kierunków i 480 próbek cyklu. Z przodu i z tyłu największy poprzeczny ruch czubka łapy wyniósł 1,11 px, a najmniejszy rozstaw łap w parze 21,53 px. W 32 maskach sylwetka zachowała ciągłość. Sprawdzono także ugryzienie i 16 końcowych póz po upadku. Test grafiki gry zakończył się wynikiem 1480/1480. [Materiał referencyjny University of Minnesota](https://vanat.ahc.umn.edu/mGaits/)

Synfig wyrenderował 61 próbek w 13,15 s. Przechwycenie obu ujęć w grze trwało 11,67 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-anatomia-chodu-20261123/szakal-chod.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-anatomia-chodu-20261123/report.json)

**23.11.2026 — płynniejszy chód i skręt szakala.**

Krzywe Synfiga prowadzą dłuższy cykl kroku, łagodne oderwanie i postawienie łapy oraz przenoszenie ciężaru z ruchem głowy i ogona. Lokalna korekta siatki zachowuje połączenie nóg i tułowia. Dopasowanie punktów głowy i podparcia między sąsiednimi widokami obniżyło średni przeskok mierzonego punktu głowy przy zmianie widoku z 12,27 do 0,72 px.

Sprawdzono 128 póz, kontakt z podłożem po upadku, trafienie ugryzieniem oraz 32 połączone maski sylwetki. Synfig wyrenderował 61 próbek w 11,56 s. Przechwycenie scen w grze trwało 11,52 s. [Podgląd chodu](C:/AI2D/pipeline/runs/szakal-plynny-chod-20261123/szakal-plynny-chod.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-plynny-chod-20261123/report.json)

**23.11.2026 — nowa ilustracja i animacje szakala.**

GPT Image przygotował nową ilustrację z puszystym ogonem, ciemnym grzbietem i fioletową chustą. Retusz w Kricie zachował alfę sylwetek. Łapy wyginają się wraz z malowidłem na wspólnej siatce. Nasady pozostają w tułowiu. Przy zmianie kierunku silnik prowadzi szerokość i środek sylwetki między widokami. Krzywe Synfiga prowadzą podparcie łap, ugryzienie i upadek na bok.

W 32 maskach z gry cała sylwetka pozostała jednym połączonym obszarem. Sprawdzono 128 póz we wszystkich kierunkach, ugryzienie i 16 końcowych póz po upadku. Synfig wyrenderował 61 próbek w 11,18 s. Przechwycenie sceny z gry trwało 6,09 s. [Grafika źródłowa i polecenie](../desert/new-enemies/jackal-repaint-prompt.md) · [Podgląd z gry](C:/AI2D/pipeline/runs/szakal-od-nowa-20261123/szakal-od-nowa.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-od-nowa-20261123/report.json)

**23.11.2026 — animowany rysunek 2D szakala.**

Szakal korzysta z rysunku o wspólnych punktach mocowania łap, grzbietu, ogona i głowy. Silnik wylicza położenie elementów z kierunku, fazy chodu i krzywych Synfiga. Sierść oraz chusta pobierają teksturę ze stałych miejsc atlasu z Krity. Filtrowanie drobnych włosów zależy od skali na ekranie. Ten sam rysunek obsługuje ugryzienie, upadek, ciało, cień i wybór kursorem.

W 128 próbkach skrętu renderer używał nowego rysunku. Wszystkie 16 sprawdzonych masek miało połączoną sylwetkę przy tolerancji 2 px. Sprawdzono rzeczywiste trafienie i 16 końcowych póz ciała w ośmiu kierunkach. Synfig wyrenderował 61 próbek w 11,10 s. Przechwycenie klatek z Morza Piasków trwało 6,14 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-rysowana-animacja-20261123/szakal-nowy-chod.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-rysowana-animacja-20261123/report.json)

**23.11.2026 — stabilna sylwetka szakala w chodzie.**

W 64 widokach wyrównano wysokość i dopasowano szerokość do kąta patrzenia. Ogon w tylnym skręcie ma osobne dopasowanie do tułowia. W 160-pikselowych klatkach zakres wysokości sylwetki spadł z 18 do 4 px, a największy skok dolnego obrysu w tylnym skręcie z 23 do 1 px. Podczas chodu stawy prowadzą łapy bez zmiany długości nóg, a krzywe Synfiga ograniczają kołysanie grzbietu i głowy w rytmie kroku.

Sprawdzono prosty chód, chód przez cztery kierunki, trafienie oraz kontakt ciała z ziemią. Synfig wyrenderował 61 próbek w 9,96 s. Przechwycenie dwóch sekwencji na Morzu Piasków trwało 5,81 s. [Podgląd](C:/AI2D/pipeline/runs/szakal-stabilny-chod-20261123/szakal-chod.webp) · [Raport](C:/AI2D/pipeline/runs/szakal-stabilny-chod-20261123/report.json)

**23.11.2026 — płynniejszy ruch szakala i żniwiarza.**

64 widoki na gatunek zastępują mieszanie sąsiednich sylwetek w grze. Geometria części zmienia się nadal z kątem obrotu. Krzywe Synfiga prowadzą osobne fazy podparcia łap szakala, wybicie do ugryzienia i amortyzację. U żniwiarza krok i zamach angażują kolana, obrót tułowia i całą rękę prowadzącą sierp. Po śmiertelnym ciosie kończyny uginają się przed przechyłem i oparciem ciała o grunt.

Sprawdzono wybór klatki i maskę kursora w 128 próbkach skrętu na gatunek, obrażenia zadawane w rzeczywistej walce i 32 końcowe pozy ciał. Synfig wyrenderował 61 próbek w 10,10 s. Render, odczyt i zapis 188 klatek z Morza Piasków trwał 4,97 s. [Podgląd z gry](C:/AI2D/pipeline/runs/pustynne-bestie-plynny-ruch-20261123/szakal-i-zniwiarz.webp) · [Raport](C:/AI2D/pipeline/runs/pustynne-bestie-plynny-ruch-20261123/report.json)

**23.11.2026 — chód, ataki i upadki pustynnych bestii.**

Przy skręcie Żniwiarz zachowuje pełny napierśnik pod unoszącym się ramieniem. Sierp ma osobny atlas z przezroczystym tłem i porusza się razem z dłonią przy zgięciu łokcia. Usunięto dawną smugę cięcia. Szakal stawia łapy z przesunięciem fazy, a jego głowa pozostaje połączona z karkiem podczas skoku i obrotu. W upadku zwierzę podkurcza łapy. Żniwiarz zgina kończyny i opiera broń przy ciele. Oba tułowie zachowują grubość, a zetknięcie z podłożem wyznacza obrys grafiki.

Sprawdzono osiem kierunków każdego gatunku, rzeczywiste trafienia i wybór ciała kursorem. W 64 próbkach ataku malowane części tworzyły po jednym połączonym obrysie przy tolerancji dwóch pikseli na krawędź. Synfig wyrenderował 61 klatek krzywych w 8,44 s. Przechwycenie 1184 klatek z gry wraz z symulacją i zapisem trwało 14,80 s. Testy startu wersji gracza i DEV wykonano w ukrytym oknie i z wyłączonym dźwiękiem. [Animowany podgląd](C:/AI2D/pipeline/runs/pustynne-bestie-ruch-20261123/pustynne-bestie-osiem-stron.webp) · [Pomiary](C:/AI2D/pipeline/runs/pustynne-bestie-ruch-20261123/report.json) · [Źródła](../desert/new-enemies/README.md)

**23.11.2026 — dwa nowe gatunki na pustyni.**

Szakal wydmowy przywiera do ziemi przed krótkim skokiem do ugryzienia. Szklany żniwiarz cofa zakrzywiony sierp, przenosi ciężar i wyprowadza szerokie cięcie. Każdy gatunek ma 32 widoki, osobne klucze ruchu i jeden atlas. W tle zarejestrowano chód ze zmianą kierunku, trafienia oraz upadki na piasku. Testy potwierdziły wystąpienie obu gatunków w każdym z sześciu sektorów oraz zachowanie pokonanych przeciwników przy migracji starszych zapisów i ponownym wczytaniu.

Synfig wyrenderował 61 próbek krzywych w 13,90 s. Przechwycenie 149 klatek z gry wraz z symulacją i zapisem trwało 10,66 s przy ukrytym oknie i wyłączonym audio. [Źródła i integracja](../desert/new-enemies/README.md) · [Podgląd z gry](C:/AI2D/pipeline/runs/pustynne-bestie-20261123/pustynne-bestie.webp) · [Wynik przechwycenia](C:/AI2D/pipeline/runs/pustynne-bestie-20261123/capture-receipt.json)

**23.11.2026 — celowanie pięścią Varkhora.**

Varkhor wybiera ramię przy rozpoczęciu zamachu. Tor pięści uwzględnia położenie bohatera z dokładnością do jego rzeczywistego kierunku i wysokości podłoża. W trakcie odtwarzania końca ciosu ramię zachowuje ostatni cel. Zapis w wersji 32 przechowuje ramię i cel, a zapis w wersji 31 nadal odtwarza stan walki. Sprawdzono trafienie, tor pięści i zachowanie celu po ciosie w ośmiu kierunkach. Przechwycenie czterech ujęć z Kaldery wraz z symulacją prób trwało 5,40 s. [Podgląd](C:/AI2D/pipeline/runs/varkhor-kierunek-ciosu-20261123/varkhor-kierunki-ciosu.webp) · [Raport](C:/AI2D/pipeline/runs/varkhor-kierunek-ciosu-20261123/report.json)

**23.11.2026 — boczne uniesienie ramienia Varkhora.**

Podczas zamachu łokieć odchodzi od tułowia i unosi się niemal na wysokość barku. Przy ciosie na wprost bark obraca się w kierunku środka sylwetki, a pięść trafia w górną część sylwetki bohatera. Oba odcinki ręki zachowują długość. Sprawdzono osiem kierunków oraz zarejestrowano cios od przodu i pod kątem. Render 61 klatek w Synfigu trwał 6,05 s. Przechwycenie dwóch ujęć z Kaldery trwało 2,61 s. [Projekt Synfiga](C:/AI2D/animations/varkhor-lokiec-w-bok-20261123/ruch/strike.sif) · [Podgląd z gry](C:/AI2D/pipeline/runs/varkhor-lokiec-w-bok-20261123/varkhor-lokiec-w-bok.webp) · [Raport](C:/AI2D/pipeline/runs/varkhor-lokiec-w-bok-20261123/report.json)

**23.11.2026 — wspólny ruch kończyny w ciosie Varkhora.**

Tor pięści z Synfiga prowadzi ramię i przedramię o stałych długościach. Łokieć pozostaje po dolnej stronie tego toru, a bark przesuwa się wraz z tułowiem. W ośmiu kierunkach sprawdzono, że pięść nie cofa się w fazie uderzenia, a przy trafieniu łokieć pozostaje lekko zgięty. Render 61 klatek w Synfigu trwał 6,09 s. Przechwycenie 37 klatek z Kaldery trwało 1,44 s. [Projekt Synfiga](C:/AI2D/animations/varkhor-naturalny-cios-20261123/ruch/strike.sif) · [Podgląd z gry](C:/AI2D/pipeline/runs/varkhor-naturalny-cios-20261123/varkhor-naturalny-cios.webp) · [Raport](C:/AI2D/pipeline/runs/varkhor-naturalny-cios-20261123/report.json)

**23.11.2026 — ruch barku w ciosie Varkhora.**

Bark i całe ramię cofają się w zamachu. Bark rusza do przodu, ramię obraca się w stronę celu, a łokieć osiąga wyprost przy trafieniu. Obrót ramienia między końcem zamachu a trafieniem wyniósł co najmniej 51° w ośmiu kierunkach. Render 61 klatek ciosu w Synfigu trwał 6,17 s, a przechwycenie 37 klatek w Kalderze 1,46 s. [Projekt Synfiga](C:/AI2D/animations/varkhor-ruch-barku-20261123/ruch/strike.sif) · [Podgląd z gry](C:/AI2D/pipeline/runs/varkhor-ruch-barku-20261123/varkhor-ruch-barku.webp) · [Raport](C:/AI2D/pipeline/runs/varkhor-ruch-barku-20261123/report.json)

**23.11.2026 — cios Varkhora z łokcia.**

Ramię i przedramię mają osobne osie obrotu. Łokieć zgina się w zamachu i prostuje w kierunku bohatera w momencie trafienia. Przetestowano obrys oraz wykrywanie kursora w czterech fazach ciosu i ośmiu kierunkach. Render 61 klatek ciosu w Synfigu trwał 5,94 s, a przechwycenie 37 klatek w Kalderze 1,56 s. [Projekt Synfiga](C:/AI2D/animations/varkhor-ciosem-z-lokcia-20261123/ruch/strike.sif) · [Podgląd z gry](C:/AI2D/pipeline/runs/varkhor-ciosem-z-lokcia-20261123/varkhor-ciosem-z-lokcia.webp) · [Raport](C:/AI2D/pipeline/runs/varkhor-ciosem-z-lokcia-20261123/report.json)

**23.11.2026 — pancerz i walka Varkhora.**

Atlas Varkhora zachowuje obrys wszystkich części. Pory, rysy i żar w pancerzu pochodzą z tekstur Sunburst przetworzonych lokalnie przez Real-ESRGAN x2plus w ComfyUI i retuszowanych w Kricie. Klucze Synfiga prowadzą przenoszenie ciężaru w chodzie, zamach pięścią i gest erupcji. Ostrzeżenia na podłożu używają promieni trafienia z gry. Sprawdzono uderzenie i kolejność zapłonu trzech szczelin w Kalderze. Synfig wyrenderował trzy przebiegi po 61 klatek w 17,51 s. Przechwycenie 107 klatek z gry trwało 5,33 s. [Projekt Krity](C:/AI2D/sprites/varkhor-odnowa-20261122/varkhor-pancerz.kra) · [Projekty Synfiga](C:/AI2D/animations/varkhor-odnowa-20261122/ruch) · [Podgląd z gry](C:/AI2D/pipeline/runs/varkhor-odnowa-20261122/varkhor-kaldera.webp) · [Raport](C:/AI2D/pipeline/runs/varkhor-odnowa-20261122/report.json)

**22.11.2026 — model Salamandry lawowej.**

Salamandra ma wydłużony pysk, wyraźniejsze łapy, grzbiet z dwoma rzędami płyt i zwężający się ogon. Materiały z Sunburst wyostrzono lokalnie w ComfyUI przez Real-ESRGAN x2plus. Kontrast łusek poprawiono na osobnej warstwie w Kricie. Chód, atak i upadek korzystają z tych samych powierzchni. W 36 kierunkach sprawdzono zgodność obrysu z wykrywaniem kursora. Render i zapis 71 klatek z Kaldery trwał 1,49 s. [Materiały w Kricie](C:/AI2D/sprites/kaldera-salamandra-model-20261122/salamandra-materialy.kra) · [Podgląd](C:/AI2D/pipeline/runs/kaldera-salamandra-model-20261122/salamandra-lawowa.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-salamandra-model-20261122/report.json)

**22.11.2026 — ogon Rogacza Żaru.**

Krótki ogon z sierścią jest osadzony w zadzie. W Synfigu dodano osobny przebieg wychylenia końcówki w rytmie chodu. Podczas upadku ogon obraca się z tułowiem i pozostaje widoczny na ciele. W 60 klatkach skrętu sylwetka pozostała połączona, a kontrola przesłaniania objęła 216 ujęć. Render 61 klatek w Synfigu trwał 2,85 s. Odczyt i zapis 66 klatek chodu z Kaldery trwał 3,37 s, a 18 klatek upadku 0,45 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-ogon-20261122/rogacz-ogon.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-ogon-20261122/report.json)

**22.11.2026 — chód Rogacza Żaru.**

W Synfigu ustawiono dłuższy krok, osobne wykresy przednich i tylnych kopyt oraz pochylenie racic w trakcie wymachu. Kolejne kopyta przejmują ciężar, a barki, zad i kark płynnie podążają za krokiem. W 1984 pomiarach prostego chodu w ośmiu kierunkach kopyto pozostawało przy punkcie oparcia. Sprawdzono też przesłanianie w 216 ujęciach. Synfig wyrenderował 61 próbek w 3,04 s, a odczyt i zapis 66 klatek z Kaldery trwał 3,43 s. [Projekt Synfiga](../volcano/cinderhorn/motion/rogacz-chod.sif) · [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-naturalny-chod-20261122/rogacz-chod.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-naturalny-chod-20261122/report.json)

**22.11.2026 — sierść na nogach Rogacza Żaru.**

Krótsze włosy na przednich i tylnych nogach są osadzone na powierzchni nad racicami. Podążają za zgięciem nóg i są przesłaniane podczas skrętu oraz upadku. Sprawdzono kolejność powierzchni w 216 ujęciach i ciągłość 60 klatek skrętu. Odczyt i zapis klatek z Kaldery podczas chodu trwał 3,40 s, a podczas upadku 0,42 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-futro-nog-20261122/rogacz-futro-nog.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-futro-nog-20261122/report.json)

**22.11.2026 — upadek i ciało Rogacza Żaru.**

Rogacz przewraca się od strony trafienia. Krzywe Synfiga prowadzą zgięcie przednich i tylnych nóg, przechył i oparcie boku o podłoże. Sierść, rogi i tułów pochodzą z tego samego modelu w walce, podczas upadku i po kontakcie z ziemią. Pozycja ciała, cień, obrys kursora i miniatura korzystają z końcowej pozy. [Projekt Synfiga](../volcano/cinderhorn/motion/rogacz-upadek.sif)

Sprawdzono osiem kierunków i oba boki upadku. Od kontaktu z ziemią klatki ciała i obrysu pokryły się piksel w piksel. Synfig wyrenderował 61 klatek w 2,97 s. Render podglądu z Kaldery wraz z odczytem i zapisem trwał 0,42 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-martwe-cialo-20261122/rogacz-upadek.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-martwe-cialo-20261122/report.json)

**22.11.2026 — proporcje tułowia Rogacza Żaru.**

Klatka piersiowa ma głębszy profil przy przednich nogach. Lędźwie są węższe, brzuch unosi się przed tylnymi nogami, a grzbiet opada ku zadowi. Mocowania nóg dopasowano do tych przekrojów. W przednim rzucie dopasowano również rozmiar ciała po upadku. Próbka klatek skrętu potwierdziła ciągłość sylwetki i przesłanianie w 216 ujęciach. Render 66 klatek z Kaldery wraz z odczytem i zapisem trwał 3,40 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-proporcje-20261122/rogacz-zaru.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-proporcje-20261122/report.json)

**22.11.2026 — pojedyncze włosy w sierści Rogacza Żaru.**

Matową sierść z Sunburst dopasowano do atlasu w lokalnym ComfyUI. W Kricie domalowano pojedyncze włosy i ich zwężające się końcówki. Dłuższe pasma na bokach, grzbiecie oraz karku są osadzone na obrysie i poruszają się wraz ze skórą. W 60 klatkach skrętu włosy były widoczne poza poprzednim obrysem, a sylwetka pozostała połączona. Ponownie sprawdzono przesłanianie w 216 ujęciach. Render 66 klatek z Kaldery wraz z odczytem i zapisem trwał 3,36 s. [Projekt Krity](C:/AI2D/sprites/kaldera-rogacz-wlosy-20261122/rogacz-wlosy.kra) · [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-wlosy-20261122/rogacz-zaru.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-wlosy-20261122/report.json)

**22.11.2026 — obrys tułowia, głowy i nóg Rogacza Żaru.**

Krzywe przekrojów prowadzą płynny obrys bioder, grzbietu, barków, karku i pyska. Nos i uszy mają łagodniejsze krawędzie. Nogi zwężają się ku racicom, a zaokrąglenia wokół kolan zachowują punkty podparcia. W 216 ujęciach sprawdzono przesłanianie części. W 60 klatkach skrętu sylwetka pozostała połączona. Render 66 klatek z Kaldery z odczytem i zapisem trwał 3,39 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-sylwetka-20261122/rogacz-zaru.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-sylwetka-20261122/report.json)

**22.11.2026 — przesłanianie rogów i nóg Rogacza Żaru.**

Każdy fragment rogu, kończyny i tułowia ma własną głębię w rzucie 2D. Nasady rogów wychodzą z czaszki na boki, a bliższe powierzchnie zasłaniają dalsze podczas skrętu. Porównano 216 ujęć chodu, przygotowania do szarży i biegu z odwróconą kolejnością przekazywania trójkątów do renderera. Żaden piksel nie przekroczył progu różnicy używanego w teście. Render 66 klatek z Kaldery wraz z odczytem i zapisem trwał 3,37 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-rogacz-przeslanianie-20261122/rogacz-zaru.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-przeslanianie-20261122/report.json)

**22.11.2026 — mocowania kończyn, głowa i ostrość sierści Rogacza Żaru.**

Nogi są osadzone wewnątrz barków i bioder. Tułów, szyja i głowa korzystają z jednego obrysu. Nasada czaszki wyznacza obrót głowy i ugięcie szyi. W lokalnym ComfyUI wyostrzono pasma istniejącej tekstury, a w Kricie dopracowano końce kępek. Synfig wyrenderował 61 próbek nowych krzywych w 3,07 s. [Projekt Krity](C:/AI2D/sprites/kaldera-rogacz-stawy-20261122/rogacz-stawy.kra) · [Projekt Synfiga](../volcano/cinderhorn/motion/rogacz-chod.sif)

Sprawdzono 8640 mocowań w różnych fazach chodu i szarży. Wszystkie 60 sprawdzonych sylwetek podczas skrętu pozostało połączonych. Średni kontrast sąsiednich pikseli w próbce futra na boku wzrósł z 10,8 do 20,3. Render 66 klatek z Kaldery wraz z odczytem i zapisem trwał 3,46 s. [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-rogacz-stawy-20261122/rogacz-zaru.webp) · [Wyniki i czasy renderu](C:/AI2D/pipeline/runs/kaldera-rogacz-stawy-20261122/report.json)

**22.11.2026 — sierść i ruch Rogacza Żaru.**

Kępki sierści z Sunburst wyostrzono lokalnie w ComfyUI i domalowano w Kricie. Zachowano wymiary atlasu 1024 × 512 px. Wspólne przekroje barków i karku prowadzą futro od grzbietu do głowy oraz uginają się podczas przygotowania do szarży. Klucze Synfiga rozdzielają podparcie i wymach przednich i tylnych nóg. [Projekt Krity](C:/AI2D/sprites/kaldera-rogacz-futro-20261122/rogacz-futro.kra) · [Projekt Synfiga](../volcano/cinderhorn/motion/rogacz-chod.sif)

Sprawdzono po 360 kątów w spoczynku, chodzie i przygotowaniu do szarży. W obu profilach szerokość wyniosła 120 px, a wysokość w spoczynku 117–120 px. Synfig wyrenderował 61 próbek w 3,13 s. Podgląd 66 klatek z gry powstał w 3,46 s wraz z odczytem i zapisem. [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-rogacz-futro-20261122/rogacz-zaru.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-rogacz-futro-20261122/report.json)

**22.11.2026 — profile skrzydeł Żarowego Draka i wspólny model Rogacza Żaru.**

Przebieg błony i grubsza krawędź skrzydeł draka eksponują większą powierzchnię od przodu i z boku. W porównaniu klatek pole widocznego pigmentu błony wzrosło z 472 do 1053 px od przodu oraz z 881 do 1203 px w prawym profilu i z 843 do 1158 px w lewym profilu.

Rogacz korzysta ze wspólnego szkieletu 2D i jednego zestawu proporcji podczas obrotu. W obu profilach ma 120 px szerokości na klatce testowej, a wysokość w spoczynku mieści się w zakresie 117–120 px. Malowanie bazaltu, sierści i rogów przygotowano w Sunburst, ComfyUI i Kricie. Render podglądu z odczytem i zapisem 66 klatek trwał 3,43 s. [Projekt Krity](C:/AI2D/sprites/kaldera-profile-20261122/rogacz-materialy-final.kra) · [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-profile-20261122/drak-i-rogacz.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-profile-20261122/report.json)

**22.11.2026 — objętość skrzydeł draka i ostrość skrętów rogacza.**

Błony Żarowego Draka są wypukłe i mają ciemniejszy spód przy krawędzi. Nasada, przedramię i żebra skrzydła korzystają z grubszych przekrojów. W 24 ujęciach sylwetka pozostała spójna.

Rogacz Żaru korzysta z dopasowanych konturów i pojedynczej próbki pigmentu na piksel. W próbie chodu z obrotem co 1° średni udział pikseli o częściowym kryciu spadł z 14,6% do 7,9%, a największa zmiana obrysu na krok z 6,7% do 5,0%. Render podglądu obu stworzeń z odczytem i zapisem 66 klatek trwał 3,54 s. [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-skrzydla-rogacz-20261122/drak-i-rogacz.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-skrzydla-rogacz-20261122/report.json)

**21.11.2026 — chód i ostrość łusek Żarowego Draka.**

Oddzielne krzywe przednich i tylnych łap prowadzą podparcie, ugięcie i przestawienie łap. Krok jest powiązany z dystansem na podłożu, a tułów, skrzydła i ogon przenoszą ciężar z przesunięciem fazy. Sprawdzono podparcie wszystkich łap w ośmiu kierunkach. Synfig wyrenderował 61 klatek kontrolnych w 3,45 s. [Projekt Synfiga](../volcano/quadruped-rigs/drake-ruch.sif)

Łuski tułowia i szyi mają indywidualne krawędzie oraz matowe powierzchnie. Pole 512 × 512 w dotychczasowym atlasie przygotowano w Sunburst, ComfyUI i Kricie. Render podglądu z gry z odczytem i zapisem 66 klatek trwał 3,47 s. [Projekt Krity](C:/AI2D/sprites/kaldera-drak-krok-20261121/drak-luski.kra) · [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-drak-krok-20261121/zarowy-drak.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-drak-krok-20261121/report.json)

**21.11.2026 — połączenie szyi i klatki Żarowego Draka.**

Nasada szyi zwęża się od klatki ku głowie, a gardło łagodnie wznosi się ku żuchwie. Szyja i tułów korzystają ze wspólnych przekrojów, współrzędnych tekstury i cieniowania. Zachowano atlas HD oraz kotwice głowy i łap.

W 24 ujęciach sylwetka pozostała spójna. Render podglądu z odczytem i zapisem 66 klatek trwał 3,68 s. [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-drak-szyja-20261121/zarowy-drak.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-drak-szyja-20261121/report.json)

**21.11.2026 — malowanie Żarowego Draka w wyższej rozdzielczości.**

Drak używa atlasu RGBA8 2048 × 1024 z polami 512 × 512 i filtrowaniem anizotropowym. Łuski klatki i pyska, skórę łap oraz błony skrzydeł przygotowano w Sunburst, przetworzono w lokalnym ComfyUI i wykończono w Kricie. W grze podkreślono oko i nozdrze. [Projekt Krity](C:/AI2D/sprites/kaldera-drak-hd-20261121/drak-hd.kra) · [Prompt](C:/AI2D/pipeline/runs/kaldera-drak-hd-20261121/prompt-sunburst.txt)

Kontury draka pokryły się w 24 kierunkach, a 90 porównanych ujęć pełzacza pokryło się piksel w piksel. Render podglądu z odczytem i zapisem 66 klatek trwał 3,47 s. [Podgląd z gry](C:/AI2D/pipeline/runs/kaldera-drak-hd-20261121/zarowy-drak.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-drak-hd-20261121/report.json)

**21.11.2026 — proporcje tułowia draka i pełzacza.**

Profil draka ma szerszą klatkę piersiową, wcięcie brzucha i osobną linię bioder. Pełzacz ma wydłużony, spłaszczony pancerz z zachodzącymi na siebie płytami i podwiniętym podbrzuszem. Przekroje są osadzone na istniejących kotwicach ruchu. Zachowano dotychczasowe malowanie atlasu.

W 48 próbkach z renderera sylwetki pozostały spójne, a ślad pozycji i kierunku chodu obu stworzeń pokrył się z poprzednią wersją. Render z odczytem i zapisem 66 klatek trwał 3,36 s. [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-tulowie-20261121/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-tulowie-20261121/report.json)

**21.11.2026 — nowe sylwetki i malowanie draka oraz pełzacza.**

Drak ma dłuższy pysk ze szczęką i osobnymi zębami, zwężający się ogon oraz skrzydła z łukowatymi wcięciami między żebrami. Pełzacz ma niższą, szerszą skorupę z nieregularnymi płytami i wyraźnym podbrzuszem. Osiem malowanych powierzchni z Sunburst przeskalowano lokalnie w ComfyUI i dopracowano w Kricie. [Projekt Krity](C:/AI2D/sprites/kaldera-anatomia-czworonogow-20261121/kaldera-anatomia.kra)

W 48 ujęciach z renderera nie wykryto oderwanych części sylwetki. Ślad ruchu obu stworzeń pokrywa się z poprzednią wersją. Render podglądu z odczytem i zapisem 66 klatek trwał 3,43 s. [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-anatomia-czworonogow-20261121/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-anatomia-czworonogow-20261121/report.json)

**21.11.2026 — nowe malowanie draka i pełzacza.**

Sunburst przygotował osiem malowanych powierzchni. Lokalny ComfyUI przetworzył je modelem Real-ESRGAN x2plus. W Kricie rozdzielono części, skorygowano oświetlenie i domalowano przetarcia na łuskach, żyłkowanie błon i krawędzie płyt. Gra ładuje atlas RGBA 1024 × 512. Współrzędne malowania obracają się razem z pancerzem, a rysowanie ciała i efektów zachowuje tekstury przy zmianie materiału. [Master Krity](C:/AI2D/sprites/kaldera-czworonogi-malowanie-20261121/kaldera-retusz-czworonogow.kra) · [Real-ESRGAN](https://github.com/xinntao/Real-ESRGAN/releases/tag/v0.2.1)

Porównano 24 ujęcia każdego modelu w rzeczywistym rendererze. Wszystkie 48 masek alfa pokrywa się piksel w piksel. Oba potwory pokonały ten sam tor w podglądzie Kaldery. Odczyt i zapis 66 klatek renderu trwał 3,60 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-czworonogi-malowanie-20261121/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-czworonogi-malowanie-20261121/report.json)

**20.11.2026 — nowe animacje i rigi 2D draka oraz pełzacza.**

Każda łapa ma wspólny punkt mocowania uda do tułowia, kolana do podudzia i pazurów do stopy. Żarowy drak opiera błony i żebra skrzydeł na wspólnych konturach. Pancerz pełzacza ma jeden kontur i zachodzące na siebie płyty. Silnik rysuje te same części podczas chodu, ataku, upadku i spoczynku, a kąt obrotu przesuwa wspólne punkty wszystkich części. Cztery malowane materiały wygenerowano w Sunburst i rozdzielono na warstwy w Kricie. [Materiały](C:/AI2D/sprites/kaldera-nowe-rigi-czworonogow-20261120/materialy-czworonogow.kra)

Nowe klucze kroku, oddechu, ugryzienia, ognia i upadku zapisano w Synfigu. Render kontroli 122 klatek krzywych trwał 5,93 s. Render pełnego obrotu w chodzie z odczytem i zapisem 66 klatek Kaldery trwał 3,70 s. W zestawie 24 sprawdzonych sylwetek każdego stworzenia nie wykryto osobnych fragmentów po uwzględnieniu jednopikselowej krawędzi. [Projekty Synfiga](../volcano/quadruped-rigs/) · [Podgląd](C:/AI2D/pipeline/runs/kaldera-nowe-rigi-czworonogow-20261120/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-nowe-rigi-czworonogow-20261120/report.json)

**20.11.2026 — przejścia kierunkowe draka i pełzacza.**

Z ośmiu malowanych kierunków i istniejących ujęć pośrednich przygotowano po 128 gotowych póz kierunkowych RGBA. Silnik dobiera ujęcia bez mapy odkształceń i stosuje jedną kolejność warstw w obrębie każdej klatki. Atlas zasila kolor, wybór kursorem i cień. Klucze Synfiga prowadzą krok również w trakcie skrętu. [Eksport atlasu](../../tools/media/caldera_direction_cels.py)

W próbie co 1° udział półprzezroczystych pikseli spadł z 8,9% do 5,8% u pełzacza i z 11,7% do 9,2% u draka. Maksymalna różnica koloru kolejnych ujęć wzrosła u obu modeli. Render z gry z odczytem i zapisem 54 klatek trwał 2,79 s. [Podgląd](C:/AI2D/pipeline/runs/drak-pelzacz-czyste-przejscia-20261120/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/drak-pelzacz-czyste-przejscia-20261120/report.json)

**20.11.2026 — chód draka i pełzacza przy zmianie kierunku.**

W Synfigu poprawiono klucze spoczynku, chodu i przechyłu. Skrzydła i szyja draka poruszają się razem z tułowiem, a pancerz i nasada ogona pełzacza korzystają ze wspólnej osi przechyłu. Łapy zachowują stałe wymiary, a punkty podparcia podążają za krokiem.

Synfig wyrenderował 206 klatek w 59,53 s. Render ujęcia z Kaldery z odczytem i zapisem 54 klatek trwał 3,04 s. [Podgląd](C:/AI2D/pipeline/runs/drak-pelzacz-synfig-chod-20261120/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/drak-pelzacz-synfig-chod-20261120/report.json)

**20.11.2026 — stabilna bryła pancerza pełzacza i skala draka.**

Obrys pancerza pełzacza korzysta z ośmiu dopasowanych kierunków bazowych i wspólnego rzutu bryły 2D. Łapy i ogon korzystają z dotychczasowych kluczy ruchu. Różnicę wysokości draka skorygowano względem kotwicy na podłożu. Paszcza, obrys kursora i cień uwzględniają tę samą korektę.

Przy stałej fazie chodu wysokość draka w próbie co 1° mieści się w zakresie 95–102 px. Przed korektą zakres wynosił 81–107 px. W porównaniu samego pancerza pełzacza zmienność pola obrysu w kolejnych odcinkach obrotu spadła o 22%. Render podglądu chodu z odczytem i zapisem 54 klatek trwał 2,80 s. [Podgląd](C:/AI2D/pipeline/runs/drak-pelzacz-stabilne-bryly-20261120/drak-i-pelzacz.webp) · [Raport](C:/AI2D/pipeline/runs/drak-pelzacz-stabilne-bryly-20261120/report.json)

**20.11.2026 — ostrość ruchu Żarowego Draka i Żużlowego Pełzacza.**

Silnik dopasowuje detale sąsiednich kierunków, prowadzi jeden kontur między nimi i rysuje pigment z wybranego ujęcia. Paszcza draka ma osobne dopasowanie dla każdego stopnia otwarcia. Cień i wybór myszą korzystają z tego samego konturu co sylwetka. [Generator atlasu](../../tools/media/caldera_crisp_turn.py)

W teście chodu co 1° maksymalna zmiana obrysu spadła z 13,1% do 7,2% u draka oraz z 8,0% do 3,4% u pełzacza. Cztery ujęcia z Kaldery pokazują obrót w miejscu, chód, ugryzienie i ognisty oddech oraz upadek i końcową pozycję obu ciał. Render 233 klatek z odczytem i zapisem trwał 12,69 s. [Raport](C:/AI2D/pipeline/runs/drak-pelzacz-ostry-ruch-20261120/report.json)

**20.11.2026 — płynna zmiana kierunku Rogacza Żaru.**

Głowa, tułów i cztery nogi przechodzą pomiędzy 32 ujęciami. Synfig przygotował ruch kopyt i pochylenie głowy. Kąt biegu i obrotu zmienia się ciągle także w chwili rozpoczęcia szarży. W teście co 1° obraz w chodzie zmienił się 360 razy, wcześniej osiem. Największa zmiana obrysu spadła z 69,75% do 6,72%. [Źródła](../volcano/cinderhorn/README.md)

Render ujęć testowych gry z odczytem i zapisem trwał 1,90 s. Synfig wyrenderował 61 klatek w 4,04 s. Lokalny FILM wygenerował 33 ujęcia w 55,48 s. [Podgląd](C:/AI2D/pipeline/runs/rogacz-plynny-skret-20261120/rogacz-obrot.webp) · [Wyniki](C:/AI2D/pipeline/runs/rogacz-plynny-skret-20261120/report.json)

**20.11.2026 — retusz twarzy Ognistego Chochlika.**

Zewnętrzne cieniowanie policzków, czoła i brody wynika teraz z koloru czaszki. Warstwy Krity zachowują osobne światło wewnątrz oczu, nosa i ust. Ponowny eksport odtworzył oba atlasy bajt w bajt. Oba atlasy i 150 ujęć renderera zachowały istniejący kanał alfa.

Lokalne kompilacje gracza i DEV zaliczyły po 180 klatek startu. Zaliczone zostały 498 prób grafiki i ruchu w Kalderze, 1414 kontroli grafiki oraz 5906 kontroli śmierci. Render testu Kaldery z odczytem i zapisem klatek trwał 4,02 s. [Podgląd](C:/AI2D/pipeline/runs/chochlik-twarz-20261120/twarz-chochlika.webp) · [Raport](C:/AI2D/pipeline/runs/chochlik-twarz-20261120/report.json)

**20.11.2026 — animacje Ognistego Chochlika w Synfigu.**

Natywne klucze Synfiga sterują spoczynkiem, podparciem i przenoszeniem nóg, obrotem, przygotowaniem kuli ognia i upadkiem. Gra korzysta z kluczy wyeksportowanych z `klucze-chochlika.sif`. Próbkowanie koloru w grze ogranicza migotanie łusek. [Źródła i eksport](../volcano/puppets/README.md)

Synfig wyrenderował 365 klatek pięciu projektów. Render pełnego obrotu trwał 22,61 s. Sprawdzono sylwetki z Synfiga i z renderera gry na tych samych pozach. Podgląd został wykadrowany z renderera gry. [Podgląd](C:/AI2D/pipeline/runs/chochlik-synfig-20261120/chochlik-obrot-synfig.webp) · [Raport](C:/AI2D/pipeline/runs/chochlik-synfig-20261120/report.json)

**20.11.2026 — nowy obrót całego ciała Ognistego Chochlika.**

Atlas 360 ujęć łączy głowę i tułów. W grze wspólny kąt steruje całym ciałem, a oba segmenty przy każdym stawie korzystają ze wspólnego mocowania. Chód ma fazę podparcia i przenoszenia nóg. Punkt startowy kuli ognia podąża za dłońmi, a całe ciało przewraca się wokół wspólnej osi. [Źródła atlasu i animacji](../volcano/puppets/README.md)

Sprawdzono 3240 póz w trzech skalach w spoczynku, w chodzie i podczas rzutu. Maksymalna zmiana obrysu w próbie co 1° wyniosła 4,88%. Render wykadrowanego podglądu z odczytem i zapisem klatek trwał 0,85 s, render 60 klatek referencyjnych w Synfigu 3,08 s, a pełnego obrotu w scenie 3,61 s. Są to czasy ukończenia renderów. [Podgląd](C:/AI2D/pipeline/runs/chochlik-nowy-obrot-ciala-20261120/chochlik-obrot-calego-ciala.webp)

Zaliczono 498 prób przeciwników Kaldery, 1414 kontroli grafiki i 5906 kontroli śmierci. Obie lokalne kompilacje mają ten sam zweryfikowany atlas i przeszły po 180 klatek testu startu. [Raport](C:/AI2D/pipeline/runs/chochlik-nowy-obrot-ciala-20261120/report.json)

**20.11.2026 — głowa chochlika jako jeden sprite.**

Z oryginalnych wycinanek powstało 360 kompletnych dwuwymiarowych ujęć głowy. Malowanie oczu i pyska podąża za powierzchnią czaszki, a nasady rogów są osadzone pod jej krawędzią. Silnik obraca całą głowę w jednym kawałku i używa tego samego sprite’a do cienia, podświetlenia i wyboru kursorem. Zachowano geometrię tułowia i kończyn oraz tory chodu i rzutu. [Źródła atlasu](../volcano/puppets/README.md)

Maksymalna zmiana obrysu głowy między kolejnymi kątami w atlasie wyniosła 1,96%. W 361 ujęciach renderera głowa zachowała jeden obrys, a w 120 ujęciach w chodzie największy dodatkowy fragment miał jeden piksel. Render wykadrowanego podglądu wraz z odczytem i zapisem trwał 1,00 s. [Podgląd](C:/AI2D/pipeline/runs/chochlik-jedna-glowa-20261120/chochlik-pelna-glowa.webp)

Zaliczono 498 prób grafiki przeciwników Kaldery, 1414 kontroli grafiki i 5906 kontroli śmierci. Obie kompilacje zawierają zweryfikowany atlas i przeszły po 180 klatek testu startu. [Raport](C:/AI2D/pipeline/runs/chochlik-jedna-glowa-20261120/report.json)

**20.11.2026 — przywrócenie pierwszej animacji chochlika z wyostrzeniem koloru.**

Przywrócono kod geometrii i animacji zapisany przed pierwszą korektą ostrości. Suma kontrolna oryginalnego atlasu jest zgodna. Shader wzmacnia lokalny kontrast koloru wewnątrz wycinanek. Porównano 361 klatek obrotu w spoczynku i 120 klatek obrotu w chodzie z zapisanym renderem pierwszej wersji. Wszystkie klatki zachowały identyczne piksele alfa, w tym pierwotne odstępy między warstwami. Render podglądu z odczytem i zapisem klatek trwał 0,96 s. [Podgląd](C:/AI2D/pipeline/runs/chochlik-pierwsza-animacja-20261120/pierwsza-animacja-chochlika-wyostrzona.webp)

Zaliczono 496 prób przeciwników Kaldery, 1414 kontroli grafiki i 5906 kontroli śmierci. Lokalne kompilacje gracza i DEV przeszły po 180 klatek testu startu. [Raport](C:/AI2D/pipeline/runs/chochlik-pierwsza-animacja-20261120/report.json)

**20.11.2026 — spójna sylwetka chochlika przy obrocie.**

Nasady rogów i pysk mają punkty mocowania wewnątrz czaszki. Uda osadzono głębiej w biodrach, a uda i golenie mają wspólny punkt wewnątrz kolana. Zachowano poprzednie filtrowanie tekstury i fazy ruchu. Renderer przeszedł test 3240 póz w trzech skalach, obejmujący spoczynek, chód i rzut. Dodatkowa próba obrotu co 1° potwierdziła ciągłość sylwetki. Render wykadrowanego podglądu z odczytem i zapisem klatek trwał 0,38 s. [Podgląd](C:/AI2D/pipeline/runs/chochlik-spojna-sylwetka-20261120/obrot-chochlika.webp)

Zaliczono 496 prób obu modeli Kaldery, 1414 kontroli grafiki i 5906 kontroli śmierci. Lokalne kompilacje gracza i DEV przeszły po 180 klatek testu startu. [Raport](C:/AI2D/pipeline/runs/chochlik-spojna-sylwetka-20261120/report.json)

**20.11.2026 — ostrzejszy chochlik i naturalniejszy ruch Obsydianowego Wojownika.**

Tekstura chochlika korzysta z oddzielnie filtrowanych poziomów pomniejszenia i przezroczystych marginesów między częściami. Shader wzmacnia kontrast detali wewnątrz sylwetki. Dla 12 ujęć w skali gry średnia różnica względem obrazu wyrenderowanego w czterokrotnie wyższej rozdzielczości i pomniejszonego uśrednianiem pikseli spadła z 9,53 do 3,93 w skali kanałów RGBA od 0 do 255.

Wojownik wyżej unosi but przy przenoszeniu nogi i kołysze biodrami. Płaszcz i ramiona podążają z opóźnieniem, a przy cięciu napierśnik skręca mocniej. Po cięciu miecz i tarcza wyhamowują osobno. Synfig wyrenderował 41 klatek chodu w 18,51 s, 61 klatek zwykłego cięcia w 25,06 s i 97 klatek Rozłupującego ciosu w 35,74 s. [Źródła ruchu](../volcano/obsidian-sentinel/redesign/pochodzenie.json)

Zaliczono 495 prób obu przeciwników, 1414 kontroli grafiki i 5906 kontroli śmierci. Lokalne kompilacje gracza i DEV przeszły po 180 klatek testu startu. Render wykadrowanego podglądu w skali gry trwał 0,72 s wraz z odczytem i zapisem klatek. [Podgląd](C:/AI2D/pipeline/runs/kaldera-ostre-animacje-20261120/kaldera-chochlik-i-wojownik.webp) · [Raport](C:/AI2D/pipeline/runs/kaldera-ostre-animacje-20261120/report.json)

**20.11.2026 — nowe modele 2D Ognistego Chochlika i Popielnego Upiora.**

Sunburst przygotował nowe osobno malowane części. Atlasy powstały z dwóch warstwowych projektów Krity. Podczas obrotu silnik przesuwa i obraca wycinanki o stałych współrzędnych tekstury. Synfig wyrenderował sześć klipów po 60 klatek. Render poszczególnych klipów trwał od 2,44 s do 3,71 s. [Źródła i przygotowanie atlasów](../volcano/puppets/README.md)

W próbie co 1° największa zmiana obrysu chochlika spadła z 13,99% do 5,97%, a udział półprzezroczystych pikseli z 16,12% do 4,80%. Dla Upiora zmiana obrysu spadła z 3,98% do 2,54%. Pełna scena potwierdziła po jednym rzucie i trafieniu obu pocisków. Render pełnego obrotu w polu trwał 3,51 s. Render wykadrowanego podglądu w skali gry wraz z odczytem i zapisem klatek trwał 2,61 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-nowe-rigi-20261119/nowe-modele-kaldery.webp)

Zaliczono 494 kontrole chochlika i wojownika oraz 468 kontroli Upiora. Po odczytaniu zapisu ciała obu przeciwników zachowują te same piksele i kotwicę co w chwili zetknięcia z ziemią. Pełne wydanie przeszło 1414 kontroli grafiki i 5906 kontroli śmierci. Po rozpakowaniu obu wersji Windows sprawdzono po 752 zasoby i 180 klatek uruchomienia. [Raport wydania](C:/AI2D/pipeline/runs/kaldera-nowe-rigi-20261119/report.json)

**19.11.2026 — płynne obrysy Ognistego Chochlika i Popielnego Upiora.**

Silnik przesuwa kotwice i obrysy części pomiędzy 16 malowanymi pozami. Każdy detal zachowuje kolor jednego ujęcia. W porównaniu pełnych obrotów co 1° największa zmiana obrysu między sąsiednimi klatkami spadła z 82,45% do 13,99% u chochlika i z 32,75% do 3,98% u Upiora. Render dwusekundowego podglądu wraz z odczytem i zapisem klatek trwał 1,98 s. Render pełnego obrotu w scenie Kaldery trwał 4,07 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-ciagly-kontur-20261119/pelne-obroty.webp)

Zaliczono 479 kontroli chochlika i wojownika, 436 kontroli Upiora, 1414 kontroli grafiki oraz 5891 kontroli śmierci. W pełnej scenie sprawdzono rzuty i trafienia obu przeciwników. Po rozpakowaniu obu wydań Windows sprawdzono po 750 zasobów i 180 klatek uruchomienia. [Raport wydania](C:/AI2D/pipeline/runs/kaldera-ciagly-kontur-20261119/report.json)

**19.11.2026 — 16 malowanych kierunków obu przeciwników Kaldery.**

Chochlik korzysta z 16 pełnych pozycji zaczerpniętych z malowanych ujęć. Atlas zawiera jedynie ich ruchome warstwy. Wszystkie części, obrys i cień wybierają ten sam kierunek. Popielny Upiór ma cztery nowe ilustracje z Sunburst i ich odbicia. Każda została podzielona na kaptur, poły, tren i rękawy w projekcie Krity. Zasadnicze ruchy podczas ataku, lotu i upadku prowadzi istniejący rig. Przejście do sąsiedniego kierunku jest widocznym krokiem.

Testy z naprzemiennym kolorem całej postaci potwierdziły, że podczas obrotu silnik wybiera jedno ujęcie. Przeszło 479 kontroli chochlika i wojownika oraz 451 kontroli Upiora. Podgląd z nieruchomą kamerą obejmuje pełne 360° i trwa dwie sekundy. Render z odczytem i zapisem trwał 2,06 s. [Podgląd](C:/AI2D/pipeline/runs/kaldera-malowane-kierunki-20261119/pelne-obroty.webp) · [Projekt Krity i prompt Upiora](../volcano/ash-wraith/paint/turn-keys/)

W pełnej scenie Kaldery sprawdzono oba rzuty, trafienia i zmianę kierunku przeciwników. Gotowe wydanie przeszło 1414 kontroli grafiki i 5891 kontroli śmierci. Obie rozpakowane paczki Windows zweryfikowano na 746 zasobach i 180 klatkach uruchomienia. [Raport wydania](C:/AI2D/pipeline/runs/kaldera-malowane-kierunki-20261119/report.json)

**19.11.2026 — pojedyncze sylwetki w obrocie chochlika i Upiora.**

Każda część chochlika wybiera pełny obrys jednego z 96 ujęć. Rejestracja przesuwa ten obrys, a malowanie przechodzi na kolejny kierunek w różnym momencie na poszczególnych częściach. U Upiora pojedyncze ujęcie haftu i fałd pochodzi z nowego atlasu 96 kierunków. Rig nadal prowadzi obrys szaty podczas lotu, rzutu i opadania.

Do prób obu modeli dodano zamianę malowania na kontrastowe kolory. Test wykrywa nakładanie kilku ujęć także wtedy, gdy średnia ostrość całej sylwetki pozostaje wysoka. Przeszło 471 kontroli chochlika i wojownika oraz 429 kontroli Upiora. W teście chochlika największa zmiana wewnątrz obrysu pomiędzy kolejnymi stopniami wyniosła 17,40 punktu RGB przy limicie 18. Wykadrowany podgląd zawiera 120 klatek na postać i trwa dwie sekundy. Render z odczytem i zapisem trwał 2,18 s. [Podgląd obu obrotów](C:/AI2D/pipeline/runs/kaldera-czyste-pozy-20261119/pelne-obroty.webp)

Ponowne przechwycenie i spakowanie szaty odtworzyło sumę SHA-256. Gotowe wydanie przeszło 1414 kontroli grafiki i 5891 kontroli śmierci. W obu paczkach Windows sprawdzono zgodność 744 zasobów i po 180 klatek uruchomienia. [Raport wydania](C:/AI2D/pipeline/runs/kaldera-czyste-pozy-20261119/report.json)

**19.11.2026 — czytelny obrót całych sylwetek Ognistego Chochlika i Popielnego Upiora.**

Cztery nowe ilustracje kierunkowe z wbudowanego Sunburst zostały rozdzielone na głowę z tułowiem, ręce, nogi i ogon. Projekt Krity zachowuje te części w osobnych warstwach i zawiera dodatkowe malowanie pod stawami. Silnik korzysta z 96 gotowych ujęć. Dopasowuje sąsiednie klatki i utrzymuje ostre detale podczas krótkiego przejścia. Chochlik i Obsydianowy Wojownik przeszli 463 próby grafiki, ataków i upadku. [Projekt Krity](../volcano/fire-imp/paint/inbetweens/polobroty.kra) · [Źródło i prompt](../volcano/fire-imp/paint/inbetweens/source.json)

Upiór korzysta ze wspólnej tekstury ubrania z osobnymi pasami dla kaptura, rękawów i pół szaty. Wzór podąża za materiałem przez pełny obrót. Sprawdzono 428 przypadków, w tym obrys, cień i ułożenie szaty po upadku. Pełne obroty obu modeli sprawdzono co 1° w powiększeniu i co 3° w natywnej skali. Render 120 klatek każdego modelu z odczytem i zapisem trwał łącznie 2,42 s. [Raport Upiora](C:/AI2D/pipeline/runs/wraith-woven-turn-20261118/report.json) · [Dwusekundowy podgląd i pomiary](C:/AI2D/pipeline/runs/kaldera-ostrosc-ruchu-weryfikacja-20261118/v6/report.json)

Ponowny eksport atlasu chochlika i mapy ruchu odtworzył sumy kontrolne. Wydanie przeszło 1414 kontroli grafiki i 5891 kontroli śmierci. W obu rozpakowanych paczkach Windows sprawdzono 743 zasoby i po 180 klatek uruchomienia. [Raport wydania](C:/AI2D/pipeline/runs/kaldera-wyrazne-obroty-20261118/report.json)

**18.11.2026 — płynny obrót Popielnego Upiora i czytelny rzut Ognistego Chochlika.**

Kaptur, rękawy i poły szaty poruszają się przez cały obrót. W próbie z nieruchomą szatą silnik wyrenderował 360 różnych obrazów dla 360 kierunków, wcześniej 32. Przy zmianie kierunku o stopień największe przesunięcie brzegu rękawa spadło z 12 do 1 piksela. Największa zmiana obrysu spadła z 23,16% do 2,18%. Sprawdzono także przejście przez 0°, cień, obrys pod kursorem i ułożenie szaty po upadku. [420 kontroli Upiora](C:/AI2D/pipeline/runs/wraith-continuous-20261118/report.json)

Chochlik formuje kulę, zginając obie ręce w łokciach. Uzupełniono malowane detale odsłaniane przez ruch ramion. Przy pełnym naładowaniu płomień zajmuje 16,3% widocznego obszaru wokół dłoni, poprzednio 34,2%. W scenie walki kula opuszcza dłonie, trafia bohatera i wybucha. Synfig wyrenderował 61 klatek rzutu w 4,86 s. Render 108 klatek podglądu sceny z odczytem i zapisem PNG zajął 6,32 s. [462 kontrole chochlika i wojownika](C:/AI2D/pipeline/runs/imp-cast-clarity-20261118/pair-final/summary.json) · [Podgląd i pomiary](C:/AI2D/pipeline/runs/kaldera-kontrola-ruchu-20261118/report.json)

Pełne wydanie przeszło 1414 kontroli grafiki i 5891 kontroli animacji śmierci. Obie paczki Windows sprawdzono po rozpakowaniu, porównując 740 zasobów i uruchamiając po 180 klatek. [Raport wydania](C:/AI2D/pipeline/runs/kaldera-obrot-upiora-rzut-chochlika-20261118/report.json)

**18.11.2026 — obrót Ognistego chochlika i nowa szata Popielnego Upiora w Kalderze.**

Podczas obrotu chochlika położenia oczu, szczęki, rogów i kończyn zmieniają się płynnie. Każdy punkt zachowuje kolor i krycie z pojedynczej malowanej ilustracji. W próbie GPU obejmującej 360 kątów co 1° kontrast detali głowy na granicy najsłabszych 10% ujęć wzrósł z 78,0% do 96,2% kontrastu sąsiednich widoków głównych. Odczyt i zapis obrotu zajął 1,28 s. [Źródła i pomiary chochlika](../volcano/fire-imp/paint/receipt.json)

Płaszcz Upiora podzielono na tren, dwie przednie poły, kaptur z peleryną i dwa puste rękawy. Pięć ilustracji Sunburst posłużyło do opracowania 32 kierunków w Kricie i ComfyUI. Lokalny FILM wyrenderował 33 klatki materiału w 18,37 s. Synfig wyrenderował spoczynek, lot, rzut i opadanie szaty w 42,32 s (268 klatek). Podczas rzutu żar wydobywa się z prawego rękawa. Przy upadku tren pierwszy dotyka podłoża, po nim osiadają rękawy i kaptur. [Grafika i projekty Krity](../volcano/ash-wraith/paint/README.md) · [Animacje Synfiga](../volcano/ash-wraith/motion)

Upiór przeszedł 417 kontroli obrysu, rzutu, cienia i opadania szaty, a chochlik i wojownik 446 kontroli grafiki i walki. W pełnym wydaniu zaliczono 1414 kontroli grafiki i 5891 kontroli śmierci. W obu rozpakowanych wydaniach sprawdzono zgodność 738 zasobów i 180 klatek uruchomienia. Render sceny w Kalderze wraz z odczytem i zapisem 123 klatek trwał 3,61 s. [Podgląd z gry](C:/AI2D/pipeline/runs/kaldera-upior-i-chochlik-20261118/chochlik-i-upior.webp) · [Pomiary](C:/AI2D/pipeline/runs/kaldera-upior-i-chochlik-20261118/report.json)

**18.11.2026 — ostre krawędzie podczas obrotu, zamach mieczem i kula ognia w Kalderze.**
U chochlika i wojownika malowane detale podążają teraz za obrotem dzięki mapom przesunięć. W próbie z krokiem 1° udział częściowo przezroczystych pikseli zmalał z 31,7% do 17,6% u chochlika i z 20,8% do 10,9% u wojownika. Render, odczyt i eksport 720 klatek obu obrotów trwał 3,047 s. Pod napierśnikiem i przy biodrach wojownika uzupełniono fragmenty widoczne przy podniesieniu ręki. Zwykły atak i Rozłupujący cios mają osobne ruchy barku, łokcia i klingi.

Chochlik przez 0,66 s zbiera ogień między dłońmi. Centrum płomienia wyznacza początek lotu. Trafienie gracza lub osłony zapala wybuch dokładnie na końcu toru. Sprawdzono 56 torów w ośmiu kierunkach oraz ciągłość przy wczytaniu zapisu. Format v31 zapamiętuje biegnący pocisk i wygasający rozbłysk. Sprawdzono również zapis wykonany w poprzedniej grze (v30) podczas jednoczesnego lotu kuli chochlika i pocisku upiora. [Animacja rzutu](../volcano/fire-imp/motion/imp-cast.sif) · [Animacje miecza](../volcano/obsidian-sentinel/redesign/motion)

W zachodnim profilu droga ostrza od szczytu zamachu do wyhamowania wynosi 154,8 px, poprzednio 86,5 px. Synfig ukończył 61 klatek zwykłego cięcia w 24,643 s, 97 klatek Rozłupującego ciosu w 41,398 s i 61 klatek rzutu w 5,166 s. Kontrola obu przeciwników zaliczyła 445 prób. Render, odczyt i eksport 194 klatek zajęły 4,259 s. Bezstratny podgląd rzeczywistego pojedynku i wyniki są w `C:\AI2D\pipeline\runs\kaldera-ostry-obrot-i-ataki-20261118`.

**18.11.2026 — Ognisty chochlik i Obsydianowy wojownik w Kalderze Popiołu.**
Chochlik ugina nogi, przenosi ciężar między kopytami i prowadzi ruch ogonem. Żar zapala się w dłoni, z której wylatuje pocisk. Wojownik osłania się tarczą na lewym przedramieniu. Zwykłe cięcie jest krótkie, a przed Rozłupującym ciosem wojownik przenosi ciężar i unosi miecz. Po śmierci chochlik osiada na boku, a wojownik rozpada się na fragmenty pancerza z pozy, w której trafił go ostatni cios.

Ilustracje Sunburst opracowano w Kricie. Każda postać ma 32 widoki złożone z sześciu ruchomych części. Lokalny FILM w ComfyUI wyrenderował po 33 klatki, w 24,584 s dla chochlika i 25,242 s dla wojownika. Render w Synfigu trwał odpowiednio 13,504 s dla 207 klatek i 96,460 s dla 261 klatek. W próbie chodu sprawdzono po osiem faz w ośmiu kierunkach dla obu postaci. [Źródła chochlika](../volcano/fire-imp/README.md) · [Źródła i pomiary wojownika](../volcano/obsidian-sentinel/redesign/pochodzenie.json)

Renderer zaliczył 407 kontroli obu przeciwników. Render, odczyt i eksport 185 klatek zajęły 3,817 s. We wszystkich 32 kombinacjach kierunku i strony trafienia ostatnia klatka upadku była pikselowo identyczna z leżącym ciałem. Ogólne testy grafiki (1414) i śmierci (5891) zakończyły się bez błędów. W obu rozpakowanych wydaniach sprawdzono po 734 zasoby i 180 klatek uruchomienia. Raport i krótki podgląd są w `C:\AI2D\pipeline\runs\kaldera-chochlik-wojownik-20261118`.

**18.11.2026 — nowy wygląd i ruch [Żużlowego Pełzacza](../volcano/slag-crawler/README.md).**
Pięć oryginalnych ilustracji posłużyło do opracowania 32 widoków. Lokalny FILM w ComfyUI ukończył 33 klatki w 36,783 s. Z końcowego projektu Krity wyeksportowano 192 wycinki. Synfig wyrenderował 227 klatek siedmiu klipów w 126,925 s. Kontrola podparcia objęła 4016 próbek. Największe odchylenie podpartego pazura od zakładanego toru wyniosło 0,045 px.

W grze spoczywający Pełzacz zachował 94 × 53 px z boku i 52 × 61 px od przodu. Obrót w chodzie daje 360 zmian obrazu przy 360 kolejnych krokach o jeden stopień. Render 120 klatek z czterema Pełzaczami na ukrytym celu trwał 0,038 s. Osobny przebieg 2071 klatek z odczytem, eksportem i kontrolami trwał 3,550 s. Po rozpakowaniu wydań gracza i DEV sprawdzono po 732 zasoby i 180 klatek startu. Ukryte próby uruchomiono z wyłączonym dźwiękiem. [Animacja](../volcano/slag-crawler/obrot.gif) · [Pomiary i sumy kontrolne](../volcano/slag-crawler/qa.json)

**18.11.2026 — płynny obrót [Żarowego Draka](../volcano/ember-drake/README.md).**
Drak jest większy o 15%. Obrót wykorzystuje 32 widoki sześciu części ciała
i przechył skrzydeł oraz tułowia z nowego klipu Synfiga. Kierunek zmienia
się płynnie także podczas ugryzienia, a struga ognia zaczyna się przy pysku.
Lokalny FILM wykonał 33 klatki w 48,07 s. Synfig wyrenderował 17 klatek
przechyłu w 8,738 s. Eksport 192 wycinków z [projektu Krity](../volcano/ember-drake/turn/drake-turn.kra)
odtworzył dokładną sumę kontrolną atlasu używanego w grze.

Przy obrocie co 1° renderer odróżnił 360 kolejnych ujęć w spoczynku i chodzie.
Przy nagłym nawrocie największy krok zmalał z 55,25° do 6,68° w symulacji 60 Hz.
W ukrytym teście czterech Draków ukończenie renderu 120 klatek trwało 0,058 s.
Osobna próba ze zmianami kierunku, odczytem i eksportem obrazu zajęła 2,911 s
dla 2015 klatek. Łącznie obie próby funkcjonalne wykonały 444 pomyślne kontrole.
Po rozpakowaniu paczki gracza i paczki deweloperskiej sprawdzono po 731 zasobów
i po 180 klatek startu w ukrytym oknie bez dźwięku.
[Podgląd obrotu](../volcano/ember-drake/turn/obrot.gif) · [Wyniki](../volcano/ember-drake/turn/qa.json)

**18.11.2026 — [Żarowy Drak z Kaldery](../volcano/ember-drake/README.md).**
Pięć malowanych ujęć z wbudowanego generatora i faktury lokalnego DreamShapera 8
posłużyły do przygotowania w Kricie postaci w ośmiu kierunkach. Dwa projekty KRA
zawierają wycinanki ciała oraz czterdzieści ujęć ruchomej paszczy. Synfig
wyrenderował 217 klatek pięciu klipów w 79,493 s łącznie z kontrolą sekwencji.
W grze sprawdzono ugryzienie, chód, fazy żarowego oddechu, widoczność ognia
na stoku oraz osiadanie skrzydeł i ciała. Render, odczyt i eksport 146 kadrów
trwały 4,773 s. Wszystkie 227 kontroli draka zakończyło się pomyślnie.
Obie rozpakowane paczki miały po 730 zgodnych zasobów i wykonały po 180 klatek
bez dźwięku w ukrytym oknie. Deweloperska przeszła także 5891 kontroli śmierci
i osiem kontroli wyłączonego dźwięku. [Podgląd](../volcano/ember-drake/preview.gif)
i [wyniki](../volcano/ember-drake/qa.json).

**18.11.2026 — wyjmowanie strzały z kołczanu.** Prawa ręka Wędrowca
sięga za bark, wysuwa strzałę wzdłuż kołczanu i po wyjściu grotu
obraca ją w stronę łuku. Dłoń zakłada nasadkę w miejscu spoczynku
cięciwy danego łuku. Siła strzału zaczyna rosnąć po 0,60 s. Krótkie
kliknięcie kończy przygotowanie i wypuszcza jedną strzałę po krótkim naciągu.
[Edytowalny ruch w Synfigu](../hero/traveler-performance/bow-nock.sif)
ma 37 klatek. Natywny render trwał 1,503 s. W 156 384 próbkach pozy
sprawdzono wyjęcie grotu, położenie nasadki na cięciwie oraz przejście
do naciągu w spoczynku i ruchu. W grze sprawdzono, po której stronie
widać sięgającą rękę z przodu i z tyłu, także z hełmem i tarczą.
Render 1312 klatek w ośmiu kierunkach, w dwóch zestawach wyposażenia,
trwał 4,814 s. Testy geometrii objęły wszystkie 13 łuków.

**18.11.2026 — bieg Wędrowca i naciąganie łuku.** W cyklu biegu
pochylenie tułowia wzrosło średnio z 9,9° do 14,6°, a zakres ruchu dłoni
przód–tył o 21%. Rozbieg i wyhamowanie trwają po 0,42 s. Prędkość świata
i rytm nóg korzystają z tej samej krzywej. Zwolnienie i ponowne wciśnięcie
Shiftu oraz zapis i wczytanie zachowują fazę kroku. W 7080 pośrednich pozach
sprawdzono podparcie stóp. Synfig wyrenderował 48 klatek biegu w ośmiu
kierunkach w 3,309 s, a osobne 48 klatek ruchu ramion w 0,619 s.

Prawa dłoń łucznika naciąga cięciwę do policzka. Po wypuszczeniu ruch palców
i powrót cięciwy mają osobne tory. Zaktualizowano także przejście od biegu
do podniesienia łuku. Edytowalne tory obu dłoni i klipu wypuszczenia są w
[`bow-arms.sif`](../hero/traveler-performance/bow-arms.sif).
Natywny render 109 klatek trwał 0,454 s. Wcześniej w 13 z 24 próbek
naciągu dłoń znikała ze wszystkich ośmiu rzutów. Po zmianie była widoczna
z co najmniej sześciu kierunków w każdej próbce, także w zestawie z hełmem
i tarczą. Sprawdzono 73 zachowania łuku w wersji gry oraz 10 711 kontroli
zapisu. [Źródła i odtworzenie ruchu](../hero/traveler-performance/README.md)

**18.11.2026 — chód, bieg i obrót głowy Wędrowca.** W Synfigu zmieniono
podparcie pięty i noska, ugięcie nóg, przenoszenie ciężaru oraz ruch barków.
Render końcowych klipów w ośmiu kierunkach trwał 3,576 s dla chodu (60 klatek)
i 3,293 s dla biegu (48 klatek). Sunburst i Krita dostarczyły dodatkowe
ujęcia głowy. Lokalny FILM wyrenderował klatki pośrednie w 4,462 s.
Runtime interpoluje sąsiednie spośród 64 ujęć pełnego obrotu.

W diagnostyce renderera gry obraz samej głowy zmieniał się w 360 z 360
kroków po 1° w obu kierunkach. W poprzedniej wersji zmieniał się osiem razy.
Największa zmiana między klatkami spadła z 12,04 do 2,29 poziomu kanału
na piksel kadru. Próba 4122 klatek objęła też otwarty i zamknięty hełm,
kaptur, chód, przejście do sprintu i nawrót. Render z eksportem trwał
13,88 s w ukrytym oknie. Test logiki i animacji zaliczył 555 921 kontroli,
renderer postaci 868, a wyposażenie 820. Każda z dwóch rozpakowanych paczek
zawierała 729 zgodnych zasobów i wykonała 180 klatek bez dźwięku.
[Źródła obrotu](../hero/wayfarer/turn/README.md) · [Źródła ruchu](../hero/traveler-performance/README.md)

**17.11.2026 — nowy Wędrowiec.** Sunburst dostarczył widoki głowy,
projekt stroju i kirysu. Lokalny DreamShaper 8 dostarczył dziesięć materiałów.
Trzy edytowalne projekty Krity odpowiadają piksel w piksel trzem arkuszom gry.
Synfig wyrenderował sześć klipów w ośmiu kierunkach (302 plansze, 20,197 s)
i dwadzieścia gestów bojowych oraz przejść (500 klatek, 1,555 s).
Ekspozycja twarzy zależy od założonego hełmu. Broń, tarcza, ubranie
i dodatki podążają za mocowaniami animowanej postaci.

Renderer bohatera przeszedł 868 kontroli w 22,37 s, w tym 1536 ujęć
32 akcji specjalistycznych. Renderer wyposażenia przeszedł 755 kontroli
w 8,07 s, w tym założenie i zdjęcie 116 przedmiotów oraz obie sceny
z Eldrinem. Pełna diagnostyka grafiki przeszła 1414 kontroli. Obie
lokalne paczki po wypakowaniu zachowały wszystkie 728 zasobów i wykonały
po 180 klatek w ukrytym oknie bez dźwięku. Źródła i podgląd są w
[`art/hero/wayfarer`](../hero/wayfarer/README.md), a logi w
`C:/AI2D/pipeline/runs/emberfall-wayfarer-20261118`.

**17.11.2026 — nowy Ognisty chochlik.** Lokalny DreamShaper 8 przygotował
koncepcję i faktury, a natywna Krita zapisała sześć warstw wycinanki w ośmiu
kierunkach. Synfig dostarczył oddech, krok zależny od pokonanego dystansu,
zebranie żaru, wyrzut pocisku i stopniowy upadek na bok. Materiał ciała
przygasa po śmierci. Budżet atlasu, maski i materiału szczątków to nadal
16,5 MiB RGBA8.

PASS: 185 kontroli renderera i 112 klatek podglądu, 5891 kontroli śmierci
i 8 trybu bezdźwiękowego w ukrytej kompilacji. Render, odczyt i eksport
zadania podglądu trwały 0,76 s, a pełnego testu śmierci 32,65 s. Nowa
kompilacja jest w `dist/Emberfall.exe`. KRA, kopie i zgłoszenie ComfyUI są
w `C:/AI2D/sprites/emberfall-fire-imp-20261117`, projekty SIF w
`C:/AI2D/animations/emberfall-fire-imp-20261117`, a podgląd i raport w
`C:/AI2D/pipeline/runs/emberfall-fire-imp-20261117`.

**17.11.2026 — dźwięki 22 przeciwników Emberfall.** Stable Audio 3.0
Small SFX wygenerował niezależne ujęcia ataku, trafienia, umierania i
kontaktu ciała z podłożem oraz sześciu ataków specjalnych. Audacity 3.7.9
usunął składową stałą i skompresował trzy taśmy, zapisując otwarte AUP3.
Do bieżącej gry podłączono 94 sygnały i 282 warianty 48 kHz stereo PCM16.
Najwyższy true peak wyniósł −3,73 dBTP, a początek kontaktu z podłożem
następuje w ciągu 20,69 ms. Raylib potwierdził wszystkie 486 wariantów gry
w 1658 kontrolach bez otwierania urządzenia audio. Ukryty EXE przeszedł
5891 kontroli animacji śmierci oraz 8 kontroli trybu bez dźwięku. Test
renderowania, odczytu i eksportu klatek zajął 32,11 s.

Opisy brzmień są w `tools/media/presets/creature_sfx.txt`, generacja w
`tools/media/stable_creature_audio.py`, a montaż, kontrola i podpinanie w
`tools/media/master_creature_audio.py`. Grafy, pliki źródłowe FLAC, mastery,
receptury, raport WAV i potwierdzenia kopii są w
`C:/AI2D/audio/emberfall-creature-sfx-20261117`. Raport integracji znajduje się w
`C:/AI2D/pipeline/runs/emberfall-creature-sfx-20261117/report.json`.
[Warunki wykorzystania komercyjnego](https://kb.stability.ai/knowledge-base/stable-audio-commerical-and-usage-licensing).

**12.11.2026 — pięć włóczni.** Zachowano dostarczone ikony, materiały i
źródła łupu. Pięć projektów Synfig daje osobne przygotowanie i powrót
do gardy, ze wspólną klatką kontaktu. Dla czterech odmian Csound
wyrenderował 24 nowe pliki WAV PCM16 48 kHz stereo. Gra wybiera
osobny bank ruchu i kontaktu dla każdego modelu. Pchnięcie zatrzymuje
się na pierwszym celu lub przeszkodzie, w tym na skrzyni. Zapis v18
zachowuje stan pchnięcia przed kontaktem i podczas powrotu do gardy.

Źródła w `C:/AI2D/animations/emberfall-spear-finish-20261112` i
`C:/AI2D/audio/emberfall-spear-finish-20261112` można odtworzyć przez
`tools/media/spear_synfig.py` i `tools/media/spear_audio.py`.
Opcja `--export-only` przenosi zmienione klucze Synfig do gry.
150 klatek Synfig, 524 756 kontroli rozgrywki, 20 087 zapisu, 532
grafiki i 724 miksera offline. Czas ukrytego renderu i eksportu
PNG w teście gry wyniósł 6,06 s. Wyniki i podgląd są w
`C:/AI2D/pipeline/runs/emberfall-spear-finish-20261112`.

**11.11.2026 — wiatr niezależny od kamery.** Obie warstwy shadera
próbkują rzut współrzędnych mapy, a ich dryf zależy od czasu. Wygląd
pustynnego piasku zachowano. W lesie są smugi podmuchów i niesione listki,
w Kalderze osobne drobiny popiołu. Tekstury dwóch krain dopracowano w Kricie
na bazie wcześniejszych źródeł ComfyUI. Przeszło 30 kontroli renderera,
w tym porównanie tych samych miejsc mapy przy nieruchomym czasie oraz
przesunięciu kamery w obu osiach. Podgląd i raport są w
`C:/AI2D/pipeline/runs/wind-world-20261111`.

**11.11.2026 — wiatr w trzech krainach.** DreamShaper 8 w lokalnym ComfyUI
i trzy warstwowe pliki Krity dostarczyły maski mgiełki, piasku i popiołu.
Shader przesuwa dwie warstwy nad scenerią, ogranicza krycie przy bohaterze
i kończy pracę przed rysowaniem HUD. Wczytuje trzy tekstury 640 × 320
przy starcie. Przeszły 24 kontrole renderera bez dźwięku, w tym zmiana
podmuchów w czasie, pokrycie alfa i zgodność pikseli bocznego HUD.
Źródła KRA są w `C:/AI2D/sprites/region-wind-20261111`.
Graf ComfyUI, raport i zrzuty są w `C:/AI2D/pipeline/runs/region-wind-20261111`.

**11.11.2026 — dopracowanie wieży i dwóch przeciwników.** Źródło odnowy
przesunięto od schodów, Miron stoi na środku pracowni, a podpisy NPC
mają przejrzyste tło. Witraże obserwatorium osadzono w kamiennej ścianie
z otwartym wyjściem na balkon. Rogacz Żaru jest większy o 45%. Zarodnik
kniei ma widok od tyłu oraz poprawione kierunki chodu i ataku. Dwa
źródła KRA i 92 klatki Synfig zapisano lokalnie pod nazwą
`tower-finish-20261111`. Przeszło 49 kontroli obrazu, interakcji i kierunków,
239 handlu oraz 479 129 logiki i animacji. Obie paczki Windows zweryfikowano
po wypakowaniu. Testy grafiki i startu działały w ukrytym oknie bez dźwięku.
Zrzuty i odbiór są w
`C:/AI2D/pipeline/runs/tower-finish-20261111`.

**11.11.2026 — trzy gatunki przeciwników.** Zarodnik kniei, Oko Wydm
i Rogacz Żaru mają po cztery spotkania w swoich krainach, własne łupy
i osobne efekty ataku, trafienia oraz upadku. Lokalny DreamShaper 8
w ComfyUI i trzy projekty KRA dostarczyły wycinanki. Synfig wyrenderował
15 klipów i 138 klatek do trzech atlasów z maskami kursora. ACE-Step 1.5
Turbo w ComfyUI, nagrania Kenney CC0 i Audacity 3.7.9 dały 27 wariantów
PCM16 stereo 48 kHz. Mastery zachowano w katalogach `C:/AI2D/sprites`,
`C:/AI2D/animations` i `C:/AI2D/audio` pod nazwą `region-enemies-20261111`.
Zapis v18 dodaje spotkania do odwiedzonych krain v1–v17, zachowując łupy.
PASS: 479 129 kontroli rozgrywki, 19 034 zapisu, 100 renderera i 624
miksera offline. Każda rozpakowana edycja Windows przeszła 180 ukrytych
klatek bez urządzenia audio. Raporty i podglądy są w
`C:/AI2D/pipeline/runs/region-enemies-20261111`.
[Wagi ACE-Step 1.5](https://huggingface.co/Comfy-Org/ACE-Step-1.5)

**11.11.2026 — wyposażenie Wieży Eldrina.** Odmalowano 25 grafik na pięciu
piętrach, w tym schody, regały, kominek, kocioł, teleskop, witraże i stoisko
Mirona. Lokalny ComfyUI z DreamShaper 8 dodał szczegóły materiałów. Akcenty
rysowane w Kricie zapisano na osobnej warstwie. Zachowano obrysy,
projekcję i maski wskazywania. Usunięto tabliczki pod schodami i stopkę
sklepu. Przeszło 91 kontroli renderera, grafiki i interakcji w ukrytym
oknie bez audio. W obu rozpakowanych edycjach zweryfikowano 312 zasobów
i 180 klatek startowych w ukrytym oknie bez audio.
Pliki KRA i grafy są w `C:/AI2D/sprites/emberfall-tower-20261111`,
a metryki, skróty plików i zrzuty w `C:/AI2D/pipeline/runs/tower-paint-20261111`.

**11.11.2026 — porządek w HUD i zgodność noszonego wyposażenia z ikonami.**
Wprowadzono 17 zmian z listy użytkownika. Pasek PD jest ciemnozielony,
a Atak i Pancerz są pod nim. Obrysy noszonego sprzętu odpowiadają używanym
ikonom, w tym spiczaste tarcze, hełmy z nosalem i topory o łukowym ostrzu.
Sprawdzono 85 przedmiotów w 2040 ujęciach, po osiem kierunków w postoju,
chodzie i biegu. Przeszło 2170 kontroli HUD i renderera oraz 477 595 kontroli
logiki i animacji. Każda paczka Windows przeszła 180 ukrytych klatek
startowych bez otwierania urządzenia audio. Wyniki i zrzuty znajdują się w
`C:/AI2D/pipeline/runs/hud-equipment-consistency-20261111`.

**11.11.2026 — naprawiony natywny MCP Sprite Studio 0.3.3.** Usunięto
zagnieżdżone `block_on`; handler renderuje w `spawn_blocking`. Zbudowano
i zainstalowano lokalny plik release z kopią oryginału. 39 testów rigu,
16 MCP oraz rzeczywiste 12 PNG / powtórny render / błąd FPS / eksport
na zainstalowanym programie: PASS. [Naprawa i odbiór](SPRITE_MAKER_FIX.md).

**11.11.2026 — kroki, zbieranie i Weź wszystko.** Sześć wariantów foley
CC0 zmontowanych w lokalnym ComfyUI i rzeczywistym Audacity (AUP3, 48 kHz).
Kroki zgodne z fazą stóp, atomowe zbieranie całego łupu i zachowane fragmenty
fabularne. 477 595 kontroli logiki, 19 030 zapisu, 534 audio i 7 UI: PASS.
Oba EXE i ZIP-y odświeżone; 287 zasobów i 180 ukrytych, bezgłośnych klatek
startowych każdej rozpakowanej edycji: PASS (bez wniosku o FPS na pierwszym planie).
[Źródła i odbiór](../audio/steps-loot-20261111/README.md).

**11.11.2026 — próba Sprite Studio 0.3.3.** Natywny import, zapis rigu,
analiza i eksport PNG działają. Render przez MCP ujawnił błąd zagnieżdżonego
runtime Tokio; bez klatek z tego renderu i bez podmiany animacji gry.
[Raport próby](SPRITE_MAKER_TRIAL.md).

**11.11.2026 — bardziej pionowe trzymanie broni.** Boczne odchylenie
ograniczone do 6 stopni dzięki chwytowi bliżej ciała i zgięciu łokcia.
Omijanie rękawa zmienia pochylenie do przodu. Kontrola 32 broni i 476 734
kontrole logiki/animacji: PASS. [Źródła i wyniki](../hero/WALK_QA.md).

**10.11.2026 — broń omija ramiona podczas chodu.** Korekta chwytu,
łokcia i orientacji ostrza/jelca dla mieczy, toporów i sztyletów; osobny
chwyt łuku. Wszystkie 32 bronie, 1384 pozycje renderera i 442 174 kontrole
logiki oraz animacji: PASS. Sprawdzono również przejścia do ataku i chowanie
broni. [Źródła i wyniki](../hero/WALK_QA.md).

**10.11.2026 — wymach ramion bohatera w chodzie.** Większy ruch obu
ramion z zachowaniem długości kończyn, zapisany w natywnym Synfigu 1.4.5.
60 klatek silnika Synfig, 120 klatek renderera w ośmiu kierunkach oraz
338 477 kontroli logiki i animacji: PASS. Wykrywanie przenośnego Synfiga
w skryptach działa bez instalatora. [Źródła i wyniki](../hero/WALK_QA.md).

**10.11.2026 — cztery gatunki leśnego bestiariusza.** Osobne projekty FLUX
i 13 warstw Krity, ciągłe mapowanie nowych materiałów i ruch z 444 klatek
Synfig. PASS w 1189 kontrolach renderera i 345 804 kontrolach logiki.
[Źródła i odbiór](../creatures/forest-20261110/README.md).

**10.11.2026 — leśni rozbójnicy.** Wyprostowana poza spoczynkowa,
skórzana kurta i bordowy kaptur z lokalnego FLUX.1 Schnell oraz Krity.
Cztery klipy Synfig i 192 sprawdzone klatki. PASS w 323 kontrolach
renderera, wskazywania kursorem i eksportu ciał oraz w 345 308 kontrolach
logiki i animacji. [Źródła i wyniki](../creatures/bandit-20261110/README.md).

**09.11.2026 — nowy wygląd Wędrowca.** FLUX.1 Schnell i SDXL, 12 warstw
Krity oraz sześć klipów ruchu tkaniny z projektów Synfig, sprawdzonych na
354 klatkach. Atlas 512 × 384, mocowania i bazowe czasy ruchu z istniejącego
rigu. PASS: 452 kontrole renderera, 594 wyposażenia, 340 709 logiki i animacji
oraz 19 047 zapisu. Obie paczki po wypakowaniu przeszły po 180 klatek,
ukryte i bez dźwięku. [Źródła i odbiór](../hero/wanderer-20261109/README.md).

**09.11.2026 — lokalne modele AI:** Piper TTS, ACE-Step 1.5, SDXL,
FLUX.1 Schnell, AnimateDiff, AudioLDM 2 i TangoFlux przeszły próby
generowania. ComfyUI działa w trybie offline i low VRAM, generatory
efektów mają osobne środowisko Pythona. Własna LoRA odłożona.
[Konfiguracja i wyniki prób](AI_SUITE_SETUP.md).

**09.11.2026 — ComfyUI i Krita MCP:** trzy nowe modele w katalogu
współdzielonym, sumy wydawców sprawdzone i trzy renderowania 512 × 512
zakończone powodzeniem na RTX 3060 6 GB. Zainstalowana Krita 5.3.4.1
połączona przez przypięty lokalny MCP 0.2.0. Próba odczytu, rysowania dwóch
warstw, podglądu oraz zapisu KRA i PNG zakończona powodzeniem.
[Konfiguracja i ścieżki do raportu](COMFY_KRITA_SETUP.md).

**09.11.2026 — ataki specjalne Kaldery:** własne warstwy Krita, cztery
tekstury z drugiego przebiegu DreamShaper 8 w lokalnym ComfyUI i ruch 2D
Blendera. Cztery ścieżki efektów, trzy gesty postaci, 12 różnych renderów
podglądu i atlas 4 MiB. PASS: 514 kontroli gry, 109 klatek z ukrytych scen.
[Źródła i odbiór](../volcano/attacks-20261109/README.md).

**09.11.2026 — lekcje Eldrina:** po pierwszym bossie płatna nauka ognia,
lodu i pioruna (600 / 1500 / 3000 złota). Własne 2D Krita i trzy pętle
Synfig, 180 rzeczywistych klatek 60 Hz. Nagrane prawdziwe łuki i podmuch
płomienia; Csound/Audacity, 21 wariantów, źródła CC0 i CC BY 3.0 z atrybucją.
50 Cue / 150 WAV; muzyka i ustawienia zachowane. Zapis v15 czyta v1–v14.
PASS: 234931 logiki/animacji, 10646 zapisu, 534 AudioSystem i 22 natywnego UI.
Praca cicha, testy gry ukryte. [Źródła i końcowy odbiór](../gameplay/eldrin-spells-20261109/README.md).

**09.11.2026 — przeturlanie i pozostałe przedmioty:** trzy warianty Roll
z nagrań tkaniny/skóry/podłoża, po 0,52 s, native Csound i Audacity.
Własne 23 ilustracje 2D w warstwowym KRA i lokalnym ComfyUI, z korektą
monet i sierści po przeglądzie. Drugi preloadowany atlas 1024 × 384,
1,5 MiB RGBA8; dotychczasowe 85 ikon sprzętu bajtowo zachowane.
Zapis v14, 45 Cue / 135 WAV, muzyka i preferencje bez zmian.
[Źródła audio](../audio/ground-roll-20261109/README.md),
[ikony i końcowy odbiór](../items/loot-20261109/README.md).

**Kula ognia, teleport, łuk i ikony HUD — 08.11.2026:** cztery nowe Cue,
12 wariantów Csound/Audacity, 45 Cue / 135 WAV łącznie. Wyrzut ognia oraz
eksplozja osobno; pojawienie po teleportacji słyszalne w odsłonięciu sceny;
fizyczny naciąg łuku zatrzymywany przy puszczeniu/anulowaniu/pauzie.
85 modeli ikon wyrenderowano rzeczywistym Blenderem 5.2.2 LTS, następnie
wykończono lokalnym ComfyUI 0.44 / DreamShaper 8 w ośmiu udanych zadaniach.
Preload jednego atlasu 1536 × 1024, 6 MiB RGBA8; runtime wyłącznie 2D.
PASS: 234864 kontrole logiki, 10244 zapisu i 482 obsługi audio bez sprzętu.
[Audio](../audio/flame-20261108/README.md), [ikony](../equipment/icons-20261108/README.md).

**Korekta audio 08.11.2026:** nagrane foley CC0 (Kenney, artisticdude,
Darsycho), filtracja i montaż Csound, rzeczywisty eksport i AUP3 Audacity.
Przebudowano 28 gestów walki / 84 warianty; cały bank ma 41 Cue / 123 WAV.
Osobne brzmienia typów wrogów, rozbójnika i mocnego ciosu; zwykły cios wroga
brzmi przy uderzeniu. Kanały domyślnie włączone, muzyka 85%, efekty 95%,
master 90%; menu/pauza 65%. Muzyka po korekcie: −16 LUFS (wieża −17),
finalne OGG −2,05 do −1,90 dBTP. Użytkownik jawnie polecił włączyć dźwięk;
lokalne konfiguracje są włączone, późniejsze wybory nadal się zapisują.
PASS: 234852 kontroli logiki/animacji, 10202 zapisu i 435 AudioSystem/
dekodera. Zapis v13 czyta v1–v12; autentyczny fixture v12 zachowuje cały
payload. Praca i testy bez playbacku, gra ukryta. [Źródła i odbiór](../audio/physical-20261108/README.md).
Końcowe paczki PASS: 253 zasoby każda, gracz 180 klatek i 435 kontroli audio,
DEV 3 × 180 klatek krain i osiem kontroli Opcji. Zatwierdzone preferencje
zachowane; końcowe licencje/instrukcje i wszystkie wpisy ZIP zweryfikowano.

**Aktualizacja map 08.11.2026:** wszystkie trzy krainy 72 × 72 → 102 × 102,
czyli 2,00694 × dawna powierzchnia. Przeciwnicy: las 91, pustynia 55,
Kaldera 61; teren: 490 / 487 / 209 obiektów, łącznie 88 wzniesień.
Rozszerzona sieć dróg i cztery nowe przeprawy, przedłużona lawa i urwiska.
Istniejące zasoby 2D są ponownie użyte; bufory podłoża 6270 × 3285 mieszczą
się w limicie 8192 i zajmują około 8,7% więcej pamięci niż poprzednie.
Zapis v12 migruje v1–v11 z zachowaniem pierwotnych mieszkańców, ciał, łupu,
przedmiotów, postępu, pozycji i RNG. PASS: 234833 kontroli logiki/animacji,
10157 zapisu, trzy pomiary projekcji i 184 renderera. Nowe spotkania i dalekie
krańce map osiągalne przez rzeczywiste kolizje. 21 podglądów, cicho i w tle.
Ukończona aktualizacja + render: średnia 8,47 / p95 10,62 ms; bez deklaracji
foreground FPS. [Źródła, przegląd i odbiór paczek](../world-expansion/README.md).
Gotowe EXE i oba ZIP-y sprawdzone po wypakowaniu: po 228 zgodnych zasobów,
gracz — 180 klatek startu i ponownie 184 kontrole rozbudowy, DEV — 3 × 180
klatek krain. Okna ukryte, audio wyłączone argumentem, preferencje zachowane.

**Aktualizacja audio 08.11.2026:** cztery nowe, 32-taktowe kompozycje LMMS
dla lasu/pustyni/Kaldery/wieży, z płynną zmianą regionu. 33 efekty Csound
w trzech wariantach (99 WAV), obejmujące walkę obu stron, skrzynie,
broń/ekwipunek, zbieranie przedmiotów i turlanie oraz zdolności specjalne.
Obróbka Audacity: pogłos muzyki, normalizacja/DC; pięć rzeczywistych AUP3
z kontrolą SQLite i pozostawieniem otwartych projektów, bez Close/playback.
Całość lokalna, bez obcych sampli. Stereo 48 kHz; muzyka Vorbis q5,
−19 LUFS (wieża −20), PCM16 efekty. PASS: 200189 kontroli logiki/animacji,
5162 zapisu i 353 cichego odbioru AudioSystem z rzeczywistym dekoderem raylib
i wyjściem w pamięci. Nie otwierano urządzenia, nie oceniano przez odsłuch.
Mastery, pochodzenie i końcowy odbiór paczek: [oprawa dźwiękowa](../audio/suite-20261108/README.md).
Gotowe EXE i obie paczki ZIP przeszły weryfikację po wypakowaniu: po 228
zgodnych zasobów, gracz — 180 klatek startu i ponowne 353 kontrole audio,
DEV — 3 × 180 klatek krain. Osiem kontroli rzeczywistego menu Opcji — PASS,
ukryte i bez urządzenia. Cztery finalne OGG mają −18,97 do −19,98 LUFS
i −5,12 do −3,73 dBTP. Zapisane wyciszenie obu kanałów użytkownika zachowane.

**Aktualizacja 08.11.2026 — zdolności silnych gatunków:** kobra piaskowa
otrzymała Jadowity wachlarz, skarabeusz miedziany Miedzianą falę,
obsydianowy wojownik Rozłupujący cios, a żarowy drak Żarowy oddech.
Każdy atak ma zapowiedź, stały kierunek i odpoczynek; działa unik oraz osłona.
Efekty 2D i znaczniki korzystają z istniejących postaci i klipów Synfig,
bez nowej produkcji bitmap lub modeli. Zapis v11 zachowuje fazy i pociski,
odczyt v1–v10 pozostaje dostępny. Logika/animacje: 200119 kontroli,
zapis: 5162, renderer: 497 (cztery zdolności, osiem kierunków, 105 obrazów,
cztery spotkania w oryginalnej scenografii) — PASS, cicho i w tle.
Oba ZIP-y po wypakowaniu: zgodne EXE i po 147 zasobów; gracz — 180 klatek
startu i ponownie 497 kontroli zdolności, DEV — 3 × 180 klatek. Audio nie
zostało otwarte, preferencje użytkownika zachowane; bez pomiaru foreground FPS.
[Opis, podgląd i końcowy odbiór paczek](../gameplay/elite-abilities/README.md).

**Aktualizacja 08.11.2026 — 44 formy terenu i ciągłe drogi Kaldery:**
las 14 / pustynia 14 / Kaldera 16, o różnych wysokościach i promieniach.
Łączą się w grzbiety i łagodne siodła. Ścieżka otrzymała nowy, czterowarstwowy
materiał z lokalnej Krity i wspólną maskę całej wygładzonej sieci.
Miękkie brzegi i skrzyżowania nie nakładają osobnych pasów. Poprawione
mieszanie alfa podczas wypieku zachowuje pełne krycie gruntu i usuwa smugi
od odsłanianego tła. Mosty i przepaść zachowane; zapis bez migracji.
PASS: 1584 kontrole wysokości/dróg, 1063 renderera Kaldery i 13 źródeł KRA/PNG.
Logika i animacje: 200013 / zapis: 4513 / obsługa skrzyń: 115 — PASS.
Wszystkie testy ciche i ukryte. Ukończony render: średnia 4,11 / p95 4,85 ms;
nie jest to pomiar foreground FPS. [Źródła, podglądy i odbiór](../terrain/diversity-20261108/README.md).
Gotowe EXE i oba ZIP-y sprawdzono po wypakowaniu: po 147 zgodnych zasobów,
gracz — 180 klatek startu i 1584 kontrole podłoża/dróg, DEV — 3 × 180 klatek.
Audio wyłączone w testach, okna ukryte, preferencje użytkownika zachowane.

**Aktualizacja 08.11.2026 — dostępne wzniesienia trzech krain:** 13 profili
wysokości (las 4 / pustynia 4 / Kaldera 5), z łagodnymi wejściami i zejściami.
Ruch bohatera i przeciwników mierzy odległość po powierzchni. Podłoże,
obiekty, cienie, kamera i odwrotny rzut kursora korzystają z tej samej
wysokości. Istniejące ilustracje są rysowane na podniesionej siatce 2.5D;
bez nowych modeli, tekstur lub migracji zapisów. Wszystkie szczyty mają
dostęp z równiny przy rzeczywistej scenografii i kolizjach. PASS: 564
kontrole wysokości/ruchu/renderera, 1063 regresji Kaldery i 115 obsługi
skrzyń. Testy ukryte i ciche, preferencje audio zachowane.
Ukończony render próby wysokości: średnia 4,06 ms / p95 4,47 ms;
nie jest to pomiar FPS widocznego okna. [Podglądy i odbiór](../terrain/hills/README.md).
Logika: 199995 / zapis: 4513 — PASS. Zaktualizowane EXE oraz oba ZIP-y
przeszły odbiór po wypakowaniu (147 zgodnych zasobów w każdej paczce,
gracz: 180 klatek startu i 564 kontrole renderera, DEV: 3 × 180 klatek).

**Aktualizacja 08.11.2026 — przepaść i skrzynie krain:** kamienne obrzeże
zastąpiono urwiskiem schodzącym w mgłę. Powierzchnia kończy się na brzegu;
tło ma warstwy odległych skał, popiół i żar, a rzeki spływają do otchłani.
Trzy style skrzyń, po trzy stany: drewno i mech / piaskowe złoto / obsydian
i żar. Nowe źródła wykonano lokalnie w oficjalnej przenośnej Kricie 5.3.4,
przez natywne Python/Qt i kritarunner, bez nowego MCP, chmury i obcych grafik.
Zapisane projekty worker zachowuje w tle. PASS: 1063 kontroli renderera,
115 obsługi skrzyń i 31 niezależnych kontroli zasobów. Testy ukryte i ciche.
[Źródła, podglądy i raporty](../volcano/abyss/README.md).

**Aktualizacja 08.11.2026 — granice i teren Kaldery:** ciągłe skalne obrzeże
przy granicy ruchu, przyciemnione zewnętrze i kontur minimapy. Istniejące
ilustracje Krity wypiekane są raz w tym samym buforze; bez nowych zasobów GPU.
Nowe rozmieszczenie ogranicza tłok i puste obszary: minimalny odstęp
1,77 → 4,45 pola, największy pusty promień próbki 13,53 → 4,97 pola.
Zachowano przejścia i interakcje. Rewizja 4 aktualizuje także starsze zapisy,
z zachowaniem pozostałych danych bajt w bajt. PASS: 199995 kontroli logiki,
4513 zapisu i 820 renderera. Testy ciche i ukryte.
[Opis, podglądy i wyniki](../volcano/layout/README.md).

**Aktualizacja 07.11.2026 — jednolite włosy bohatera:** usunięto jasną
plamę nad czołem, dopasowano kolor przednich i tylnych pasm do korony,
zachowując ich delikatny detal. Korekta istniejącego materiału jest wykonywana
raz przy wczytywaniu; źródłowy PNG i KRA są niezmienione. 428 kontroli
renderera i 25 niezależnych kontroli obrazów — PASS. Wszystkie okna ukryte,
bez urządzenia audio. [Porównanie i raport](../hero/hair/README.md).

**Aktualizacja 04.11.2026 — przeciwnicy Kaldery / ComfyUI:** na wyraźne
zamówienie użytkownika i po potwierdzeniu lokalnego lekkiego modelu wykonano
56 rzeczywistych generacji img2img w ComfyUI 0.41.3 / DreamShaper 8.
Nowe malowanie wszystkich sześciu gatunków i Varkhora jest rzutowane na
istniejące części animowane przez Synfig. Osiem kierunków, dokładna alfa,
kotwice i stan rozgrywki są zachowane. Widoczne detale obejmują rysy twarzy,
łuski, błony skrzydeł, pancerze, fałdy szat i żar kamiennych szczelin.
PASS: 728 kontroli źródeł, grafów, wejść i atlasów oraz 787 renderera,
w tym 224 ujęcia chodu / bezruchu / ataku / szczątków. Testy pozostały
ukryte i ciche. Ukończony render: średnia 4,03 ms / p95 5,55 ms;
nie jest to pomiar foreground FPS. Atlasowe RGBA8 nadal 55 050 240 bajtów,
bez dodatkowych tekstur lub odczytu plików w klatce.
Zaktualizowano obie aktywne kopie zasobów i oba ZIP-y; każdy inny wpis
archiwum pozostał identyczny bajtowo, w tym oba pliki EXE. Rzeczywisty
EXE gracza przeszedł 180 klatek startu z zamkniętym audio i ukrytym oknem.
Źródła ComfyUI, oryginały i siedem warstwowych plików OpenRaster są
w `C:\AI2D\sprites\emberfall-caldera-comfy-20261104`. Na tym PC Krity
ani jej bridge nie ma, mimo historycznego opisu; nie deklarujemy zapisów KRA.
Pochodzenie modelu, komercyjne uprawnienia do obrazów, hashe, backupy,
workflow i wyniki: [ComfyUI](../volcano/comfy/README.md),
`../volcano/comfy/QA.json`. Nie użyto chmury ani płatnych węzłów.

**Aktualizacja 04.11.2026 — portal, ciągły grunt i ostrość:** cztery nowe
warstwowe KRA oraz natywny SIF. Kamienna brama ma runy i wirujący rdzeń,
120 klatek / 60 Hz / pętla 2 s. Grunt jest zawiniętą fakturą bez ciemnych
brzegów kafli, a duże pola popiołu mają miękkie przejścia. Poprawiono
podwójne mnożenie gotowej klatki przez alfę; obraz końcowy zachowuje RGB
piksel w piksel w DXGI i OpenGL. Kaldera korzysta z natywnego bufora świata.
Monitor 1920×1080 / 60 Hz, DPI 125%; rzeczywisty V-sync interval 1.
PASS: 293 kontrole źródeł/eksportów/prezentacji, 787 renderera, 194309
logiki i 4090 zapisu. Finalne paczki sprawdzono po wypakowaniu: po 143
zasoby; gracz — 180 klatek i 787 kontroli, DEV — 540 klatek trzech lokacji.
Testy ukryte, bez audio, preferencje zachowane. Ukończony render gracza:
średnia 5,53 ms / p95 6,17 ms; nie jest to pomiar FPS widocznych na ekranie.
Portal dodaje 14,07 MiB tekstur i nie generuje zasobów w klatce.
[Źródła, porównania i raport](../volcano/gateway/README.md),
`../volcano/gateway/QA.json`. Nowsze podłoże zastępuje dwa eksporty
terenowe z poprzedniej produkcji Krita/Laigter.

**Aktualizacja 04.11.2026 — przeciwnicy i otoczenie Krita/Laigter:**
6 gatunków, Varkhor i 32 odmiany scenografii korzystają z bardziej
szczegółowych natywnych ilustracji. Dopracowano 12 KRA; Laigter 1.14.0
wygenerował 48 map z profilem HeightMap. Oświetlenie wypalono w PNG,
z zachowaniem żaru i alfy, bez dodatkowych tekstur/przebiegów w grze.
Dodano osady mineralne, pory, odpryski, łuski i duże pola zastygłych potoków.
Poprawiono też głębokość rysowania miecza za hełmem podczas zamachu.
PASS: 811 kontroli źródeł/map/eksportów i 708 renderera, w tym 224 pozycje
w ośmiu kierunkach. Finalny ZIP gracza wypakowano i sprawdzono: 139 zasobów,
180 klatek startu oraz pełny test 708 kontroli. DEV: 139 zasobów i 540
klatek w trzech punktach Kaldery. Wszystkie testy gry ukryte, bez audio;
preferencje dźwięku pozostały niezmienione. Ukończony render paczki gracza:
średnia 4,41 ms / p95 4,84 ms; nie jest to foreground FPS ani pomiar V-sync.
12 zastępowanych tekstur zajmuje przed i po 82 014 208 bajtów RGBA8.
Źródła, profile, podglądy i wyniki: `../volcano/materials/README.md`
oraz `../volcano/materials/QA.json`. Starsze raporty dotyczą poprzedniej grafiki.

**Aktualizacja 04.11.2026 — koryta lawy, teren i zamach Wojownika:**
ukończono natywne materiały oraz integrację rozpoczętą 30.10. Lawa płynie
poniżej gruntu, między skalnymi ścianami, i wychodzi poza obie granice mapy.
Nowe powierzchnie, brzegi oraz 16 wariantów głównej scenografii pochodzą
z Krity; duży zamach Wojownika w ośmiu kierunkach i nurt lawy — z Synfiga.
Finalne źródła: 6 KRA, 9 SIF, 480 klatek ataku i 60 klatek lawy. PASS:
790 kontroli natywnych źródeł/eksportów, 708 renderera, 194309 logiki
i 4090 zapisu. Zaktualizowano EXE i oba ZIP-y. Wypakowana paczka gracza
przeszła 180 klatek startu i pełne 708 kontroli; DEV — 540 klatek w trzech
punktach Kaldery. Sprawdzono po 139 zasobów każdej paczki, brak natywnych
masterów w ZIP-ach i niezmienione preferencje audio. Wszystkie okna testów
były ukryte, audio wyłączone argumentem. Ukończony render paczki gracza:
średnia 4,79 ms, p95 5,18 ms; nie jest to pomiar foreground FPS ani V-sync.
Ściana dodaje 0,5 MiB tekstury bez kolejnego bufora mapy i bez I/O w klatce.
Źródła, podglądy i raport: `../volcano/depth/README.md`, `../volcano/depth/QA.json`.

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

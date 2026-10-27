# Zachodni start i Kamienny Strażnik

Nowa gra używa wspólnego `ForestArrival = {MapWalkMin, MapWalkMax}`:
(1,6; 69,4), skrajnie lewego dostępnego narożnika rzutu izometrycznego x−y.
Prolog i deweloperska pozycja `forest` odwołują się do tego samego punktu.
Nowa ścieżka prowadzi do polany (14;61), a dawna przesieka zachowuje układ.
Jeden najbliższy wilczy patrol przesunięto z (10;68) na (12;68), aby cały
zasięg patrolowania i widzenia pozostawał poza bezpiecznym początkiem.
Boss nadal stoi w (21;5), ponad 67 pól od początku. Starsze zapisy zachowują
swoją pozycję, ekwipunek i postęp; nie teleportują postaci podczas wczytania.

## Rzut kamieniem

Wyłącznie żywy Kamienny Strażnik w lesie. Pierwsze przygotowanie po 0,7 s
widocznej walki, następne po 4,5 s od rzutu. Cel musi być widoczny, bez
przeszkód i w odległości 2,5–6,5 pola. Z bliska pozostaje dotychczasowa walka.

- Przygotowanie 2,2 s, oznaczenie miejsca na ziemi; cel zostaje ustalony.
- Boss przykuca i chwyta obiema rękami głaz tkwiący częściowo w ziemi.
  Wyrywa go z grudkami ziemi i odłamkami, unosi przed twarzą, potem nad głowę.
  Obie dłonie trzymają dolne boki tej samej skały; ręce mijają głowę po bokach.
- Wypuszczenie znad głowy, wspólny środkowy punkt głazu, 0,6 s dokończenia
  zamachu i powrotu. Tor podnoszenia zachowuje prześwit przed głową.
- Lot po łuku ku oznaczonemu miejscu, 7 pól/s. Brak samonaprowadzania.
- Uderzenie raz w promieniu 0,72 pola: 24 obrażenia fizyczne przed pancerzem.
  Można zejść z celu lub skorzystać z pierwszych 0,28 s przewrotu.
- Przecięcie odcinka lotu z przeszkodą zatrzymuje kamień. Osłona blokuje
  obrażenia, również gdy pojawi się już po rozpoczęciu rzutu.
- Śmierć bossa usuwa jego pociski. Zmiana krainy i teleport do wieży anulują
  przygotowanie i pociski. Pauza/menu i rozdział punktów zamrażają walkę.
- Maksymalnie osiem kamieni i 24 krótkie efekty uderzenia; zwykle jeden kamień
  w powietrzu. Jedna tekstura 192×192 (144 KiB RGBA8), rozmiar w grze 76 px
  zamiast 24 px. Wczytywana przy `makeArt`.
  Bez wczytywania/generowania zasobów w klatce. Użyto istniejących dźwięków.

## Darmowe lokalne źródła

`guardian-stone.kra`, `stone-provenance.json`: własny granit, mech, runa,
grudy ziemi i urwane korzenie, sześć warstw + przezroczysta baza, natywne
Krita MCP. Runtime:
`assets/sprites/guardian-stone.png`. Master:
`C:\AI2D\sprites\emberfall_guardian_overhead_20261023`.

`motion/guardian-throw.sif`: własny natywny cutout 2D, lustrzane ramiona,
pięć par kontrolerów dłoni, łokci, rozstawu chwytu, głazu i tułowia.
168 rzeczywiście wyrenderowanych klatek Synfig przy 60 Hz, 169 próbek
zapisanych w SIF i skompilowanych do `src/generated/guardian_throw_data.hpp`.
Wyrwanie w klatkach 24–42, unoszenie 42–102, chwyt nad głową 102–117,
wypuszczenie w 132, spoczynek w 0 i 168. Projekt to cutout i kontrolery 2D;
gra stosuje zapisane punkty do istniejącej artykulacji bossa.
Nie tworzono modelu 3D ani osobnego zestawu obrazów dla kolejnych klatek bossa.

Łokcie zachowują kierunek zgięcia spoczynkowego przez cały gest i powrót.
Wybrano właściwą stronę rozwiązania dwuczłonowego ramienia oraz skorygowano
ustawienie barków w fazie unoszenia, aby staw nie blokował się w wyproście.
673 pozycje klatek i podklatek mają dodatnie zgięcie oraz bezpieczny zakres
ugięcia. Dłonie, głaz, chwyt i chwila wypuszczenia zachowują dotychczasowy tor.

Master Synfig i sekwencja:
`C:\AI2D\animations\emberfall_guardian_elbows_20261023_v2`.
Produkcja (nowe katalogi, brak nadpisywania):

```powershell
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/create_guardian_stone.py --root C:\AI2D\sprites\stone_new --no-audio
& $py tools/media/guardian_throw_synfig.py --root C:\AI2D\animations\throw_new --stone C:\AI2D\sprites\stone_new\guardian-stone.png --header src/generated/guardian_throw_data.hpp --no-audio
```

## Zapis i testy

### Rozpad głazu przy kontakcie z ziemią

`shatter/guardian-fragments.kra` i `assets/sprites/guardian-fragments.png`:
sześć oryginalnych malowanych fragmentów Krity, atlas 288×192 (komórki 96 px),
216 KiB RGBA8. Wyróżniają je świeże powierzchnie pęknięć i pozostałości mchu
oraz ziemi. Poszczególne części składają się w sylwetkę głazu.

`shatter/guardian-shatter.sif`: 1,2 s natywnej animacji Synfig, 72 rendery
przy 60 Hz i 438 zapisanych próbek sześciu odłamków. Każdy ma osobny tor,
obroty, pierwszy upadek, drobne wtórne odbicie, osiadanie i zanik. Dolna
krawędź obracającego się fragmentu styka się z podłożem; kawałki nie lewitują.
`src/generated/guardian_shatter_data.hpp` pochodzi z odczytanego zapisu SIF.

Przy trafieniu znika pełny pocisk i pojawia się dokładnie jeden rozpad.
Każdy większy odłamek ma własny cień i miejsce w sortowaniu świata, więc
przechodzi przed lub za postacią zgodnie z położeniem na ziemi. Drobny gruz
i krótki pył uzupełniają efekt. Fragmenty są kosmetyczne: nie zadają kolejnych
obrażeń, nie blokują ruchu i nie przechwytują kursora. Pauza zamraża animację,
a zapis/odczyt wznawia jej fazę. Licznik efektu pozostaje w zakresie 0–0,4;
jest skalowany do długości nowego klipu.

Master Krity: `C:\AI2D\sprites\emberfall_guardian_fragments_20261023`.
Master Synfiga: `C:\AI2D\animations\emberfall_guardian_shatter_20261023`.
Produkcja w nowych katalogach:

```powershell
& $py tools/media/guardian_fragments.py --root C:\AI2D\sprites\fragments_new --no-audio
& $py tools/media/guardian_shatter_synfig.py --root C:\AI2D\animations\shatter_new --atlas C:\AI2D\sprites\fragments_new\guardian-fragments.png --provenance C:\AI2D\sprites\fragments_new\provenance.json --header src/generated/guardian_shatter_data.hpp --no-audio
```

Weryfikacja `verify_guardian_shatter.py` jest włączona do
`verify_guardian.py` i testu rozpakowanej paczki. Bada KRA/PNG, wszystkie
próbki SIF, kontakt obróconych fragmentów z ziemią, rendery i rzeczywiste
klatki gry po trafieniu oraz uniku. Grafika i animacja rzucającego bossa,
w tym poprawiona praca łokci, korzystają z dotychczasowych źródeł.

### Odrzut bohatera

Uderzenie w ziemię odrzuca żywego bohatera w promieniu 1,6 pola. Kierunek
jest radialny od punktu upadku; przy trafieniu dokładnie w środek używany
jest kierunek lotu kamienia. Droga wynosi 1,35 pola przy środku i maleje
do 0,54 pola na skraju. Przesunięcie trwa 0,48 s i łagodnie wyhamowuje. Pierwotny
promień obrażeń 0,72 pola i pancerz pozostają bez zmian; sama fala odrzutu
nie zadaje kolejnych obrażeń. Chroniona faza uniku unieważnia również odrzut.

Impuls przerywa napinanie łuku, zamach i niechronioną końcówkę przewrotu.
Aktualny klip Synfiga odrywa stopy od ziemi i odrzuca bohatera na plecy.
Leży on twarzą do góry od 0,66 do 1,03 s, obraca się na bok, podpiera dłonią,
przechodzi przez pozycję na kolanie i powoli wstaje. Całość trwa 2,8 s;
sterowanie wraca po wstaniu. Nie można jej skasować atakiem lub ruchem. Broń,
pancerz, cień i pozostałe elementy pozostają na tym samym rigu; portret
zachowuje stojącą pozę. Chowanie miecza jest zamrożone podczas leżenia.
Źródło: `art/hero/knockdown`. Bohater nie odgrywa chodu ani kroków podczas
przesuwania. Ruch jest całkowany
niezależnie od FPS i dzielony na kroki nie większe niż 0,04 pola; istniejące
kolizje drzew, skał, skrzyń oraz granic mapy zatrzymują go lub pozwalają
ześlizgnąć się wzdłuż przeszkody. Osłona blokuje falę. Pauza zamraża impuls,
śmierć i zmiana lokacji go kasują. Nie dodano zasobów ani odczytów w klatce.

Format 2 dodał timery/cel rzutu każdego przeciwnika oraz kamienie w locie
i efekty uderzenia. Format 3 zapisał wektor i pozostały czas przesunięcia.
Format 4 dodał czas upadku/wstawania i kąt patrzenia. Format 5 wydłuża zegary;
podczas odczytu starszych plików ich faza jest przeliczana proporcjonalnie.
Czytnik obsługuje formaty 1–5, nadal weryfikuje sumę,
zakresy i prywatnego kandydata przed podmianą sesji. Zapis v1 nie ma nowych
pól: otrzymuje bezpieczny czas pierwszego rzutu i puste pociski. Rzeczywisty
plik `fixtures/legacy-v1.efs` powstał przed zmianą serializatora, z osobnym
programem i bez dostępu do zapisów użytkownika. `fixtures/legacy-v2.efs`
pochodzi z testu poprzedniej paczki gracza, sprzed dodania odrzutu;
SHA-256 `023b182f97f8dd13c592acc03400dc5546a17118dc0c1a5a564b20e527eaf178`.
`fixtures/legacy-v3.efs` pochodzi z rzeczywistego odrzutu w poprzedniej paczce;
SHA-256 `7d9efe5655bade45cc50d10842e0826b8d1343a71f800bde6294ad20503839e3`.
Pliki v1–v3 zachowują swoje pola i otrzymują zerową fazę upadku. Nowsze
nieobsługiwane wersje pozostają odrzucane. Poprzedni EXE nie obsługuje zapisów
v5. `fixtures/legacy-v4.efs` pochodzi z poprzedniej paczki podczas upadku;
SHA-256 `ee54e734eb807f8b9fce039fea6e07738a32d83aaf39ddbc150c5ab7209c10bb`.
Nie odczytywano ani nie nadpisywano zapisów użytkownika.

Testy: `build.ps1 -Test`, `tests/guardian_qa.ps1 -NoAudio`,
`tools/media/verify_guardian.py --qa <folder> --no-audio`,
`tests/guardian_package_qa.ps1 -NoAudio`. Raport: [QA.md](QA.md).

# Żarowy Drak

Drak poluje przy ziemi. Miedziane błony skrzydeł kontrastują z chłodnymi płytkami grzbietu. Jasne rogi, kły i gardziel wyznaczają kierunek ataku. Przed zionięciem drak nabiera powietrza i rozstawia skrzydła. Przy ugryzieniu najpierw cofa szyję, a potem wysuwa pysk i zamyka żuchwę.

## Źródła

- [Postać w Kricie](zarowy-drak.kra) ma sześć warstw w ośmiu kierunkach. Osobno namalowane są skrzydła, przekątne pary łap, opancerzony tułów z ogonem oraz szyja z głową.
- [Pysk w Kricie](ember-drake-mouth.kra) zawiera górną szczękę, gardziel z kłami i żuchwę. Osiem stopni otwarcia przypada na każdy z pięciu widoków, w których widać zęby.
- [Materiały referencyjne](references) zawierają ilustracje wygenerowane wbudowanym narzędziem, opisy ujęć i oryginalny wzór płomienia. W [katalogu ComfyUI](references/comfy) są lokalny render DreamShaper 8, graf, identyfikator zadania, suma wag i źródło warunków licencji. Fragmenty membrany i łusek posłużyły do malowania faktur w Kricie.
- Projekty Synfig otwierają się z dołączonym katalogiem `motion/parts`. Dostępne są [spoczynek](motion/idle.sif), [chód](motion/walk.sif), [przechył w zakręcie](motion/bank.sif), [ugryzienie](motion/bite.sif), [Żarowy Oddech](motion/breath.sif) i [upadek](motion/fall.sif).

## Edycja i integracja

1. Po edycji KRA wyeksportuj oba arkusze jako przezroczyste PNG w natywnych wymiarach. [Pakowanie draka](../../../tools/media/pack_ember_drake.py) przygotowuje materiał wygaszonego ciała i sprawdza marginesy wycinanek. [Podział malowanych ujęć](../../../tools/media/ember_drake_cutouts.py), [pędzle materiałowe Krity](../../../tools/media/krita_ember_drake.py) i [wycinanka żuchwy](../../../tools/media/ember_drake_jaw.py) pozwalają powtórzyć etap produkcyjny w nowym katalogu roboczym.
2. Po zmianie kluczy ruchu w istniejącym SIF uruchom z katalogu repozytorium `python tools/media/ember_drake_synfig.py art/volcano/ember-drake/motion --header src/generated/ember_drake_motion.hpp --no-audio`. Dodanie `--render` uruchomi Synfig i sprawdzi osiem widoków, kompletność sekwencji i zamknięcie pętli.
3. Gra ładuje postać z `assets/volcano/ember-drake.png`, otwarty pysk z `assets/volcano/ember-drake-mouth.png`, a materiał szczątków z `assets/volcano/corpses/ember-drake.png`. Jasność płyt i oczu na ziemi zmienia się wraz z osiadaniem ciała. Faza kroku pochodzi z przebytej odległości. Współrzędne żaru zależą od kierunku, kąta szyi i otwarcia szczęki.
4. Struga zajmuje drugą komórkę istniejącego atlasu `assets/volcano/caldera-attack-fx.png`. Do jej odtworzenia wytnij z `references/flame-source.png` prostokąt (23, 38, 1826, 789), zmniejsz do 508 × 508 px z filtrem Lanczosa i umieść na przezroczystej komórce 512 × 512 px z marginesem 2 px. W grze płomień wygina się pomiędzy pyskiem a stożkiem trafienia. Przy zionięciu w głąb krainy dalszą część pyska zasłaniają rogi i barki.
5. W projekcie `bank.sif` środkowa klatka to pozycja neutralna. Skrajne pozy przechylają tułów i skrzydła w lewo lub prawo. W grze stopień przechyłu wyznacza prędkość obrotu. Obrót nakłada się na aktualny krok, dzięki czemu oparta łapa zachowuje fazę podporu. Aby zmienić szybkość obrotu, edytuj `advanceVolcanoAnimation` w `src/volcano_system.hpp`.

## Grafika obrotu

[Arkusz obrotu w Kricie](turn/drake-turn.kra) układa widoki w czterech rzędach po osiem. Każdą pozę można poprawić na warstwach skrzydeł, łap, pancerza z ogonem i głowy. Przy widoku od tyłu głowa ma własną warstwę za pancerzem. Rozmiar pola to 160 × 160 pikseli.

1. Skopiuj KRA do `C:\AI2D\sprites`, popraw i zapisz w Kricie. Z katalogu repozytorium wykonaj `python tools/media/ember_drake_turn_krita.py export KOPIA_KRA NOWY_KATALOG --no-audio`. Skrypt eksportuje części z natywnych warstw do podkatalogu `cells`.
2. W PowerShellu wykonaj `& "$env:LOCALAPPDATA/Comfy-Desktop/ComfyUI-Installs/Emberfall/standalone-env/python.exe" tools/media/ember_drake_turn.py pack KATALOG_ROBOCZY --edited-cells NOWY_KATALOG/cells --no-audio`. Interpreter lokalnego ComfyUI zawiera SciPy. [Skrypt](../../../tools/media/ember_drake_turn.py) pakuje części razem z ujęciami otwartej paszczy. Po zmianie granic nieprzezroczystych pikseli przebuduj także grę, ponieważ współrzędne wycinków są częścią wygenerowanego nagłówka C++.

[Obrót Draka w grze](turn/obrot.gif) · [Porównanie przed i po](turn/qa.json) · [Próba kierunków i płynności](../../../tests/ember_drake_turn_qa.hpp) · [Graf ComfyUI FILM](turn/film-workflow.json)

[Podgląd ugryzienia i oddechu](preview.gif) · [Test renderera](../../../tests/ember_drake_art_qa.cpp) · [Pierwotne kontrole ośmiu malowanych ujęć](qa.json)

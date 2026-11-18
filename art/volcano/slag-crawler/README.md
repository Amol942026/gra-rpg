# Żużlowy Pełzacz

Pełzacz z Kaldery ma niski profil, cztery krótkie nogi i ogon z zachodzących na siebie płyt. Przy zwrocie bokiem widać jaśniejsze krawędzie bazaltowego pancerza i bursztynowe oko. Od przodu głowę wyróżnia jasny, klinowaty pysk. Skala modelu w grze wynosi 1,0. Przyjęliśmy dotychczasowy obrys Pełzacza jako wzorzec wielkości.

## Grafika

Pięć [oryginalnych ujęć](references/sunburst-turnaround.png) z wbudowanego generatora posłużyło do opracowania ośmiu kierunków bazowych. [Punkty dopasowania](paint-markers.json) wyznaczają położenie oczu, pyska i płyt pancerza. Przygotowanie kierunków pośrednich łączy rejestrację elementów głowy z interpolacją pozostałych części przez lokalny model FILM w ComfyUI. [Projekt Krity](pelzacz.kra) zawiera końcową grafikę wszystkich 32 kierunków, w tym ręcznie wskazane korekty oczu. Eksport daje 192 wycinki, po sześć na kierunek. Z nich powstają atlas obrotu, podstawowa plansza postaci i materiał ciała po śmierci.

## Ruch

Siedem [projektów Synfiga](motion) zawiera oddech, chód, ugryzienie, przechył, upadek i dwa kierunki obrotu w miejscu. Chód zależy od przebytej drogi. Przy skręcaniu w miejscu łapy unoszą się według przebytego kąta. Przy ugryzieniu przednie łapy przejmują ciężar, a pysk osiąga pełne wysunięcie w chwili zadania obrażeń. Po trafieniu kończyny składają się kolejno i pozostają związane z korpusem podczas osiadania.

## Edycja i sprawdzenie w grze

1. Edytuj warstwy w `pelzacz.kra`. Wyeksportuj komórki przez `tools/media/slag_crawler_krita.py export <plik.kra> <nowy_katalog> --no-audio`. Most Krity korzysta z katalogu roboczego pod `C:\AI2D`.
2. Spakuj je przez `tools/media/slag_crawler_paint.py` z plikiem referencyjnym, punktami dopasowania, nowym katalogiem roboczym i opcją `--edited-cells` wskazującą wyeksportowane komórki. Opcja `--output` wskazuje katalog docelowy atlasu, a `--header` ścieżkę nagłówka z tabelą wycinków. Dodaj `--no-audio`. Dane modelu i przebiegu zapisano w [metryce grafiki](paint/provenance.json). Przed podmianą zachowaj kopię działających zasobów.
3. Po edycji ruchu w Synfigu uruchom `tools/media/slag_crawler_motion.py art/volcano/slag-crawler/motion --header src/generated/slag_crawler_motion.hpp --renders <nowy_katalog> --no-audio`. Skrypt odczytuje krzywe SIF, renderuje każdy projekt i mierzy przesuwanie podpartych łap. Opcja `--cells` służy do ponownego zbudowania projektów po zmianie anatomii.
4. Test pełnego obrotu uruchom przez `pwsh -File tests/ember_drake_turn_qa.ps1 -Creature crawler -Case slag-crawler-turn -NoAudio`. Obejmuje chód, zmianę kierunku, atak, zaznaczenie, upadek i cień.

[Animacja z renderera gry](obrot.gif) · [Wyniki pomiarów i kontroli wydania](qa.json)

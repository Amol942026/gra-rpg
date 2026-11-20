# Ognisty chochlik

Bieżący model i animacje opisano w [projektach wycinankowych Kaldery](../puppets/README.md). Poniżej zachowano źródła poprzednich atlasów.

Chochlik ma zakrzywione, warstwowe rogi, bursztynowe oczy, miedzianą łuskę, skórzany pas i długi ogon z kamiennym grotem. Pięć [ilustracji kierunkowych](redesign/references/sunburst-turnaround.png) wyznacza kształt sylwetki. Program przygotowuje osiem podstawowych ujęć i rozdziela ogon, nogi, tułów z głową oraz ręce. Pod stawami zostaje zapas pomalowanej skóry. Punkty dopasowania źrenic, rogów i zębów prowadzą obrót głowy. Lokalny model FILM interpoluje fakturę kończyn między ujęciami. Przebieg ogona wyznacza krzywa namalowana od nasady do grotu na każdej ilustracji kierunkowej.

[Projekt Krity](paint/chochlik.kra) zawiera 32 ujęcia w sześciu warstwach. Do gry trafiają 192 wycinki, podstawowy atlas i materiał wygaszający blask skóry po śmierci. Prostokąt ogona podąża za jego krzywizną. Pozostałe części zachowują połowę wspólnej ramki podczas obrotu, co stabilizuje proporcje sylwetki. [Punkty stawów i podział ilustracji](paint/landmarks.json) podają współrzędne w komórkach 160 × 160. Punkty obu dłoni i stawów ramion prowadzą formowanie ognia.

[Cztery pliki Synfiga](motion) przechowują tory spoczynku, kroku, rzutu i upadku. Chochlik przenosi ciężar między kopytami. Ogon pozostaje w tyle za miednicą, po czym podąża za jej ruchem. Przez 0,66 s chochlik zbiera dłonie przed sobą i formuje między nimi wirującą kulę ognia. Krótkie wypchnięcie obu ramion uwalnia kulę w 45% klipu. Pocisk celuje w tułów bohatera. Przy trafieniu jego płomień rozrywa się na języki ognia i iskry. Podczas upadku kolana uginają się kolejno, tułów opada na bok, ogon kończy ruch przy kontakcie z ziemią.

Obrót opiera się na 16 malowanych ujęciach. Cztery dodatkowe ilustracje oraz ich lustrzane odbicia uzupełniają główne kierunki. Położenia, mocowania i obrysy sześciu części płynnie przechodzą między pozami. Każdy detal pochodzi z jednego malowanego ujęcia. Ręce, nogi i ogon poruszają się także zgodnie z animacją akcji. [Warstwy dodatkowych ujęć w Kricie](paint/inbetweens/polobroty.kra) · [Arkusz źródłowy](paint/inbetweens/sunburst.png) · [Pochodzenie i prompt](paint/inbetweens/source.json)

Generator zapisuje atlas `directional-cels.png` i współrzędne wycinków. Do ponownego przygotowania plików użyj Pythona z Pillow, NumPy, SciPy i OpenCV.

```powershell
& $mediaPython tools/media/fire_imp_bake_cels.py assets/volcano/imp-turn/directional-cels.png src/generated/fire_imp_cels.hpp --painted-only --no-audio
```

Po przygotowaniu malowanych atlasów obu przeciwników `tools/media/painted_turn_geometry.py <katalog-wyjsciowy> --no-audio` tworzy pola konturu i jego lokalnych przemieszczeń. Pliki `imp-ink.png` i `imp-flow.png` umieść w `assets/volcano/imp-turn` jako `shape-ink.png` i `shape-flow.png`.

## Odtworzenie

1. Wybierz nowy katalog roboczy pod `C:\AI2D\sprites`. W `tools/media/fire_imp_paint.py` podaj ten katalog, `--source` z ilustracją kierunkową, `--marks` z punktami i `--no-audio`. Skrypt zapisze wejściowy APNG materiałów oraz graf FILM. Uruchom [graf na lokalnym ComfyUI](paint/film-workflow.json) z modelem `film_net_fp16.safetensors`. Nazwa w węźle LoadImage musi odpowiadać nazwie przesłanego APNG.
2. Podaj skryptowi ten sam katalog i punkty oraz `--film` z katalogiem 33 wyeksportowanych klatek. Ustawienie `--rig src/generated/fire_imp_rig.hpp` eksportuje położenia stawów. Do edycji w Kricie użyj `tools/media/fire_imp_krita.py <katalog-cells> <nowy-katalog-Krity> --no-audio`. Program zapisze KRA i wyeksportuje sześć warstw.
3. Złóż wyeksportowane części za pomocą `fire_imp_paint.py` z opcjami `--cells`, `--atlas`, `--header`, `--body`, `--relief` i `--no-audio`. Przed podmianą działających plików zachowaj ich kopie. Gra ładuje atlas `assets/volcano/imp-turn/parts-0.png` i materiały `fire-imp.png` z katalogów `volcano` i `volcano/corpses`.
4. Edytuj tory w SIF. Funkcje `read_tracks` i `export` w `tools/media/fire_imp_motion_synfig.py` zapisują ruch do `src/generated/fire_imp_motion_data.hpp`. Upadek eksportuje funkcja `export` w `tools/media/fire_imp_fall_synfig.py`. Oba narzędzia mogą też odtworzyć pliki SIF w nowym katalogu i wyrenderować wszystkie klatki bez dźwięku.

Przy pakowaniu chochlika moduł `fire_imp_turn_maps.py` rejestruje każdą z sześciu części ciała względem dwóch sąsiednich ilustracji z Krity. Klatki pośrednie wyznaczają ruch, a kolor i kontur każdego punktu pochodzą z jednego malowanego kierunku. Mapa `turn-flow.png` przechowuje oba przesunięcia UV w czterech kanałach. Siatka 48 × 48 dla każdego z pięciu węzłów ośmiu sektorów mieści się w teksturze 1920 × 288. Wariant bez shadera ma 32 widoki wypalone z tych samych danych. Pakowanie wymaga OpenCV (`opencv-python-headless==5.1.0.95`).

Podczas rzutu bark, łokieć i dłoń prowadzą ruch każdego ramienia. Palce obejmują żar, który powiększa się po opuszczeniu dłoni. Żebra oraz pas pod pachami wykorzystują pociągnięcia pędzla z przedniego i tylnego ujęcia tej samej postaci. Test `caldera_pair_art_qa.cpp` sprawdza widoczność palców przy naładowanej kuli oraz drogę obu dłoni od początku gestu do końca powrotu w ośmiu kierunkach.

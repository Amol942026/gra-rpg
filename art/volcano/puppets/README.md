# Ognisty Chochlik i Popielny Upiór

Grafiki obu przeciwników powstały w Sunburst jako osobno malowane części. Chochlik ma bazaltowe łuski, płyty grzbietowe, rogi, ogon i racice. Upiór składa się z pustego kaptura, haftowanej pelerynki, trzech pół i rękawów. Krawędzie, haft i błyski oczu dopracowano w projektach Krity. [Ilustracje źródłowe, prompty, warstwowe KRA i sumy plików](provenance.json)

Chochlik ma jeden kąt obrotu dla całego ciała. Atlas `assets/volcano/fire-imp-core.png` zawiera 360 ujęć tułowia z szyją, czaszką, pyskiem i rogami. Ramiona, nogi i ogon korzystają z mocowań wyliczanych z tego samego kąta. Dwie części przy każdym stawie używają identycznego punktu mocowania. Shader `src/fire_imp_composite.hpp` składa pełną pozę 2D i jej obrys. Położenie dłoni określa początek toru kuli ognia. Upadek ma wspólną oś całego ciała, a obrys ciała wyznacza jego kontakt z ziemią.

Atlasy wycinanek `assets/volcano/fire-imp-puppet.png` i `assets/volcano/ash-wraith-puppet.png` mają po 480 × 640 px. Generator `tools/media/fire_imp_head_turn.py` przygotowuje ujęcia głowy. `tools/media/fire_imp_core_turn.py` łączy je z malowanym tułowiem w atlas 2016 × 1740 px. Oba generatory używają Pythona z Pillow, NumPy i SciPy.

Klucze spoczynku, chodu, obrotu, rzutu i upadku chochlika są w `C:/AI2D/animations/chochlik-synfig-20261120/klucze-chochlika.sif`. Silnik korzysta z ich eksportu w `src/generated/fire_imp_synfig.hpp`. Kąt postaci pochodzi ze sterowania w grze. Rytm kluczy chodu podąża za przebytą drogą, a krok w miejscu za zmianą kierunku. Faza wyrzutu pozostaje związana z utworzeniem pocisku. Katalog `gotowe` zawiera pięć projektów Synfiga z klatkami kluczowymi przesunięcia, obrotu i skrótu perspektywicznego namalowanych warstw. `tools/media/fire_imp_synfig_poses.cpp` eksportuje ustawienia tych warstw z tej samej pozy 2D, której używa gra.

Po edycji kluczy zapisz nowy nagłówek i przebuduj obie lokalne kompilacje.

```powershell
& $mediaPython tools/media/fire_imp_synfig_rig.py 'C:/AI2D/animations/chochlik-synfig-20261120' --from-master 'C:/AI2D/animations/chochlik-synfig-20261120/klucze-chochlika.sif' --header src/generated/fire_imp_synfig.hpp --no-audio
```

Atlas tułowia można odtworzyć następującym poleceniem.

```powershell
& $mediaPython tools/media/fire_imp_core_turn.py assets/volcano/fire-imp-puppet.png assets/volcano/fire-imp-head.png 'C:/AI2D/sprites/fire-imp-core.png' --no-audio
```

Mastery lotu, rzutu i osiadania tkaniny Upiora są w `C:/AI2D/animations/kaldera-nowe-rigi-20261119`. Po zmianie jego kluczy użyj poniższego eksportu. Argument `--atlases assets/volcano` tworzy projekty z atlasów w nowym katalogu wyjściowym.

```powershell
& $mediaPython tools/media/caldera_puppet_motion.py --output 'C:/AI2D/animations/kaldera-nowe-rigi-20261119' --header src/generated/caldera_puppet_motion.hpp --no-audio
```

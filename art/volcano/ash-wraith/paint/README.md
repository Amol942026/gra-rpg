# Popielny Upiór

Pięć autorskich ilustracji przedstawia pustą szatę z wypalonym haftem, miedzianą broszą i otwartymi rękawami. Zostały wygenerowane w GPT-Image-2.6 Sunburst dla Emberfall. Podstawą ośmiu kierunków są profil, oba przednie ujęcia, tylny skos i plecy. Symetryczne ujęcia korzystają z odbicia i zamiany anatomicznej stron. [Źródło](../redesign/references/sunburst-turnaround.png)

Projekt Krity zawiera sześć edytowalnych warstw. Tren, dwie poły, peleryna z kapturem i oba rękawy mają oddzielnie podmalowane miejsca, które odsłania ruch. Miedziane bliki na broszy zostały naniesione w Kricie i wyeksportowane z powrotem do atlasu. [Projekt Krity](plaszcz.kra)

Do przygotowania 32 kierunków geometrii generator wykorzystuje lokalny model FILM w ComfyUI. Kotwice tkaniny śledzą kaptur, ramiona i mankiety. Ostatnie włókna przy dole przepuszczają trochę światła. [Graf ComfyUI](comfy-film.json)

Kolor powierzchni pochodzi z sześciu stałych pasów tkaniny w pliku `weave.png`. Generator zszywa przód, profile i tył ubrania wzdłuż pasujących fałd. Pobiera detale z pełnowymiarowej ilustracji i zachowuje podszewkę domalowaną w Kricie. Każdy fragment szaty ma własne współrzędne na tkaninie. Renderer śledzi je przez cały obrót, dzięki czemu brosza, mankiety i haft zachowują pojedynczy kontur. Przy silnym skrócie perspektywicznym filtrowanie anizotropowe uwzględnia szerokość widocznego pasa. [Generator powierzchni](../../../../tools/media/ash_wraith_surface.py)

Atlas materiału zwłok ma to samo rozmieszczenie części co atlas koloru. Kanały RGB opisują przestygnięcie tkaniny, a alfa jej wypukłość na fałdach. Animacja może więc składać żywą szatę aż do końcowej pozy na podłożu bez podmiany rysunku.

Do ponownego przygotowania mediów, z katalogu głównego projektu i interpreterem z NumPy, SciPy, Pillow, OpenCV i requests, uruchom oba polecenia w kolejności. Nowy katalog roboczy powinien znajdować się w katalogu sprite’ów dopuszczonym w moście Krity.

```powershell
& $mediaPython tools/media/ash_wraith_paint.py $katalogRoboczy --source art/volcano/ash-wraith/redesign/references/sunburst-turnaround.png --krita --no-audio
& $mediaPython tools/media/ash_wraith_turn.py $katalogRoboczy --comfy http://127.0.0.1:8188 --pack --no-audio
```

Drugi skrypt zapisuje obraz i relief 32 × 6 części, osiem podstawowych kierunków dla trybu zapasowego oraz współrzędne prostokątów dla renderera. Po przepakowaniu atlasu przebuduj grę z nowym nagłówkiem.

Skrypt zapisuje pole przemieszczeń obrysu w pliku `registration.png`. W `edges.png` kanał czerwony przechowuje odległość od brzegu, a zielony i niebieski pozycję na pasie tkaniny. Z tego samego obrysu korzystają cień i wykrywanie trafienia kursorem.

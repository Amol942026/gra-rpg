# Ostrość Bramy Świtu

Oryginalny obraz 640 × 400 px przeszedł lokalne powiększenie ×4 modelem
Real-ESRGAN w ComfyUI. W Kricie nowa warstwa zachowuje 82% udziału, dzięki
czemu delikatna faktura pierwotnego malowania pozostaje widoczna. W
edytowalnym pliku `brama-switu-hd.kra` są też oryginalny obraz, ryty, cień
pod napisami i warstwy efektów.

Silnik ładuje tło 1920 × 1200 px. Trzy ruchome tekstury pozostają w rozmiarze
1440 × 900 px. Łączne użycie pamięci tekstur menu wzrosło z 19,8 do 23,6 MiB.
Powiększenie zostało wypalone podczas produkcji.

[Porównanie wycinka w skali 1:1](porownanie.png) i [pełny podgląd](podglad.png)
pochodzą z rozpakowanej wersji gracza. [Raport QA](QA.json) zapisuje pomiar
krawędzi kamiennej bramy, kontrolę 60 ujęć pętli oraz wyniki uruchomienia obu
edycji w ukryciu i bez dźwięku. Ogólny test nadal zgłasza dziewięć wcześniejszych
błędów kontroli alfy menu pauzy.

Model Real-ESRGAN udostępnił Xintao Wang na licencji BSD-3-Clause.
[Licencja](Real-ESRGAN-LICENSE.txt) i [pochodzenie](provenance.json)
zawierają źródło i sumy SHA-256. Workflow `upscale.json` korzysta z
oryginalnego renderu zachowanego razem z kopią przed wdrożeniem w
`C:/AI2D/sprites/emberfall-menu-clarity-20261106`.

# Odświeżone otoczenie Lasu Mchu i Morza Piasków

W lokalnym ComfyUI powstało pięć materiałów kory, liści, omszałego kamienia,
piaskowca i kaktusów. Dopasowano je do 14 istniejących sylwetek i wykończono
w dwóch dokumentach Krity. Akcenty malowane w Kricie obejmują żyłki liści,
światło na krawędziach kamienia, żłobienia kory i inskrypcje ruin.

- Las Mchu otrzymał nowe detale świerku, brzozy, dębu, krzewu, głazu, kolumny
  i skupiska grzybów.
- Morze Piasków otrzymało nowe detale kaktusa świecznikowego, opuncji,
  piaskowca, obelisku, suchego krzewu, palmy oazy i Bramy Eldrina.

Cztery leśne PNG używają dotychczasowej ścieżki wczytywania. Dziesięć
pozostałych grafik znajduje się w atlasie 1440 × 1380 px. Gra kopiuje jego
komórki do dotychczasowych buforów, po czym zwalnia teksturę atlasu.
Maski alfa, położenia, kotwice, wiatr i kolizje korzystają z istniejących danych.

[Porównanie w grze](porownanie.png) pokazuje trzy z sześciu sprawdzonych
scen. [Raport QA](QA.json) zapisuje zgodność masek i tekstur po rozpakowaniu
wersji gracza oraz uruchomienie obu edycji w ukryciu i bez dźwięku.
Zasoby Kaldery porównano piksel po pikselu.

Pliki `las-mchu.kra` i `morze-piaskow.kra` zachowują oryginały na ukrytych
warstwach. `workflows/` zawiera pięć grafów ComfyUI, a `provenance.json`
źródła modeli i sumy SHA-256. Surowe rendery, skrypty produkcji i kopie
przed wdrożeniem są w `C:/AI2D/sprites/emberfall-scenery-comfy-20261106`.

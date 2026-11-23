# Szakal wydmowy i Szklany żniwiarz

Szakal ma płowy grzbiet, ciemną pręgę, płócienną obrożę i przysiad przed skokiem do ugryzienia. Żniwiarz nosi pancerz z matowego szkła i brązu. Przed cięciem cofa sierp i przenosi ciężar do przodu. Oba gatunki mają osobny chód, reakcję na trafienie, atak i upadek. Faza chodu zależy od przebytego dystansu, a obrót zachowuje jej ciągłość.

Bazowe widoki z GPT Image zapisano w plikach `*-source.png`. Złożenie sześciu części, punkty obrotu i 32 widoki na gatunek odtwarza `tools/media/desert_enemy_paint.py`. Lokalny FILMNet z ComfyUI dostarczył korektę barw klatek pośrednich. Kształt i ostre krawędzie pochodzą z wyciętych warstw. Pliki `*-krita.png` są eksportami warstwy ze śladami pędzla Krity. Projekty warstwowe zapisano jako `C:/AI2D/sprites/emberfall-jackal-20261123.kra` oraz `emberfall-reaper-20261123.kra`.

`ruch-pustynnych-bestii.sif` zachowuje krzywe kroku, zaparcia łap, natarcia, zamachu, zgięcia łokcia i kolejnych faz upadku. `tools/media/desert_enemy_motion.py` eksportuje je jako tablice dla renderera, a `desert_enemy_art.hpp` prowadzi według nich malowane warstwy. W chodzie barki i biodra przenoszą ciężar w rytmie kroku, głowa reaguje z niewielkim opóźnieniem, a szakal przesuwa cztery łapy w różnych fazach. Po przechyle ciało zachowuje grubość tułowia. Podparcie wylicza obrys pomalowanych części.

Każdy gatunek ma jeden atlas RGBA o szerokości 1024 px. Główny atlas Żniwiarza zawiera puste dłonie. Osobny sierp 96 × 96 px, narysowany w Kricie, korzysta z tej samej kotwicy dłoni w animacji, wyborze kursorem i cieniu. Edytowalne źródło broni to `C:/AI2D/sprites/sierp-szklanego-zniwiarza-rgba-20261123.kra`. `tools/media/reaper_sickle_krita.py` zapisuje jego eksport.

W każdym z sześciu sektorów pustyni znajduje się po jednym osobniku każdego gatunku. Miejsca startowe uwzględniają położenia i siedliska dotychczasowych przeciwników, przeszkody oraz strefy wejścia, oazy, bossa, skrzyń i kapliczek. Gra uzupełnia wcześniej odwiedzoną pustynię przy wczytaniu zapisu starszego niż wersja 33. Oba gatunki w trybie DEV są też dostępne na liście przyzywania.

# Rogacz Żaru — obrót i chód

Sierść ma matowe pasma z pojedynczymi włosami okrywowymi. Część kępek wychodzi poza obrys karku i grzbietu, a ich nasady poruszają się zgodnie z położeniem skóry. Grzbiet, szyja i głowa mają wspólny obrys. Pysk obraca się wokół nasady czaszki, a szyja ugina się w stronę tego samego punktu. [Projekt Krity](C:/AI2D/sprites/kaldera-rogacz-wlosy-20261122/rogacz-wlosy.kra) · [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-rogacz-wlosy-20261122/rogacz-zaru.webp)

Przednie i tylne nogi zaczynają się wewnątrz barków i bioder. Krótkie pasma sierści na udach i podudziach podążają za ugięciem stawów i kończą się nad racicami. Kolejne kopyta unoszą się, przechylają podczas wymachu i opierają o ziemię zgodnie z przebytą drogą. Klucze Synfiga prowadzą też ruch karku i przechył tułowia w stronę nogi podporowej. [Projekt Synfiga](motion/rogacz-chod.sif) · [Podgląd chodu w Kalderze](C:/AI2D/pipeline/runs/kaldera-rogacz-naturalny-chod-20261122/rogacz-chod.webp)

Widoczność rogów, kończyn i tułowia ustala bufor głębi dla poszczególnych pikseli. Nasady rogów są osadzone w czaszce i na początku zakręcają na zewnątrz. Krzywe obrysu prowadzą od głębokiej klatki przez węższe lędźwie do zadu. Brzuch unosi się przed tylnymi nogami. [Podgląd skrętu z Kaldery](C:/AI2D/pipeline/runs/kaldera-rogacz-proporcje-20261122/rogacz-zaru.webp)

Po trafieniu Rogacz podwija nogi i opiera bok o podłoże. Leżące ciało korzysta z końcowej pozy tego samego modelu. Cień, obrys i miniatura podążają za tą pozą. [Projekt upadku w Synfigu](motion/rogacz-upadek.sif) · [Podgląd z Kaldery](C:/AI2D/pipeline/runs/kaldera-rogacz-martwe-cialo-20261122/rogacz-upadek.webp)

## Poprzednie źródła grafiki

Cztery nowe ujęcia zachowują długie brązowe rogi, bazaltowe płyty, ciemną sierść i racice Rogacza. Ujęcie od tyłu w trzech czwartych pochodzi z dotychczasowej grafiki. [Ilustracje i polecenie generowania](paint/prompt.json) · [Warstwowy projekt Krity](paint/rogacz.kra)

Tułów, rogi i cztery łapy korzystają z jednego zestawu proporcji i wspólnych stawów. Renderer rzuca tę samą sylwetkę 2D dla dowolnego kierunku. Stała kalibracja rzutu wyrównuje wysokość między stroną i przodem. Kopyta wykonują cykl zsynchronizowany z przebytą drogą. W miejscu rogacz przestawia łapy pod ciężarem ciała i pochyla łeb podczas przygotowania do szarży. Cień, wybór kursorem i początek upadku korzystają z tej samej pozy.

Atlas malowania ma 1024 × 512 px i zajmuje 2 097 152 bajty RGBA8 na poziomie bazowym. Zawiera bazalt, sierść, rogi i racice. [Projekt Krity](C:/AI2D/sprites/kaldera-profile-20261122/rogacz-materialy-final.kra) · [Podgląd z gry](C:/AI2D/pipeline/runs/kaldera-profile-20261122/drak-i-rogacz.webp)

### Odtworzenie atlasu kierunkowego

1. Eksportuj warstwy projektu Krity do `paint/rogacz.png`. Kolejne kolumny zawierają prawy profil, widok w trzech czwartych z przodu, przód, tył i tył w trzech czwartych. Każda warstwa ma własny wiersz.
2. Uruchom `tools/media/cinderhorn_paint.py` z `paint/rogacz.png`, katalogiem roboczym i opcjami `--prepare-film <katalog>/flow.png --no-audio`. Wczytaj APNG do [lokalnego grafu FILM](paint/film-workflow.json) i wyeksportuj 33 klatki. Bieżąca produkcja użyła tego samego modelu i architektury FILMNet na CPU.
3. Ponów polecenie z `--film <katalog klatek> --output <katalog atlasu> --header <plik nagłówka> --no-audio`. Dopasowanie obrysu i odtworzenie ostrej faktury są wykonywane w tym kroku.
4. Wygeneruj `turn-ink.png` i `turn-flow.png` przez `tools/media/caldera_crisp_turn.py assets/volcano/cinderhorn-turn/parts-0.png src/generated/cinderhorn_turn_atlas.hpp assets/volcano/cinderhorn-turn --sharpen .74 --no-audio`.
5. Klucze chodu, szarży, pochylenia rogów i ruchu ogona edytuj w [projekcie Synfiga](motion/rogacz-chod.sif). Eksportuj przez `tools/media/cinderhorn_turn_synfig.py art/volcano/cinderhorn/motion/rogacz-chod.sif src/generated/cinderhorn_synfig.hpp --no-audio`.

[Krótki obrót w chodzie](C:/AI2D/pipeline/runs/rogacz-plynny-skret-20261120/rogacz-obrot.webp) · [Wyniki kontroli](C:/AI2D/pipeline/runs/rogacz-plynny-skret-20261120/report.json)

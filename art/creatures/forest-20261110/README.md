# Leśny bestiariusz — 10.11.2026

| Przeciwnik | Grafika | Ruch |
| --- | --- | --- |
| Szlam | Pęcherzyki w zielonym żelu, jasne refleksy i nieregularna obwódka przy ziemi | Sprężyste ugięcie przed wypadem, wydłużenie sylwetki w ataku i powrót do spoczynku |
| Wilk | Ciemna okrywa, jasny pysk i kryza, warstwowe uszy i wyraźniejsze łapy | Naprzemienny ruch czterech łap, opóźniony ogon, wysunięcie głowy i rozwarcie szczęki |
| Szkielet | Spękana kość, ciemne oczodoły z żarem, głębsze żebra i zardzewiałe okucia | Wyprostowane nogi w spoczynku i płynne przenoszenie ciężaru. Dłoń prowadzi ostrze o stałej długości |
| Drzewiec | Żłobiona kora z mchem, rozwidlone konary, korona z liści i korzenie | Ciężki krok, przechył tułowia i korona, która podąża za ruchem z opóźnieniem |

Łapy wilka i stopy szkieletu korzystają z fazy liczonej po rozstrzygnięciu
kolizji oraz rozwiązania stawów o stałych długościach. Drzewiec zachowuje
dotychczasowy kontakt korzeni z gruntem. Krzywe Synfig sterują dodatkowymi
ruchami nad istniejącą mechaniką chodu i ataków.

## Źródła

- Cztery projekty z lokalnego FLUX.1 Schnell i odpowiadające im grafy API,
  historie i hashe znajdują się w
  `C:/AI2D/pipeline/runs/forest-creatures-20261110/concepts`.
- Master KRA, 13 warstw, oryginalny eksport i logi Krita MCP są w
  `C:/AI2D/sprites/emberfall-forest-creatures-20261110-v2`.
- Atlas ma 512 × 640 pikseli. Dodatkowy wiersz pozwala nadać drzewcowi
  osobny materiał listowia przy zachowaniu materiału Kamiennego Strażnika.
  Nowe tekstury są prowadzone po obłych powierzchniach z pojedynczym
  wygaszonym łączeniem z tyłu.
- Cztery projekty SIF w
  `C:/AI2D/animations/emberfall-forest-creatures-20261110` zawierają
  spoczynek, ruch i intensywność zamachu. Synfig wyrenderował 444 klatki
  podglądów kontrolnych. Ostateczny wygląd widać w przechwyceniach silnika.
- Skrypty produkcyjne to `tools/media/forest_paints.py` i
  `tools/media/forest_motion.py`. Wyniki natywnego renderera i kopie
  poprzednich zasobów są w `C:/AI2D/pipeline/runs/forest-creatures-20261110`.

[FLUX.1 Schnell, Apache-2.0](https://huggingface.co/black-forest-labs/FLUX.1-schnell) ·
[Konwersja GGUF](https://huggingface.co/city96/FLUX.1-schnell-gguf)

## Odbiór

Renderer gracza przeszedł 1189 kontroli. Obejmują 96 póz w ośmiu kierunkach,
288 klatek chodu i ataków, wskazywanie kursorem, cienie w leśnej scenie
oraz eksport 32 ujęć ciał. Testy logiki i animacji przeszły 345 804 kontrole.
Porównanie pikseli wykazało zmiany w sześciu komórkach materiałów
i czterech rzędach atlasu ciał. Dane ruchu z nagłówka odpowiadają
wyeksportowanym waypointom SIF.

Oba ZIP-y po wypakowaniu przeszły porównanie wszystkich 287 zasobów
i próbę 180 klatek. Okna testowe były ukryte, a urządzenie audio zamknięte.
Podgląd i raport zbiorczy są w `C:/AI2D/pipeline/runs/forest-creatures-20261110`.

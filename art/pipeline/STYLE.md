# Emberfall — zasady produkcji 2D

## Kierunek artystyczny

Izometryczne, czytelne fantasy: ciepła ochra i stare złoto, chłodne turkusowe
światło magii, ciemne granatowe cienie. Zachowuj sylwetkę czytelną przy zwykłym
oddaleniu kamery. Detal oceniaj też w rzeczywistej skali, na tle lasu i pustyni.
Nowy asset powinien pasować do istniejącego obrazu gry; kryształ testowy jest
próbką techniczną, nie obowiązkowym wzorcem wszystkich przyszłych postaci.

- Pracuj wyłącznie w 2D. Warstwowe wycinanki, wektory i efekty światła mogą
  dawać wrażenie 2.5D. Nie wprowadzaj modelowania ani procesu produkcji 3D.
- Master: KRA, sRGB, RGBA U8, przezroczyste tło; nazwy warstw opisują części.
  Rozdziel ciało, ubranie, broń, detale, cień i efekty, gdy ułatwia to animację.
- Zachowuj te same proporcje, wyposażenie i rozpoznawalne detale między
  klatkami. Animuj zapisane części/rig zamiast generowania odrębnej postaci
  dla każdej klatki. Testowa animacja wykorzystuje jedną ilustrację z Krity.
- Utrzymuj stały rozmiar komórki i kotwicę stóp. Kierunki ekranu w kodzie gry:
  E, SE, S, SW, W, NW, N, NE. Kotwice i wielkości sprawdzaj względem aktualnej
  konkretnej animacji, nie kopiuj bezkrytycznie wartości z przykładu kryształu.
- Dla chodu/biegu oceniaj kontakt stóp, rytm, tułów, ramiona i płaszcz.
  Miecz podczas biegu jest schowany; przy ataku dobyty. Dla ataków sprawdzaj
  pełny zasięg broni i brak obcięcia na krawędzi klatki.
- Pętle mają wirtualny punkt końcowy równy początkowi, poza eksportowaną
  sekwencją. Sprawdzaj także przejście ostatnia -> pierwsza klatka.

## Oświetlenie i budżety

Mapy Laigtera są estymacją z obrazu. Oceniaj kierunek światła, krawędzie i
materiały; nie traktuj automatycznie wyznaczonej wysokości jako wiernej geometrii.
Bieżący renderer Emberfall nie ma shadera map materiałowych sprite'ów.
Normal/specular/occlusion wykorzystuj do bake PNG; parallax zachowuj w źródłach.

- GPU: 60 fps oznacza budżet całej klatki ok. 16,67 ms, nie samych animacji.
  Test eksportu 60 klatek/s nie dowodzi wydajności całej gry.
- Wczytuj atlas raz przy wejściu do sceny, rysuj gotowe prostokąty UV, zwalniaj
  przy wyjściu. Bez otwierania plików, pakowania atlasów i generowania map w pętli gry.
- Limit pojedynczego atlasu w narzędziu: 2048 px na oś. Dziel duże zestawy na
  klipy/kierunki. Domyślne komórki testowe 128 px nie są wymogiem całej gry.
- Metadane podają faktyczne bajty RGBA8. Tekstury NPOT ograniczają puste miejsce;
  używaj marginesu między komórkami, kontroluj alfa i bilinear filtering.
- Synfig renderuj domyślnie dwoma wątkami; sekwencyjnie wykonuj ciężkie etapy
  na Ryzen 5 7530U / 16 GB / zintegrowanym Radeon. Nie trzymaj wielu dużych
  dokumentów i rendererów aktywnych równocześnie.

## Dźwięk

Muzyka: własna kompozycja i instrumenty, logiczna fraza oraz ogony renderowane
przed wycięciem pętli. Przykład renderuje trzy cykle i pobiera środkowy.
WAV master 48 kHz stereo PCM16, stream OGG Vorbis quality 5.
Punkt wyjścia ambientu: -18 LUFS, maks. -2 dBTP; docelowy poziom konkretnego
utworu dobieraj do miksu gry. Nie podnoś całej muzyki do poziomu krótkiego SFX.

SFX: rozpoznawalny atak, korpus i wygaszony ogon, kilka kontrolowanych wariantów
gdy mechanika tego wymaga. Punkt wyjścia: szczyt -2 dBFS z zapasem dla nakładania
się zdarzeń. Nie dodawaj ciszy na początku uderzenia. Minimalizuj trzaski, DC
i niepotrzebny bas; zachowuj dynamikę zamiast maksymalizować głośność.

Sprawdzaj dekodowanie, pełną liczbę próbek, kanały, RMS, peak/true peak,
clipping, długość i szew pętli. Ciche testy potwierdzają parametry techniczne;
nie zastępują oceny artystycznej przez odsłuch. Nie odtwarzaj audio bez nowej
wyraźnej prośby użytkownika.

## Prawa do zasobów

Obecny test używa natywnego rysowania i syntezy, bez zewnętrznych sampli,
obrazów, modeli AI ani płatnych usług. Zachowuj manifest pochodzenia.
Nowe zasoby zewnętrzne wymagają konkretnej licencji dopuszczającej zastosowanie
komercyjne i zachowania autorstwa, jeżeli jest wymagane. Licencja programu
nie zastępuje licencji użytego obrazu, sampla, fontu ani modelu.

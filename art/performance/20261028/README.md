# Optymalizacja Emberfall — 28.10.2026

Na tym komputerze porównano ten sam scenariusz 900 klatek, dziewięć obszarów
pustyni, walkę, zaznaczanie przeciwników i piaskowe więzienie bossa.
Ryzen 5 7530U, zintegrowany Radeon, ekran 1920 × 1080 / 60 Hz, końcowy bufor
1728 × 1080. Wszystkie uruchomienia miały `--no-audio`; ustawień użytkownika
ani procesów innych aplikacji nie zmieniano.

| Pomiar rzeczywistej gry | Przed zmianą | Po zmianie, DXGI |
| --- | ---: | ---: |
| Średnie FPS | 46,17 | 59,91 |
| Czas klatki p95 | 28,36 ms | 17,72 ms |
| Czas klatki p99 | 29,07 ms | 18,57 ms |
| Najdłuższa klatka | 33,03 ms | 23,17 ms |
| Klatki powyżej 25 ms | 219 | 0 |

Źródła liczb: [przed zmianą](before-desert.txt),
[po zmianie](after-desert-dxgi.txt).
To wynik lokalnego testu, a nie gwarancja identycznego czasu każdej klatki
przy dowolnym obciążeniu Windows. Nie utożsamiano czasu samego rysowania
ani ukrytego okna z rzeczywistą liczbą wyświetlanych klatek.

## Zmiany

- Bohater, przeciwnicy i obrys zaznaczenia korzystają ze wspólnych buforów
  GPU. Bieżące modele są współdzielone przez cienie, rysowanie i wybieranie
  kursorem. Magazyn pamięci modeli jest ponownie używany, z limitem 64.
- Portret HUD jest odtwarzany po zmianie wyposażenia, widocznych strzał
  lub skali. Kopia RGBA zachowuje pierwotną przezroczystość krawędzi.
- Statyczne mapy terenu i warstwy tła nie mają zbędnych buforów głębi.
  Głębia postaci i aktywnej sceny pozostaje włączona.
- Warstwy gór są pomijane, gdy w całości zasłania je nieprzezroczysty teren,
  i zachowują bufor przy nieruchomej kamerze. Woda i pył nadal się animują.
- Kolizje najpierw odrzucają odległe obiekty prostym testem obwiedni.
  W pobliżu przeszkody wykonują wcześniejszy, dokładny test okręgu.
  AI, promienie kolizji, tempo ruchu i zapis v7 pozostają zgodne.
- Domyślna prezentacja to istniejący DXGI flip z V-sync. Sprawdza rzeczywisty
  piksel współdzielony przez GPU; w razie braku obsługi przechodzi na OpenGL.
  Nie wprowadzono osobnego limitera rywalizującego z odświeżaniem ekranu.

## Rozdzielczość i ostrość

Na ekranie większym niż bazowe 1440 × 900 świat lasu i pustyni jest rysowany
do bufora 945 × 810 i powiększany biliniowo. Przy 1080p oznacza to około
44% mniej pikseli świata. Sceneria jest nieco bardziej miękka; geometria,
animacje i liczba obiektów nie są upraszczane. Wszystkie napisy, etykiety,
portret, ekwipunek i HUD pozostają w natywnej rozdzielczości końcowej.
Wieża, prolog i menu zachowują poprzedni przebieg renderowania.

`Uruchom.bat --native-world` przywraca pełną rozdzielczość świata kosztem
wydajności. `--opengl` wybiera zapasowy sposób wyświetlania; w porównaniu
tego samego scenariusza uzyskano z nim 57,71 FPS po optymalizacji.

## Powtarzanie testów

1. `pwsh -NoProfile -File build.ps1 -Test` — logika i zapisy.
2. `pwsh -NoProfile -File tests/render_optimization_qa.ps1 -NoAudio`
   — porównanie pikseli, wyposażenia, tła, portretu i geometrii kursora.
3. `pwsh -NoProfile -File tests/performance60_qa.ps1 -NoAudio -Present -Quick -Dxgi`
   — siedem widoków pustyni, końcowe klatki z V-sync, niezależne zapytania
   GPU bez `glFinish` w pętli pomiaru. Okno powinno pozostać na pierwszym
   planie; raport zawiera liczbę klatek z fokusem.
4. `dist/Emberfall.exe --no-audio --desert-benchmark` — rzeczywista gra,
   dziewięć obszarów i walka; wynik `dist/desert-benchmark-v43.txt`.
5. `pwsh -NoProfile -File tests/performance60_package_qa.ps1 -NoAudio -RendererHelperRoot <folder-testu-renderera> -PerformanceHelperRoot <folder-testu-FPS> -Present -Dxgi`
   — uruchomienie wypakowanego ZIP-a, zgodność zasobów i ponowne testy.

Wczesne pomiary diagnostyczne i warianty synchronizacji w `dist/qa-performance60-*`
są lokalnymi plikami roboczymi. Wyników z barierami `glFinish`, ukrytego lub
zminimalizowanego okna i prób bez manifestu aplikacji nie wykorzystano
do deklaracji końcowych FPS. Nie zmieniono sterownika ani ustawień Windows.

## Potwierdzone kontrole

- 158 661 kontroli logiki i animacji — PASS, w tym 48 000 porównań kolizji
  z poprzednim algorytmem, również na granicach przeszkód i dla wszystkich
  promieni przeciwników.
- 3403 kontrole zapisu i odczytu — PASS.

Kopie sprzed zmian: `art/pipeline/backups/performance-20261028-122142`.
Źródła natywne i istniejące grafiki nie były zastępowane. Nie publikowano
projektu ani nie wykonywano commita/pusha.

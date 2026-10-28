# Optymalizacja Emberfall — 28.10.2026

**Nowszy wariant i aktualny ZIP:** [dalsza optymalizacja](round2/README.md).
Poniższe pomiary oraz hashe opisują wcześniejszy etap prac.

Cel: 60 FPS na Ryzen 5 7530U, 16 GB RAM i zintegrowanym Radeonie.
Pomiary ekranu wykonano przy 1920 × 1080 / 60 Hz, z końcowym buforem
1728 × 1080. Wszystkie testy były bez dźwięku. Po poleceniu użytkownika
ostatnie testy pracują wyłącznie w ukrytych oknach.

## Wyniki i zakres ich potwierdzenia

Przed prośbą o ukrywanie okien porównano rzeczywistą grę w scenariuszu
900 klatek: dziewięć obszarów pustyni, walka, zaznaczanie przeciwników
i piaskowe więzienie bossa.

| Pomiar na ekranie | Przed zmianą | Dwa przebiegi po optymalizacji |
| --- | ---: | ---: |
| Średnie FPS | 46,17 | 58,45–59,91 |
| Czas klatki p95 | 28,36 ms | 17,72–19,10 ms |
| Klatki powyżej 25 ms | 219 | 0–7 |

Źródła: [przed zmianą](before-desert.txt), [pierwszy przebieg](after-desert-dxgi.txt),
[powtórzenie](after-desert-repeat.txt). Nie pominięto słabszego powtórzenia.
Las przed końcowym buforowaniem wiatru nadal miał około 48–50 FPS:
[DXGI](forest-before-wind-dxgi.txt), [OpenGL](forest-before-wind-opengl.txt).

Końcowy wariant sprawdzono w ukrytym oknie. Pełne zakończone renderowanie
klatki, łącznie z oczekiwaniem na GPU, mieściło się w następujących granicach:

| Scena | p95 przed buforowaniem wiatru | p95 po |
| --- | ---: | ---: |
| Północ lasu | 9,67 ms | 8,34 ms |
| Gęsty las | 11,14 ms | 9,71 ms |
| Okolice leśnego bossa | 10,86 ms | 9,82 ms |
| Krawędź lasu | 16,33 ms | 12,54 ms |
| Oaza | 8,04 ms | 7,76 ms |
| Boss pustyni | 7,05 ms | 7,03 ms |

[Dane pomiaru](render-cost.json): 80 mierzonych klatek na wariant i scenę,
po 20 klatkach rozgrzewki, identyczny stan początkowy animacji. Jedna bariera
GPU kończy pomiar całej klatki; nie ma barier między etapami rysowania.
Wartości mieszczą się w budżecie 16,67 ms na renderowanie, ale **nie są
pomiarem wyświetlanych FPS ani potwierdzeniem stałych 60 FPS końcowej wersji**.
Windows inaczej obsługuje prezentację ukrytych i zminimalizowanych okien.
Po zmianie preferencji użytkownika nie przywracano okien testowych.

## Wdrożone zmiany

- Wspólne, preładowane bufory GPU dla bohatera, przeciwników i obrysów.
  Bieżąca geometria jest używana przez cienie, obraz i wybieranie kursorem.
  Pamięć modeli jest ponownie wykorzystywana, z limitem 64.
- Portret HUD jest odtwarzany po zmianie sprzętu, widocznych strzał lub skali.
  Kopia RGBA zachowuje krawędzie bez podwójnego mieszania przezroczystości.
- Statyczny teren i warstwy tła nie mają zbędnych buforów głębi. Postacie
  i aktywna scena zachowują właściwe testowanie głębi.
- Góry pustyni są pomijane, gdy zasłania je teren, i buforowane przy
  nieruchomej kamerze. Woda i pył pozostają animowane.
- Odległe przeszkody są odrzucane prostym testem obwiedni przed dokładnym
  testem kołowym. Wyniki kolizji i format zapisu v7 pozostają zgodne.
- Wspólne składowe wiatru są liczone raz na drzewo i przebieg renderowania.
  47 próbek jest współdzielonych przez sąsiednie pasy korony lub jej cienia.
  Zachowano wcześniejszy wzór, deformację, geometrię kursora i piksele obrazu.
- Domyślnie używany jest istniejący DXGI flip z V-sync, z testem rzeczywistych
  pikseli GPU i automatycznym powrotem do OpenGL przy braku obsługi.
- Nowy `--background-test`, wymagający `--no-audio`, ukrywa rzeczywiste okno
  gry. Raport odróżnia szybkość ukrytej pętli od FPS na ekranie.

Próba odrzucania nakładających się fragmentów cienia przez głębię nie
zachowała wszystkich pikseli oazy, więc została wycofana. Końcowy renderer
nie zawiera tej zmiany ani eksperymentów z kolejką prezentacji.

## Ostrość

Na ekranie większym od bazowych 1440 × 900 świat lasu i pustyni korzysta
z bufora 945 × 810, powiększanego biliniowo. Przy 1080p oznacza to około
44% mniej pikseli świata. Obraz świata jest nieco bardziej miękki; geometria,
animacje i liczba obiektów pozostają zachowane. HUD, portret, ekwipunek,
napisy i etykiety zachowują natywną rozdzielczość. Wieża, prolog i menu
zachowują poprzedni sposób rysowania.

`Uruchom.bat --native-world` przywraca pełną rozdzielczość świata kosztem
wydajności. `--opengl` wybiera zapasowy sposób prezentacji.
[Pełna rozdzielczość](desert-native.png), [wariant wydajnościowy](desert-balanced.png).
To statyczne obrazy z testu renderera; ich licznik FPS nie jest pomiarem.

## Testy

- 158 661 kontroli logiki i animacji — PASS, w tym 48 000 porównań kolizji
  z poprzednim algorytmem, na granicach przeszkód i dla wszystkich promieni bestii.
- 3403 kontrole zapisu i odczytu — PASS.
- [810 kontroli renderera](renderer.json) — PASS: 219 par modeli GPU i
  wcześniejszego renderera z zerową różnicą pikseli, 75 odmian sprzętu,
  396 prób kursora, zgodny HUD i sześć dokładnych porównań wiatru/cieni.
  [Szczegóły](render-checks.txt).
- [193 wcześniejsze kontrole prezentacji](presentation-checks.txt) — PASS,
  w tym rzeczywisty DXGI i zmiany rozmiaru/F11. Presenter nie był później
  zmieniany; tego testu nie powtarzano po prośbie o ukrywanie okien.

Końcowa [weryfikacja wypakowanej paczki](package-verification.json) — PASS:
115 zgodnych zasobów, 180 klatek startu rzeczywistej wersji gracza w ukrytym
oknie, powtórzone 810 kontroli i pomiary renderowania. Audio nieotwarte,
ustawienia zachowane, brak DEV i źródeł natywnych w ZIP.
SHA-256 ZIP: `3357a8acc7b51e3ffe03d2ac9e01ce097c28d173e890b0df6daf9d22bc3392ad`.
Player EXE: `94144ebe1cf2a50a47199fece7cea8b9aa4214caead95691781fe775b23d47db`.
[Manifest źródeł](source-manifest.json) i raport zawierają dane do odtworzenia
sprawdzonego wariantu. Pakiet nie był publikowany.

Powtarzanie kontroli bez pokazywania gry:

```powershell
pwsh -NoProfile -File build.ps1 -Test
pwsh -NoProfile -File tests/render_optimization_qa.ps1 -NoAudio
pwsh -NoProfile -File tests/performance60_package_qa.ps1 -NoAudio -RendererHelperRoot <folder-z-aktualnym-helperem>
```

Rzeczywisty EXE można sprawdzać przez
`Emberfall.exe --no-audio --background-test --startup-smoke`.
Normalne uruchomienie użytkownika zachowuje widoczne okno i dźwięk.
Do potwierdzenia FPS na ekranie potrzebny jest osobno dopuszczony test
widocznego okna; nie zastępuje go czas renderowania w tle.

Źródła i pierwotny ZIP zachowano w
`art/pipeline/backups/performance-20261028-122142` oraz
`art/pipeline/backups/performance-resume-20261028`.
Nie zmieniano innych aplikacji ani ustawień Windows, nie publikowano projektu
i nie wykonywano commita ani pusha. Wyniki robocze w `dist/qa-performance60-*`
nie są automatycznie dodawane do Gita.

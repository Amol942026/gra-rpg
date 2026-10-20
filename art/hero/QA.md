# Weryfikacja bohatera — 20.10.2026

Nowa grafika z Krity i dodatkowe animacje z Synfiga są podłączone do gry.
Oceniono pełne sylwetki, postać w lesie i na pustyni, portret, wyposażenie,
pracę stóp, ruch całego ciała oraz miecz, łuk i przeturlanie. Testy odbywały
się bez otwierania urządzenia audio, z jawnym `--no-audio`.

## Wyniki

| Kontrola | Wynik |
| --- | --- |
| Logika gry, interpolacja i zachowanie długości kończyn / kontaktu stóp | 75 587 PASS |
| Nowy malowany bohater, 8 kierunków i 420 klatek siedmiu stanów | 424/424 PASS |
| Pełna grafika, animowane sylwetki i trafianie kursorem | 873/873 PASS |
| 39 przedmiotów, portret, świat i sceny fabularne | 364/364 PASS |
| Celowanie mieczem i łukiem podczas poruszania się | 47/47 PASS |
| Miecz / mocne cięcie, wszystkie kierunki, zgodność ostrza i kursora | 440/440 PASS |
| Łuk | 65/65 PASS |
| Sterowanie | 42/42 PASS |
| Synfig: pełne sekwencje, unikalne PNG, brak brakujących klatek | 354/354 PASS |
| Dokładne wartości krzywych SIF a wygenerowane dane C++ | PASS |

Wyniki i obrazy: `dist/qa-hero-art`. Podgląd `hero-preview.gif` składa się
wyłącznie z przechwyconych klatek rzeczywistego renderera. KRA ma 14 nazwanych
warstw, zachowane części i sumy kontrolne. Test renderowania sprawdza także
zachowanie malowania po opróżnianiu bufora GPU, aby wykluczyć biały prostokąt.

## Wydajność

Dwa kolejne benchmarki po 900 klatek, bez jednoczesnej kompilacji lub produkcji
mediów. To samo okno 1440×900 (natywny bufor 1688×1055), las, OpenGL V-sync
60 Hz, identyczny scenariusz z walką i łukiem, audio wyłączone.

| Parametr | Zachowana wersja przed zmianą | Nowy bohater |
| --- | ---: | ---: |
| Średnie fps | 58,13 | 59,19 |
| Średni czas CPU klatki | 10,19 ms | 8,06 ms |
| p95 CPU | 11,25 ms | 9,15 ms |
| p95 całej klatki | 20,85 ms | 19,29 ms |
| Klatki ponad 25 ms | 0 | 0 |

Czas CPU zmalał o ok. 21%. Atlas zajmuje 0,75 MiB pamięci RGBA8 i jest
wczytywany raz. Krzywe są skompilowane do EXE; w czasie klatki nie ma renderów
Synfiga, odczytu projektów ani generowania obrazów. Wynik jest bliski celowi
60 fps, ale nie stanowi gwarancji stałych 60 fps w każdej sytuacji gry.
Surowe logi: `benchmark-before.txt` i `benchmark-after.txt`. Pierwszy próbny
pomiar `benchmark-before-contended.txt` był równoległy do kompilacji i został
wykluczony z porównania.

## Istniejące ograniczenie

Kontrola cieni: **489/490**. Ten sam jeden test alfa/deformacji wiatrem obiektu
otoczenia nie przechodzi również na zachowanym EXE sprzed zmiany bohatera,
w tej samej pozycji raportu. Porównanie: `shadow-before.txt` i
`shadow-after.txt`. Testy cienia bohatera, uzbrojenia, ataków, rolla i scen
fabularnych przechodzą. Nie zmieniano ani nie osłabiano testu otoczenia.

## Paczka gracza

`tests/hero_package_qa.ps1` wypakowuje gotowy ZIP do nowego katalogu,
porównuje SHA-256 EXE oraz wszystkich zasobów z projektem, odrzuca pliki
produkcyjne KRA/SIF/BLEND/MMP/CSD w paczce, uruchamia prawdziwe EXE gracza na
180 klatek bez audio i powtarza 424 kontrole renderera na wypakowanych
zasobach. Końcowy wynik i SHA-256 paczki: `dist/qa-hero-art/package-verification.json`.
Poprzednie EXE, ZIP i zmienione źródła są zachowane w katalogu
`dist/qa-hero-art/backup-20261020-145500`.

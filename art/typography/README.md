# Czytelniejsze liternictwo — 26.10.2026

Cały interfejs, opisy, dialogi i liczby używają prawdziwego **Lato Bold 700**.
Nagłówki zachowują krój Cinzel, teraz w rzeczywistej wadze **Bold 700**.
Drobne napisy interfejsu mają minimum 15 jednostek zamiast 14.
Nazwy na mapie zachowują własne rozmiary, ale również używają odmian Bold.

Nie dodano nakładanych kopii liter, obrysu ani skalowanego obrazu tekstu.
Fonty są rasteryzowane w docelowej liczbie fizycznych pikseli, kotwice
zaokrąglane do pikseli, a atlas próbkowany 1:1. Pomiar szerokości korzysta
z dokładnie tej samej odmiany i rozmiaru co rysowanie. Wstępne ładowanie
obejmuje także 31-punktowy nagłówek Mirona; testy 57 widoków nie tworzą
atlasów podczas rysowania. Przy skali 1 liczba atlasów pozostaje 27.

`assets/Cinzel-Bold.ttf` pochodzi z utrwalenia osi `wght=700` istniejącego
`assets/Cinzel.ttf`. Własne zasoby gry ani oryginalne fonty nie zostały usunięte.
Fonty pozostają na dołączonych licencjach SIL OFL 1.1, dopuszczających
dystrybucję z grą komercyjną. Proces był lokalny, bez płatnych usług.
Hashe i metoda: [provenance.json](provenance.json).
Końcowe wyniki i hashe paczki: [QA.md](QA.md).

## Odtworzenie i testy

Przygotowanie fontu: Python + `fonttools==4.60.1`,
`tools/media/prepare_typography.py`. Gra nie wymaga Pythona ani FontTools.
Skrypt nie zastępuje istniejącego odmiennego pliku bez uprzedniej kopii.

`pwsh -NoProfile -File tests/typography_qa.ps1 -NoAudio`
buduje pomocniczy renderer wersji gracza w świeżym katalogu `dist`.
Wykonuje 57 zrzutów rzeczywistej gry przy skalach 80%, 100%, 120%, porównuje
pokrycie pikseli liter ze starymi odmianami, sprawdza wszystkie rozmowy
oraz istniejące testy opisów, handlu, statystyk, umiejętności i tekstu walki.
Rejestruje faktyczne wywołania rysowania tekstu w `text-bounds.tsv`.

`tools/media/verify_typography.py --qa <katalog-checks> --no-audio`
niezależnie odczytuje wagi fontów, hashe, zrzuty, pokrycie i położenie napisów.

Po `package.ps1`: `tests/typography_package_qa.ps1 -NoAudio -HelperRoot <katalog-testu>`
rozpakowuje świeży ZIP, sprawdza hashe wszystkich zasobów, uruchamia 180 klatek
rzeczywistego EXE i powtarza cały test typografii na rozpakowanych zasobach.
Kontroluje brak trybu DEV, brak dźwięku i niezmienione preferencje audio.

Kopia poprzedniej paczki i zmienianych źródeł:
`art/pipeline/backups/typography-20261026-162842`.

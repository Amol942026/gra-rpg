# Kaldera Popiołu i bieg bohatera — 10.11.2026

## Przeprawy i wcześniejsze zapisy

Sceneria w rewizji 7 usuwa dekoracje, których rzut zahacza o pomost.
Sprawdzane są wszystkie 12 przepraw z uwzględnieniem wysokości podłoża
i rozmiaru widocznej dekoracji. Po wczytaniu wyprawy rewizja czyści
zarówno aktywną Kalderę, jak i stan Kaldery zapisany podczas pobytu
w innej krainie. Porównanie zostało wyrenderowane z układu rewizji 6
i pokazuje ten sam most, krater i stalagmity co zgłoszenie.

## Ruch ramion

[Edytowalny klip Synfig](../hero/run-arms.sif) ma zakres wymachu
od −37° do +35°, podczas gdy wcześniej wynosił od −26° do +24°.
Osobna krzywa zgięcia reguluje pracę przedramienia. Przywiedzenie
ramion wciąż prowadzi łokcie blisko tułowia. Synfig wyrenderował
48 klatek. Wyeksportowane próbki są w `src/generated/hero_run_arms_data.hpp`.

## Materiały przeciwników

ComfyUI 0.44.2 wygenerował na CPU siedem ujęć img2img 384 × 384
za pomocą DreamShaper 8 i LCM-LoRA SD 1.5. Z obrazu każdego gatunku
przeniesiono kolor i fakturę na osobne części w ośmiu kierunkach.
Krita dodała detale materiałów na siódmej warstwie każdego mastera KRA.
Wyjściowe atlasy zachowują rozmiary, kotwice i alfę piksel w piksel.
Identyfikatory zadań, licencje i sumy plików są w
[`redesign-receipt.json`](../../assets/volcano/redesign-receipt.json).

Przebieg `C:/AI2D/pipeline/runs/emberfall-caldera-art-20261110` zawiera
grafy, surowe obrazy, siedem masterów KRA, eksporty, porównania oraz kopie
zastąpionych paczek. Comfy Desktop uruchomił instancję po aktualizacji,
a grafy wykonano w tle przez lokalny port 8188.

## Odbiór

536 kontroli renderera objęło mosty, bieg w ośmiu kierunkach, a także
ruch, atak, ciało i maskę trafienia każdego gatunku. Próba dobywania
i chowania miecza objęła 485 856 kombinacji pozy oraz 201 600 klatek
symulacji wejścia. Brak przecięć głowy i tułowia. Testy gry wykonano
w ukrytych oknach bez otwierania urządzenia audio.

# Szczątki Żywej Wydmy — 26.10.2026

Oryginalny warstwowy master Krita: `dune-fallen.kra`. Pięć warstw obejmuje
osiadły piasek, załamane pasma osadu, bezwładne odnóża, pękniętą maskę
z zamkniętymi oczami i rozsypane drobiny bursztynu. Nie jest to obrócony
zrzut żywej postaci ani nowy model 3D. Narysowano osiem widoków 2D.

Runtime: `assets/sprites/dune-fallen.png`, 1536×512 RGBA; osiem komórek
384×256 i kotwica (192, 160). Źródło jest proste RGBA, loader wykonuje
premultiplikację alfa wymaganą przez obecny renderer ciał. Zastępuje tylko
wiersz drugiego bossa przy ładowaniu; stary działający atlas innych ciał
pozostaje niezmieniony. Cień i maska kursora korzystają z nowych pikseli.
Nie dodaje pracy generującej grafikę w pętli gry. Rezydentne komórki zajmują
3 145 728 bajtów RGBA8, tyle samo co osiem poprzednich komórek tego bossa.

Produkcja: `tools/media/create_dune_corpse.py --root <świeży-katalog-sprites>
--no-audio` (jedno polecenie). Natywne narzędzia istniejącego Krita MCP,
sprawdzenie krita_state i krita_preview, zapis i ponowny odczyt KRA/PNG.
Master: `C:\AI2D\sprites\emberfall_dune_fallen_20261026_v2`.
Bez zewnętrznych materiałów, usług, sampli i opłat.

Testy: `tests/balance_qa.ps1 -NoAudio`,
`tools/media/verify_balance.py --qa <katalog-checks> --no-audio`,
`tests/balance_package_qa.ps1 -NoAudio`. Kontrola porównuje źródło KRA,
każdy z ośmiu faktycznie załadowanych obrazów, poprawność alfa i klikania,
łup z prawdziwego bossa i zgodność zasobów po rozpakowaniu paczki gracza.

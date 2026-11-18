# Animacje Wędrowca

Sześć projektów Synfig pokazuje spoczynek, chód, cięcie, bieg, mocne cięcie i przewrót w ośmiu rzutach. Wycinanki zawierają malowany kaftan, płaszcz i głowę obróconą odpowiednio do ruchu. Sylwetki kontrolne pokazują zachowanie stóp, barków i dłoni. Biblioteka wartości w każdym SIFZ zawiera dziewięć kanałów używanych w grze. Pliki zachowują punkty kontaktu aktualnego rigu i dają płynny obrót pomiędzy kierunkami.

`gestures.sifz` zawiera 20 nazwanych ujęć na jednej osi czasu. Obejmują one ataki toporem, sztyletem i włócznią, naciąg i wypuszczenie cięciwy, przygotowania i wykonania mocnych ataków, chowanie lub dobywanie broni przy pasie i za plecami, reakcję na cios oraz upadek i wstawanie. Warstwy karku i płaszcza korzystają z wycinanek przygotowanych z nowych materiałów Krity w katalogu `parts`. W grze faza ujęcia podąża za rzeczywistym ruchem broni. Pancerz, obuwie, hełmy, kaptury, tarcze i kołczan zachowują indywidualne mocowania.

Źródłem kontaktów stóp, przenoszenia ciężaru i rotacji miednicy w chodzie jest `art/hero/walk/walk.sif`. W biegu odpowiadają za nie kanały `gait_*` w `run.sifz`. Zaokrąglona podeszwa przetacza się przez piętę i nosek. Podczas biegu po wybiciu obie stopy są chwilowo nad ziemią. Ruch ramion pochodzi z `art/hero/run-arms.sif`.

Podczas biegu tułów pochyla się średnio o 14,6° do przodu, a kark utrzymuje wzrok na kierunku ruchu. Łokieć cofa się za biodro, po czym zgięte przedramię wychodzi przed tułów razem z barkiem. Arkusze z ośmiu kierunków i osobne studium ramion wskazują pola `run_render_dir` i `run_arm_render_dir` w `motion-receipt.json`.

Po edycji i zapisaniu projektów można odtworzyć tablice ruchu z głównego katalogu repozytorium.

```powershell
$mediaPython = "$env:USERPROFILE/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe"
& $mediaPython tools/media/walk_synfig.py --export-project art/hero/walk/walk.sif --header src/generated/hero_walk_data.hpp --no-audio
& $mediaPython tools/media/traveler_performance.py --directory art/hero/traveler-performance --header src/generated/traveler_performance_data.hpp --run-gait-header src/generated/hero_run_gait_data.hpp --export-only --no-audio
& $mediaPython tools/media/traveler_gestures.py --directory art/hero/traveler-performance --header src/generated/traveler_gestures_data.hpp --export-only --no-audio
```

Do odświeżenia plansz po edycji wartości użyj `traveler_performance.py` z opcją `--refresh-from art/hero/traveler-performance` i nowym katalogiem `--directory`. Opcja `--render` wykonuje pełny eksport PNG.

Pliki `motion-receipt.json` i `gesture-receipt.json` podają sumy plików, czasy renderu i liczbę różnych klatek. Gotowe PNG dla aktualnego chodu i biegu są w katalogu wskazanym przez `locomotion_render_dir`. Pozostałe ujęcia są w `C:/AI2D/animations/emberfall-wanderer-performance-20261117`, w katalogach `painted-final` i `gesture-final`.

Test `tests/hero_art_qa.cpp` korzysta z renderera postaci w wersji gracza. Oprócz podstawowych animacji sprawdza 32 akcje w ośmiu kierunkach i sześciu fazach. Dla każdej pary akcja–kierunek zapisuje liczebność sylwetki, liczbę pikseli przy brzegu i liczbę różnych klatek w `qa-hero-art/traveler-motion/coverage.csv`. Plansze PNG obok tabeli przedstawiają rzeczywiste wyposażenie i aktualne materiały bohatera.

## Naciąg łuku

W `bow-nock.sif` pierwsze 0,60 s strzału przedstawiają sięgnięcie prawą dłonią za bark,
wyjęcie strzały wzdłuż osi kołczanu i założenie jej na cięciwę. Przód i profil pokazują
chwyt łuku, prawą dłoń oraz oba końce strzały w 37 kolejnych pozycjach. Początek
profilu leży w 56. jednostce. Po wyjęciu grotu dłoń prowadzi strzałę przed tułów,
a przy docelowej cięciwie palce przesuwają się do nasadki. Gra uwzględnia kształt
wybranego łuku, żeby zacząć naciąg dokładnie w miejscu założenia nasadki.
Prowadnice w Synfigu mają dodatni kierunek X po stronie ręki sięgającej po strzałę.
W rzucie gry od przodu bark ten widać po lewej stronie obrazu. Mapowanie tej osi
oraz kości obu dłoni łucznika znajduje się w `src/hero_quiver.hpp`.

W `bow-arms.sif` zielony punkt oznacza chwyt łuku, złoty palce na cięciwie,
a miedziany tor palców po strzale. Lewa połowa dokumentu pokazuje przód,
prawa profil. Ujęcia dzielą wysokość nad podłożem. Współrzędna pozioma
po stronie profilu ma początek w 32. jednostce. Pierwsze 85 klatek prowadzi
dłoń od nałożenia strzały pod prawą żuchwę. Dalsze 24 interwały opisują
jej ruch po zwolnieniu. Gra skaluje ten ruch względem rzeczywistego naciągu.
Cięciwa wraca niezależnie od dłoni.

Po zmianie prowadnic w Synfigu odtwórz dane z głównego katalogu repozytorium.

```powershell
& $mediaPython tools/media/hero_bow_synfig.py art/hero/traveler-performance/bow-nock.sif src/generated/hero_bow_nock_data.hpp --no-audio
& $mediaPython tools/media/hero_bow_synfig.py art/hero/traveler-performance/bow-arms.sif src/generated/hero_bow_data.hpp --no-audio
```

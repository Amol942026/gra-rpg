# Naturalniejszy chód — 21.10.2026

Nowy główny klip chodu powstał z natywnych krzywych Synfiga. Stopy mają
węższe, równoległe tory. Kolana są rozwiązywane w kierunku przodu nogi,
przy stałych długościach uda i goleni. Wysokość bioder rośnie w pojedynczym
podparciu i maleje przy przejęciu ciężaru przez drugą nogę. Ograniczono
skręcanie klatki piersiowej, utrzymano przeciwbalans i stabilne spojrzenie.
Noga powrotna jest unoszona niżej, z płynną zmianą kierunku stopy.

Kontakt uwzględnia rzeczywiste trzy zaokrąglone części buta. Początek kroku
ma uniesiony czubek, środek płaskie podparcie, a koniec wybicie z palców.
Stały ruch stopy w fazie podparcia jest określony w układzie postaci.
Prędkości gry, sterowanie i odległościowy zegar animacji pozostały zachowane.

Klip działa w dotychczasowym rendererze i systemie wyposażenia. Nie zmieniono
grafiki postaci, tekstur ani źródeł pozostałych bazowych klipów. Starsza
dodatkowa rotacja tułowia i ramion nie jest powtórnie dodawana do nowego
chodu; warstwa peleryny pozostała. Przejście do ataku zaczyna się od tej samej
pozy, bez przywracania starego kołysania w pierwszej klatce.

## Źródła i odtwarzanie

- Synfig: `C:\AI2D\animations\emberfall_walk_20261021_v1` — master SIF,
  części Krity, 60 natywnych renderów i manifest.
- Kopia w repo: `art/hero/walk`.
- Generator/eksporter: `tools/media/walk_synfig.py`.
- Runtime: `src/generated/hero_walk_data.hpp`, 121 póz, 17 istniejących kości,
  57 596 bajtów danych klipu; interpolacja w `src/hero_motion.hpp`.
- Podgląd SIF przedstawia kontrolny rig 2D. Końcowy wygląd wyposażenia i ruchu
  oceniamy w rendererze gry. Nie uruchamiano Blendera ani modelowania 3D.
- Kopia stanu sprzed zmian:
  `dist/qa-walk/backup-20261021-134602`.
- Rzeczywisty render poprzedniej wersji: `dist/qa-walk-before`.

Eksport z zapisanego SIF i ponowny render: instrukcja w `README.md`.
Skrypty testowe: `tests/walk_qa.ps1`, `tests/walk_metrics.cpp` oraz
`tools/media/verify_walk.py`. Wszystkie uruchomienia otrzymują `--no-audio`.

## Sprawdzone wyniki

- 87 214 testów logiki i animacji: PASS, w tym kontakt buta z podłożem,
  stała długość obu segmentów nóg, oś kolan, podparcie przez całą pętlę,
  zamknięcie pętli i zgodne rozpoczęcie ataku.
- Test bohatera w rzeczywistym rendererze: 424/424 PASS.
- Chowanie miecza: 485 856 kombinacji pozy oraz 201 600 klatek wejścia
  przy 30/60/120 Hz, bez przecięć powiększonych brył głowy i napierśnika.
- Natywny Synfig: 60 poprawnych PNG; ponowny eksport SIF daje bajtowo
  identyczny nagłówek C++.
- Zapisane punkty kontrolne SIF i faktyczne stawy z C++: maksymalna różnica
  odczytu CSV poniżej 0,00006 jednostki.
- Osiem kierunków × dziesięć póz oraz 120 klatek z trzech stron. Wizualnie
  porównano przód i profil z poprzednim rendererem.

Pomiar dla pełnego chodu, w jednostkach istniejącego rigu:

| Parametr | Przed | Po |
| --- | --- | --- |
| Najniższa część buta względem poziomu podłoża | -3,36564 | -0,00047 |
| Najwyższa z dolnych powierzchni nogi podporowej | -0,14580 | 0,00390 |
| Maksymalny rozstaw kolan | 14,0469 | 12,8286 |
| Maksymalny skręt klatki piersiowej | 6,997° | 0,5° |

Kontakt jest kontrolowany na bryłach zaokrąglonych części buta i oceniany
również na obrazach. Nie oznacza pełnej symulacji kontaktu z dowolną geometrią
terenu ani zmiany prędkości poruszania postaci.

Raporty i podgląd: `dist/qa-walk/asset-verification.json`, `metrics.json`,
`clearance.json`, `unit-tests.txt`, `render-verification.json` i
`walk-preview.gif`.

## Wydanie gracza i wydajność

Ponownie zbudowany ZIP rozpakowano do
`dist/qa-hero-art/package-20261021-140454/Emberfall`. Zweryfikowano EXE,
wszystkie 95 zasobów oraz 180 klatek uruchomienia wydania gracza bez urządzenia
audio i bez trybu deweloperskiego. Native KRA/SIF i inne mastery nie trafiły
do archiwum. Na rozpakowanej kopii ponownie przeszły:

- renderer bohatera 424/424;
- wyposażenie 364/364, pięć poprawnie zapisanych i zdekodowanych PNG;
- gesty miecza 724/724;
- 120 klatek chodu, osiem kierunków, bez obcięcia i błędów zapisu;
- porównanie stawów z SIF i ponowny eksport nagłówka C++.

SHA-256 ZIP:
`e516108188018eb26f054986f38f4710881f323b23f06956cfeb2f39e3b70302`.
Automatyczne ponowienie po aktualnym `tests/hero_package_qa.ps1`:
`pwsh -NoProfile -File tests/walk_package_qa.ps1`.
Inny master można podać parametrem `-SynfigProject`.

Benchmark po przebudowie, 900 klatek, 1440×900, OpenGL/V-sync 60 Hz:
**59,128 fps**, CPU średnio **8,11639 ms**, CPU p95 **9,3382 ms**,
p99 całej klatki **20,3562 ms**. Jedna klatka przekroczyła 25 ms,
maksimum **28,0035 ms**. Jest to wynik bieżącego testu, nie gwarancja
stałych 60 fps we wszystkich scenach. Przygotowanie klipu nie dodaje
generowania animacji ani odczytu zasobów w pętli gry.

Dowody: `dist/qa-walk/package-verification.json`,
`packaged-render-verification.json`, `packaged-equipment-tests.txt`,
`packaged-stow-tests.txt`, `benchmark.txt` oraz
`dist/qa-hero-art/package-verification.json`.

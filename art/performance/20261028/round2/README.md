# Dalsza optymalizacja — 28.10.2026

Wdrożono cztery dalsze usprawnienia bez zmiany rozdzielczości, animacji,
szczegółowości i liczby obiektów względem poprzedniego wariantu.
Wszystkie testy były ukryte i uruchamiane z `--no-audio`.

## Zmiany

1. Geometria postaci i obrysów trafia bezpośrednio do przygotowanych tablic.
   Usunięto tysiące małych operacji `vector::insert` i powtarzanych zmian
   rozmiaru wektora podczas pakowania. Wysyłana jest rzeczywista liczba
   wierzchołków, a nie cała pojemność bufora.
2. Próbki wiatru są współdzielone przez koronę, cień i obrys drzewa w tej
   samej klatce. Cache ma najwyżej 256 wpisów, jest wstępnie rezerwowany,
   uwzględnia pozycję oraz czas i jest czyszczony na początku klatki.
3. Lokalizacje pięciu parametrów shadera cieni są pobierane przy jego
   tworzeniu i odtwarzane po ponownym załadowaniu. Nie są wyszukiwane
   ponownie w każdej klatce.
4. Shadery i bufory geometrii cieni są przygotowane podczas ładowania.
   Początkowy bufor mieści 98 304 wierzchołki. Gdy potrzeba większej
   pojemności, rośnie skokowo, zamiast być odtwarzany dla każdego niewielkiego
   przyrostu geometrii. W sprawdzonych scenach nie alokowano go ponownie.

Oryginalne ścieżki pakowania i obliczania wiatru pozostają referencją dla QA.
Normalna gra wybiera szybszy wariant. Poprzednia rozdzielczość świata/HUD,
sposób prezentacji DXGI i parametry rozgrywki pozostają zachowane.

## Pomiar porównawczy

Poniżej wynik powtórzenia na zasobach wypakowanego ZIP-a. Każdy wariant
ma trzy naprzemienne serie, po 20 klatkach rozgrzewki i 80 mierzonych
klatkach; łącznie 240 próbek na wariant i scenę. Stan początkowy jest taki
sam, a pakowanie i współdzielenie wiatru są włączane/wyłączane w tym samym
procesie. Średni czas renderowania sześciu scen spadł o **13,03%**.

| Scena | Średnio przed | Średnio po | Zmniejszenie czasu |
| --- | ---: | ---: | ---: |
| Północ lasu | 7,08 ms | 5,98 ms | 15,6% |
| Gęsty las | 8,65 ms | 6,97 ms | 19,4% |
| Leśny boss | 8,40 ms | 7,38 ms | 12,1% |
| Krawędź lasu | 10,80 ms | 10,25 ms | 5,2% |
| Oaza | 6,61 ms | 5,80 ms | 12,3% |
| Boss pustyni | 6,22 ms | 5,17 ms | 16,9% |

[Pomiar wypakowanej paczki](package-render-cost.json),
[pierwszy pomiar trzech serii](render-cost.json).
Pierwszy przebieg dał około 12,9% poprawy średniej; jego p95 przy leśnym
bossie było gorsze mimo lepszej średniej. Dane pozostawiono w raporcie.
W powtórzeniu na paczce p95 zmniejszyło się we wszystkich sześciu scenach.
Pomiar A/B nie przypisuje osobnego procentu wstępnej alokacji ani
buforowaniu lokalizacji parametrów shadera, obecnym w obu ścieżkach.

Czas obejmuje ukończoną pracę renderera CPU i GPU z jedną synchronizacją
na końcu klatki. **To nie jest pomiar wyświetlanych FPS.** Ukryte okno
nie potwierdza stałych 60 FPS na ekranie. Nie uruchamiano widocznych
testów ani nie przywracano okien wbrew preferencji użytkownika.

## Weryfikacja

- 158 661 kontroli logiki i animacji oraz 3403 zapisów — PASS.
- [813 kontroli renderera](renderer.json) — PASS: 219 porównań modeli,
  75 odmian sprzętu, 396 prób kursora, zgodność HUD i sześciu scen,
  stały budżet cache i używanie wstępnie zaalokowanych buforów.
  [Pełny log](render-checks.txt).
- [Wypakowany ZIP](package-verification.json) — PASS: 115 zgodnych zasobów,
  180 klatek rzeczywistej wersji gracza w ukrytym oknie, ponowione
  813 kontroli i pomiary A/B. Audio nieotwarte, preferencje zachowane,
  brak DEV i źródeł natywnych w archiwum.
- [Manifest źródeł](source-manifest.json).

ZIP: `Emberfall-Windows-x64.zip`.
SHA-256: `42f30cfeaa14a42b40ba905106fd378a49388c4e89733f464c9038bfdd38373c`.
Player EXE: `9c1111a61d2419de604bfbb3707cedd998ab62ec302bb632cdc16f3fa11ce9b1`.

Backup poprzedniego ZIP-a i zmienianych źródeł:
`art/pipeline/backups/performance-round2-20261028`.
Nie zmieniano innych aplikacji, ustawień Windows ani zapisów użytkownika.
Nie wykonywano commita, pusha ani publikacji projektu.

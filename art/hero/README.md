# Bohater — Krita i Synfig, 20.10.2026

Nowe malowane detale są faktycznie używane przez renderer bohatera w świecie,
portrecie i scenach fabularnych. Wyposażenie nadal wybiera własne materiały,
kształty i insygnia dla wszystkich 39 przedmiotów. Miecz podczas biegu pozostaje
w pochwie, a łuk korzysta z istniejącego IK rąk. Nie wykonano nowego modelu 3D.

## Grafika

- `hero-paints.kra`: oryginalny master, 14 nazwanych warstw Krity, 512×384 RGBA.
- `assets/sprites/hero-paints.png`: jedyny nowy atlas runtime, 786432 bajty RGBA8
  (0,75 MiB). Wczytywany w `makeArt`, zwalniany przy zamykaniu gry.
- Twarz, włosy, szwy tkaniny, skórzane karwasze i detale butów są mapowane na
  istniejące animowane części. Korzystają ze wspólnego testu głębokości z eq.
- Komórki tułowia, ramion, nóg, szala i miecza służą do edytowalnego podglądu
  wycinanki w Synfigu. Nie zastępują dynamicznych 39 wariantów eq w grze.
- `paint-provenance.json`: UUID warstw, sumy PNG i części, stan zgodności MCP.
  Aktualny master: `C:\AI2D\sprites\emberfall_hero_20261020_features_only_v4`.
  Starsze wersje są zachowane. Komórka twarzy zawiera wyłącznie oczy, brwi,
  nos i usta na przezroczystym tle. Skórę i jej oświetlenie zapewnia oryginalna
  powierzchnia głowy. Usunięto malowane obrysy, płaszczyzny policzków/skroni,
  zarost i bliznę; bez hełmu nie są też rysowane brązowe paski przy policzkach.
  UV są ograniczone do wnętrza komórki, aby filtrowanie nie pobierało
  sąsiedniej grafiki. Nie ma osobnego panelu ani odsunięcia powierzchni.
  Przód i tył włosów także korzystają z istniejących powierzchni głowy,
  z zachowaniem brązowej podmalówki pod transparentnymi pasmami. Nakładka
  używa równej głębokości, bez przesuwania położenia lub obrysu głowy.
  Transparentne części podglądu wycinanki w `motion/parts` pozostają z v1
  i służą jako niezależne materiały poglądowe.

Ponowna produkcja przez rzeczywiste natywne narzędzia Krita MCP (nowy katalog):

```powershell
$py = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/create_hero_paints.py --root C:\AI2D\sprites\hero_nowy --logs C:\AI2D\pipeline\hero_nowy --no-audio
```

## Animacja

Sześć plików `motion/*.sif` z podłączonymi częściami Krity: idle, walk, run,
slash, heavy, roll. Synfig 1.4.5 wyrenderował wszystkie 354 klatki PNG przy 60 Hz
bez braków. `C:\AI2D\animations\emberfall_hero_20261020_v1\renders` zawiera
oryginalne sekwencje, logi i manifesty dekodowania każdej klatki.

Są to warstwy ruchu dodatkowego: ciężar tułowia, oddech, przeciwbalans głowy,
ramiona i opóźnienie peleryny. Dawne klipy pozostają w źródłach; chód ma od
21.10.2026 nowy klip główny opisany poniżej. Synfig nie był używany do
odtwarzania lub modyfikowania dawnych
źródeł Blendera. Wszystkie nowe krzywe są natywnymi `rotate/animated/waypoint`
SIF; eksport czyta dokładnie te zapisane wartości do
`src/generated/hero_secondary_data.hpp`. Gra interpoluje je między próbkami,
nakłada na bazową pozę i nie wykonuje operacji plikowych w pętli klatki.
Sama warstwa dodatkowa nie zmienia pozycji ani kontaktu stóp klipu głównego.

Ponowny eksport po edycji istniejących SIF:

```python
from pathlib import Path
from tools.media.hero_synfig import compile_tracks
compile_tracks(sorted(Path('art/hero/motion').glob('*.sif')),
               Path('src/generated/hero_secondary_data.hpp'))
```

Nowy komplet projektów i renderów (nowy katalog):

```powershell
& $py tools/media/hero_synfig.py --root C:\AI2D\animations\hero_nowy --parts C:\AI2D\sprites\hero_nowy\parts --header src/generated/hero_secondary_data.hpp --render --no-audio
```

SIF pokazuje części w widoku 2D do redagowania ruchu; końcowy wygląd, widoczne
eq, wszystkie kierunki i animacje oceniamy w rzeczywistym rendererze gry.
Test `tests/hero_art_qa.ps1` zapisuje pełną postać w ośmiu kierunkach, 420 klatek
siedmiu stanów i zrzuty lasu/pustyni. Dodatkowo obowiązują testy eq i celowania.
Każde uruchomienie testowe otrzymuje `--no-audio`.

## Naturalny chód — 21.10.2026

Aktualny master: `C:\AI2D\animations\emberfall_walk_20261021_v1\walk.sif`.
Przenośna kopia jest w `art/hero/walk`. Natywne węzły Synfiga określają
kontakty i przetaczanie stóp, tory kolan, przenoszenie ciężaru, ruch bioder,
przeciwbalans tułowia i dłonie. Pętla ma 61 zapisanych punktów kontrolnych.
`tools/media/walk_synfig.py` eksportuje z zapisanych węzłów 121 póz dla
istniejących kości, z IK i stałymi długościami nóg/rąk, do
`src/generated/hero_walk_data.hpp`. Nie używa Blendera ani nowych modeli.

Gra wykorzystuje nowy klip w `hero_motion::evaluate`, przy zachowanym
sterowaniu, prędkości i zegarze ruchu. Stary dodatkowy obrót tułowia i ramion
jest pomijany wyłącznie dla składowej chodu, bo te ruchy zawiera już nowy klip.
Pozostały ruch peleryny, biegu i ciosów jest nadal osobno mieszany.
Tablica klipu ma 57 596 bajtów danych, bez odczytów plików w pętli gry.

Nowy master i rzeczywisty render Synfiga:

```powershell
& $py tools/media/walk_synfig.py --root C:\AI2D\animations\hero_walk_nowy --parts art/hero/motion/parts --header src/generated/hero_walk_data.hpp --render --no-audio
```

Eksport po ręcznej edycji istniejącego SIF:

```python
import sys
from pathlib import Path
sys.path.insert(0, 'tools/media')
from walk_synfig import compile_clip
compile_clip(Path('art/hero/walk/walk.sif'), Path('src/generated/hero_walk_data.hpp'))
```

Podgląd SIF jest dwuwymiarowym podglądem rigu z częściami Krity; końcową
postać i wyposażenie oceniamy w rendererze gry. Kontrole, porównanie z poprzednim
chodem i wydanie gracza: [WALK_QA.md](WALK_QA.md).

## Chowanie i wyciąganie miecza

`stow/sheathe.sif` i `stow/draw.sif` to natywne projekty Synfiga. Aktualne
oryginały i 72 sprawdzone klatki są w
`C:\AI2D\animations\emberfall_sword_stow_clearance_20261020_v1`. Każdy gest trwa 0,6 s;
powrót do chodu odwraca ruch od bieżącego miejsca. Krzywe z zapisanych węzłów
`animated/waypoint` trafiają do `src/generated/sword_stow_data.hpp`.

`src/hero_sheath.hpp` nakłada je na istniejącą pozę i rozwiązuje IK prawej ręki
do wlotu pochwy przy lewym biodrze. Ostrze najpierw przechodzi przed postać po
prawej stronie, obraca się czubkiem w dół, a dopiero potem trafia do pochwy.
Podpisany kąt obrotu zastępuje najkrótszą interpolację kwaternionu,
która w poprzedniej wersji przechodziła przez głowę. Wlot pochwy i jej oś
także są eksportowane z natywnych węzłów SIF. Ten sam miecz wsuwa się za pochwę:
nie pojawia się druga rękojeść, a długości ostrza i ramienia pozostają stałe.
Nogi i kontakt stóp zachowują bazowy ruch. Po wsunięciu dłoń puszcza rękojeść;
atak przerywa gest, umożliwiając natychmiastowe uderzenie.

Eksport po ręcznej edycji krzywych:

```python
from pathlib import Path
import sys
sys.path.insert(0, 'tools/media')
from sword_stow_synfig import export
export(Path('art/hero/stow/sheathe.sif'),
       Path('src/generated/sword_stow_data.hpp'))
```

Nowy komplet źródeł i renderów:

```powershell
& $py tools/media/sword_stow_synfig.py --root C:\AI2D\animations\hero_stow_nowy --parts art/hero/stow/parts --header src/generated/sword_stow_data.hpp --no-audio
```

Końcową korektę toru ostrza i weryfikację ZIP opisuje
[SWORD_CLEARANCE_QA.md](SWORD_CLEARANCE_QA.md). Starsze `QA.md`
i `SWORD_STOW_QA.md` są raportami poprzednich wersji.

Wyłącznie oryginalne grafiki i ruch przygotowane dla Emberfall. Bez obrazów,
sampli, modeli ani usług zewnętrznych. Master KRA i SIF pozostają poza assets,
więc nie trafiają do archiwum gracza.

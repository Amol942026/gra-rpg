# Kaldera: dopracowanie grafiki i kompozycji

Zakończono 29.10.2026. Raport odbioru: `QA.json`.

- Kopia wcześniejszej wersji: `art/pipeline/backups/volcano-polish-20261029`.
- Nowe źródła Krita: `C:/AI2D/sprites/emberfall-volcano-polish-v2-20261029`.
- Staging eksportów: `C:/AI2D/pipeline/volcano-polish-20261029/runtime`.
- Autor: `tools/media/polish_volcano_art.py`, wyłącznie natywne Krita MCP.
- Animacje: `tools/media/polish_volcano_motion.py`, natywny Synfig, osobne części.
- Miejsce docelowe nowych SIF: `C:/AI2D/animations/emberfall-volcano-polish-20261029`.
- Zmiany kodu: ciągłe powierzchnie podłoża/lawy, grupy dekoracji, cztery
  warianty każdego elementu i wspólna geometria rysowania/cienia/picking.
- Format zapisu v10 dodaje tylko rewizję scenografii. Prawdziwy v9 fixture
  powstał przed zmianą protokołu. Test wymaga zachowania wszystkich pozostałych
  bajtów danych v9, obu strumieni losowych i dotychczasowego postępu.
- Odbiór: pełne testy logiki/zapisu, dekodowanie KRA/PNG/SIF, ukryty renderer,
  oględziny widoków oraz oba wypakowane ZIP-y. Każdy start gry wymaga
  `--no-audio --background-test`, bez przywracania i fokusowania okien.

Pierwsza próba miała zerową długość natywnego pociągnięcia po zaokrągleniu.
Zastąpiono ją rzeczywistym pociągnięciem 4 px i sprawdzono niepuste bounds
warstwy. Nieudane próby zachowano poza eksportem produkcyjnym. Przerwany
odczyt preview upiora wznowiono po sprawdzeniu stanu; zapisany KRA/PNG już
istniał, więc nie ponawiano malowania ani zapisu.

# Tor miecza przy chowaniu i dobywaniu — 20.10.2026

Poprzedni gest interpolował najkrótszy obrót od uniesionego ostrza do osi
pochwy. Taki łuk przechodził przez głowę; testy chwytu i ciągłości tego nie
wykrywały. Nowe natywne krzywe Synfiga określają podpisany kąt ostrza oraz
osobne współrzędne chwytu. Miecz najpierw odsuwa się po prawej stronie,
obraca do przodu i w dół, po czym ręka przenosi go przed ciałem do pochwy.

Pochwa jest mocowana po zewnętrznej stronie lewego biodra, z wlotem nieco
przesuniętym do przodu. Wlot i oś pochodzą z zapisanych parametrów SIF.
IK zachowuje rzeczywisty zasięg ręki. Jelec i rękojeść także omijają ciało.
Przy szybkich zmianach chód/bieg pozycja ręki z mieczem pozostaje na zewnątrz
napierśnika mimo utrzymującego się ruchu ramion z biegu.

Pozostały czas 0,6 s, odwracanie od aktualnego punktu, mocowanie miecza
po wsunięciu, natychmiastowa reakcja na atak i oryginalny ruch nóg.
Nowy gest jest stosowany tylko do mieczy poza aktywnym ciosem.

## Edytowalne źródła

- `C:\AI2D\animations\emberfall_sword_stow_clearance_20261020_v1`:
  sheathe.sif, draw.sif, części Krity, 72 wyrenderowane PNG i manifesty.
- Repo: `art/hero/stow` oraz `src/generated/sword_stow_data.hpp`.
- Generator i eksport: `tools/media/sword_stow_synfig.py`.
- Poprzedni kod, projekty, EXE, ZIP i podgląd zachowano w
  `dist/qa-sheath-clearance/backup-20261020-172016`.
- Grafika twarzy pozostaje bez zmian, SHA-256 PNG:
  `b9a426d0b76e46c6ae3fbaa22df7b2a5a07465d674098606f5060d3bcc9538e7`.

## Kontrole geometrii i obrazu

`tests/sword_stow_clearance.cpp` niezależnie sprawdza odcinki widocznego
ostrza, jelca i rękojeści względem powiększonych brył głowy oraz napierśnika
w ich aktualnych układach kości. Objętości obejmują włosy/hełm i szerokość
ostrza. Nie porównują jedynie parametrów ścieżki z tymi samymi parametrami.

- 485 856 kombinacji fazy gestu, kroku, ruchu i pośredniego chód/bieg:
  zero przecięć. Sprawdzane są także stany pośrednie między klatkami źródła.
- 201 600 klatek z rzeczywistym `AnimationState.advance`, 30/60/120 Hz,
  zatrzymywaniem i przełączaniem biegu co 80/120/300 ms: zero przecięć.
- Maksymalny błąd kontaktu z wlotem pochwy: poniżej 0,0001 jednostki.
- 84 088 testów logiki i animacji: PASS, w tym regresja przecinania głowy.
- GPU/rzeczywiste wejście i pełna postać: 724/724 PASS.
- Renderer bohatera: 424/424 PASS.
- 11 etapów gestu × 8 kierunków: wygenerowano i zdekodowano obrazy;
  wizualnie sprawdzono odsunięcie, ustawienie nad pochwą i końcowe wsuwanie.
- 180 klatek z trzech stron: `dist/qa-sheath/transition-preview.gif`.
- 72 natywne PNG Synfiga oraz dokładny eksport krzywych do C++: PASS.

Wszystkie uruchomienia gry i diagnostyki otrzymują `--no-audio`.
Jedno polecenie ponawia kontrolę geometrii i renderera:

```powershell
pwsh -NoProfile -File tests/sword_stow_qa.ps1
```

Raporty: `dist/qa-sheath-clearance/clearance.json`,
`dist/qa-sheath/asset-verification.json`, `dist/qa-sheath/tests.txt`.
Wyniki geometrii dotyczą brył testowych; do oceny końcowego wyglądu służą
również obrazy rzeczywistego renderera ze wszystkich kierunków.

## Paczka i wydajność

`Emberfall-Windows-x64.zip` rozpakowano do
`dist/qa-hero-art/package-20261020-173744/Emberfall`.
Hashe EXE i wszystkich 95 zasobów są zgodne. Uruchomiono 180 klatek wydania
gracza, bez urządzenia audio i trybu deweloperskiego. Na rozpakowanych zasobach
ponownie przeszły kontrole bohatera 424/424, gestu 724/724 oraz wyposażenia
364/364. Wszystkie pięć obrazów testu wyposażenia zapisano w tym świeżym
katalogu i niezależnie zdekodowano, omijając blokadę starszych podglądów.

SHA-256 ZIP:
`297c6a72cf7b7b0436684e2279857b15cbabadc934be18d147246d5829997a57`.

Benchmark: 900 klatek, 1440×900, OpenGL/V-sync 60 Hz, `--no-audio`.
Pierwsze dwa przebiegi nowej wersji: 58,9163 i 54,2721 fps.
Następnie wykonano bez kompilacji między próbami porównanie kopii poprzedniego
EXE i aktualnego EXE z tym samym katalogiem zasobów:

| Wersja | Średnie fps | CPU średnio | Klatki >25 ms |
| --- | --- | --- | --- |
| Poprzednia, z kopii przed tą poprawką | 57,9447 | 8,39979 ms | 2 |
| Poprawiony tor ostrza | 58,2938 | 8,04425 ms | 5 |

W tym porównaniu nie wykazano regresji średniej wydajności poprawki.
Wyniki bieżącej sesji nie potwierdzają stałych 60 fps; nie należy zastępować
ich korzystniejszym pomiarem z poprzedniej wersji. Zachowano wszystkie
przebiegi w `dist/qa-sheath-clearance/benchmark*.txt`.
Raport paczki: `dist/qa-sheath-clearance/package-verification.json`.

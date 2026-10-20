# Leśny krzew — Krita, 20.10.2026

`forest-bush.kra` to oryginalny master z siedmioma nazwanymi warstwami:
cień kontaktowy, ściółka, zdrewniałe pędy, podszyt, liście tylne, liście
przednie i refleksy. Grafika ma nieregularną, szeroką koronę z pojedynczymi
liśćmi, zamiast odrębnych okrągłych kęp. Pędy wyrastają z kotwicy podłoża.
Nie użyto zasobów zewnętrznych ani płatnych usług.

- Master i eksport: `C:\AI2D\sprites\emberfall_forest_bush_20261020_v1`.
- Dostarczany PNG: `assets/sprites/forest-bush.png`, 360×460 RGBA.
- Kotwica korzeni: (180,416) w źródle, czyli (90,208) po eksporcie 0,5×.
  Jest wspólna z sortowaniem, maską kursora i rzutowaniem cienia.
- Rzeczywiste granice alfa: [61,294,312,426]. Nieprzezroczyste pędy i ściółka
  przechodzą przez kotwicę. Półprzezroczysty cień nie trafia do maski kursora.
- `forest_bush_art.hpp` wczytuje źródło tylko podczas `makeArt`, wypala je
  w istniejącym `propArt[2]` w rozdzielczości 2×, po czym zwalnia źródło.
  W klatce gry pozostaje jeden zwykły sprite i istniejący cień otoczenia.
- Tekstura krzewu zajmuje 662400 bajtów RGBA8. Wzrost względem starego
  bufora 180×230: 496800 bajtów (ok. 0,47 MiB). Bez generowania grafiki i I/O
  w czasie klatki.

Ponowna produkcja natywnym Krita MCP do nowego katalogu:

```powershell
$py = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/create_forest_bush.py --root C:\AI2D\sprites\forest_bush_nowy --logs C:\AI2D\pipeline\forest_bush_nowy --no-audio
```

Sumy i UUID warstw: `forest-bush-provenance.json`.
Testy rzeczywistego renderera: `tests/face_bush_qa.ps1`.

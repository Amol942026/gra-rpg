# Brama Świtu w menu startowym

Nową ilustrację lasu, kamiennego łuku i ścieżki wygenerowano lokalnie w
ComfyUI z oryginalnego rysunku Emberfall. Kontury kompozycji prowadził
ControlNet Canny. W Kricie dodano ryty i odbicia na kamieniu, przygaszono
partie pod typografią oraz ustawiono gęstość ruchomej mgły.

Warstwowy plik `brama-switu.kra` zawiera sześć warstw. Gra wczytuje cztery
eksporty PNG 1440 × 900 z `assets/menu/`. Istniejąca pętla Synfig porusza
pierwszym planem, mgłą i blaskiem. Tytuł oraz przyciski dalej rysuje silnik.
Łączny budżet tekstur pozostaje równy 20 736 000 bajtów RGBA8.

- `gate.json` i dołączona referencja pozwalają odtworzyć generowanie w ComfyUI.
- `provenance.json` zawiera SHA-256 plików, źródła i licencje trzech modeli.
- [Podgląd w grze](podglad.png) pochodzi z rozpakowanej wersji gracza.
- [Raport QA](QA.json) obejmuje trzy skale menu, 60 ujęć animacji i zgodność
  klatki kończącej pętlę z pierwszą. Obie rozpakowane edycje ukończyły po
  180 klatek startu w ukryciu i bez dźwięku. Dziewięć wcześniejszych błędów
  kontroli alfy pauzy wystąpiło w obu przebiegach ogólnego testu.

Surowy render, oryginalne tekstury i kopie przed wdrożeniem są w
`C:/AI2D/sprites/emberfall-menu-comfy-20261106`.

Aktualne tło 1920 × 1200 px i edytowalny master HD są w [Ostrości Bramy Świtu](ostrosc/README.md). Poniżej pierwsza wersja ilustracji.

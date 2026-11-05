# Bestie Morza Piasków

ComfyUI wygenerowało lokalnie cztery materiały z oryginalnego atlasu Emberfall.
Bursztynowy skorpion dostał drobniejsze nakrapianie pancerza, skarabeusz
turkusowe płytki i miedziane żłobienia, kobra zachodzące na siebie łuski,
a Żywa Wydma smugi nawianego piasku. Te same materiały wykorzystano przy
wypalaniu wyglądu ciał we wszystkich ośmiu kierunkach.

[Porównanie przed i po](porownanie.png) pokazuje identyczne pozy wyrenderowane
przez grę. [Warstwowy atlas Krity](desert-materials.kra) zawiera cztery
pustynne materiały i osobną warstwę pozostałych pól. Silnik korzysta dalej
z jednego atlasu 512 × 512 i dotychczasowych tekstur ciał.

## Źródła

- `inputs/` zawiera referencje z własnych grafik projektu. Nazwy odpowiadają
  węzłom LoadImage w `workflows/`. Każdy graf używa DreamShaper 8,
  LCM-LoRA SD 1.5 i ControlNet Canny SD 1.5.
- `provenance.json` zapisuje źródła modeli, licencje, identyfikatory
  zadań ComfyUI oraz SHA-256 wejść i eksportów.
- Surowe rendery, skrypty montażu, oryginalne tekstury i kopie przed wdrożeniem
  są w lokalnym katalogu `C:/AI2D/sprites/emberfall-desert-comfy-20261105`.

## Weryfikacja

[Raport QA](QA.json) obejmuje 96 póz pustynnych bestii w ośmiu kierunkach
oraz 32 widoki ciał. Porównanie potwierdziło zgodność renderera GPU z jego
trybem bez buforowania i zachowanie maski wskazywania skorpiona.
192 pozy leśnych przeciwników i cztery próbki animacji bohatera są
identyczne piksel po pikselu. Ogólny test bestiariusza miał te same 14
błędów przed zmianą i po niej.

Rozpakowana wersja gracza przeszła test startu, a wersja deweloperska
uruchomiła się przy Żywej Wydmie. Obie ukończyły po 180 klatek w ukryciu
i bez dźwięku. Porównano też 143 zasoby każdej paczki. KRA i eksport PNG
zweryfikowano przez odczyt pikseli po rozłączeniu funkcji podglądu MCP Krity.

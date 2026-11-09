# Leśny rozbójnik — 10.11.2026

Bordowy kaptur, maska z ciemnego płótna, wytarta skórzana kurta, pasek
z metalową sprzączką i buty z wyraźną podeszwą. Model FLUX.1 Schnell
przygotował lokalny projekt w ComfyUI. Tekstury skóry i kaptura dopracowano
w Kricie, dodając szwy i przetarcia. Współdzielony atlas 512 × 512 używa
nowych pikseli w dwóch komórkach rozbójnika.

Za głęboki przysiad w spoczynku wynikał z wysokości bioder 32 przy stopach
na wysokości 4 i długości nogi 33,4. W nowej pozie biodra stoją na wysokości
37,37, a kąt ugięcia kolan wynosi ok. 5°. Pozycje bioder i stóp są mieszane
z pełną pozą chodu przed ponownym rozwiązaniem kolan. Kontakt kroku i faza
zależą od faktycznej drogi po rozstrzygnięciu kolizji. Oddycha tułów,
a lekkie ugięcie wraca przy przygotowaniu ciosu i zamachu.

Cztery projekty Synfig zawierają klucze spoczynku, chodu, przygotowania
i zamachu. Runtime używa ich do prowadzenia barków, ręki z mieczem i luźnego
końca kaptura. Ciało po walce ma ten sam kaptur i maskę. W gotowym atlasie
ciał podmieniony jest wiersz rozbójnika we wszystkich ośmiu kierunkach.

## Źródła

- Graf API i historia ComfyUI w
  `C:/AI2D/pipeline/runs/media-ai-suite-20261109`, zadanie
  `b1af77cd-83a6-41ce-9442-09bbb8b40b6e`.
- Pięć warstw KRA, eksport i logi Krita w
  `C:/AI2D/sprites/emberfall-bandit-20261110`.
- Cztery SIF, 192 klatki Synfig, nagłówek i raporty w
  `C:/AI2D/animations/emberfall-bandit-20261110`.
- Skrypty `tools/media/bandit_paints.py` i `tools/media/bandit_motion.py`.
- Kopie sprzed zmiany w `C:/AI2D/pipeline/runs/bandit-20261110/backup`.

[FLUX.1 Schnell, Apache-2.0](https://huggingface.co/black-forest-labs/FLUX.1-schnell) ·
[Konwersja GGUF](https://huggingface.co/city96/FLUX.1-schnell-gguf)

## Sprawdzenie

Test renderera objął 104 ujęcia, w tym osiem kierunków spoczynku i po
48 klatek chodu oraz ataku. Wszystkie 323 kontrole przeszły, razem
z widocznością ostrza i wskazywaniem kursorem. Test logiki i animacji
przeszedł 345 308 kontroli. Porównanie pikseli wykazało zmiany wyłącznie
w dwóch komórkach materiałów i ośmiu kierunkach ciała rozbójnika.
Oba ZIP-y po wypakowaniu przeszły sprawdzenie 287 zasobów i próbę
180 klatek. Próby gry były ukryte i bez otwarcia urządzenia audio.
Podglądy i raport są w `C:/AI2D/pipeline/runs/bandit-20261110`.

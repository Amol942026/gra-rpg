# Zależności i zasoby

- Efekty specjalne Kaldery `assets/volcano/caldera-attack-fx.png` powstały
  z własnych warstwowych ilustracji Emberfall w Kricie. Materiały dopracowano
  w lokalnym ComfyUI z modelem DreamShaper 8 Lykona, na warunkach opisanych
  niżej. Ruch kart 2D i pozy wycinanek przygotowano w Blenderze. Mastery,
  grafy i wagi modelu pozostają poza paczką gry.

- Atlas zaklęć `assets/sprites/spells.png` to własne ilustracje 2D Emberfall
  z warstwowej Krity, animowane w Synfig 1.4.5. Nowe efekty żywiołów i łuku
  zmontowano lokalnie w Csound i Audacity: nagrania łuków Bena Jaszczaka
  i Briana Nelsona / StillNorthMedia (CC0), „Torch Fire Spell” spookymodem
  (CC BY 3.0) oraz „Ice & Electricity Magic” Iwana „qubodup” Gabovitcha
  (CC BY 3.0). Pełna atrybucja, źródła, licencje i opis modyfikacji:
  `assets/licenses/audio-elements-credits.txt`.
  Dokumentacja: `art/gameplay/eldrin-spells-20261109/`. Mastery poza paczką.

- Pozostałe ikony przedmiotów `assets/sprites/loot-icons.png` są własnymi
  warstwowymi ilustracjami 2D Emberfall z Krity, wykończonymi lokalnie
  w ComfyUI / DreamShaper 8 (Lykon, warunki autora wymienione niżej).
  Źródła i workflow: `art/items/loot-20261109/`. Trzy warianty przeturlania
  wykorzystują nagrania CC0 Kenney (tkanina, skóra i kontakt z podłożem),
  zmontowane lokalnie w Csound/Audacity. Licencje: `assets/licenses/audio-foley-CC0.txt`.
  Można rozprowadzać z grą. Mastery i wagi pozostają poza paczką gracza.

- Muzyka `assets/audio/` pochodzi z oryginalnych kompozycji LMMS Emberfall.
  Aktualny zestaw zawiera 50 efektów w trzech wariantach i obróbkę Audacity.
  28 efektów walki wykorzystuje nagrania **CC0 1.0**: Kenney (Impact Sounds,
  RPG Audio), artisticdude (Swishes Sound Pack), Darsycho (Monster snarls),
  przycięte, filtrowane i łączone w Csound. Pełne źródła, autorzy i licencja:
  `assets/licenses/audio-foley-CC0.txt`, `art/audio/physical-20261108/README.md`.
  Wcześniejsze pozostałe efekty są oryginalną syntezą projektu; nowe nagrania
  żywiołów i łuków wymieniono wyżej. Wszystko można rozprowadzać
  z grą. Nie użyto płatnych usług; narzędzi produkcyjnych nie dołączamy.

- Atlas ikon wyposażenia `assets/sprites/equipment-icons.png` pochodzi z
  nowych źródeł przygotowanych dla Emberfall w Blenderze 5.2.2 LTS,
  z materiałami wykończonymi lokalnie w ComfyUI / DreamShaper 8 (Lykon,
  uprawnienia autora i odnośniki wymienione niżej). Zachowano źródła,
  workflow i pochodzenie w `art/equipment/icons-20261108/`.
  Można go rozpowszechniać z grą; nie zawiera zewnętrznych modeli 3D ani
  cudzych zdjęć. Modele źródłowe i wagi nie są częścią paczki gracza.

- **raylib 5.5**, Ramon Santamaria i współtwórcy, licencja zlib: https://github.com/raysan5/raylib/blob/5.5/LICENSE . Pełna licencja jest dołączona w `assets/licenses/raylib-LICENSE.txt`.
- **Manrope**, Mikhail Sharanda i współtwórcy, SIL Open Font License 1.1. Pełna licencja: `assets/Manrope-LICENSE.txt`. Źródło: https://github.com/google/fonts/tree/main/ofl/manrope .
- **Lato**, Łukasz Dziedzic, SIL Open Font License 1.1. Pełna licencja: `assets/Lato-LICENSE.txt`. Źródło: https://github.com/google/fonts/tree/main/ofl/lato .
- **Cinzel**, Natanael Gama, SIL Open Font License 1.1. Pełna licencja: `assets/Cinzel-LICENSE.txt`. Źródło: https://github.com/google/fonts/tree/main/ofl/cinzel .
  `assets/Cinzel-Bold.ttf` to lokalnie utrwalona odmiana o wadze 700 z dołączonego fontu zmiennego, na tej samej licencji OFL. Proces i hashe: `art/typography/provenance.json`.
- **w64devkit 2.0.0 / MinGW-w64** — narzędzia kompilacji, osobne licencje w folderze `tools/w64devkit`. Licencje runtime dystrybuowanego z plikiem EXE znajdują się w `assets/licenses/`.
- Grafiki i kod gry Emberfall powstały dla tego projektu. Nie zawierają grafik z Tibii.
- Materiał ciągłej ścieżki Kaldery `assets/volcano/road.png` został narysowany
  lokalnie na czterech warstwach Krity dla Emberfall, bez zewnętrznych grafik,
  modeli i płatnych usług. Można go rozpowszechniać z grą. Natywny master,
  pochodzenie i odbiór: `art/terrain/diversity-20261108/`.
- Otoczenie Kaldery w `assets/volcano/` oraz bazowe części przeciwników są
  oryginalnymi ilustracjami 2D wykonanymi w Kricie dla Emberfall. Animacje
  pochodzą z lokalnego Synfiga. Otoczenie nie używa zewnętrznych obrazów,
  modeli ani płatnych usług. Detale scenografii i płynąca lawa:
  `art/volcano/river-detail/README.md`; poprzednia warstwa:
  `art/volcano/polish/README.md`.
- Siedem atlasów przeciwników Kaldery (`slag-crawler`, `lava-salamander`,
  `fire-imp`, `ash-wraith`, `obsidian-sentinel`, `ember-drake`, `varkhor`)
  zawiera od 04.11.2026 malowanie materiałów wygenerowane lokalnie w ComfyUI
  na podstawie własnych części projektu. Model **DreamShaper 8**, Lykon:
  https://huggingface.co/Lykon/DreamShaper oraz
  https://civitai.com/models/4384/dreamshaper . Opublikowane uprawnienia autora
  dopuszczają komercyjne wykorzystanie wygenerowanych obrazów (`Image`)
  i modyfikacje; zachowano kopię metadanych licencji i SHA-256 modelu.
  Nie rozpowszechniamy wag modelu w grze. Generowanie nie korzysta z płatnych
  API ani wysyłania projektu do chmury. Źródła, workflow i pochodzenie:
  `art/volcano/comfy/README.md` i `provenance.json`.
- Atlas bohatera `assets/sprites/hero-paints.png` zawiera malowanie materiałów
  wygenerowane lokalnie w ComfyUI z własnego atlasu Emberfall. Użyte wagi:
  DreamShaper 8 (Lykon, warunki wymienione wyżej), LCM-LoRA SD 1.5
  (https://huggingface.co/latent-consistency/lcm-lora-sdv1-5, OpenRAIL++)
  i ControlNet Canny SD 1.5 (https://huggingface.co/lllyasviel/control_v11p_sd15_canny,
  OpenRAIL; konwersja FP16 w formacie SafeTensors). Pochodzenie i hashe:
  `art/hero/comfy/provenance.json`.
- Materiały Skorpiona bursztynowego, Skarabeusza miedzianego, Kobry
  piaskowej i Żywej Wydmy w Morzu Piasków opracowano lokalnie w ComfyUI
  na bazie grafik Emberfall. Wykorzystano wymienione wyżej DreamShaper 8,
  LCM-LoRA SD 1.5 i ControlNet Canny SD 1.5. Pochodzenie, licencje,
  workflow i hashe są zapisane w art/creatures/desert-comfy/.
- Odłamki `assets/sprites/guardian-fragments.png` powstały jako sześć oryginalnych warstw Krity; ich rozpad, obroty i odbicia opracowano w natywnym projekcie Synfig. Źródła i pochodzenie: `art/gameplay/guardian/shatter/`. Bez zewnętrznych grafik, modeli i płatnych usług.
- Duży głaz Strażnika `assets/sprites/guardian-stone.png` to własna sześciowarstwowa grafika z Krity, z ziemią i korzeniami; wyrwanie i oburęczny rzut znad głowy opracowano w natywnym projekcie 2D Synfig. Źródła i pochodzenie: `art/gameplay/guardian/`. Bez zewnętrznych grafik, modeli i płatnych usług.
- Ilustrację Bramy Świtu w `assets/menu/` odświeżono lokalnie w ComfyUI
  na podstawie własnego rysunku Emberfall. Wykorzystano opisane wyżej
  DreamShaper 8, LCM-LoRA SD 1.5 i ControlNet Canny SD 1.5. Wykończenie
  warstw wykonano w Kricie, a animacja korzysta z istniejącej pętli Synfig.
  Pochodzenie i edytowalne źródło są w `art/menu/comfy/`.
  Detale tła powiększono lokalnie w ComfyUI modelem Real-ESRGAN
  (realesr-general-x4v3, Xintao Wang, BSD-3-Clause). Źródło, licencja i
  dokument Krity są w `art/menu/comfy/ostrosc/`.
- Malowane detale bohatera `assets/sprites/hero-paints.png` powstały w Kricie, a dodatkowe krzywe animacji w Synfig Studio, wyłącznie z oryginalnych części projektu. Źródła i pochodzenie: `art/hero/README.md`. Bez zewnętrznych modeli, grafik ani płatnych usług.
- Pięć materiałów scenerii Lasu Mchu i Morza Piasków wygenerowano
  lokalnie w ComfyUI z użyciem DreamShaper 8 i LCM-LoRA SD 1.5.
  Pomalowano czternaście obiektów i dopracowano detale w Kricie.
  Edytowalne źródła i pochodzenie są w `art/scenery/forest-desert-comfy/`.
- Leśny krzew `assets/sprites/forest-bush.png` został narysowany w Kricie dla Emberfall, bez zasobów zewnętrznych. Warstwowy master i pochodzenie: `art/scenery/README.md`.
- Świerk, brzoza i dąb w `assets/sprites/forest-{spruce,birch,oak}.png` to oryginalne warstwowe ilustracje narysowane natywnymi narzędziami Krity dla Emberfall. Bez zewnętrznych grafik, modeli i płatnych usług. Źródła i pochodzenie: `art/scenery/trees/`, `art/scenery/README.md`.

- Dwie pustynne fontanny w `assets/sprites/fountain-*.png` to oryginalne grafiki Emberfall narysowane na warstwach w Kricie. Wodę i opróżnianie animowano w lokalnym Synfigu. Bez zewnętrznych grafik, modeli, sampli i płatnych usług; można rozpowszechniać z grą. Natywne źródła i pochodzenie: `art/gameplay/fountains/`.

- Szczątki Żywej Wydmy w `assets/sprites/dune-fallen.png` to oryginalna warstwowa grafika Emberfall narysowana w Kricie w ośmiu kierunkach. Bez zewnętrznych obrazów, modeli i płatnych usług; można rozpowszechniać z grą. Master KRA i pochodzenie: `art/gameplay/dune-fallen/`.

- Panorama Doliny Świtu i dwie warstwy balkonu w `assets/sprites/tower-{valley,balcony-back,balcony-front}.png` to własne ilustracje Emberfall narysowane natywnymi narzędziami Krity. Bez zewnętrznych obrazów, modeli i płatnych usług; można rozpowszechniać z grą. Warstwowe źródła KRA i pochodzenie: `art/scenery/tower-balcony/README.md`, `art/scenery/tower-balcony/provenance.json`.

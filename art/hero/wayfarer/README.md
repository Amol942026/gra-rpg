# Wędrowiec

Nowy bohater ma ciemnobrązowe, zaczesane włosy, krótki turkusowy płaszcz
z różą wiatrów i dopasowany kirys startowy. Obrót głowy ma 64 ujęcia wokół
pełnego okręgu. Podczas skręcania gra interpoluje dwa sąsiednie ujęcia według
rzeczywistego kąta karku. Dotyczy to także przejścia przez przód i tył.
Otwarte hełmy i kaptury odsłaniają część twarzy, a zamknięte hełmy
korzystają z własnego poszycia. Wygląd broni, zbroi, tarcz i dodatków
pochodzi z przedmiotów założonych w grze.

## Źródła graficzne

- `heads.kra` zawiera pięć widoków i oddzielny retusz brwi, skroni i oczu.
  Eksport `assets/sprites/hero-heads.png` (640 × 128 px) zachowuje pierwotne
  ujęcia i służy do wyświetlania głowy na sprzęcie bez obsługi shadera obrotu.
- W `turn` znajdują się cztery dodatkowe ujęcia Sunburst, ich retusz w Kricie,
  eksport RGBA i graf lokalnego ComfyUI. Dziewięć malowanych ujęć i odbicia
  siedmiu składają się na 16 klatek kluczowych. Model FILM dopowiada trzy klatki
  pomiędzy każdą parą. Do gry trafia `assets/sprites/hero-head-turn.png`
  (1024 × 1024 px). Sposób odtworzenia opisuje [źródło obrotu](turn/README.md).
- `materials.kra` zawiera ubranie, włosy, rysy twarzy, buty, karwasze,
  rękawy, bryczesy i kaletkę. Eksport to `assets/sprites/hero-paints.png`,
  512 × 384 px. Dolny rząd służy też animowanym częściom kończyn.
- `armor.kra` zawiera nowy front kirysu w osobnej warstwie. Jego eksport
  zajmuje dotychczasowy arkusz `assets/sprites/hero-armor-paints.png`
  (512 × 256 px). Nowy panel zajmuje trzecią komórkę w drugim rzędzie.
- W `sources` są trzy ilustracje Sunburst w pierwotnej rozdzielczości,
  a w `sources/comfy` obydwa grafy DreamShaper 8 i dziesięć malowanych
  materiałów po lokalnym img2img. Sumy źródeł, identyfikatory przebiegów,
  wersje programów i odwzorowanie eksportów opisuje `provenance.json`.

Krita przechowuje edytowalne warstwy. Gra wczytuje teksturę obrotu, maskę
sylwetki oraz wspólną teksturę materiałów postaci podczas inicjalizacji.
Ten sam zestaw materiałów służy do wyświetlania postaci w świecie, portrecie
i rozmowach z Eldrinem.

[Edytowalne animacje Synfig i odtworzenie krzywych](../traveler-performance/README.md)
· [Podgląd z renderera gry](preview.gif)

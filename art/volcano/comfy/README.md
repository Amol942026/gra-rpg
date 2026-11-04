# Kaldera Popiołu — malowanie przeciwników w lokalnym ComfyUI

04.11.2026. Użytkownik zamówił ComfyUI i potwierdził generowanie lekkim
modelem na lokalnej karcie RTX 3060 Laptop 6 GB. Źródłem są własne atlasy
Emberfall; niczego nie wysyłano do usług generujących obrazy.

## Zakres

Sześć gatunków i Varkhor: `slag-crawler`, `lava-salamander`, `fire-imp`,
`ash-wraith`, `obsidian-sentinel`, `ember-drake`, `varkhor`.
Przemalowanie materiałów obejmuje osiem kierunków każdej postaci: kamienne
płyty, odpryski i żar szczelin, łuski i skrzydła, fałdy szat, metal pancerza,
twarze oraz broń. Sylwetki, alfa, kotwice, sześć części, animacje Synfig
i dane rozgrywki pozostają zachowane.

Nie generowano osobnych klatek chodu. Złożony widok postaci jest wejściem
img2img, a jego nowe malowanie jest rzutowane na widoczne obszary części.
Zakryte połączenia zachowują poprzednie piksele. Dokładna oryginalna alfa
utrzymuje działające obrysy, cienie i wskazywanie kursorem. Żar jasnych
szczelin jest chroniony przy końcowym mieszaniu.

## Źródła i model

Komplet roboczy: `C:\AI2D\sprites\emberfall-caldera-comfy-20261104`.

- `backup/`: siedem atlasów sprzed zmiany, ich hashe w `originals.json`.
- `inputs/`, `projection.json`: 56 złożonych wejść i odwracalne mapowanie.
- `workflows/`: 56 rzeczywistych grafów API ComfyUI z promptami i ziarnami.
- `outputs/`: natywne PNG z ComfyUI; ich metadane zawierają wykonany graf.
- `runtime/`: siedem finalnych atlasów RGBA o identycznych rozmiarach i alfie.
- `masters/`: edytowalne pliki OpenRaster (ORA), sześć warstw części plus
  ukryty oryginał i potwierdzenia odczytu. Na tym PC nie ma Krity ani jej
  bridge, mimo starszego opisu środowiska; nie deklarujemy natywnych zapisów
  KRA. Źródłami produkcyjnymi są wykonane grafy ComfyUI, PNG i warstwowe ORA.
  Można je później otworzyć w Kricie bez utraty części lub oryginałów.
- `logs/`, `model.json`, `model-license.json`, `acceptance.json`: wykonanie,
  pochodzenie, kontrola źródeł i wyniki.

Model: [DreamShaper 8 / Lykon](https://huggingface.co/Lykon/DreamShaper),
`DreamShaper_8_pruned.safetensors`, 2 132 625 894 bajty, SHA-256
`879db523c30d3b9017143d56705015e15a2cb5628762c11d086fed9538abd7fd`.
Zweryfikowano hash LFS wydawcy. Opublikowane
[uprawnienia autora](https://civitai.com/api/v1/models/4384) dopuszczają
komercyjne użycie wygenerowanych obrazów (`Image`) oraz modyfikacje.
Wagi modelu pozostają poza grą. Pobieranie CLI miało błąd certyfikatu przed
transferem; użyto systemowego magazynu certyfikatów PowerShell, a następnie
porównano pełny SHA-256. Nie wyłączano weryfikacji TLS.

ComfyUI 0.41.3, tylko `127.0.0.1:8188`, standardowe lokalne węzły:
CheckpointLoaderSimple, LoadImage, CLIPTextEncode, VAEEncode, KSampler,
VAEDecode i SaveImage. Brak węzłów API. Próbki 512 × 512, 28 kroków,
DPM++ 2M / Karras, CFG 6,5, denoise 0,42; partie wykonywane kolejno.
Pierwsze siedem widoków uruchomiono przez MCP ComfyUI, dalszą partię przez
natywny lokalny endpoint kolejki. Każdy PNG zawiera faktycznie wykonany graf.

## Odtwarzanie

Python z aktualnego lokalnego runtime Codex (na tym komputerze profil Amol).
Wszystkie wejścia skryptów wymagają `--no-audio`.

1. `caldera_comfy.py prepare --root <nowy-katalog> --comfy <lokalny-ComfyUI> --no-audio`.
2. Zweryfikuj pierwszy graf przez ComfyUI, obejrzyj wynik przed dalszą partią.
3. `caldera_comfy_batch.py --root <katalog> --comfy <lokalny-ComfyUI> --no-audio`.
   Po niepewnym wyniku POST sprawdź kolejkę i historię; nie ponawiaj w ciemno.
4. `caldera_comfy.py project --root <katalog> --no-audio`.
5. `caldera_comfy_masters.py --root <katalog> --no-audio`.
6. `verify_caldera_comfy.py --root <katalog> --no-audio`.
7. Po ocenie grafiki `caldera_comfy.py deploy --root <katalog> --no-audio`.
   Zmiana aktywnego źródła od snapshotu blokuje wdrożenie.
8. `pwsh -NoProfile -File tests/volcano_qa.ps1 -NoAudio -OutputName <nowy-katalog>`.

Testy gry używają ukrytych okien i `--no-audio --background-test`.
Czas ukończonego renderowania nie jest pomiarem foreground FPS.
Aktualne wyniki i podglądy: `QA.json`, `previews/`. Starsze raporty
Krita/Laigter dotyczą poprzedniego malowania tych siedmiu atlasów;
podłoże, portal, lawa i scenografia z tamtych produkcji pozostają aktualne.

## Wyniki

- **728 kontroli eksportów**: 56 natywnych obrazów ComfyUI, zgodność grafów,
  rzeczywiste hashe wejść zapisane przez LoadImage, dokładna alfa 336 części,
  zmiana malowania każdego kierunku, siedem poprawnych źródeł ORA.
- **787 kontroli renderera**: siedem gatunków, osiem kierunków, 224 ujęcia
  bezruchu / chodu / ataku / szczątków, wskazywanie postaci i przejścia
  istniejącej lokacji. Obejrzano porównanie postaci, wojownika w ośmiu
  kierunkach i Kalderę z nowymi grafikami.
- Ukończony render na tym komputerze: średnia **4,03 ms**, p95 **5,55 ms**.
  Ukryte okno — nie jest to pomiar FPS prezentowanych na ekranie.
- **180 klatek** rzeczywistego startu EXE gracza: ukryte okno, brak audio.
- Zaktualizowane zasoby w `dist` i `dist/player` oraz oba ZIP-y.
  Każdy wpis archiwum sprawdzono bajtowo; zmieniono jedynie siedem PNG,
  potwierdzenie pochodzenia i LICENSES. Pozostałe wpisy i EXE są identyczne.
  Kopie zapasowe poprzednich paczek i zasobów: `delivery-backup/` w źródłach.
- Atlasowe RGBA8: **55 050 240 bajtów**, tyle samo co wcześniej. Oba
  preładowane obrazy tekstury / sylwetki nadal korzystają z tych samych
  wymiarów. Nie dodano shaderów, tekstur ani pracy na plikach w klatce.

Detale i hashe: `QA.json`, `delivery.json`, `provenance.json`.

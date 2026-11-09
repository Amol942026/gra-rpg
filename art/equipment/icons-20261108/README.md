# Szczegółowe ikony broni i wyposażenia

85 rodzajów wyposażenia ma własne ilustracje HUD: miecze, sztylety, topory,
łuki, hełmy, zbroje, tarcze, buty, płaszcze, pierścienie i amulety. Dotyczy
to plecaka, noszonego sprzętu, skrzyń, ciał, handlu i koła wyboru broni,
które korzystają z tego samego renderera ikon. Przedmioty i ich statystyki,
identyfikatory, rzadkość, liczniki oraz mechanika przeciągania pozostają te same.

Na zamówienie użytkownika źródła przygotowano w **Blenderze 5.2.2 LTS**:
nowe modele przedmiotów, zaokrąglone krawędzie, faktura materiałów, nity,
oploty, szwy, inkrustacje, kamienie i oświetlenie studyjne. Następnie lokalne
**ComfyUI 0.44.0 / DreamShaper 8** wykonało osobny img2img każdego przedmiotu:
512 × 512, DPM++ 2M/Karras, 20 kroków, CFG 6,4, denoise 0,44, zapisane seedy.
Zachowano pełną alfę Blenderowego renderu, aby nie zmienić sylwetek, cięciw
i otworów pierścieni. Wagi tego samego modelu i warunki autora były używane
już w poprzednich grafikach Emberfall; odnośniki są w `LICENSES.md`.

Gra nadal rysuje wyłącznie 2D. Dostarczony atlas RGBA ma 1536 × 1024,
komórki 128 × 128 z marginesem oraz 6291456 bajtów RGBA8. Wczytywany jest
raz podczas przygotowania grafiki, zwalniany przy wyjściu. Modele, Blender,
wagi, ComfyUI i generowanie nie są potrzebne graczowi i nie są dołączane
do ZIP. Nie użyto płatnych API ani przetwarzania w chmurze.

Edytowalny master `.blend`, 85 renderów, wejścia, indywidualne grafy,
siedem grafów zbiorczych, osiem natywnych historii Comfy, wyniki i podglądy:
`C:/AI2D/sprites/emberfall-equipment-20261108/v2`. Pierwszy jaśniejszy render
pozostawiono w nadrzędnym folderze; dostarczono poprawioną paletę sRGB.
Produkcja: `tools/media/blender_equipment_icons.py` i `equipment_icon_comfy.py`.
Odbiór prawdziwego HUD i handlerów teleportacji: `tests/inventory_icons_qa.ps1 -NoAudio`.
Końcowe wyniki: `QA.json`, pochodzenie: `provenance.json`.

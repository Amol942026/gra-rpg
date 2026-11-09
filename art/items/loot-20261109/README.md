# Pozostałe 23 przedmioty ekwipunku

Wszystkie 23 rodzaje przedmiotów poza wyposażeniem mają nowe ilustracje:
mikstury zdrowia/many, strzały, złoto, kryształ, część Serca Świtu, borowik,
materiały i esencje lasu/pustyni. 85 wcześniejszych ilustracji wyposażenia
pozostaje bajtowo identycznych. Wspólny renderer obsługuje oba atlasy w HUD,
plecaku, łupie, handlu, skrzyniach i pasku akcji. Nie zmienia statystyk,
identyfikatorów, stosów, cen, używania przedmiotów ani przeciągania.

Nowe źródła są 2D: własne warstwy Krity 5.3.4 (korpusy, gradienty materiałów,
szkło, refleksy i detale) w zapisanym KRA. Lokalny ComfyUI 0.44.0 /
DreamShaper 8 dopracował materiały: 512 × 512, 24 kroki DPM++ 2M/Karras,
CFG 6,6, denoise 0,5. Po przeglądzie dopracowano jeszcze monety i sierść,
z nowym źródłem KRA i mniejszym denoise. Oryginalne etapy zachowano.
Finalne malowanie ma dokładną alfę źródłową, z marginesem w komórkach.

Atlas `assets/sprites/loot-icons.png` ma 1024 × 384, 23 komórki 128 × 128
w ośmiu kolumnach, 1572864 bajty RGBA8. Jest wczytywany raz przy starcie
i zwalniany przy wyjściu. Mikstury zachowują czerwony/niebieski płyn, a
odłamek Serca Świtu złoty kolor. Oznaczenia rzadkości i liczby stosów
pozostają w interfejsie. Nie ma generowania ani odczytu plików w pętli klatek.

Mastery i pełne grafy/historie: `C:/AI2D/sprites/emberfall-loot-20261109`.
Produkcja: `tools/media/krita_loot_icons.py`, `loot_icon_comfy.py`.
Użyto istniejących lokalnych wag Lykona, z warunkami opisanymi w
`LICENSES.md` i pochodzeniu poprzedniego wyposażenia. Bez płatnych API,
nowych obrazów zewnętrznych i chmury. Mastery nie są częścią ZIP gracza.
Odbiór rzeczywistego HUD: `tests/inventory_icons_qa.ps1 -NoAudio`.
Końcowe wyniki: `QA.json`, źródła: `provenance.json`.

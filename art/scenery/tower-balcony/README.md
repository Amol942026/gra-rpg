# Balkon obserwatorium — Dolina Świtu

Na piątym piętrze wieży Eldrina można wyjść w prawo z obserwatorium na
kamienny balkon. Przejście jest ciągłe: zwykły chód lub bieg, bez teleportu
i dodatkowego okna. Balustrady i ścięte narożniki mają fizyczne granice.
Powrót do sali i schodów działa tą samą drogą.

Panorama przedstawia Dolinę Świtu, góry, lasy, jezioro, rzekę z mostem
i odległe ruiny. Wszystkie trzy ilustracje powstały lokalnie przez natywne
narzędzia Krita MCP. Nie użyto zewnętrznych materiałów ani usług AI.

Warstwowe źródła i eksporty:
`C:\AI2D\sprites\emberfall_tower_balcony_20261027_v3`.
`tower-valley.kra` ma siedem warstw; taras i przednia balustrada są osobnymi
dokumentami KRA. Pełne metadane i SHA-256 znajdują się w `provenance.json`.
Oryginały i wcześniejsza paczka są zachowane; kopia przed zmianą:
`art/pipeline/backups/tower-balcony-20261027-185601`.

Eksporty gry: `assets/sprites/tower-valley.png` (1050 × 900),
`tower-balcony-back.png` i `tower-balcony-front.png` (po 480 × 320).
Kotwica tarasu: piksel (64, 160), lokalne współrzędne radialne (6,1; 0).
Przednia balustrada jest rysowana po postaci, tylna przed nią. Panorama
pojawia się wyłącznie na piętrze obserwatorium. Pozostałe sale zachowują tło.

Trzy tekstury są wczytywane przed rozgrywką i zwalniane ze źródłami sal.
Łączna pamięć RGBA8: 5 008 800 bajtów, bez mipmap. Brak generowania zasobów
i odczytu plików w pętli renderowania. Nie dodano modeli ani shadera.
Pozycja balkonu mieści się w dotychczasowych współrzędnych świata;
istniejący zapis v7 przechowuje ją bez nowego pola lub migracji.

Odtworzenie grafiki w nowym folderze:

    python tools/media/create_tower_balcony.py --root C:\AI2D\sprites\<nowy-folder> --no-audio

Kontrola ruchu, zapisu, warstw, trzech skal i istniejących pięter:

    pwsh -NoProfile -File tests/tower_balcony_qa.ps1 -NoAudio

Każde uruchomienie gry wymaga `--no-audio`. Testy nie dotykają zapisów
ani preferencji użytkownika. Szczegółowy wynik końcowy jest w `QA.json`.

## Zweryfikowany wynik — 27.10.2026

- 110 661 kontroli logiki i animacji oraz 3403 kontrole zapisów — PASS.
- 1337 kontroli balkonu: zwykły ruch i bieg, ograniczenia każdej balustrady,
  powrót, łup, zapis v7, 12 widoków w skalach 80/100/120%, kolejność warstw
  i zachowane interakcje wszystkich pięciu pięter — PASS.
- 97 niezależnych kontroli trzech KRA i ich kompozycji, PNG, renderów,
  rzeczywistych pikseli minimapy i pomiarów renderowania — PASS.
- W końcowym ZIP-ie sprawdzono 115 zasobów i rzeczywisty start EXE przez
  180 klatek. Ponowiono 1337 + 97 kontroli. Brak DEV, źródeł natywnych
  i zmiany preferencji audio. Urządzenie audio pozostało zamknięte.
- P95 ukończonego renderowania GPU z rozpakowanej paczki: 13,37 ms
  w obserwatorium i 13,75 ms na balkonie; poniżej budżetu 16,67 ms.
  Pomiar nie obejmuje prezentacji obrazu ani pełnego FPS.

Paczka: `Emberfall-Windows-x64.zip` (15 616 701 bajtów).
SHA-256: `c91a11686b93d2a1c433d75b81d1b787463f3f04961c835481e3e8a720f3e613`.
Test rozpakowanej paczki: `dist/qa-tower-balcony-package-20261027-190848`.
Podglądy `preview.png` i `interior.png` pochodzą z tego przebiegu.

    python tools/media/verify_tower_balcony.py --qa <checks> --no-audio
    pwsh -NoProfile -File tests/tower_balcony_package_qa.ps1 -NoAudio -HelperRoot <folder-testu>

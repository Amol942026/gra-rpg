# Ataki specjalne Kaldery — 09.11.2026

- **Rozłupujący cios** ma wyraźniejsze cofnięcie i zamach wojownika,
  narastający fioletowy błysk przy ostrzu, znaczniki na ziemi oraz smugę obsydianu.
- **Żarowy oddech** ma unoszenie głowy podczas nabierania powietrza i trzy
  impulsy płomieni wyprowadzone od pyska draka. Na ziemi pozostaje obrys stożka.
- **Erupcja Varkhora** łączy gest maga, rozgrzewanie szczelin i następujące
  po sobie słupy lawy. Pociski żaru mają osobny gorący rdzeń i smugę.

Fazy obrazu korzystają z dotychczasowych zegarów zdolności. Geometria
trafienia, obrażenia, odnowienie, zapis i zdarzenia dźwiękowe pozostają zgodne
z mechaniką walki. Kierunki, cienie i wykrywanie kursora używają tej samej
pozy części postaci.

Warstwowe sylwetki powstały w Kricie. Dwie lokalne próby img2img w ComfyUI
na DreamShaper 8 Lykona dopracowały powierzchnie. Do atlasu weszły cztery
tekstury z drugiej próby z zachowaną alfą oryginału. Blender 5.2.2 LTS
zawiera cztery animowane karty 2D, trzy ścieżki gestów, spakowane obrazy oraz
wyrenderowane podglądy. Gra interpoluje krzywe odczytane z Blendera w 60 Hz.
Zasoby gry mieszczą się w jednym atlasie RGBA 2048 × 512 (4 MiB).

Mastery, grafy, renderowanie i logi są w
`C:/AI2D/sprites/emberfall-caldera-attacks-20261109`. Końcowy plik
`blender-delivery/caldera-attack-motion.blend` zawiera krzywe użyte przez
`src/generated/caldera_attack_motion.hpp`. Źródła produkcyjne są w
`tools/media/krita_caldera_attack_fx.py`, `tools/media/caldera_attack_comfy.py`
i `tools/media/blender_caldera_attacks.py`. Informacje o licencji modelu
są w `art/volcano/comfy/` oraz `LICENSES.md`.

Natywne potwierdzenia i wynik odbioru są obok tego opisu. Testy gry używają
`--no-audio --background-test` i działają w ukrytych oknach.

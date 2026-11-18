# Obrót głowy Wędrowca

`sunburst-inbetweens.png` przedstawia tę samą postać co wcześniejszy arkusz
`../sources/sunburst-heads.png` pod kątami 22,5°, 67,5°, 112,5° i 157,5°.
`turn-keys.kra` zachowuje oddzielne ujęcia oraz retusz skroni i włosów.
`turn-keys.png` to eksport RGBA z Krity 5.3.4.1. Oryginalne malowane ujęcia
przodu i tyłu są wspólne dla obu stron obrotu.

1. Skopiuj te źródła do nowego katalogu roboczego w `C:\AI2D\sprites`.
   Z głównego katalogu gry wykonaj `tools/media/wayfarer_turn.py prepare KATALOG --no-audio`
   interpreterem potoku mediów. Powstanie animowany PNG zawierający 16 ujęć
   oraz powtórzenie pierwszej klatki. Lewa połowa zawiera malunek na czerni,
   prawa ten sam malunek na bieli. Para służy do odzyskania przezroczystości włosów.
2. W lokalnym ComfyUI załaduj ten PNG jako wejście do `film-workflow.json`.
   Uruchom graf w formacie API. Mnożnik 4 daje 65 obrazów łącznie z klatką
   zamykającą pętlę. Użyj nowego katalogu wyjściowego dla każdego przebiegu.
3. Wykonaj `tools/media/wayfarer_turn.py pack KATALOG --rendered KATALOG_Z_65_PNG --no-audio`.
   Gotowy arkusz ma osiem rzędów po osiem komórek, liczonych od przodu przez
   prawy profil do tyłu i przez lewy profil z powrotem do przodu. Co czwarta
   komórka zachowuje dokładne piksele malowanego ujęcia. Plik docelowy gry
   to `assets/sprites/hero-head-turn.png`.

Lokalny ComfyUI 0.44.2 wykonał graf FILM w 4,462 s. Identyfikator przebiegu
to `0786f2ff-7fce-4437-8ede-a3b532b35b46`. Wagi FP16 o sumie SHA-256
`f226e51375dc839d4b40e5c3d63da560dd1ea1c962364ec78f5adf2d05db05c0`
służą wyłącznie do wytworzenia atlasu. [Wagi Comfy-Org](https://huggingface.co/Comfy-Org/Frame-Interpolation/tree/aa7009be4d1471ea0bac91d6fe4d7e4b637cbbfc/frame_interpolation) · [Model Google FILM i licencja](https://github.com/google-research/frame-interpolation)

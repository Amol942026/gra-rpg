# Dolina Świtu z obserwatorium

Panoramę wygenerowano lokalnie w ComfyUI na podstawie dotychczasowej grafiki
Emberfall. DreamShaper 8, LCM-LoRA i Canny ControlNet przygotowały ujęcie
doliny o świcie. Real-ESRGAN powiększył obraz przed eksportem w rozmiarze
1050 × 900. W Kricie dodano warstwy kamiennego mostu, ruin strażnicy
i refleksów na wodzie. Układ balkonu i kolejność rysowania obu balustrad
pochodzą z pierwotnego projektu. Podpis nad tarasem został usunięty.

Edytowalny dokument `dolina-switu.kra`, pierwotny KRA, obrazy pośrednie
i dziennik Krita MCP są w `C:\AI2D\sprites\emberfall-balcony-20261106`.
Zapisany eksport RGB z nieprzezroczystym kanałem alfa jest w
`assets/sprites/tower-valley.png`. Tekstura zajmuje 3 780 000 bajtów RGBA8,
tyle samo co poprzednia panorama. `provenance.json` zapisuje grafy,
identyfikatory wykonanych zadań, licencje oraz skróty SHA-256. Kopia sprzed
podmiany jest w podfolderze `delivery-backup` przy masterze.

`QA.json` zawiera wyniki testów obu rozpakowanych wydań, Opcji, sterowania
oraz balkonu. Testy nie otwierają urządzenia audio i pracują w ukrytych
oknach. Pomiary renderowania nie obejmują odświeżania monitora.


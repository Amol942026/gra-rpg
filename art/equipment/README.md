# Wygląd noszonego ekwipunku

Wszystkie 39 noszonych rodzajów przedmiotów korzysta z dokładnego Kind i tier
z sześciu slotów Game::equipment. Wspólny kontrakt jest w
src/equipment_appearance.hpp. Świat, portret, cień i wybieranie sylwetki myszą
przekazują ten sam zestaw do istniejącego renderera 2D/2.5D.

Palety materiałów i ornamenty zostały narysowane natywnie w Kricie przez
istniejące lokalne MCP. Nie użyto zewnętrznych grafik, modeli ani płatnych usług.
Master: C:\AI2D\sprites\emberfall_equipment_paints_20261019_v1.kra.
Kopia eksportu i pochodzenie: equipment-paints.png / equipment-paints.json.

Autorowanie nowej wersji, przy użyciu skonfigurowanego Pythona:

    python tools/media/create_equipment_paints.py --name UNIQUE_NAME --log-directory C:/AI2D/pipeline/UNIQUE_NAME --no-audio

Opcja --bake-only ponownie odczytuje zapisany PNG po edycji KRA i eksporcie
w Kricie. Generuje src/generated/equipment_paints.hpp: próbki kolorów i
scalone poziome fragmenty ornamentów. Gra korzysta z tych małych stałych
danych; nie wczytuje ani nie przetwarza wzornika w pętli renderowania.
PNG/KRA są źródłami i nie są wymagane w paczce dla gracza.

Geometria istniejących animowanych części pozostaje wspólna. Materiały,
nakładki i widoczność części zależą od konkretnego przedmiotu. Zdjęta zbroja
odsłania bazową tunikę; płaszcz pojawia się po założeniu płaszcza. Brak broni
nie rysuje domyślnego miecza. Przy biegu miecz pozostaje w pochwie, a tarcza
na plecach; łuk również przenosi tarczę na plecy.

Walidacja: build.ps1 -Test i tests/equipment_qa.ps1. Drugi test używa kodu
wersji gracza, rzeczywistych przeniesień ekwipunku, porównuje piksele świata
i portretu, sprawdza osiem kierunków i pięć stanów animacji. Wymaga --no-audio.
Wyniki i przegląd grafik: dist/qa-equipment.

# Wynik weryfikacji — 26.10.2026

PASS: 107 937 kontroli logiki, 1891 zapisu, 49 nowych kontroli interfejsu,
22 wcześniejsze statystyk, 90 umiejętności, 48 niezależnych kontroli KRA/PNG.
Powtórzono testy po rozpakowaniu nowego ZIP; sprawdzono 111 zasobów,
180 klatek startu, brak DEV, brak otwartego audio i niezmienione preferencje.

Zweryfikowane zachowania:
- Bez przycisków +/−; bez pustych kontrolek przy braku punktów. DODAJ/COFNIJ
  zmienia jedynie projekt przydziału; jawne zatwierdzenie odblokowuje umiejętność.
- Wspólny zapas 20 strzał dla MAX, dwukliku i pojedynczych zakupów. Brak
  strzał nie pobiera złota. Sprzedaż, podróż i odczyt nie odnawiają zapasu.
  Pierwsza śmierć Wydmy przywraca go do 20; duplikat zdarzenia nie daje dostawy.
- Rzeczywista śmierć drugiego bossa, nowy cień, wybór kursorem, otwieranie
  łupu PPM i zapis szczątków. Osiem obrazów GPU odpowiada Kricie po poprawnej
  premultiplikacji alfa; brak obcięcia i ciemnych prostokątów.
- Nowe progi PD: 100, 204, 312, 424, 540, 660, 784, 912, 1044, 1180.
  Rzeczywiste pliki zapisów v1–v6 działają. Migracja v7 zachowuje poziom,
  rozdane punkty i proporcjonalny postęp; zapas jest zapisany i walidowany.

Nieruchome tło pod P/U jest przechowywane w natywnej rozdzielczości;
interfejs odświeża się na bieżąco. Ruchoma gra nie używa tej kopii.
P95 renderowania z ukończeniem GPU (180 próbek/scenę): 4,4362 ms statystyki,
7,8336 ms szczątki Wydmy, 13,0342 ms Strażnik; obie fontanny poniżej 10 ms.
Budżet 16,6667 ms zachowany w pięciu scenach. To pomiar renderowania,
nie potwierdzenie pełnego FPS prezentacji w tle.

ZIP SHA-256: `2c071a7775beb780b73685474608f8429711560d5eb73a140ed7c28cd75bd97d`.
EXE SHA-256: `a106d11016b2758fc7d8c4641d00c3283cb9183b176d1915a4075bfea327e3c8`.

Szczegóły maszynowe: [QA.json](QA.json). Źródła: [README.md](README.md).

# Bliższa pochwa i szybkie rozdawanie atrybutów — 27.10.2026

Pochwę przysunięto do lewego biodra: wlot z `(-22, 8, 37)` na
`(-16, 13, 36)`. Jest o sześć jednostek bliżej osi ciała, nieco przed
biodrem, przy pasie. Oba skórzane mocowania wychodzą teraz z bocznej części
pasa. Pochwa nadal podąża za miednicą, a wsunięta broń za jej wlotem.
Zmiana obejmuje wszystkie miecze wykorzystujące tę samą konstrukcję.

Nowy natywny master Synfig:
`C:\AI2D\animations\emberfall_sheath_close_20261027_v1`.
Pliki `sheathe.sif`, `draw.sif`, receipt i dokładny eksport C++ zachowano
również w `art/hero/stow`. Ruch ręki dostosowano do nowego wlotu, pozostawiając
czas 0,6 s, odwracanie gestu przy zmianie chodu/biegu oraz natychmiastowe
dobycie przy ataku. Nie zmieniano grafik twarzy, stroju ani animacji nóg.

## Okno atrybutów P

Każdy wiersz ma dodatkowy **DODAJ 5**, kiedy pozostało co najmniej pięć punktów.
Przycisk wydaje dokładnie pięć punktów, bez automatycznego zatwierdzania.
Można użyć go kilka razy przy zaległych poziomach. DODAJ nadal dodaje jeden,
COFNIJ zwraca jeden z bieżącego podglądu; wcześniejsze punkty są zachowane.
Zatwierdzenie atrybutów nadal prowadzi do osobnego, wymaganego wyboru
umiejętności. Nie uczy ich przed potwierdzeniem.

Po najechaniu na nazwę, opis lub przyciski wiersza pojawia się opis przy
kursorze: działanie atrybutu, wynik obecny i po przydziale oraz efekt pięciu
punktów. Opisy odzwierciedlają faktyczne zasady: siła +2 ataku, moc magiczna
+3 obrażeń Kuli ognia, obrony +1 z redukcją 0,45 HP (minimum obrażeń 2 HP),
wytrzymałość +10 HP i mana +5 MP. Leczenie i regeneracja many pozostają
stałe. Tekst zawija się do szerokości panelu; panel mieści się na ekranie
i nie zasłania kursora. Jest rysowany ponad pozostałym HUD-em.

Zawinięte opisy są przechowywane dla aktualnej skali. Nie ma nowych tekstur,
ładowania plików ani generowania fontów w pętli wyświetlania podpowiedzi.
Nieruchomy świat w oknie P nadal korzysta z istniejącego cache'u.

## Wyniki testów

- 110 661 kontroli logiki i animacji — PASS.
- 485 856 wariantów gestu/kroku/chodu/biegu i 201 600 klatek odwracanego
  wejścia: zero przecięć ostrza, jelca lub rękojeści z powiększonymi bryłami
  głowy i pancerza. Błąd styku z wlotem: maks. 0,000094 jednostki.
- 301 kontroli rzeczywistego renderera i sterowania — PASS; przydział pięciu
  do każdego atrybutu, cofanie, brak nadmiernego wydawania, wiele poziomów,
  brak wydawania w trybie podglądu, zatwierdzanie i zachowane blokady zdolności.
- 18 podpowiedzi w trzech skalach (80/100/120%), 32 ujęcia całej postaci
  w ośmiu kierunkach, 72 rzeczywiście wyrenderowane klatki Synfiga.
- 233 niezależne kontrole zapisanych SIF, dokładnego eksportu, obrazów,
  granic podpowiedzi oraz wcześniejszych testów atrybutów — PASS.

To kontrola geometrii i renderowania; bryły testowe nie zastępują oceny
obrazu. Wizualnie sprawdzono przód/profil bohatera w spoczynku i biegu,
okno przydziału oraz opis many przy dolnej krawędzi HUD-u.

## Końcowa paczka gracza

`Emberfall-Windows-x64.zip` sprawdzono po świeżym rozpakowaniu w
`dist/qa-sheath-stats-package-20261027-175905`. Start rzeczywistego EXE
przeszedł 180 klatek bez trybu deweloperskiego i bez otwierania audio.
Wszystkie 112 zasobów ma zgodne hashe. Ponowiono 301 kontroli renderera
oraz 233 kontrole niezależne — PASS. Preferencje dźwięku pozostały bez zmian.
Paczka nie zawiera natywnych źródeł ani narzędzi testowych.

SHA-256 ZIP: `4e2a356f9563528e1ab622716173462e98d2199695e2dd7bad5c56509440a756`.
SHA-256 EXE: `ca7a7b866eb4bd09888d815d56894b69630fc28a6b411a8c32904a33fb0224b7`.
Pełne wyniki: `QA.json`; lokalne podglądy: `attributes.png`,
`attribute-description.png`, `hero-front.png` i `hero-running.png`.

## Powtarzanie

    pwsh -NoProfile -File tests/sheath_stats_qa.ps1 -NoAudio
    python tools/media/verify_sheath_stats.py --qa <checks> --no-audio
    pwsh -NoProfile -File tests/sheath_stats_package_qa.ps1 -NoAudio -HelperRoot <test>

Dokładne geometryczne odtworzenie: `tests/sword_stow_clearance.cpp` z
`--no-audio`; wyniki tego wydania są w
`dist/qa-sheath-attributes-20261027-v1/clearance.json`.
Native SIF / eksport / rendery: `tools/media/sword_stow_synfig.py`.
GUI i postać: `dist/qa-sheath-stats-20261027-v1/checks`.

Kopia wcześniejszych źródeł i ZIP:
`art/pipeline/backups/sheath-attributes-20261027-174248`.
Źródła korzystają z własnych istniejących materiałów Krity, bez płatnych
usług i bez nowych modeli. Każde uruchomienie/test gry używa `--no-audio`.

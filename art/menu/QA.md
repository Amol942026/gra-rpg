# Menu i zapis wyprawy — końcowy wynik 22.10.2026

**PASS.** Developerskie EXE i ZIP gracza zostały przebudowane po ostatniej
poprawce. Nowy ZIP rozpakowano i sprawdzono niezależnie. Nie odtwarzano
dźwięku, nie zmieniano preferencji audio gracza, niczego nie publikowano.

## Faktycznie sprawdzone

- **87 214** dotychczasowych testów mechaniki i animacji: PASS.
- **1124** kontroli zapisu: dokładne odtworzenie wszystkich pól i obu RNG,
  stan lasu/pustyni po zmianie lokacji, skrzynie, łup, wieża, wszystkie fazy
  obu scenek, pociski, śmierć, polskie ścieżki, trzy sloty, nadpisanie,
  odzyskiwanie `.bak`, obcięte i zmienione pliki, błędne zakresy i wersja,
  zachowanie starego pliku po błędzie: PASS.
- **158** kontroli rzeczywistego kodu interfejsu wersji gracza: PASS.
  Obejmuje start przed prologiem, pięć przycisków głównych, pauzę w świecie
  i dialogach, opcje, twórców, zapis/odczyt, potwierdzenia, powrót do gry,
  punkty oczekujące na zatwierdzenie i zachowanie sesji po błędnym odczycie.
  Enter, Escape i strzałkę sprawdzono także przez callbacki raylib
  i produkcyjną funkcję `input`, bez robotyzacji fizycznej klawiatury.
- **19** renderów w skalach 80/100/120% obejrzano i zweryfikowano natywnie.
  Każdy piksel końcowego menu ma alfa 255; po rysowaniu przywracany jest
  stan mieszania. Wcześniejszą nieprawidłową alfę wykrył niezależny test
  obrazów i poprawiono kompozycję przed tą końcową weryfikacją.
- Niezależny Python/Pillow wykonał **80** sprawdzeń **20** PNG z rozpakowanej
  paczki: komplet, wymiary, dekodowanie, alfa, zawartość i szczegóły obrazu.
  Dwudziesty plik to zrzut faktycznie zaprezentowanego ekranu startowego.
- ZIP: **97** zasobów zgodnych SHA-256 ze źródłami i zgodny plik EXE;
  bez masterów, zapisów gry, trybu developerskiego i programów QA.
- Wyodrębnione EXE wyrenderowało **180** klatek normalnej ścieżki startu:
  `Main menu open: 1`, `Intro active: 0`, `Developer mode: 0`,
  `Audio device opened: no`. Istniejący testowy `audio-settings.cfg`
  zachował identyczny hash również po całym QA menu.

## Wydajność

Po **900 klatek** rzeczywistej rozgrywki, OpenGL z V-sync, natywny bufor
1688×1055, bez dźwięku, na tym komputerze:

| Scena | Średnia FPS | CPU średnio | CPU P95 |
| --- | ---: | ---: | ---: |
| Las | 59,8446 | 10,5653 ms | 13,7773 ms |
| Pustynia | 60,1255 | 6,49341 ms | 9,7484 ms |

Cel pozostaje 60 FPS; nie jest to gwarancja braku pojedynczych dłuższych
klatek. Test lasu miał jedną klatkę ponad 25 ms, pustyni trzy.
W menu nie ma odczytu zapisów ani generowania zasobów w pętli rysowania.

Krótki test startu menu uzyskał CPU P95 **3,9441 ms** i P95 klatki
**17,3426 ms**. Jego surowa średnia **55,3812 FPS** zawiera jeden kosztowny
odczyt GPU i eksport PNG w klatce 119 (maksimum odstępu **192,46 ms**).
Nie przedstawiamy tej średniej jako pomiaru zwykłego renderowania menu.
Właściwe benchmarki rozgrywki nie wykonują zrzutów w trakcie pomiaru.

## Artefakty końcowe

- [Raport paczki](../../dist/qa-menu-package-20261022-134804/package-verification.json).
- [Kontrole menu z paczki](../../dist/qa-menu-package-20261022-134804/Emberfall/qa-menu/checks.txt).
- [Niezależna kontrola obrazów](../../dist/qa-menu-package-20261022-134804/Emberfall/qa-menu/image-verification.json).
- [Test zapisu](../../dist/qa-save-1792669546270405000/report.txt).
- [Benchmarki](../../dist/qa-menu-benchmark-20261022-134558/benchmarks.json).
- [Menu startowe](preview.png), [pauza](pause-preview.png).

ZIP SHA-256:
`7b13a6400aed1ed1a1af1f9bd977dea69fa5e6ca197772238b4484f808f391c9`.

EXE gracza SHA-256:
`76ad3b5df5f49b380d450f30c816955a3c77069e762ab0b1ccd9cd6689dfbd46`.

EXE developerskie SHA-256:
`bffc186ff17b380b8c923897bbd6b57421622187de0a0d47ce7580bbdde32ae0`.

Menu pasuje do Pieczęci Popiołu i Serca Świtu, używając istniejącego
obserwatorium, złota, turkusu i typografii gry. Nowa gra odtwarza prolog;
po wczytaniu należy wybrać **Wróć do gry**. Przed wyjściem zapisz ręcznie
postęp — gra nie ma autozapisu. Sloty są wspólne dla kolejnych kopii gry
w `%LOCALAPPDATA%\Emberfall\saves`. Testy używały osobnych katalogów.

# Twarz bohatera i leśne krzewy — 20.10.2026

## Poprawki

Twarz jest teraz malowana na tych samych wierzchołkach co istniejąca głowa.
Usunięto odsunięty panel twarzy i dodatkowy kształt dolnego policzka. UV kończą
się wewnątrz komórki atlasu, więc krawędzie nie pobierają sąsiedniej grafiki.
Także przód i tył włosów korzystają z powierzchni głowy, zamiast osobnych
odsuniętych paneli. Transparentne pasma zachowują nieprzezroczystą podmalówkę;
porównanie równej głębokości jest przywracane do standardowego po rysowaniu.
Poprawiono również ustawianie trybu trójkątów po zmianie tekstury w rlgl,
aby pierwsza ściana nowej partii nie traciła pojedynczego trójkąta.

Nowy krzew narysowano w Kricie na siedmiu warstwach: łodygi, ściółka,
nieregularna korona, liście, drobne owoce i cień kontaktowy. Korzenie mają
wspólną kotwicę z pozycją świata, sortowaniem, maską kursora i cieniem.
Źródło 360×460 jest wypalane raz w `propArt[2]` i zwalniane po przygotowaniu.
Nie ma generowania grafiki ani odczytów plików w pętli klatki.

Master twarzy ma skorygowaną nieprzezroczystą podmalówkę skóry. Wszystkie
pozostałe komórki arkusza bohatera są identyczne piksel w piksel z poprzednią
wersją. KRA i PNG porównano po dekodowaniu. Oryginalne źródła zachowano;
nie użyto zewnętrznych materiałów, płatnych narzędzi ani usług.

## Weryfikacja

| Kontrola | Wynik |
| --- | --- |
| Obrys malowanej i niemalowanej głowy, 16 kątów wraz z oboma profilami | zero różniących się pikseli alfa |
| Twarz, krzewy, kotwica korzeni, maska kursora i rzeczywista scena | 38/38 PASS |
| Bohater w 8 kierunkach i 420 klatkach siedmiu stanów | 424/424 PASS |
| 39 elementów wyposażenia, portret i sceny fabularne | 364/364 PASS |
| Logika gry i animacje szkieletowe | 75 587 PASS |
| Zachowane krzywe Synfiga i wszystkie klatki źródłowe | 354/354 PASS |
| Zgodność KRA/PNG i niezmienione komórki poza twarzą | PASS |

Wszystkie uruchomienia gry, testów i benchmarku miały jawne `--no-audio`.
Nie zmieniano zapisanych preferencji dźwięku. Dane shaderów, pozy, trafianie
kursorem i rzeczywiste zasoby są testowane w rendererze gracza.

Pliki w `dist/qa-face-bush`:

- `face-closeups.png`, `face-profiles.png`: rzeczywiste profile bohatera.
- `bush-scales.png`, `bush-forest.png`: krzew w różnych skalach i w świecie gry.
- `tests.txt`, `packaged-tests.txt`: kontrole lokalne i po wypakowaniu ZIP-a.
- `asset-validation.json`: porównanie natywnych masterów i eksportów.
- `package-verification.json`: SHA-256 ZIP-a, porównanie wszystkich zasobów,
  180 klatek uruchomienia EXE gracza oraz ponowne 424+38 kontroli GPU.
- `benchmark-final.txt`: aktualny końcowy pomiar 900 klatek w lesie.

Końcowy benchmark w oknie 1440×900, OpenGL V-sync 60 Hz: **60,05 fps**,
średni czas CPU **6,96 ms**, zero klatek dłuższych niż 25 ms. Wykonano go
po zakończeniu produkcji zasobów i kompilacji, bez równoległego renderowania.

Poprzednie źródła, grafiki, EXE i ZIP:
`dist/qa-face-bush/backup-20261020-161746`. Poprzedni benchmark tej iteracji
`benchmark.txt` dotyczy wersji sprzed ostatecznego dopasowania włosów;
miarodajny dla paczki jest `benchmark-final.txt`.

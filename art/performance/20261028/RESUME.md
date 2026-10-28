# Stan końcowy po wznowieniu — 28.10.2026

Aktualizacja: wdrożono i sprawdzono dalszą rundę optymalizacji.
Aktualny raport i hashe ZIP-a: `round2/README.md` i
`round2/package-verification.json`. Ten opis dotyczy poprzedniego wariantu.

Wdrożono i zweryfikowano optymalizacje opisane w README.md w tym folderze.
Wszystkie procesy testowe i kompilacje zakończone. Nie ma zadania w tle.

Aktualny raport: art/performance/20261028/README.md.
Końcowy ZIP: Emberfall-Windows-x64.zip, SHA-256
3357a8acc7b51e3ffe03d2ac9e01ce097c28d173e890b0df6daf9d22bc3392ad.
Weryfikacja wypakowania: package-verification.json — PASS; 115 zasobów,
180 klatek ukrytego startu, 810 kontroli renderera i pomiary sześciu scen.
Logika: 158661 kontroli — PASS; zapis/odczyt: 3403 — PASS.

Użytkownik wymaga teraz wszystkich testów w zminimalizowanych/ukrytych
oknach oraz bez dźwięku. Dodano --background-test wymagający --no-audio,
przetestowany w rzeczywistym player EXE. Nie aktywować okien testowych.
AGENTS.md zawiera tę preferencję. Normalny start użytkownika ma dźwięk.

Pustynia we wcześniejszym teście widocznej gry: 46,17 → 58,45–59,91 FPS.
Ostatnia poprawka hoistuje wspólne obliczenia wiatru drzew i cieni,
z dokładnie tymi samymi pikselami; nie dodano odrzucania maski przez głębię.
Las i pustynia przy dużym ekranie używają bufora świata 945 × 810, natywny
HUD/napisy; --native-world przywraca pełną rozdzielczość. Domyślnie DXGI.

Ostateczne testy ukryte sprawdzają ukończoną pracę CPU/GPU, nie FPS
wyświetlania. Ich p95 mieści się w budżecie renderowania 16,67 ms, lecz
nie potwierdzono bezwarunkowych stałych 60 FPS wszystkich lokacji końcowego
builda na ekranie. Nie przedstawiać czasu ukrytego renderu jako pomiaru FPS.
Dane wcześniejszego problemu lasu i wszystkie ograniczenia są w raporcie.

Obie kompilacje oraz aktualny ZIP są zapisane. Nie wykonywano commita,
pusha, publikacji, zmian audio ani ustawień Windows. Wcześniejsze poprawki
schodów i pozostałe zastane zmiany checkoutu pozostawiono.

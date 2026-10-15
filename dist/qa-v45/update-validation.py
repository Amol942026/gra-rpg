from pathlib import Path
import json
root=Path(__file__).resolve().parents[2]
qa=root/'dist/qa-v45'
e=json.loads((qa/'package-verification.json').read_text(encoding='utf-8'))
f=e['benchmarks']['forest']; d=e['benchmarks']['desert']
section=f"""## Wersja 0.45 — stabilny obraz przy biegu i uniku

Sprawdzone 14 października 2026 w Dokumenty/GitHub/gra-rpg. Wszystkie uruchomienia gry, testy i pomiary miały --no-audio. Preferencje audio zachowano bajt w bajt.

- Świat i HUD powstają bezpośrednio w końcowym buforze o natywnej rozdzielczości. Usunięto pośrednią teksturę świata, jej powiększanie i późniejsze zmniejszanie filtrem biliniowym. Kamera i punkty obiektów korzystają ze wspólnej siatki rzeczywistych pikseli. Maska cieni jest wyrównana do tej samej fazy kamery, z kompensacją przesunięcia przy jej połowie rozdzielczości. Tempo ruchu i animacji pozostaje liczone w pełnej precyzji.
- Przezroczyste elementy świata są składane poprawnym source-over dla RGB i kanału alfa. Gotowy nieprzezroczysty świat nie jest powtórnie przyciemniany. Nie ma dodatkowego przebiegu pełnoekranowego. Domyślna prezentacja to OpenGL z V-sync; parametr --dxgi zachowuje alternatywną prezentację DirectX.
- Porównano rzeczywiste piksele architektury dla 48 pozycji kamery i skal 0,8 / 1,0 / 1,2: ponad 1,12 mln próbek w każdej kompilacji. Przed zmianą do około 30% pikseli zmieniało się o więcej niż 2 poziomy RGB podczas przesunięcia o jeden piksel. Po zmianie takich różnic jest 0. Natywny obraz jest przesuwany piksel w piksel. 10/10 kontroli świata w projekcie i kolejne 10/10 w kompilacji gracza. 658/658 kontroli napisów w każdej kompilacji, 82/82 kursora, 129/129 pełnego ekranu i prezentacji oraz 490/490 cieni.
- Rzeczywisty bieg i przeturlanie: 11/11 kontroli; {e['run_roll_fps']:.2f} FPS, średnia {e['run_roll_mean_ms']:.3f} ms, p95 {e['run_roll_p95_ms']:.3f} ms, monitor 60 Hz, OpenGL V-sync. Osobne scenariusze po 900 klatek w pełnym ekranie 1920×1080: las {f['fps']:.2f} FPS, p95 {f['p95_ms']:.3f} ms, p99 {f['p99_ms']:.3f} ms; pustynia {d['fps']:.2f} FPS, p95 {d['p95_ms']:.3f} ms, p99 {d['p99_ms']:.3f} ms. Są pojedyncze klatki powyżej 25 ms ({int(f['frames_over_25_ms'])} w lesie i {int(d['frames_over_25_ms'])} na pustyni); pomiar nie oznacza stałych 60 FPS na każdym sprzęcie.
- Usunięto wyłącznie ciemny prostokąt pod obrażeniami krytycznymi. Napis KRYTYK!, złota liczba, ich rozmiary, pozycja, obrys i animacja pozostają identyczne. 69/69 istniejących kontroli walki i leczenia; obejrzano obraz rzeczywistych trafień bez tabliczki.
- 6308/6308 testów mechaniki i animacji. Kompilacja gracza: 17/17 kontroli interfejsu i braku DEV, 440/440 animacji miecza i Mocnego cięcia oraz 147/147 handlu.
- ZIP 0.45 dla gracza: {e['package_bytes']} bajtów ({e['package_bytes']/1000000:.2f} MB), {e['zip_entries']} wpisów. Wszystkie {e['verified_payload_files']} pliki gry, zasobów i dokumentów zgodne po SHA-256. Rozpakowana gra wykonała 180 klatek prologu w pełnym ekranie, bez DEV i otwartego urządzenia audio. Parametry --dev, --start=desert-boss, --labels-qa i --world-motion-qa nie uruchamiają sesji testowych w paczce. Lokalny developerski EXE i zapisane ustawienia audio nie zostały zmienione przez pakowanie. Tymczasową rozpakowaną kopię usunięto po zachowaniu wyników.
- SHA-256 ZIP: {e['zip_sha256']}. EXE gracza: {e['player_exe_sha256']}. Preferencje audio: {e['audio_preferences_sha256']}.

Dowody: dist/qa-v45/package-verification.json, baseline-pixels.txt, world-pixels.txt, player-world-pixels.txt, motion-tests.txt, forest-benchmark.txt, desert-benchmark.txt, labels-tests.txt, player-labels-tests.txt, pointer-tests.txt, refresh-tests.txt, shadow-tests.txt, combat-tests.txt, player-build-output.txt, player-build-tests.txt, player-sword-tests.txt, player-trade-tests.txt i packaged-startup.txt. Obraz trafień: critical-without-background.png. Raporty porównania świata wskazują osobne katalogi natywnych klatek każdego uruchomienia.

"""
p=root/'VALIDATION.md';s=p.read_text(encoding='utf-8')
assert s.startswith('# Weryfikacja prototypu\n\n')
assert '## Wersja 0.45' not in s
s=s.replace('# Weryfikacja prototypu\n\n','# Weryfikacja prototypu\n\n'+section,1)
p.write_text(s,encoding='utf-8')
(root/'dist/VALIDATION.md').write_text(s,encoding='utf-8')
print('Zapisano końcową weryfikację wersji 0.45.')

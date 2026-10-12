$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
Add-Type -AssemblyName System.IO.Compression,System.IO.Compression.FileSystem
$packagePath = Join-Path $PSScriptRoot 'Emberfall-Windows-x64.zip'
$temporaryPackage = Join-Path $PSScriptRoot ('Emberfall-package-' + [guid]::NewGuid().ToString('N') + '.zip')
$versionManifest = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'resources/Emberfall.manifest'))
$releaseVersion = [regex]::Match($versionManifest, 'assemblyIdentity version="([0-9]+\.[0-9]+)').Groups[1].Value
$archive = [IO.Compression.ZipFile]::Open($temporaryPackage, [IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($entry in @(@('dist/Emberfall.exe','Emberfall.exe'), @('LICENSES.md','LICENSES.md'), @('LOOT.md','LOOT.md'))) {
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, (Join-Path $PSScriptRoot $entry[0]), ('Emberfall/' + $entry[1]), [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
    $assetDirectory = Join-Path $PSScriptRoot 'dist/assets'
    foreach ($assetFile in Get-ChildItem -LiteralPath $assetDirectory -File -Recurse) {
        $relativeAsset = $assetFile.FullName.Substring($assetDirectory.Length + 1).Replace('\','/')
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, $assetFile.FullName, ('Emberfall/assets/' + $relativeAsset), [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
    $instructions = @"
EMBERFALL - Echa zapomnianych ruin, wersja $releaseVersion
Gra dla Windows x64. Nie wymaga instalacji ani dodatkowych narzedzi.

URUCHOMIENIE
1. Kliknij ZIP prawym przyciskiem i wybierz Wyodrebnij wszystkie.
2. Otworz wypakowany folder Emberfall.
3. Uruchom Uruchom.bat lub Emberfall.exe. Gra rozpocznie sie prologiem.
Folder assets musi pozostac obok pliku Emberfall.exe.
Gra startuje domyslnie na pelnym ekranie. F11 przywraca tryb okna.
Gra sprawdza przesylanie obrazu GPU; przy bledzie sama wybiera OpenGL.
W razie problemu etapy startu znajdziesz w emberfall-startup.log obok EXE.
Uruchom.bat --opengl wymusza OpenGL; --windowed uruchamia gre w oknie.
Synchronizacja z monitorem oraz wygladzanie terenu poprawiaja plynnosc ruchu.
Poczatkowy sprzet zostaje w plecaku; nie ma rozrzuconych startowych przedmiotow.
Wersja 0.24: mag Eldrin w fioletowych szatach opowiada o Sercu Switu.
Artefakt ma piec czesci, kazdej strzeze osobny straznik. Po slowach
"Ruszaj wiec" Eldrin teleportuje bohatera do Lasu Mchu.
Przybycie otacza bohatera niebieskim slupem magii z pierscieniami i iskrami.
Eldrin ma szczegolowe szaty, hafty, brode i ozdobna laske;
obserwatorium zdobia kamienny luk, okragly podest, pnacza i swiece.
Wedrowiec ma oble ksztalty, dokladniejsza twarz i zbroje oraz falujacy plaszcz.
Nowy wyglad dziala w lesie, prologu i portrecie, przy chodzie, walce i unikach.
Dialog przewija sie automatycznie. Enter, Spacja lub LPM pokazuja tekst
i przechodza dalej; ESC pomija rozmowe, zachowujac animacje teleportacji.
F11 zmienia tryb ekranu. W czasie prologu swiat i walka sa zatrzymane.
Wersja 0.25: las ma 72 x 72 pola, 2,25 raza wiekszy obszar niz poprzednio.
46 przeciwnikow jest rozlozonych w odstepach co najmniej 8 pol.
Przesieka po teleportacji jest spokojna; straznik nadal czeka w starych ruinach.
Nowe trasy prowadza do Zacisznego zagajnika, Starego traktu i kamiennego kregu.
W kazdej wyprawie rosna 64 borowiki w losowych miejscach wiekszego lasu.
Wersja 0.26: Kamienny Straznik zostawia jedna czesc Serca Switu (100%).
Otworz cialo PPM, zbierz pozostale nagrody, potem przeciagnij czesc do plecaka.
Zebranie czesci uruchamia sciemnienie i powrot do wiezy Eldrina.
Bohater zdaje relacje, mag go chwali i zapowiada wyprawe do drugiej krainy.
Po scence mozna chodzic po wiezy. Podejdz do Eldrina: E, LPM lub PPM - rozmowa.
Dziennik pokazuje 1/5 czesci; poziom, statystyki i ekwipunek sa zachowane.
Artefakt zostaje w plecaku; nie mozna go porzucic ani zuzyc.
Druga kraina jest na razie zapowiedziana w dialogu; wyprawa konczy sie w wiezy.
Podejdz na 2,5 pola i przeciagnij grzyb z mapy lub panelu W ZASIEGU do plecaka.
PPM na borowiku w plecaku przywraca 10 HP (do maksimum); stos do 99 sztuk.
Przy pelnym HP grzyb nie znika. Pelny plecak pozostawia go w lesie.
Las ma 72 x 72 pola, nowe sciezki, brzozy, deby i grzyby. Na mapie jest
46 przeciwnikow z 8 gatunkow, w tym dziki, pajaki, rozbojnicy i ozywione drzewa.
Kazdy gatunek ma wlasny loot: m.in. kiel wilka 75%, siersc wilka 55%.
Losowania sa niezalezne; dokladne szanse i ilosci znajdziesz w LOOT.md.
Materialy lacza sie w stosy do 99 sztuk. Nowy sprzet: sztylet, plaszcz, amulet.
Wersja 0.21 ma bardziej szczegolowe postacie, potwory, drzewa, teren i ikony.
Nowe ksztalty i tekstury zachowuja osiem kierunkow animacji i wiatr.
Potwory patroluja okolice respawnu, gdy nie widza bohatera. Wykrycie go
rozpoczyna poscig; po utracie widoku wracaja do domu. Przeszkody zaslaniaja widok.

Wersja 0.23: Q lub przycisk BRON - kolo broni; startowy miecz i luk.
LPM na sektorze wybiera bron. Przeciagnij bron z plecaka do sektora,
aby dodac skrot. PPM usuwa tylko skrot. Q/ESC zamyka kolo; gra w menu stoi.
Luk: przytrzymaj LPM i pusc, aby wystrzelic. Na start 20 strzal w plecaku.
Naciag do 1,4 s daje zasieg 2-12 pol i obrazenia 22-34 (miecz: 14).
Kazdy strzal zuzywa jedna strzale. Menu i unik anuluja naciag bez kosztu.
Celownik pokazuje kierunek, naciag i zasieg; kolczan jest na plecach.
Strzala trafia pierwszy cel; drzewa i inne przeszkody ja zatrzymuja.

Wersja 0.27: po awansie rozdaj 5 punktow statystyk, potem 1 punkt umiejetnosci.
Gra pozostaje zatrzymana do zatwierdzenia obu etapow. U otwiera podglad rang.
Z - Mocne ciecie: tylko miecz, 10 many, 150% obrazen na randze 1.
X - przygotuj Przeszywajaca strzale: tylko luk; potem naciagnij i pusc LPM.
Strzal kosztuje 20 many i jedna strzale; przebija do 3 wrogow na randze 1.
Zadaje takie same obrazenia jak zwykla strzala przy tym samym naciagu.
Kolejne punkty wzmacniaja ciecie lub zwiekszaja liczbe przeszywanych celow.
Umiejetnosci maja osobne animacje, przyciski HUD i blokady niewlasciwej broni.

Wersja 0.28: wieza Eldrina ma 5 pieter polaczonych schodami.
Podejdz do schodow lub wyposazenia i uzyj E / LPM / PPM.
Pietro 1 - sala przybycia, Eldrin, kominek i zrodlo odnawiajace cale HP/mane.
Pietro 2 - biblioteka z regalami i trzema rozdzialami kronik.
Pietro 3 - alchemia: 3 borowiki -> mikstura zdrowia (+50 HP),
3 esencje lasu -> mikstura many (+35 many). Brak miejsca nie zuzywa skladnikow.
Pietro 4 - trzy runiczne kamienie, promienie i odbicie Serca Switu.
Pietro 5 - obserwatorium z luneta, astrolabium i trzema mapami nieba.
Przedmioty pozostawione na podlodze zostaja na swoim pietrze.
Okna ksiegi, alchemii i lunety zamkniesz ESC / ENTER / przyciskiem X.

Wersja 0.29 - TRYB DEWELOPERSKI
F3 lub przycisk DEV otwiera wybor lokacji, takze w prologu i po smierci.
Tryb_deweloperski.bat od razu otwiera ten wybor.
Test_wiezy.bat zaczyna na pierwszym pietrze; Test_bossa.bat przy Strazniku.
Wybierz obszar lasu lub jedno z 5 pieter wiezy.
NOWA SESJA TESTOWA resetuje wyprawe i przeciwnikow; PRZENIES OBECNA POSTAC
zachowuje plecak, poziom i rangi. Start odnawia cale HP i mane.
Opcjonalny zestaw: wszystkie trzy umiejetnosci, 100 strzal, po 15 borowikow/esencji
oraz po 10 mikstur zdrowia/many. Nie niszczy przedmiotow przy pelnym plecaku.
ESC / F3 / X zamyka wybor; ENTER zaczyna nowa sesje w wybranej lokacji.
Po smierci ENTER rozpoczyna ponownie ten sam test. Sesje oznacza napis SESJA TESTOWA.
NOWA ZWYKLA GRA - PROLOG resetuje wyprawe i wraca do normalnej rozgrywki.
Mozesz tez uzyc Uruchom.bat --start tower-3 (alchemia) lub --start ruins (boss).
Identyfikatory: forest, ruins, birches, grove, heart, trail, wolves, spiders,
tower-1, tower-2, tower-3, tower-4, tower-5. --no-dev-kit wylacza zestaw testowy.
Zwykly start bez parametrow nadal rozpoczyna sie prologiem.

Wersja 0.30: kazdy atak bohatera ma 5% szansy na krytyk:
miecz, luk, Kula ognia, Mocne ciecie i Przeszywajaca strzala.
Krytyk zadaje 150% wiecej (2,5 raza zwykle obrazenia po uwzglednieniu obrony).
Jeden zamach/pocisk = jedno losowanie, wspolne dla wszystkich trafionych celow.
Nad wrogiem widac duze zlote cyfry z napisem KRYTYK! i ciemnym tlem.
V / zielony kafelek LECZENIE: 20 many -> +30 HP, do maksimum. Odnowienie 1 s.
Poznaj Leczenie za 1 punkt w menu po awansie (najpierw rozdaj statystyki).
Leczenie ma jedna range, dziala z kazda bronia i w wiezy.
Pelne HP nie zuzywa many. Zielone pierscienie i krzyzyki pokazuja efekt.

Wersja 0.31: pelnowymiarowe kamienne schody w wiezy.
W gore: 14 stopni, spocznik, murowane boki, slupki i kamienne porecze.
W dol: otwor w posadzce, stopnie znikajace w glebi i balustrada z wolnym wejsciem.
Fugi, podstopnice, kamienne krawedzie i cienie sa dopasowane do skali bohatera.
Najazd myszka wskazuje rzeczywista sylwetke. Kolizje chronia przed wejsciem w szyb.
Podejdz do wejscia i uzyj E / LPM / PPM, aby przejsc na sasiednie pietro.

Wersja 0.32: szczegolowe wyposazenie wszystkich pieciu pieter wiezy.
Rzezbione kolumny i postumenty, kominek z kamiennym lukiem, drewniane meble,
kolorowe ksiegi ze zloconymi grzbietami, otwarte strony i wstazki.
Alchemia: fiolki, mozdzierze, kociol z uchwytami i nitami, skrzynie ziol.
Luneta: trojnog, segmenty tubusu i soczewka; astrolabium: metalowe kregi.
Okna maja kamienne obramowania, luk, parapet i rombowe szyby.
Ogien, woda, para, ziola i magia sa animowane. Mysz wskazuje rzeczywisty
ksztalt obiektu, dodaje zloty obrys i informacje w HUD. E / LPM / PPM
nadal otwieraja ksiegi, alchemie, mapy nieba i uruchamiaja pozostale interakcje.
Grafika jest przygotowywana raz na starcie, w podwojnej rozdzielczosci.

Wersja 0.33: MORZE PIASKOW - DRUGA KRAINA
Po Kamiennym Strazniku zbierz pierwszy fragment do plecaka i wroc do wiezy.
Podejdz do Eldrina na pierwszym pietrze: E / LPM / PPM - porozmawiaj.
Po rozmowie kliknij odpowiedz "Przenies mnie na pustynie.".
Mozesz zostac w wiezy; czekanie, Enter i Spacja w wyborze nie teleportuja.
Na pustyni: kaktusy, opuncje, piaskowce, ruiny, palmy i Oaza Bursztynu.
27 potworow: skorpiony, skarabeusze i kobry; wlasne animacje i ciala z lupem.
Nowe trofea: zadlo skorpiona 75%, pancerz skarabeusza 85%, luska kobry 80%.
Drugi boss - Zywa Wydma - czeka na poludniowym wschodzie; 560 HP.
Jej piaskowe cialo zawiera mocniejszy miecz, esencje piasku, mikstury,
monety i druga czesc Serca Switu ze szansa 100%. Szczegoly w LOOT.md.
Otworz cialo PPM i przeciagnij nagrody do plecaka; fragment zbierz na koncu.
Fragment uruchamia powrot do Eldrina, nowy dialog i postep 2 / 5.
Brama Eldrina w miejscu przybycia: E / LPM / PPM - powrot do wiezy.
Krainy zachowuja ciala, pozostawiony lup i stan pokonanych bossow.
Poziom, statystyki, umiejetnosci, ekwipunek, zdrowie i mana sa zachowane.
F3 ma teraz 16 lokacji. Test_pustyni.bat zaczyna od razu w drugiej krainie.
Uruchom.bat --start desert-oasis lub --start desert-boss - test oazy / bossa.
Zwykly start bez parametrow nadal rozpoczyna sie prologiem.

STEROWANIE
WSAD - chodzenie; kombinacje klawiszy pozwalaja isc ukosnie.
Przytrzymany Shift + WSAD - bieg o 50% szybszy od chodu.
Spacja + WSAD - przeturlanie; sama Spacja - unik w kierunku ustawienia.
LPM z mieczem - zamach na klikniecie; trafia wszystkich w promieniu 2,1 pola.
LPM z lukiem - przytrzymaj, by naciagac; puszczenie wystrzeliwuje strzale.
Przytrzymanie LPM nie powtarza ataku. Nie ma wybierania celu kliknieciem.
Potwory odpychaja sie i odbijaja od siebie przy zderzeniu.
Najazd na potwora lub obiekt - informacje w prawym panelu POD KURSOREM.
C - wybrany czar w strone kursora (Kula ognia: 10 many).
PPM na ciele potwora - otwarcie jego lupu w prawym HUD.
LPM i przeciagniecie z okna ciala do plecaka - zebranie przedmiotu.
Loot z zabojstwa zostaje w ciele, ktore pozostaje na mapie.
Otwieranie i zbieranie: maks. 2,5 pola. X w okienku zamyka loot. 1 / 2 - mikstura zdrowia / many.
K lub klikniecie kafelka czaru - ksiega i wybor czaru. P - statystyki bohatera.
Po awansie swiat pauzuje: rozdaj 5 punktow przyciskami + / - i zatwierdz.
Start: 100 many. Regeneracja: 1 punkt co 5 sekund. Wytrzymalosc +10 HP,
statystyka Mana +5 maksymalnej many za punkt.
LPM i przeciagniecie - przenoszenie lupu do plecaka lub na ekwipunek.
PPM na miksturze w plecaku lub panelu przedmiotow na ziemi - wypicie. F1 - pomoc. Esc - pauza.
F2 - glosnosc muzyki i efektow. M - wyciszenie / wlaczenie dzwieku.
F11 lub PELNY EKRAN u gory HUD - pelny ekran. Ponowne F11 / OKNO przywraca okno.
Obraz dopasowuje sie do rozdzielczosci i skali Windows bez ucinania HUD.
Drzewa kolysza sie na wietrze, a po wodzie przesuwaja sie animowane fale.
MUZYKA / EFEKTY w prawym dolnym rogu HUD - osobne wlaczenie / wylaczenie.
Przyciski sa stale widoczne, takze podczas pauzy. Zachowuja glosnosc suwakow.
Enter po smierci - nowa wyprawa.

Gra nie zapisuje postepu wyprawy. Ustawienia dzwieku zapisuja sie przy zamykaniu.
Licencje: LICENSES.md i pliki w assets.
"@
    $entry = $archive.CreateEntry('Emberfall/JAK_URUCHOMIC.txt')
    $writer = New-Object IO.StreamWriter($entry.Open(), (New-Object Text.UTF8Encoding($true)))
    try { $writer.Write($instructions) } finally { $writer.Dispose() }
    foreach ($launcher in @('Tryb_deweloperski.bat','Test_wiezy.bat','Test_bossa.bat','Test_pustyni.bat')) {
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, (Join-Path $PSScriptRoot ('dist/' + $launcher)), ('Emberfall/' + $launcher), [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
    $entry = $archive.CreateEntry('Emberfall/Uruchom.bat')
    $writer = New-Object IO.StreamWriter($entry.Open(), [Text.Encoding]::ASCII)
    try { $writer.Write("@echo off`r`ncd /d `"%~dp0`"`r`nstart `"Emberfall`" `"Emberfall.exe`" %*`r`n") } finally { $writer.Dispose() }
} finally { $archive.Dispose() }
Move-Item -LiteralPath $temporaryPackage -Destination $packagePath -Force
Write-Output ('Gotowe: {0} ({1:N2} MB)' -f $packagePath, ((Get-Item -LiteralPath $packagePath).Length / 1000000))

$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
& (Join-Path $PSScriptRoot 'build.ps1') -Player
Add-Type -AssemblyName System.IO.Compression,System.IO.Compression.FileSystem
$packagePath = Join-Path $PSScriptRoot 'Emberfall-Windows-x64.zip'
$temporaryPackage = Join-Path $PSScriptRoot ('Emberfall-package-' + [guid]::NewGuid().ToString('N') + '.zip')
$versionManifest = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'resources/Emberfall.manifest'))
$releaseVersion = [regex]::Match($versionManifest, 'assemblyIdentity version="([0-9]+\.[0-9]+)').Groups[1].Value
$archive = [IO.Compression.ZipFile]::Open($temporaryPackage, [IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($entry in @(@('dist/player/Emberfall.exe','Emberfall.exe'), @('LICENSES.md','LICENSES.md'), @('LOOT.md','LOOT.md'))) {
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, (Join-Path $PSScriptRoot $entry[0]), ('Emberfall/' + $entry[1]), [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
    $assetDirectory = Join-Path $PSScriptRoot 'dist/player/assets'
    foreach ($assetFile in Get-ChildItem -LiteralPath $assetDirectory -File -Recurse) {
        $relativeAsset = $assetFile.FullName.Substring($assetDirectory.Length + 1).Replace('\','/')
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, $assetFile.FullName, ('Emberfall/assets/' + $relativeAsset), [IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
    $instructions = @"
EMBERFALL - Echa zapomnianych ruin, wersja $releaseVersion
WERSJA DLA GRACZA / Windows x64

URUCHOMIENIE
1. Kliknij ZIP prawym przyciskiem i wybierz Wyodrebnij wszystkie.
2. Otworz wypakowany folder Emberfall.
3. Uruchom Uruchom.bat lub Emberfall.exe.
Nie trzeba instalowac silnika ani dodatkowych narzedzi.
Folder assets musi pozostac obok Emberfall.exe.
Gra zaczyna sie prologiem i wlacza domyslnie pelny ekran.
F11 przelacza miedzy pelnym ekranem a oknem.
Domyslnie: natywne OpenGL z V-sync i ostry obraz bez powtornego skalowania.
Opcjonalnie: Uruchom.bat --windowed lub Uruchom.bat --dxgi.
Etapy uruchamiania sa zapisywane w emberfall-startup.log obok EXE.

STEROWANIE
W S A D - ruch, rowniez ukosnie przy kombinacji klawiszy.
Przytrzymany Shift - bieg. Spacja - przeturlanie w kierunku ruchu.
W biegu miecz jest w pochwie przy biodrze, rece pracuja naprzemiennie.
LPM z mieczem - jeden zamach na klikniecie; trafia wrogow w zasiegu.
Miecz: przygotowanie, ciecie przed bohaterem i powrot do pozycji.
Ruch zachowuje dlugosc ostrza i ramienia; tulow i peleryna podazaja za ciosem.
LPM z lukiem - przytrzymaj, by naciagac cieciwe, i pusc, by strzelic.
Dlugosc naciagu zmienia zasieg. Na poczatek masz 20 strzal.
Q / BRON - kolo wyboru broni. Przeciagnij bron z plecaka na kolo.
PPM na ciele bestii - otworz jego loot. Przeciagnij lup do plecaka.
Przeciagnij przedmiot z plecaka na pasujacy slot ekwipunku.
Najazd na przedmiot - opis i rzadkosc; na obiekt lub bestie - informacje.
1 / 2 - mikstura zdrowia / many.
PPM na miksturze lub grzybie w plecaku - uzyj. Grzyb odnawia 10 HP.
C - rzuc wybrany czar w kierunku kursora. K - wybor czaru.
Kula ognia kosztuje 10 many. Mana odnawia sie o 1 punkt co 5 sekund.
Z - Mocne ciecie (miecz, 10 many).
X - Przeszywajaca strzala (luk, 20 many).
V - Leczenie (30 HP za 20 many).
U - umiejetnosci. P - statystyki.
Po awansie rozdaj 5 punktow statystyk i 1 punkt umiejetnosci.
Umiejetnosci odblokowuje dopiero wydanie i zatwierdzenie punktu po awansie.
E - interakcja w wiezy lub powrot przez portal na pustyni.
Miron - handlarz na 3. pietrze wiezy (pracownia alchemiczna).
Podejdz do stoiska i nacisnij E lub kliknij LPM / PPM.
Kliknij towar po lewej, aby kupic, lub przedmiot po prawej, aby sprzedac.
Ustaw liczbe sztuk przez - / + lub MAX i potwierdz KUP / SPRZEDAJ.
Sprzedasz tez noszone eq i materialy bestii, np. luski kobry.
Wartosc w opisie oznacza cene sprzedazy sztuki; stos ma rowniez sume.
Cena kupna jest dwukrotnie wyzsza. Monety w plecaku sluza do placenia.
Fragmentow Serca Switu nie mozna sprzedac. Saldo zlota widac w HUD.
Brak miejsca lub zlota nie powoduje utraty przedmiotow ani pieniedzy.
F1 - pomoc. Esc - pauza. Enter po smierci - nowa wyprawa.
F2 - glosnosc. MUZYKA / EFEKTY w HUD - osobne wyciszenie.
F11 / przycisk u gory ekranu - pelny ekran lub okno.
W rozmowach Enter, Spacja lub LPM przewija tekst. Esc pomija prolog.

WYPRAWA I LUP
Odkryj las, pokonaj Kamiennego Straznika i dotrzyj do wiezy Eldrina.
Wieza ma piec tematycznych pieter. Eldrin otwiera droge na pustynie,
gdzie czeka Zywa Wydma. Rozmowa z magiem pozwala wybrac podroz.
Pospolity: bialy / 60%; Rzadki: niebieski / 15%.
Bardzo rzadki: zolty / 4%; Niezwykle rzadki: fioletowy / 1%.
Kazda pozycja lupu losuje sie niezaleznie, wedlug swojej rzadkosci.
Fragment Serca Switu jest gwarantowana nagroda fabularna.
Lup zostaje w cialach. Puste cialo moze nie zawierac przedmiotow.
PPM otwiera cialo w zasiegu 2,5 pola. Strony lupu: < / > lub rolka.
Po zwyciestwie mozesz znalezc bronie, zbroje, helmy i materialy bestii.

Gra nie zapisuje postepu wyprawy. Glosnosc zapisuje sie przy zamykaniu.
Licencje: LICENSES.md i pliki w assets. Tabele lupu: LOOT.md.
"@
    $entry = $archive.CreateEntry('Emberfall/JAK_URUCHOMIC.txt')
    $writer = New-Object IO.StreamWriter($entry.Open(), (New-Object Text.UTF8Encoding($true)))
    try { $writer.Write($instructions) } finally { $writer.Dispose() }
    $entry = $archive.CreateEntry('Emberfall/Uruchom.bat')
    $writer = New-Object IO.StreamWriter($entry.Open(), [Text.Encoding]::ASCII)
    try { $writer.Write("@echo off`r`ncd /d `"%~dp0`"`r`nstart `"Emberfall`" `"Emberfall.exe`" %*`r`n") } finally { $writer.Dispose() }
} finally { $archive.Dispose() }
Move-Item -LiteralPath $temporaryPackage -Destination $packagePath -Force
Write-Output ('Gotowe: {0} ({1:N2} MB)' -f $packagePath, ((Get-Item -LiteralPath $packagePath).Length / 1000000))

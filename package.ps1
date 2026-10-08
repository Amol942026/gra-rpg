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
3. Uruchom Uruchom.bat lub Emberfall.exe, potem nacisnij Enter.
Folder assets musi pozostac obok pliku Emberfall.exe.
Gra startuje domyslnie na pelnym ekranie. F11 przywraca tryb okna.
Synchronizacja z monitorem oraz wygladzanie terenu poprawiaja plynnosc ruchu.
Nowa gra nie ma przedmiotow na ziemi. Poczatkowy sprzet zostaje w plecaku.
Las ma 48 x 48 pol, nowe sciezki, brzozy, deby i grzyby. Na mapie jest
46 przeciwnikow z 8 gatunkow, w tym dziki, pajaki, rozbojnicy i ozywione drzewa.
Kazdy gatunek ma wlasny loot: m.in. kiel wilka 75%, siersc wilka 55%.
Losowania sa niezalezne; dokladne szanse i ilosci znajdziesz w LOOT.md.
Materialy lacza sie w stosy do 99 sztuk. Nowy sprzet: sztylet, plaszcz, amulet.
Potwory patroluja okolice respawnu, gdy nie widza bohatera. Wykrycie go
rozpoczyna poscig; po utracie widoku wracaja do domu. Przeszkody zaslaniaja widok.

STEROWANIE
WSAD - chodzenie; kombinacje klawiszy pozwalaja isc ukosnie.
Przytrzymany Shift + WSAD - bieg o 50% szybszy od chodu.
Spacja + WSAD - przeturlanie; sama Spacja - unik w kierunku ustawienia.
LPM na mapie - jeden zamach na klikniecie; trafia wszystkich w promieniu 2,1 pola.
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
    $entry = $archive.CreateEntry('Emberfall/Uruchom.bat')
    $writer = New-Object IO.StreamWriter($entry.Open(), [Text.Encoding]::ASCII)
    try { $writer.Write("@echo off`r`ncd /d `"%~dp0`"`r`nstart `"Emberfall`" `"Emberfall.exe`"`r`n") } finally { $writer.Dispose() }
} finally { $archive.Dispose() }
Move-Item -LiteralPath $temporaryPackage -Destination $packagePath -Force
Write-Output ('Gotowe: {0} ({1:N2} MB)' -f $packagePath, ((Get-Item -LiteralPath $packagePath).Length / 1000000))

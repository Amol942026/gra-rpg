param([switch]$NoBuild)
$ErrorActionPreference='Stop'
Set-Location -LiteralPath $PSScriptRoot
if(!$NoBuild){& (Join-Path $PSScriptRoot 'build.ps1')}
$stage=Join-Path $PSScriptRoot ('dist/qa-volcano-dev-stage-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss-fff'))
$folder=Join-Path $stage 'Emberfall-DEV'
New-Item -ItemType Directory -Path $folder | Out-Null
Copy-Item -LiteralPath 'dist/Emberfall.exe' -Destination (Join-Path $folder 'Emberfall.exe')
Copy-Item -LiteralPath 'dist/assets' -Destination (Join-Path $folder 'assets') -Recurse
foreach($file in @('Uruchom.bat','LICENSES.md','LOOT.md')){
 Copy-Item -LiteralPath $file -Destination (Join-Path $folder $file)
}
$instructions=@'
EMBERFALL — WERSJA DEWELOPERSKA

Rozpakuj caly folder Emberfall-DEV i uruchom Uruchom.bat.
W menu glownym wybierz Tryb deweloperski albo nacisnij F3.
Wybierz zakladke krainy i lokacje. W zakladce Kaldera Popiolu znajdziesz m.in.:
- Brama Popiolu: poczatek trzeciej krainy.
- Obsydianowe Kuznie: wschodni szlak kaldery.
- Tron Serca Kaldery: bezposrednio przy Varkhorze.

W wierszu polecen Uruchom.bat --dev otwiera wybor lokacji,
a Uruchom.bat --start=volcano-boss uruchamia sesje przy Varkhorze.
NOWA SESJA TESTOWA tworzy nowa wyprawe i zywych bossow.
PRZENIES OBECNA POSTAC zachowuje sprzet, statystyki, zdobyte punkty i postep walk.
W obu opcjach postac ma poziom 100 i zna wszystkie umiejetnosci oraz czary.
Domyslnie ma pelne 920 HP i 510 many. Postac DEV jest odporna na obrazenia.
Plecak jest uzupelniany do 100 strzal i po 50 mikstur zdrowia oraz many.
Plecak zawiera wszystkie rodzaje wyposazenia z aktualnego katalogu gry.
Strzalki pod plecakiem przelaczaja strony po 25 pol.
Zapisy z menu pauzy w sesji DEV korzystaja z osobnych slotow.
F5 otwiera menu zapisu w dowolnym miejscu.
Aktywne pociski, erupcje i przewrocenie sa czyszczone podczas teleportu.
Start DEV w kalderze ustawia testowy postep drugiego aktu,
aby mozna bylo wracac tam takze przez Eldrina.
F3 wraca do wyboru lokacji.
Folder assets musi pozostac obok Emberfall.exe.
'@
[IO.File]::WriteAllText((Join-Path $folder 'JAK_URUCHOMIC_DEV.txt'),$instructions,[Text.UTF8Encoding]::new($true))
Add-Type -AssemblyName System.IO.Compression.FileSystem
$temporary=Join-Path $PSScriptRoot ('Emberfall-DEV-package-'+[guid]::NewGuid().ToString('N')+'.zip')
[IO.Compression.ZipFile]::CreateFromDirectory($folder,$temporary,[IO.Compression.CompressionLevel]::Optimal,$true)
$target=Join-Path $PSScriptRoot 'Emberfall-DEV-Windows-x64.zip'
if(Test-Path -LiteralPath $target){
 $backup=Join-Path $PSScriptRoot ('art/pipeline/backups/dev-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss-fff'))
 New-Item -ItemType Directory -Path $backup | Out-Null
 Copy-Item -LiteralPath $target -Destination (Join-Path $backup 'Emberfall-DEV-Windows-x64.zip')
}
Move-Item -LiteralPath $temporary -Destination $target -Force
Write-Output ('Gotowe: '+$target)

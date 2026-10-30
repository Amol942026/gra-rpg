param([switch]$NoBuild)
$ErrorActionPreference='Stop'
Set-Location -LiteralPath $PSScriptRoot
if(!$NoBuild){& (Join-Path $PSScriptRoot 'build.ps1')}
$stage=Join-Path $PSScriptRoot ('dist/qa-volcano-dev-stage-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss-fff'))
$folder=Join-Path $stage 'Emberfall-DEV'
New-Item -ItemType Directory -Path $folder | Out-Null
Copy-Item -LiteralPath 'dist/Emberfall.exe' -Destination (Join-Path $folder 'Emberfall.exe')
Copy-Item -LiteralPath 'dist/assets' -Destination (Join-Path $folder 'assets') -Recurse
foreach($file in @('Tryb_deweloperski.bat','Test_kaldery.bat','Test_Varkhora.bat','LICENSES.md','LOOT.md')){
 $source=if($file.EndsWith('.bat')){Join-Path 'dist' $file}else{$file}
 Copy-Item -LiteralPath $source -Destination (Join-Path $folder $file)
}
$instructions=@'
EMBERFALL — WERSJA DEWELOPERSKA

Rozpakuj caly folder Emberfall-DEV. Uruchom Tryb_deweloperski.bat.
F3 otwiera wybor lokacji. Prawa kolumna: KALDERA POPIOLU.
- Kaldera — Brama Popiolu: poczatek trzeciej krainy.
- Obsydianowe Kuznie: wschodni szlak kaldery.
- Boss — Varkhor: bezposrednio przy trzecim bossie.

Test_kaldery.bat i Test_Varkhora.bat uruchamiaja wybrany punkt od razu.
NOWA SESJA TESTOWA tworzy nowa wyprawe i zywych bossow.
PRZENIES OBECNA POSTAC zachowuje ekwipunek, poziom, punkty i postep walk.
Aktywne pociski, erupcje i przewrocenie sa czyszczone podczas teleportu.
Start DEV w kalderze ustawia testowy postep drugiego aktu,
aby mozna bylo wracac tam takze przez Eldrina. Nie przyznaje rang umiejetnosci.
ZESTAW TESTOWY jest opcjonalny. F3 wraca do wyboru lokacji.
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

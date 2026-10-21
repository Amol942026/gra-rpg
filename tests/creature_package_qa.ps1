param([string]$SynfigProject='C:\AI2D\animations\emberfall_creatures_20261021_v1')
$ErrorActionPreference='Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$qaRoot=Join-Path $PWD 'dist\qa-creatures'
$package=Join-Path $PWD 'Emberfall-Windows-x64.zip'
$extract=Join-Path $qaRoot ('package-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath $package -DestinationPath $extract
$gameRoot=Join-Path $extract 'Emberfall'
$exe=Join-Path $gameRoot 'Emberfall.exe'
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist\player\Emberfall.exe').Hash){throw 'Player executable mismatch'}
$count=0;$sourceAssets=Join-Path $PWD 'assets'
foreach($asset in Get-ChildItem -LiteralPath $sourceAssets -Recurse -File){
 $relative=$asset.FullName.Substring($sourceAssets.Length+1);$target=Join-Path (Join-Path $gameRoot 'assets') $relative
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $asset.FullName).Hash -ne (Get-FileHash -LiteralPath $target).Hash){throw "Asset mismatch: $relative"};$count++
}
if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp')}){throw 'Native masters in player package'}
$smoke=Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WindowStyle Hidden -Wait -PassThru
if($smoke.ExitCode -ne 0){throw 'Extracted player startup failed'}
$startup=Get-Content -LiteralPath (Join-Path $gameRoot 'startup-smoke.txt') -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0')){if(!$startup.Contains($expected)){throw "Missing startup evidence: $expected"}}
Copy-Item -LiteralPath 'dist\creature_qa.exe' -Destination (Join-Path $gameRoot 'creature_qa.exe')
New-Item -ItemType Directory -Path (Join-Path $gameRoot 'qa-creatures\before') -Force | Out-Null
Copy-Item -LiteralPath 'dist\qa-creatures\before\hero' -Destination (Join-Path $gameRoot 'qa-creatures\before') -Recurse
$renderer=Start-Process -FilePath (Join-Path $gameRoot 'creature_qa.exe') -ArgumentList '--no-audio','qa-creatures\runtime' -WindowStyle Hidden -Wait -PassThru
if($renderer.ExitCode -ne 0){throw 'Extracted player creature QA failed'}
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py 'tools\media\verify_creatures.py' '--project' $SynfigProject '--qa' (Join-Path $gameRoot 'qa-creatures\runtime') '--no-audio'
if($LASTEXITCODE -ne 0){throw 'Extracted player image or native animation validation failed'}
$native=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-creatures\runtime\verification.json') -Raw | ConvertFrom-Json
$report=[ordered]@{status='PASS';package=$package;package_sha256=(Get-FileHash -LiteralPath $package).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;startup_frames=180;renderer_checks=$native.checks;poses=288;animation_frames=60;species=12;directions=8;audio_device_opened=$false;native_sources_in_player=$false}
$report | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $qaRoot 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json

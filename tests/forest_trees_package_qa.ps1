param([switch]$NoAudio)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Extracted forest tree test requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$package=Join-Path $PWD 'Emberfall-Windows-x64.zip'
$output=Join-Path $PWD ('dist/qa-trees-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath $package -DestinationPath $output
$gameRoot=Join-Path $output 'Emberfall';$exe=Join-Path $gameRoot 'Emberfall.exe'
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist/player/Emberfall.exe').Hash){throw 'Player executable hash mismatch'}
foreach($name in @('LICENSES.md','LOOT.md')){
 if((Get-FileHash -LiteralPath (Join-Path $gameRoot $name)).Hash -ne (Get-FileHash -LiteralPath $name).Hash){throw "Package document mismatch: $name"}
}
$assets=Join-Path $PWD 'assets';$count=0
foreach($asset in Get-ChildItem -LiteralPath $assets -Recurse -File){
 $relative=$asset.FullName.Substring($assets.Length+1);$target=Join-Path (Join-Path $gameRoot 'assets') $relative
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $asset.FullName).Hash -ne (Get-FileHash -LiteralPath $target).Hash){throw "Asset mismatch: $relative"};$count++
}
if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -eq 'AGENTS.md' -or $_.Name -match '_qa\.exe$'}){throw 'Native source or developer helpers in player release'}
$settings=Join-Path $gameRoot 'audio-settings.cfg';[IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine);$settingsHash=(Get-FileHash -LiteralPath $settings).Hash
$started=[DateTime]::UtcNow
$job=Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WindowStyle Hidden -Wait -PassThru
if($job.ExitCode -ne 0){throw 'Extracted player startup failed'}
$startupPath=Join-Path $gameRoot 'startup-smoke.txt'
if((Get-Item -LiteralPath $startupPath).LastWriteTimeUtc -lt $started){throw 'Stale startup evidence'}
$startup=Get-Content -LiteralPath $startupPath -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1','Intro active: 0')){if(!$startup.Contains($expected)){throw "Unexpected startup: $expected"}}
Copy-Item -LiteralPath 'dist/forest_trees_qa.exe' -Destination (Join-Path $gameRoot 'forest_trees_qa.exe')
$job=Start-Process -FilePath (Join-Path $gameRoot 'forest_trees_qa.exe') -ArgumentList '--no-audio','qa-trees' -WindowStyle Hidden -Wait -PassThru
if($job.ExitCode -ne 0){throw 'Extracted tree renderer diagnostic failed'}
$checks=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-trees/checks.txt') -Raw
if(!$checks.Contains('Failures: 0')){throw 'Tree diagnostic has no successful result'}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'A silent test changed audio preferences'}
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/verify_forest_trees.py --qa (Join-Path $gameRoot 'qa-trees') --no-audio
if($LASTEXITCODE -ne 0){throw 'Independent tree source/image checks failed'}
$source=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-trees/source-verification.json') -Raw | ConvertFrom-Json
$report=[ordered]@{status='PASS';package_sha256=(Get-FileHash -LiteralPath $package).Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;renderer_checks=[int]([regex]::Match($checks,'Checks: (\d+)').Groups[1].Value);independent_checks=$source.checks;startup_frames=180;audio_device_opened=$false;audio_preferences_unchanged=$true;native_sources_in_player=$false;tree_caches=$source.runtime_cache_size;additional_tree_and_outline_bytes=$source.added_cache_and_outline_rgba8_bytes}
$report | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json -Depth 5

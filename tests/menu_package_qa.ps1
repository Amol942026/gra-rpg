param([switch]$NoAudio)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Menu package verification requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$package=Join-Path $PWD 'Emberfall-Windows-x64.zip'
$output=Join-Path $PWD ('dist\qa-menu-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath $package -DestinationPath $output
$gameRoot=Join-Path $output 'Emberfall'
$exe=Join-Path $gameRoot 'Emberfall.exe'
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist\player\Emberfall.exe').Hash){throw 'Player executable mismatch'}
foreach($document in @('LICENSES.md','LOOT.md')){
 if((Get-FileHash -LiteralPath (Join-Path $gameRoot $document)).Hash -ne (Get-FileHash -LiteralPath $document).Hash){throw "Package document mismatch: $document"}
}
$assetRoot=Join-Path $PWD 'assets';$count=0
foreach($asset in Get-ChildItem -LiteralPath $assetRoot -File -Recurse){
 $relative=$asset.FullName.Substring($assetRoot.Length+1);$target=Join-Path (Join-Path $gameRoot 'assets') $relative
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $asset.FullName).Hash -ne (Get-FileHash -LiteralPath $target).Hash){throw "Asset mismatch: $relative"};$count++
}
if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -eq 'AGENTS.md' -or $_.Name -match '_qa\.exe$'}){throw 'Native masters, developer test or saves in player package'}
$settings=Join-Path $gameRoot 'audio-settings.cfg'
# A diagnostic must not overwrite even an existing real-format preference file.
[IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine)
$settingsHash=(Get-FileHash -LiteralPath $settings).Hash
$started=[DateTime]::UtcNow
$job=Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WindowStyle Hidden -Wait -PassThru
if($job.ExitCode -ne 0){throw 'Extracted player startup failed'}
$startupPath=Join-Path $gameRoot 'startup-smoke.txt'
if((Get-Item -LiteralPath $startupPath).LastWriteTimeUtc -lt $started){throw 'Stale startup evidence'}
$startup=Get-Content -LiteralPath $startupPath -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1','Intro active: 0')){if(!$startup.Contains($expected)){throw "Missing startup evidence: $expected"}}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Silent startup changed saved audio preferences'}
if(!(Test-Path -LiteralPath (Join-Path $gameRoot 'screenshots\emberfall-startup.png'))){throw 'No actual presented startup screenshot'}
Copy-Item -LiteralPath 'dist\menu_qa.exe' -Destination (Join-Path $gameRoot 'menu_qa.exe')
$job=Start-Process -FilePath (Join-Path $gameRoot 'menu_qa.exe') -ArgumentList '--no-audio','qa-menu' -WindowStyle Hidden -Wait -PassThru
if($job.ExitCode -ne 0){throw 'Extracted player menu QA failed'}
$checks=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-menu\checks.txt') -Raw
if(!$checks.Contains('Failures: 0')){throw 'Missing successful menu QA report'}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Menu QA changed saved audio preferences'}
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py 'tools\media\verify_menu.py' '--qa' (Join-Path $gameRoot 'qa-menu') '--startup' (Join-Path $gameRoot 'screenshots\emberfall-startup.png') '--no-audio'
if($LASTEXITCODE -ne 0){throw 'Independent PNG validation failed'}
$images=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-menu\image-verification.json') -Raw | ConvertFrom-Json
& $py 'tools\media\verify_forest_menu.py' '--qa' (Join-Path $gameRoot 'qa-menu') '--no-audio'
if($LASTEXITCODE -ne 0){throw 'Forest assets or actual animation verification failed'}
$forest=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-menu\forest-menu-verification.json') -Raw | ConvertFrom-Json
$rendererChecks=[int]([regex]::Match($checks,'Checks: (\d+)').Groups[1].Value)
$fps=[double]([regex]::Match($startup,'Average FPS: ([\d.]+)').Groups[1].Value)
$cpu=[double]([regex]::Match($startup,'CPU frame mean \(ms\): ([\d.]+)').Groups[1].Value)
$report=[ordered]@{status='PASS';package_sha256=(Get-FileHash -LiteralPath $package).Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;menu_checks=$rendererChecks;menu_images=@(Get-ChildItem -LiteralPath (Join-Path $gameRoot 'qa-menu') -Filter '*.png').Count;independent_image_checks=$images.checks;forest_animation_checks=$forest.checks;native_synfig_frames=$forest.native_frames;runtime_menu_frames=$forest.runtime_menu_frames;runtime_treant_frames=$forest.runtime_treant_frames;startup_frames=180;startup_menu=$true;startup_intro=$false;menu_fps_including_png_capture=$fps;menu_cpu_ms_including_png_capture=$cpu;audio_device_opened=$false;audio_preferences_unchanged=$true;native_sources_in_package=$false}
$report | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json

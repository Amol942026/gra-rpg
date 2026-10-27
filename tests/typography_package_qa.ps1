param([switch]$NoAudio,[string]$HelperRoot='dist/qa-typography-20261026-v2')
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Player package verification requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$package=Join-Path $PWD 'Emberfall-Windows-x64.zip'
$output=Join-Path $PWD ('dist/qa-typography-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath $package -DestinationPath $output
$gameRoot=Join-Path $output 'Emberfall';$exe=Join-Path $gameRoot 'Emberfall.exe'
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist/player/Emberfall.exe').Hash){throw 'Player executable mismatch'}
$assets=Join-Path $PWD 'assets';$count=0
foreach($asset in Get-ChildItem -LiteralPath $assets -Recurse -File){
 $target=Join-Path (Join-Path $gameRoot 'assets') $asset.FullName.Substring($assets.Length+1)
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $asset.FullName).Hash -ne (Get-FileHash -LiteralPath $target).Hash){throw ('Asset mismatch: '+$asset.Name)};$count++
}
if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -eq 'AGENTS.md' -or $_.Name -match '_qa\.exe$'}){throw 'Native source or helper in player ZIP'}
$settings=Join-Path $gameRoot 'audio-settings.cfg';[IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine);$settingsHash=(Get-FileHash -LiteralPath $settings).Hash
$started=[DateTime]::UtcNow
$job=Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru
if(!$job.WaitForExit(60000)){$job.Kill();throw 'Extracted player startup timed out'}
if($job.ExitCode -ne 0){throw 'Extracted player startup failed'}
$startupPath=Join-Path $gameRoot 'startup-smoke.txt';if((Get-Item -LiteralPath $startupPath).LastWriteTimeUtc -lt $started){throw 'Stale startup test'}
$startup=Get-Content -LiteralPath $startupPath -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1')){if(!$startup.Contains($expected)){throw "Startup mismatch: $expected"}}
Copy-Item -LiteralPath (Join-Path $HelperRoot 'typography_qa.exe') -Destination (Join-Path $gameRoot 'typography_qa.exe')
$job=Start-Process -FilePath (Join-Path $gameRoot 'typography_qa.exe') -ArgumentList '--no-audio','checks' -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru
if(!$job.WaitForExit(120000)){$job.Kill();throw 'Extracted typography test timed out'}
if($job.ExitCode -ne 0){throw 'Extracted typography test failed'}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Silent tests changed audio preferences'}
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/verify_typography.py --qa (Join-Path $gameRoot 'checks') --assets (Join-Path $gameRoot 'assets') --no-audio
if($LASTEXITCODE -ne 0){throw 'Independent font and image checks failed'}
$verified=Get-Content -LiteralPath (Join-Path $gameRoot 'checks/verification.json') -Raw | ConvertFrom-Json
$report=[ordered]@{status='PASS';package_sha256=(Get-FileHash -LiteralPath $package).Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;tests=$verified;startup_frames=180;audio_device_opened=$false;audio_preferences_unchanged=$true;native_sources_in_player=$false}
$report | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json -Depth 6

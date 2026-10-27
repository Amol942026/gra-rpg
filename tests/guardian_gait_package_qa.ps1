param([switch]$NoAudio,[string]$HelperRoot='dist/qa-guardian-gait-20261027-v3',[switch]$ExpandedLoot)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Guardian gait package QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$package=Join-Path $PWD 'Emberfall-Windows-x64.zip'
$output=Join-Path $PWD ('dist/qa-guardian-gait-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
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
$job=Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru
if(!$job.WaitForExit(60000)){$job.Kill();throw 'Extracted startup timed out'}
if($job.ExitCode -ne 0){throw 'Extracted player startup failed'}
$startup=Get-Content -LiteralPath (Join-Path $gameRoot 'startup-smoke.txt') -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1')){if(!$startup.Contains($expected)){throw "Startup mismatch: $expected"}}
Copy-Item -LiteralPath (Join-Path $HelperRoot 'guardian_gait_qa.exe') -Destination (Join-Path $gameRoot 'guardian_gait_qa.exe')
$job=Start-Process -FilePath (Join-Path $gameRoot 'guardian_gait_qa.exe') -ArgumentList '--no-audio','checks' -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru
if(!$job.WaitForExit(180000)){$job.Kill();throw 'Extracted gait QA timed out'}
if($job.ExitCode -ne 0){throw 'Extracted gait QA failed'}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Silent tests changed audio preferences'}
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/verify_guardian_gait.py --qa (Join-Path $gameRoot 'checks') --no-audio
if($LASTEXITCODE -ne 0){throw 'Independent native curve, contact and frame checks failed'}
if($ExpandedLoot){
 & $py tools/media/verify_expanded_loot.py --qa (Join-Path $gameRoot 'checks') --no-audio
 if($LASTEXITCODE -ne 0){throw 'Expanded loot and knee extension checks failed'}
}
$report=[ordered]@{status='PASS';package_sha256=(Get-FileHash -LiteralPath $package).Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;renderer=(Get-Content -LiteralPath (Join-Path $gameRoot 'checks/summary.json') -Raw | ConvertFrom-Json);independent=(Get-Content -LiteralPath (Join-Path $gameRoot 'checks/verification.json') -Raw | ConvertFrom-Json);startup_frames=180;audio_device_opened=$false;audio_preferences_unchanged=$true;native_sources_in_player=$false}
$report | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
if($ExpandedLoot){$report['expansion']=Get-Content -LiteralPath (Join-Path $gameRoot 'checks/expansion-verification.json') -Raw | ConvertFrom-Json;$report | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8}
$report | ConvertTo-Json -Depth 6

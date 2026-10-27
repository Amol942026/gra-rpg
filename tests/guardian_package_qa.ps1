param([switch]$NoAudio)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Guardian player package verification requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$package=Join-Path $PWD 'Emberfall-Windows-x64.zip'
$output=Join-Path $PWD ('dist/qa-guardian-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath $package -DestinationPath $output
$gameRoot=Join-Path $output 'Emberfall';$exe=Join-Path $gameRoot 'Emberfall.exe'
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist/player/Emberfall.exe').Hash){throw 'Player executable mismatch'}
foreach($name in @('LICENSES.md','LOOT.md')){if((Get-FileHash -LiteralPath (Join-Path $gameRoot $name)).Hash -ne (Get-FileHash -LiteralPath $name).Hash){throw "Document mismatch: $name"}}
$assets=Join-Path $PWD 'assets';$count=0
foreach($asset in Get-ChildItem -LiteralPath $assets -Recurse -File){
 $relative=$asset.FullName.Substring($assets.Length+1);$target=Join-Path (Join-Path $gameRoot 'assets') $relative
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $asset.FullName).Hash -ne (Get-FileHash -LiteralPath $target).Hash){throw "Asset mismatch: $relative"};$count++
}
if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -eq 'AGENTS.md' -or $_.Name -match '_qa\.exe$'}){throw 'Native masters or developer helper in player package'}
$settings=Join-Path $gameRoot 'audio-settings.cfg';[IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine);$settingsHash=(Get-FileHash -LiteralPath $settings).Hash
$started=[DateTime]::UtcNow
$job=Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WindowStyle Hidden -Wait -PassThru
if($job.ExitCode -ne 0){throw 'Extracted player failed to start'}
$startupPath=Join-Path $gameRoot 'startup-smoke.txt'
if((Get-Item -LiteralPath $startupPath).LastWriteTimeUtc -lt $started){throw 'Stale startup test'}
$startup=Get-Content -LiteralPath $startupPath -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1','Intro active: 0')){if(!$startup.Contains($expected)){throw "Unexpected startup: $expected"}}
Copy-Item -LiteralPath 'dist/guardian_qa.exe' -Destination (Join-Path $gameRoot 'guardian_qa.exe')
$job=Start-Process -FilePath (Join-Path $gameRoot 'guardian_qa.exe') -ArgumentList '--no-audio','qa-guardian' -WindowStyle Hidden -Wait -PassThru
if($job.ExitCode -ne 0){throw 'Extracted player Guardian/arrival diagnostic failed'}
$checks=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-guardian/checks.txt') -Raw
if(!$checks.Contains('Failures: 0')){throw 'Missing successful Guardian result'}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Silent test changed audio preferences'}
$py='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py tools/media/verify_guardian.py --qa (Join-Path $gameRoot 'qa-guardian') --previews --no-audio
if($LASTEXITCODE -ne 0){throw 'Independent native source and frame checks failed'}
$images=Get-Content -LiteralPath (Join-Path $gameRoot 'qa-guardian/image-verification.json') -Raw|ConvertFrom-Json
$report=[ordered]@{status='PASS';package_sha256=(Get-FileHash -LiteralPath $package).Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;renderer_checks=[int]([regex]::Match($checks,'Checks: (\d+)').Groups[1].Value);independent_checks=$images.checks;native_synfig_frames=$images.native_synfig_frames;compiled_control_samples=$images.compiled_control_samples;game_frames=$images.game_frames;shatter=$images.shatter;knockback=$images.knockback;knockdown=$images.knockdown;fountains=$images.fountains;startup_frames=180;audio_device_opened=$false;audio_preferences_unchanged=$true;native_sources_in_player=$false}
$report | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json -Depth 5

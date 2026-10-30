param([switch]$NoAudio,[Parameter(Mandatory)][string]$HelperRoot)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Developer package QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$output=Join-Path $PWD ('dist/qa-volcano-dev-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath 'Emberfall-DEV-Windows-x64.zip' -DestinationPath $output
$root=Join-Path $output 'Emberfall-DEV';$exe=Join-Path $root 'Emberfall.exe'
if(Get-ChildItem -LiteralPath $root -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -match '_qa\.exe$'}){throw 'Native master or test helper in developer ZIP'}
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist/Emberfall.exe').Hash){throw 'Wrong developer executable in ZIP'}
$assetRoot=Join-Path $PWD 'assets';$assetCount=0
foreach($asset in Get-ChildItem -LiteralPath $assetRoot -Recurse -File){
 $relative=$asset.FullName.Substring($assetRoot.Length+1);$target=Join-Path (Join-Path $root 'assets') $relative
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target).Hash -ne (Get-FileHash -LiteralPath $asset.FullName).Hash){throw ('Wrong packaged asset: '+$relative)};$assetCount++
}
foreach($launcher in @('Tryb_deweloperski.bat','Test_kaldery.bat','Test_Varkhora.bat')){
 $text=Get-Content -LiteralPath (Join-Path $root $launcher) -Raw
 if($text.Contains('dist\Emberfall.exe') -or !$text.Contains('"Emberfall.exe"')){throw ('Wrong packaged launcher path: '+$launcher)}
}
$settings=Join-Path $root 'audio-settings.cfg';[IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine);$settingsHash=(Get-FileHash -LiteralPath $settings).Hash
function Invoke-Hidden([string]$program,[string[]]$arguments){
 if('--no-audio' -notin $arguments -or '--background-test' -notin $arguments){throw 'Missing silent/hidden test arguments'}
 $process=Start-Process -FilePath $program -ArgumentList $arguments -WorkingDirectory $root -WindowStyle Hidden -PassThru
 if(!$process.WaitForExit(90000)){$process.Kill();throw 'Owned developer test timed out'}
 if($process.ExitCode -ne 0){throw ('Developer test failed: '+$process.ExitCode)}
}
foreach($destination in @('volcano','volcano-forges','volcano-boss')){
 Invoke-Hidden $exe @('--dev-smoke',('--start='+$destination),'--no-dev-kit','--no-audio','--background-test','--windowed')
 $report=Get-Content -LiteralPath (Join-Path $root 'dev-startup-v43.txt') -Raw
 foreach($expected in @('Build edition: developer','Frames: 180','World location: volcano',('Developer destination: '+$destination),'Developer mode: 1','Developer menu open: 0','Audio device opened: no','Window hidden: yes','Intro active: 0')){if(!$report.Contains($expected)){throw ('Developer startup mismatch: '+$expected)}}
 Copy-Item -LiteralPath (Join-Path $root 'dev-startup-v43.txt') -Destination (Join-Path $output ($destination+'-startup.txt'))
}
Copy-Item -LiteralPath (Join-Path $HelperRoot 'developer_volcano_qa.exe') -Destination (Join-Path $root 'developer_volcano_qa.exe')
Invoke-Hidden (Join-Path $root 'developer_volcano_qa.exe') @('--no-audio','developer-checks','--background-test')
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Developer tests changed saved audio preferences'}
$report=[ordered]@{status='PASS';zip_sha256=(Get-FileHash -LiteralPath 'Emberfall-DEV-Windows-x64.zip').Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$root;verified_assets=$assetCount;startup_destinations=3;startup_frames=540;audio_preferences_unchanged=$true;native_masters_in_zip=$false;ui=(Get-Content -LiteralPath (Join-Path $root 'developer-checks/summary.json') -Raw | ConvertFrom-Json)}
$report | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json -Depth 7

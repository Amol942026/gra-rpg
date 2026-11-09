param([switch]$NoAudio,[Parameter(Mandatory)][string]$HelperRoot)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Package QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$output=Join-Path $PWD ('dist/qa-volcano-audio_suite-packages-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $output | Out-Null
$reports=@()
foreach($edition in @('player','developer')){
 $archive=if($edition -eq 'player'){'Emberfall-Windows-x64.zip'}else{'Emberfall-DEV-Windows-x64.zip'}
 $folder=if($edition -eq 'player'){'Emberfall'}else{'Emberfall-DEV'}
 $built=if($edition -eq 'player'){'dist/player/Emberfall.exe'}else{'dist/Emberfall.exe'}
 $destination=Join-Path $output $edition
 Expand-Archive -LiteralPath $archive -DestinationPath $destination
 $gameRoot=Join-Path $destination $folder;$exe=Join-Path $gameRoot 'Emberfall.exe'
 if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath $built).Hash){throw 'Wrong executable in ZIP'}
 $assetRoot=Join-Path $PWD 'assets';$count=0
 foreach($asset in Get-ChildItem -LiteralPath $assetRoot -Recurse -File){
  $target=Join-Path (Join-Path $gameRoot 'assets') $asset.FullName.Substring($assetRoot.Length+1)
  if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target).Hash -ne (Get-FileHash -LiteralPath $asset.FullName).Hash){throw ('Asset mismatch: '+$asset.Name)};$count++
 }
 if((Get-ChildItem -LiteralPath (Join-Path $gameRoot 'assets') -Recurse -File).Count -ne $count){throw 'Unexpected assets in ZIP'}
 if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -eq 'AGENTS.md' -or $_.Name -match '_qa\.exe$'}){throw 'Native source, save or helper in ZIP'}
 $settings=Join-Path $gameRoot 'audio-settings.cfg'
 [IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine)
 $settingsHash=(Get-FileHash -LiteralPath $settings).Hash
 function Invoke-SilentCheck([string]$Program,[string[]]$RunArguments){
  if('--no-audio' -notin $RunArguments -or '--background-test' -notin $RunArguments){throw 'Missing silent/hidden argument'}
  $job=Start-Process -FilePath $Program -ArgumentList $RunArguments -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru
  if(!$job.WaitForExit(180000)){throw 'Owned silent package test timed out'}
  if($job.ExitCode -ne 0){throw ('Package test failed: '+$job.ExitCode)}
 }
 $smokes=@()
 if($edition -eq 'player'){
  Invoke-SilentCheck $exe @('--startup-smoke','--windowed','--no-audio','--background-test')
  $startup=Get-Content -LiteralPath (Join-Path $gameRoot 'startup-smoke.txt') -Raw
  foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1','Window hidden: yes')){if(!$startup.Contains($expected)){throw "Startup mismatch: $expected"}}
  $smokes+=@{scenario='main-menu';frames=180}
  Copy-Item -LiteralPath (Join-Path $HelperRoot 'audio_suite_qa.exe') -Destination (Join-Path $gameRoot 'audio_suite_qa.exe')
  Invoke-SilentCheck (Join-Path $gameRoot 'audio_suite_qa.exe') @('--no-audio','audio_suite-checks','--background-test')
  $audio_suite=Get-Content -LiteralPath (Join-Path $gameRoot 'audio_suite-checks/summary.json') -Raw | ConvertFrom-Json
 }else{
  foreach($realm in @('forest','desert','volcano')){
   Invoke-SilentCheck $exe @('--dev-smoke',"--start=$realm",'--windowed','--no-audio','--background-test')
   $startup=Get-Content -LiteralPath (Join-Path $gameRoot 'dev-startup-v43.txt') -Raw
   foreach($expected in @('Build edition: developer','Frames: 180','Audio device opened: no','Developer mode: 1',"World location: $realm",'Window hidden: yes')){if(!$startup.Contains($expected)){throw "Developer startup mismatch: $expected"}}
   Copy-Item -LiteralPath (Join-Path $gameRoot 'dev-startup-v43.txt') -Destination (Join-Path $destination ($realm+'-startup.txt'))
   $smokes+=@{scenario=$realm;frames=180}
  }
  $audio_suite=$null
 }
 if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Silent checks changed audio preferences'}
 $reports+=[ordered]@{status='PASS';edition=$edition;package_sha256=(Get-FileHash -LiteralPath $archive).Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();verified_assets=$count;startup=$smokes;audio_suite=$audio_suite;audio_preferences_unchanged=$true;native_sources_in_zip=$false}
}
$reports | ConvertTo-Json -Depth 9 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$reports | ConvertTo-Json -Depth 9

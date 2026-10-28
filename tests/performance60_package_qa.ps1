param([switch]$NoAudio,[Parameter(Mandatory)][string]$RendererHelperRoot,[Parameter(Mandatory)][string]$PerformanceHelperRoot,[switch]$Present,[switch]$Dxgi)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Package verification requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$output=Join-Path $PWD ('dist/qa-performance60-package-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath 'Emberfall-Windows-x64.zip' -DestinationPath $output
$gameRoot=Join-Path $output 'Emberfall';$exe=Join-Path $gameRoot 'Emberfall.exe'
if((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath 'dist/player/Emberfall.exe').Hash){throw 'Wrong player executable in ZIP'}
$assetRoot=Join-Path $PWD 'assets';$count=0
foreach($asset in Get-ChildItem -LiteralPath $assetRoot -Recurse -File){
 $target=Join-Path (Join-Path $gameRoot 'assets') $asset.FullName.Substring($assetRoot.Length+1)
 if(!(Test-Path -LiteralPath $target) -or (Get-FileHash -LiteralPath $target).Hash -ne (Get-FileHash -LiteralPath $asset.FullName).Hash){throw ('Asset mismatch: '+$asset.Name)};$count++
}
if(Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object {$_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp','.efs','.bak') -or $_.Name -eq 'AGENTS.md' -or $_.Name -match '_qa\.exe$'}){throw 'Native source or test helper in ZIP'}
$settings=Join-Path $gameRoot 'audio-settings.cfg';[IO.File]::WriteAllText($settings,'0.31 0.67 0 0 1'+[Environment]::NewLine);$settingsHash=(Get-FileHash -LiteralPath $settings).Hash
function Invoke-SilentCheck([string]$Program,[string[]]$RunArguments,[int]$Timeout=60){
 if('--no-audio' -notin $RunArguments){throw 'Missing silent argument'}
 $job=Start-Process -FilePath $Program -ArgumentList $RunArguments -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru
 if(!$job.WaitForExit($Timeout*1000)){$job.Kill();throw 'Owned silent package test timed out'}
 if($job.ExitCode -ne 0){throw ('Package test failed: '+$job.ExitCode)}
}
Invoke-SilentCheck $exe @('--startup-smoke','--windowed','--no-audio')
$startup=Get-Content -LiteralPath (Join-Path $gameRoot 'startup-smoke.txt') -Raw
foreach($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0','Main menu open: 1')){if(!$startup.Contains($expected)){throw "Startup mismatch: $expected"}}
Copy-Item -LiteralPath (Join-Path $RendererHelperRoot 'render_optimization_qa.exe') -Destination (Join-Path $gameRoot 'render_optimization_qa.exe')
Invoke-SilentCheck (Join-Path $gameRoot 'render_optimization_qa.exe') @('--no-audio','renderer-checks') 120
if($Present){
 Copy-Item -LiteralPath (Join-Path $PerformanceHelperRoot 'performance60_qa.exe') -Destination (Join-Path $gameRoot 'performance60_qa.exe')
 $performanceArguments=@('--no-audio','performance-checks','--present','--quick');if($Dxgi){$performanceArguments+='--dxgi'}
 Invoke-SilentCheck (Join-Path $gameRoot 'performance60_qa.exe') $performanceArguments 120
}
if((Get-FileHash -LiteralPath $settings).Hash -ne $settingsHash){throw 'Silent checks changed audio preferences'}
$report=[ordered]@{status='PASS';package_sha256=(Get-FileHash -LiteralPath 'Emberfall-Windows-x64.zip').Hash.ToLowerInvariant();exe_sha256=(Get-FileHash -LiteralPath $exe).Hash.ToLowerInvariant();extracted=$gameRoot;verified_assets=$count;startup_frames=180;renderer=(Get-Content -LiteralPath (Join-Path $gameRoot 'renderer-checks/summary.json') -Raw | ConvertFrom-Json);presentation_measured=[bool]$Present;audio_device_opened=$false;audio_preferences_unchanged=$true;native_sources_in_zip=$false}
if($Present){$report.performance=Get-Content -LiteralPath (Join-Path $gameRoot 'performance-checks/timing.json') -Raw | ConvertFrom-Json}
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json -Depth 8

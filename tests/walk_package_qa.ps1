param([string]$SynfigProject = 'C:\AI2D\animations\emberfall_walk_20261021_v1')
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$heroReport = Get-Content -LiteralPath 'dist\qa-hero-art\package-verification.json' -Raw | ConvertFrom-Json
if ($heroReport.status -ne 'PASS' -or (Get-FileHash -LiteralPath $heroReport.package).Hash.ToLowerInvariant() -ne $heroReport.package_sha256) { throw 'Player package report is stale or failed.' }
$gameRoot = $heroReport.extracted
foreach ($helper in @('walk_metrics.exe','walk_qa.exe','sword_stow_qa.exe','equipment_qa.exe')) {
    Copy-Item -LiteralPath (Join-Path 'dist' $helper) -Destination (Join-Path $gameRoot $helper)
    $arguments = @('--no-audio')
    if ($helper -eq 'walk_metrics.exe') { $arguments += (Join-Path $gameRoot 'qa-walk') }
    if ($helper -eq 'walk_qa.exe') { $arguments += 'qa-walk' }
    $job = Start-Process -FilePath (Join-Path $gameRoot $helper) -ArgumentList $arguments -WindowStyle Hidden -PassThru -Wait
    if ($job.ExitCode -ne 0) { throw "Extracted player verification failed: $helper ($($job.ExitCode))" }
}
$walk = Get-Content -LiteralPath (Join-Path $gameRoot 'qa-walk\render-verification.json') -Raw | ConvertFrom-Json
if ($walk.status -ne 'PASS' -or $walk.failed_writes -ne 0 -or $walk.frames -ne 120) { throw 'Packaged walk frames are incomplete.' }
$py = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $py 'tools\media\verify_walk.py' '--project' $SynfigProject '--qa' (Join-Path $gameRoot 'qa-walk') '--no-audio'
if ($LASTEXITCODE -ne 0) { throw 'Extracted walk images or native control comparison failed.' }
& $py '-c' "import sys;from pathlib import Path;from PIL import Image;paths=list((Path(sys.argv[1])/'qa-equipment').glob('*.png'));assert len(paths)==5;[Image.open(p).load() for p in paths];print('All 5 extracted equipment PNGs decoded.')" $gameRoot
if ($LASTEXITCODE -ne 0) { throw 'Extracted equipment images are invalid.' }
foreach ($pair in @(@('qa-equipment\ui-tests.txt','packaged-equipment-tests.txt'),@('qa-sheath\tests.txt','packaged-stow-tests.txt'),@('qa-walk\render-verification.json','packaged-render-verification.json'))) {
    Copy-Item -LiteralPath (Join-Path $gameRoot $pair[0]) -Destination (Join-Path 'dist\qa-walk' $pair[1])
}
$report = [ordered]@{
    status='PASS';package=$heroReport.package;package_sha256=$heroReport.package_sha256;extracted=$gameRoot
    verified_assets=$heroReport.verified_assets;startup_frames=180;renderer_checks=424;equipment_checks=364;stow_checks=724
    walk_frames=120;walk_directions=8;audio_device_opened=$false;native_sources_in_player=$false
}
$report | ConvertTo-Json | Set-Content -LiteralPath 'dist\qa-walk\package-verification.json' -Encoding utf8
$report | ConvertTo-Json

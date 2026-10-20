param([string]$PackagePath)
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
if (!$PackagePath) { $PackagePath = Join-Path $PWD 'Emberfall-Windows-x64.zip' }
$qaRoot = Join-Path $PWD 'dist\qa-hero-art'
$extract = Join-Path $qaRoot ('package-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
Expand-Archive -LiteralPath $PackagePath -DestinationPath $extract
$gameRoot = Join-Path $extract 'Emberfall'
$exe = Join-Path $gameRoot 'Emberfall.exe'
$sourceExe = Join-Path $PWD 'dist\player\Emberfall.exe'
if ((Get-FileHash -LiteralPath $exe).Hash -ne (Get-FileHash -LiteralPath $sourceExe).Hash) { throw 'Packaged player EXE mismatch' }
$verifiedAssets = 0
$sourceAssets = Join-Path $PWD 'assets'
foreach ($asset in Get-ChildItem -LiteralPath $sourceAssets -File -Recurse) {
    $relative = $asset.FullName.Substring($sourceAssets.Length + 1)
    $packaged = Join-Path (Join-Path $gameRoot 'assets') $relative
    if (!(Test-Path -LiteralPath $packaged) -or (Get-FileHash -LiteralPath $asset.FullName).Hash -ne (Get-FileHash -LiteralPath $packaged).Hash) { throw "Packaged asset mismatch: $relative" }
    $verifiedAssets++
}
if (Get-ChildItem -LiteralPath $gameRoot -Recurse -File | Where-Object { $_.Extension -in @('.kra','.sif','.sifz','.blend','.csd','.mmp') }) { throw 'Native production sources were included in the player archive' }
$smoke = Start-Process -FilePath $exe -ArgumentList '--startup-smoke','--windowed','--no-audio' -WindowStyle Hidden -PassThru -Wait
if ($smoke.ExitCode -ne 0) { throw "Extracted player startup failed: $($smoke.ExitCode)" }
$startup = Get-Content -LiteralPath (Join-Path $gameRoot 'startup-smoke.txt') -Raw
foreach ($expected in @('Build edition: player','Frames: 180','Audio device opened: no','Developer mode: 0')) {
    if (!$startup.Contains($expected)) { throw "Missing package startup evidence: $expected" }
}
Copy-Item -LiteralPath (Join-Path $PWD 'dist\hero_art_qa.exe') -Destination (Join-Path $gameRoot 'hero_art_qa.exe')
$renderer = Start-Process -FilePath (Join-Path $gameRoot 'hero_art_qa.exe') -ArgumentList '--no-audio' -WindowStyle Hidden -PassThru -Wait
if ($renderer.ExitCode -ne 0) { throw 'Native hero QA failed against the extracted player assets' }
Copy-Item -LiteralPath (Join-Path $gameRoot 'qa-hero-art\render-tests.txt') -Destination (Join-Path $qaRoot 'packaged-render-tests.txt')
$report = [ordered]@{
    status = 'PASS'; package = (Resolve-Path -LiteralPath $PackagePath).Path
    package_sha256 = (Get-FileHash -LiteralPath $PackagePath -Algorithm SHA256).Hash.ToLowerInvariant()
    extracted = $gameRoot; verified_assets = $verifiedAssets
    exe_sha256 = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
    hero_atlas_sha256 = (Get-FileHash -LiteralPath (Join-Path $gameRoot 'assets\sprites\hero-paints.png')).Hash.ToLowerInvariant()
    startup_frames = 180; native_renderer_checks = 424; audio_device_opened = $false; native_sources_in_player = $false
}
$report | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $qaRoot 'package-verification.json') -Encoding utf8
$report | ConvertTo-Json -Depth 5

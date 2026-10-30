param([switch]$NoAudio)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Release verification requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$mediaPython='C:/Users/Amadeusz/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
& $mediaPython 'tools/media/verify_volcano_river_detail.py' '--paints' 'C:/AI2D/sprites/emberfall-volcano-river-detail-20261029' '--previous' 'C:/AI2D/sprites/emberfall-volcano-polish-v2-20261029' '--motion' 'C:/AI2D/animations/emberfall-volcano-river-detail-20261029' '--runtime' 'assets/volcano' '--header' 'src/generated/volcano_motion.hpp' '--output' 'art/volcano/river-detail/asset-verification.json' '--no-audio'
if($LASTEXITCODE -ne 0){throw 'Native asset verification failed'}
$helperName='qa-volcano-final-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')
& (Join-Path $PSScriptRoot 'volcano_qa.ps1') -NoAudio -OutputName $helperName
& (Join-Path (Split-Path -Parent $PSScriptRoot) 'package.ps1')
& (Join-Path $PSScriptRoot 'volcano_package_qa.ps1') -NoAudio -HelperRoot (Join-Path $PWD ('dist/'+$helperName))

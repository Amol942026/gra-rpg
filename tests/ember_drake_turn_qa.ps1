param(
    [Parameter(Mandatory)][switch]$NoAudio,
    [ValidateSet('current','baseline','staged')][string]$Edition = 'current',
    [string]$Case = 'ember-drake-turn',
    [ValidateSet('drake','crawler')][string]$Creature = 'drake'
)
$ErrorActionPreference = 'Stop'
if (-not $NoAudio) {throw 'Wymagany parametr -NoAudio.'}
if ($Case -notmatch '^[a-zA-Z0-9_-]+$') {throw 'Niepoprawna nazwa próby.'}
$repo = Split-Path $PSScriptRoot -Parent
$stage = Join-Path $repo "dist/qa-local/$Case/$Edition"
$source = if ($Edition -eq 'current') {$repo} else {$stage}
$output = Join-Path $stage 'drake-qa.exe'
New-Item -ItemType Directory -Force -Path $stage | Out-Null
if ($Edition -ne 'current') {
    if (!(Test-Path "$source/src/volcano_actors.hpp")) {throw 'Brak zapisanej kopii źródeł odniesienia.'}
    Copy-Item "$PSScriptRoot/ember_drake_art_qa.cpp","$PSScriptRoot/ember_drake_turn_qa.hpp","$PSScriptRoot/enemy_ability_fixture.hpp" "$source/tests" -Force
} else {
    New-Item -ItemType Directory -Force "$stage/assets" | Out-Null
    Copy-Item "$repo/assets/volcano" "$stage/assets" -Recurse -Force
}
$compiler = "$repo/tools/w64devkit/bin/g++.exe"
$lib = "$repo/external/raylib-5.5_win64_mingw-w64"
$env:PATH = (Split-Path $compiler) + ';' + $env:PATH
$compileLog = "$stage/compile.log"
& $compiler "$source/tests/ember_drake_art_qa.cpp" "$source/src/sync_present.cpp" '-std=c++17' '-O1' '-DNDEBUG' '-w' "-I$lib/include" "-L$lib/lib" '-lraylib' '-lopengl32' '-lgdi32' '-lwinmm' '-ldwmapi' '-ld3d11' '-ldxgi' '-ld3dcompiler' '-Wl,--wrap=glfwSwapBuffers' '-static' '-mwindows' '-o' $output *> $compileLog
if ($LASTEXITCODE) {Get-Content $compileLog -Tail 20; throw 'Kompilacja próby nie powiodła się.'}
$prefix = if ($Creature -eq 'crawler') {'crawler'} else {'turn'}
$mode = if ($Edition -eq 'baseline') {"--$prefix-baseline"} else {"--$prefix-qa"}
$process = Start-Process -FilePath $output -ArgumentList '--no-audio','--background-test',$mode -WindowStyle Hidden -Wait -PassThru
Get-Content "$stage/drake-summary.json","$stage/turn-summary.json"
if ($process.ExitCode) {Get-Content "$stage/drake-checks.txt","$stage/turn-checks.txt" | Select-String '^FAIL'; throw 'Test renderera zakończył się błędem.'}

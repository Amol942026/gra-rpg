param([switch]$Test)
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
$compiler = Join-Path $PSScriptRoot 'tools\w64devkit\bin\g++.exe'
if (!(Test-Path -LiteralPath $compiler)) {
    $compiler = (Get-Command g++ -ErrorAction Stop).Source
}
$raylib = Join-Path $PSScriptRoot 'external\raylib-5.5_win64_mingw-w64'
if (!(Test-Path -LiteralPath "$raylib\include\raylib.h")) {
    throw 'Brak raylib 5.5. Zobacz README.md.'
}
$env:PATH = (Split-Path -Parent $compiler) + ';' + $env:PATH
New-Item -ItemType Directory -Force 'dist' | Out-Null
$resourceCompiler = Join-Path (Split-Path -Parent $compiler) 'windres.exe'
& $resourceCompiler '-Iresources' 'resources\Emberfall.rc' '-O' 'coff' '-o' 'dist\resources.o'
if ($LASTEXITCODE -ne 0) { throw 'Kompilacja manifestu DPI nie powiodła się.' }
& $compiler '-std=c++17' '-O2' '-DNDEBUG' '-Wall' '-Wextra' '-Wno-misleading-indentation' '-Wno-narrowing' 'src\main.cpp' 'src\sync_present.cpp' 'dist\resources.o' "-I$raylib\include" "-L$raylib\lib" '-lraylib' '-lopengl32' '-lgdi32' '-lwinmm' '-ldwmapi' '-ld3d11' '-ldxgi' '-ld3dcompiler' '-Wl,--wrap=glfwSwapBuffers' '-static' '-mwindows' '-o' 'dist\Emberfall.exe'
if ($LASTEXITCODE -ne 0) { throw 'Kompilacja gry nie powiodła się.' }
# Keep identical assets untouched, including PNGs held by a preview window.
$sourceAssetRoot = Join-Path $PSScriptRoot 'assets'
$targetAssetRoot = Join-Path $PSScriptRoot 'dist\assets'
foreach ($sourceAsset in Get-ChildItem -LiteralPath $sourceAssetRoot -File -Recurse) {
    $relativeAsset = $sourceAsset.FullName.Substring($sourceAssetRoot.Length + 1)
    $targetAsset = Join-Path $targetAssetRoot $relativeAsset
    if ((Test-Path -LiteralPath $targetAsset) -and
        (Get-FileHash -LiteralPath $sourceAsset.FullName).Hash -eq (Get-FileHash -LiteralPath $targetAsset).Hash) { continue }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $targetAsset) | Out-Null
    Copy-Item -LiteralPath $sourceAsset.FullName -Destination $targetAsset -Force
}
Copy-Item -LiteralPath 'README.md' -Destination 'dist\README.md' -Force
Copy-Item -LiteralPath 'LOOT.md' -Destination 'dist\LOOT.md' -Force
Copy-Item -LiteralPath 'LICENSES.md' -Destination 'dist\LICENSES.md' -Force
if ($Test) {
    & $compiler '-std=c++17' '-O2' '-Wall' '-Wextra' '-Wno-misleading-indentation' '-static' 'tests\game_tests.cpp' '-o' 'dist\game_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Kompilacja testów nie powiodła się.' }
    & '.\dist\game_tests.exe' '--no-audio'
    if ($LASTEXITCODE -ne 0) { throw 'Testy logiki gry nie przeszły.' }
}
Write-Host 'Gotowe: dist\Emberfall.exe'

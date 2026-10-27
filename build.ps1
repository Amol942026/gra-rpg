param([switch]$Test, [switch]$Player)
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
$outputDirectory = if ($Player) { 'dist\player' } else { 'dist' }
New-Item -ItemType Directory -Force $outputDirectory | Out-Null
$resourceObject = Join-Path $outputDirectory 'resources.o'
$gameExecutable = Join-Path $outputDirectory 'Emberfall.exe'
$resourceCompiler = Join-Path (Split-Path -Parent $compiler) 'windres.exe'
& $resourceCompiler '-Iresources' 'resources\Emberfall.rc' '-O' 'coff' '-o' $resourceObject
if ($LASTEXITCODE -ne 0) { throw 'Kompilacja manifestu DPI nie powiodła się.' }
$gameArguments = @('-std=c++17', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Wno-misleading-indentation', '-Wno-narrowing', "-I$raylib\include", "-L$raylib\lib", '-lraylib', '-lopengl32', '-lgdi32', '-lwinmm', '-ldwmapi', '-ld3d11', '-ldxgi', '-ld3dcompiler', '-Wl,--wrap=glfwSwapBuffers', '-static')
if ($Player) { $gameArguments += '-DEMBERFALL_PLAYER_BUILD=1' }
& $compiler 'src\main.cpp' 'src\sync_present.cpp' $resourceObject @gameArguments '-mwindows' '-o' $gameExecutable
if ($LASTEXITCODE -ne 0) { throw 'Kompilacja gry nie powiodła się.' }
# Keep identical assets untouched, including PNGs held by a preview window.
$sourceAssetRoot = Join-Path $PSScriptRoot 'assets'
$targetAssetRoot = Join-Path (Join-Path $PSScriptRoot $outputDirectory) 'assets'
foreach ($sourceAsset in Get-ChildItem -LiteralPath $sourceAssetRoot -File -Recurse) {
    $relativeAsset = $sourceAsset.FullName.Substring($sourceAssetRoot.Length + 1)
    $targetAsset = Join-Path $targetAssetRoot $relativeAsset
    if ((Test-Path -LiteralPath $targetAsset) -and
        (Get-FileHash -LiteralPath $sourceAsset.FullName).Hash -eq (Get-FileHash -LiteralPath $targetAsset).Hash) { continue }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $targetAsset) | Out-Null
    Copy-Item -LiteralPath $sourceAsset.FullName -Destination $targetAsset -Force
}
if (!$Player) {
    # Local developer launchers point to the adjacent EXE.
    foreach ($launcher in @('Tryb_deweloperski.bat','Test_wiezy.bat','Test_bossa.bat','Test_pustyni.bat')) {
        $launcherText = [IO.File]::ReadAllText((Join-Path $PSScriptRoot $launcher)).Replace('"dist\Emberfall.exe"','"Emberfall.exe"')
        [IO.File]::WriteAllText((Join-Path $PSScriptRoot ('dist/' + $launcher)), $launcherText, [Text.Encoding]::ASCII)
    }
}
# Like assets, identical documents may be mapped by an open preview. Leave them
# in place instead of needlessly rewriting the same bytes and failing the build.
$documents = @('LOOT.md','LICENSES.md')
if (!$Player) { $documents += 'README.md' }
foreach ($documentName in $documents) {
    $targetDocument = Join-Path $outputDirectory $documentName
    if ((Test-Path -LiteralPath $targetDocument) -and
        (Get-FileHash -LiteralPath $documentName).Hash -eq (Get-FileHash -LiteralPath $targetDocument).Hash) { continue }
    Copy-Item -LiteralPath $documentName -Destination $targetDocument -Force
}
if ($Test) {
    & $compiler '-std=c++17' '-O2' '-Wall' '-Wextra' '-Wno-misleading-indentation' '-static' 'tests\game_tests.cpp' '-o' 'dist\game_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Kompilacja testów nie powiodła się.' }
    & '.\dist\game_tests.exe' '--no-audio'
    if ($LASTEXITCODE -ne 0) { throw 'Testy logiki gry nie przeszły.' }
    & $compiler '-std=c++17' '-O2' '-Wall' '-Wextra' '-Wno-misleading-indentation' '-static' 'tests\save_game_tests.cpp' '-o' 'dist\save_game_tests.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Kompilacja testów zapisu nie powiodła się.' }
    & '.\dist\save_game_tests.exe' '--no-audio'
    if ($LASTEXITCODE -ne 0) { throw 'Testy zapisu i odczytu nie przeszły.' }
}
if ($Player -and $Test) {
    $qaExecutable = Join-Path $outputDirectory 'player_build_qa.exe'
    & $compiler 'tests\player_build_qa.cpp' 'src\sync_present.cpp' $resourceObject @gameArguments '-o' $qaExecutable
    if ($LASTEXITCODE -ne 0) { throw 'Kompilacja testu wersji gracza nie powiodła się.' }
    & (Join-Path $PSScriptRoot $qaExecutable) '--no-audio'
    if ($LASTEXITCODE -ne 0) { throw 'Test wersji gracza nie przeszedł.' }
}
Write-Host ('Gotowe: ' + $gameExecutable)

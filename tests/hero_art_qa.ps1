$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$compilerDirectory = Join-Path $PWD 'tools\w64devkit\bin'
$env:PATH = $compilerDirectory + ';' + $env:PATH
$raylibDirectory = Join-Path $PWD 'external\raylib-5.5_win64_mingw-w64'
$arguments = @('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory\include", "-L$raylibDirectory\lib", '-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& (Join-Path $compilerDirectory 'g++.exe') 'tests\hero_art_qa.cpp' 'src\sync_present.cpp' @arguments '-o' 'dist\hero_art_qa.exe'
if ($LASTEXITCODE -ne 0) { throw 'Hero renderer test build failed.' }
$heroQa = Start-Process -FilePath (Join-Path $PWD 'dist\hero_art_qa.exe') -ArgumentList '--no-audio' -WindowStyle Hidden -PassThru -Wait
if ($heroQa.ExitCode -ne 0) { throw 'Hero renderer tests failed. See dist/qa-hero-art/render-tests.txt.' }

$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$compilerDirectory = Join-Path $PWD 'tools\w64devkit\bin'
$env:PATH = $compilerDirectory + ';' + $env:PATH
$raylibDirectory = Join-Path $PWD 'external\raylib-5.5_win64_mingw-w64'
$arguments = @('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory\include", "-L$raylibDirectory\lib", '-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& (Join-Path $compilerDirectory 'g++.exe') 'tests\treasure_qa.cpp' 'src\sync_present.cpp' @arguments '-o' 'dist\treasure_qa.exe'
if ($LASTEXITCODE -ne 0) { throw 'Treasure UI test build failed.' }
& '.\dist\treasure_qa.exe' '--no-audio'
if ($LASTEXITCODE -ne 0) { throw 'Treasure UI tests failed. See dist/qa-treasure/ui-tests.txt.' }

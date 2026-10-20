$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$compilerDirectory = Join-Path $PWD 'tools\w64devkit\bin'
$env:PATH = $compilerDirectory + ';' + $env:PATH
New-Item -ItemType Directory -Force -Path 'dist\qa-sheath-clearance' | Out-Null
& (Join-Path $compilerDirectory 'g++.exe') '-std=c++17' '-O2' '-static' 'tests\sword_stow_clearance.cpp' '-o' 'dist\qa-sheath-clearance\clearance.exe'
if ($LASTEXITCODE -ne 0) { throw 'Sword swept-geometry test build failed.' }
& '.\dist\qa-sheath-clearance\clearance.exe' '--no-audio' 'dist\qa-sheath-clearance\clearance.json'
if ($LASTEXITCODE -ne 0) { throw 'Sword intersects the padded head or breastplate.' }
$raylibDirectory = Join-Path $PWD 'external\raylib-5.5_win64_mingw-w64'
$arguments = @('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory\include", "-L$raylibDirectory\lib", '-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& (Join-Path $compilerDirectory 'g++.exe') 'tests\sword_stow_qa.cpp' 'src\sync_present.cpp' @arguments '-o' 'dist\sword_stow_qa.exe'
if ($LASTEXITCODE -ne 0) { throw 'Sword stow QA build failed.' }
& '.\dist\sword_stow_qa.exe' '--no-audio'
if ($LASTEXITCODE -ne 0) { throw 'Sword stow QA failed. See dist/qa-sheath/tests.txt.' }

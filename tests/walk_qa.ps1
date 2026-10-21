param([string]$OutputName = 'qa-walk', [string]$ExecutableName = 'walk_qa.exe')
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$compilerDirectory = Join-Path $PWD 'tools\w64devkit\bin'
$env:PATH = $compilerDirectory + ';' + $env:PATH
& (Join-Path $compilerDirectory 'g++.exe') '-std=c++17' '-O2' '-static' 'tests\walk_metrics.cpp' '-o' 'dist\walk_metrics.exe'
if ($LASTEXITCODE -ne 0) { throw 'Walk metrics build failed.' }
& '.\dist\walk_metrics.exe' '--no-audio' (Join-Path 'dist' $OutputName)
if ($LASTEXITCODE -ne 0) { throw 'Walk contact/loop verification failed.' }
$raylibDirectory = Join-Path $PWD 'external\raylib-5.5_win64_mingw-w64'
$arguments = @('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory\include", "-L$raylibDirectory\lib", '-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& (Join-Path $compilerDirectory 'g++.exe') 'tests\walk_qa.cpp' 'src\sync_present.cpp' @arguments '-o' (Join-Path 'dist' $ExecutableName)
if ($LASTEXITCODE -ne 0) { throw 'Walk renderer build failed.' }
& (Join-Path (Join-Path $PWD 'dist') $ExecutableName) '--no-audio' $OutputName
if ($LASTEXITCODE -ne 0) { throw 'Walk renderer QA failed.' }

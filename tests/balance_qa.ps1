param([switch]$NoAudio,[string]$OutputName=('qa-balance-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')))
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Balance QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$root=Join-Path $PWD ('dist/'+$OutputName)
if(Test-Path -LiteralPath $root){throw 'Use a fresh QA folder'}
New-Item -ItemType Directory -Path $root | Out-Null
Copy-Item -LiteralPath 'assets' -Destination (Join-Path $root 'assets') -Recurse
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
$helper=Join-Path $root 'balance_qa.exe'
& (Join-Path $compilerDirectory 'g++.exe') 'tests/balance_qa.cpp' 'src/sync_present.cpp' @arguments '-o' $helper
if($LASTEXITCODE -ne 0){throw 'Balance QA build failed'}
& $helper '--no-audio' 'checks'
if($LASTEXITCODE -ne 0){throw 'Balance QA failed'}

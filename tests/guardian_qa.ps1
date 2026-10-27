param([switch]$NoAudio,[switch]$BuildOnly,[string]$OutputName=('qa-guardian-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')))
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Guardian QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& (Join-Path $compilerDirectory 'g++.exe') 'tests/guardian_qa.cpp' 'src/sync_present.cpp' @arguments '-o' 'dist/guardian_qa.exe'
if($LASTEXITCODE -ne 0){throw 'Guardian renderer compilation failed'}
if(!$BuildOnly){& './dist/guardian_qa.exe' '--no-audio' $OutputName;if($LASTEXITCODE -ne 0){throw 'Guardian renderer QA failed'}}

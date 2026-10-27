param([switch]$NoAudio,[string]$GameRoot,[string]$OutputName='resource-render-timing.json')
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Resource render timing requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
if(!$GameRoot){$GameRoot=Join-Path $PWD 'dist'}
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
$helper=Join-Path $GameRoot 'resource_performance_qa.exe'
& (Join-Path $compilerDirectory 'g++.exe') 'tests/resource_performance_qa.cpp' 'src/sync_present.cpp' @arguments '-o' $helper
if($LASTEXITCODE -ne 0){throw 'Resource timing build failed'}
& $helper '--no-audio' $OutputName
if($LASTEXITCODE -ne 0){throw 'Resource timing failed'}

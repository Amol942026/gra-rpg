param([switch]$NoAudio,[switch]$BuildOnly,[string]$OutputName=('qa-forest-trees-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')))
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Forest tree diagnostics require -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin'
$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& (Join-Path $compilerDirectory 'g++.exe') 'tests/forest_trees_qa.cpp' 'src/sync_present.cpp' @arguments '-o' 'dist/forest_trees_qa.exe'
if($LASTEXITCODE -ne 0){throw 'Forest tree diagnostic build failed'}
if(!$BuildOnly){
 & './dist/forest_trees_qa.exe' '--no-audio' $OutputName
 if($LASTEXITCODE -ne 0){throw 'Forest tree diagnostic failed; inspect checks.txt'}
}

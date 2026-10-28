param([switch]$NoAudio,[string]$OutputName=('qa-performance60-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')),[switch]$Present,[switch]$Quick,[switch]$Dxgi,[switch]$CompileOnly)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Performance QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$root=Join-Path $PWD ('dist/'+$OutputName)
if(Test-Path -LiteralPath $root){throw 'Use a fresh QA folder'}
New-Item -ItemType Directory -Path $root | Out-Null
Copy-Item -LiteralPath 'assets' -Destination (Join-Path $root 'assets') -Recurse
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
$helper=Join-Path $root 'performance60_qa.exe'
& (Join-Path $compilerDirectory 'windres.exe') '-Iresources' 'resources/Emberfall.rc' '-O' 'coff' '-o' (Join-Path $root 'resources.o')
if($LASTEXITCODE -ne 0){throw 'Performance QA DPI resource build failed'}
& (Join-Path $compilerDirectory 'g++.exe') 'tests/performance60_qa.cpp' 'src/sync_present.cpp' (Join-Path $root 'resources.o') @arguments '-o' $helper
if($LASTEXITCODE -ne 0){throw 'Performance QA build failed'}
if($CompileOnly){Write-Host ('Built: '+$helper);return}
$runArguments=@('--no-audio','checks');if($Present){$runArguments+='--present'}
if($Quick){$runArguments+='--quick'};if($Dxgi){$runArguments+='--dxgi'}
& $helper @runArguments
if($LASTEXITCODE -ne 0){throw 'Performance QA failed'}

param([switch]$NoAudio,[string]$OutputName=('qa-volcano-clarity-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')),[string]$ResultName='checks')
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Image clarity probe requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$root=Join-Path $PWD ('dist/'+$OutputName)
if(Test-Path -LiteralPath $root){throw 'Use a fresh probe folder'}
New-Item -ItemType Directory -Path $root | Out-Null
Copy-Item -LiteralPath assets -Destination (Join-Path $root 'assets') -Recurse
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
$helper=Join-Path $root 'image_clarity_probe.exe'
& (Join-Path $compilerDirectory 'windres.exe') '-Iresources' 'resources/Emberfall.rc' '-O' 'coff' '-o' (Join-Path $root 'resources.o')
if($LASTEXITCODE -ne 0){throw 'DPI manifest compilation failed'}
& (Join-Path $compilerDirectory 'g++.exe') 'tests/image_clarity_probe.cpp' 'src/sync_present.cpp' (Join-Path $root 'resources.o') @arguments '-o' $helper
if($LASTEXITCODE -ne 0){throw 'Clarity probe compilation failed'}
$job=Start-Process -FilePath $helper -ArgumentList @('--no-audio',$ResultName,'--background-test') -WorkingDirectory $root -WindowStyle Hidden -PassThru
if(!$job.WaitForExit(180000)){$job.Kill();throw 'Owned hidden clarity probe timed out'}
if($job.ExitCode -ne 0){throw 'Clarity probe failed'}
Get-Content -LiteralPath (Join-Path $root ($ResultName+'/summary.json'))

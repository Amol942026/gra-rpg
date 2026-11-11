param([Parameter(Mandatory)][switch]$NoAudio,[string]$OutputRoot=('C:/AI2D/pipeline/runs/hud-equipment-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')))
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Silent QA requires -NoAudio'}
$repo=Split-Path -Parent $PSScriptRoot;Set-Location -LiteralPath $repo
$qa=[IO.Path]::GetFullPath($OutputRoot)
if(Test-Path -LiteralPath $qa){throw 'Choose a fresh output directory'}
New-Item -ItemType Directory -Path $qa | Out-Null
New-Item -ItemType Junction -Path (Join-Path $qa 'assets') -Target (Join-Path $repo 'assets') | Out-Null
$compiler=Join-Path $repo 'tools/w64devkit/bin/g++.exe';$env:PATH=(Split-Path -Parent $compiler)+';'+$env:PATH
$ray=Join-Path $repo 'external/raylib-5.5_win64_mingw-w64';$exe=Join-Path $qa 'hud_equipment_qa.exe'
$flags=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$ray/include","-L$ray/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& $compiler 'tests/hud_equipment_qa.cpp' 'src/sync_present.cpp' @flags '-o' $exe
if($LASTEXITCODE -ne 0){throw 'HUD/equipment QA build failed'}
$worker=Start-Process -FilePath $exe -ArgumentList '--no-audio',('"{0}"' -f $qa) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $qa 'stdout.log') -RedirectStandardError (Join-Path $qa 'stderr.log')
if(!$worker.WaitForExit(240000)){throw 'Owned hidden QA worker still running; inspect before retry'}
if($worker.ExitCode -ne 0){throw 'HUD/equipment QA failed; inspect checks.txt'}
Get-Content -LiteralPath (Join-Path $qa 'report.json')

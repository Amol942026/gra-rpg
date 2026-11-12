param([Parameter(Mandatory)][switch]$NoAudio,[string]$OutputRoot)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Silent QA requires -NoAudio'}
$repo=Split-Path -Parent $PSScriptRoot;Set-Location -LiteralPath $repo
if(!$OutputRoot){$OutputRoot=Join-Path $repo ('dist/qa-local/loot-all-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss-fff'))}
$qa=[IO.Path]::GetFullPath($OutputRoot)
if(Test-Path -LiteralPath $qa){throw 'Choose a fresh output directory'}
New-Item -ItemType Directory -Path $qa | Out-Null
New-Item -ItemType Junction -Path (Join-Path $qa 'assets') -Target (Join-Path $repo 'assets') | Out-Null
$compiler=Join-Path $repo 'tools/w64devkit/bin/g++.exe';$env:PATH=(Split-Path -Parent $compiler)+';'+$env:PATH
$ray=Join-Path $repo 'external/raylib-5.5_win64_mingw-w64';$exe=Join-Path $qa 'loot_all_qa.exe'
$flags=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$ray/include","-L$ray/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
& $compiler 'tests/loot_all_qa.cpp' 'src/sync_present.cpp' @flags '-o' $exe
if($LASTEXITCODE -ne 0){throw 'Loot UI QA build failed'}
$worker=Start-Process $exe -ArgumentList '--no-audio',('"{0}"' -f $qa) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $qa 'stdout.log') -RedirectStandardError (Join-Path $qa 'stderr.log')
if(!$worker.WaitForExit(180000)){throw 'Owned QA worker is still running; inspect before retrying'}
if($worker.ExitCode -ne 0){throw 'Loot UI QA failed'}
Get-Content -LiteralPath (Join-Path $qa 'ui-verification.json')

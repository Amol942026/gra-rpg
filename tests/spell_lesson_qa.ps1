param([switch]$NoAudio,[string]$OutputName=('qa-eldrin-spells-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')))
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Spell QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$output=Join-Path $PWD ('dist/'+$OutputName)
if(Test-Path -LiteralPath $output){throw 'Use a fresh QA folder'}
New-Item -ItemType Directory -Path $output | Out-Null
Copy-Item -LiteralPath 'assets' -Destination (Join-Path $output 'assets') -Recurse
$compilerRoot=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerRoot+';'+$env:PATH
$raylibRoot=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$helper=Join-Path $output 'spell_lesson_qa.exe'
& (Join-Path $compilerRoot 'g++.exe') 'tests/spell_lesson_qa.cpp' 'src/sync_present.cpp' '-std=c++17' '-O2' '-DNDEBUG' '-Wno-narrowing' "-I$raylibRoot/include" "-L$raylibRoot/lib" '-lraylib' '-lopengl32' '-lgdi32' '-lwinmm' '-ldwmapi' '-ld3d11' '-ldxgi' '-ld3dcompiler' '-Wl,--wrap=glfwSwapBuffers' '-static' '-o' $helper
if($LASTEXITCODE -ne 0){throw 'Spell QA compilation failed'}
$job=Start-Process -FilePath $helper -ArgumentList @('--no-audio','checks','--background-test') -WorkingDirectory $output -WindowStyle Hidden -PassThru
if(!$job.WaitForExit(60000)){throw 'Owned hidden spell QA has not finished'}
if($job.ExitCode -ne 0){throw ('Spell QA failed: '+$job.ExitCode)}
Get-Content -LiteralPath (Join-Path $output 'checks/summary.json')

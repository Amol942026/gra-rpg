param([switch]$NoAudio,[string]$OutputName=('qa-cursor-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')))
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Cursor QA requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$root=Join-Path $PWD ('dist/'+$OutputName)
if(Test-Path -LiteralPath $root){throw 'Use a fresh QA folder'}
New-Item -ItemType Directory -Path $root | Out-Null
$compilerDirectory=Join-Path $PWD 'tools/w64devkit/bin';$env:PATH=$compilerDirectory+';'+$env:PATH
$raylibDirectory=Join-Path $PWD 'external/raylib-5.5_win64_mingw-w64'
$arguments=@('-std=c++17','-O2','-DNDEBUG','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-narrowing',"-I$raylibDirectory/include","-L$raylibDirectory/lib",'-lraylib','-lopengl32','-lgdi32','-lwinmm','-ldwmapi','-ld3d11','-ldxgi','-ld3dcompiler','-Wl,--wrap=glfwSwapBuffers','-static')
$helper=Join-Path $root 'cursor_qa.exe'
$native=Join-Path $root 'native_cursor_qa.exe'
& (Join-Path $compilerDirectory 'g++.exe') '-std=c++17' '-O2' 'tests/native_cursor_qa.cpp' '-static' '-luser32' '-lgdi32' '-o' $native
if($LASTEXITCODE -ne 0){throw 'Native cursor QA compilation failed'}
$nativeJob=Start-Process -FilePath $native -ArgumentList @('--no-audio',('"'+$root+'"'),'--background-test') -WorkingDirectory $root -WindowStyle Hidden -PassThru
if(!$nativeJob.WaitForExit(15000)){$nativeJob.Kill();throw 'Native isolated cursor QA timed out'}
$nativeJob.Refresh()
if($nativeJob.ExitCode -ne 0){throw ('Native cursor QA failed: '+$nativeJob.ExitCode)}
Get-Content -LiteralPath (Join-Path $root 'native-summary.json')
& (Join-Path $compilerDirectory 'windres.exe') '-Iresources' 'resources/Emberfall.rc' '-O' coff '-o' (Join-Path $root 'resources.o')
if($LASTEXITCODE -ne 0){throw 'Cursor manifest compilation failed'}
& (Join-Path $compilerDirectory 'g++.exe') 'tests/game_cursor_qa.cpp' 'src/sync_present.cpp' (Join-Path $root 'resources.o') @arguments '-o' $helper
if($LASTEXITCODE -ne 0){throw 'Cursor QA build failed'}
foreach($mode in @('auto','opengl')){
 $job=Start-Process -FilePath $helper -ArgumentList @('--no-audio',$mode,'--background-test',$mode) -WorkingDirectory $root -WindowStyle Hidden -PassThru
 if(!$job.WaitForExit(45000)){$job.Kill();throw 'Owned silent cursor QA timed out'}
 $job.Refresh()
 if($job.ExitCode -ne 0){throw ('Cursor QA failed: '+$mode+' / '+$job.ExitCode)}
 Get-Content -LiteralPath (Join-Path $root ($mode+'/summary.json'))
}

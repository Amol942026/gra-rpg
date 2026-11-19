param([Parameter(Mandatory)][switch]$NoAudio, [Parameter(Mandatory)][string]$OutputPath,
      [switch]$Baseline, [switch]$NoBuild)
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath (Split-Path $PSScriptRoot -Parent)
$output = [IO.Path]::GetFullPath($OutputPath)
$binaryFolder = Join-Path (Get-Location) 'dist/qa-local/caldera-pair-art'
New-Item -ItemType Directory -Force -Path $output,$binaryFolder | Out-Null
$compiler = (Resolve-Path 'tools/w64devkit/bin').Path
$lib = (Resolve-Path 'external/raylib-5.5_win64_mingw-w64').Path
$exe = Join-Path $binaryFolder 'caldera-pair-art.exe'
if (!$NoBuild) {
    $env:PATH = "$compiler;$env:PATH"
    & "$compiler/g++.exe" -std=c++17 -O2 -ffunction-sections -fdata-sections -DNDEBUG -Wno-narrowing "-I$lib/include" tests/caldera_pair_art_qa.cpp src/sync_present.cpp "-L$lib/lib" -lraylib -lopengl32 -lgdi32 -lwinmm -ldwmapi -ld3d11 -ldxgi -ld3dcompiler '-Wl,--wrap=glfwSwapBuffers,--gc-sections' -static -o $exe 2> "$output/build.log"
    if ($LASTEXITCODE) { Get-Content "$output/build.log" -Tail 20; throw 'Kompilacja testu nie powiodła się.' }
}
$arguments = @('--no-audio','--background-test',('"' + $output + '"'))
if ($Baseline) { $arguments += '--baseline' }
$p = Start-Process $exe -ArgumentList $arguments -WorkingDirectory (Get-Location) -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput "$output/stdout.log" -RedirectStandardError "$output/stderr.log"
if (Test-Path "$output/summary.json") { Get-Content "$output/summary.json" }
if ($p.ExitCode) { Get-Content "$output/checks.txt" | Select-String '^FAIL' | Select-Object -First 25; throw "Test zwrócił $($p.ExitCode)." }

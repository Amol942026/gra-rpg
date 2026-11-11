param(
    [string]$SourceRoot='C:/AI2D/Tools/SpriteStudio-v0.3.3/patched-source',
    [ValidateSet('debug','release')][string]$Profile='release',
    [switch]$Test
)
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$rustRoot='C:/AI2D/Tools/Rust'
$compilerRoot=Join-Path $repo 'tools/w64devkit/bin'
$cargo=Join-Path $rustRoot 'cargo/bin/cargo.exe'
if(!(Test-Path -LiteralPath $cargo)){throw 'Install the scoped Rust GNU toolchain documented in SPRITE_MAKER_FIX.md'}
$saved=@{}
$keys=@('CARGO_HOME','RUSTUP_HOME','PATH','CARGO_TARGET_DIR','CARGO_BUILD_JOBS','RUSTFLAGS','TAURI_CONFIG','CARGO_PROFILE_RELEASE_LTO','CARGO_PROFILE_RELEASE_CODEGEN_UNITS','LIBRARY_PATH')
foreach($key in $keys){$saved[$key]=[Environment]::GetEnvironmentVariable($key,'Process')}
Push-Location -LiteralPath (Join-Path $SourceRoot 'src-tauri')
try {
    $env:CARGO_HOME=Join-Path $rustRoot 'cargo'
    $env:RUSTUP_HOME=Join-Path $rustRoot 'rustup'
    $env:PATH=(Join-Path $rustRoot 'cargo/bin')+';'+$compilerRoot+';'+$env:PATH
    $env:CARGO_TARGET_DIR='C:/AI2D/Tools/SpriteStudio-v0.3.3/rust-target'
    $env:CARGO_BUILD_JOBS='3'
    # w64devkit intentionally omits separate libgcc_eh; Rust ships compatible
    # static GNU runtime libraries in its self-contained toolchain component.
    $env:RUSTFLAGS='-C link-self-contained=yes'
    # Build the headless MCP binary only. The installed GUI is left intact.
    $env:TAURI_CONFIG='{"build":{"frontendDist":null}}'
    $env:CARGO_PROFILE_RELEASE_LTO='false'
    $env:CARGO_PROFILE_RELEASE_CODEGEN_UNITS='8'
    # The compact existing MinGW kit omits this Windows import library.
    # Derive symbols from the local OS DLL; never modify the system library.
    $support='C:/AI2D/Tools/SpriteStudio-v0.3.3/gnu-support'
    New-Item -ItemType Directory -Force -Path $support | Out-Null
    $importLib=Join-Path $support 'libktmw32.a'
    if(!(Test-Path -LiteralPath $importLib)){
        $exports=(& (Join-Path $compilerRoot 'objdump.exe') -p (Join-Path $env:SystemRoot 'System32/KtmW32.dll')) -join "`n"
        if($LASTEXITCODE -ne 0){throw 'Cannot read system DLL export table'}
        $symbols=@([regex]::Matches($exports,'(?m)^\s*\[\s*\d+\]\s+([A-Za-z_]\w*)\s*$') | ForEach-Object {$_.Groups[1].Value})
        if($symbols.Count -lt 20 -or 'CreateTransaction' -notin $symbols){throw 'Unexpected system export table'}
        $def=Join-Path $support 'ktmw32.def'
        [IO.File]::WriteAllText($def,"LIBRARY KtmW32.dll`nEXPORTS`n"+($symbols -join "`n")+"`n",[Text.Encoding]::ASCII)
        & (Join-Path $compilerRoot 'dlltool.exe') -d $def -l $importLib -D KtmW32.dll
        if($LASTEXITCODE -ne 0){throw 'Cannot create the GNU import library'}
    }
    $env:LIBRARY_PATH=$support+';'+$env:LIBRARY_PATH
    $sidecar=Join-Path $PWD 'binaries/ffmpeg-x86_64-pc-windows-gnu.exe'
    if(!(Test-Path -LiteralPath $sidecar)){
        Copy-Item -LiteralPath 'C:/AI2D/Tools/SpriteStudio-v0.3.3/extracted/PFiles/Sprite Studio/ffmpeg.exe' -Destination $sidecar
    }
    if($Test){
        $messages=& $cargo test --locked --lib --no-run --message-format=json
        if($LASTEXITCODE -ne 0){throw 'Native unit-test build failed'}
        $testBinaries=@(foreach($message in $messages){
            $artifact=$message | ConvertFrom-Json
            if($artifact.reason -eq 'compiler-artifact' -and $artifact.profile.test -and $artifact.executable){$artifact.executable}
        })
        if($testBinaries.Count -ne 1){throw 'Expected one owned native lib-test binary'}
        & 'C:/Users/Amol/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' (Join-Path $PSScriptRoot 'embed_sprite_test_manifest.py') $testBinaries[0]
        if($LASTEXITCODE -ne 0){throw 'Cannot embed the test Common Controls manifest'}
        foreach($filter in @('rig::','mcp::')){
            & $cargo test --locked --lib $filter -- --test-threads=2
            if($LASTEXITCODE -ne 0){throw "Native tests failed: $filter"}
        }
    }else{
        $arguments=@('build','--locked','--bin','sprite-studio-mcp')
        if($Profile -eq 'release'){$arguments+='--release'}
        & $cargo @arguments
        if($LASTEXITCODE -ne 0){throw 'Sprite Studio MCP build failed'}
        Get-Item -LiteralPath (Join-Path $env:CARGO_TARGET_DIR "$Profile/sprite-studio-mcp.exe")
    }
}finally{
    Pop-Location
    foreach($key in $keys){[Environment]::SetEnvironmentVariable($key,$saved[$key],'Process')}
}

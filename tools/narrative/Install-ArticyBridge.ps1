# Requires PowerShell 7. Builds only with the official Articy.MDK reference package.
[CmdletBinding()]
param([string]$Sdk = 'C:\AI2D\Tools\articy-dev\dotnet-sdk-10\dotnet.exe')
$ErrorActionPreference = 'Stop'
if ($PSVersionTable.PSVersion.Major -lt 7) { throw 'Use PowerShell 7 (pwsh).' }
$env:DOTNET_CLI_TELEMETRY_OPTOUT = '1'
$env:DOTNET_NOLOGO = '1'
$build = 'C:\AI2D\narrative\articy-plugin-build'
$project = Join-Path $PSScriptRoot 'Emberfall.DialogueBridge\Emberfall.DialogueBridge.csproj'
& $Sdk build $project --configuration Release --source 'C:\AI2D\Tools\articy-dev' -o $build
if ($LASTEXITCODE -ne 0) { throw 'Plugin build failed.' }
$install = Join-Path $env:APPDATA 'Articy Software\articy\4.x\Plugins\Local\Emberfall.DialogueBridge\1.0.1'
if (Test-Path -LiteralPath $install) {
    $backup = 'C:\AI2D\narrative\plugin-backups\' + [guid]::NewGuid().ToString('N')
    New-Item -ItemType Directory -Path $backup -Force | Out-Null
    Copy-Item -LiteralPath $install -Destination $backup -Recurse
}
New-Item -ItemType Directory -Path $install -Force | Out-Null
$files = @('Emberfall.DialogueBridge.dll', 'Emberfall.DialogueBridge.deps.json', 'PluginManifest.xml')
foreach ($name in $files) {
    Copy-Item -LiteralPath (Join-Path $build $name) -Destination (Join-Path $install $name) -Force
}
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = 'C:\AI2D\narrative\Emberfall.DialogueBridge_1.0.1_' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.mdk'
[System.IO.Compression.ZipFile]::CreateFromDirectory($install, $archive)
$hashes = foreach ($name in $files) { Get-FileHash -LiteralPath (Join-Path $install $name) -Algorithm SHA256 | Select-Object Path,Hash }
[pscustomobject]@{ installed=$install; archive=$archive; hashes=$hashes; live_test='run status after background startup' } | ConvertTo-Json -Depth 5

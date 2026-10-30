# Starts a minimized client with the owned project's MDK. Select the local
# project once in the client; the supported standalone OpenProject API is MU only.
[CmdletBinding()]
param([string]$ProjectFile = 'C:\AI2D\narrative\EmberfallDialogues\ProjectInfo.aph')
$ErrorActionPreference = 'Stop'
if ($PSVersionTable.PSVersion.Major -lt 7) { throw 'Use PowerShell 7 (pwsh).' }
$running = Get-CimInstance Win32_Process | Where-Object {
    $_.Name -eq 'dotnet.exe' -and $_.CommandLine -like '*ArticyDraft.dll*'
}
$nativeWindow = Get-Process -Name ArticyDraft -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 }
if ($running -or $nativeWindow) { throw 'articy is already running; inspect status instead of launching another instance.' }
$resolved = (Resolve-Path -LiteralPath $ProjectFile).Path
[xml]$project = Get-Content -LiteralPath $resolved -Raw
if ($project.ProjectInfo.Content.Project.Guid -ne 'cb99e03a-ae90-4443-b79f-7f8da1539c34' -or $resolved -ne 'C:\AI2D\narrative\EmberfallDialogues\ProjectInfo.aph') {
    throw 'This launcher is restricted to the verified local Emberfall project.'
}
$bin = 'C:\Program Files\articy draft X\bin'
$manifest = Join-Path $env:APPDATA 'Articy Software\articy\4.x\Plugins\Local\Emberfall.DialogueBridge\1.0.1\PluginManifest.xml'
if (!(Test-Path -LiteralPath $manifest)) { throw 'Run Install-ArticyBridge.ps1 first.' }
$start = [System.Diagnostics.ProcessStartInfo]::new('C:\Program Files\dotnet\dotnet.exe')
$start.WorkingDirectory = $bin
$start.UseShellExecute = $false
$start.CreateNoWindow = $true
$start.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Minimized
$start.Environment['EMBERFALL_ARTICY_BACKGROUND'] = '1'
foreach ($argument in @((Join-Path $bin 'ArticyDraft.dll'), '-nosplash', '-singleuser', '-log.NoEnvDump', '-api.DebugManifest', $manifest)) {
    $start.ArgumentList.Add($argument)
}
$worker = [System.Diagnostics.Process]::Start($start)
$python = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
$watcher = Join-Path $PSScriptRoot 'articy_background.py'
Start-Process -FilePath $python -ArgumentList @(('"' + $watcher + '"'), '--wait-ready', '180') -WindowStyle Hidden -WorkingDirectory $PSScriptRoot -RedirectStandardOutput ('C:\AI2D\narrative\background-' + $worker.Id + '.json') -RedirectStandardError ('C:\AI2D\narrative\background-' + $worker.Id + '.stderr.log')
[pscustomobject]@{ processId=$worker.Id; project=$resolved; background=$true; connection='verify live status after startup' } | ConvertTo-Json

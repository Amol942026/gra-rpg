[CmdletBinding()]
param(
    [Parameter(Mandatory=$true,Position=0)]
    [ValidateSet('doctor','demo','maps','animate','music','sfx','process-audio','import-pack','bind-audio','restore-audio','loop-music','resume-integration')]
    [string]$Action,
    [Parameter(ValueFromRemainingArguments=$true)][string[]]$PipelineArguments,
    [string]$PythonPath='C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe',
    [switch]$NoAudio
)
$ErrorActionPreference='Stop'
if (!$NoAudio) { throw 'Pass -NoAudio. This workflow never plays audio.' }
if (!(Test-Path -LiteralPath $PythonPath -PathType Leaf)) { throw 'Pass the configured Python interpreter via -PythonPath.' }
& $PythonPath (Join-Path $PSScriptRoot 'media_pipeline.py') '--no-audio' $Action @PipelineArguments
if ($LASTEXITCODE -ne 0) { throw "Media pipeline failed (exit $LASTEXITCODE). Inspect the job logs." }

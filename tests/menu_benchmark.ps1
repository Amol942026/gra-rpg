param([switch]$NoAudio)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Menu release benchmark requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$output=Join-Path $PWD ('dist\qa-menu-benchmark-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $output | Out-Null
$runs=@()
foreach($scenario in @('menu','forest','desert')){
 $flag=if($scenario -eq 'menu'){'--menu-benchmark'}elseif($scenario -eq 'forest'){'--benchmark'}else{'--desert-benchmark'}
 $source=if($scenario -eq 'menu'){'dist\menu-benchmark.txt'}elseif($scenario -eq 'forest'){'dist\benchmark-v43.txt'}else{'dist\desert-benchmark-v43.txt'}
 $started=[DateTime]::UtcNow
 $job=Start-Process -FilePath (Join-Path $PWD 'dist\Emberfall.exe') -ArgumentList $flag,'--windowed','--no-audio' -WindowStyle Hidden -Wait -PassThru
 if($job.ExitCode -ne 0 -or (Get-Item -LiteralPath $source).LastWriteTimeUtc -lt $started){throw "Missing fresh benchmark: $scenario"}
 $report=Get-Content -LiteralPath $source -Raw
 $menuOpen=if($scenario -eq 'menu'){'Main menu open: 1'}else{'Main menu open: 0'}
 foreach($expected in @('Build edition: developer','Frames: 900','Audio device opened: no',$menuOpen)){if(!$report.Contains($expected)){throw "Unexpected benchmark report: $expected"}}
 if($scenario -ne 'menu' -and !$report.Contains("Benchmark scenario: $scenario")){throw 'Wrong world benchmark scenario'}
 Copy-Item -LiteralPath $source -Destination (Join-Path $output "$scenario.txt")
 $fps=[double]([regex]::Match($report,'Average FPS: ([\d.]+)').Groups[1].Value)
 $cpu=[double]([regex]::Match($report,'CPU frame mean \(ms\): ([\d.]+)').Groups[1].Value)
 $p95=[double]([regex]::Match($report,'CPU frame p95 \(ms\): ([\d.]+)').Groups[1].Value)
 $runs += [ordered]@{scenario=$scenario;frames=900;average_fps=$fps;cpu_mean_ms=$cpu;cpu_p95_ms=$p95;audio_device_opened=$false}
}
$result=[ordered]@{status='PASS';exe_sha256=(Get-FileHash -LiteralPath 'dist\Emberfall.exe').Hash.ToLowerInvariant();runs=$runs}
$result | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $output 'benchmarks.json') -Encoding utf8
$result | ConvertTo-Json -Depth 5

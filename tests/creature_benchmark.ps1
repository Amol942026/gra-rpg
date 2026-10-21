param([switch]$NoAudio)
$ErrorActionPreference='Stop'
if(!$NoAudio){throw 'Silent benchmark requires -NoAudio'}
Set-Location -LiteralPath (Split-Path -Parent $PSScriptRoot)
$reports=@()
foreach($scenario in @('forest','desert')){
 $flag=if($scenario -eq 'forest'){'--benchmark'}else{'--desert-benchmark'}
 $source=if($scenario -eq 'forest'){'dist\benchmark-v43.txt'}else{'dist\desert-benchmark-v43.txt'}
 $started=(Get-Date).ToUniversalTime()
 $job=Start-Process -FilePath (Join-Path $PWD 'dist\Emberfall.exe') -ArgumentList $flag,'--windowed','--no-audio' -WindowStyle Hidden -Wait -PassThru
 if($job.ExitCode -ne 0 -or (Get-Item -LiteralPath $source).LastWriteTimeUtc -lt $started){throw "Missing fresh benchmark: $scenario"}
 $content=Get-Content -LiteralPath $source -Raw
 foreach($expected in @('Build edition: developer','Frames: 900',"Benchmark scenario: $scenario",'Audio device opened: no')){if(!$content.Contains($expected)){throw "Unexpected benchmark report: $expected"}}
 Copy-Item -LiteralPath $source -Destination "dist\qa-creatures\$scenario-benchmark.txt" -Force
 $fps=[double]([regex]::Match($content,'Average FPS: ([\d.]+)').Groups[1].Value)
 $cpu=[double]([regex]::Match($content,'CPU frame mean \(ms\): ([\d.]+)').Groups[1].Value)
 $reports += [ordered]@{scenario=$scenario;frames=900;average_fps=$fps;cpu_frame_mean_ms=$cpu;audio_device_opened=$false}
 Write-Output "$scenario : $fps FPS, CPU $cpu ms"
}
[ordered]@{status='PASS';exe_sha256=(Get-FileHash -LiteralPath 'dist\Emberfall.exe').Hash.ToLowerInvariant();runs=$reports} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath 'dist\qa-creatures\benchmarks.json' -Encoding utf8

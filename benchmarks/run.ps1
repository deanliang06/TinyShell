param([ValidateSet('count','output','memory','repeat')][string]$Scenario = 'count')
$ErrorActionPreference = 'Stop'
Set-Location (Split-Path $PSScriptRoot -Parent)
$build = Join-Path $PSScriptRoot 'generated'
New-Item -ItemType Directory -Force $build | Out-Null
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class ResidentMemory {
 [DllImport("psapi.dll", SetLastError=true)] static extern bool QueryWorkingSet(IntPtr process, IntPtr buffer, int size);
 public static long PrivateBytes(IntPtr process) {
  int size=16*1024*1024;
  IntPtr buffer=Marshal.AllocHGlobal(size);
  try {
   if(!QueryWorkingSet(process,buffer,size)) throw new System.ComponentModel.Win32Exception();
   long count=Marshal.ReadIntPtr(buffer).ToInt64(), pages=0;
   for(long i=0;i<count;i++) {
    long flags=Marshal.ReadIntPtr(buffer,checked((int)((i+1)*IntPtr.Size))).ToInt64();
    if((flags & 0x100)==0) pages++;
   }
   return pages*Environment.SystemPageSize;
  } finally { Marshal.FreeHGlobal(buffer); }
 }
}
'@
$counter = Join-Path $build "$Scenario.exe"
$receipt = Join-Path $build 'receipt.csv'
$repeats = if ($Scenario -eq 'repeat') { 10 } else { 1 }
if ($Scenario -eq 'count') {
 & gcc -O2 benchmarks/count.c -o $counter
} else {
 & gcc -O2 "-DWORKLOAD_$($Scenario.ToUpper())" benchmarks/workloads.c -o $counter
}
if ($LASTEXITCODE) { throw 'Counter compilation failed' }
$gate = Join-Path $build 'go'
$done = Join-Path $build 'done'
$native = Join-Path $build 'tinyshell-bench.exe'
# Benchmark-only insertion: wait for a gate, then use the existing worker path.
$source = Get-Content window.c -Raw
$hook = @'
    while (GetFileAttributesA("GATE_PATH") == INVALID_FILE_ATTRIBUTES) Sleep(10);
    HANDLE benchThread = CreateThread(NULL, 0, benchmarkWorker, textHW, 0, NULL);
    if (!benchThread) return 2;
    CloseHandle(benchThread);
'@
$worker = @'
DWORD WINAPI benchmarkWorker(LPVOID arg) {
    for (int run = 0; run < REPEAT_COUNT; ++run) {
        ShellArgs *benchArgs = (ShellArgs *)malloc(sizeof(ShellArgs));
        benchArgs->hwnd = (HWND)arg;
        benchArgs->text = strdup("COUNTER_PATH");
        completeCommand(wConfig->ch, benchArgs->text);
        handleTextUpdate(benchArgs);
    }
    FILE *benchDone = fopen("DONE_PATH", "w");
    if (benchDone) { fputs("complete", benchDone); fclose(benchDone); }
    return 0;
}

'@
$worker = $worker.Replace('REPEAT_COUNT', [string]$repeats).Replace('COUNTER_PATH', $counter.Replace('\','\\'))
$source = $source.Replace('LRESULT CALLBACK InputProc', $worker.Replace('DONE_PATH', $done.Replace('\','\\')) + 'LRESULT CALLBACK InputProc')
$hook = $hook.Replace('GATE_PATH', $gate.Replace('\','\\')).Replace('COUNTER_PATH', $counter.Replace('\','\\')).Replace('DONE_PATH', $done.Replace('\','\\'))
$source = $source.Replace('SetFocus(textHW);', "SetFocus(textHW);`n$hook")
$tempSource = Join-Path $build 'window-bench.c'
[IO.File]::WriteAllText($tempSource, $source)
& gcc -I . -municode -UUNICODE $tempSource memory.c display.c handleText.c -o $native -lgdi32 -lcomctl32 -lshlwapi
if ($LASTEXITCODE) { throw 'Shell compilation failed' }
$psScript = Join-Path $build 'powershell-task.ps1'
@"
while (-not (Test-Path -LiteralPath '$gate')) { Start-Sleep -Milliseconds 10 }
foreach (`$run in 1..$repeats) {
 & '$counter'
 if (`$LASTEXITCODE -ne 0) { throw 'Workload failed' }
}
Set-Content -LiteralPath '$done' -Value complete
Start-Sleep -Seconds 30
"@ | Set-Content $psScript
$rows = @()
foreach ($trial in 1..3) {
 foreach ($shell in @('TinyShell','Windows PowerShell')) {
  Remove-Item -LiteralPath $gate,$done,$receipt -Force -ErrorAction SilentlyContinue
  $info = New-Object System.Diagnostics.ProcessStartInfo
  $info.UseShellExecute = $false
  $info.CreateNoWindow = $true
  $info.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Hidden
  $info.RedirectStandardOutput = $true
  $info.RedirectStandardError = $true
  $info.WorkingDirectory = (Get-Location).Path
  $info.EnvironmentVariables['TINYSHELL_BENCH_RECEIPT'] = $receipt
  if ($shell -eq 'TinyShell') { $info.FileName = $native } else {
   $info.FileName = "$env:SystemRoot\System32\WindowsPowerShell\v1.0\powershell.exe"
   $info.Arguments = '-NoLogo -NoProfile -ExecutionPolicy Bypass -File "' + $psScript + '"'
  }
  $proc = [Diagnostics.Process]::Start($info)
  try {
   Start-Sleep -Seconds 2
   if ($proc.HasExited) { throw "Shell exited: $($proc.StandardError.ReadToEnd())" }
   $idle = [ResidentMemory]::PrivateBytes($proc.Handle)
   $samples = @()
   [IO.File]::WriteAllText($gate,'go')
   $timer = [Diagnostics.Stopwatch]::StartNew()
   do {
    if ($proc.HasExited) { throw "Shell exited unexpectedly: $($proc.StandardError.ReadToEnd())" }
    $samples += [ResidentMemory]::PrivateBytes($proc.Handle)
    Start-Sleep -Milliseconds 20
   } while (-not (Test-Path -LiteralPath $done) -and $timer.Elapsed.TotalSeconds -lt 15)
   if (-not (Test-Path -LiteralPath $done)) { throw 'Task did not finish within 15 seconds' }
   Start-Sleep -Milliseconds 100
   $post = [ResidentMemory]::PrivateBytes($proc.Handle)
   if ($Scenario -ne 'count') {
    $proof = @(Get-Content -LiteralPath $receipt)
    if ($proof.Count -ne $repeats -or @($proof | Where-Object { $_ -ne "$Scenario,ok" }).Count) { throw 'Workload receipt validation failed' }
   }
   $rows += [pscustomobject]@{Scenario=$Scenario;Shell=$shell;Trial=$trial;IdleBytes=$idle;PeakSampledBytes=($samples | Measure-Object -Maximum).Maximum;PostBytes=$post;Samples=$samples.Count;VerifiedRuns=$repeats}
   $rows[-1] | Format-List | Out-Host
  } finally {
   if (-not $proc.HasExited) { $proc.Kill(); $proc.WaitForExit() }
   $proc.Dispose()
  }
 }
}
$resultFile = if ($Scenario -eq 'count') { 'results.csv' } else { "$Scenario-results.csv" }
$rows | Export-Csv (Join-Path $PSScriptRoot $resultFile) -NoTypeInformation
[pscustomobject]@{Date=(Get-Date -Format 'yyyy-MM-dd');OS=[Environment]::OSVersion.VersionString;PowerShell=$PSVersionTable.PSVersion.ToString();Compiler=(& gcc --version | Select-Object -First 1);Metric='QueryWorkingSet resident pages with Shared bit clear; shell process only';Trials=3;SampleIntervalMs=20} | ConvertTo-Json | Set-Content (Join-Path $PSScriptRoot 'environment.json')

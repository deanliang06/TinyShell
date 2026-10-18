$ErrorActionPreference = 'Stop'
$inv = [Globalization.CultureInfo]::InvariantCulture
function N($v) { ([double]$v).ToString('0.00',$inv) }
function Median($rows,$field) { $a=@($rows | ForEach-Object { [double]$_.$field } | Sort-Object); $a[1]/1MB }
$svg = New-Object Text.StringBuilder
$scenarios = @(@('output','memory','repeat') | Where-Object { Test-Path (Join-Path $PSScriptRoot "$_-results.csv") })
if ($scenarios.Count -eq 0) { throw 'No completed additional workload results found' }
$height = 195 + 235*$scenarios.Count
[void]$svg.AppendLine("<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='$height' viewBox='0 0 1000 $height' role='img' aria-labelledby='title desc'><title id='title'>Additional TinyShell memory workloads</title><desc id='desc'>Shell-only private working set, median sampled execution peak from three trials. Linear axes start at zero. Only completed workloads are shown.</desc><rect width='1000' height='$height' rx='24' fill='#0b1220'/><g font-family='Segoe UI,Arial,sans-serif'><text x='48' y='55' fill='#f1f5f9' font-size='28' font-weight='700'>Beyond the counter: output and host memory</text><text x='48' y='87' fill='#a9b8ce' font-size='15'>Shell-only private working set | MiB | median sampled peak, 3 trials</text>")
$index=0
foreach ($scenario in $scenarios) {
 $rows=Import-Csv (Join-Path $PSScriptRoot "$scenario-results.csv")
 $title=switch ($scenario) {output {'100 PACED OUTPUT LINES'} memory {'CHILD TOUCHES 64 MiB'} repeat {'10 SEQUENTIAL EXECUTABLE LAUNCHES'}}
 $y=120+235*$index
 [void]$svg.AppendLine("<rect x='32' y='$y' width='936' height='215' rx='16' fill='#131f32'/><text x='56' y='$($y+32)' fill='#b8c8df' font-size='14' font-weight='700'>$title</text>")
 foreach ($tick in @(0,10,20,30,40)) {
  $x=240+$tick*15
  [void]$svg.AppendLine("<line x1='$x' y1='$($y+50)' x2='$x' y2='$($y+165)' stroke='#29384d'/><text x='$x' y='$($y+190)' text-anchor='middle' fill='#91a4bf' font-size='12'>$tick</text>")
 }
 $i=0
 foreach ($shell in @('TinyShell','Windows PowerShell')) {
  $values=@($rows | Where-Object Shell -eq $shell)
  $peak=Median $values 'PeakSampledBytes'
  $post=Median $values 'PostBytes'
  $by=$y+65+60*$i
  $color=if ($i -eq 0) {'#34d399'} else {'#60a5fa'}
  [void]$svg.AppendLine("<text x='56' y='$($by+23)' fill='#edf3fc' font-size='15'>$shell</text><rect x='240' y='$by' width='$(N ($peak*15))' height='34' rx='6' fill='$color'/><text x='$(N (250+$peak*15))' y='$($by+23)' fill='#edf3fc' font-size='16' font-weight='700'>$(N $peak) MiB</text>")
  Write-Output "$scenario | $shell | peak $(N $peak) | post $(N $post)"
  $i++
 }
 $index++
}
[void]$svg.AppendLine("<text x='48' y='$($height-35)' fill='#a9b8ce' font-size='14'>Excludes child and console-host memory. Lower host overhead does not establish feature parity.</text></g></svg>")
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'extended-memory.svg'),$svg.ToString())

$ErrorActionPreference = 'Stop'
$data = Import-Csv (Join-Path $PSScriptRoot 'results.csv')
$inv = [Globalization.CultureInfo]::InvariantCulture
function Number($value) { ([double]$value).ToString('0.00', $inv) }
$native = @($data | Where-Object Shell -eq 'TinyShell')
$ps = @($data | Where-Object Shell -eq 'Windows PowerShell')
function Median($rows, $field) { $values = @($rows | ForEach-Object { [double]$_.$field } | Sort-Object); $values[1] / 1MB }
$idleNative = Median $native 'IdleBytes'
$idlePS = Median $ps 'IdleBytes'
$peakNative = Median $native 'PeakSampledBytes'
$peakPS = Median $ps 'PeakSampledBytes'
$svg = New-Object Text.StringBuilder
[void]$svg.AppendLine('<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="680" viewBox="0 0 1000 680" role="img" aria-labelledby="title desc"><title id="title">TinyShell versus Windows PowerShell private working set</title><desc id="desc">Median idle and sampled peak memory from three fresh processes per shell. Linear axes start at zero. Shell process only.</desc><rect width="1000" height="680" rx="24" fill="#0b1220"/><g font-family="Segoe UI,Arial,sans-serif"><text x="48" y="55" fill="#f1f5f9" font-size="28" font-weight="700">Small shell. Measured footprint.</text><text x="48" y="87" fill="#a9b8ce" font-size="15">Private working set &#183; MiB &#183; median of 3 trials &#183; lower is smaller</text>')
$panels = @(@{Y=120;Title='01 / AT IDLE';A=$idleNative;B=$idlePS},@{Y=355;Title='02 / DURING EXECUTABLE RUN';A=$peakNative;B=$peakPS})
foreach ($p in $panels) {
 $y = $p.Y
 [void]$svg.AppendLine("<rect x='32' y='$y' width='936' height='215' rx='16' fill='#131f32'/><text x='56' y='$($y+32)' fill='#b8c8df' font-size='14' font-weight='700'>$($p.Title)</text>")
 foreach ($tick in @(0,10,20,30,40)) {
  $x=240+$tick*15
  [void]$svg.AppendLine("<line x1='$x' y1='$($y+50)' x2='$x' y2='$($y+165)' stroke='#29384d'/><text x='$x' y='$($y+190)' text-anchor='middle' fill='#91a4bf' font-size='12'>$tick</text>")
 }
 $bar=0
 foreach ($s in @(@{Name='TinyShell';Value=$p.A;Color='#34d399'},@{Name='Windows PowerShell';Value=$p.B;Color='#60a5fa'})) {
  $by=$y+65+$bar*60
  $width=Number ($s.Value*15)
  $label=Number $s.Value
  $labelX=Number (250+$s.Value*15)
  [void]$svg.AppendLine("<text x='56' y='$($by+23)' fill='#edf3fc' font-size='15'>$($s.Name)</text><rect x='240' y='$by' width='$width' height='34' rx='6' fill='$($s.Color)'/><text x='$labelX' y='$($by+23)' fill='#edf3fc' font-size='16' font-weight='700'>$label MiB</text>")
  $bar++
 }
}
$reduction=Number (100*(1-$peakNative/$peakPS))
[void]$svg.AppendLine("<text x='48' y='615' fill='#34d399' font-size='23' font-weight='700'>$reduction% less private resident memory in this workload</text><text x='48' y='646' fill='#a9b8ce' font-size='13'>1,000,000 internal increments + one output line + 2 s hold. Excludes child and console-host memory.</text></g></svg>")
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'memory.svg'), $svg.ToString())
Write-Output "Idle: $(Number $idleNative) / $(Number $idlePS) MiB; sampled peak: $(Number $peakNative) / $(Number $peakPS) MiB; reduction: $reduction%"

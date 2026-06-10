param(
    [string]$PortName = "COM4",
    [int]$BaudRate = 115200,
    [string]$OutputPath = (Join-Path $PSScriptRoot "event_log.txt")
)

$port = New-Object System.IO.Ports.SerialPort `
    $PortName, $BaudRate, ([System.IO.Ports.Parity]::None), 8, `
    ([System.IO.Ports.StopBits]::One)
$port.NewLine = "`n"
$port.ReadTimeout = 1000
$capturing = $false
$eventLines = New-Object System.Collections.Generic.List[string]

try {
    $port.Open()
    Write-Host "Listening on $PortName. Event dumps will be written to $OutputPath."

    while ($true) {
        try {
            $line = $port.ReadLine().Trim()
        }
        catch [System.TimeoutException] {
            continue
        }

        Write-Host $line

        if ($line.Contains("EVENT_LOG_BEGIN")) {
            $eventLines.Clear()
            $capturing = $true
        }

        if ($capturing) {
            $eventLines.Add($line)
        }

        if ($capturing -and $line.Contains("EVENT_LOG_END")) {
            $eventLines | Set-Content -LiteralPath $OutputPath -Encoding ASCII
            Write-Host "Updated $OutputPath"
            $capturing = $false
        }
    }
}
finally {
    if ($port.IsOpen) {
        $port.Close()
    }
    $port.Dispose()
}

param([string]$Emulator = (Join-Path (Split-Path $PSScriptRoot -Parent) '.tools\Ardens.exe'))
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
& (Join-Path $PSScriptRoot 'Build.ps1') -Mode test
if (!(Test-Path -LiteralPath $Emulator)) { throw 'Pass the path to Ardens.exe using -Emulator.' }
$outputDir = Join-Path $projectRoot 'build\test'
$firmware = Join-Path $outputDir 'ardudino.ino.hex'
$log = Join-Path $outputDir 'emulator.log'
$errors = Join-Path $outputDir 'emulator-error.log'
$testProcess = Start-Process -FilePath $Emulator -ArgumentList @('headless=15000', 'usb=true', "file=`"$firmware`"") -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput $log -RedirectStandardError $errors
if (!$testProcess.WaitForExit(60000)) {
    $testProcess.Kill()
    throw 'Emulator test timeout (60 seconds).'
}
$testProcess.Refresh()
$report = Get-Content -LiteralPath $log -Raw
Write-Host $report
$diagnostics = Get-Content -LiteralPath $errors -Raw
if ($diagnostics) { Write-Host $diagnostics }
if ($testProcess.ExitCode -ne 0 -or $report -notmatch 'RESULT \d+ checks, 0 failures' -or $report -match '(?m)^FAIL ') {
    throw 'Regression tests failed. Inspect build/test/emulator.log.'
}

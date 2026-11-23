param(
    [ValidateSet('normal', 'demo', 'test')]
    [string]$Mode = 'demo'
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$sketchDir = Join-Path $projectRoot 'build\sketch\ardudino'
$outputDir = Join-Path $projectRoot "build\$Mode"
New-Item -ItemType Directory -Force $sketchDir, $outputDir | Out-Null
Get-ChildItem -LiteralPath $projectRoot -File |
    Where-Object { $_.Extension -in '.ino', '.h' } |
    Copy-Item -Destination $sketchDir -Force
New-Item -ItemType Directory -Force (Join-Path $sketchDir 'tests') | Out-Null
Copy-Item (Join-Path $projectRoot 'tests\SelfTest.h') (Join-Path $sketchDir 'tests') -Force
$compileArgs = @('compile', '--fqbn', 'arduino:avr:leonardo', '--build-path', $outputDir)
if ($Mode -eq 'demo') {
    $compileArgs += @('--build-property', 'compiler.cpp.extra_flags=-DARDUDINO_DEMO -DARDUDINO_MINUTE_MS=1000UL')
} elseif ($Mode -eq 'test') {
    $compileArgs += @('--build-property', 'compiler.cpp.extra_flags=-DARDUDINO_SELF_TEST -DARDUDINO_DEMO -DARDUDINO_MINUTE_MS=1000UL')
}
& arduino-cli @compileArgs $sketchDir
if ($LASTEXITCODE -ne 0) { throw 'Arduino compilation failed.' }
Write-Host "Firmware: $outputDir\ardudino.ino.hex"

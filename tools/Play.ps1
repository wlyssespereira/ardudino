param(
    [ValidateSet('normal', 'demo')]
    [string]$Mode = 'demo',
    [string]$Emulator = (Join-Path (Split-Path $PSScriptRoot -Parent) '.tools\ArdensPlayer.exe')
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$firmware = Join-Path $projectRoot "build\$Mode\ardudino.ino.hex"
if (!(Test-Path -LiteralPath $firmware)) { & (Join-Path $PSScriptRoot 'Build.ps1') -Mode $Mode }
if (!(Test-Path -LiteralPath $Emulator)) {
    throw 'Ardens not found. Download it from https://github.com/tiberiusbrown/Ardens/releases and pass -Emulator C:\path\ArdensPlayer.exe'
}
# Ardens persists EEPROM by firmware hash in its own user data directory.
# Normal and demo binaries have different hashes and independent pet state.
Start-Process -FilePath $Emulator -ArgumentList @('size=1024x512', 'palette=retro', 'volume=25', "file=`"$firmware`"") -WorkingDirectory $projectRoot

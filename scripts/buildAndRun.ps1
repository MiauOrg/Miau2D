#Requires -Version 5.1

[CmdletBinding()]
param(
    [ValidateSet('debug', 'release')]
    [string]$Config = 'debug',

    [switch]$Run
)

$ErrorActionPreference = 'Continue'

$root = Split-Path -Parent $PSScriptRoot
$mingwBin = 'C:\msys64\mingw64\bin'

if ((Test-Path $mingwBin) -and ($env:PATH -notlike "*$mingwBin*")) {
    $env:PATH = "$mingwBin;$env:PATH"
}

Push-Location $root
try {
    Write-Host "==> Configuring ($Config)" -ForegroundColor Cyan
    cmake --preset $Config
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE." }

    Write-Host "==> Building ($Config)" -ForegroundColor Cyan
    cmake --build --preset $Config
    if ($LASTEXITCODE -ne 0) { throw "Build failed with exit code $LASTEXITCODE." }

    $exe = Join-Path $root "build\$Config\bin\Hesan2D.exe"
    Write-Host "==> Built $exe" -ForegroundColor Green

    if ($Run) {
        Write-Host "==> Running" -ForegroundColor Cyan
        & $exe
    }
}
finally {
    Pop-Location
}


.\build\debug\bin\Hesan2D.exe
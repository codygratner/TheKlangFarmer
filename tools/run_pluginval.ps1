<#
.SYNOPSIS
    Portable Headless pluginval CLI Runner for The Klang Suite.

.DESCRIPTION
    Validates deployed VST3 plugin bundles using Tracktion's pluginval test harness.
    Checks default system VST3 locations with configurable strictness levels.

.PARAMETER Strictness
    The pluginval strictness level (1 to 10). Default is 5.

.PARAMETER PluginvalPath
    Explicit path to pluginval.exe if not located in tools\ or system PATH.

.EXAMPLE
    .\tools\run_pluginval.ps1 -Strictness 5
#>

[CmdletBinding()]
param (
    [int]$Strictness = 5,
    [string]$PluginvalPath = "",
    [string[]]$Plugins = @(
        "C:\Program Files\Common Files\VST3\The Klang Farmer.vst3",
        "C:\Program Files\Common Files\VST3\The Klang Planter.vst3"
    )
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "THE KLANG SUITE - HEADLESS PLUGINVAL RUNNER" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Strictness Level: $Strictness"

# Resolve pluginval binary location
$resolvedExe = $null

if ($PluginvalPath -and (Test-Path $PluginvalPath)) {
    $resolvedExe = (Resolve-Path $PluginvalPath).Path
} elseif (Test-Path "$PSScriptRoot\pluginval.exe") {
    $resolvedExe = "$PSScriptRoot\pluginval.exe"
} elseif (Test-Path "$PSScriptRoot\pluginval\pluginval.exe") {
    $resolvedExe = "$PSScriptRoot\pluginval\pluginval.exe"
} else {
    $cmd = Get-Command "pluginval.exe" -ErrorAction SilentlyContinue
    if ($cmd) {
        $resolvedExe = $cmd.Source
    }
}

if (-not $resolvedExe) {
    Write-Host ""
    Write-Host "[NOTICE] 'pluginval.exe' was not detected in tools\ or system PATH." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To install pluginval on Windows:"
    Write-Host "1. Download the latest release from Tracktion:"
    Write-Host "   https://github.com/Tracktion/pluginval/releases" -ForegroundColor Green
    Write-Host "2. Extract 'pluginval.exe' into 'tools\pluginval.exe' or add its directory to your PATH."
    Write-Host ""
    Write-Host "Automated PowerShell install command:"
    Write-Host '  Invoke-WebRequest -Uri "https://github.com/Tracktion/pluginval/releases/latest/download/pluginval_Windows.zip" -OutFile "tools\pluginval.zip"' -ForegroundColor Gray
    Write-Host '  Expand-Archive -Path "tools\pluginval.zip" -DestinationPath "tools\pluginval" -Force' -ForegroundColor Gray
    Write-Host ""
    exit 0
}

Write-Host "Found pluginval: $resolvedExe" -ForegroundColor Green
Write-Host ""

$failedCount = 0
$passedCount = 0

foreach ($plugin in $Plugins) {
    Write-Host "----------------------------------------"
    Write-Host "Testing Plugin: $plugin" -ForegroundColor White

    if (-not (Test-Path $plugin)) {
        Write-Host "  [SKIP] Target plugin binary not found at '$plugin'." -ForegroundColor Yellow
        Write-Host "  Please run 'deploy.ps1' or build Release VST3 targets before validating."
        continue
    }

    Write-Host "  Executing: $resolvedExe --validate --strictness-level $Strictness --validate-in-process `"$plugin`"" -ForegroundColor Gray
    
    $proc = Start-Process -FilePath $resolvedExe `
                          -ArgumentList @("--validate", "--strictness-level", "$Strictness", "--validate-in-process", "$plugin") `
                          -NoNewWindow -PassThru -Wait

    if ($proc.ExitCode -eq 0) {
        Write-Host "  [PASS] $plugin passed pluginval validation!" -ForegroundColor Green
        $passedCount++
    } else {
        Write-Host "  [FAIL] $plugin failed validation with exit code $($proc.ExitCode)!" -ForegroundColor Red
        $failedCount++
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "PLUGINVAL SUMMARY: Passed: $passedCount | Failed: $failedCount"
Write-Host "========================================" -ForegroundColor Cyan

if ($failedCount -gt 0) {
    exit 1
}
exit 0

param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "verify-format.ps1")
& (Join-Path $PSScriptRoot "verify-abi.ps1")
& (Join-Path $PSScriptRoot "test-headless.ps1") -Configuration $Configuration
& (Join-Path $PSScriptRoot "package-runtime.ps1") -Configuration $Configuration

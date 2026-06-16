param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    $preset = if ($Configuration -eq "Release") { "windows-msvc-release" } else { "windows-msvc-debug" }

    & (Join-Path $PSScriptRoot "test-headless.ps1") -Configuration $Configuration
    ctest --preset $preset -L window
    & (Join-Path $PSScriptRoot "run-sample.ps1") -Configuration $Configuration
    & (Join-Path $PSScriptRoot "run-template.ps1") -Configuration $Configuration
}
finally {
    Pop-Location
}

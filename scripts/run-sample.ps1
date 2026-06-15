param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$sampleExe = Join-Path $repoRoot "build/sample-game-cpp/$Configuration/zeno_sample_game_cpp.exe"

if (-not (Test-Path $sampleExe)) {
    & (Join-Path $PSScriptRoot "build-all.ps1") -Configuration $Configuration
}

Push-Location $repoRoot
try {
    & $sampleExe
}
finally {
    Pop-Location
}

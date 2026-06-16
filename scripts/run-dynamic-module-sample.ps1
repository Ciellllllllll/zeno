param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$preset = if ($Configuration -eq "Release") { "windows-msvc-release" } else { "windows-msvc-debug" }
$sampleExe = Join-Path $repoRoot "build/$preset/bin/$Configuration/zeno_dynamic_module_sample.exe"

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

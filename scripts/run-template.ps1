param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$preset = if ($Configuration -eq "Release") { "windows-msvc-release" } else { "windows-msvc-debug" }
$templateExe = Join-Path $repoRoot "build/$preset/bin/$Configuration/zeno_template_game_cpp.exe"

if (-not (Test-Path $templateExe)) {
    & (Join-Path $PSScriptRoot "build-all.ps1") -Configuration $Configuration
}

Push-Location $repoRoot
try {
    & $templateExe
}
finally {
    Pop-Location
}

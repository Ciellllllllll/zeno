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
    $cargoProfileArgs = if ($Configuration -eq "Release") { @("--release") } else { @() }

    cargo build -p zeno_abi @cargoProfileArgs
    cargo test --workspace

    cmake --preset $preset
    cmake --build --preset $preset
    ctest --preset $preset -E "window|sample|manual"
}
finally {
    Pop-Location
}

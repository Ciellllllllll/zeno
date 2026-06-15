param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    cargo build -p zeno_abi
    cargo test

    cmake -S native -B build/native
    cmake --build build/native --config $Configuration

    cmake -S sdk/cpp -B build/sdk-cpp
    cmake --build build/sdk-cpp --config $Configuration

    cmake -S samples/sample_game_cpp -B build/sample-game-cpp
    cmake --build build/sample-game-cpp --config $Configuration
}
finally {
    Pop-Location
}

param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    $packageDir = Join-Path $repoRoot "build/package-sdk/ZenoEngine-SDK-v0.1.0-dev"
    $externalBuildDir = Join-Path $repoRoot "build/external-game"

    & (Join-Path $PSScriptRoot "package-sdk.ps1") -Configuration All

    if (Test-Path $externalBuildDir) {
        Remove-Item -LiteralPath $externalBuildDir -Recurse -Force
    }

    cmake -S examples/external-game -B $externalBuildDir -DZenoEngine_DIR="$packageDir/cmake"
    cmake --build $externalBuildDir --config $Configuration

    $externalExe = Join-Path $externalBuildDir "$Configuration/zeno_external_game.exe"
    if (Test-Path $externalExe) {
        & $externalExe
    } else {
        & (Join-Path $externalBuildDir "zeno_external_game.exe")
    }
}
finally {
    Pop-Location
}

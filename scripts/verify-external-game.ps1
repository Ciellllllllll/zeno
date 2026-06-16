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
    $packageDir = Join-Path $repoRoot "build/package-sdk/$preset"
    $externalBuildDir = Join-Path $repoRoot "build/external-game"

    & (Join-Path $PSScriptRoot "package-sdk.ps1") -Configuration $Configuration

    if (Test-Path $externalBuildDir) {
        Remove-Item -LiteralPath $externalBuildDir -Recurse -Force
    }

    cmake -S examples/external-game -B $externalBuildDir -DZENO_DIR="$packageDir/cmake"
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

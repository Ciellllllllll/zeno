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
    $cargoProfileDir = if ($Configuration -eq "Release") { "release" } else { "debug" }
    $buildDir = Join-Path $repoRoot "build/$preset"
    $packageDir = Join-Path $repoRoot "build/package/$preset"
    $packageBinDir = Join-Path $packageDir "bin"
    $templateBinDir = Join-Path $packageBinDir "template-game"

    cargo build -p zeno_abi @cargoProfileArgs
    cmake --preset $preset
    cmake --build --preset $preset

    if (Test-Path $packageDir) {
        Remove-Item -LiteralPath $packageDir -Recurse -Force
    }

    cmake --install $buildDir --prefix $packageDir --config $Configuration

    New-Item -ItemType Directory -Force -Path $packageBinDir | Out-Null
    Copy-Item `
        -LiteralPath (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll") `
        -Destination (Join-Path $packageBinDir "zeno_abi.dll") `
        -Force
    if (Test-Path $templateBinDir) {
        Copy-Item `
            -LiteralPath (Join-Path $repoRoot "target/$cargoProfileDir/zeno_abi.dll") `
            -Destination (Join-Path $templateBinDir "zeno_abi.dll") `
            -Force
    }

    Write-Host "Packaged runtime to $packageDir"
}
finally {
    Pop-Location
}

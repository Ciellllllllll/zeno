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

    $runtimeDir = Join-Path $repoRoot "build/$preset/bin/$Configuration"
    & (Join-Path $runtimeDir "zeno_shader_smoke.exe")
    & (Join-Path $runtimeDir "zeno_texture_smoke.exe")
    & (Join-Path $runtimeDir "zeno_mesh_smoke.exe")
    & (Join-Path $runtimeDir "zeno_material_smoke.exe")
    & (Join-Path $runtimeDir "zeno_audio_smoke.exe")
    & (Join-Path $runtimeDir "zeno_debug_draw_smoke.exe")
    & (Join-Path $runtimeDir "zeno_sdk_app_smoke.exe")
    & (Join-Path $runtimeDir "zeno_sdk_collision_smoke.exe")
    & (Join-Path $runtimeDir "zeno_sdk_audio_smoke.exe")
    & (Join-Path $runtimeDir "zeno_sdk_asset_smoke.exe")
    & (Join-Path $runtimeDir "zeno_sdk_scene_smoke.exe")
    & (Join-Path $runtimeDir "zeno_sdk_serialization_smoke.exe")
}
finally {
    Pop-Location
}

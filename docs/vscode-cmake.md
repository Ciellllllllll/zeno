# VS Code And CMake SDK Use

VS Code is an optional editor path. Use rust-analyzer for Rust sources and CMake Tools for C++.

## Build The SDK Package

From the repository root:

```powershell
.\scripts\package-sdk.ps1
```

This creates both Debug and Release SDK artifacts by default.

## Configure A Template Against The SDK

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-dev).Path
cmake -S "$sdkRoot\templates\cpp_empty" -B build\cpp-empty-from-sdk -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build\cpp-empty-from-sdk --config Debug
```

The same settings can be entered in CMake Tools by setting `ZenoEngine_DIR` to:

```text
<sdk-root>/cmake
```

## Configure Packaged Samples

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-dev).Path
cmake -S "$sdkRoot\samples\sdk_feature_samples_cpp" -B build\sdk-feature-samples -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build\sdk-feature-samples --config Debug
```

## Repository Development

For in-repository work, use the existing presets:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

`scripts/build-all.ps1` remains the compact local path for Cargo build/test, CMake configure/build, and non-window smoke tests.

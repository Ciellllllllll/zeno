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
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1).Path
cmake --preset windows-msvc-debug -S "$sdkRoot\templates\cpp_empty"
cmake --build "$sdkRoot\templates\cpp_empty\build\windows-msvc-debug" --config Debug
cmake --preset windows-msvc-release -S "$sdkRoot\templates\cpp_empty"
cmake --build "$sdkRoot\templates\cpp_empty\build\windows-msvc-release" --config Release
```

In CMake Tools, open the packaged `templates/cpp_empty` folder and select `windows-msvc-debug` or `windows-msvc-release`. The packaged presets set `ZenoEngine_DIR` to the SDK `cmake` directory.

## Configure Packaged Samples

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1).Path
cmake --preset windows-msvc-debug -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-debug" --config Debug
cmake --preset windows-msvc-release -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-release" --config Release
```

In CMake Tools, open the packaged `samples/sdk_feature_samples_cpp` folder and select the same Debug or Release presets. Running the sample executables is a GUI validation step.

## Repository Development

For in-repository work, use the existing presets:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
```

`scripts/build-all.ps1` remains the compact local path for Cargo build/test, CMake configure/build, and non-window smoke tests.

# Getting Started With ZENO SDK

This guide starts from a packaged SDK, not from the engine repository internals.

## Requirements

- Windows 10 or Windows 11.
- Visual Studio 2022 17.7 or newer with MSVC v143 and a Windows SDK.
- CMake 3.24 or newer on `PATH`.
- Rust is required to build the SDK package from the repository. SDK consumers only need the packaged headers, libraries, DLL, and CMake config.

## Create The SDK Package

From the repository root:

```powershell
.\scripts\package-sdk.ps1
```

This creates:

```text
build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1/
build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1.zip
```

Use `-Configuration Debug` or `-Configuration Release` to create only one configuration during local verification.

## Use The SDK From CMake

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1).Path
cmake -S "$sdkRoot\templates\cpp_empty" -B build\cpp-empty-from-sdk -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build\cpp-empty-from-sdk --config Debug
.\build\cpp-empty-from-sdk\Debug\zeno_cpp_empty.exe
```

The packaged template also includes Debug and Release presets:

```powershell
cmake --preset windows-msvc-debug -S "$sdkRoot\templates\cpp_empty"
cmake --build "$sdkRoot\templates\cpp_empty\build\windows-msvc-debug" --config Debug
& "$sdkRoot\templates\cpp_empty\build\windows-msvc-debug\Debug\zeno_cpp_empty.exe"
cmake --preset windows-msvc-release -S "$sdkRoot\templates\cpp_empty"
cmake --build "$sdkRoot\templates\cpp_empty\build\windows-msvc-release" --config Release
& "$sdkRoot\templates\cpp_empty\build\windows-msvc-release\Release\zeno_cpp_empty.exe"
```

External CMake projects should use:

```cmake
find_package(ZenoEngine CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE ZenoEngine::zeno_sdk_cpp)
```

Copy `zeno_abi.dll` beside the executable before running. The packaged template does this with `${ZenoEngine_BIN_DIR}/$<CONFIG>/zeno_abi.dll`.

## Run Packaged Samples

After packaging:

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1).Path
cmake -S "$sdkRoot\samples\sdk_feature_samples_cpp" -B build\sdk-feature-samples -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build\sdk-feature-samples --config Debug
```

Or use the packaged presets:

```powershell
cmake --preset windows-msvc-debug -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-debug" --config Debug
cmake --preset windows-msvc-release -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-release" --config Release
```

The sample executables are windowed. Run them only when opening local windows is acceptable:

```powershell
.\build\sdk-feature-samples\Debug\zeno_sample_2d_input_audio_cpp.exe
.\build\sdk-feature-samples\Debug\zeno_sample_3d_mesh_cpp.exe
```

Packaged sample builds copy both `zeno_abi.dll` and the required `assets/` directory beside each executable. The samples load projects, scenes, audio, textures, and the manifest from that executable-relative `assets/` directory, not from the shell working directory.

## Verify Headless External Use

```powershell
.\scripts\verify-external-game.ps1 -Configuration Debug
.\scripts\verify-external-game.ps1 -Configuration Release
```

This packages the SDK, configures `examples/external-game` against the package config, builds it, and runs one headless frame.

For full SDK package consumption QA, including ZIP extraction, packaged template Debug/Release run, packaged sample Debug/Release build, external-game Debug/Release run, DLL placement, asset placement, and private header checks:

```powershell
.\scripts\verify-sdk-package-consumption.ps1
```

This QA path also validates the packaged sample asset inventory in the extracted SDK and in the Debug/Release sample runtime output directories.

## Next Steps

- [SDK API Reference](api/index.md) explains public SDK concepts.
- [SDK Tutorials](tutorials/index.md) walks through the packaged template and focused samples.
- [SDK Layout](sdk-layout.md) lists the files included in the package and ZIP.

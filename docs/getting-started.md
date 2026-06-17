# Getting Started With ZENO SDK

This guide starts from a packaged SDK, not from the engine repository internals.

## Requirements

- Windows 10 or Windows 11.
- Visual Studio 2022 with MSVC v143 and a Windows SDK.
- CMake 3.24 or newer on `PATH`.
- Rust is required to build the SDK package from the repository. SDK consumers only need the packaged headers, libraries, DLL, and CMake config.

## Create The SDK Package

From the repository root:

```powershell
.\scripts\package-sdk.ps1
```

This creates:

```text
build/package-sdk/ZenoEngine-SDK-v0.1.0-dev/
build/package-sdk/ZenoEngine-SDK-v0.1.0-dev.zip
```

Use `-Configuration Debug` or `-Configuration Release` to create only one configuration during local verification.

## Use The SDK From CMake

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-dev).Path
cmake -S templates\cpp_empty -B build\cpp-empty-from-sdk -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build\cpp-empty-from-sdk --config Debug
.\build\cpp-empty-from-sdk\Debug\zeno_cpp_empty.exe
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
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-dev).Path
cmake -S "$sdkRoot\samples\sdk_feature_samples_cpp" -B build\sdk-feature-samples -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build\sdk-feature-samples --config Debug
```

The sample executables are windowed. Run them only when opening local windows is acceptable:

```powershell
.\build\sdk-feature-samples\Debug\zeno_sample_2d_input_audio_cpp.exe
.\build\sdk-feature-samples\Debug\zeno_sample_3d_mesh_cpp.exe
```

## Verify Headless External Use

```powershell
.\scripts\verify-external-game.ps1 -Configuration Debug
```

This packages the SDK, configures `examples/external-game` against the package config, builds it, and runs one headless frame.

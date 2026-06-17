# SDK Layout

The Phase44 SDK package is generated under:

```text
build/package-sdk/ZenoEngine-SDK-v0.1.0-dev/
```

The ZIP uses the same top-level directory name.

## Directory Layout

```text
ZenoEngine-SDK-v0.1.0-dev/
  include/
    zeno/
  lib/
    Debug/
    Release/
  bin/
    Debug/
    Release/
  samples/
    sdk_feature_samples_cpp/
      CMakePresets.json
  templates/
    cpp_empty/
      CMakePresets.json
  docs/
    getting-started.md
    sdk-layout.md
    vs2022.md
    vscode-cmake.md
  cmake/
    ZenoEngineConfig.cmake
  README.md
```

## Include

`include/zeno/` contains the public C++ SDK headers from `sdk/cpp/include/zeno` and the public C ABI headers from `native/zeno_native/include/zeno`.

Allowed public headers include:

- `zeno.hpp`
- `game_module.hpp`
- `game_module_abi.h`
- `math.hpp`
- `zeno_abi.h`
- `zeno_native_backend.h`

Private implementation headers such as `native/zeno_native/src/native_backend.h` are not copied.

## Libraries And Runtime DLLs

`lib/<Config>/` contains:

- `zeno_sdk_cpp.lib`
- `zeno_native.lib`
- `zeno_abi.dll.lib`

`bin/<Config>/` contains:

- `zeno_abi.dll`

`Debug` and `Release` artifacts are intentionally separated. Do not mix Debug and Release libraries in one external build.

## CMake Package

`cmake/ZenoEngineConfig.cmake` defines:

- `ZenoEngine::zeno_sdk_cpp`
- `ZenoEngine::zeno_native`
- `ZenoEngine::zeno_abi_rust`

Compatibility `ZENO::` targets are also defined for older local examples.

## Samples And Templates

`samples/sdk_feature_samples_cpp/` contains the Phase43 focused SDK samples. In the SDK package, their `CMakeLists.txt` consumes the packaged SDK with `find_package(ZenoEngine CONFIG REQUIRED)`.

`templates/cpp_empty/` is the minimal external CMake template. It runs one headless engine frame and copies `zeno_abi.dll` beside the executable.

Both packaged roots include `windows-msvc-debug` and `windows-msvc-release` CMake presets. These presets set `ZenoEngine_DIR` to the SDK `cmake` directory, so CLI, Visual Studio 2022 Open Folder, and VS Code CMake Tools use the same package graph.

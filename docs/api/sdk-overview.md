# SDK Overview

The packaged ZENO SDK is a Windows MSVC C++ SDK. It is meant for external CMake projects that consume the generated package, not for projects that include repository-internal source paths.

## Package Boundary

The SDK package contains:

- `include/zeno/`: public C++ SDK headers and public C ABI headers.
- `lib/Debug/` and `lib/Release/`: MSVC import/static libraries.
- `bin/Debug/` and `bin/Release/`: `zeno_abi.dll` runtime copies.
- `cmake/`: package configuration for `find_package(ZenoEngine CONFIG REQUIRED)`.
- `templates/cpp_empty/`: minimal external CMake project.
- `samples/sdk_feature_samples_cpp/`: focused SDK samples.
- `docs/`: setup, IDE, tutorial, and API documentation.

Do not add repository `sdk/cpp/src`, `native/zeno_native/src`, or other private source directories to an external project include path.

## CMake Target

External projects should consume the package with:

```cmake
find_package(ZenoEngine CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE ZenoEngine::zeno_sdk_cpp)
```

`ZenoEngine::zeno_sdk_cpp` links the public C++ SDK wrapper and its required native/ABI libraries. The packaged template and focused samples also copy the matching `bin/<Config>/zeno_abi.dll` beside each executable after build.

## Results And Diagnostics

Most SDK operations return `zeno::Result`. Check `result.ok()` before using output objects and use `result.message()` for a stable diagnostic string.

The SDK also exposes a lightweight log and last-diagnostic path:

- `zeno::set_log_sink`
- `zeno::clear_log_sink`
- `zeno::last_diagnostic`
- `zeno::clear_last_diagnostic`

These diagnostics are intended for setup, asset, scene, shader, backend, and ABI wrapper failures.

## Ownership Model

The C++ wrappers are RAII types. They are move-only when they own backend or engine handles, and their destructors release owned resources. `zeno::ResourceManager` stores wrapper resources and returns SDK-only IDs such as `TextureId`, `MaterialId`, `MeshId`, `SoundId`, and `TriangleId`.

SDK IDs are not native backend handles. Treat them as lookup keys owned by `ResourceManager`.

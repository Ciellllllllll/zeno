# SDK Layout

The SDK package is generated under:

```text
build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1/
```

The ZIP uses the same top-level directory name.

## Directory Layout

```text
ZenoEngine-SDK-v0.1.0-rc.1/
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
      assets/
        sample_manifest.txt
        project.zproj
        projects/
          2d_input_audio.zproj
          3d_mesh.zproj
        scenes/
          sample_scene.zscene
          2d_input_audio.zscene
          3d_mesh.zscene
        shaders/
          sample_triangle.hlsl
        audio/
          sample_click.wav
        textures/
          sample_sprite_2x2.bmp
  templates/
    cpp_empty/
      CMakePresets.json
  docs/
    getting-started.md
    sdk-layout.md
    release-notes.md
    release-checklist.md
    vs2022.md
    vscode-cmake.md
    api/
    tutorials/
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

`samples/sdk_feature_samples_cpp/` contains the focused SDK samples. In the SDK package, their `CMakeLists.txt` consumes the packaged SDK with `find_package(ZenoEngine CONFIG REQUIRED)`.

The packaged sample asset directory is a merged, source-controlled asset set:

- Shared sample fixtures from `samples/sample_game_cpp/assets/`.
- Focused SDK sample project and scene files from `samples/sdk_feature_samples_cpp/assets/`.

Packaged sample builds copy this directory beside each sample executable as `assets/`. Runtime paths are therefore executable-relative:

- `projects/2d_input_audio.zproj`
- `projects/3d_mesh.zproj`
- `scenes/2d_input_audio.zscene`
- `scenes/3d_mesh.zscene`
- `sample_manifest.txt`
- `audio/sample_click.wav`
- `textures/sample_sprite_2x2.bmp`

The package consumption QA validates this asset inventory in the extracted SDK and again in each Debug/Release sample runtime output directory.

`templates/cpp_empty/` is the minimal external CMake template. It runs one headless engine frame and copies `zeno_abi.dll` beside the executable.

Both packaged roots include `windows-msvc-debug` and `windows-msvc-release` CMake presets. These presets set `ZenoEngine_DIR` to the SDK `cmake` directory, so CLI, Visual Studio 2022 Open Folder, and VS Code CMake Tools use the same package graph.

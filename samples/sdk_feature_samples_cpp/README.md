# ZENO SDK Feature Samples

This directory contains focused C++ game samples that use ZENO only through the public SDK headers.

- `zeno_sample_2d_input_audio_cpp` covers `GameApp`, asset-root loading, texture-backed sprite rendering, keyboard/mouse input, short WAV playback, debug text, and clean module shutdown.
- `zeno_sample_3d_mesh_cpp` covers `GameApp`, runtime scene ownership, SDK mesh/material creation, camera transforms, 3D mesh rendering, debug lines, debug text, and clean module shutdown.

Both targets are included from the repository root `CMakeLists.txt`, so Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds use the same CMake graph and SDK library. The sample code includes only public SDK headers from `sdk/cpp/include/zeno`.

This README describes repository-root builds. In the packaged SDK, this directory is copied under `samples/sdk_feature_samples_cpp/` and uses `find_package(ZenoEngine CONFIG REQUIRED)` through the packaged CMake presets.

## Build

From the repository root:

```powershell
cargo build -p zeno_abi
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug --target zeno_sample_2d_input_audio_cpp
cmake --build --preset windows-msvc-debug --target zeno_sample_3d_mesh_cpp
```

## Run

```powershell
.\build\windows-msvc-debug\bin\Debug\zeno_sample_2d_input_audio_cpp.exe
.\build\windows-msvc-debug\bin\Debug\zeno_sample_3d_mesh_cpp.exe
```

Each target copies the existing public sample texture/audio/shader assets plus the project and scene files in this directory into the executable `assets` directory. For repository Debug builds, the runtime layout is:

```text
build/windows-msvc-debug/bin/Debug/
  zeno_sample_2d_input_audio_cpp.exe
  zeno_abi.dll
  assets/
```

The 2D sample requires:

- `assets/projects/2d_input_audio.zproj`
- `assets/scenes/2d_input_audio.zscene`
- `assets/textures/sample_sprite_2x2.bmp`
- `assets/audio/sample_click.wav`
- `assets/sample_manifest.txt`

The 2D sample opens a 640x360 window, moves one texture-backed sprite with WASD/arrows, plays a short WAV with Space or left mouse, draws debug text/rect helpers, and auto-closes after about 12 seconds. It does not use private engine headers or private native-backend APIs. On failure, the executable prints `2D SDK sample failed: <Result::message()>`; deeper detail is available through SDK diagnostics such as `zeno::last_diagnostic()` and `zeno::set_log_sink`.

The 3D sample requires:

- `assets/projects/3d_mesh.zproj`
- `assets/scenes/3d_mesh.zscene`

The 3D sample opens a 640x360 window, creates a hardcoded color cube mesh through `ResourceManager::create_mesh`, creates a `MaterialKind::mesh_color` material, attaches both IDs to a runtime scene object, renders it with a perspective camera, draws debug line/text helpers, and auto-closes after about 12 seconds. The scene reference is intentionally `builtin:cube`; there is no mesh file importer, animation, editor tooling, lighting redesign, or gameplay sample in this target. On failure, the executable prints `3D SDK sample failed: <Result::message()>`; deeper detail is available through SDK diagnostics such as `zeno::last_diagnostic()` and `zeno::set_log_sink`.

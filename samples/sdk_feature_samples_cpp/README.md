# ZENO SDK Feature Samples

This directory contains focused C++ game samples that use ZENO only through the public SDK headers.

- `zeno_sample_2d_input_audio_cpp` covers `GameApp`, asset-root loading, texture-backed sprite rendering, keyboard/mouse input, short WAV playback, debug text, and clean module shutdown.
- `zeno_sample_3d_mesh_cpp` covers `GameApp`, runtime scene ownership, SDK mesh/material creation, camera transforms, 3D mesh rendering, debug lines, debug text, and clean module shutdown.

Both targets are included from the repository root `CMakeLists.txt`, so Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds use the same CMake graph and SDK library. The sample code includes only public SDK headers from `sdk/cpp/include/zeno`.

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

Each target copies the existing public sample texture/audio/shader assets plus the project and scene files in this directory into the executable `assets` directory. No private engine or native-backend headers are required by the sample code.

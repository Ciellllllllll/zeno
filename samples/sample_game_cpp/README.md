# ZENO Sample Game

This sample is a small C++ game-module demo that runs through the ZENO C++ SDK.

It creates a Win32 window through the SDK/native backend path, initializes the DirectX 11 renderer, calls the sample module lifecycle, reads a keyboard/mouse input snapshot, compiles shader assets, loads a BMP texture asset, creates minimal material handles, clears the screen with a changing color, draws a visible transformed triangle, materialized texture-backed sprite, and materialized basic cube mesh through a perspective camera for a few seconds, and shuts down cleanly.

## Build

From the repository root:

```powershell
cargo build -p zeno_abi
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

The sample target is part of the repository-level CMake preset graph. Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds all use the same `zeno_sample_game_cpp` target.

## Assets

Source assets live in `samples/sample_game_cpp/assets/`. The CMake sample target copies that directory to `$<TARGET_FILE_DIR:zeno_sample_game_cpp>/assets` after build and installs it to `bin/assets`. The sample resolves assets from the executable directory, not from the current working directory.

The current sample reads `sample_manifest.txt` during `on_init` to prove the copied runtime layout is available before rendering. It also compiles `shaders/sample_triangle.hlsl` into vertex and pixel shader handles before creating the triangle resource, loads `textures/sample_sprite_2x2.bmp` into a texture handle for sprite rendering, creates sprite and mesh material handles, and creates a hardcoded cube mesh through the SDK mesh API. Shader compile failures are logged with the stage, asset path, entry point, result text, and bounded compiler output.

## Run

```powershell
.\scripts\run-sample.ps1
```

## Expected Result

A 640x360 window opens. The background clear color changes over time, a colored triangle rendered through asset shader handles rotates through the SDK transform/camera path, a small texture-backed sprite is drawn with an alpha material, a cube mesh rotates with an opaque depth-tested material, then the sample closes after about four seconds. Mouse position influences the background tint, A/Left and D/Right adjust the tint and triangle transform, and Escape requests shutdown. The console logs the native backend lifecycle and sample module init/shutdown calls.

## Limitations

- Rendering is limited to a DirectX 11 clear color, fixed triangle resources with transform/camera matrices and explicit shader handles, a fixed sprite draw path with texture/material handles, and indexed position/color mesh resources with material render state.
- Input is limited to a small keyboard/mouse snapshot.
- There is no gamepad, IME/text editing, rebinding UI, raw input, or cursor capture yet.
- There is no mesh loader, atlas system, font renderer, or asset pipeline yet.
- There is no shader reflection, material graph, or shader hot reload yet.
- The game module is statically linked into the sample executable.

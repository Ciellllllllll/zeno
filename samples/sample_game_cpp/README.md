# ZENO Sample Game

This sample is a small C++ game-module demo that runs through the ZENO C++ SDK `GameApp` runtime.

`GameApp` loads `assets/project.zproj` and `assets/scenes/sample_scene.zscene`, creates a Win32 window through the SDK/native backend path, initializes the Rust runtime, DirectX 11 renderer, and minimal audio path, calls the sample module lifecycle, and reads a keyboard/mouse input snapshot. The module compiles shader assets, loads a BMP texture asset and short PCM WAV effect, creates minimal material handles, organizes object IDs, transforms, and renderable data through the runtime scene, checks sample-owned AABB collision, clears the screen with a changing color, draws a visible transformed triangle, materialized texture-backed sprite, materialized basic cube mesh, and debug collision rectangles through a perspective camera for a few seconds, and shuts down cleanly.

## Build

From the repository root:

```powershell
cargo build -p zeno_abi
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

The sample target is part of the repository-level CMake preset graph. Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds all use the same `zeno_sample_game_cpp` target. The executable `main.cpp` only creates `zeno::GameApp` and runs the static-linked module.

## Assets

Source assets and startup data live in `samples/sample_game_cpp/assets/`. The CMake sample target copies that directory to `$<TARGET_FILE_DIR:zeno_sample_game_cpp>/assets` after build and installs it to `bin/assets`. The sample resolves assets from the executable directory, not from the current working directory.

The host reads `project.zproj` before creating the window. The project file selects the asset root, initial scene path, and window size. The scene file is a strict UTF-8 line-oriented text format with `version=1`, one `[object]` block per object, `position`, `rotation_z`, `scale`, `color`, and a small renderable reference such as `builtin:cube`, `builtin:triangle`, or `textures/sample_sprite_2x2.bmp`.

The current sample reads `sample_manifest.txt` during `on_init` to prove the copied runtime layout is available before rendering. It also compiles `shaders/sample_triangle.hlsl` into vertex and pixel shader handles before creating the triangle resource, loads the scene-referenced sprite texture into a texture handle for sprite rendering, loads `audio/sample_click.wav` into a sound handle, creates sprite and mesh material handles, creates a hardcoded cube mesh through the SDK mesh API, and attaches those resources to scene objects. Shader compile failures are logged with the stage, asset path, entry point, result text, and bounded compiler output.

## Run

```powershell
.\scripts\run-sample.ps1
```

## Expected Result

A 640x360 window opens using the size from `project.zproj`. The background clear color changes over time, a scene-managed colored triangle rendered through asset shader handles rotates through the SDK transform/camera path, a small scene-referenced texture-backed sprite is drawn with an alpha material, a cube mesh rotates with an opaque depth-tested material, then the sample closes after about four seconds. Mouse position influences the background tint, WASD/arrows move the sprite, sprite/triangle AABB overlap changes the sprite color and plays `audio/sample_click.wav`, Space toggles debug collision rectangles, and Escape requests shutdown. The console logs the native backend lifecycle and sample module init/shutdown calls.

## Limitations

- Rendering is limited to a DirectX 11 clear color, fixed triangle resources with transform/camera matrices and explicit shader handles, a fixed sprite draw path with texture/material handles, and indexed position/color mesh resources with material render state.
- The scene layer is component-lite organization for object IDs, transforms, and one renderable per object. It is not a full ECS, hierarchy, prefab, or editor system.
- Collision is sample-owned SDK AABB helper usage. It is not a physics engine, rigid body solver, swept collision implementation, or scene collider component system.
- Debug draw is temporary development visualization for collision bounds.
- Project/scene loading is a strict minimal text format for startup data. It is not an editor save format, binary scene format, prefab system, or asset database.
- Audio is limited to short PCM WAV effects. There is no streaming BGM, spatial audio, mixer graph, or compressed audio decode.
- `GameApp` currently runs static-linked modules only. It is not dynamic plugin loading, hot reload, or scripting.
- Input is limited to a small keyboard/mouse snapshot.
- There is no gamepad, IME/text editing, rebinding UI, raw input, or cursor capture yet.
- There is no mesh loader, atlas system, font renderer, or asset pipeline yet.
- There is no shader reflection, material graph, or shader hot reload yet.
- The game module is statically linked into the sample executable.

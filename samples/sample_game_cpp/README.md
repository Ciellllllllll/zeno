# ZENO Sample Game

This sample is a small C++ game-module demo that runs through the ZENO C++ SDK.

It creates a Win32 window through the SDK/native backend path, initializes the DirectX 11 renderer, calls the sample module lifecycle, reads a keyboard/mouse input snapshot, clears the screen with a changing color, draws a visible transformed triangle through an orthographic camera for a few seconds, and shuts down cleanly.

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

The current sample reads `sample_manifest.txt` during `on_init` to prove the copied runtime layout is available before rendering.

## Run

```powershell
.\scripts\run-sample.ps1
```

## Expected Result

A 640x360 window opens. The background clear color changes over time, a colored triangle rotates through the SDK transform/camera path, then the sample closes after about four seconds. Mouse position influences the background tint, A/Left and D/Right adjust the tint and triangle transform, and Escape requests shutdown. The console logs the native backend lifecycle and sample module init/shutdown calls.

## Limitations

- Rendering is limited to a DirectX 11 clear color and a fixed triangle resource with transform/camera matrices.
- Input is limited to a small keyboard/mouse snapshot.
- There is no gamepad, IME/text editing, rebinding UI, raw input, or cursor capture yet.
- There is no mesh loader, sprite renderer, texture system, or asset pipeline yet.
- The game module is statically linked into the sample executable.

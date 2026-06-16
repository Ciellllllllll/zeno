# ZENO Sample Game

This sample is a small C++ game-module demo that runs through the ZENO C++ SDK.

It creates a Win32 window through the SDK/native backend path, initializes the DirectX 11 renderer, calls the sample module lifecycle, clears the screen with a changing color, draws a visible triangle for a few seconds, and shuts down cleanly.

## Build

From the repository root:

```powershell
cargo build -p zeno_abi
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

The sample target is part of the repository-level CMake preset graph. Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds all use the same `zeno_sample_game_cpp` target.

## Run

```powershell
.\scripts\run-sample.ps1
```

## Expected Result

A 640x360 window opens. The background clear color changes over time, a colored triangle is visible, then the sample closes after about four seconds. The console logs the native backend lifecycle and sample module init/shutdown calls.

## Limitations

- Rendering is limited to a DirectX 11 clear color and a fixed triangle resource.
- There is no input system yet.
- There is no mesh loader, sprite renderer, texture system, or asset pipeline yet.
- The game module is statically linked into the sample executable.

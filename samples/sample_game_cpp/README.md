# ZENO Sample Game

This sample is a small C++ game-module demo that runs through the ZENO C++ SDK.

It creates a Win32 window through the SDK/native backend path, initializes the DirectX 11 renderer, calls the sample module lifecycle, clears the screen with a changing color for a few seconds, and shuts down cleanly.

## Build

From the repository root:

```powershell
cargo build -p zeno_abi
cmake -S samples/sample_game_cpp -B build/sample-game-cpp
cmake --build build/sample-game-cpp --config Debug
```

## Run

```powershell
.\build\sample-game-cpp\Debug\zeno_sample_game_cpp.exe
```

## Expected Result

A 640x360 window opens. The background clear color changes over time, then the sample closes after about four seconds. The console logs the native backend lifecycle and sample module init/shutdown calls.

## Limitations

- Rendering is a DirectX 11 clear color only.
- There is no input system yet.
- There is no mesh, sprite, texture, or asset pipeline yet.
- The game module is statically linked into the sample executable.

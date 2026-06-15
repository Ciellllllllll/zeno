# ZENO Engine

ZENO Engine is a Windows-first custom game engine portfolio project. It is intended to show a small, explainable runtime architecture rather than compete with commercial engines.

The first milestone uses Rust for the high-level engine core, C++ for native backend and game-facing SDK code, DirectX 11 as the first renderer, and a strict C ABI boundary between Rust and C++.

## Current Status

The first milestone currently boots a Rust engine core through a C ABI, creates a Win32 window through the C++ native backend, initializes DirectX 11, exposes a small C++ SDK, and runs a sample C++ game module. The sample opens a window and clears it with a color that changes over time before shutting down cleanly.

## First Milestone Scope

- Windows 10/11
- Visual Studio 2022 / MSVC
- Rust stable
- Cargo workspace
- CMake skeletons for C++ projects
- DirectX 11 in a later renderer phase
- C ABI between Rust and C++

Out of scope for the first milestone: Linux, macOS, Vulkan, DirectX 12, editor tooling, scripting, physics, audio, networking, and a full asset pipeline.

## Repository Layout

```text
zeno/
  crates/
    zeno_core/      Rust engine runtime and internal core logic
    zeno_abi/       Rust ABI boundary crate for future C exports
  native/
    zeno_native/    C++ native backend skeleton
  sdk/
    cpp/            C++ Game SDK skeleton
  samples/
    sample_game_cpp/  Sample C++ game using the SDK/module path
  docs/             Project specs and phase reports
  tools/            Future local tools
  scripts/          Future helper scripts
```

## Build And Test

Rust workspace:

```powershell
cargo build -p zeno_abi
cargo test
```

C++ native backend and SDK:

```powershell
cmake -S native -B build/native
cmake --build build/native --config Debug

cmake -S sdk/cpp -B build/sdk-cpp
cmake --build build/sdk-cpp --config Debug
```

Sample game:

```powershell
cmake -S samples/sample_game_cpp -B build/sample-game-cpp
cmake --build build/sample-game-cpp --config Debug
.\build\sample-game-cpp\Debug\zeno_sample_game_cpp.exe
```

The sample should show a 640x360 window with a DirectX 11 clear color that changes for a few seconds. Current limitations: no input system, no mesh rendering, no asset pipeline, and no dynamic game-module loading yet.

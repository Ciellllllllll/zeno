# ZENO Engine

ZENO Engine is a Windows-first custom game engine portfolio project. It is intended to show a small, explainable runtime architecture rather than compete with commercial engines.

The first milestone uses Rust for the high-level engine core, C++ for native backend and game-facing SDK code, DirectX 11 as the first renderer, and a strict C ABI boundary between Rust and C++.

## Current Status

Phase 01 is the repository bootstrap. The repository currently contains the initial Rust workspace, C++ project skeletons, documentation, and phase reporting structure. It does not yet create a window, initialize DirectX 11, expose the full ABI, or run a sample game.

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
    sample_game_cpp/  Sample C++ game skeleton
  docs/             Project specs and phase reports
  tools/            Future local tools
  scripts/          Future helper scripts
```

## Build

Rust workspace:

```powershell
cargo test
```

C++ skeleton configure:

```powershell
cmake -S native -B build/native
cmake -S sdk/cpp -B build/sdk-cpp
cmake -S samples/sample_game_cpp -B build/sample-game-cpp
```

The C++ projects are intentionally minimal in Phase 01 and do not implement engine behavior yet.

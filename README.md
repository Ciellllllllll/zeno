# ZENO Engine

ZENO Engine is a Windows-first custom game engine portfolio project. It is a small, explainable runtime architecture, not an attempt to compete with Unity or Unreal Engine.

The first milestone uses Rust for the high-level engine core, C++ for the native backend and game-facing SDK, DirectX 11 as the first renderer, and a strict C ABI boundary between Rust and C++.

## Current Features

- Rust engine core with runtime lifecycle, frame stepping, frame timing, shutdown state, and error model.
- Rust C ABI crate that exposes engine creation, stepping, shutdown request, destruction, result codes, and POD frame/config structs.
- C++ native backend with Win32 window creation and DirectX 11 device/swap-chain/render-target bootstrap.
- Handle-based native backend resource example for clear-color resources.
- C++ Game SDK with RAII wrappers over engine/backend handles and explicit `zeno::Result` returns.
- Static-linked C++ game module lifecycle with `on_init`, `on_update`, `on_render`, and `on_shutdown`.
- Runnable sample game that opens a 640x360 window and clears with a changing DirectX 11 color before shutting down.
- Windows helper scripts for local build, run, and cleanup.

## Why This Shape

Rust owns high-level runtime state because the core benefits from explicit ownership, strong enums, and safe state transitions.

C++ owns the native backend because Windows and DirectX 11 integration are naturally C++-oriented and commonly expected in game-engine roles.

The boundary is C ABI because C++ ABI and Rust layout are not stable cross-language contracts. Public boundary data uses result codes, handles, fixed-layout POD structs, and explicit create/destroy ownership.

DirectX 11 is first because it is practical for a Windows portfolio slice, has broad tooling support, and is smaller in scope than starting with multiple graphics APIs.

## Build And Test

Supported local environment:

- Windows 10 or Windows 11
- Visual Studio 2022 with MSVC v143 and a Windows SDK
- CMake on `PATH`
- Rust stable with Cargo
- VS Code with rust-analyzer and CMake Tools is the intended lightweight editor workflow

Recommended full local build:

```powershell
.\scripts\build-all.ps1
```

Manual Rust build:

```powershell
cargo build -p zeno_abi
cargo test
```

Manual C++ build:

```powershell
cmake -S native -B build/native
cmake --build build/native --config Debug

cmake -S sdk/cpp -B build/sdk-cpp
cmake --build build/sdk-cpp --config Debug

cmake -S samples/sample_game_cpp -B build/sample-game-cpp
cmake --build build/sample-game-cpp --config Debug
```

Run the sample:

```powershell
.\scripts\run-sample.ps1
```

The sample should show a 640x360 window with a DirectX 11 clear color that changes for a few seconds. Console output shows native backend initialization/shutdown and sample module init/shutdown.

Clean generated local outputs:

```powershell
.\scripts\clean-build.ps1
```

## Repository Layout

```text
zeno/
  crates/
    zeno_core/          Rust engine runtime and internal core logic
    zeno_abi/           Rust C ABI exports and ABI-safe wrapper types
  native/
    zeno_native/        C++ Win32 and DirectX 11 native backend
  sdk/
    cpp/                C++ Game SDK wrappers over the ABI/backend APIs
  samples/
    sample_game_cpp/    Sample C++ game using the SDK/module path
  scripts/              Local Windows build/run helpers
  tools/                Reserved for future local tools
```

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) explains runtime ownership, native backend ownership, SDK/game-module roles, and the ABI handle/result model.
- [PORTFOLIO_NOTES.md](PORTFOLIO_NOTES.md) summarizes tradeoffs, limitations, roadmap, and interview talking points.
- [samples/sample_game_cpp/README.md](samples/sample_game_cpp/README.md) gives sample-specific build and run notes.

The local `docs/` directory contains phase specs and phase reports in this working copy, but it is intentionally ignored and not part of the public repository.

## Screenshots Or GIFs

Run `.\scripts\run-sample.ps1`, capture the 640x360 sample window while the clear color is changing, and place approved public media under a future tracked `media/` directory. Do not commit generated binaries or local build output when adding media.

## Current Limitations

- Rendering is currently a clear-color DirectX 11 path only.
- There is no input system, mesh renderer, sprite renderer, texture loading, asset pipeline, editor, physics, audio, or scripting.
- The sample game module is statically linked; dynamic module loading is left for a later phase.
- The first milestone is Windows-only.

## Roadmap

Near-term realistic next steps:

- Add a minimal input API.
- Add triangle or sprite rendering with handle-based shader/buffer/texture resources.
- Add a small asset-loading convention for sample resources.
- Introduce dynamic game module loading through C ABI entry points.
- Add conservative Windows CI once the local workflow remains stable.

Longer-term ideas:

- DirectX 12 after the DirectX 11 path is stable.
- Editor tooling much later, after runtime and rendering foundations are more complete.

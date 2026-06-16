# ZENO Engine

ZENO Engine is a Windows-first custom game engine portfolio project. It is a small, explainable runtime architecture, not an attempt to compete with Unity or Unreal Engine.

The first milestone uses Rust for engine runtime state, C++ for the native backend and game-facing SDK, DirectX 11 as the first renderer, and C ABI boundaries where Rust and C++ communicate.

## Current Features

- Rust engine core with runtime lifecycle, frame stepping, frame timing, shutdown state, and error model.
- Rust C ABI crate that exposes engine creation, stepping, shutdown request, destruction, result codes, and POD frame/config structs.
- C++ native backend with Win32 window creation and DirectX 11 device/swap-chain/render-target bootstrap.
- Handle-based native backend resources for clear colors, shader resources, textures, sprites, meshes, minimal material/render-state handles, and DirectX 11 draw paths.
- Keyboard and mouse input snapshot support for a small engine-owned key/button set.
- Minimal C++ SDK math foundations: vectors, row-major matrices, transforms, and orthographic/perspective cameras.
- Executable-relative sample asset root support with copied `assets/` content.
- C++ Game SDK with RAII wrappers over engine/backend handles and explicit `zeno::Result` returns.
- Static-linked C++ game module lifecycle with `on_init`, `on_update`, `on_render`, and `on_shutdown`.
- Runnable sample game whose C++ host drives the current loop, calls the static-linked module lifecycle, clears with a changing DirectX 11 color, draws a visible triangle, material-driven texture-backed sprite, and material-driven basic 3D mesh, and shuts down cleanly.
- Canonical Cargo and CMakePresets build graph shared by CLI, Visual Studio 2022 Open Folder, and VS Code CMake Tools.
- Windows helper scripts for local build, run, and cleanup.

## Why This Shape

Rust owns high-level runtime state because the core benefits from explicit ownership, strong enums, and safe state transitions.

C++ owns the native backend because Windows and DirectX 11 integration are naturally C++-oriented and commonly expected in game-engine roles.

The boundary is C ABI because C++ ABI and Rust layout are not stable cross-language contracts. Public boundary data uses result codes, handles, fixed-layout POD structs, and explicit create/destroy ownership.

DirectX 11 is first because it is practical for a Windows portfolio slice, has broad tooling support, and is smaller in scope than starting with multiple graphics APIs.

The current sample executable is an integration smoke for the C++ SDK, static-linked game module, native backend, and DirectX 11 presentation path. The Rust runtime is built and tested through the Rust and SDK/ABI smoke paths, but it is not yet the outer loop that drives the sample window.

## Build And Test

Supported local environment:

- Windows 10 or Windows 11
- Visual Studio 2022 with MSVC v143 and a Windows SDK
- CMake on `PATH`
- Rust stable with Cargo
- VS Code with rust-analyzer and CMake Tools is the intended lightweight editor workflow

The canonical build inputs are:

- Rust: `Cargo.toml` / Cargo workspace
- C++: `CMakeLists.txt` / `CMakePresets.json`
- Scripts: thin wrappers around Cargo and CMake presets

Do not treat generated `.sln`, `.vcxproj`, or editor task files as source-of-truth build files.

Recommended full local build:

```powershell
.\scripts\build-all.ps1
```

Manual Rust build:

```powershell
cargo build -p zeno_abi
cargo test --workspace
```

Manual C++ build:

```powershell
cmake --list-presets
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

Run the sample:

```powershell
.\scripts\run-sample.ps1
```

The sample should show a 640x360 window with a DirectX 11 clear color that changes for a few seconds, a visible rotating colored triangle drawn with shader assets, a small BMP-backed sprite using an alpha material, and a basic 3D cube mesh using an opaque depth-tested material. Mouse position influences the background tint, A/Left and D/Right adjust the tint and triangle transform, and Escape requests shutdown. Console output shows native backend initialization/shutdown, sample module init/shutdown, and asset shader compile failures if they occur.

Sample assets live under `samples/sample_game_cpp/assets/` in source and are copied beside the sample executable as `assets/` during the CMake build. The sample resolves assets from the executable directory, not the process working directory, so CLI, Visual Studio 2022, and VS Code launches use the same runtime layout.

Visual Studio 2022 should open this repository as a folder and consume `CMakePresets.json`. VS Code should use rust-analyzer for the Cargo workspace and CMake Tools for the same presets. Both IDEs are auxiliary entry points over the same targets used by the CLI.

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
      assets/           Source sample assets copied to the runtime output
  scripts/              Local Windows build/run helpers
  tools/                Reserved for future local tools
  CMakePresets.json     Canonical C++ configure/build/test presets
```

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) explains runtime ownership, native backend ownership, SDK/game-module roles, and the ABI handle/result model.
- [PORTFOLIO_NOTES.md](PORTFOLIO_NOTES.md) summarizes tradeoffs, limitations, roadmap, and interview talking points.
- [samples/sample_game_cpp/README.md](samples/sample_game_cpp/README.md) gives sample-specific build and run notes.

The local `docs/` directory contains phase specs and phase reports in this working copy, but it is intentionally ignored and not part of the public repository.

## Screenshots Or GIFs

Run `.\scripts\run-sample.ps1`, capture the 640x360 sample window while the triangle is visible and the clear color is changing, and place approved public media under a future tracked `media/` directory. Do not commit generated binaries or local build output when adding media.

## Current Limitations

- Rendering is currently limited to a clear-color DirectX 11 path plus fixed minimal triangle, sprite, indexed mesh, and material/render-state resources with transform, camera, texture, depth, and explicit vertex/pixel shader handles.
- Input is limited to a small keyboard/mouse snapshot. There is no gamepad, IME/text editing, rebinding UI, raw input, or cursor capture.
- There is no shader reflection, material graph, hot reload, mesh importer, atlas system, font rendering, asset pipeline, editor, physics, audio, or scripting.
- The sample game module is statically linked; dynamic module loading is left for a later phase.
- The sample loop currently drives the native backend directly through the C++ SDK; integrating the Rust runtime as the sample's outer frame scheduler is future work.
- The first milestone is Windows-only.

## Roadmap

Near-term realistic next steps:

- Add scene/object ownership over the existing handle-based render resources.
- Add a small asset pipeline convention for sample resources.
- Introduce dynamic game module loading through C ABI entry points.
- Add conservative Windows CI once the local workflow remains stable.

Longer-term ideas:

- DirectX 12 after the DirectX 11 path is stable.
- Editor tooling much later, after runtime and rendering foundations are more complete.

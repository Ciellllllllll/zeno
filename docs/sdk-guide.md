# Portfolio Notes

## What This Project Demonstrates

- A Rust runtime core with explicit lifecycle and error handling.
- A C ABI boundary designed around handles, POD structs, and stable result codes.
- C++ native backend ownership for Win32 and DirectX 11 resources.
- A C++ SDK layer that is ergonomic for game code without leaking through the ABI.
- A high-level C++ `GameApp` runtime that gathers setup, frame stepping, input, assets, audio, scene data, and module lifecycle into one host API.
- Explicit `GameApp` failure cleanup semantics: partial `on_init` failure is followed by `on_shutdown` before SDK-owned backend/audio resources are reset.
- A first Windows dynamic module loader that uses a C-compatible descriptor entry point without exposing SDK classes, STL, Win32, DirectX, or Rust internals across the DLL boundary.
- A DirectX 11 resize baseline that rebuilds swap-chain render target/depth resources and detects device removed/reset without claiming full recovery.
- SDK-side ResourceManager IDs that decouple scene render components from raw SDK wrapper pointers and native backend handles.
- A minimal embedded 5x7 ASCII debug overlay for FPS, frame, score, state, and diagnostics without external font assets.
- A small sample game module that proves init/update/render/shutdown flow.
- A separate minimal C++ template game that proves a second game target can reuse the same SDK, `GameApp`, assets/config layout, and CMakePresets workflow.
- A packaged SDK layout and external CMake example that prove SDK consumption outside the repository sample/template graph.
- A minimal handle-owned DirectX 11 triangle draw path.
- Backend-owned DirectX 11 vertex/pixel shader handles with bounded compile diagnostics.
- Backend-owned texture handles and a small sprite draw path using WIC-loaded image bytes.
- Backend-owned indexed mesh handles with immutable vertex/index buffers and depth testing.
- Backend-owned material handles that bundle fixed shader choice, texture dependency, blend mode, depth mode, and cull mode.
- SDK-owned component-lite scene objects and strict project/scene startup data loading.
- Backend-owned XAudio2 audio engine and short PCM WAV sound handles.
- SDK-side 2D AABB collision helpers and temporary DirectX 11 debug line/rectangle visualization.
- A keyboard/mouse input snapshot connected to the Win32 message loop.
- A small math, transform, and camera foundation with documented DirectX-first conventions.
- Practical Windows build workflow using Cargo, CMakePresets, and Visual Studio 2022/MSVC.
- A documented v0 regression baseline covering clean build, smoke tests, sample/template runs, packaging, and ABI forbidden-type checks.
- A conservative Windows CI baseline for format, ABI scan, headless smoke tests, and package creation.

## Design Decisions

- Rust is used where ownership and state-machine clarity matter most.
- C++ is used where platform and DirectX 11 integration are most natural.
- C ABI is used instead of Rust/C++ native ABI to keep the boundary compiler-stable and easy to audit.
- DirectX 11 is the first renderer to keep the milestone narrow and demonstrable.
- Static game-module linking remains the default sample/template path; dynamic loading uses a small C ABI descriptor and explicit API version validation.
- `GameApp` is SDK-owned orchestration over existing Rust/runtime and native/backend handles, not a new cross-language ABI.
- Module shutdown is part of the cleanup contract. Module authors must tolerate partially initialized state and clean up only resources they actually created.
- Cargo and CMakePresets are the canonical build inputs so Visual Studio 2022, VS Code, and CLI usage share the same targets.
- Runtime packaging is intentionally a local script over the canonical build/install graph, not a custom installer or separate project generator.
- SDK packaging exposes imported CMake targets for external projects while keeping private specs, generated build trees, and Codex files out of the package.
- SDK matrices are row-major, row-vector, left-handed, and DirectX clip-space oriented so the first renderer and sample can stay easy to inspect.

## Tradeoffs

- The engine is Windows-only for now. This keeps the first milestone focused but does not prove portability.
- Rendering, audio, and collision are intentionally small. The current milestone proves backend initialization, frame presentation, shader resource creation, SDK ResourceManager ownership for texture/mesh/material/sound/triangle references, SDK scene object organization, project/scene startup loading, transform/camera constant buffer binding, depth testing, fixed triangle/sprite/mesh draw paths, temporary debug visualization, minimal embedded debug overlay text, SDK-side AABB checks, and short PCM WAV effects, not a full renderer, physics engine, or audio engine.
- Input is intentionally small. It proves per-frame keyboard/mouse state without claiming text input, gamepad, or rebinding support.
- The SDK is intentionally small. `GameApp` removes boilerplate from game hosts while still exposing the low-level wrappers for focused tests and future engine work.
- Build, verification, and package scripts are local PowerShell wrappers over Cargo and CMake presets rather than separate build definitions. This keeps the workflow practical while CI remains conservative and headless by default.

## Difficult Parts

- Keeping Rust, C ABI, C++ SDK, and C++ native backend responsibilities separate.
- Avoiding convenient but unsafe ABI shortcuts such as C++ classes, STL types, or Rust layouts across the boundary.
- Making DirectX 11 ownership internal while still exposing a usable render path.
- Keeping the portfolio scope honest and small enough to finish.

## Known Limitations

- No gamepad, IME/text editing, rebinding UI, raw input, or cursor capture.
- No shader reflection, material graph, hot reload, mesh importer, atlas system, font rendering, or generalized render pipeline beyond the fixed DirectX 11 triangle, sprite, and mesh paths.
- Debug overlay text is limited to an embedded 5x7 ASCII line font for diagnostics. It is not a general font renderer, UI toolkit, rich text system, or Unicode shaping implementation.
- No streaming BGM, 3D spatial audio, mixer graph, or compressed audio decode.
- Math/collision is limited to the SDK primitives currently needed by the sample; there are no quaternions, decomposition helpers, 3D collision volumes, physics solver, swept collision, broadphase, or SIMD optimizations yet.
- No asset pipeline.
- No hot reload, scripting, editor plugins, or generalized plugin ecosystem.
- Dynamic modules currently receive only an opaque reserved host context; ABI-safe host services are future work.
- No full DirectX device-lost recovery or resource rehydration system yet; Phase 39 detects device removed/reset and returns backend errors.
- No project generator or installer; the template game and packaging script are deliberately minimal.
- No editor.
- CI covers the headless baseline only; visual sample/template execution remains local/manual.
- No multi-platform support.
- No checked-in screenshots or GIFs yet; capture approved media from the sample window when needed.
- No generated Visual Studio solution or project files are source-of-truth artifacts; IDEs consume the preset-based build graph.

## Roadmap

Near-term:

- Add ABI-safe host service tables for dynamic modules.
- Harden Rust C ABI panic containment.

Later:

- Add a simple asset convention.
- Add a richer sample scene.
- Consider DirectX 12 after DirectX 11 is stable.
- Consider editor tooling only after runtime and rendering foundations are mature.

## Interview Talking Points

- The project uses Rust for high-level runtime safety while keeping Windows and DirectX work in C++.
- The C ABI boundary avoids unstable Rust/C++ layout and calling convention issues.
- Handles prevent external callers from depending on internal Rust, C++, Win32, or DirectX object layouts.
- The sample is deliberately modest but playable: the C++ host delegates boilerplate to `GameApp`, while the module proves project/scene startup loading, engine boot, window creation, DirectX 11 presentation, SDK-owned scene objects, handle-owned triangle, materialized sprite and mesh draw paths, keyboard movement, goal score/restart flow, SDK-side AABB checks with debug visualization, overlay diagnostics, short WAV effect playback, static-linked game-module lifecycle, and clean shutdown.
- The current v0 baseline is intentionally modest but reproducible: clone, build, run the sample/template, package the runtime layout, and explain the boundaries.
- The next most valuable technical step is ABI-safe host services and renderer robustness, not adding an editor or multi-platform abstraction.

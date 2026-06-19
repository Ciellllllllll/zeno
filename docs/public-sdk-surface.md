# Public SDK Surface Inventory

This inventory defines the current SDK-visible surface for pre-1.0 stability work.

It does not publish `v1.0.0`, create a GitHub Release, create a git tag, upload release assets, add gameplay samples, add engine features, redesign SDK/API/native/backend internals, or authorize committing generated SDK ZIPs or build outputs.

## Classification Model

Each SDK-visible file or concept belongs to one of these categories:

- Stable public SDK: intended for packaged SDK consumers and protected by the compatibility policy in `docs/sdk-compatibility-policy.md`.
- Experimental public SDK: visible to packaged SDK consumers but not yet stable enough to treat as a 1.0 contract.
- Demonstration-only sample surface: source, assets, and docs that demonstrate supported workflows but are not reusable API contracts.
- Private implementation: repository implementation detail, test helper, or internal build input. It must not be treated as public API.
- Generated/build artifact: output from Cargo, CMake, packaging, CI, or local execution. It must not be committed or treated as source.

## Packaged Include Policy

Only these headers are currently allowed in packaged `include/zeno/`:

| Packaged header | Source path | Classification | Notes |
| --- | --- | --- | --- |
| `zeno.hpp` | `sdk/cpp/include/zeno/zeno.hpp` | Stable public SDK | Main C++ SDK convenience layer for current game-facing workflows. |
| `math.hpp` | `sdk/cpp/include/zeno/math.hpp` | Stable public SDK | SDK math helpers used by samples, scenes, rendering descriptions, and collision helpers. |
| `game_module.hpp` | `sdk/cpp/include/zeno/game_module.hpp` | Stable public SDK with experimental dynamic-module area | `GameApp`, static `GameModule`, and `GameContext` are stable public SDK for the current scope. `DynamicGameModule` remains experimental until host services and support boundary are finalized. |
| `game_module_abi.h` | `sdk/cpp/include/zeno/game_module_abi.h` | Experimental public SDK | C-compatible dynamic module descriptor. The host context is reserved and not yet a mature game-module service API. |
| `zeno_abi.h` | `native/zeno_native/include/zeno/zeno_abi.h` | Stable public C ABI | Rust engine lifecycle ABI using handles, POD structs, result codes, explicit sizes, and API versions. |
| `zeno_native_backend.h` | `native/zeno_native/include/zeno/zeno_native_backend.h` | Experimental public C ABI | Native backend ABI is packaged for current SDK wrappers and low-level tests. It is public C ABI, but direct game-author use is lower-level and less stable than `GameApp` and `ResourceManager`. |

No other header may be copied into packaged `include/zeno/` unless a future phase updates this inventory and the package script together.

## Stable Public SDK

The current stable public SDK is the narrow game-facing layer that external C++ consumers are expected to use:

- `zeno::Result`, `zeno::ResultCode`, `zeno::LogLevel`, `zeno::LogMessage`, diagnostic sink functions, and last-diagnostic helpers.
- `zeno::AssetRoot` and `zeno::AssetPath` for executable-relative and explicit asset roots.
- `zeno::Engine`, `zeno::EngineConfig`, and `zeno::EngineFrameInfo`.
- `zeno::GameApp`, `zeno::GameAppConfig`, `zeno::GameModule`, `zeno::GameContext`, and static-linked module lifecycle helpers.
- `zeno::ResourceManager` and SDK-only resource IDs: `TextureId`, `MaterialId`, `MeshId`, `SoundId`, and `TriangleId`.
- RAII wrappers for current backend-owned resources: `NativeBackend`, `RenderTriangle`, `VertexShader`, `PixelShader`, `Texture`, `Mesh`, `Material`, `AudioEngine`, and `Sound`.
- Current 2D, 3D, material, input, audio, scene, project, and serialization descriptors exposed by `zeno.hpp`.
- Math and collision helpers in `math.hpp`: vectors, matrices, transforms, cameras, AABB helpers, and intersection helpers.
- Public C ABI engine lifecycle in `zeno_abi.h`.
- Packaged CMake target `ZenoEngine::zeno_sdk_cpp`.
- Packaged SDK layout entries documented in `docs/sdk-layout.md`: `include/`, `lib/`, `bin/`, `cmake/`, `templates/cpp_empty/`, `samples/sdk_feature_samples_cpp/`, and `docs/`.

Stable means future phases should preserve source compatibility where practical, document any change, and treat accidental changes as release-blocking until reviewed.

## Experimental Public SDK

These surfaces are currently visible to SDK consumers but are not mature enough to promise as a final 1.0 contract:

- `zeno::DynamicGameModule`.
- `ZenGameModuleDescriptor`, `ZenGameModuleHostContext`, and `ZEN_GAME_MODULE_ENTRY_POINT`.
- Direct game-author use of `zeno_native_backend.h`.
- Low-level direct calls through `zeno::NativeBackend` for shader, renderer, material, texture, mesh, debug draw, and audio resource creation.
- Renderer backend selection and capability reporting concepts described in `docs/renderer-backend-strategy.md` until a future phase adds concrete public API.
- Project and scene text formats represented by `.zproj` and `.zscene`.
- Debug draw and debug text behavior.
- Diagnostic category names and exact diagnostic message text.

Experimental public SDK may be used by samples and advanced consumers, but future phases may refine it before 1.0. Any refinement must be documented in this inventory and in release notes.

## Demonstration-Only Sample Surface

These files are intentionally useful as examples, not public API contracts:

- `samples/sample_game_cpp/**`
- `samples/sdk_feature_samples_cpp/**`
- `samples/dynamic_module_cpp/**`
- `templates/game-cpp/**`
- sample assets under `samples/**/assets/**`
- template assets under `templates/**/assets/**`
- tutorial code snippets in `docs/tutorials/**`
- `examples/external-game/**`

Demonstration-only files may show current best practice, but external projects must not depend on their private helper functions, asset names, gameplay logic, or sample-specific CMake structure as a stable API.

## Private Implementation

The following must never be treated as public API:

- `sdk/cpp/src/**`
- `native/zeno_native/src/**`
- `crates/**/src/**`
- `tests/**`
- `native/zeno_native/src/native_backend.h`
- smoke test sources such as `*_smoke.cpp`
- repository root build orchestration files when used internally rather than from the packaged SDK
- `.codex/**`
- `.github/**`
- local scripts under `scripts/**`, except as documented contributor/release tooling
- implementation CMake targets that are not documented packaged targets

Private implementation may change whenever needed to preserve the public SDK contract. Private files must not be copied into packaged `include/zeno/`.

## Generated And Build Artifacts

Generated/build artifacts include:

- `build/**`
- `target/**`
- generated SDK package directories
- generated SDK ZIPs
- downloaded GitHub Actions artifact wrapper archives
- `.sln`, `.vcxproj`, or IDE-generated project files
- executables, DLLs, LIBs, PDBs, object files, logs, caches, and temporary package extraction directories

These outputs must not be committed unless a later phase explicitly changes repository policy. They must not be used as source-of-truth for public API.

## What Must Never Be Public API

Do not treat any of the following as public API:

- C++ implementation classes or private structs outside packaged headers.
- Rust internal types, module paths, or memory layouts.
- DirectX COM interfaces, Win32 handles/types, XAudio2 types, or WIC types.
- DirectX 11 or DirectX 12 devices, contexts, swap chains, command queues, command lists, descriptor heaps, fences, resources, views, pipeline states, root signatures, DXGI adapters, or shader compiler objects.
- CMake build-tree paths or Visual Studio generated project files.
- sample gameplay rules, asset file names, scores, movement constants, or scene object names.
- exact diagnostic message wording unless a future diagnostics contract explicitly stabilizes it.
- private helper functions in sample or smoke-test sources.
- generated package layout paths under `build/`.

## Updating This Inventory

Every future phase that changes SDK-visible files or concepts must:

1. Identify whether the change affects stable public SDK, experimental public SDK, demonstration-only sample surface, private implementation, or generated/build artifacts.
2. Update this inventory in the same phase if classification changes.
3. Update `docs/sdk-compatibility-policy.md` when the change affects compatibility, deprecation, or breaking-change rules.
4. Update `docs/1.0-stability-gate.md` when the change adds or removes a release-blocking check.
5. Update `docs/renderer-backend-strategy.md` when the change affects renderer backend selection, capability reporting, DirectX 11 support, or future DirectX 12 planning.
6. Record public API or ABI changes in the phase report.
7. Keep generated ZIPs, build outputs, and downloaded artifacts out of commits.

If a phase is unsure whether a visible item is public, classify it as experimental until a later explicit stability phase promotes it.

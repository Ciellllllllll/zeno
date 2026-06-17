# SDK Guide

This document describes the current C++ SDK surface for building small ZENO sample games. The SDK is a convenience layer over the Rust engine ABI and C++ native backend; it is not itself part of the C ABI.

## Boundaries

- Game code uses C++ SDK types such as `zeno::GameApp`, `zeno::GameModule`, `zeno::NativeBackend`, `zeno::Scene`, and `zeno::ResourceManager`.
- Rust/C++ communication stays behind the C ABI and uses handles, POD structs, primitive values, and `ZenResultCode`.
- Win32, DirectX 11, XAudio2, C++ classes, STL containers, references, templates, and exceptions do not cross the C ABI boundary.
- Native backend resources are still owned by the backend. SDK wrappers and SDK resource IDs only manage game-facing lifetime and lookup.

## GameApp Runtime

`zeno::GameApp` is the high-level host used by the sample and template games. It owns the engine runtime wrapper, native backend wrapper, asset root, audio engine, resource manager, runtime scene, loaded project config, loaded scene description, and game context.

The static module flow is:

1. Create engine/backend/assets/project/scene/window/renderer/audio.
2. Fill `zeno::GameContext`.
3. Call module `on_init`.
4. Per frame, call engine begin, poll/input, `on_update`, `on_render`, and engine end.
5. Call `on_shutdown` and release SDK/backend resources.

If `on_init` fails after runtime setup, `GameApp` calls `on_shutdown` before resetting resources. Module shutdown code must therefore tolerate partially initialized state.

## Game Module Shape

A static-linked game module returns a `zeno::GameModule` with optional lifecycle callbacks:

```cpp
zeno::GameModule create_sample_game_module()
{
    zeno::GameModule module{};
    module.on_init = on_init;
    module.on_update = on_update;
    module.on_render = on_render;
    module.on_shutdown = on_shutdown;
    return module;
}
```

The template under `templates/game-cpp/` is the smallest current example. The sample under `samples/sample_game_cpp/` demonstrates project/scene loading, resource creation, scene rendering, input, debug drawing, overlay diagnostics, collision helpers, and audio playback.

## Resources And Scene

`zeno::ResourceManager` owns SDK wrappers for textures, materials, meshes, sounds, and render triangles. It returns SDK-only IDs such as `TextureId`, `MaterialId`, `MeshId`, `SoundId`, and `TriangleId`. These IDs are not native backend handles.

`zeno::Scene` stores object IDs, transforms, and render components. Render components store SDK resource IDs, and `Scene::render` resolves those IDs through `ResourceManager` before calling backend draw APIs. Invalid, stale, missing, or wrong resource IDs return `ZEN_RESULT_INVALID_ARGUMENT`.

The older direct wrapper APIs on `zeno::NativeBackend` remain available for focused tests and simple code paths. Prefer `ResourceManager` for scene-owned resources.

## Diagnostics And Overlay

The SDK provides a lightweight diagnostic sink:

- `zeno::set_log_sink`
- `zeno::clear_log_sink`
- `zeno::last_diagnostic`
- `zeno::clear_last_diagnostic`

The sample draws a minimal debug overlay through `NativeBackend::draw_debug_text`. This uses an embedded 5x7 ASCII line font in the native backend and is intended for FPS, frame index, score, state, and diagnostic display. It is not a general font renderer or UI system.

## Packaging

Use:

```powershell
.\scripts\package-sdk.ps1
.\scripts\verify-external-game.ps1
```

The SDK package is written under `build/package-sdk/ZenoEngine-SDK-v0.1.0-dev/` and also as `build/package-sdk/ZenoEngine-SDK-v0.1.0-dev.zip`. It exposes imported CMake targets, including `ZenoEngine::zeno_sdk_cpp`. The external example under `examples/external-game/` consumes that package without repo-relative include paths.

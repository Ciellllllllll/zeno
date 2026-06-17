# SDK API Reference

These pages describe the public C++ SDK concepts shipped in the packaged SDK. They are user-facing notes for the headers under `include/zeno/`, not a replacement for the headers themselves.

## Start Here

- [SDK Overview](sdk-overview.md): package boundaries, public headers, result handling, diagnostics, and CMake targets.
- [Engine Lifecycle](engine-lifecycle.md): `Engine`, `GameApp`, `GameModule`, frame flow, shutdown, and module safety.
- [Assets, Input, And Audio](assets-input-audio.md): executable-relative assets, project/scene loading, input snapshots, and short sound playback.
- [Math, Camera, And Collision](math.md): vectors, matrices, transforms, cameras, and AABB helpers.
- [2D Rendering](rendering-2d.md): sprites, texture materials, cameras, debug rectangles, and debug text.
- [3D Rendering](rendering-3d.md): meshes, materials, perspective camera setup, scene rendering, and debug lines.

## Public Header Boundary

SDK users normally include:

```cpp
#include <zeno/game_module.hpp>
```

That header includes the C++ SDK surface used by the packaged template and focused samples. Direct use of `zeno_abi.h` and `zeno_native_backend.h` is available in the package for ABI and backend interop, but normal C++ SDK projects should stay on the C++ wrappers unless they are writing low-level integration tests.

Private implementation headers from repository `src/` directories are not part of the SDK package.

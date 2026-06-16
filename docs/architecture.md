# ZENO Architecture

ZENO is organized as a small vertical engine slice. The goal is to keep ownership and language boundaries easy to explain.

```text
Sample Game Host
    |-- C++ Game Module
    |-- C++ Game SDK -> Rust C ABI -> Rust Engine Core
    `-- C++ Game SDK -> C++ Native Backend -> Windows / DirectX 11
```

The current sample host owns the outer loop. It calls the SDK/module lifecycle and drives the native backend render path through the SDK. The Rust runtime path is verified by Rust, ABI, and SDK smoke tests in this milestone; wiring it in as the sample's outer frame scheduler is future work.

## Canonical Build Graph

Rust build ownership stays with the Cargo workspace rooted at `Cargo.toml`.

C++ build ownership stays with the top-level `CMakeLists.txt` and `CMakePresets.json`. The canonical preset names are `windows-msvc-debug` and `windows-msvc-release`. Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds all consume the same native backend, SDK, and sample game CMake targets.

The PowerShell scripts in `scripts/` are wrappers over Cargo and CMake presets; they are not separate build definitions.

## Rust Engine Core

`crates/zeno_core` owns high-level runtime behavior:

- engine configuration,
- lifecycle state,
- frame stepping,
- frame timing,
- shutdown request state,
- engine errors and log categories.

Rust internals are not exposed to C++ callers.

## Rust C ABI Layer

`crates/zeno_abi` exposes C-compatible functions and data:

- integer result codes,
- opaque engine handles,
- fixed-layout config/frame structs,
- explicit create/destroy lifecycle calls.

This layer converts between ABI-safe data and Rust core types. Unsafe code is kept at this boundary.

## C++ Native Backend

`native/zeno_native` owns platform and renderer implementation:

- Win32 window creation and message polling,
- Win32 keyboard/mouse message collection,
- DirectX 11 device/context/swap-chain setup,
- render target creation,
- depth buffer and depth state creation,
- clear/present operations,
- backend-owned clear-color, shader, texture, sprite, material, triangle, and mesh render resources.

The public backend API exposes handles and POD structs, not Win32 or DirectX objects.

Renderer resources such as shaders, textures, materials, sprite state, mesh vertex/index buffers, depth buffers, and DirectX state objects are owned by the native backend. The SDK owns RAII wrappers over opaque handles only.

Vertex and index data cross the ABI as borrowed pointers plus explicit counts and stride. The native backend copies that data into backend-owned GPU buffers during creation. Material choices cross as fixed integer enums and handles. Transform and camera data cross as POD matrices. DirectX buffers, input layouts, blend/rasterizer/depth stencil objects, shader resource views, COM pointers, and Win32 handles remain native-backend private.

## Input Model

Keyboard and mouse input live in the native backend. Win32 messages are translated into an engine-owned input snapshot with current, pressed, and released states for a small key/button set plus mouse position and wheel detents.

Game code reads input through the C++ SDK snapshot. The public ABI uses only POD arrays and primitive fields; it does not expose `HWND`, Win32 message values, raw virtual-key constants, or borrowed internal pointers. Input is single-threaded with the window loop: call `poll_events`, then read the snapshot for that frame.

## Renderer Frame Order

The native renderer is single-threaded per backend handle. Resource creation can happen after renderer initialization, but drawing must follow this order every frame:

```text
begin_frame
  clear or clear_with_resource
  draw_triangle zero or more times
  draw_sprite zero or more times
  draw_mesh zero or more times
present
```

`begin_frame` binds the render target and depth stencil view and clears depth. Mesh drawing enables depth testing and writes; the current triangle and sprite paths explicitly use depth-disabled state. `present` closes the active frame. Calls such as nested `begin_frame`, `clear` before `begin_frame`, draw calls outside an active frame, or `present` outside an active frame return a stable `ZenResultCode` instead of relying on undefined DirectX state.

## Math And Coordinate Convention

The SDK math layer is intentionally small and DirectX-first. It defines `Vec2`, `Vec3`, `Vec4`, `Mat4`, `Transform`, and `Camera` for sample/game-facing code.

ZENO uses a left-handed coordinate system for this milestone:

- `+X` points right.
- `+Y` points up.
- `+Z` points forward.
- Distances are engine units.
- Rotations are radians.
- DirectX clip-space depth is `0..1`.

`Mat4` stores 16 `float` values in row-major order and uses row-vector multiplication. Translation lives in elements `12`, `13`, and `14`, equivalent to row 3, columns 0 through 2. Transform composition is `scale * rotation * translation`, and camera composition is `view * projection`. The DirectX 11 triangle shader declares matrices as `row_major` and transforms vertices with `mul(position, matrix)`.

Only POD matrix data crosses the native backend ABI as `ZenMatrix4x4`. SDK math types and C++ helper classes remain SDK-only and do not cross the ABI boundary.

## C++ Game SDK

`sdk/cpp` is the game-facing convenience layer. It provides small C++ wrappers such as `zeno::Engine`, `zeno::NativeBackend`, `zeno::Result`, a minimal component-lite `zeno::Scene`, and game-module callback helpers.

SDK classes are allowed for game code ergonomics, but they do not cross the Rust/C++ ABI boundary.

The scene layer is SDK-owned. It tracks object IDs, transforms, and one renderable record per object, then emits existing SDK draw calls in creation order. Scene objects are not exposed to Rust, the C ABI, or the native backend.

## Game Module And Sample

`samples/sample_game_cpp` statically links a small game module into the sample executable. The module implements:

- `on_init`,
- `on_update`,
- `on_render`,
- `on_shutdown`.

The current sample changes the DirectX 11 clear color over time, updates SDK scene objects, draws a colored triangle, a materialized texture-backed sprite, and a materialized indexed cube mesh through the SDK, then exits cleanly after a short demo loop.

The game module is statically linked into the sample executable. Dynamic module loading and hot reload are not implemented in this milestone.

## ABI Model

The ABI uses:

- `ResultCode`-style status returns for fallible calls,
- `uint64_t` handle values for engine/backend/resource ownership,
- POD structs with size/version fields where extension is likely,
- explicit init/create/destroy naming.

The ABI does not expose:

- C++ classes,
- C++ references,
- `std::string`,
- `std::vector`,
- templates,
- exceptions,
- Rust internal structs,
- DirectX COM interfaces.

## Scope Choice

ZENO intentionally starts with one platform and one renderer. Windows and DirectX 11 keep the first milestone focused on proving runtime/backend/render/module architecture instead of building a premature multi-platform abstraction.

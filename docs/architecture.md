# ZENO Architecture

ZENO is organized as a small vertical engine slice. The goal is to keep ownership and language boundaries easy to explain.

```text
Sample Game Host
    |-- C++ Game Module
    |-- C++ Game SDK -> Rust C ABI -> Rust Engine Core
    `-- C++ Game SDK -> C++ Native Backend -> Windows / DirectX 11
```

The current sample host delegates the outer loop to `zeno::GameApp`. `GameApp` owns the Rust engine runtime wrapper, native backend wrapper, asset root, audio engine, runtime scene, project/scene startup data, input snapshot, and module lifecycle. The sample and template use the static-linked module path. A separate headless sample proves the Windows DLL module loading path. Game modules still render through SDK calls and do not access native backend internals directly.

## Canonical Build Graph

Rust build ownership stays with the Cargo workspace rooted at `Cargo.toml`.

C++ build ownership stays with the top-level `CMakeLists.txt` and `CMakePresets.json`. The canonical preset names are `windows-msvc-debug` and `windows-msvc-release`. Visual Studio 2022 Open Folder, VS Code CMake Tools, and CLI builds all consume the same native backend, SDK, and sample game CMake targets.

The template game under `templates/game-cpp/` is another CMake target in the same graph. It links the C++ SDK and uses `zeno::GameApp` like the sample, but keeps its module intentionally minimal so it can act as a starting point for a new static-linked game.

The PowerShell scripts in `scripts/` are wrappers over Cargo and CMake presets; they are not separate build definitions. `scripts/package-runtime.ps1` builds the canonical preset and installs a local runtime package containing the sample executable/assets, template executable/assets, and required Rust ABI DLL. `scripts/package-sdk.ps1` creates an external SDK package with public headers, static libraries, the Rust ABI import library/DLL, and CMake package config files. `BUILDING.md` records the regression command matrix for the v0 usable baseline.

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
- backend-owned clear-color, shader, texture, sprite, material, triangle, mesh render resources, temporary debug line/rectangle draw state, audio engines, and sound resources.

The public backend API exposes handles and POD structs, not Win32 or DirectX objects.

Renderer resources such as shaders, textures, materials, sprite state, mesh vertex/index buffers, depth buffers, and DirectX state objects are owned by the native backend. The SDK owns RAII wrappers over opaque handles only.

Audio resources follow the same boundary rule. XAudio2 engine, mastering voice, source voices, WAV parsing, and PCM sample storage are native backend internals. ABI-facing audio data is limited to handles, a small POD descriptor, borrowed WAV bytes, primitive volume values, and result codes.

Window resize is handled inside the native backend. `WM_SIZE` is translated into primitive resize state; minimized or zero-size windows skip swap-chain resizing, and nonzero resize recreates only swap-chain-dependent render target/depth resources and updates the viewport. DirectX 11 device removed/reset is detected and surfaced through the existing backend error result path; full device-loss resource recovery is intentionally out of scope for this baseline.

Vertex and index data cross the ABI as borrowed pointers plus explicit counts and stride. The native backend copies that data into backend-owned GPU buffers during creation. Material choices cross as fixed integer enums and handles. Transform and camera data cross as POD matrices. DirectX buffers, input layouts, blend/rasterizer/depth stencil objects, shader resource views, COM pointers, and Win32 handles remain native-backend private.

Debug draw is an immediate development visualization path, not a general renderer feature set. Debug line and rectangle data cross the native backend ABI as POD descriptors with primitive coordinates and color values only. Collision helpers stay SDK/sample-owned and do not introduce native collision resources.

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

The SDK math layer is intentionally small and DirectX-first. It defines `Vec2`, `Vec3`, `Vec4`, `Mat4`, `Transform`, `Camera`, and SDK-side 2D AABB helpers for sample/game-facing code.

ZENO uses a left-handed coordinate system for this milestone:

- `+X` points right.
- `+Y` points up.
- `+Z` points forward.
- Distances are engine units.
- Rotations are radians.
- DirectX clip-space depth is `0..1`.

`Mat4` stores 16 `float` values in row-major order and uses row-vector multiplication. Translation lives in elements `12`, `13`, and `14`, equivalent to row 3, columns 0 through 2. Transform composition is `scale * rotation * translation`, and camera composition is `view * projection`. The DirectX 11 triangle shader declares matrices as `row_major` and transforms vertices with `mul(position, matrix)`.

Only POD matrix data crosses the native backend ABI as `ZenMatrix4x4`. SDK math types and C++ helper classes remain SDK-only and do not cross the ABI boundary.

`Aabb2` is a helper primitive, not a physics subsystem. It supports simple overlap and point containment checks for axis-aligned rectangles; rotation is ignored when deriving an AABB from a transform.

## C++ Game SDK

`sdk/cpp` is the game-facing convenience layer. It provides small C++ wrappers such as `zeno::Engine`, `zeno::NativeBackend`, `zeno::Result`, a minimal component-lite `zeno::Scene`, SDK-owned `zeno::ResourceManager`, `zeno::GameApp`, and game-module callback helpers.

SDK classes are allowed for game code ergonomics, but they do not cross the Rust/C++ ABI boundary.

The scene layer is SDK-owned. It tracks object IDs, transforms, and one renderable record per object, then emits existing SDK draw calls in creation order. Scene renderers store SDK resource IDs, not raw wrapper pointers or native backend handles. `ResourceManager` owns the move-only SDK wrappers for textures, materials, meshes, sounds, and triangles, and `Scene::render` resolves those IDs before calling the existing backend wrapper APIs. Missing, stale, invalid, or type-mismatched SDK resource IDs return `ZEN_RESULT_INVALID_ARGUMENT` at the SDK layer. Scene objects and resource IDs are not exposed to Rust, the C ABI, or the native backend.

Project and scene loading are also SDK-owned. The current format is a strict versioned UTF-8 line format used for sample startup data. Parsed scene data becomes SDK objects and existing resource creation calls; filenames and scene structures do not cross the C ABI.

Diagnostics are SDK-owned for this milestone. Game code may install a `zeno::LogSink` callback and query `zeno::last_diagnostic()` for a copied message after failures. The sink receives `std::string_view` values that are borrowed only for the callback duration; `last_diagnostic()` returns an owning `std::string`. Asset path failures, scene parse failures, shader compile errors, ResourceManager ID resolution failures, selected native backend failures, and selected Rust ABI wrapper failures feed this path. No logging framework, telemetry channel, or C ABI callback was added in this phase.

`GameApp` is a high-level SDK runtime, not a new Rust ABI layer. It centralizes the app setup sequence: create Rust engine runtime, create native backend, resolve executable-relative assets, load project/scene text data, create the Win32 window, initialize the DirectX 11 renderer, create the minimal audio engine, run the frame loop, and clean up in reverse ownership order. It accepts the existing static `zeno::GameModule` path and a Windows-loaded `zeno::DynamicGameModule` wrapper.

If `on_init` fails after `GameApp` has created the runtime context, `GameApp` treats the module as partially initialized and calls `on_shutdown` once before resetting SDK-owned resources. This gives the module a chance to release SDK wrappers it created during `on_init` while backend/audio handles are still valid. If both `on_init` and cleanup fail, the cleanup failure is returned because it may indicate leaked or invalid cleanup work. Module `on_shutdown` implementations must therefore be idempotent over their own partial state and must not assume `on_update` or `on_render` ran.

The `GameApp` frame loop keeps the engine frame clock separate from renderer frame commands:

```text
engine begin_frame
  poll window/input
  module on_update(context)
  module on_render(context)
    renderer begin_frame
    clear/draw
    present
engine end_frame
  frame pacing
  max-test-frame check
```

`engine begin_frame` computes `frame_index` and `delta_time_seconds` before game update/render work. `engine end_frame` applies target-FPS pacing after module update/render/present and then updates max-test-frame shutdown state. Renderer `begin_frame` remains the native DirectX 11 render-target binding step; it does not compute game delta time.

## Game Module And Sample

`samples/sample_game_cpp` statically links a small game module into the sample executable and launches it through `zeno::GameApp`. The module implements:

- `on_init`,
- `on_update`,
- `on_render`,
- `on_shutdown`.

The current sample loads project and scene startup data, changes the DirectX 11 clear color over time, updates SDK scene objects, moves a player sprite with keyboard input, checks sample-owned AABB overlap against a triangle goal and cube obstacle, tracks a short console score, supports Space restart, draws collision debug rectangles, draws a colored triangle, a materialized texture-backed sprite, and a materialized indexed cube mesh through the SDK, then exits cleanly after a short demo loop.

The game module is statically linked into the sample executable. `samples/dynamic_module_cpp` separately builds a DLL module and a headless host executable that loads it with `LoadLibraryW`, resolves `zeno_get_game_module`, validates descriptor size and API version, calls lifecycle callbacks, and unloads the DLL.

The dynamic module ABI is intentionally narrow. `sdk/cpp/include/zeno/game_module_abi.h` exposes only macros, fixed-width fields, `ZenResultCode`, function pointers, and an opaque reserved host context. It does not expose `zeno::GameContext`, SDK classes, Win32 handles, DirectX types, STL containers, references, templates, exceptions, or Rust internals. Hot reload and scripting are not implemented.

`templates/game-cpp` follows the same static-linked module shape as the sample. It is not a generator, installer, or plugin loader; it is a small buildable target that proves another game executable can reuse the SDK/runtime/package path without copying engine internals.

`examples/external-game` is deliberately outside the repository build graph. It uses `find_package(ZENO CONFIG REQUIRED)` and links to `ZENO::zeno_sdk_cpp` from `build/package-sdk/...`, proving that a CMake project can consume the packaged SDK without repo-relative include paths.

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

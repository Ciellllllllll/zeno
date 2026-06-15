# ZENO Architecture

ZENO is organized as a small vertical engine slice. The goal is to keep ownership and language boundaries easy to explain.

```text
Sample Game
    ↓
C++ Game Module
    ↓
C++ Game SDK
    ↓ C ABI / handle APIs
Rust Engine Core
    ↓ C ABI / handle APIs
C++ Native Backend
    ↓
Windows / DirectX 11
```

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
- DirectX 11 device/context/swap-chain setup,
- render target creation,
- clear/present operations,
- backend-owned render resources.

The public backend API exposes handles and POD structs, not Win32 or DirectX objects.

## C++ Game SDK

`sdk/cpp` is the game-facing convenience layer. It provides small C++ wrappers such as `zeno::Engine`, `zeno::NativeBackend`, `zeno::Result`, and game-module callback helpers.

SDK classes are allowed for game code ergonomics, but they do not cross the Rust/C++ ABI boundary.

## Game Module And Sample

`samples/sample_game_cpp` statically links a small game module into the sample executable. The module implements:

- `on_init`,
- `on_update`,
- `on_render`,
- `on_shutdown`.

The current sample changes the DirectX 11 clear color over time and exits cleanly after a short demo loop.

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

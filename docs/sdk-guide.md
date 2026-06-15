# Portfolio Notes

## What This Project Demonstrates

- A Rust runtime core with explicit lifecycle and error handling.
- A C ABI boundary designed around handles, POD structs, and stable result codes.
- C++ native backend ownership for Win32 and DirectX 11 resources.
- A C++ SDK layer that is ergonomic for game code without leaking through the ABI.
- A small sample game module that proves init/update/render/shutdown flow.
- Practical Windows build workflow using Cargo, CMake, and Visual Studio 2022/MSVC.

## Design Decisions

- Rust is used where ownership and state-machine clarity matter most.
- C++ is used where platform and DirectX 11 integration are most natural.
- C ABI is used instead of Rust/C++ native ABI to keep the boundary compiler-stable and easy to audit.
- DirectX 11 is the first renderer to keep the milestone narrow and demonstrable.
- Static game-module linking is used for the first module phase; dynamic loading should use C ABI entry points later.

## Tradeoffs

- The engine is Windows-only for now. This keeps the first milestone focused but does not prove portability.
- Rendering only clears the screen. This is intentionally honest: the current milestone proves backend initialization and frame presentation, not a full renderer.
- The SDK is intentionally small. It is enough to demonstrate ownership and lifecycle without introducing a framework too early.
- Build scripts are local PowerShell scripts rather than CI. This keeps the workflow practical before adding public automation.

## Difficult Parts

- Keeping Rust, C ABI, C++ SDK, and C++ native backend responsibilities separate.
- Avoiding convenient but unsafe ABI shortcuts such as C++ classes, STL types, or Rust layouts across the boundary.
- Making DirectX 11 ownership internal while still exposing a usable render path.
- Keeping the portfolio scope honest and small enough to finish.

## Known Limitations

- No input system.
- No mesh, sprite, texture, or shader abstraction beyond the DirectX 11 clear path.
- No asset pipeline.
- No dynamic game-module loading yet.
- No editor.
- No CI yet.
- No multi-platform support.
- No checked-in screenshots or GIFs yet; capture approved media from the sample window when needed.

## Roadmap

Near-term:

- Add a minimal input API.
- Add triangle or sprite rendering.
- Add handle-based shader, buffer, or texture resources.
- Introduce dynamic C ABI game-module entry points.
- Add conservative Windows CI.

Later:

- Add a simple asset convention.
- Add a richer sample scene.
- Consider DirectX 12 after DirectX 11 is stable.
- Consider editor tooling only after runtime and rendering foundations are mature.

## Interview Talking Points

- The project uses Rust for high-level runtime safety while keeping Windows and DirectX work in C++.
- The C ABI boundary avoids unstable Rust/C++ layout and calling convention issues.
- Handles prevent external callers from depending on internal Rust, C++, Win32, or DirectX object layouts.
- The sample is deliberately modest: the C++ host proves engine boot, window creation, DirectX 11 presentation, SDK use, static-linked game-module lifecycle, and clean shutdown.
- The next most valuable technical step is rendering a simple triangle or sprite with handle-owned resources, not adding an editor or multi-platform abstraction.

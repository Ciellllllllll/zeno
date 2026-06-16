# Roadmap

ZENO is at a v0 usable milestone: it can build, run, and package a small Windows/DirectX 11 sample and a minimal C++ template game through the same Rust runtime, C ABI, native backend, and C++ SDK path.

## v0 Baseline

- Rust owns high-level runtime state.
- C++ owns the Windows, DirectX 11, XAudio2, and SDK-facing implementation.
- Rust/C++ communication uses C ABI handles, POD data, and result codes.
- The sample game demonstrates input, scene startup data, texture/sprite rendering, mesh rendering, materials, audio effects, AABB checks, debug draw, and clean shutdown.
- The template game demonstrates how a second static-linked game executable uses `zeno::GameApp`.
- The package script creates a local runtime layout for sample and template executables, assets, config, and `zeno_abi.dll`.
- Windows CI runs the current headless baseline: format, ABI scan, Cargo tests, CMake build, headless CTest, and package creation.

## Near-Term

- Add a small public media capture after the sample window has approved screenshots or GIFs.
- Add dynamic game-module loading through C ABI entry points.
- Harden Rust C ABI panic containment.

## Later

- Add a minimal asset convention or importer for common sample resources.
- Expand renderer features only after the current DirectX 11 path remains stable.
- Consider DirectX 12 after the DirectX 11 baseline is mature.
- Consider editor tooling only after runtime, SDK, packaging, and module loading are stable.

## Not Planned For v0

- Linux, macOS, Vulkan, DirectX 12, Metal, or OpenGL.
- Unity/Unreal-scale editor or asset pipeline.
- Scripting, networking, marketplace/package manager, or installer.
- Full physics, ECS, animation, font/text rendering, or production audio pipeline.

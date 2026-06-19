# SDK Compatibility Policy

This policy defines how ZENO SDK compatibility is handled during pre-1.0 maturity work.

It does not publish `v1.0.0`, create a GitHub Release, create a git tag, upload release assets, add gameplay samples, add engine features, redesign SDK/API/native/backend internals, or authorize committing generated SDK ZIPs or build outputs.

## Compatibility Scope

The compatibility scope is the SDK-visible surface classified in `docs/public-sdk-surface.md`.

The primary compatibility target is:

- Windows 10 or Windows 11
- Visual Studio 2022 / MSVC v143
- C++20 SDK consumers
- DirectX 11 as the default and currently supported native renderer backend
- packaged SDK consumption through `find_package(ZenoEngine CONFIG REQUIRED)`
- packaged target `ZenoEngine::zeno_sdk_cpp`

DirectX 12 is planned future work only. No compatibility promise is made for DirectX 12 until a later explicit phase implements, validates, documents, and promotes it. No compatibility promise is made for Linux, macOS, Vulkan, Metal, OpenGL, editor workflows, scripting, networking, hot reload, installers, or production asset pipelines.

## Public C++ SDK Compatibility

Stable public C++ SDK changes should preserve source compatibility when practical.

Allowed compatible changes:

- adding a new non-required overload
- adding a new type, enum value, helper, or function that does not change existing call behavior
- adding documentation
- strengthening diagnostics without changing result-code semantics
- fixing a bug while preserving the documented contract
- adding optional sample/tutorial coverage for an existing API

Potentially breaking changes:

- removing or renaming a public type, function, enum value, field, or CMake target
- changing public function signatures
- changing ownership or lifetime rules
- changing required call order
- changing return-code behavior for documented cases
- changing default behavior in a way that breaks existing sample or external-game code
- changing the default renderer backend away from DirectX 11
- requiring game code to use DirectX-specific APIs or headers
- moving a stable public item to experimental or private classification

Pre-1.0 may still allow breaking changes, but they must be explicit, justified, documented, and treated as release-blocking until reviewed.

## C ABI Compatibility

The C ABI has stricter rules than the C++ SDK surface.

Allowed C ABI patterns:

- handles with explicit invalid/null values
- POD structs
- primitive integer and floating-point fields
- fixed-size arrays
- borrowed pointers with explicit lifetime rules
- explicit byte lengths for text or buffers
- `size` and `api_version` fields for extensible structs
- `ZenResultCode` status returns when failure is possible

Forbidden across the C ABI:

- C++ classes
- `std::string`
- `std::vector`
- templates
- references
- exceptions
- DirectX COM types
- DirectX 11 or DirectX 12 devices, contexts, swap chains, command queues, command lists, descriptor heaps, fences, resources, views, pipeline states, root signatures, DXGI adapter objects, or shader compiler objects
- Win32 types
- XAudio2 types
- Rust internal layouts

Potentially breaking C ABI changes:

- changing existing enum numeric values
- changing handle layout or invalid-handle meaning
- changing struct field order, size, alignment, or required initialization
- removing an exported function
- changing ownership or lifetime rules for a pointer, handle, or buffer
- changing the success/failure meaning of an existing result code
- allowing panics or exceptions to cross the boundary

Any C ABI change must be recorded in the phase report and reflected in public docs before a release candidate.

## Experimental Public SDK

Experimental public SDK is visible to consumers but not yet guaranteed as final 1.0 contract.

Rules:

- It must be documented as experimental.
- It may change before 1.0.
- It must still obey ABI safety rules when it crosses the C ABI.
- It must not be used to claim mature 1.0 support.
- Changes must update `docs/public-sdk-surface.md`.

Experimental status is not a loophole for hidden breakage. If a change affects packaged consumers or samples, document it.

## Demonstration-Only Surface

Demonstration-only sample and template files may change to keep examples clear.

Rules:

- Do not describe demonstration-only helpers, assets, gameplay constants, or sample internals as stable API.
- Keep samples aligned with the public SDK contract.
- If a sample depends on experimental API, say so in the relevant docs.
- Do not add gameplay samples in phases that explicitly prohibit them.

## Deprecation Rules

Before 1.0, deprecation is mostly documentation-based.

When a stable public item is planned for removal or replacement:

1. Mark it as deprecated in API docs or compatibility notes.
2. Explain the replacement path.
3. Keep it working through at least one release candidate unless the item is unsafe or actively misleading.
4. Record the deprecation in the phase report.
5. Update `docs/public-sdk-surface.md`.

After 1.0, removals should wait for a later major-version plan unless there is a security, correctness, or ABI-safety reason.

## Breaking-Change Rules

A breaking SDK-visible change before 1.0 is allowed only when all of the following are true:

- The change is necessary for a clearer 1.0 contract.
- The affected classification is identified.
- The old behavior and new behavior are documented.
- Samples, templates, tutorials, and API docs are updated in the same phase when affected.
- The phase report lists the breaking change under Public API / ABI Changes.
- The 1.0 stability gate treats the change as release-blocking until reviewed.

Breaking changes are not allowed as incidental cleanup.

## Release-Blocking SDK Surface Changes

The following block release candidate promotion until resolved or explicitly waived in a phase report:

- Any packaged `include/zeno/` file not listed in `docs/public-sdk-surface.md`.
- Any private implementation header copied into `include/zeno/`.
- Any forbidden type crossing the C ABI.
- Any undocumented stable public SDK addition, removal, or signature change.
- Any undocumented C ABI function, struct, enum, handle, ownership, or lifetime change.
- Any stable public SDK item moved to experimental/private without migration notes.
- Any renderer backend change that exposes DirectX native types in public SDK or C ABI headers.
- Any renderer backend change that requires game code to call DirectX-specific APIs.
- Any docs or release notes claim that DirectX 12 support is complete before explicit implementation and validation phases.
- Any silent change away from DirectX 11 as the default supported renderer backend.
- Any generated SDK ZIP, build output, executable, DLL, LIB, PDB, Cargo `target` output, or CMake build tree staged for commit.
- Any package or docs claim that implies unsupported platform, renderer, editor, scripting, networking, hot reload, installer, or production asset pipeline support.
- Any external package consumer that needs repository-private include paths.
- Any package text file that leaks private local absolute paths.

## Future Phase Requirements

Every future phase that touches SDK-visible files must include:

- classification of changed files/concepts
- compatibility assessment
- public API / ABI change notes in the phase report
- updates to `docs/public-sdk-surface.md` when classification changes
- updates to this policy when compatibility rules change
- updates to `docs/1.0-stability-gate.md` when checks change
- updates to `docs/renderer-backend-strategy.md` when renderer backend policy, selection, capability reporting, or DirectX 12 planning changes

If the phase is implementation-only and no SDK-visible surface changed, the phase report should say so explicitly.

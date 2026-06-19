# Renderer Backend Strategy

This document defines the backend-neutral renderer strategy for future DirectX 11 and DirectX 12 work.

Phase57 is planning and foundation documentation only. It does not implement full DirectX 12 rendering, replace the DirectX 11 backend, break existing DirectX 11 behavior, expose DirectX native types in public SDK or C ABI headers, require game code to use DirectX-specific APIs, publish `v1.0.0`, create a GitHub Release, create a git tag, upload release assets, commit generated SDK ZIPs or build outputs, or claim DirectX 12 support is complete.

## Current Support Position

DirectX 11 is the default renderer backend and the only currently supported renderer backend.

DirectX 12 is planned future work. It is not complete, not selectable by game code, not validated by samples, and not part of the current SDK support claim.

Any future DirectX 12 work must preserve existing DirectX 11 behavior until a later explicit phase changes support status.

## Backend-Neutral Goal

Game code should describe what it wants to render, not which native graphics API object should do the work.

The public SDK should remain centered on backend-neutral concepts:

- `GameApp`
- `NativeBackend` as an SDK wrapper over opaque backend handles
- `ResourceManager`
- SDK resource IDs such as `TextureId`, `MaterialId`, `MeshId`, and `TriangleId`
- `Scene`, `SpriteRenderer`, `MeshRenderer`, and `TriangleRenderer`
- `Color`, `Transform`, `Mat4`, `Camera`, and POD matrix data
- `MeshVertex`, index data, and current material state descriptors
- backend-neutral frame commands: begin, clear, set camera, draw, debug draw, present
- `zeno::Result` and `ZenResultCode`

These concepts may map to different native implementations later. They must not require game code to include DirectX 11 or DirectX 12 headers.

## Current Renderer Audit

### SDK-Facing Concepts

The current SDK-visible renderer concepts are:

- `zeno::NativeBackend`
- `zeno::RenderTriangle`
- `zeno::VertexShader`
- `zeno::PixelShader`
- `zeno::Texture`
- `zeno::Mesh`
- `zeno::Material`
- `zeno::ResourceManager`
- `zeno::Scene`
- `zeno::SpriteRenderer`
- `zeno::MeshRenderer`
- `zeno::TriangleRenderer`
- `zeno::MaterialDesc`, `MaterialKind`, `BlendMode`, `DepthMode`, and `CullMode`
- `zeno::SpriteDrawDesc`
- `zeno::DebugTextDesc`
- `zeno::MeshVertex`
- `zeno::Camera`, `Mat4`, and `Transform`

These are SDK concepts. They are not DirectX object contracts.

### C ABI Concepts

The current C ABI renderer concepts are:

- opaque backend and renderer resource handles
- POD config, input, shader log, vertex input, sprite draw, mesh, material, debug draw, and matrix descriptors
- primitive enum values for material, blend, depth, cull, vertex semantic, and vertex format choices
- borrowed pointers with explicit byte counts for shader source, image bytes, vertex bytes, index data, and debug text
- `ZenResultCode` returns

These are allowed to remain visible only if they stay DirectX-neutral at the type level. Header comments may mention the current DirectX 11 implementation, but C ABI types must not expose `ID3D11*`, `ID3D12*`, `IDXGI*`, `HWND`, `D3D*`, `DXGI*`, `XAudio*`, WIC objects, COM pointers, or Rust layouts.

### Backend-Private Concepts

The following remain backend-private implementation details:

- `DirectX11Renderer`
- DirectX device, context, swap chain, render target, depth, shader, buffer, input layout, sampler, blend, rasterizer, and depth-stencil objects
- DirectX shader bytecode and compiler objects
- DXGI objects
- COM pointers and `Microsoft::WRL::ComPtr`
- Win32 window handles and messages
- WIC texture decode objects
- XAudio2 objects
- backend registries and native resource maps
- shader source strings embedded in `native_backend.cpp`
- any future DirectX 12 device, command queue, command list, descriptor heap, root signature, pipeline state, resource barrier, fence, or allocator object

These must not be exposed in public SDK headers, C ABI headers, public docs as required user types, samples as required user code, or package configuration.

## Backend Selection Policy

The current backend selection policy is:

1. DirectX 11 is selected by default.
2. DirectX 11 remains the only supported backend.
3. Game code does not select DirectX 12 in this phase.
4. Any future backend selector must be backend-neutral, such as an SDK enum or config value, not a DirectX native object.
5. Unknown or unavailable backends must fail through `zeno::Result` or `ZenResultCode`.
6. Default selection must remain stable and must not silently change existing DirectX 11 behavior.
7. Build and package defaults must continue to validate the DirectX 11 path.

A later phase may add a backend selection field or capability query, but it must update the public SDK surface inventory, compatibility policy, readiness criteria, API docs, release notes, and phase report.

## Capability Reporting Policy

Future capability reporting should answer backend-neutral questions:

- which backend is active
- whether the backend is supported, experimental, or unavailable
- whether texture, mesh, material, debug draw, debug text, resize, shader compilation, and presentation paths are available
- which limits apply in primitive terms, such as maximum vertex input elements or supported sample feature set

Capability reporting must not expose:

- DirectX interfaces or COM pointers
- Win32 handles
- DXGI adapter objects
- native resource pointers
- descriptor heap handles
- command queues or command lists
- shader compiler objects

Capability reporting should use:

- fixed integer enums
- POD structs with `size` and `api_version`
- booleans or bitmasks
- fixed-size scalar fields
- copied strings only when ownership is explicit
- `ZenResultCode` or `zeno::Result`

Until such reporting exists, docs must not imply runtime DirectX 12 capability detection.

## Public SDK Rules

The public SDK must stay backend-neutral:

- Do not require game code to include DirectX headers.
- Do not expose DirectX object lifetimes to game code.
- Do not require game code to branch on DirectX-specific resource types.
- Do not place DirectX-specific handles in `zeno::Scene`, `ResourceManager`, or SDK resource IDs.
- Do not make sample gameplay depend on a DirectX-specific API.
- Keep `ResourceManager` IDs SDK-owned and backend-neutral.
- Keep camera, transform, color, mesh vertex, material, and debug draw descriptions expressed with SDK or POD types.

DirectX-specific details may be mentioned as current implementation notes, but they must not become required SDK usage.

## C ABI Rules

The C ABI must stay backend-neutral at the type level:

- Use opaque handles for backend and resource ownership.
- Use POD descriptors for renderer commands.
- Use primitive enums for backend-neutral choices.
- Use explicit `size` and `api_version` fields for extensible structs.
- Use borrowed pointers only with explicit lengths and lifetime rules.
- Return `ZenResultCode` for fallible calls.

The C ABI must not expose C++ classes, STL containers, references, templates, exceptions, DirectX COM types, Win32 types, XAudio types, WIC types, DXGI types, or Rust internal layouts.

## Documentation Rules

Public docs may say:

- DirectX 11 is the current default and supported backend.
- DirectX 12 is planned future work.
- The renderer strategy is backend-neutral at the SDK boundary.
- The current implementation is Windows/MSVC/DirectX 11 first.

Public docs must not say:

- DirectX 12 is implemented.
- DirectX 12 is supported.
- DirectX 12 is selectable by game code.
- ZENO has a complete multi-backend renderer.
- ZENO supports Vulkan, Metal, OpenGL, Linux, or macOS.

## Future DirectX 12 Phases

Future DirectX 12 work should be split into small explicit phases:

1. Internal renderer backend interface proposal: define private native C++ interfaces and ownership rules without changing public SDK or C ABI.
2. DirectX 11 adapter phase: move the existing DirectX 11 implementation behind the private interface while preserving behavior and tests.
3. Backend-neutral selection and capability API phase: add public POD/SDK selection and reporting only after the private backend interface is stable.
4. DirectX 12 bootstrap phase: create device/swap-chain/command infrastructure privately without exposing DX12 types or claiming feature parity.
5. DirectX 12 resource parity phase: add texture, mesh, material, debug draw, and presentation parity incrementally behind existing SDK concepts.
6. DirectX 12 validation phase: add backend-specific smoke tests, capability checks, package/readiness docs, and sample validation.
7. Experimental opt-in phase: allow DX12 selection only when docs, tests, and fallback behavior are ready.
8. Support promotion phase: promote DX12 from experimental to supported only after explicit readiness criteria pass.

Each phase must preserve DirectX 11 as the default unless that phase explicitly changes the policy.

## Release Blocking Rules

The following block any future 1.0 or renderer-backend release candidate:

- DirectX native type appears in public SDK headers.
- DirectX native type appears in C ABI headers.
- Game code is required to use DirectX-specific APIs.
- DirectX 11 behavior regresses without an explicit approved breaking-change phase.
- DirectX 12 is described as complete before implementation and validation phases pass.
- Backend selection silently changes the default away from DirectX 11.
- Capability reporting exposes native graphics objects.
- Generated SDK ZIPs or build outputs are staged.
- Release publication, tag creation, or asset upload occurs without an explicit release phase.

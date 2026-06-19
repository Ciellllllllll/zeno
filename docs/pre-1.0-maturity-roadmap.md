# Pre-1.0 Maturity Roadmap

This roadmap changes the project direction from immediate `v1.0.0` publication to pre-1.0 maturity planning.

`v1.0.0` publication is deferred. Do not publish `v1.0.0`, create a `v1.0.0` GitHub Release, create a git tag, upload release assets, or commit generated SDK ZIPs or build outputs until a later explicit release phase authorizes those actions.

The existing `v0.1.0-rc.1` SDK RC and release-publication workflow remains valuable release infrastructure. It should be preserved as the repeatable artifact, checklist, provenance, package-consumption, and future publication path while the SDK matures toward a defensible 1.0 baseline.

## Maturity Target

For this project, 1.0 maturity means the ZENO SDK is stable enough to present as a small, explainable, Windows-first game SDK rather than just a release candidate package.

The target is not feature breadth. The target is a GSLIB-level SDK baseline:

- clear public SDK boundary
- predictable build and package consumption
- documented lifecycle and ownership
- stable ABI rules
- repeatable validation
- focused samples that prove the advertised workflows
- known limitations that are honest and intentionally scoped
- no hidden dependency on repository-private paths or generated local state

GSLIB in this roadmap means the game-facing SDK library layer: public C++ headers, C ABI headers, CMake package targets, templates, focused samples, API docs, tutorials, package layout, diagnostics, and validation scripts that an external game project would actually consume.

## Preserved Release Infrastructure

The current SDK RC workflow should stay in place and continue to be improved without being treated as an immediate 1.0 publication path.

Preserve:

- `scripts/package-sdk.ps1`
- `scripts/verify-sdk-package-consumption.ps1`
- Debug and Release headless release gates
- external-game package consumption verification
- SDK ZIP structure checks
- SDK package layout documentation
- release candidate checklist
- release notes with known limitations
- release publication runbook for later explicit publication
- manual GitHub Actions artifact workflow as an artifact-only RC path

Do not turn this infrastructure into a published 1.0 release until every readiness criterion in `docs/v1.0.0-readiness-criteria.md` passes and a later explicit release phase authorizes publication.

## GSLIB-Level Maturity Checklist

### Public SDK Surface

- Public SDK headers expose a coherent game-facing API.
- Public headers are documented at concept level, not only by samples.
- Game authors can distinguish stable public API from internal implementation.
- Public, experimental, and demonstration-only API areas are labeled before 1.0.
- Compatibility, deprecation, and breaking-change rules are defined before 1.0.
- Public C++ SDK types do not imply ABI stability across compiler boundaries.
- Rust/C++ communication remains isolated behind C ABI handles, POD structs, primitive values, and result codes.
- No C++ classes, STL containers, references, templates, exceptions, DirectX types, Win32 types, XAudio types, or Rust layouts cross the C ABI.

### Lifecycle And Ownership

- Engine, backend, audio, resources, scene, and game module lifecycle order is documented.
- Shutdown behavior is defined for normal exit and partial initialization failure.
- RAII wrappers and SDK-only IDs have clear ownership rules.
- Backend-owned resource lifetime and invalid-handle behavior are documented.
- Dynamic module loading has an explicit support boundary.

### Build And Package Consumption

- The SDK package is consumed through `find_package(ZenoEngine CONFIG REQUIRED)`.
- External projects do not require repository source paths.
- Debug and Release layouts are separated and documented.
- `zeno_abi.dll` copy rules are automated for template/sample consumers.
- Visual Studio 2022, VS Code CMake Tools, and CLI use the same package graph.
- Package text files do not include private local paths.
- Artifact provenance, workflow inputs, and checksums are recorded for every release candidate.

### Samples And Templates

- The minimal template proves external SDK startup.
- A practical external game template proves the `GameApp` and static `GameModule` authoring path.
- Focused samples prove 2D input/audio and 3D mesh workflows.
- Samples use only public packaged headers and package targets.
- Sample assets are copied and resolved from executable-relative paths.
- Samples demonstrate diagnostics and failure handling.
- Windowed behavior has an inspection story before public 1.0 publication.

### Diagnostics And Errors

- Recoverable failures return `zeno::Result` or C ABI result codes.
- Diagnostic strings are stable enough for users to act on.
- Asset, scene, shader, backend, audio, and ABI failures feed documented diagnostics.
- Panic containment and C ABI failure conversion are hardened before 1.0.

### Documentation

- Getting started docs can take a new user from SDK package to running template.
- API concept docs cover lifecycle, rendering, input, audio, assets, scene, math, and diagnostics.
- Tutorials cover the supported sample workflows.
- Release notes honestly list limitations and blockers.
- Architecture docs explain why Rust, C++, DirectX 11, and C ABI are separated.

### Validation

- Formatting, ABI scan, Cargo tests, CMake build, CTest, and package creation pass.
- Debug and Release headless tests pass.
- SDK package consumption QA passes.
- SDK ZIP structure and public-header inventory are checked.
- No generated package ZIP, build output, executable, DLL, LIB, PDB, Cargo target output, or CMake build tree is committed.
- Manual visual validation requirements are recorded where automation is not yet stable.
- Windowed visual/audio validation has recorded results or an explicit release-owner waiver before 1.0.

## Current SDK Audit

### Mature Enough To Preserve

- Windows/MSVC and DirectX 11 scope is explicit.
- Rust owns high-level runtime state and C++ owns native backend/SDK implementation.
- C ABI headers use handles, POD structs, explicit sizes, API versions, borrowed pointer rules, and result codes.
- C++ SDK uses RAII wrappers for engine, backend, shaders, textures, meshes, materials, audio, sounds, and triangles.
- `GameApp` provides a high-level static-linked game runtime path.
- Dynamic module loading exists as a Windows DLL path with a C-compatible descriptor.
- `ResourceManager` provides SDK-only IDs for scene-owned resources.
- Focused SDK samples cover 2D input/audio and 3D mesh rendering.
- A minimal external CMake template exists.
- SDK packaging creates a versioned external-consumption layout and ZIP.
- Package consumption QA verifies external `find_package` usage.
- Release candidate checklist, release notes, and publication runbook exist.

### Not Yet 1.0 Mature

- The public SDK surface is still shaped by samples and smoke tests more than by a formal stability policy.
- Some lifecycle and ownership rules are documented, but not yet complete enough to serve as a 1.0 contract.
- Dynamic modules do not yet expose meaningful ABI-safe host services.
- Rust C ABI panic containment is still listed as near-term hardening.
- Project and scene loading are strict sample text formats without a complete format specification, compatibility policy, or migration story.
- Rendering is limited to fixed minimal paths and debug primitives.
- Audio is limited to short PCM WAV effects.
- Input is limited to a small keyboard/mouse snapshot.
- The packaged `cpp_empty` template proves SDK consumption but is thinner than a practical `GameApp` game authoring template.
- Windowed sample visual inspection is still manual.
- There is no compatibility policy for SDK API evolution after 1.0.
- There is no final 1.0 release acceptance record.

## Prioritized Future Phases

### P0 - 1.0 Stability Gate

Define and enforce the public SDK stability boundary. This includes API inventory, public/experimental/demo-only labels, compatibility and deprecation rules, documented support status, ABI scan coverage, generated package inventory checks, and a release-blocker list tied to objective verification commands.

### P0 - ABI And Panic Hardening

Harden Rust C ABI panic containment and confirm every fallible exported function reports failure through `ZenResultCode` without leaking Rust internals or panics across the boundary.

### P0 - Package Consumption Hardening

Strengthen SDK package QA so Debug and Release external consumption, asset copying, public-header inventory, ZIP structure, and private-path exclusion are repeatable and recorded.

### P1 - Lifecycle And Diagnostics Contract

Document and test lifecycle behavior for `GameApp`, static modules, dynamic modules, partial initialization failure, shutdown ordering, diagnostics, and invalid resource IDs.

### P1 - SDK API Documentation Completion

Complete API concept docs for the current public surface and link each advertised workflow to a sample, tutorial, or smoke test.

### P1 - External Game Template Upgrade

Add or package a practical external SDK game template that uses `GameApp`, a static `GameModule`, assets, project data, and scene data without depending on repository-internal source paths.

### P1 - Sample Visual Validation

Add a practical visual validation plan for the focused SDK samples and sample game. Automation is preferred only when it is stable; otherwise define a repeatable manual screenshot/GIF checklist.

### P1 - Dynamic Module Host Services

Add minimal ABI-safe host services only after their 1.0 support boundary is specified. Keep the static-linked path stable.

### P2 - Asset And Scene Convention

Formalize a small asset/project/scene convention for sample resources, including `.zproj` and `.zscene` format notes, version field meaning, invalid-case tests, and future compatibility expectations without expanding into a general editor or importer.

### P2 - Release Candidate Iteration

Use the preserved RC infrastructure to produce one or more future RC labels after the P0/P1 maturity phases pass. Each RC should record workflow inputs, provenance, checksum, package QA, windowed/manual validation results or waiver, and known limitations.

### P3 - Explicit 1.0 Publication Phase

Only after all readiness criteria pass, run a separate explicit 1.0 publication phase. That phase may create a tag, GitHub Release, and uploaded assets only if its own instructions explicitly authorize those actions.

## Out Of Scope Before 1.0

- Linux, macOS, Vulkan, DirectX 12, Metal, or OpenGL
- editor tooling
- scripting
- networking
- hot reload
- installer or package manager
- production asset importer
- ECS or physics engine
- large rendering feature expansion
- generalized audio pipeline

These may be future portfolio extensions, but they are not required to make the current Windows/DirectX 11 SDK baseline mature.

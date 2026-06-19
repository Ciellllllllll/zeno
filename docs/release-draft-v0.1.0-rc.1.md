# ZENO SDK v0.1.0-rc.1

This is unpublished draft text for a later explicit release publication phase.

Do not publish this draft, create a tag, sign artifacts, or upload assets during Phase54.

## Release Metadata

- Release title: `ZENO SDK v0.1.0-rc.1`
- Future tag: `sdk-v0.1.0-rc.1`
- Prerelease: yes
- Asset to upload later: `ZenoEngine-SDK-v0.1.0-rc.1.zip`
- Verified SHA-256: `ebe50458edfb8f25f10d08fe8f13ad5681515147c986ffe6a7a164ad99369a29`

## Copy/Paste Release Body

```md
## ZENO SDK v0.1.0-rc.1

This is the first release candidate for the ZENO Engine C++ SDK v0.1.0.

ZENO is a Windows-first portfolio engine built to demonstrate a small, explainable Rust/C++ architecture. This release candidate packages the current external C++ SDK surface for MSVC/CMake consumers.

### Artifact

- `ZenoEngine-SDK-v0.1.0-rc.1.zip`
- SHA-256: `ebe50458edfb8f25f10d08fe8f13ad5681515147c986ffe6a7a164ad99369a29`

### Provenance

The SDK ZIP was produced by the manual GitHub Actions workflow `SDK RC Artifact` with:

- `package_version=0.1.0-rc.1`
- Debug headless tests: passed
- Release headless tests: passed
- SDK package consumption QA: passed
- ZIP structure inspection: passed

### Included

- Public SDK and ABI headers under `include/zeno/`
- Debug and Release MSVC libraries under `lib/`
- Debug and Release `zeno_abi.dll` runtime copies under `bin/`
- Focused SDK samples under `samples/sdk_feature_samples_cpp/`
- Minimal external CMake template under `templates/cpp_empty/`
- SDK setup, layout, API, tutorial, IDE, release notes, and checklist documentation under `docs/`
- CMake package files under `cmake/`

### Highlights

- Rust-owned engine runtime with a C ABI boundary.
- C++ native backend with Windows and DirectX 11 first.
- Public C++ SDK wrappers for engine, backend, resources, scenes, diagnostics, and game module workflows.
- `GameApp` runtime for static-linked sample and template applications.
- External CMake consumption through `find_package(ZenoEngine CONFIG REQUIRED)`.
- Focused 2D input/audio and 3D mesh SDK samples.
- Package consumption QA covering Debug/Release template runs, sample builds, external-game consumption, DLL placement, asset placement, private header exclusion, generated entry exclusion, and local path leak checks.

### Known Limitations

- Windows/MSVC only.
- DirectX 11 only.
- The focused SDK samples are windowed; automated QA builds them and validates assets but does not perform visual inspection.
- Rendering is limited to the current fixed triangle, texture-backed sprite, indexed position/color mesh, material state, debug draw, and debug text paths.
- Audio is limited to short PCM WAV effects.
- Input is limited to keyboard and mouse snapshots.
- Collision remains SDK/sample-owned AABB helper logic.
- Dynamic module loading is Windows DLL based and is not hot reload or an editor plugin system.
- No editor, installer, project generator, asset importer, mesh file format, animation, lighting redesign, scripting, hot reload, networking, or cross-platform backend is included.

### Notes

This release candidate is intended for review and validation. A later release candidate may supersede it if publication checks or user testing find issues.
```

## Future Publication Inputs

Use `docs/release-publication-runbook-v0.1.0-rc.1.md` before publishing.

Required later-phase checks:

- Confirm tag `sdk-v0.1.0-rc.1` does not already exist.
- Confirm release title is `ZENO SDK v0.1.0-rc.1`.
- Confirm the release is marked as a prerelease.
- Upload only `ZenoEngine-SDK-v0.1.0-rc.1.zip`.
- Confirm the uploaded asset checksum matches `ebe50458edfb8f25f10d08fe8f13ad5681515147c986ffe6a7a164ad99369a29`.

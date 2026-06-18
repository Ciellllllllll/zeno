# Draft GitHub Release - ZENO SDK v0.1.0-rc.1

Status: unpublished draft text.

Do not publish this draft until the `SDK RC Artifact` GitHub Actions workflow has passed on GitHub and its uploaded artifact has been inspected.

## Title

ZENO SDK v0.1.0-rc.1

## Tag

Do not create a tag during Phase53.

Suggested tag for a later explicit release phase:

```text
sdk-v0.1.0-rc.1
```

## Summary

This is the first release candidate for the ZENO Engine C++ SDK v0.1.0. It packages the current Windows/MSVC SDK surface for external CMake consumption, including public headers, Debug/Release libraries, the Rust ABI runtime DLL, focused SDK samples, an external template, and public documentation.

The SDK remains a portfolio-oriented Windows-first release candidate. It is intended to demonstrate a small, explainable Rust/C++ engine architecture rather than a general-purpose commercial engine.

## Release Gate

The release candidate artifact must come from the manual GitHub Actions workflow:

```text
SDK RC Artifact
package_version=0.1.0-rc.1
```

That workflow must pass:

- Debug headless tests
- Release headless tests
- SDK package consumption QA

Expected uploaded Actions artifact:

```text
ZenoEngine-SDK-v0.1.0-rc.1
```

Expected file inside the downloaded Actions artifact:

```text
ZenoEngine-SDK-v0.1.0-rc.1.zip
```

## Included Package Contents

- `include/zeno/` public SDK and ABI headers
- `lib/Debug/` and `lib/Release/` MSVC libraries
- `bin/Debug/` and `bin/Release/` `zeno_abi.dll` runtime copies
- `samples/sdk_feature_samples_cpp/` focused SDK samples
- `templates/cpp_empty/` minimal external CMake template
- `docs/` SDK setup, layout, API, tutorial, IDE, release notes, and release checklist docs
- `cmake/ZenoEngineConfig.cmake`
- `cmake/ZenoEngineConfigVersion.cmake`

## Highlights

- Rust-owned engine runtime with C ABI boundary.
- C++ native backend with Windows and DirectX 11 first.
- Public C++ SDK wrappers for engine/backend/resource/game-module workflows.
- `GameApp` runtime for static-linked sample and template applications.
- External CMake package support through `find_package(ZenoEngine CONFIG REQUIRED)`.
- Focused 2D input/audio and 3D mesh SDK samples.
- Headless package consumption QA that validates Debug/Release template runs, sample builds, external-game consumption, DLL placement, asset placement, private header exclusion, forbidden generated entry exclusion, and local path leak checks.

## Known Limitations

- Windows/MSVC only.
- DirectX 11 only.
- No GitHub Release, tag, or signing is performed by the artifact workflow.
- Focused SDK samples are windowed; automated QA builds them and validates assets but does not perform visual inspection.
- Rendering is limited to the current fixed triangle, texture-backed sprite, indexed position/color mesh, material state, debug draw, and debug text paths.
- Audio is limited to short PCM WAV effects.
- Input is limited to keyboard and mouse snapshots.
- Collision remains SDK/sample-owned AABB helper logic.
- No editor, installer, project generator, asset importer, mesh file format, animation, lighting redesign, scripting, hot reload, networking, or cross-platform backend.

## Manual Verification Notes

Fill these before publishing in a later explicit release phase:

```text
Workflow run URL:
Workflow branch/ref:
package_version input:
Debug headless result:
Release headless result:
SDK package consumption QA result:
Downloaded Actions artifact name:
Contained ZIP file:
ZIP top-level directory:
Inspector:
Inspection date:
```

## Promotion Boundary

Publishing this draft, creating a git tag, signing artifacts, or attaching/uploading ZIPs to a release page must happen only in a later explicit release phase.

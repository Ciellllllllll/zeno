# ZENO SDK Release Notes

## v0.1.0-rc.1

`v0.1.0-rc.1` is the first SDK v0.1.0 release candidate package label. The package name is `ZenoEngine-SDK-v0.1.0-rc.1`.

The CMake package keeps `ZenoEngine_VERSION` numeric as `0.1.0` so `find_package(ZenoEngine 0.1.0 CONFIG REQUIRED)` can use normal CMake version comparison. The release-candidate identity is exposed separately as `ZenoEngine_RELEASE_LABEL=0.1.0-rc.1`.

### Release Gate

The SDK package consumption QA path is the release gate for this release candidate:

```powershell
.\scripts\test-headless.ps1 -Configuration Debug
.\scripts\test-headless.ps1 -Configuration Release
.\scripts\verify-sdk-package-consumption.ps1
```

The gate must pass before publishing any release artifact. This phase does not publish a GitHub Release and does not create a git tag.

### Included SDK Scope

- Public C++ SDK headers and C ABI headers under `include/zeno/`.
- Debug and Release MSVC libraries under `lib/`.
- Debug and Release `zeno_abi.dll` runtime copies under `bin/`.
- Minimal external CMake template under `templates/cpp_empty/`.
- Focused public SDK samples for 2D input/audio and 3D mesh rendering under `samples/sdk_feature_samples_cpp/`.
- Public SDK setup, API, tutorial, IDE, layout, release notes, and checklist documentation.

### Known Limitations

- Windows/MSVC only.
- DirectX 11 only.
- The focused SDK samples are windowed; package consumption QA builds them and validates copied assets, but it does not perform automated visual inspection.
- Rendering is limited to the current fixed triangle, texture-backed sprite, indexed position/color mesh, material state, debug draw, and debug text paths.
- There is no editor, project generator, installer, asset importer, mesh file format, animation, lighting redesign, scripting, hot reload, network layer, or cross-platform backend.
- Audio is limited to short PCM WAV effects.
- Input is limited to keyboard and mouse snapshots.
- Collision remains SDK/sample-owned AABB helper logic.
- Dynamic game module loading is Windows DLL based and is not hot reload or an editor plugin system.

### Release Blockers

- Any failure in Debug or Release `test-headless.ps1`.
- Any failure in `verify-sdk-package-consumption.ps1`.
- Any generated SDK ZIP, package directory, build output, executable, DLL, LIB, PDB, CMake build tree, or Cargo target output staged for commit.
- Any private source header, `.codex` planning input, absolute local path, or generated IDE/build file inside the SDK package.
- Any undocumented ABI or public SDK surface change.
- Any new gameplay sample or engine feature added outside the release-candidate preparation scope.

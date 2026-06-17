# Phase 48 Report - SDK Asset Pipeline Stabilization

## Summary

Phase48 stabilizes the SDK-facing sample asset pipeline by documenting the expected packaged asset layout, validating packaged sample assets through the SDK package consumption QA path, and improving missing asset diagnostics with resolved asset paths.

No gameplay samples, engine features, editor tooling, asset database design, hot reload, importer UI, SDK/API redesign, ABI changes, native backend redesign, renderer redesign, audio redesign, input redesign, or scene redesign were added.

## Issue Scope

- Source issue: https://github.com/Ciellllllllll/zeno/issues/41
- In scope: SDK sample asset layout, executable-relative packaged sample assets, package copy validation, package consumption QA asset checks, missing asset diagnostics, and SDK docs.
- Out of scope: generated artifact commits, GUI sample launch automation, new gameplay samples, asset database design, hot reload, importer UI, and engine subsystem redesigns.

## SDK Asset Layout

The packaged SDK sample asset root is:

```text
samples/sdk_feature_samples_cpp/assets/
```

The package script builds this from the source-controlled shared sample assets plus focused SDK sample project and scene files. Expected packaged assets are:

```text
assets/
  sample_manifest.txt
  project.zproj
  projects/
    2d_input_audio.zproj
    3d_mesh.zproj
  scenes/
    sample_scene.zscene
    2d_input_audio.zscene
    3d_mesh.zscene
  shaders/
    sample_triangle.hlsl
  audio/
    sample_click.wav
  textures/
    sample_sprite_2x2.bmp
```

Packaged sample builds copy that directory beside each sample executable as `assets/`. Runtime loads are executable-relative, so CLI, Visual Studio 2022 Open Folder, and VS Code CMake Tools use the same layout after CMake builds the samples.

## Changes

- `scripts/package-sdk.ps1`
  - Added the expected SDK sample asset inventory.
  - Added non-empty required asset checks after package asset copy.
  - Added a collision guard for the two source asset trees before merging them.
- `scripts/verify-sdk-package-consumption.ps1`
  - Added full SDK sample asset inventory validation.
  - Added project and scene graph checks for package and runtime asset roots.
  - Added fixture checks for the BMP texture, PCM WAV sound, HLSL shader entry points, and manifest audio target.
  - Added SHA-256 copy-integrity checks from generated package layout to extracted SDK and from extracted SDK to Debug/Release runtime output.
- `sdk/cpp/src/asset.cpp`
  - Missing/open text and binary asset diagnostics now include the requested path, active root, and resolved path.
- Documentation
  - Documented the packaged SDK sample asset tree and executable-relative runtime contract.
  - Updated sample tutorials with their project, scene, texture, audio, and builtin asset paths.
  - Updated QA docs to note sample asset integrity validation.

## Validation Coverage

The package consumption QA now validates:

- Required Debug/Release SDK libraries and runtime DLLs.
- Public header allowlist and private header exclusion.
- Forbidden generated/local package entries.
- Local absolute path leakage in package text files.
- Extracted SDK sample asset inventory and fixture shape.
- Debug/Release sample runtime asset inventory and copy integrity.
- Project files, initial scene files, scene references, manifest audio reference, BMP texture shape, WAV PCM shape, and HLSL entry points.
- Debug/Release packaged template build/run.
- Debug/Release packaged sample builds.
- Debug/Release external-game build/run through extracted SDK `find_package`.

## Runtime Diagnostics

`AssetRoot::read_text` and `AssetRoot::read_binary` now report missing or unopened assets with:

- the requested relative asset path,
- the active asset root,
- the resolved filesystem path.

This keeps result codes and public SDK headers unchanged while making missing project, scene, texture, audio, shader, and manifest failures easier to diagnose through the existing asset-loading path.

## Validation Results

Executed:

```powershell
.\scripts\test-headless.ps1 -Configuration Debug
.\scripts\test-headless.ps1 -Configuration Release
.\scripts\verify-sdk-package-consumption.ps1 -CMakeExe 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
```

Result: Passed.

Notes:

- This shell did not have `cmake` on `PATH`; `test-headless.ps1` was run with the Visual Studio bundled CMake directory prepended to `PATH`.
- `verify-sdk-package-consumption.ps1` was run with `-CMakeExe` because it supports explicit nested CMake selection.
- Full SDK package consumption QA regenerated the package and ZIP, extracted the ZIP, validated extracted and runtime sample assets, built Debug/Release packaged samples, ran Debug/Release packaged templates, and ran Debug/Release external-game consumers.

## Known Limits

- Windowed packaged samples are built and their assets are validated, but the QA script does not launch GUI windows.
- The SDK package and QA path remain Windows/MSVC-focused.
- The 3D sample uses `builtin:cube`; no mesh file importer or mesh asset file format is added.
- Asset validation uses current source-controlled sample fixtures and avoids hard-coded absolute paths.

## Phase49 Carryover

- Consider adding the full SDK package consumption QA path to CI if runtime is acceptable.
- Consider consolidating narrower package checks around this script if future CI needs one canonical SDK release gate.
- Keep GUI sample launch validation manual until stable window automation is selected.

## SubAgent Inputs

- Asset Layout Agent: identified the merged packaged SDK sample asset tree and executable-relative runtime layout, plus the need to document it explicitly.
- Asset Validation Agent: recommended inventory, project/scene graph, fixture, manifest, and dynamic hash/copy checks without hard-coded golden hashes.
- Runtime Diagnostics Agent: recommended minimal public-SDK-safe missing asset diagnostics without result-code or API changes.
- Packaging QA Agent: identified gaps in sample asset copy validation and recommended non-empty package/runtime asset checks.
- Report and Docs Agent: identified the required docs updates, report structure, validation commands, and known limits.

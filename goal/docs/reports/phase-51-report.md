# Phase 51 Report - Prepare SDK v0.1.0 Release Candidate

## Summary

Phase51 prepares the SDK v0.1.0 release candidate without publishing a GitHub Release, creating a git tag, committing generated SDK ZIPs/build outputs, or adding gameplay samples or engine features.

The release candidate package label is now `0.1.0-rc.1`. The previous `0.1.0-dev` package label was replaced in active package scripts and public docs because this phase is explicitly preparing an RC package. The CMake package version remains numeric `0.1.0` for normal `find_package` version comparison, while the RC identity is exposed as `ZenoEngine_RELEASE_LABEL=0.1.0-rc.1`.

SDK package consumption QA remains the release gate.

## Files Added

- `docs/release-notes.md`
- `docs/release-checklist.md`
- `goal/docs/reports/phase-51-report.md`

## Files Modified

- `README.md`
- `cmake/ZenoEngineConfig.cmake`
- `cmake/ZenoEngineConfigVersion.cmake`
- `docs/build-guide.md`
- `docs/getting-started.md`
- `docs/index.md`
- `docs/sdk-guide.md`
- `docs/sdk-layout.md`
- `docs/tutorials/01-create-empty-project.md`
- `docs/tutorials/02-build-and-run-sdk-template.md`
- `docs/tutorials/03-use-2d-input-audio-sample.md`
- `docs/tutorials/04-use-3d-mesh-sample.md`
- `docs/vs2022.md`
- `docs/vscode-cmake.md`
- `examples/external-game/README.md`
- `scripts/package-sdk.ps1`
- `scripts/verify-external-game.ps1`
- `scripts/verify-sdk-package-consumption.ps1`
- `templates/cpp_empty/README.md`

## Public API / ABI Changes

None.

No engine feature, gameplay sample, SDK API surface, native backend API, Rust ABI, or C ABI shape was changed. The CMake package metadata now exposes `ZenoEngine_RELEASE_LABEL` and compatibility `ZENO_RELEASE_LABEL` variables, but no code-facing SDK or ABI function/type was added.

## Tests Added or Updated

No runtime tests were added.

Release candidate documentation now defines the repeatable release gate:

- `.\scripts\test-headless.ps1 -Configuration Debug`
- `.\scripts\test-headless.ps1 -Configuration Release`
- `.\scripts\verify-sdk-package-consumption.ps1`

`verify-sdk-package-consumption.ps1` now defaults to `0.1.0-rc.1` and validates that package label through generation, ZIP extraction, packaged template builds/runs, packaged sample builds, external-game consumption, asset integrity, private header exclusion, forbidden generated entry exclusion, and local path leak checks.

## Build / Test Result

Executed:

```powershell
$env:PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;$env:PATH"
.\scripts\test-headless.ps1 -Configuration Debug
.\scripts\test-headless.ps1 -Configuration Release
.\scripts\verify-sdk-package-consumption.ps1 -CMakeExe 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
```

Result: Passed.

Notes:

- This shell did not have `cmake` or `ctest` on `PATH`, so the Visual Studio bundled CMake directory was prepended for `test-headless.ps1`.
- `verify-sdk-package-consumption.ps1` was run with `-CMakeExe` because it supports explicit nested CMake selection.
- Debug headless CTest reported 21/21 tests passed.
- Release headless CTest reported 21/21 tests passed.
- SDK package consumption QA generated and validated `build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1/` and `build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1.zip`.
- SDK package consumption QA reported `SDK package consumption QA passed`.
- Generated package directories, ZIPs, build trees, executable outputs, DLLs, LIBs, and Cargo outputs remain generated local artifacts and were not staged for commit.

## Deviations From Spec

None.

## Follow-Up Work

- Publish a GitHub Release only in a later explicit release phase.
- Create a git tag only in a later explicit release phase.
- If another RC is required, bump the package label consistently across package scripts, release notes, checklist, and public docs.
- Consider manual windowed sample visual inspection before final release promotion if a release owner wants visual confirmation beyond the headless package gate.

## Release Blockers

Current blockers: None after the Phase51 gate passed.

Future blocker conditions are documented in `docs/release-notes.md` and `docs/release-checklist.md`, including any failed release-gate command, generated artifacts staged for commit, private package contents, local absolute path leaks, undocumented API/ABI changes, or out-of-scope engine/gameplay additions.

## SubAgent Inputs

- SpecGuard / API Surface: confirmed the phase stayed within release-candidate metadata/docs/checklist scope and did not add engine features, gameplay samples, tags, GitHub releases, generated artifacts, SDK APIs, or ABI changes.
- Rust Core / ABI: confirmed Rust crate versions remain `0.1.0`; no Rust ABI layout or function changed.
- Native Backend / Platform: confirmed no native backend, DirectX, Win32, audio, input, asset, renderer, or scene implementation changed.
- C++ SDK / Sample / Template: confirmed packaged template/sample docs now point at `ZenoEngine-SDK-v0.1.0-rc.1` and package metadata exposes a separate release label.
- Build / Test / Docs: confirmed SDK package consumption QA is the RC gate and all required Debug, Release, and package-consumption commands passed.

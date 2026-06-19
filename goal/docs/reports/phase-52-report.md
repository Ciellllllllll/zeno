# Phase 52 Report - Add SDK RC Artifact Workflow

## Summary

Phase52 adds a manual GitHub Actions workflow that generates the SDK v0.1.0 RC ZIP artifact after running the release gate. The workflow is artifact-only: it does not publish a GitHub Release, create a git tag, sign artifacts, commit generated SDK ZIPs/build outputs, or add gameplay samples or engine features.

The workflow uploads `ZenoEngine-SDK-v0.1.0-rc.1.zip` as the Actions artifact `ZenoEngine-SDK-v0.1.0-rc.1`.

## Files Added

- `.github/workflows/sdk-rc-artifact.yml`
- `goal/docs/reports/phase-52-report.md`

## Files Modified

- `docs/build-guide.md`
- `docs/release-checklist.md`
- `docs/release-notes.md`

## Public API / ABI Changes

None.

No SDK API, Rust ABI, C ABI, native backend, renderer, audio, input, asset, scene, or sample/gameplay surface changed.

## Tests Added or Updated

No runtime tests were added.

Added the manual GitHub Actions workflow `SDK RC Artifact`, which runs these gates before upload:

1. `.\scripts\test-headless.ps1 -Configuration Debug`
2. `.\scripts\test-headless.ps1 -Configuration Release`
3. `.\scripts\verify-sdk-package-consumption.ps1 -PackageVersion '${{ inputs.package_version }}'`

Only after those steps pass does the workflow verify the generated ZIP path and upload it with `actions/upload-artifact`.

## Build / Test Result

Executed locally:

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
- SDK package consumption QA generated and validated `build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1.zip`.
- SDK package consumption QA reported `SDK package consumption QA passed`.
- The GitHub artifact upload step itself was not executed locally; it will run inside GitHub Actions when `SDK RC Artifact` is manually dispatched.

## Deviations From Spec

None.

## Follow-Up Work

- Manually run `SDK RC Artifact` from GitHub Actions when an RC artifact is needed.
- Publish a GitHub Release only in a later explicit release phase.
- Create a git tag only in a later explicit release phase.
- Add signing only in a later explicit signing/release phase.

## Workflow Usage

1. Open the repository Actions tab.
2. Select `SDK RC Artifact`.
3. Click `Run workflow`.
4. Leave `package_version` as `0.1.0-rc.1` for this RC.
5. Wait for Debug headless, Release headless, and SDK package consumption QA to pass.
6. Open the completed run and download the artifact named `ZenoEngine-SDK-v0.1.0-rc.1`.
7. Extract that downloaded Actions artifact archive to retrieve `ZenoEngine-SDK-v0.1.0-rc.1.zip`.

## SubAgent Inputs

- SpecGuard / API Surface: confirmed the workflow is manual artifact generation only and does not publish releases, create tags, sign artifacts, commit generated outputs, or add features/samples.
- Rust Core / ABI: confirmed no Rust or ABI source changes were required.
- Native Backend / Platform: confirmed no native/backend/platform implementation changed.
- C++ SDK / Sample / Template: confirmed no SDK sample or template behavior changed.
- Build / Test / Docs: confirmed the workflow orders release-gate commands before artifact upload and docs explain how to run the workflow and retrieve the artifact.

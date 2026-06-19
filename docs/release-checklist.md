# SDK v0.1.0 Release Candidate Checklist

Use this checklist before promoting an SDK v0.1.0 release candidate.

## Version And Scope

- Confirm the package label is `0.1.0-rc.1` or the intended next RC label.
- Confirm `scripts/package-sdk.ps1` and `scripts/verify-sdk-package-consumption.ps1` default to the same package label.
- Confirm `ZenoEngineConfig.cmake` exposes numeric `ZenoEngine_VERSION=0.1.0`.
- Confirm `ZenoEngineConfig.cmake` exposes `ZenoEngine_RELEASE_LABEL` with the RC label.
- Confirm public docs use the same package directory name.
- Confirm no git tag is created during RC preparation.
- Confirm no GitHub Release is published during RC preparation.
- Confirm no generated SDK ZIP or build output is staged.

## Release Gate

Run from the repository root:

```powershell
.\scripts\test-headless.ps1 -Configuration Debug
.\scripts\test-headless.ps1 -Configuration Release
.\scripts\verify-sdk-package-consumption.ps1
```

The release gate passes only when all three commands pass.

## GitHub Actions Artifact Workflow

The manual workflow `.github/workflows/sdk-rc-artifact.yml` runs the same release gate before uploading an SDK ZIP artifact.

To run it:

1. Open the repository Actions tab.
2. Select `SDK RC Artifact`.
3. Use `Run workflow`.
4. Keep `package_version` as `0.1.0-rc.1` unless preparing a later RC label.
5. Wait for `Debug headless tests`, `Release headless tests`, and `SDK package consumption QA` to pass.

If `SDK RC Artifact` is not listed, confirm `.github/workflows/sdk-rc-artifact.yml` is present on the repository default branch. GitHub only offers manual workflow dispatch for workflows available to Actions on GitHub.

To retrieve the artifact:

1. Open the completed workflow run.
2. Download the artifact named `ZenoEngine-SDK-v0.1.0-rc.1`.
3. Extract the downloaded artifact archive to get `ZenoEngine-SDK-v0.1.0-rc.1.zip`.

The workflow does not publish a GitHub Release, create a git tag, sign artifacts, or commit generated package outputs.

## Package Audit

- Confirm the package root is under `build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1/`.
- Confirm the ZIP extracts to exactly one top-level SDK directory.
- Confirm `include/zeno/` contains only public headers.
- Confirm `lib/Debug`, `lib/Release`, `bin/Debug`, and `bin/Release` contain the expected artifacts.
- Confirm packaged templates build and run one headless frame in Debug and Release.
- Confirm packaged SDK feature samples build in Debug and Release.
- Confirm external-game consumes the extracted SDK through `find_package`.
- Confirm packaged sample assets are present in the extracted SDK and copied runtime directories.
- Confirm package text files do not contain local absolute paths.

## Known Limits Review

- Confirm `docs/release-notes.md` lists current limitations.
- Confirm any new blocker is documented before release promotion.
- Confirm windowed sample visual inspection remains manual unless a later phase adds stable automation.

## Promotion Boundary

Passing this checklist means the SDK package is ready to be considered a release candidate. Publishing a GitHub Release, creating a git tag, signing artifacts, or uploading generated ZIPs is a separate explicit phase.

Before any later publication phase, use `docs/release-publication-runbook-v0.1.0-rc.1.md` for tag, asset upload, rollback/correction, and post-publish checks.

# Phase 47 Report - SDK Package Consumption QA Automation

## Summary

Phase47 adds deterministic SDK package consumption QA automation. The new script generates the SDK package and ZIP, extracts the generated ZIP into a clean ignored validation directory, and validates the extracted package as an SDK consumer would use it.

No gameplay samples, engine features, SDK API changes, renderer changes, audio changes, input changes, scene changes, asset-system changes, ABI changes, or native backend redesigns were added.

## Issue Scope

- Source issue: https://github.com/Ciellllllllll/zeno/issues/40
- In scope: package generation, ZIP extraction, extracted SDK consumption, Debug/Release template validation, Debug/Release sample build validation, external-game validation through `find_package`, runtime DLL checks, asset checks, library checks, and private header exclusion checks.
- Out of scope: generated artifact commits, GUI/window launch automation, CI artifact upload, release publishing, gameplay samples, and engine features.

## Changes

- Added `scripts/verify-sdk-package-consumption.ps1`.
- Updated `README.md` to mention full packaged SDK consumption QA.
- Updated `docs/build-guide.md` verification scripts and regression matrix with the new command.
- Updated `docs/getting-started.md` to point SDK users to the full package consumption QA command.
- Updated `docs/index.md` to route repository contributors to the documented QA command.

## QA Command

Default command:

```powershell
.\scripts\verify-sdk-package-consumption.ps1
```

The script accepts:

- `-PackageVersion`, default `0.1.0-dev`.
- `-ValidationDir`, default `build/sdk-package-consumption-qa`.
- `-CMakeExe`, default `cmake`.

The validation directory must stay under ignored `build/`.

## Script Coverage

The script performs these checks:

- Runs `scripts/package-sdk.ps1 -Configuration All`.
- Requires `build/package-sdk/ZenoEngine-SDK-v0.1.0-dev.zip`.
- Extracts the ZIP into `build/sdk-package-consumption-qa/`.
- Requires exactly one extracted top-level SDK directory.
- Verifies required Debug and Release libraries are present and non-empty.
- Verifies required Debug and Release `zeno_abi.dll` files are present and non-empty.
- Verifies CMake package config files are present and non-empty.
- Verifies packaged template and sample `CMakePresets.json` files exist.
- Verifies the public header set is exactly the expected SDK header set.
- Rejects private/internal headers and local/generated package entries.
- Scans extracted SDK text files for local absolute path leakage.
- Configures, builds, and runs extracted `templates/cpp_empty` in Debug and Release.
- Configures and builds extracted `samples/sdk_feature_samples_cpp` in Debug and Release.
- Verifies runtime `zeno_abi.dll` placement beside template, sample, and external-game executables.
- Verifies packaged sample asset placement beside sample executables.
- Configures, builds, and runs `examples/external-game` in Debug and Release using `ZenoEngine_DIR` from the extracted SDK.

## Validation Results

Executed:

```powershell
.\scripts\verify-sdk-package-consumption.ps1 -CMakeExe '<VS2022 bundled cmake.exe>'
```

Result: Passed.

Observed successful validation:

- SDK package and ZIP generated.
- ZIP extracted into `build/sdk-package-consumption-qa/`.
- Package artifact integrity checks passed.
- Debug packaged template built and printed `ZENO SDK template ran frame 0`.
- Release packaged template built and printed `ZENO SDK template ran frame 0`.
- Debug packaged samples built.
- Release packaged samples built.
- Debug external game built and printed `ZENO external game linked and ran one headless frame`.
- Release external game built and printed `ZENO external game linked and ran one headless frame`.

## Documentation Updates

- `README.md`: added the full packaged SDK consumption QA command near the CI-style verification path.
- `docs/build-guide.md`: added the command to verification scripts and the regression matrix.
- `docs/getting-started.md`: added the command after the narrower external-game verification commands.
- `docs/index.md`: routed contributors to the documented QA command.

## Generated Artifacts

The QA command writes generated outputs only under ignored directories:

- `build/package-sdk/`
- `build/sdk-package-consumption-qa/`
- existing CMake build trees under `build/`
- Cargo output under `target/`

No generated ZIP, extracted SDK directory, build tree, executable, library, DLL, or local artifact is intended for commit.

## Known Limits

- The focused packaged samples are built but not launched because they are windowed GUI programs.
- The script is Windows/MSVC-focused, matching the current SDK package scope.
- The default command expects `cmake` on `PATH`; environments without it can pass `-CMakeExe`.

## Phase48 Carryover

- Consider adding this script to CI after runtime and duration are acceptable.
- Consider adding an optional file inventory or hash comparison between live package layout and extracted ZIP.
- Keep GUI/window sample launch validation manual until stable automation is selected.
- Consider consolidating narrower package checks around this script if future CI needs one canonical SDK release gate.

## SubAgent Inputs

- QA Script Agent: recommended a full extracted-ZIP consumption script that reuses `package-sdk.ps1`.
- Package Extraction Agent: recommended an ignored `build/` validation directory and ZIP top-level/package-leak checks.
- External Build Agent: confirmed the Debug/Release template, sample, and external-game command paths.
- Artifact Integrity Agent: recommended non-empty library/DLL checks and exact public header allowlist validation.
- Report and Docs Agent: identified minimal README, build guide, getting-started, and docs index updates.

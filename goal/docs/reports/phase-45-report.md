# Phase 45 Report - VS2022 / VS Code SDK Compatibility Validation

## Summary

Phase45 validated the Phase44 SDK distribution across CLI, Visual Studio 2022 Open Folder, VS Code + CMake Tools, Debug, and Release consumption paths. No new game sample was added.

The main compatibility fix is that packaged SDK template and sample roots now receive their own `CMakePresets.json` files. These presets use the same `windows-msvc-debug` and `windows-msvc-release` names as the repository and set `ZenoEngine_DIR` back to the packaged SDK `cmake` directory. The repository `zeno_package_sdk` target now creates the full Debug/Release SDK package from either active Visual Studio preset.

## Issue Scope

- Source issue: https://github.com/Ciellllllllll/zeno/issues/38
- In scope: SDK package generation, external CMake consumption, Debug/Release switching, DLL placement, asset copy behavior, IDE-facing preset documentation, and CLI verification.
- Out of scope: new game samples, SDK/ABI/native redesign, renderer/audio/input/asset-manager feature work, private header exposure, and committing generated ZIP/build outputs.

## Environment

| Item | Value |
| --- | --- |
| OS | Microsoft Windows 11 Home 10.0.26200 |
| Visual Studio toolchain | Visual Studio 2022 Community, MSBuild 17.14.40.60911, MSVC 19.44.35226.0 |
| CMake | 3.31.6-msvc6 from Visual Studio 2022 Community |
| Rust | rustc 1.95.0, cargo 1.95.0 |
| Shell | PowerShell |

Note: this shell did not have `cmake` on `PATH` by default. CLI validation used the Visual Studio bundled CMake path explicitly. `zeno_package_sdk` now passes `${CMAKE_COMMAND}` into `scripts/package-sdk.ps1`, so the Visual Studio target does not depend on the user shell PATH for nested CMake calls.

## Verification Matrix

| Area | Route | Configuration | Command / Action | Mode | Result |
| --- | --- | --- | --- | --- | --- |
| Repository configure | CLI | Debug | `cmake --preset windows-msvc-debug` | Headless | Passed |
| Repository build | CLI | Debug | `cmake --build --preset windows-msvc-debug` | Headless | Passed |
| Repository configure | CLI | Release | `cmake --preset windows-msvc-release` | Headless | Passed |
| Repository build | CLI | Release | `cmake --build --preset windows-msvc-release` | Headless | Passed |
| Headless tests | CLI | Debug | `.\scripts\test-headless.ps1 -Configuration Debug` | Headless | Passed: Rust tests and 19 CTest headless tests |
| Headless tests | CLI | Release | `.\scripts\test-headless.ps1 -Configuration Release` | Headless | Passed: Rust tests, Release ABI build, and 19 CTest headless tests |
| SDK package | CLI | Debug + Release | `.\scripts\package-sdk.ps1` | Headless | Passed |
| SDK package target | VS2022/CMake target | Debug active preset | `cmake --build --preset windows-msvc-debug --target zeno_package_sdk` | Headless IDE-equivalent | Passed; produced Debug and Release package artifacts |
| External game | CLI | Debug | `.\scripts\verify-external-game.ps1 -Configuration Debug` | Headless | Passed; printed `ZENO external game linked and ran one headless frame` |
| External game | CLI | Release | `.\scripts\verify-external-game.ps1 -Configuration Release` | Headless | Passed; printed `ZENO external game linked and ran one headless frame` |
| ZIP extraction | CLI | Debug + Release | `Expand-Archive build/package-sdk/ZenoEngine-SDK-v0.1.0-dev.zip` | Headless | Passed |
| Packaged template | Extracted ZIP + CLI presets | Debug | `cmake --preset windows-msvc-debug -S <sdk>/templates/cpp_empty`, build, run | Headless | Passed; printed `ZENO SDK template ran frame 0` |
| Packaged template | Extracted ZIP + CLI presets | Release | `cmake --preset windows-msvc-release -S <sdk>/templates/cpp_empty`, build, run | Headless | Passed; printed `ZENO SDK template ran frame 0` |
| Packaged samples | Extracted ZIP + CLI presets | Debug | `cmake --preset windows-msvc-debug -S <sdk>/samples/sdk_feature_samples_cpp`, build | Build-only | Passed |
| Packaged samples | Extracted ZIP + CLI presets | Release | `cmake --preset windows-msvc-release -S <sdk>/samples/sdk_feature_samples_cpp`, build | Build-only | Passed |
| VS2022 repository Open Folder | CMakePresets contract | Debug + Release | Open repository root and select `windows-msvc-debug` / `windows-msvc-release` | IDE route | Presets validated by CLI and VS generator builds |
| VS2022 packaged roots | CMakePresets contract | Debug + Release | Open packaged `templates/cpp_empty` or `samples/sdk_feature_samples_cpp` | IDE route | Packaged presets generated and validated by extracted ZIP builds |
| VS Code repository CMake Tools | CMakePresets contract | Debug + Release | Open repository root and select `windows-msvc-debug` / `windows-msvc-release` | IDE route | Presets validated by CLI builds |
| VS Code packaged roots | CMakePresets contract | Debug + Release | Open packaged template or sample folder and select preset | IDE route | Packaged presets generated and validated by extracted ZIP builds |
| DLL placement | Packaged template/sample builds | Debug + Release | Check executable output folders | Headless | Passed; `zeno_abi.dll` copied beside template and sample executables |
| Asset placement | Packaged sample builds | Debug + Release | Check sample output folders | Headless | Passed; `assets/` copied beside sample executables |
| Private header check | Package layout and extracted ZIP | All | Scan `include/` for `src`, `native_backend.h`, and `.codex` leakage | Headless | Passed; no hits |

## Headless Evidence

- Debug CMake configure/build completed with Visual Studio 17 2022 generator.
- Release CMake configure/build completed with Visual Studio 17 2022 generator.
- `scripts/test-headless.ps1 -Configuration Debug` passed Rust tests and 19 headless CTest entries.
- `scripts/test-headless.ps1 -Configuration Release` passed after fixing the Release Cargo invocation; it now builds `zeno_abi` with `--release`.
- `scripts/package-sdk.ps1` produced `build/package-sdk/ZenoEngine-SDK-v0.1.0-dev/` and `build/package-sdk/ZenoEngine-SDK-v0.1.0-dev.zip`.
- `scripts/verify-external-game.ps1 -Configuration Debug` and `-Configuration Release` both built and ran the headless external game using `find_package(ZenoEngine CONFIG REQUIRED)`.
- Extracted ZIP-only validation built and ran `templates/cpp_empty` in Debug and Release.
- Extracted ZIP-only validation built `samples/sdk_feature_samples_cpp` in Debug and Release.

## GUI / IDE Evidence

No manual GUI launch of Visual Studio 2022 or VS Code was performed in this phase report run. The IDE-facing contract was validated through CMakePresets and Visual Studio generator builds:

- Repository root presets configure and build in Debug and Release.
- Packaged template and sample roots now contain equivalent Debug/Release presets.
- `zeno_package_sdk` is available as a CMake target and was built through the Visual Studio generator.
- Packaged sample executables are still GUI/windowed checks. This phase validates their build, DLL placement, and asset placement headlessly; manual launch remains a local desktop validation step.

## SDK Package / ZIP Evidence

The package and extracted ZIP contain:

- `lib/Debug/zeno_sdk_cpp.lib`
- `lib/Debug/zeno_native.lib`
- `lib/Debug/zeno_abi.dll.lib`
- `bin/Debug/zeno_abi.dll`
- `lib/Release/zeno_sdk_cpp.lib`
- `lib/Release/zeno_native.lib`
- `lib/Release/zeno_abi.dll.lib`
- `bin/Release/zeno_abi.dll`
- `templates/cpp_empty/CMakePresets.json`
- `samples/sdk_feature_samples_cpp/CMakePresets.json`

The generated presets set:

```json
"ZenoEngine_DIR": "${sourceDir}/../../cmake"
```

This keeps CLI, VS2022 Open Folder, and VS Code CMake Tools on the same packaged SDK graph.

## Private / Internal Header Check

Both the package layout and extracted ZIP were scanned under `include/`. The scan found no paths containing `\src\`, no `native_backend.h`, and no `.codex` content.

## Docs Changes And Rationale

- `README.md`: raised the documented Visual Studio 2022 requirement to 17.7+ because the project uses CMakePresets schema version 6.
- `docs/build-guide.md`: added Release CLI commands, Debug/Release external-game verification, and packaged template/sample preset checks.
- `docs/getting-started.md`: added packaged template/sample preset commands and Release external-game verification.
- `docs/sdk-layout.md`: documented generated `CMakePresets.json` files under packaged template and sample roots.
- `docs/vs2022.md`: documented Visual Studio 2022 17.7+, `zeno_package_sdk`, and packaged root preset use without manual `ZenoEngine_DIR` setup.
- `docs/vscode-cmake.md`: documented selecting packaged Debug/Release presets in CMake Tools.

## Script / CMake Changes And Rationale

- `scripts/package-sdk.ps1`: generates packaged `CMakePresets.json` files for `templates/cpp_empty` and `samples/sdk_feature_samples_cpp`, accepts `-CMakeExe`, and uses that executable for nested CMake calls.
- `scripts/test-headless.ps1`: fixes Release Cargo build invocation by calling `cargo build -p zeno_abi --release` explicitly.
- `scripts/verify-external-game.ps1`: always packages Debug and Release artifacts before building the requested external-game configuration, so both Debug and Release consume the same SDK distribution shape.
- `CMakeLists.txt`: changes `zeno_package_sdk` to package Debug and Release and passes `${CMAKE_COMMAND}` to the script.

## Deviations From Spec

- Actual VS2022 and VS Code UI sessions were not opened manually. Their Open Folder compatibility was validated through the same CMakePresets files and Visual Studio generator builds those IDEs consume.
- Windowed packaged samples were not launched. They were build-validated in Debug and Release, and their DLL/assets runtime placement was checked.

## Phase46 Carryover

- Add a CI artifact upload or release workflow for `ZenoEngine-SDK-v0.1.0-dev.zip` after public release policy is defined.
- Consider adding an automated package-consumption QA script that expands the ZIP and validates template/sample Debug/Release presets in one command.
- Keep GUI sample launch checks manual until a stable window automation strategy is chosen.

## SubAgent Inputs

- VS2022 Compatibility Agent: identified the Visual Studio 2022 17.7+ preset schema requirement and the need for `zeno_package_sdk` to produce both Debug and Release artifacts.
- VS Code + CMake Compatibility Agent: identified the missing packaged `CMakePresets.json` files and recommended Debug/Release presets for packaged template/sample roots.
- CLI / CMake Preset QA Agent: started command validation but was stopped after background build processes remained active; the main thread reran the CLI matrix sequentially.
- SDK External Consumption Agent: started package validation but was stopped with the CLI agent; the main thread reran ZIP-only template/sample validation sequentially.
- Compatibility Report / Regression Guard Agent: provided the report outline and required evidence categories used here.

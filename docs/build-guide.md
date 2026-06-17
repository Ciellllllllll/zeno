# Building ZENO

This repository is Windows-first for the current milestone.

For packaged SDK consumption, start with [Getting Started With ZENO SDK](getting-started.md) and [SDK Tutorials](tutorials/index.md). This page is the repository maintainer build and regression guide.

## Requirements

- Windows 10 or Windows 11.
- Visual Studio 2022 17.7 or newer with MSVC v143 and a Windows SDK.
- CMake 3.24 or newer on `PATH`.
- Rust stable with Cargo on `PATH`.
- VS Code with rust-analyzer and CMake Tools is optional.

The source-of-truth build files are `Cargo.toml`, `CMakeLists.txt`, and `CMakePresets.json`. Generated Visual Studio solutions/projects and editor settings are not canonical.

## Clean Build

```powershell
.\scripts\clean-build.ps1
.\scripts\build-all.ps1
```

`build-all.ps1` builds the Rust ABI crate, runs Rust workspace tests, configures/builds the CMake preset, and runs the non-window smoke executables used by the current local workflow.

## Verification Scripts

```powershell
.\scripts\verify-format.ps1
.\scripts\verify-abi.ps1
.\scripts\test-headless.ps1
.\scripts\verify-sdk-package-consumption.ps1
.\scripts\verify-all.ps1
```

`verify-all.ps1` is the CI-style local baseline: Rust format, whitespace checks, ABI forbidden-type scan, Cargo tests, CMake configure/build, headless CTest, and package creation.

`verify-sdk-package-consumption.ps1` is the full SDK package consumption QA path. It creates the SDK package and ZIP, extracts the ZIP into an ignored validation directory, validates Debug/Release packaged template and sample builds, validates the external headless example through `find_package`, checks package artifact integrity, and validates packaged sample asset integrity in the extracted SDK and runtime output directories.

For SDK v0.1.0 release candidates, this package consumption QA path is the release gate. A release candidate is not ready unless Debug headless tests, Release headless tests, and `verify-sdk-package-consumption.ps1` all pass.

The manual GitHub Actions workflow `SDK RC Artifact` runs the same release gate and uploads `ZenoEngine-SDK-v0.1.0-rc.1.zip` as a workflow artifact. It does not publish a GitHub Release, create a git tag, sign artifacts, or commit generated outputs.

```powershell
.\scripts\test-all-local.ps1
```

`test-all-local.ps1` adds window-capable CTest smoke and launches the sample/template executables. It is intentionally local/manual, not part of the default CI workflow.

## Manual Commands

```powershell
cargo build -p zeno_abi
cargo test --workspace
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

`ctest --preset windows-msvc-debug` includes smoke executables that may open a window. Run it from an environment where window creation is acceptable. CI uses `ctest --preset windows-msvc-debug -E "window|sample|manual"`.

## Run

```powershell
.\scripts\run-sample.ps1
.\scripts\run-template.ps1
.\scripts\run-dynamic-module-sample.ps1
```

These scripts build first if the target executable is missing. The sample and template resolve assets relative to their executable directory. The dynamic module sample is headless and validates DLL load failure paths, API version mismatch, lifecycle callbacks, and unload/reset.

## Package

```powershell
.\scripts\package-runtime.ps1
```

The package is written under `build/package/windows-msvc-debug/bin/` by default:

- `bin/zeno_sample_game_cpp.exe`
- `bin/zeno_abi.dll`
- `bin/assets/`
- `bin/template-game/zeno_template_game_cpp.exe`
- `bin/template-game/zeno_abi.dll`
- `bin/template-game/assets/`

Runtime packaging is for running the in-repository sample/template executables.

```powershell
.\scripts\package-sdk.ps1
.\scripts\verify-external-game.ps1 -Configuration Debug
.\scripts\verify-external-game.ps1 -Configuration Release
```

SDK packaging is for external CMake projects. It creates `build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1/` and `build/package-sdk/ZenoEngine-SDK-v0.1.0-rc.1.zip` with:

- `include/zeno/`
- `lib/Debug/` and `lib/Release/`
- `bin/Debug/` and `bin/Release/`
- `samples/sdk_feature_samples_cpp/`
- `templates/cpp_empty/`
- `docs/`
- `cmake/ZenoEngineConfig.cmake`
- `cmake/ZenoEngineConfigVersion.cmake`

`ZenoEngineConfig.cmake` exposes imported CMake targets `ZenoEngine::zeno_sdk_cpp`, `ZenoEngine::zeno_native`, and `ZenoEngine::zeno_abi_rust`. It also defines numeric `ZenoEngine_VERSION` as `0.1.0` and `ZenoEngine_RELEASE_LABEL` as `0.1.0-rc.1`.

`0.1.0-rc.1` is the SDK package label. The CMake package version remains numeric `0.1.0` for normal `find_package` version comparison.

External projects can consume it with:

```powershell
$zenoDir = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1\cmake).Path
cmake -S examples\external-game -B build\external-game -DZenoEngine_DIR="$zenoDir"
cmake --build build\external-game --config Debug
cmake --build build\external-game --config Release
```

## Regression Matrix

| Purpose | Command | Expected Result | Notes |
| --- | --- | --- | --- |
| Clean generated outputs | `.\scripts\clean-build.ps1` | Removes `build/` and `target/` | Destructive only for generated local output. |
| Rust/whitespace format | `.\scripts\verify-format.ps1` | Passes | Runs `cargo fmt --check` and `git diff --check`. |
| ABI surface | `.\scripts\verify-abi.ps1` | Passes | Scans C ABI headers for forbidden C++/Win32/DirectX/XAudio surface text. |
| Rust tests | `cargo test --workspace` | Passes | Covered by `build-all.ps1`. |
| Configure/build/smoke subset | `.\scripts\build-all.ps1` | Passes | Uses Cargo plus `windows-msvc-debug`; runs non-window smoke executables. |
| CTest listing | `ctest --preset windows-msvc-debug -N` | Lists current smoke tests | Does not execute tests. |
| Headless CTest | `ctest --preset windows-msvc-debug -E "window|sample|manual"` | Passes | Excludes window-capable and manual run tests by name; tests are also labeled for local selection. |
| Diagnostics smoke | `build/windows-msvc-debug/bin/Debug/zeno_sdk_diagnostics_smoke.exe` | Verifies SDK log sink and last-diagnostic messages | Headless smoke; included in `test-headless.ps1`. |
| Debug overlay smoke | `ctest --preset windows-msvc-debug -R zeno_debug_draw_smoke --output-on-failure` | Verifies debug line/rect/text descriptor validation | Headless smoke. |
| Full CTest smoke | `ctest --preset windows-msvc-debug` | Manual window validation | May open windows. |
| Sample game | `.\scripts\run-sample.ps1` | Opens sample window and exits cleanly with debug overlay visible | Window run. |
| Template game | `.\scripts\run-template.ps1` | Opens template window and exits cleanly | Window run. |
| Package | `.\scripts\package-runtime.ps1` | Creates sample/template package layout | Uses CMake install plus DLL copy. |
| SDK package | `.\scripts\package-sdk.ps1` | Creates external SDK package layout and ZIP | Includes headers, Debug/Release static libs, ABI import lib/DLL, samples, templates, docs, and CMake config files. |
| SDK package consumption QA | `.\scripts\verify-sdk-package-consumption.ps1` | Generates, extracts, builds, runs, and validates the SDK package ZIP | Headless. Checks Debug/Release template run, sample builds, external-game runs, DLL/assets placement, sample asset integrity, required libs, and private header exclusion. |
| External game Debug package check | `.\scripts\verify-external-game.ps1 -Configuration Debug` | Builds and runs the headless external example | Uses packaged `ZenoEngine::zeno_sdk_cpp`, not in-tree includes. |
| External game Release package check | `.\scripts\verify-external-game.ps1 -Configuration Release` | Builds and runs the headless external example | Uses packaged `ZenoEngine::zeno_sdk_cpp`, not in-tree includes. |
| Packaged template presets | `cmake --preset windows-msvc-debug -S "$sdkRoot\templates\cpp_empty"` and `cmake --preset windows-msvc-release -S "$sdkRoot\templates\cpp_empty"` | Configures the packaged template through SDK presets | The same preset names are available to VS2022 Open Folder and VS Code CMake Tools. |
| Packaged sample presets | `cmake --preset windows-msvc-debug -S "$sdkRoot\samples\sdk_feature_samples_cpp"` and `cmake --preset windows-msvc-release -S "$sdkRoot\samples\sdk_feature_samples_cpp"` | Configures packaged samples through SDK presets | Build-only validation is headless; running samples is a GUI check. |
| Dynamic module sample | `.\scripts\run-dynamic-module-sample.ps1` | Builds/runs the headless DLL module sample | Uses `LoadLibraryW`, descriptor version validation, lifecycle callbacks, and unload. |
| GameApp failed-init cleanup | `build/windows-msvc-debug/bin/Debug/zeno_sdk_failed_init_smoke.exe` | Verifies `on_shutdown` after failed `on_init` | Window-capable smoke; run only when opening local windows is acceptable. |
| Renderer resize smoke | `build/windows-msvc-debug/bin/Debug/zeno_resize_smoke.exe` | Verifies minimized and nonzero resize path | Window-capable smoke; run only when opening local windows is acceptable. |
| Package layout | `Test-Path build/package/windows-msvc-debug/bin/zeno_abi.dll` and related sample/template asset paths | Passes | Also check the package does not contain `.codex` or other private planning inputs. |
| CI-style baseline | `.\scripts\verify-all.ps1` | Passes | Runs format, ABI, headless test, and package scripts. |
| Local full validation | `.\scripts\test-all-local.ps1` | Manual window validation | Includes window-capable checks; run manually. |
| Final status | `git status --short --ignored` | Review tracked and ignored output | `.codex/`, `build/`, and `target/` remain ignored locally; `docs/` contains only public docs. |

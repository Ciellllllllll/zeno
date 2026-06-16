# Building ZENO

This repository is Windows-first for the current milestone.

## Requirements

- Windows 10 or Windows 11.
- Visual Studio 2022 with MSVC v143 and a Windows SDK.
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
.\scripts\verify-all.ps1
```

`verify-all.ps1` is the CI-style local baseline: Rust format, whitespace checks, ABI forbidden-type scan, Cargo tests, CMake configure/build, headless CTest, and package creation.

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
.\scripts\verify-external-game.ps1
```

SDK packaging is for external CMake projects. It creates `build/package-sdk/windows-msvc-debug/` with:

- `include/zeno/`
- `lib/zeno_sdk_cpp.lib`
- `lib/zeno_native.lib`
- `lib/zeno_abi.dll.lib`
- `bin/zeno_abi.dll`
- `cmake/ZENOConfig.cmake`
- `cmake/ZENOConfigVersion.cmake`

`ZENOConfig.cmake` exposes imported CMake targets `ZENO::zeno_sdk_cpp`, `ZENO::zeno_native`, and `ZENO::zeno_abi_rust`. It also defines `ZENO_VERSION` as `0.1.0`.

External projects can consume it with:

```powershell
$zenoDir = (Resolve-Path .\build\package-sdk\windows-msvc-debug\cmake).Path
cmake -S examples\external-game -B build\external-game -DZENO_DIR="$zenoDir"
cmake --build build\external-game
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
| SDK package | `.\scripts\package-sdk.ps1` | Creates external SDK package layout | Includes headers, static libs, ABI import lib/DLL, and CMake config files. |
| External game package check | `.\scripts\verify-external-game.ps1` | Builds and runs the headless external example | Uses packaged `ZENO::zeno_sdk_cpp`, not in-tree includes. |
| Dynamic module sample | `.\scripts\run-dynamic-module-sample.ps1` | Builds/runs the headless DLL module sample | Uses `LoadLibraryW`, descriptor version validation, lifecycle callbacks, and unload. |
| GameApp failed-init cleanup | `build/windows-msvc-debug/bin/Debug/zeno_sdk_failed_init_smoke.exe` | Verifies `on_shutdown` after failed `on_init` | Window-capable smoke; run only when opening local windows is acceptable. |
| Renderer resize smoke | `build/windows-msvc-debug/bin/Debug/zeno_resize_smoke.exe` | Verifies minimized and nonzero resize path | Window-capable smoke; run only when opening local windows is acceptable. |
| Package layout | `Test-Path build/package/windows-msvc-debug/bin/zeno_abi.dll` and related sample/template asset paths | Passes | Also check the package does not contain `AGENTS.md`, `docs`, or `goal`. |
| CI-style baseline | `.\scripts\verify-all.ps1` | Passes | Runs format, ABI, headless test, and package scripts. |
| Local full validation | `.\scripts\test-all-local.ps1` | Manual window validation | Includes window-capable checks; run manually. |
| Final status | `git status --short --ignored` | Review tracked and ignored output | `AGENTS.md`, `goal/`, `build/`, `target/`, and private `docs/*` inputs remain ignored locally. |

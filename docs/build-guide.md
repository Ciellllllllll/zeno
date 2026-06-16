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

## Manual Commands

```powershell
cargo build -p zeno_abi
cargo test --workspace
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

`ctest --preset windows-msvc-debug` includes smoke executables that may open a window. Run it from an environment where window creation is acceptable.

## Run

```powershell
.\scripts\run-sample.ps1
.\scripts\run-template.ps1
```

Both scripts build first if the target executable is missing. The sample and template resolve assets relative to their executable directory.

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

## Regression Matrix

| Purpose | Command | Expected Result | Notes |
| --- | --- | --- | --- |
| Clean generated outputs | `.\scripts\clean-build.ps1` | Removes `build/` and `target/` | Destructive only for generated local output. |
| Rust format | `cargo fmt --check` | Passes | No source rewriting. |
| Rust tests | `cargo test --workspace` | Passes | Covered by `build-all.ps1`. |
| Configure/build/smoke subset | `.\scripts\build-all.ps1` | Passes | Uses Cargo plus `windows-msvc-debug`; runs non-window smoke executables. |
| CTest listing | `ctest --preset windows-msvc-debug -N` | Lists current smoke tests | Does not execute tests. |
| CTest excluding window-capable smoke | `ctest --preset windows-msvc-debug -E "zeno_native_backend_smoke|zeno_sdk_smoke"` | Passes | Useful before window interaction is approved. |
| Full CTest smoke | `ctest --preset windows-msvc-debug` | Passes | May open windows. |
| Sample game | `.\scripts\run-sample.ps1` | Opens sample window and exits cleanly | Window run. |
| Template game | `.\scripts\run-template.ps1` | Opens template window and exits cleanly | Window run. |
| Package | `.\scripts\package-runtime.ps1` | Creates sample/template package layout | Uses CMake install plus DLL copy. |
| Package layout | `Test-Path build/package/windows-msvc-debug/bin/zeno_abi.dll` and related sample/template asset paths | Passes | Also check the package does not contain `AGENTS.md`, `docs`, or `goal`. |
| ABI forbidden type scan | `rg -n "std::string|std::vector|std::filesystem|template|class |HRESULT|ComPtr|IXAudio|XAUDIO|WAVEFORMAT|HWND|ID3D|IWIC|DXGI|D3D11|std::" native/zeno_native/include/zeno/zeno_native_backend.h native/zeno_native/include/zeno/zeno_abi.h -S` | No matches | `rg` exits 1 when there are no matches. |
| Whitespace | `git diff --check` | Passes | Run before commit. |
| Final status | `git status --short --ignored` | Review tracked and ignored output | `AGENTS.md`, `docs/`, `goal/`, `build/`, and `target/` are intentionally ignored locally. |

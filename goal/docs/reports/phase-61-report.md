# Phase 61 Report — Private RendererBackend No-Window Smoke

## Summary

Phase61 adds private headless smoke coverage for the RendererBackend object returned by the private renderer factory before any window-based setup.

The implementation keeps DirectX 11 as the default and only implemented renderer backend. DirectX 12 remains unavailable and unimplemented. No public backend selection, public capability reporting, DirectX 12 bootstrap work, public ABI exposure, or DirectX 11 renderer refactor was added.

The headless test runner was also adjusted to exclude tests by label instead of by test name so the required `zeno_renderer_backend_no_window_smoke` test is not filtered out by the word `window` in its name.

## Files Added

- `native/zeno_native/src/renderer_backend_no_window_smoke.cpp`
- `goal/docs/reports/phase-61-report.md`

## Files Modified

- `native/zeno_native/CMakeLists.txt`
- `scripts/test-headless.ps1`

## Public API / ABI Changes

None.

The new smoke test includes the private native header `native/zeno_native/src/native_backend.h` directly. No SDK header or C ABI header changed.

Searches for `RendererBackend`, `RendererBackendKind`, `RendererBackendCapabilities`, `create_renderer_backend`, `renderer_backend_capabilities`, and `renderer_backend_name` found no matches under `native/zeno_native/include` or `sdk/cpp/include`.

Searches for native DirectX/Win32 type names found no matches under `sdk/cpp/include`. Under `native/zeno_native/include`, the existing comments still mention Win32 and DirectX 11 behavior, but no DirectX COM/D3D/DXGI or Win32 native types were exposed by this phase.

## Tests Added or Updated

- Added `zeno_renderer_backend_no_window_smoke`.
- Wired it into `native/zeno_native/CMakeLists.txt` as a `headless` CTest.
- The smoke test does not create a Win32 window.
- The smoke test creates the private DirectX 11 renderer through `create_renderer_backend(RendererBackendKind::directx11)`.
- The smoke test verifies the renderer is non-null, reports DirectX 11 kind, and reports capabilities matching `renderer_backend_capabilities(RendererBackendKind::directx11)`.
- The smoke test verifies representative frame, clear-resource, texture, mesh, material, debug draw, and presentation calls fail safely before window-based setup.
- The smoke test verifies create-style failure paths leave output handles unchanged.
- The smoke test verifies `create_renderer_backend(RendererBackendKind::directx12)` still returns null.
- Updated `scripts/test-headless.ps1` from name exclusion (`-E`) to label exclusion (`-LE`) so required headless tests whose names contain `window` still run when labeled `headless`.

## Build / Test Result

Commands run:

```powershell
git branch --show-current
git status --short
.\scripts\test-headless.ps1 -Configuration Debug
$env:Path='C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;' + $env:Path; .\scripts\test-headless.ps1 -Configuration Debug
.\scripts\test-headless.ps1 -Configuration Release
$env:Path='C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;' + $env:Path; .\scripts\test-headless.ps1 -Configuration Release
.\scripts\verify-sdk-package-consumption.ps1
$env:Path='C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;' + $env:Path; .\scripts\verify-sdk-package-consumption.ps1
.\scripts\verify-abi.ps1
git diff --check
```

Results:

- `git branch --show-current`
  - Exit code: `0`
  - Output: `develop`
- Initial `git status --short`
  - Exit code: `0`
  - Output: no output
- Current `git status --short`
  - Exit code: `0`
  - Output:

```text
 M native/zeno_native/CMakeLists.txt
 M scripts/test-headless.ps1
?? goal/docs/reports/phase-61-report.md
?? native/zeno_native/src/renderer_backend_no_window_smoke.cpp
```

- `.\scripts\test-headless.ps1 -Configuration Debug`
  - Exit code: `1`
  - Rust tests completed successfully before the environment blocker:
    - `zeno_abi`: `22 passed; 0 failed`
    - `zeno_core`: `15 passed; 0 failed`
    - doc tests: `0 passed; 0 failed`
  - Blocker:

```text
cmake: D:\Git\zeno\scripts\test-headless.ps1:21
Line |
  21 |      cmake --preset $preset
     |      ~~~~~
     | The term 'cmake' is not recognized as a name of a cmdlet, function, script file, or executable program. Check
     | the spelling of the name, or if a path was included, verify that the path is correct and try again.
```

- `$env:Path='C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;' + $env:Path; .\scripts\test-headless.ps1 -Configuration Debug`
  - Exit code: `0`
  - Rust tests:
    - `zeno_abi`: `22 passed; 0 failed`
    - `zeno_core`: `15 passed; 0 failed`
    - doc tests: `0 passed; 0 failed`
  - Native headless CTest: `100% tests passed, 0 tests failed out of 24`
  - `zeno_renderer_backend_no_window_smoke`: `Passed`

- `.\scripts\test-headless.ps1 -Configuration Release`
  - Exit code: `1`
  - Rust tests completed successfully before the environment blocker:
    - `zeno_abi`: `22 passed; 0 failed`
    - `zeno_core`: `15 passed; 0 failed`
    - doc tests: `0 passed; 0 failed`
  - Blocker:

```text
cmake: D:\Git\zeno\scripts\test-headless.ps1:21
Line |
  21 |      cmake --preset $preset
     |      ~~~~~
     | The term 'cmake' is not recognized as a name of a cmdlet, function, script file, or executable program. Check
     | the spelling of the name, or if a path was included, verify that the path is correct and try again.
```

- `$env:Path='C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;' + $env:Path; .\scripts\test-headless.ps1 -Configuration Release`
  - Exit code: `0`
  - Rust tests:
    - `zeno_abi`: `22 passed; 0 failed`
    - `zeno_core`: `15 passed; 0 failed`
    - doc tests: `0 passed; 0 failed`
  - Native headless CTest: `100% tests passed, 0 tests failed out of 24`
  - `zeno_renderer_backend_no_window_smoke`: `Passed`

- `.\scripts\verify-sdk-package-consumption.ps1`
  - Exit code: `1`
  - Rust `dev` build completed before the environment blocker.
  - Blocker:

```text
&: D:\Git\zeno\scripts\package-sdk.ps1:221
Line |
 221 |          & $CMakeExe --preset $preset
     |            ~~~~~~~~~
     | The term 'cmake' is not recognized as a name of a cmdlet, function, script file, or executable program. Check
     | the spelling of the name, or if a path was included, verify that the path is correct and try again.
```

- `$env:Path='C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;' + $env:Path; .\scripts\verify-sdk-package-consumption.ps1`
  - Exit code: `0`
  - Result: `SDK package consumption QA passed`
  - Packaged SDK ZIP to `D:\Git\zeno\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1.zip`
  - Packaged SDK layout to `D:\Git\zeno\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1`
  - Debug and Release packaged template runs printed `ZENO SDK template ran frame 0`
  - Debug and Release external game runs printed `ZENO external game linked and ran one headless frame`

- `.\scripts\verify-abi.ps1`
  - Exit code: `0`
  - Output: `ABI forbidden-type scan passed.`

- `git diff --check`
  - Exit code: `0`
  - Output:

```text
warning: in the working copy of 'native/zeno_native/CMakeLists.txt', LF will be replaced by CRLF the next time Git touches it
warning: in the working copy of 'scripts/test-headless.ps1', LF will be replaced by CRLF the next time Git touches it
```

## Deviations From Spec

None.

The required commands were run. The first headless and SDK package consumption attempts exposed that `cmake` was not on `PATH`; after prepending the Visual Studio bundled CMake path for the process, the required CMake-based verification passed.

## Follow-Up Work

None.

## Role Checks

- SpecGuard / Scope: Phase61 stayed limited to one private RendererBackend no-window smoke test, CMake test wiring, a test-runner filter correction needed for the required test name, and this report. No DirectX 12 implementation, public backend selection, public capability reporting, or broad DirectX 11 renderer refactor was added.
- Renderer Backend: The smoke test creates the private DirectX 11 backend through `create_renderer_backend(RendererBackendKind::directx11)`, verifies kind and capabilities, checks safe no-window failures across representative calls, verifies failed create-style calls preserve output handles, and verifies DirectX 12 factory creation remains unavailable.
- ABI / Public Surface: No public SDK or C ABI files changed. Searches found no private renderer backend type exposure under `native/zeno_native/include` or `sdk/cpp/include`; no SDK DirectX/Win32 native type exposure was introduced.
- Build/Test: Debug headless, Release headless, SDK package consumption, ABI verification, and `git diff --check` were run and recorded. CMake-dependent commands passed after adding Visual Studio's bundled CMake to the process `PATH`.
- Documentation: `goal/docs/reports/phase-61-report.md` was added with Summary, Files Added, Files Modified, Public API / ABI Changes, Tests, Build / Test Result, Deviations, Follow-Up Work, and the required five role checks.

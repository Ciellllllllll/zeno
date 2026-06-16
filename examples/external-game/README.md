# ZENO External Game Example

This example is intentionally outside the in-repository sample/template build graph. It consumes a packaged ZENO SDK through `find_package(ZENO CONFIG REQUIRED)` and links only to the imported public target `ZENO::zeno_sdk_cpp`.

Build from the repository root after creating the SDK package:

```powershell
.\scripts\package-sdk.ps1
$zenoDir = (Resolve-Path .\build\package-sdk\windows-msvc-debug\cmake).Path
cmake -S examples\external-game -B build\external-game -DZENO_DIR="$zenoDir"
cmake --build build\external-game
```

The example uses the SDK engine frame clock without creating a Win32 window, so it is safe for headless package verification. It is not dynamic module loading.

The package config exposes `ZENO::zeno_sdk_cpp` and `ZENO_VERSION` as `0.1.0`.

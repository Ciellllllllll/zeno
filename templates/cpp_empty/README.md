# ZENO Empty C++ Template

This is a minimal external CMake project for consuming a packaged ZENO SDK.

```powershell
$sdkRoot = Resolve-Path ..\ZenoEngine-SDK-v0.1.0-dev
cmake -S . -B build -DZenoEngine_DIR="$sdkRoot\cmake"
cmake --build build --config Debug
.\build\Debug\zeno_cpp_empty.exe
```

The template links only to `ZenoEngine::zeno_sdk_cpp` and copies `zeno_abi.dll` beside the executable after build.

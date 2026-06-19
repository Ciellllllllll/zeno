# Build And Run The SDK Template

This tutorial validates the packaged SDK template in Debug and Release using the Phase45 verified command path.

## Set The SDK Root

From the repository root after packaging:

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1).Path
```

If you extracted the ZIP elsewhere, set `$sdkRoot` to that extracted SDK directory instead.

## Build Debug

```powershell
cmake --preset windows-msvc-debug -S "$sdkRoot\templates\cpp_empty"
cmake --build "$sdkRoot\templates\cpp_empty\build\windows-msvc-debug" --config Debug
& "$sdkRoot\templates\cpp_empty\build\windows-msvc-debug\Debug\zeno_cpp_empty.exe"
```

Expected output:

```text
ZENO SDK template ran frame 0
```

## Build Release

```powershell
cmake --preset windows-msvc-release -S "$sdkRoot\templates\cpp_empty"
cmake --build "$sdkRoot\templates\cpp_empty\build\windows-msvc-release" --config Release
& "$sdkRoot\templates\cpp_empty\build\windows-msvc-release\Release\zeno_cpp_empty.exe"
```

Expected output:

```text
ZENO SDK template ran frame 0
```

## IDE Use

Visual Studio 2022 Open Folder and VS Code CMake Tools use the same packaged presets:

- Open `$sdkRoot\templates\cpp_empty`.
- Select `windows-msvc-debug` or `windows-msvc-release`.
- Build the default target.

The template is headless, so it is safe to run from non-window validation sessions.

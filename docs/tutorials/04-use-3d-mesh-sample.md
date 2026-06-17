# Use The 3D Mesh Sample

The packaged 3D sample demonstrates the existing SDK path for a windowed static `GameApp` module with mesh creation, material state, perspective camera setup, scene rendering, debug lines, and debug text.

## Build From The Packaged SDK

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-dev).Path
cmake --preset windows-msvc-debug -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-debug" --config Debug --target zeno_sample_3d_mesh_cpp
```

For Release:

```powershell
cmake --preset windows-msvc-release -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-release" --config Release --target zeno_sample_3d_mesh_cpp
```

## Run

Run only from a local desktop session where opening a Win32 window is acceptable:

```powershell
& "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-debug\Debug\zeno_sample_3d_mesh_cpp.exe"
```

Escape exits. The sample also exits after its fixed sample duration.

## What To Read In The Code

Open `samples/sdk_feature_samples_cpp/src/sample_3d_mesh.cpp` in the package.

Key SDK concepts:

- `zeno::GameModule` and `zeno::GameApp`.
- `GameAppConfig::project_path = "projects/3d_mesh.zproj"`.
- `ResourceManager::create_mesh`.
- `ResourceManager::create_material`.
- `Scene::set_mesh_renderer`.
- `zeno::Camera::perspective`.
- `NativeBackend::set_camera_matrix`.
- `NativeBackend::draw_debug_line`.
- `NativeBackend::draw_debug_text`.

The mesh is built directly in code from existing `zeno::MeshVertex` and index arrays. No new asset importer or engine feature is required.

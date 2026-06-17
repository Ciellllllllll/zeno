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

The build copies `zeno_abi.dll` and `assets/` beside the executable. Do not run the executable from a directory where that copied `assets/` directory is missing.

On failure, the executable prints `3D SDK sample failed: <Result::message()>`. More specific setup and draw details are available through the SDK diagnostic channel (`zeno::last_diagnostic()` or `zeno::set_log_sink`), including missing scene data, mesh/material creation failures, camera setup failures, mesh draw failures, and debug overlay draw failures.

## Asset Layout

The sample uses executable-relative assets:

- `GameAppConfig::project_path = "projects/3d_mesh.zproj"`.
- The project uses `asset_root=.`.
- The project loads `initial_scene=scenes/3d_mesh.zscene`.
- The scene mesh reference is `builtin:cube`.

No mesh file importer or new mesh asset format is introduced by this sample.

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

## Render Order

The sample keeps the public 3D frame sequence explicit:

1. Begin the frame.
2. Clear the render target.
3. Set a perspective camera matrix.
4. Render the runtime scene through `Scene::render`.
5. Draw a debug line and debug text overlay.
6. Present the frame.

Keep mesh and material resources alive in `ResourceManager` for as long as the scene renderer IDs refer to them. Destroying or clearing resources before `Scene::render` makes the mesh/material IDs stale and returns `ZEN_RESULT_INVALID_ARGUMENT`.

## What This Sample Does Not Cover

This sample does not add gameplay rules, editor tooling, mesh import, animation, lighting redesign, or a general material system. It is a focused check that a game author can create a hardcoded color mesh, attach it to a runtime scene object, render it with a perspective camera, and get useful diagnostics when the setup is wrong.

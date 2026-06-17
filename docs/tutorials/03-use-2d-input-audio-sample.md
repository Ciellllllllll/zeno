# Use The 2D Input And Audio Sample

The packaged 2D sample demonstrates the existing SDK path for a windowed static `GameApp` module with sprite rendering, input, audio, debug rectangles, and debug text.

## Build From The Packaged SDK

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-dev).Path
cmake --preset windows-msvc-debug -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-debug" --config Debug --target zeno_sample_2d_input_audio_cpp
```

For Release:

```powershell
cmake --preset windows-msvc-release -S "$sdkRoot\samples\sdk_feature_samples_cpp"
cmake --build "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-release" --config Release --target zeno_sample_2d_input_audio_cpp
```

## Run

Run only from a local desktop session where opening a Win32 window is acceptable:

```powershell
& "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-debug\Debug\zeno_sample_2d_input_audio_cpp.exe"
```

The build copies `zeno_abi.dll` and `assets/` beside the executable. Do not run the executable from a directory where that copied `assets/` directory is missing.

## Asset Layout

The sample uses executable-relative assets:

- `GameAppConfig::project_path = "projects/2d_input_audio.zproj"`.
- The project uses `asset_root=.`.
- The project loads `initial_scene=scenes/2d_input_audio.zscene`.
- The scene sprite reference is `textures/sample_sprite_2x2.bmp`.
- The sample reads `sample_manifest.txt`.
- The sample loads `audio/sample_click.wav`.

Controls:

- WASD or arrow keys move the sprite.
- Space or left mouse plays the sample sound.
- Escape exits.

## What To Read In The Code

Open `samples/sdk_feature_samples_cpp/src/sample_2d_input_audio.cpp` in the package.

Key SDK concepts:

- `zeno::GameModule` and `zeno::GameApp`.
- `GameAppConfig::project_path = "projects/2d_input_audio.zproj"`.
- `context.assets->read_text`.
- `ResourceManager::create_texture`.
- `ResourceManager::create_sprite_material`.
- `ResourceManager::load_sound`.
- `InputSnapshot::down` and `InputSnapshot::pressed`.
- `Scene::set_sprite_renderer`.
- `NativeBackend::draw_debug_rect_2d`.
- `NativeBackend::draw_debug_text`.

The sample uses only public SDK headers and packaged assets.

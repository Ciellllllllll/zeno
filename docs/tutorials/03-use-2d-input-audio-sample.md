# Use The 2D Input And Audio Sample

The packaged 2D sample demonstrates the existing SDK path for a windowed static `GameApp` module with sprite rendering, input, audio, debug rectangles, and debug text.

## Build From The Packaged SDK

```powershell
$sdkRoot = (Resolve-Path .\build\package-sdk\ZenoEngine-SDK-v0.1.0-rc.1).Path
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

For Release:

```powershell
& "$sdkRoot\samples\sdk_feature_samples_cpp\build\windows-msvc-release\Release\zeno_sample_2d_input_audio_cpp.exe"
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

In the package, the source-of-truth files are:

- `assets/projects/2d_input_audio.zproj`.
- `assets/scenes/2d_input_audio.zscene`.
- `assets/textures/sample_sprite_2x2.bmp`.
- `assets/audio/sample_click.wav`.
- `assets/sample_manifest.txt`.

Controls:

- WASD or arrow keys move the sprite.
- Space or left mouse plays the sample sound.
- Escape exits.

Expected behavior:

- A 640x360 window opens.
- The sprite moves with WASD or arrow keys and clamps to the sample bounds.
- Space or left mouse plays `audio/sample_click.wav`.
- The overlay shows frame index, sound state, and mouse coordinates.
- The sample exits on Escape or after about 12 seconds.

## Troubleshooting

- If the executable fails before opening a window, confirm `zeno_abi.dll` and `assets/` are beside the executable.
- If asset loading fails, check `zeno::last_diagnostic()` or install `zeno::set_log_sink`; missing assets report the requested relative path, active root, and resolved path.
- If the scene fails to initialize, confirm it contains a sprite object named `player`.
- Run the sample only from a local desktop session where Win32 windows and audio playback are available.

## What To Read In The Code

Open `samples/sdk_feature_samples_cpp/src/sample_2d_input_audio.cpp` in the package.

The sample uses only public SDK headers and starts from:

```cpp
#include <zeno/game_module.hpp>
```

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

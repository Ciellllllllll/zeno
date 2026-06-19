# Assets, Input, And Audio

The SDK samples use executable-relative assets, frame-local input snapshots, and short PCM WAV playback.

## Asset Root

`zeno::AssetRoot` resolves paths relative to the executable asset directory, not the shell working directory. `GameApp` loads the project file named by `zeno::GameAppConfig::project_path`, then uses the project asset root and initial scene path.

The startup order is:

1. Resolve `<executable directory>/assets`.
2. Load `GameAppConfig::project_path` from that root.
3. Apply the project's `asset_root` if it is not `.`.
4. Load the project's `initial_scene`.

Missing text and binary assets are logged with the requested relative path plus the active root and resolved filesystem path. Invalid project and scene files are reported as parse failures with the relative project or scene path.

Useful operations include:

- `AssetRoot::resolve`
- `AssetRoot::read_text`
- `AssetRoot::read_binary`
- `load_project_config`
- `load_scene_description`

The 2D focused sample reads `sample_manifest.txt`, loads a texture from the scene object reference, and loads `audio/sample_click.wav`.

Packaged SDK samples expect this executable-relative asset layout after CMake builds them:

```text
assets/
  sample_manifest.txt
  project.zproj
  projects/
    2d_input_audio.zproj
    3d_mesh.zproj
  scenes/
    sample_scene.zscene
    2d_input_audio.zscene
    3d_mesh.zscene
  shaders/
    sample_triangle.hlsl
  audio/
    sample_click.wav
  textures/
    sample_sprite_2x2.bmp
```

For packaged Debug builds, the copied runtime directory is:

```text
samples/sdk_feature_samples_cpp/build/windows-msvc-debug/Debug/
  zeno_sample_2d_input_audio_cpp.exe
  zeno_abi.dll
  assets/
```

Repository-root sample builds copy the shared sample assets plus focused SDK project/scene assets into `build/windows-msvc-debug/bin/Debug/assets/`. Packaged SDK sample builds copy the already-merged package `assets/` directory.

## Project And Scene Data

`zeno::ProjectConfig` provides window size, asset root, and initial scene. `zeno::SceneDescription` provides simple object descriptions with names, transforms, colors, renderable kinds, and references.

The current text format is intentionally small and used by the existing samples. It is not an editor project format or asset pipeline.

## Input Snapshot

`zeno::InputSnapshot` is available through `GameContext::input`.

Keyboard helpers:

```cpp
context.input.down(zeno::Key::a);
context.input.pressed(zeno::Key::space);
context.input.released(zeno::Key::escape);
```

Mouse helpers:

```cpp
context.input.pressed(zeno::MouseButton::left);
context.input.mouse_x;
context.input.mouse_y;
context.input.mouse_wheel_delta;
```

The focused 2D sample uses WASD/arrows for movement, Space or left mouse for sound playback, and Escape to exit.

Input is exposed as a frame-local snapshot through the public `zeno::Key` and `zeno::MouseButton` enums. It is not an event queue or remapping system.

## Audio

Create an audio engine through `GameApp` or `NativeBackend::create_audio_engine`, then load sounds through `ResourceManager` or `AudioEngine`.

The focused 2D sample uses:

```cpp
context.resources->load_sound(*context.audio, *context.assets, "audio/sample_click.wav", g_sound);
zeno::Sound* sound = context.resources->sound(g_sound);
sound->set_volume(0.25f);
sound->play();
```

Audio is currently for short PCM WAV effects. It is not a streaming music system.

When debugging asset, input, or audio setup, check the returned `zeno::Result`, `zeno::Result::message()`, `zeno::last_diagnostic()`, or install a `zeno::set_log_sink` callback. Missing assets report the requested relative path, active root, and resolved filesystem path. Unsupported texture and WAV data reports the asset path and byte count when creation fails.

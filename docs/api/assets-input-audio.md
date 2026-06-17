# Assets, Input, And Audio

The SDK samples use executable-relative assets, frame-local input snapshots, and short PCM WAV playback.

## Asset Root

`zeno::AssetRoot` resolves paths relative to the executable asset directory. `GameApp` loads the project file named by `zeno::GameAppConfig::project_path`, then uses the project asset root and initial scene path.

Useful operations include:

- `AssetRoot::resolve`
- `AssetRoot::read_text`
- `AssetRoot::read_binary`
- `load_project_config`
- `load_scene_description`

The 2D focused sample reads `sample_manifest.txt`, loads a texture from the scene object reference, and loads `audio/sample_click.wav`.

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

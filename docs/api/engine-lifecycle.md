# Engine Lifecycle

The SDK offers two levels of runtime control:

- `zeno::Engine` for direct headless frame stepping.
- `zeno::GameApp` for windowed sample-style applications with engine, backend, assets, audio, scene, resources, input, and module lifecycle wiring.

## Minimal Engine Step

The packaged `templates/cpp_empty` project uses direct engine stepping:

```cpp
zeno::Engine engine;
zeno::EngineConfig config{};
config.target_fps = 1000000.0;
config.max_test_frames = 1;

zeno::Result result = zeno::Engine::create(config, engine);
zeno::EngineFrameInfo frame{};
result = engine.step_frame(frame);
```

This is the smallest headless check that the SDK links, loads `zeno_abi.dll`, and can run one engine frame.

## GameApp Flow

Focused samples use `zeno::GameApp` and a static `zeno::GameModule`:

```cpp
zeno::GameModule module{};
module.on_init = on_init;
module.on_update = on_update;
module.on_render = on_render;
module.on_shutdown = on_shutdown;

zeno::GameAppConfig config{};
config.project_path = "projects/2d_input_audio.zproj";

zeno::GameApp app;
zeno::Result result = app.run(module, config);
```

`GameApp` owns the runtime services and fills `zeno::GameContext` for lifecycle callbacks.

## Frame Order

The high-level frame order is:

1. Engine frame begin.
2. Window/event polling and input snapshot update.
3. Module `on_update`.
4. Module `on_render`.
5. Engine frame end.

`context.frame_index`, `context.delta_time_seconds`, `context.input`, and `context.should_close` are updated through this loop.

## Shutdown Safety

If `on_init` fails after runtime setup, `GameApp` calls `on_shutdown` before resetting resources. Write shutdown code so it can handle partially initialized state. The focused samples do this by checking pointers and clearing SDK-owned IDs/resources.

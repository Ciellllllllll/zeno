#include <zeno/game_module.hpp>

#include <utility>

namespace {

zeno::Result never_called(zeno::GameContext&)
{
    return zeno::Result(ZEN_RESULT_INTERNAL_ERROR);
}

} // namespace

int main()
{
    zeno::GameApp app;
    zeno::GameAppConfig config{};
    config.project_path = "missing-project.zproj";

    zeno::GameModule module{};
    module.on_init = never_called;

    zeno::Result result = app.run(module, config);
    if (result.ok() || app.running()) {
        return 1;
    }

    app.reset();
    if (app.running()) {
        return 2;
    }

    zeno::GameApp moved_app(std::move(app));
    if (moved_app.running()) {
        return 3;
    }

    zeno::GameApp assigned_app;
    assigned_app = std::move(moved_app);
    if (assigned_app.running() || moved_app.running()) {
        return 4;
    }

    zeno::Engine engine;
    zeno::EngineConfig engine_config{};
    engine_config.target_fps = 1000000.0;
    engine_config.max_test_frames = 3;
    result = zeno::Engine::create(engine_config, engine);
    if (!result.ok()) {
        return 5;
    }

    zeno::EngineFrameInfo frame_info{};
    result = engine.step_frame(frame_info);
    if (!result.ok() || frame_info.frame_index != 0 || frame_info.delta_time_seconds < 0.0) {
        return 6;
    }

    result = engine.begin_frame(frame_info);
    if (!result.ok() || frame_info.frame_index != 1 || frame_info.delta_time_seconds < 0.0) {
        return 7;
    }

    result = engine.begin_frame(frame_info);
    if (result.ok()) {
        return 8;
    }

    result = engine.end_frame();
    if (!result.ok()) {
        return 9;
    }

    result = engine.end_frame();
    if (result.ok()) {
        return 10;
    }

    engine.reset();
    return 0;
}

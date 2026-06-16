#include <zeno/game_module.hpp>

#include <filesystem>
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

    zeno::DynamicGameModule dynamic_module;
    result = assigned_app.run(dynamic_module, config);
    if (result.ok()) {
        return 5;
    }

    result = zeno::DynamicGameModule::load(std::filesystem::path("missing-dynamic-module.dll"), dynamic_module);
    if (result.ok() || dynamic_module.valid()) {
        return 6;
    }

    zeno::Engine engine;
    zeno::EngineConfig engine_config{};
    engine_config.target_fps = 1000000.0;
    engine_config.max_test_frames = 3;
    result = zeno::Engine::create(engine_config, engine);
    if (!result.ok()) {
        return 7;
    }

    zeno::EngineFrameInfo frame_info{};
    result = engine.step_frame(frame_info);
    if (!result.ok() || frame_info.frame_index != 0 || frame_info.delta_time_seconds < 0.0) {
        return 8;
    }

    result = engine.begin_frame(frame_info);
    if (!result.ok() || frame_info.frame_index != 1 || frame_info.delta_time_seconds < 0.0) {
        return 9;
    }

    result = engine.begin_frame(frame_info);
    if (result.ok()) {
        return 10;
    }

    result = engine.end_frame();
    if (!result.ok()) {
        return 11;
    }

    result = engine.end_frame();
    if (result.ok()) {
        return 12;
    }

    engine.reset();
    return 0;
}

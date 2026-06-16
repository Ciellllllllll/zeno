#include <zeno/game_module.hpp>

#include <filesystem>

namespace {

struct ModuleState final {
    bool init_called = false;
    bool update_called = false;
    bool render_called = false;
    bool shutdown_called = false;
    bool scene_object_created = false;
    bool sound_loaded = false;
    bool sound_reset = false;
};

ModuleState g_state{};
zeno::Sound g_sound{};

zeno::Result failing_init(zeno::GameContext& context)
{
    g_state.init_called = true;
    if (context.runtime_scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INTERNAL_ERROR);
    }

    const zeno::ObjectId id = context.runtime_scene->create_object();
    g_state.scene_object_created = id.valid();

    if (context.audio == nullptr || context.assets == nullptr) {
        return zeno::Result(ZEN_RESULT_INTERNAL_ERROR);
    }

    zeno::Result result = context.audio->load_sound(*context.assets, "audio/sample_click.wav", g_sound);
    if (!result.ok()) {
        return result;
    }

    g_state.sound_loaded = g_sound.valid();
    return zeno::Result(ZEN_RESULT_INTERNAL_ERROR);
}

zeno::Result update_should_not_run(zeno::GameContext&)
{
    g_state.update_called = true;
    return zeno::Result(ZEN_RESULT_INTERNAL_ERROR);
}

zeno::Result render_should_not_run(zeno::GameContext&)
{
    g_state.render_called = true;
    return zeno::Result(ZEN_RESULT_INTERNAL_ERROR);
}

zeno::Result cleanup_after_failed_init(zeno::GameContext& context)
{
    g_state.shutdown_called = true;
    if (g_sound.valid()) {
        g_sound.reset();
        g_state.sound_reset = !g_sound.valid();
    }

    if (context.runtime_scene != nullptr) {
        context.runtime_scene->clear();
    }

    return zeno::Result();
}

} // namespace

int main()
{
    zeno::GameAppConfig config{};
    config.project_path = "assets/project.zproj";
    config.engine.target_fps = 1000000.0;
    config.engine.max_test_frames = 1;

    zeno::GameModule module{};
    module.on_init = failing_init;
    module.on_update = update_should_not_run;
    module.on_render = render_should_not_run;
    module.on_shutdown = cleanup_after_failed_init;

    zeno::GameApp app;
    zeno::Result result = app.run(module, config);
    if (result.ok() || result.native_code() != ZEN_RESULT_INTERNAL_ERROR) {
        return 1;
    }

    if (app.running()) {
        return 2;
    }

    if (!g_state.init_called || !g_state.scene_object_created || !g_state.sound_loaded || !g_state.sound_reset
        || !g_state.shutdown_called || g_state.update_called || g_state.render_called) {
        return 3;
    }

    app.reset();
    if (app.running() || g_sound.valid()) {
        return 4;
    }

    return 0;
}

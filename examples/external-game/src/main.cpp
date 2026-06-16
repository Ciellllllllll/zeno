#include <zeno/game_module.hpp>

#include <iostream>

namespace {

zeno::Result on_update(zeno::GameContext& context)
{
    context.should_close = true;
    return zeno::Result();
}

} // namespace

int main()
{
    zeno::GameModule module{};
    module.on_update = on_update;

    zeno::Engine engine;
    zeno::EngineConfig config{};
    config.target_fps = 1000000.0;
    config.max_test_frames = 1;

    zeno::Result result = zeno::Engine::create(config, engine);
    if (!result.ok()) {
        std::cerr << "failed to create ZENO engine: " << result.message() << "\n";
        return 1;
    }

    zeno::EngineFrameInfo frame{};
    result = engine.begin_frame(frame);
    if (!result.ok()) {
        std::cerr << "failed to begin frame: " << result.message() << "\n";
        return 2;
    }

    zeno::GameContext context{};
    context.engine = &engine;
    context.frame_index = frame.frame_index;
    context.delta_time_seconds = frame.delta_time_seconds;
    result = module.on_update(context);
    if (!result.ok()) {
        std::cerr << "external module update failed: " << result.message() << "\n";
        return 3;
    }

    result = engine.end_frame();
    if (!result.ok()) {
        std::cerr << "failed to end frame: " << result.message() << "\n";
        return 4;
    }

    std::cout << "ZENO external game linked and ran one headless frame\n";
    return 0;
}

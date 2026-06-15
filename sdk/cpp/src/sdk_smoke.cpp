#include <zeno/zeno.hpp>

int main()
{
    zeno::Engine engine;
    zeno::EngineConfig config{};
    config.target_fps = 1000000.0;
    config.max_test_frames = 2;

    zeno::Result result = zeno::Engine::create(config, engine);
    if (!result.ok() || !engine.valid()) {
        return 1;
    }

    result = engine.step();
    if (!result.ok()) {
        return 2;
    }

    result = engine.request_shutdown();
    if (!result.ok()) {
        return 3;
    }

    return 0;
}

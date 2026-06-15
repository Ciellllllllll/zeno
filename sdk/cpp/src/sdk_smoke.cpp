#include <zeno/zeno.hpp>

#include <utility>

int main()
{
    zeno::Engine failed_engine;
    zeno::EngineConfig invalid_config{};
    invalid_config.target_fps = 0.0;

    zeno::Result result = zeno::Engine::create(invalid_config, failed_engine);
    if (result.ok() || failed_engine.valid()) {
        return 1;
    }

    zeno::Engine engine;
    zeno::EngineConfig config{};
    config.target_fps = 1000000.0;
    config.max_test_frames = 4;

    result = zeno::Engine::create(config, engine);
    if (!result.ok() || !engine.valid()) {
        return 2;
    }

    zeno::Engine moved_engine(std::move(engine));
    if (engine.valid() || !moved_engine.valid()) {
        return 3;
    }

    zeno::Engine assigned_engine;
    result = zeno::Engine::create(config, assigned_engine);
    if (!result.ok() || !assigned_engine.valid()) {
        return 4;
    }

    assigned_engine = std::move(moved_engine);
    if (moved_engine.valid() || !assigned_engine.valid()) {
        return 5;
    }

    result = assigned_engine.step();
    if (!result.ok()) {
        return 6;
    }

    result = zeno::Engine::create(invalid_config, assigned_engine);
    if (result.ok() || !assigned_engine.valid()) {
        return 7;
    }

    result = assigned_engine.request_shutdown();
    if (!result.ok()) {
        return 8;
    }

    assigned_engine.reset();
    if (assigned_engine.valid()) {
        return 9;
    }

    zeno::NativeBackend backend;
    result = zeno::NativeBackend::create(backend);
    if (!result.ok() || !backend.valid()) {
        return 10;
    }

    zeno::NativeBackend moved_backend(std::move(backend));
    if (backend.valid() || !moved_backend.valid()) {
        return 11;
    }

    zeno::NativeBackend assigned_backend;
    result = zeno::NativeBackend::create(assigned_backend);
    if (!result.ok() || !assigned_backend.valid()) {
        return 12;
    }

    assigned_backend = std::move(moved_backend);
    if (moved_backend.valid() || !assigned_backend.valid()) {
        return 13;
    }

    assigned_backend.reset();
    if (assigned_backend.valid()) {
        return 14;
    }

    return 0;
}

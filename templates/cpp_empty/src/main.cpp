#include <zeno/game_module.hpp>

#include <iostream>

int main()
{
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
    result = engine.step_frame(frame);
    if (!result.ok()) {
        std::cerr << "failed to run one frame: " << result.message() << "\n";
        return 2;
    }

    std::cout << "ZENO SDK template ran frame " << frame.frame_index << "\n";
    return 0;
}

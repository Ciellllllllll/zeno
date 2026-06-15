#include "sample_module.h"

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    zeno::NativeBackend backend;
    zeno::Result result = zeno::NativeBackend::create(backend);
    if (!result.ok()) {
        std::cerr << "backend create failed: " << result.message() << "\n";
        return 1;
    }

    result = backend.create_window(zeno::WindowConfig{ 640, 360 });
    if (!result.ok()) {
        std::cerr << "window create failed: " << result.message() << "\n";
        return 2;
    }

    result = backend.initialize_renderer();
    if (!result.ok()) {
        std::cerr << "renderer init failed: " << result.message() << "\n";
        return 3;
    }

    zeno::GameModule module = create_sample_game_module();
    zeno::GameContext context{};
    context.backend = &backend;

    result = zeno::initialize_game_module(module, context);
    if (!result.ok()) {
        return 4;
    }

    int exit_code = 0;
    auto previous_frame_time = std::chrono::steady_clock::now();
    while (!context.should_close) {
        const auto current_frame_time = std::chrono::steady_clock::now();
        context.delta_time_seconds = std::chrono::duration<double>(current_frame_time - previous_frame_time).count();
        previous_frame_time = current_frame_time;

        bool window_should_close = false;
        result = backend.poll_events(window_should_close);
        if (!result.ok()) {
            exit_code = 5;
            break;
        }
        context.should_close = window_should_close;
        if (context.should_close) {
            break;
        }

        result = zeno::run_game_module_frame(module, context);
        if (!result.ok()) {
            exit_code = 6;
            break;
        }

        ++context.frame_index;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    result = zeno::shutdown_game_module(module, context);
    if (!result.ok()) {
        return 7;
    }

    return exit_code;
}

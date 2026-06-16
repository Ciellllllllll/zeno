#include "sample_module.h"

#include <chrono>
#include <filesystem>
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

    zeno::AssetRoot asset_root;
    result = zeno::AssetRoot::from_executable(asset_root);
    if (!result.ok()) {
        std::cerr << "asset root resolve failed: " << result.message() << "\n";
        return 2;
    }

    zeno::ProjectConfig project;
    result = zeno::load_project_config(asset_root, "project.zproj", project);
    if (!result.ok()) {
        std::cerr << "project load failed: " << result.message() << "\n";
        return 3;
    }

    if (project.asset_root != ".") {
        zeno::AssetPath project_asset_root;
        result = asset_root.resolve(project.asset_root, project_asset_root);
        if (!result.ok()) {
            std::cerr << "project asset root resolve failed: " << result.message() << "\n";
            return 4;
        }

        result = zeno::AssetRoot::from_path(project_asset_root.path(), asset_root);
        if (!result.ok()) {
            std::cerr << "project asset root load failed: " << result.message() << "\n";
            return 5;
        }
    }

    zeno::SceneDescription scene;
    result = zeno::load_scene_description(asset_root, project.initial_scene, scene);
    if (!result.ok()) {
        std::cerr << "scene load failed: " << result.message() << "\n";
        return 6;
    }

    result = backend.create_window(zeno::WindowConfig{ project.window_width, project.window_height });
    if (!result.ok()) {
        std::cerr << "window create failed: " << result.message() << "\n";
        return 7;
    }

    result = backend.initialize_renderer();
    if (!result.ok()) {
        std::cerr << "renderer init failed: " << result.message() << "\n";
        return 8;
    }

    zeno::GameModule module = create_sample_game_module();

    zeno::GameContext context{};
    context.backend = &backend;
    context.assets = &asset_root;
    context.project = &project;
    context.scene = &scene;

    result = zeno::initialize_game_module(module, context);
    if (!result.ok()) {
        return 9;
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
            exit_code = 8;
            break;
        }
        context.should_close = window_should_close;
        if (context.should_close) {
            break;
        }

        result = backend.input_snapshot(context.input);
        if (!result.ok()) {
            exit_code = 9;
            break;
        }

        result = zeno::run_game_module_frame(module, context);
        if (!result.ok()) {
            exit_code = 10;
            break;
        }

        ++context.frame_index;
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    result = zeno::shutdown_game_module(module, context);
    if (!result.ok()) {
        return 11;
    }

    return exit_code;
}

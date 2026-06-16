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

    zeno::NativeBackend render_backend;
    result = zeno::NativeBackend::create(render_backend);
    if (!result.ok() || !render_backend.valid()) {
        return 15;
    }

    zeno::InputSnapshot input{};
    result = render_backend.input_snapshot(input);
    if (!result.ok()
        || input.down(zeno::Key::escape)
        || input.pressed(zeno::Key::space)
        || input.released(zeno::Key::a)
        || input.down(zeno::MouseButton::left)
        || input.mouse_x != 0
        || input.mouse_y != 0
        || input.mouse_wheel_delta != 0) {
        return 16;
    }

    zeno::RenderTriangle failed_triangle;
    result = render_backend.create_triangle(failed_triangle);
    if (result.ok() || failed_triangle.valid()) {
        return 17;
    }

    result = render_backend.create_window(zeno::WindowConfig{ 320, 180 });
    if (!result.ok()) {
        return 18;
    }

    result = render_backend.initialize_renderer();
    if (!result.ok()) {
        return 19;
    }

    zeno::RenderTriangle triangle;
    result = render_backend.create_triangle(triangle);
    if (!result.ok() || !triangle.valid()) {
        return 20;
    }

    result = render_backend.draw_triangle(triangle);
    if (result.ok()) {
        return 21;
    }

    result = render_backend.present();
    if (result.ok()) {
        return 22;
    }

    zeno::RenderTriangle moved_triangle(std::move(triangle));
    if (triangle.valid() || !moved_triangle.valid()) {
        return 23;
    }

    zeno::RenderTriangle assigned_triangle;
    result = render_backend.create_triangle(assigned_triangle);
    if (!result.ok() || !assigned_triangle.valid()) {
        return 24;
    }

    assigned_triangle = std::move(moved_triangle);
    if (moved_triangle.valid() || !assigned_triangle.valid()) {
        return 25;
    }

    result = render_backend.begin_frame();
    if (!result.ok()) {
        return 26;
    }

    result = render_backend.begin_frame();
    if (result.ok()) {
        return 27;
    }

    result = render_backend.clear(zeno::Color{ 0.05f, 0.08f, 0.12f, 1.0f });
    if (!result.ok()) {
        return 28;
    }

    result = render_backend.draw_triangle(assigned_triangle);
    if (!result.ok()) {
        return 29;
    }

    result = render_backend.present();
    if (!result.ok()) {
        return 30;
    }

    result = render_backend.draw_triangle(assigned_triangle);
    if (result.ok()) {
        return 31;
    }

    result = render_backend.present();
    if (result.ok()) {
        return 32;
    }

    assigned_triangle.reset();
    if (assigned_triangle.valid()) {
        return 33;
    }
    assigned_triangle.reset();

    return 0;
}

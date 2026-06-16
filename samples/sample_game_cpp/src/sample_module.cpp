#include "sample_module.h"

#include <iostream>

namespace {

constexpr double kDemoDurationSeconds = 4.0;
constexpr double kColorCycleSeconds = 2.0;
constexpr float kMouseColorScale = 1.0f / 640.0f;

double g_elapsed_seconds = 0.0;
float g_keyboard_tint = 0.0f;
zeno::RenderTriangle g_triangle;
zeno::Transform g_triangle_transform;
zeno::Camera g_camera;

zeno::Result on_init(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds = 0.0;
    g_keyboard_tint = 0.0f;
    g_triangle_transform = zeno::Transform{};
    g_triangle_transform.scale = zeno::Vec3{ 0.75f, 0.75f, 1.0f };
    g_camera = zeno::Camera::orthographic(2.0f, 2.0f, 0.0f, 1.0f);
    std::cerr << "[ZENO][sample] game init\n";
    zeno::Result result = context.backend->set_camera_matrix(g_camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    return context.backend->create_triangle(g_triangle);
}

zeno::Result on_update(zeno::GameContext& context)
{
    g_elapsed_seconds += context.delta_time_seconds;
    if (context.input.pressed(zeno::Key::escape)) {
        context.should_close = true;
        return zeno::Result();
    }

    if (context.input.down(zeno::Key::a) || context.input.down(zeno::Key::left)) {
        g_keyboard_tint -= static_cast<float>(context.delta_time_seconds);
    }

    if (context.input.down(zeno::Key::d) || context.input.down(zeno::Key::right)) {
        g_keyboard_tint += static_cast<float>(context.delta_time_seconds);
    }

    if (g_keyboard_tint < -0.25f) {
        g_keyboard_tint = -0.25f;
    }
    if (g_keyboard_tint > 0.25f) {
        g_keyboard_tint = 0.25f;
    }

    g_triangle_transform.rotation_z_radians = static_cast<float>(g_elapsed_seconds);
    g_triangle_transform.position.x = 0.25f * g_keyboard_tint;

    context.should_close = g_elapsed_seconds >= kDemoDurationSeconds;
    return zeno::Result();
}

zeno::Result on_render(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    const double cycle_position = g_elapsed_seconds / kColorCycleSeconds;
    const float t = static_cast<float>(cycle_position - static_cast<int>(cycle_position));
    const float mouse_t = static_cast<float>(context.input.mouse_x) * kMouseColorScale;
    zeno::Result result = context.backend->begin_frame();
    if (!result.ok()) {
        return result;
    }

    const zeno::Color color{
        0.05f + 0.25f * t + g_keyboard_tint,
        0.12f,
        0.20f + 0.20f * (1.0f - t) + 0.20f * mouse_t,
        1.0f,
    };

    result = context.backend->clear(color);
    if (!result.ok()) {
        return result;
    }

    result = context.backend->set_camera_matrix(g_camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    result = context.backend->draw_triangle(g_triangle, g_triangle_transform);
    if (!result.ok()) {
        return result;
    }

    return context.backend->present();
}

zeno::Result on_shutdown(zeno::GameContext&)
{
    g_triangle.reset();
    std::cerr << "[ZENO][sample] game shutdown\n";
    return zeno::Result();
}

} // namespace

zeno::GameModule create_sample_game_module()
{
    zeno::GameModule module{};
    module.on_init = on_init;
    module.on_update = on_update;
    module.on_render = on_render;
    module.on_shutdown = on_shutdown;
    return module;
}

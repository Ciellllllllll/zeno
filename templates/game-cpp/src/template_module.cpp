#include "template_module.h"

#include <iostream>

namespace {

constexpr double kTemplateDurationSeconds = 3.0;

double g_elapsed_seconds = 0.0;
zeno::RenderTriangle g_triangle;
zeno::Camera g_camera;

zeno::Result on_init(zeno::GameContext& context)
{
    if (context.backend == nullptr || context.project == nullptr || context.scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds = 0.0;
    const float aspect_ratio = context.project->window_height != 0
        ? static_cast<float>(context.project->window_width) / static_cast<float>(context.project->window_height)
        : 640.0f / 360.0f;
    g_camera = zeno::Camera::orthographic(2.0f * aspect_ratio, 2.0f, 0.0f, 10.0f);

    zeno::Result result = context.backend->set_camera_matrix(g_camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    result = context.backend->create_triangle(g_triangle);
    if (!result.ok()) {
        return result;
    }

    std::cerr << "[ZENO][template] init\n";
    return zeno::Result();
}

zeno::Result on_update(zeno::GameContext& context)
{
    g_elapsed_seconds += context.delta_time_seconds;
    if (context.input.pressed(zeno::Key::escape) || g_elapsed_seconds >= kTemplateDurationSeconds) {
        context.should_close = true;
    }

    return zeno::Result();
}

zeno::Result on_render(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Result result = context.backend->begin_frame();
    if (!result.ok()) {
        return result;
    }

    result = context.backend->clear(zeno::Color{ 0.08f, 0.10f, 0.14f, 1.0f });
    if (!result.ok()) {
        return result;
    }

    zeno::Transform transform{};
    transform.rotation_z_radians = static_cast<float>(g_elapsed_seconds);
    result = context.backend->draw_triangle(g_triangle, transform);
    if (!result.ok()) {
        return result;
    }

    return context.backend->present();
}

zeno::Result on_shutdown(zeno::GameContext&)
{
    g_triangle.reset();
    std::cerr << "[ZENO][template] shutdown\n";
    return zeno::Result();
}

} // namespace

zeno::GameModule create_template_game_module()
{
    zeno::GameModule module{};
    module.on_init = on_init;
    module.on_update = on_update;
    module.on_render = on_render;
    module.on_shutdown = on_shutdown;
    return module;
}

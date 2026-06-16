#include "sample_module.h"

#include <iostream>

namespace {

constexpr double kDemoDurationSeconds = 4.0;
constexpr double kColorCycleSeconds = 2.0;

double g_elapsed_seconds = 0.0;
zeno::RenderTriangle g_triangle;

zeno::Result on_init(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds = 0.0;
    std::cerr << "[ZENO][sample] game init\n";
    return context.backend->create_triangle(g_triangle);
}

zeno::Result on_update(zeno::GameContext& context)
{
    g_elapsed_seconds += context.delta_time_seconds;
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
    zeno::Result result = context.backend->begin_frame();
    if (!result.ok()) {
        return result;
    }

    const zeno::Color color{
        0.05f + 0.25f * t,
        0.12f,
        0.30f + 0.20f * (1.0f - t),
        1.0f,
    };

    result = context.backend->clear(color);
    if (!result.ok()) {
        return result;
    }

    result = context.backend->draw_triangle(g_triangle);
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

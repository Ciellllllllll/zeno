#include "sample_module.h"

#include <iostream>

namespace {

zeno::Result on_init(zeno::GameContext&)
{
    std::cerr << "[ZENO][sample] game init\n";
    return zeno::Result();
}

zeno::Result on_update(zeno::GameContext& context)
{
    context.should_close = context.frame_index >= 120;
    return zeno::Result();
}

zeno::Result on_render(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    const float t = static_cast<float>((context.frame_index % 120) / 119.0);
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

    return context.backend->present();
}

zeno::Result on_shutdown(zeno::GameContext&)
{
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

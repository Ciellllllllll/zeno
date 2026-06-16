#include <zeno/game_module.hpp>

namespace {

ZenResultCode on_init(ZenGameModuleHostContext* context)
{
    return context == nullptr ? ZEN_RESULT_INVALID_ARGUMENT : ZEN_RESULT_OK;
}

ZenResultCode on_update(ZenGameModuleHostContext* context)
{
    return context == nullptr ? ZEN_RESULT_INVALID_ARGUMENT : ZEN_RESULT_OK;
}

ZenResultCode on_render(ZenGameModuleHostContext* context)
{
    return context == nullptr ? ZEN_RESULT_INVALID_ARGUMENT : ZEN_RESULT_OK;
}

ZenResultCode on_shutdown(ZenGameModuleHostContext* context)
{
    return context == nullptr ? ZEN_RESULT_INVALID_ARGUMENT : ZEN_RESULT_OK;
}

} // namespace

extern "C" __declspec(dllexport) ZenResultCode zeno_get_game_module(ZenGameModuleDescriptor* out_descriptor)
{
    if (out_descriptor == nullptr) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    *out_descriptor = {};
    out_descriptor->size = ZEN_GAME_MODULE_DESCRIPTOR_SIZE;
    out_descriptor->api_version = ZEN_GAME_MODULE_API_VERSION;
    out_descriptor->on_init = on_init;
    out_descriptor->on_update = on_update;
    out_descriptor->on_render = on_render;
    out_descriptor->on_shutdown = on_shutdown;
    return ZEN_RESULT_OK;
}

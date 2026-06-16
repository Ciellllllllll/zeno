#include <zeno/game_module.hpp>

extern "C" __declspec(dllexport) ZenResultCode zeno_get_game_module(ZenGameModuleDescriptor* out_descriptor)
{
    if (out_descriptor == nullptr) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    *out_descriptor = {};
    out_descriptor->size = ZEN_GAME_MODULE_DESCRIPTOR_SIZE;
    out_descriptor->api_version = ZEN_GAME_MODULE_API_VERSION + 1u;
    return ZEN_RESULT_OK;
}

#pragma once

#include <stdint.h>

#include <zeno/zeno_abi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZEN_GAME_MODULE_API_VERSION 1u
#define ZEN_GAME_MODULE_ENTRY_POINT "zeno_get_game_module"

typedef struct ZenGameModuleHostContext {
    /* Reserved for future ABI-safe host services. Borrowed only during a callback. */
    void* reserved;
} ZenGameModuleHostContext;

typedef ZenResultCode (*ZenGameModuleLifecycleFn)(ZenGameModuleHostContext* context);

typedef struct ZenGameModuleDescriptor {
    uint32_t size;
    uint32_t api_version;
    ZenGameModuleLifecycleFn on_init;
    ZenGameModuleLifecycleFn on_update;
    ZenGameModuleLifecycleFn on_render;
    ZenGameModuleLifecycleFn on_shutdown;
    void* reserved;
} ZenGameModuleDescriptor;

#define ZEN_GAME_MODULE_DESCRIPTOR_SIZE ((uint32_t)sizeof(ZenGameModuleDescriptor))

typedef ZenResultCode (*ZenGetGameModuleFn)(ZenGameModuleDescriptor* out_descriptor);

#ifdef __cplusplus
}
#endif

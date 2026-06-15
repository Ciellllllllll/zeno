#pragma once

#include <stdint.h>

#include <zeno/zeno_abi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZEN_NATIVE_BACKEND_CONFIG_API_VERSION 1u
#define ZEN_NATIVE_BACKEND_CONFIG_SIZE ((uint32_t)sizeof(ZenNativeBackendConfig))

typedef struct ZenNativeBackendHandle {
    /* Non-zero native backend registry handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenNativeBackendHandle;

typedef struct ZenNativeBackendConfig {
    /* Must be set to ZEN_NATIVE_BACKEND_CONFIG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_NATIVE_BACKEND_CONFIG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Reserved for future backend flags. Must be 0 for this phase. */
    uint32_t flags;
    uint32_t reserved;
} ZenNativeBackendConfig;

/*
 * Creates the native backend shell.
 *
 * config: borrowed, must not be null, only needed during the call.
 * out_backend: borrowed output pointer, must not be null. Receives an owned
 * non-zero handle on success.
 * The caller must eventually pass the returned handle to zen_native_backend_destroy.
 * Thread-safety: this function may be called concurrently with other create
 * or destroy calls.
 */
ZenResultCode zen_native_backend_create(
    const ZenNativeBackendConfig* config,
    ZenNativeBackendHandle* out_backend);

/*
 * Destroys an owned native backend handle.
 *
 * backend: must be a non-zero handle returned by zen_native_backend_create.
 * After ZEN_RESULT_OK, the handle is invalid and must not be used again.
 * Thread-safety: this function may be called concurrently for different
 * handles. The same handle must not be destroyed concurrently from multiple
 * threads.
 */
ZenResultCode zen_native_backend_destroy(ZenNativeBackendHandle backend);

#ifdef __cplusplus
}
#endif

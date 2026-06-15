#pragma once

#include <stdint.h>

#include <zeno/zeno_abi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZEN_NATIVE_BACKEND_CONFIG_API_VERSION 1u
#define ZEN_NATIVE_BACKEND_CONFIG_SIZE ((uint32_t)sizeof(ZenNativeBackendConfig))
#define ZEN_NATIVE_WINDOW_CONFIG_API_VERSION 1u
#define ZEN_NATIVE_WINDOW_CONFIG_SIZE ((uint32_t)sizeof(ZenNativeWindowConfig))

typedef struct ZenNativeBackendHandle {
    /* Non-zero native backend registry handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenNativeBackendHandle;

typedef struct ZenRenderClearColorHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenRenderClearColorHandle;

typedef struct ZenNativeBackendConfig {
    /* Must be set to ZEN_NATIVE_BACKEND_CONFIG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_NATIVE_BACKEND_CONFIG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Reserved for future backend flags. Must be 0 for this phase. */
    uint32_t flags;
    uint32_t reserved;
} ZenNativeBackendConfig;

typedef struct ZenNativeWindowConfig {
    /* Must be set to ZEN_NATIVE_WINDOW_CONFIG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_NATIVE_WINDOW_CONFIG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Client area width in physical pixels. Must be greater than 0. */
    uint32_t width;
    /* Client area height in physical pixels. Must be greater than 0. */
    uint32_t height;
} ZenNativeWindowConfig;

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
 * handles. The same handle must not be used by destroy, create_window, or
 * poll_events concurrently from multiple threads. Renderer functions also
 * must not run concurrently with destroy for the same handle.
 */
ZenResultCode zen_native_backend_destroy(ZenNativeBackendHandle backend);

/*
 * Creates the backend-owned Win32 window using a default title.
 *
 * backend: must be a live non-zero backend handle.
 * config: borrowed, must not be null, only needed during the call.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_create_window(
    ZenNativeBackendHandle backend,
    const ZenNativeWindowConfig* config);

/*
 * Polls pending Win32 messages without blocking.
 *
 * backend: must be a live non-zero backend handle.
 * out_should_close: borrowed output pointer, must not be null. Receives 1 when
 * the window has requested close, otherwise 0.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_poll_events(
    ZenNativeBackendHandle backend,
    uint32_t* out_should_close);

/*
 * Initializes DirectX 11 resources for the backend-owned window.
 *
 * backend: must be a live non-zero backend handle with a created window.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_initialize_renderer(ZenNativeBackendHandle backend);

/*
 * Begins one render frame by binding the current render target.
 *
 * backend: must have an initialized renderer.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_begin_frame(ZenNativeBackendHandle backend);

/*
 * Clears the current render target to the supplied color.
 *
 * backend: must have an initialized renderer.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_clear(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a);

/*
 * Creates a backend-owned clear color resource.
 *
 * backend: must have an initialized renderer.
 * out_clear_color: borrowed output pointer, must not be null. Receives an owned
 * non-zero resource handle on success.
 * On failure, out_clear_color is left unchanged.
 * The caller must release a successful handle with
 * zen_native_backend_destroy_clear_color before or during backend shutdown.
 * Destroying the backend invalidates outstanding clear color handles.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_create_clear_color(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a,
    ZenRenderClearColorHandle* out_clear_color);

/*
 * Destroys a backend-owned clear color resource.
 *
 * backend: must have an initialized renderer. clear_color must be a non-zero
 * handle created by zen_native_backend_create_clear_color for the same backend.
 * Returns ZEN_RESULT_NOT_INITIALIZED when a non-zero handle is not present,
 * including double destroy and handles invalidated by backend destruction.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_destroy_clear_color(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color);

/*
 * Clears the current render target using a backend-owned clear color resource.
 *
 * backend: must have an initialized renderer. clear_color must be valid for
 * that backend.
 * Returns ZEN_RESULT_NOT_INITIALIZED when a non-zero resource handle is not
 * present or the renderer is no longer initialized.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_clear_with_resource(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color);

/*
 * Presents the current swap chain frame.
 *
 * backend: must have an initialized renderer.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_present(ZenNativeBackendHandle backend);

#ifdef __cplusplus
}
#endif

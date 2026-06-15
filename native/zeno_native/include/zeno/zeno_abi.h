#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZEN_ENGINE_CONFIG_API_VERSION 1u
#define ZEN_ENGINE_CONFIG_SIZE ((uint32_t)sizeof(ZenEngineConfig))
#define ZEN_MAX_TEST_FRAMES_UNLIMITED UINT64_MAX

typedef enum ZenResultCode {
    ZEN_RESULT_OK = 0,
    ZEN_RESULT_INVALID_ARGUMENT = 1,
    ZEN_RESULT_NOT_INITIALIZED = 2,
    ZEN_RESULT_ALREADY_INITIALIZED = 3,
    ZEN_RESULT_BACKEND_ERROR = 4,
    ZEN_RESULT_INTERNAL_ERROR = 100,
} ZenResultCode;

typedef struct ZenEngineHandle {
    /* Non-zero engine registry handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenEngineHandle;

typedef struct ZenEngineConfig {
    /* Must be set to ZEN_ENGINE_CONFIG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_ENGINE_CONFIG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Must be finite and greater than zero. */
    double target_fps;
    /* Use ZEN_MAX_TEST_FRAMES_UNLIMITED for no test frame limit. */
    uint64_t max_test_frames;
} ZenEngineConfig;

typedef struct ZenEngineFrameInfo {
    uint64_t frame_index;
    double delta_time_seconds;
} ZenEngineFrameInfo;

/*
 * Creates an engine runtime.
 *
 * config: borrowed, must not be null, only needed during the call.
 * out_engine: borrowed output pointer, must not be null. Receives an owned
 * non-zero handle on success.
 * The caller must eventually pass the returned handle to zen_engine_destroy.
 */
ZenResultCode zen_engine_create(const ZenEngineConfig* config, ZenEngineHandle* out_engine);

/*
 * Destroys an owned engine runtime handle.
 *
 * engine: must be a non-zero handle returned by zen_engine_create. After
 * ZEN_RESULT_OK, the handle is invalid and must not be used again.
 */
ZenResultCode zen_engine_destroy(ZenEngineHandle engine);

/*
 * Advances the engine by one frame.
 *
 * engine: must be a live non-zero handle. The handle remains owned by caller.
 */
ZenResultCode zen_engine_step(ZenEngineHandle engine);

/*
 * Advances the engine by one frame and optionally writes frame information.
 *
 * engine: must be a live non-zero handle. out_frame_info may be null. When
 * non-null, it is borrowed for the duration of the call.
 */
ZenResultCode zen_engine_step_frame(ZenEngineHandle engine, ZenEngineFrameInfo* out_frame_info);

/*
 * Requests shutdown on a live engine.
 *
 * engine: must be a live non-zero handle. The handle remains owned by caller
 * and must still be destroyed.
 */
ZenResultCode zen_engine_request_shutdown(ZenEngineHandle engine);

/*
 * Returns a static, non-owned, null-terminated string for a result code value.
 * Accepts raw integer codes so unknown values can be reported safely.
 */
const char* zen_result_to_string(uint32_t code);

#ifdef __cplusplus
}
#endif

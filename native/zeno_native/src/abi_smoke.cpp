#include <zeno/zeno_abi.h>

int main()
{
    ZenEngineConfig config{};
    config.size = ZEN_ENGINE_CONFIG_SIZE;
    config.api_version = ZEN_ENGINE_CONFIG_API_VERSION;
    config.target_fps = 1000000.0;
    config.max_test_frames = 1;

    ZenEngineHandle engine{};
    ZenResultCode result = zen_engine_create(&config, &engine);
    if (result != ZEN_RESULT_OK) {
        return zen_result_to_string((uint32_t)result) == nullptr ? 10 : 1;
    }

    ZenEngineFrameInfo frame_info{};
    result = zen_engine_step_frame(engine, &frame_info);
    if (result != ZEN_RESULT_OK) {
        zen_engine_destroy(engine);
        return 2;
    }

    result = zen_engine_request_shutdown(engine);
    if (result != ZEN_RESULT_OK) {
        zen_engine_destroy(engine);
        return 3;
    }

    result = zen_engine_destroy(engine);
    return result == ZEN_RESULT_OK ? 0 : 4;
}

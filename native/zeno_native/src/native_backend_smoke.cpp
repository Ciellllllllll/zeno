#include <zeno/zeno_native_backend.h>

#include <chrono>
#include <thread>

int main()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;
    config.flags = 0;
    config.reserved = 0;

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 1;
    }

    ZenNativeWindowConfig window_config{};
    window_config.size = ZEN_NATIVE_WINDOW_CONFIG_SIZE;
    window_config.api_version = ZEN_NATIVE_WINDOW_CONFIG_API_VERSION;
    window_config.width = 640;
    window_config.height = 360;

    result = zen_native_backend_create_window(backend, &window_config);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 2;
    }

    result = zen_native_backend_initialize_renderer(backend);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 3;
    }

    for (int i = 0; i < 120; ++i) {
        uint32_t should_close = 0;
        result = zen_native_backend_poll_events(backend, &should_close);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy(backend);
            return 4;
        }

        if (should_close != 0) {
            break;
        }

        result = zen_native_backend_begin_frame(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy(backend);
            return 5;
        }

        result = zen_native_backend_clear(backend, 0.05f, 0.20f, 0.35f, 1.0f);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy(backend);
            return 6;
        }

        result = zen_native_backend_present(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy(backend);
            return 7;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    result = zen_native_backend_destroy(backend);
    return result == ZEN_RESULT_OK ? 0 : 8;
}

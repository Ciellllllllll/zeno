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

    ZenRenderClearColorHandle clear_color{};
    result = zen_native_backend_create_clear_color(backend, 0.05f, 0.20f, 0.35f, 1.0f, &clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 4;
    }

    ZenRenderClearColorHandle second_clear_color{};
    result = zen_native_backend_create_clear_color(backend, 0.35f, 0.10f, 0.10f, 1.0f, &second_clear_color);
    if (result != ZEN_RESULT_OK || second_clear_color.value == 0 || second_clear_color.value == clear_color.value) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 5;
    }

    result = zen_native_backend_destroy_clear_color(backend, second_clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 6;
    }

    ZenRenderClearColorHandle invalid_clear_color{};
    result = zen_native_backend_clear_with_resource(backend, invalid_clear_color);
    if (result != ZEN_RESULT_INVALID_ARGUMENT) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 7;
    }

    for (int i = 0; i < 120; ++i) {
        uint32_t should_close = 0;
        result = zen_native_backend_poll_events(backend, &should_close);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 8;
        }

        if (should_close != 0) {
            break;
        }

        result = zen_native_backend_begin_frame(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 9;
        }

        result = zen_native_backend_clear_with_resource(backend, clear_color);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 10;
        }

        result = zen_native_backend_present(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 11;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    result = zen_native_backend_destroy_clear_color(backend, clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 12;
    }

    result = zen_native_backend_clear_with_resource(backend, clear_color);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 13;
    }

    result = zen_native_backend_destroy_clear_color(backend, clear_color);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 14;
    }

    result = zen_native_backend_destroy(backend);
    return result == ZEN_RESULT_OK ? 0 : 15;
}

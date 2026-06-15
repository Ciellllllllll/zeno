#include <zeno/zeno_native_backend.h>

#include <chrono>
#include <thread>

namespace {

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

} // namespace

int main()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;
    config.flags = 0;
    config.reserved = 0;

    if (!expect(zen_native_backend_create(nullptr, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 1;
    }

    ZenNativeBackendHandle scratch_backend{ 777 };
    if (!expect(zen_native_backend_create(nullptr, &scratch_backend), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_backend.value != 777) {
        return 2;
    }

    if (!expect(zen_native_backend_create(&config, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 3;
    }

    ZenNativeBackendConfig invalid_config = config;
    invalid_config.size = 0;
    scratch_backend.value = 777;
    if (!expect(zen_native_backend_create(&invalid_config, &scratch_backend), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_backend.value != 777) {
        return 4;
    }

    ZenNativeBackendHandle invalid_backend{};
    if (!expect(zen_native_backend_destroy(invalid_backend), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 5;
    }

    invalid_backend.value = 123456;
    if (!expect(zen_native_backend_begin_frame(invalid_backend), ZEN_RESULT_NOT_INITIALIZED)) {
        return 6;
    }

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 7;
    }

    if (!expect(zen_native_backend_create_window(backend, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    ZenNativeWindowConfig invalid_window_config{};
    invalid_window_config.size = ZEN_NATIVE_WINDOW_CONFIG_SIZE;
    invalid_window_config.api_version = ZEN_NATIVE_WINDOW_CONFIG_API_VERSION;
    invalid_window_config.width = 0;
    invalid_window_config.height = 360;
    if (!expect(zen_native_backend_create_window(backend, &invalid_window_config), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    uint32_t should_close = 99;
    if (!expect(zen_native_backend_poll_events(backend, nullptr), ZEN_RESULT_INVALID_ARGUMENT)
        || should_close != 99) {
        zen_native_backend_destroy(backend);
        return 10;
    }

    if (!expect(zen_native_backend_initialize_renderer(backend), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    if (!expect(zen_native_backend_begin_frame(backend), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 12;
    }

    if (!expect(zen_native_backend_clear(backend, 0.0f, 0.0f, 0.0f, 1.0f), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 13;
    }

    ZenRenderClearColorHandle scratch_clear_color{ 777 };
    if (!expect(
            zen_native_backend_create_clear_color(backend, 0.0f, 0.0f, 0.0f, 1.0f, &scratch_clear_color),
            ZEN_RESULT_BACKEND_ERROR)
        || scratch_clear_color.value != 777) {
        zen_native_backend_destroy(backend);
        return 14;
    }

    if (!expect(zen_native_backend_present(backend), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 15;
    }

    ZenNativeWindowConfig window_config{};
    window_config.size = ZEN_NATIVE_WINDOW_CONFIG_SIZE;
    window_config.api_version = ZEN_NATIVE_WINDOW_CONFIG_API_VERSION;
    window_config.width = 640;
    window_config.height = 360;

    result = zen_native_backend_create_window(backend, &window_config);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 16;
    }

    result = zen_native_backend_initialize_renderer(backend);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 17;
    }

    ZenRenderClearColorHandle clear_color{};
    result = zen_native_backend_create_clear_color(backend, 0.05f, 0.20f, 0.35f, 1.0f, &clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 18;
    }

    ZenRenderClearColorHandle second_clear_color{};
    result = zen_native_backend_create_clear_color(backend, 0.35f, 0.10f, 0.10f, 1.0f, &second_clear_color);
    if (result != ZEN_RESULT_OK || second_clear_color.value == 0 || second_clear_color.value == clear_color.value) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 19;
    }

    result = zen_native_backend_destroy_clear_color(backend, second_clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 20;
    }

    ZenRenderClearColorHandle invalid_clear_color{};
    result = zen_native_backend_clear_with_resource(backend, invalid_clear_color);
    if (result != ZEN_RESULT_INVALID_ARGUMENT) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 21;
    }

    for (int i = 0; i < 120; ++i) {
        uint32_t should_close = 0;
        result = zen_native_backend_poll_events(backend, &should_close);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 22;
        }

        if (should_close != 0) {
            break;
        }

        result = zen_native_backend_begin_frame(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 23;
        }

        result = zen_native_backend_clear_with_resource(backend, clear_color);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 24;
        }

        result = zen_native_backend_present(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 25;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    result = zen_native_backend_destroy_clear_color(backend, clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 26;
    }

    result = zen_native_backend_clear_with_resource(backend, clear_color);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 27;
    }

    result = zen_native_backend_destroy_clear_color(backend, clear_color);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 28;
    }

    result = zen_native_backend_destroy(backend);
    if (result != ZEN_RESULT_OK) {
        return 29;
    }

    result = zen_native_backend_poll_events(backend, &should_close);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        return 30;
    }

    result = zen_native_backend_destroy(backend);
    return result == ZEN_RESULT_NOT_INITIALIZED ? 0 : 31;
}

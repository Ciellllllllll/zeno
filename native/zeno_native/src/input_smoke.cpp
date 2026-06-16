#include <zeno/zeno_native_backend.h>

#include "native_backend.h"

namespace {

bool expect(bool condition)
{
    return condition;
}

bool expect_code(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

bool verify_internal_input_state()
{
    zeno::native::NativeBackend backend;
    zeno::native::NativeBackendConfig config{};
    if (!backend.initialize(config)) {
        return false;
    }

    bool should_close = true;
    if (!backend.poll_events(should_close) || should_close) {
        return false;
    }

    zeno::native::InputSnapshot snapshot{};
    if (!backend.get_input_snapshot(snapshot)) {
        return false;
    }

    if (!expect(!snapshot.key_down[ZEN_INPUT_KEY_A]
            && !snapshot.key_pressed[ZEN_INPUT_KEY_A]
            && !snapshot.key_released[ZEN_INPUT_KEY_A])) {
        return false;
    }

    if (!backend.debug_set_key_state(ZEN_INPUT_KEY_A, true)
        || !backend.debug_set_mouse_state(120, 80, ZEN_INPUT_MOUSE_BUTTON_LEFT, true, 1)) {
        return false;
    }

    snapshot = {};
    if (!backend.get_input_snapshot(snapshot)) {
        return false;
    }

    if (!expect(snapshot.key_down[ZEN_INPUT_KEY_A]
            && snapshot.key_pressed[ZEN_INPUT_KEY_A]
            && !snapshot.key_released[ZEN_INPUT_KEY_A])) {
        return false;
    }

    if (!expect(snapshot.mouse_x == 120
            && snapshot.mouse_y == 80
            && snapshot.mouse_wheel_delta == 1
            && snapshot.mouse_down[ZEN_INPUT_MOUSE_BUTTON_LEFT]
            && snapshot.mouse_pressed[ZEN_INPUT_MOUSE_BUTTON_LEFT]
            && !snapshot.mouse_released[ZEN_INPUT_MOUSE_BUTTON_LEFT])) {
        return false;
    }

    if (!backend.poll_events(should_close)) {
        return false;
    }

    snapshot = {};
    if (!backend.get_input_snapshot(snapshot)) {
        return false;
    }

    if (!expect(snapshot.key_down[ZEN_INPUT_KEY_A]
            && !snapshot.key_pressed[ZEN_INPUT_KEY_A]
            && !snapshot.key_released[ZEN_INPUT_KEY_A]
            && snapshot.mouse_wheel_delta == 0)) {
        return false;
    }

    if (!backend.debug_set_key_state(ZEN_INPUT_KEY_A, false)
        || !backend.debug_set_mouse_state(130, 90, ZEN_INPUT_MOUSE_BUTTON_LEFT, false, -1)) {
        return false;
    }

    snapshot = {};
    if (!backend.get_input_snapshot(snapshot)) {
        return false;
    }

    if (!expect(!snapshot.key_down[ZEN_INPUT_KEY_A]
            && !snapshot.key_pressed[ZEN_INPUT_KEY_A]
            && snapshot.key_released[ZEN_INPUT_KEY_A])) {
        return false;
    }

    if (!expect(snapshot.mouse_x == 130
            && snapshot.mouse_y == 90
            && snapshot.mouse_wheel_delta == -1
            && !snapshot.mouse_down[ZEN_INPUT_MOUSE_BUTTON_LEFT]
            && !snapshot.mouse_pressed[ZEN_INPUT_MOUSE_BUTTON_LEFT]
            && snapshot.mouse_released[ZEN_INPUT_MOUSE_BUTTON_LEFT])) {
        return false;
    }

    if (!backend.debug_set_key_state(ZEN_INPUT_KEY_W, true)
        || !backend.debug_set_mouse_state(10, 20, ZEN_INPUT_MOUSE_BUTTON_RIGHT, true, 0)) {
        return false;
    }

    if (!backend.poll_events(should_close)) {
        return false;
    }

    backend.clear_input_state();
    snapshot = {};
    if (!backend.get_input_snapshot(snapshot)) {
        return false;
    }

    return expect(snapshot.key_released[ZEN_INPUT_KEY_W]
        && snapshot.mouse_released[ZEN_INPUT_MOUSE_BUTTON_RIGHT]);
}

} // namespace

int main()
{
    if (!verify_internal_input_state()) {
        return 1;
    }

    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;

    ZenInputSnapshot snapshot{};
    snapshot.size = ZEN_INPUT_SNAPSHOT_SIZE;
    snapshot.api_version = ZEN_INPUT_SNAPSHOT_API_VERSION;

    if (!expect_code(zen_native_backend_get_input_snapshot(ZenNativeBackendHandle{}, &snapshot), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 2;
    }

    ZenNativeBackendHandle fake_backend{ 12345 };
    if (!expect_code(zen_native_backend_get_input_snapshot(fake_backend, &snapshot), ZEN_RESULT_NOT_INITIALIZED)) {
        return 3;
    }

    ZenNativeBackendHandle backend{};
    if (!expect_code(zen_native_backend_create(&config, &backend), ZEN_RESULT_OK)) {
        return 4;
    }

    if (!expect_code(zen_native_backend_get_input_snapshot(backend, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 5;
    }

    ZenInputSnapshot invalid_snapshot{};
    invalid_snapshot.size = 0;
    invalid_snapshot.api_version = ZEN_INPUT_SNAPSHOT_API_VERSION;
    if (!expect_code(zen_native_backend_get_input_snapshot(backend, &invalid_snapshot), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 6;
    }

    invalid_snapshot.size = ZEN_INPUT_SNAPSHOT_SIZE;
    invalid_snapshot.api_version = 0;
    if (!expect_code(zen_native_backend_get_input_snapshot(backend, &invalid_snapshot), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 7;
    }

    snapshot = {};
    snapshot.size = ZEN_INPUT_SNAPSHOT_SIZE;
    snapshot.api_version = ZEN_INPUT_SNAPSHOT_API_VERSION;
    if (!expect_code(zen_native_backend_get_input_snapshot(backend, &snapshot), ZEN_RESULT_OK)) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    if (!expect(snapshot.size == ZEN_INPUT_SNAPSHOT_SIZE
            && snapshot.api_version == ZEN_INPUT_SNAPSHOT_API_VERSION
            && snapshot.key_down[ZEN_INPUT_KEY_ESCAPE] == 0
            && snapshot.mouse_down[ZEN_INPUT_MOUSE_BUTTON_LEFT] == 0)) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    if (!expect_code(zen_native_backend_destroy(backend), ZEN_RESULT_OK)) {
        return 10;
    }

    return 0;
}

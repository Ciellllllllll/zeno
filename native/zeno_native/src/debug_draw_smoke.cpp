#include <zeno/zeno_native_backend.h>

#include <limits>

namespace {

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

ZenNativeBackendConfig make_config()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;
    return config;
}

ZenDebugLineDesc make_line()
{
    ZenDebugLineDesc desc{};
    desc.size = ZEN_DEBUG_LINE_DESC_SIZE;
    desc.api_version = ZEN_DEBUG_LINE_DESC_API_VERSION;
    desc.start[0] = -0.25f;
    desc.start[1] = -0.25f;
    desc.start[2] = 0.0f;
    desc.end[0] = 0.25f;
    desc.end[1] = 0.25f;
    desc.end[2] = 0.0f;
    desc.color[0] = 0.2f;
    desc.color[1] = 1.0f;
    desc.color[2] = 0.3f;
    desc.color[3] = 1.0f;
    return desc;
}

ZenDebugRectDesc make_rect()
{
    ZenDebugRectDesc desc{};
    desc.size = ZEN_DEBUG_RECT_DESC_SIZE;
    desc.api_version = ZEN_DEBUG_RECT_DESC_API_VERSION;
    desc.center[0] = 0.0f;
    desc.center[1] = 0.0f;
    desc.half_extents[0] = 0.35f;
    desc.half_extents[1] = 0.2f;
    desc.z = 0.0f;
    desc.color[0] = 1.0f;
    desc.color[1] = 0.5f;
    desc.color[2] = 0.1f;
    desc.color[3] = 1.0f;
    return desc;
}

} // namespace

int main()
{
    ZenDebugLineDesc line = make_line();
    ZenDebugRectDesc rect = make_rect();

    if (!expect(zen_native_backend_draw_debug_line({}, &line), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 1;
    }
    if (!expect(zen_native_backend_draw_debug_rect({}, &rect), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 2;
    }
    if (!expect(zen_native_backend_draw_debug_line({ 12345 }, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 3;
    }
    if (!expect(zen_native_backend_draw_debug_rect({ 12345 }, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 4;
    }

    line.size = 0;
    if (!expect(zen_native_backend_draw_debug_line({ 12345 }, &line), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 5;
    }
    line = make_line();
    line.color[0] = std::numeric_limits<float>::infinity();
    if (!expect(zen_native_backend_draw_debug_line({ 12345 }, &line), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 6;
    }

    rect.api_version = 0;
    if (!expect(zen_native_backend_draw_debug_rect({ 12345 }, &rect), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 7;
    }
    rect = make_rect();
    rect.half_extents[1] = -0.1f;
    if (!expect(zen_native_backend_draw_debug_rect({ 12345 }, &rect), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 8;
    }

    ZenNativeBackendHandle backend{};
    const ZenNativeBackendConfig config = make_config();
    if (!expect(zen_native_backend_create(&config, &backend), ZEN_RESULT_OK)) {
        return 9;
    }

    line = make_line();
    rect = make_rect();
    if (!expect(zen_native_backend_draw_debug_line(backend, &line), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 10;
    }
    if (!expect(zen_native_backend_draw_debug_rect(backend, &rect), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    if (!expect(zen_native_backend_destroy(backend), ZEN_RESULT_OK)) {
        return 12;
    }

    return 0;
}

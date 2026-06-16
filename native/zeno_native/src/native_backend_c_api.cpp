#include <zeno/zeno_native_backend.h>

#include "native_backend.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace {

constexpr std::uint32_t kConfigApiVersion = 1;
constexpr std::uint32_t kConfigSize = sizeof(ZenNativeBackendConfig);
constexpr std::uint32_t kWindowConfigApiVersion = 1;
constexpr std::uint32_t kWindowConfigSize = sizeof(ZenNativeWindowConfig);
constexpr std::uint64_t kInvalidHandle = 0;

std::mutex g_backend_mutex;
std::unordered_map<std::uint64_t, std::shared_ptr<zeno::native::NativeBackend>> g_backends;
std::uint64_t g_next_backend_handle = 1;

bool is_valid_config(const ZenNativeBackendConfig& config)
{
    return config.size == kConfigSize
        && config.api_version == kConfigApiVersion
        && config.flags == 0
        && config.reserved == 0;
}

bool is_valid_window_config(const ZenNativeWindowConfig& config)
{
    return config.size == kWindowConfigSize
        && config.api_version == kWindowConfigApiVersion
        && config.width > 0
        && config.height > 0;
}

bool allocate_handle(std::uint64_t& out_handle)
{
    if (g_next_backend_handle == UINT64_MAX) {
        return false;
    }

    out_handle = g_next_backend_handle++;
    return out_handle != kInvalidHandle;
}

ZenResultCode map_render_command_result(zeno::native::RenderCommandResult result)
{
    switch (result) {
    case zeno::native::RenderCommandResult::ok:
        return ZEN_RESULT_OK;
    case zeno::native::RenderCommandResult::wrong_state:
        return ZEN_RESULT_BACKEND_ERROR;
    case zeno::native::RenderCommandResult::missing_resource:
        return ZEN_RESULT_NOT_INITIALIZED;
    }

    return ZEN_RESULT_INTERNAL_ERROR;
}

ZenResultCode with_backend(
    ZenNativeBackendHandle backend,
    const auto& action)
{
    if (backend.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    std::shared_ptr<zeno::native::NativeBackend> native_backend;
    {
        std::lock_guard<std::mutex> lock(g_backend_mutex);
        auto found = g_backends.find(backend.value);
        if (found == g_backends.end()) {
            return ZEN_RESULT_NOT_INITIALIZED;
        }

        native_backend = found->second;
    }

    return action(*native_backend);
}

} // namespace

extern "C" ZenResultCode zen_native_backend_create(
    const ZenNativeBackendConfig* config,
    ZenNativeBackendHandle* out_backend)
{
    try {
        if (config == nullptr || out_backend == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_config(*config)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        auto backend = std::make_shared<zeno::native::NativeBackend>();
        zeno::native::NativeBackendConfig native_config{};
        native_config.flags = config->flags;

        if (!backend->initialize(native_config)) {
            return ZEN_RESULT_INTERNAL_ERROR;
        }

        std::lock_guard<std::mutex> lock(g_backend_mutex);
        std::uint64_t handle = kInvalidHandle;
        if (!allocate_handle(handle)) {
            backend->shutdown();
            return ZEN_RESULT_INTERNAL_ERROR;
        }

        g_backends.emplace(handle, std::move(backend));
        out_backend->value = handle;
        return ZEN_RESULT_OK;
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy(ZenNativeBackendHandle backend)
{
    try {
        if (backend.value == kInvalidHandle) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        std::shared_ptr<zeno::native::NativeBackend> owned_backend;
        {
            std::lock_guard<std::mutex> lock(g_backend_mutex);
            auto found = g_backends.find(backend.value);
            if (found == g_backends.end()) {
                return ZEN_RESULT_NOT_INITIALIZED;
            }

            owned_backend = std::move(found->second);
            g_backends.erase(found);
        }

        owned_backend->shutdown();
        return ZEN_RESULT_OK;
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_window(
    ZenNativeBackendHandle backend,
    const ZenNativeWindowConfig* config)
{
    try {
        if (config == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_window_config(*config)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [config](zeno::native::NativeBackend& native_backend) {
            zeno::native::NativeWindowConfig native_config{};
            native_config.width = config->width;
            native_config.height = config->height;

            return native_backend.create_window(native_config)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_INTERNAL_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_poll_events(
    ZenNativeBackendHandle backend,
    std::uint32_t* out_should_close)
{
    try {
        if (out_should_close == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [out_should_close](zeno::native::NativeBackend& native_backend) {
            bool should_close = false;
            if (!native_backend.poll_events(should_close)) {
                return ZEN_RESULT_INTERNAL_ERROR;
            }

            *out_should_close = should_close ? 1u : 0u;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_initialize_renderer(ZenNativeBackendHandle backend)
{
    try {
        return with_backend(backend, [](zeno::native::NativeBackend& native_backend) {
            return native_backend.initialize_renderer()
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_begin_frame(ZenNativeBackendHandle backend)
{
    try {
        return with_backend(backend, [](zeno::native::NativeBackend& native_backend) {
            return native_backend.begin_frame()
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_clear(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a)
{
    try {
        return with_backend(backend, [r, g, b, a](zeno::native::NativeBackend& native_backend) {
            return native_backend.clear(r, g, b, a)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_clear_color(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a,
    ZenRenderClearColorHandle* out_clear_color)
{
    try {
        if (out_clear_color == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [r, g, b, a, out_clear_color](zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_clear_color(r, g, b, a, handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_clear_color->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_clear_color(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color)
{
    try {
        if (clear_color.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [clear_color](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_clear_color(clear_color.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_clear_with_resource(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color)
{
    try {
        if (clear_color.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [clear_color](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(native_backend.clear_with_resource(clear_color.value));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle* out_triangle)
{
    try {
        if (out_triangle == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [out_triangle](zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_triangle(handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_triangle->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle)
{
    try {
        if (triangle.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [triangle](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_triangle(triangle.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle)
{
    try {
        if (triangle.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [triangle](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(native_backend.draw_triangle(triangle.value));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_present(ZenNativeBackendHandle backend)
{
    try {
        return with_backend(backend, [](zeno::native::NativeBackend& native_backend) {
            return native_backend.present()
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

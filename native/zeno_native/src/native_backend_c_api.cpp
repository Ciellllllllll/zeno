#include <zeno/zeno_native_backend.h>

#include "native_backend.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace {

constexpr std::uint32_t kConfigApiVersion = 1;
constexpr std::uint32_t kConfigSize = sizeof(ZenNativeBackendConfig);
constexpr std::uint64_t kInvalidHandle = 0;

std::mutex g_backend_mutex;
std::unordered_map<std::uint64_t, std::unique_ptr<zeno::native::NativeBackend>> g_backends;
std::uint64_t g_next_backend_handle = 1;

bool is_valid_config(const ZenNativeBackendConfig& config)
{
    return config.size == kConfigSize
        && config.api_version == kConfigApiVersion
        && config.flags == 0
        && config.reserved == 0;
}

bool allocate_handle(std::uint64_t& out_handle)
{
    if (g_next_backend_handle == UINT64_MAX) {
        return false;
    }

    out_handle = g_next_backend_handle++;
    return out_handle != kInvalidHandle;
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

        auto backend = std::make_unique<zeno::native::NativeBackend>();
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

        std::unique_ptr<zeno::native::NativeBackend> owned_backend;
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

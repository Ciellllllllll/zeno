#include "native_backend.h"

#include <iostream>

namespace zeno::native {

bool NativeBackend::initialize(const NativeBackendConfig& config)
{
    if (initialized_) {
        return false;
    }

    config_ = config;
    initialized_ = true;
    std::cerr << "[ZENO][native] backend initialized\n";
    return true;
}

void NativeBackend::shutdown()
{
    if (!initialized_) {
        return;
    }

    initialized_ = false;
    std::cerr << "[ZENO][native] backend shutdown\n";
}

bool NativeBackend::is_initialized() const
{
    return initialized_;
}

} // namespace zeno::native

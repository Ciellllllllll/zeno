#pragma once

#include <cstdint>

namespace zeno::native {

struct NativeBackendConfig final {
    std::uint32_t flags = 0;
};

class NativeBackend final {
public:
    bool initialize(const NativeBackendConfig& config);
    void shutdown();

    bool is_initialized() const;

private:
    bool initialized_ = false;
    NativeBackendConfig config_{};
};

} // namespace zeno::native

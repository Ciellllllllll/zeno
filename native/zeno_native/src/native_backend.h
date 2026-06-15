#pragma once

#include <cstdint>

struct HWND__;

namespace zeno::native {

struct NativeBackendConfig final {
    std::uint32_t flags = 0;
};

struct NativeWindowConfig final {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

class NativeBackend final {
public:
    bool initialize(const NativeBackendConfig& config);
    void shutdown();

    bool create_window(const NativeWindowConfig& config);
    bool poll_events(bool& out_should_close);
    void notify_window_destroyed(HWND__* window);
    bool is_initialized() const;
    bool has_window() const;

private:
    void destroy_window();

    bool initialized_ = false;
    bool should_close_ = false;
    NativeBackendConfig config_{};
    HWND__* window_handle_ = nullptr;
};

} // namespace zeno::native

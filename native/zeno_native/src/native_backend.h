#pragma once

#include <cstdint>
#include <memory>

struct HWND__;

namespace zeno::native {

class DirectX11Renderer;

struct NativeBackendConfig final {
    std::uint32_t flags = 0;
};

struct NativeWindowConfig final {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

class NativeBackend final {
public:
    NativeBackend();
    ~NativeBackend();

    bool initialize(const NativeBackendConfig& config);
    void shutdown();

    bool create_window(const NativeWindowConfig& config);
    bool poll_events(bool& out_should_close);
    void notify_window_destroyed(HWND__* window);
    bool initialize_renderer();
    bool begin_frame();
    bool clear(float r, float g, float b, float a);
    bool present();
    bool is_initialized() const;
    bool has_window() const;

private:
    void destroy_renderer();
    void destroy_window();

    bool initialized_ = false;
    bool should_close_ = false;
    NativeBackendConfig config_{};
    HWND__* window_handle_ = nullptr;
    std::unique_ptr<DirectX11Renderer> renderer_;
};

} // namespace zeno::native

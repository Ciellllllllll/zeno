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

enum class RenderCommandResult {
    ok,
    wrong_state,
    missing_resource,
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
    bool create_clear_color(float r, float g, float b, float a, std::uint64_t& out_handle);
    bool destroy_clear_color(std::uint64_t handle);
    RenderCommandResult clear_with_resource(std::uint64_t handle);
    bool create_triangle(std::uint64_t& out_handle);
    bool destroy_triangle(std::uint64_t handle);
    RenderCommandResult draw_triangle(std::uint64_t handle);
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

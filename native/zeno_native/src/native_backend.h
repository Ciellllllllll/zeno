#pragma once

#include <cstdint>
#include <memory>

struct HWND__;

namespace zeno::native {

class DirectX11Renderer;

constexpr std::uint32_t kInputKeyCount = 11;
constexpr std::uint32_t kInputMouseButtonCount = 3;

struct NativeBackendConfig final {
    std::uint32_t flags = 0;
};

struct NativeWindowConfig final {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

struct InputSnapshot final {
    bool key_down[kInputKeyCount]{};
    bool key_pressed[kInputKeyCount]{};
    bool key_released[kInputKeyCount]{};
    bool mouse_down[kInputMouseButtonCount]{};
    bool mouse_pressed[kInputMouseButtonCount]{};
    bool mouse_released[kInputMouseButtonCount]{};
    std::int32_t mouse_x = 0;
    std::int32_t mouse_y = 0;
    std::int32_t mouse_wheel_delta = 0;
};

enum class RenderCommandResult {
    ok,
    wrong_state,
    missing_resource,
};

struct Matrix4x4 final {
    float elements[16]{};
};

struct SpriteDrawDesc final {
    Matrix4x4 model_matrix{};
    float color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct ShaderCompileLog final {
    char message[1024]{};
    std::uint32_t message_length = 0;
};

struct VertexInputElement final {
    std::uint32_t semantic = 0;
    std::uint32_t semantic_index = 0;
    std::uint32_t format = 0;
    std::uint32_t input_slot = 0;
    std::uint32_t aligned_byte_offset = 0;
};

struct VertexInputLayoutDesc final {
    VertexInputElement elements[8]{};
    std::uint32_t element_count = 0;
};

class NativeBackend final {
public:
    NativeBackend();
    ~NativeBackend();

    bool initialize(const NativeBackendConfig& config);
    void shutdown();

    bool create_window(const NativeWindowConfig& config);
    bool poll_events(bool& out_should_close);
    bool get_input_snapshot(InputSnapshot& out_snapshot) const;
    bool debug_set_key_state(std::uint32_t key_code, bool is_down);
    bool debug_set_mouse_state(std::int32_t x, std::int32_t y, std::uint32_t button, bool is_down, std::int32_t wheel_delta);
    void handle_key_message(std::uint32_t key_code, bool is_down);
    void handle_mouse_move(std::int32_t x, std::int32_t y);
    void handle_mouse_button(std::uint32_t button, bool is_down);
    void handle_mouse_wheel(std::int32_t wheel_delta);
    void clear_input_state();
    void notify_window_destroyed(HWND__* window);
    bool initialize_renderer();
    bool begin_frame();
    bool clear(float r, float g, float b, float a);
    bool create_clear_color(float r, float g, float b, float a, std::uint64_t& out_handle);
    bool destroy_clear_color(std::uint64_t handle);
    RenderCommandResult clear_with_resource(std::uint64_t handle);
    bool create_triangle(std::uint64_t& out_handle);
    bool create_triangle_with_shaders(
        std::uint64_t vertex_shader,
        std::uint64_t pixel_shader,
        const VertexInputLayoutDesc& input_layout,
        std::uint64_t& out_handle);
    bool destroy_triangle(std::uint64_t handle);
    bool create_vertex_shader_from_source(
        const char* source,
        std::uint64_t source_length,
        const char* entry,
        std::uint64_t entry_length,
        const char* profile,
        std::uint64_t profile_length,
        ShaderCompileLog& compile_log,
        std::uint64_t& out_handle);
    bool create_pixel_shader_from_source(
        const char* source,
        std::uint64_t source_length,
        const char* entry,
        std::uint64_t entry_length,
        const char* profile,
        std::uint64_t profile_length,
        ShaderCompileLog& compile_log,
        std::uint64_t& out_handle);
    bool destroy_vertex_shader(std::uint64_t handle);
    bool destroy_pixel_shader(std::uint64_t handle);
    bool create_texture_from_memory(const std::uint8_t* image_bytes, std::uint64_t image_byte_count, std::uint64_t& out_handle);
    bool destroy_texture(std::uint64_t handle);
    RenderCommandResult draw_sprite(std::uint64_t texture, const SpriteDrawDesc& desc);
    RenderCommandResult draw_triangle(std::uint64_t handle);
    bool set_camera_matrix(const Matrix4x4& matrix);
    RenderCommandResult draw_triangle_transformed(std::uint64_t handle, const Matrix4x4& model_matrix);
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
    bool current_keys_[kInputKeyCount]{};
    bool previous_keys_[kInputKeyCount]{};
    bool current_mouse_buttons_[kInputMouseButtonCount]{};
    bool previous_mouse_buttons_[kInputMouseButtonCount]{};
    std::int32_t mouse_x_ = 0;
    std::int32_t mouse_y_ = 0;
    std::int32_t frame_mouse_wheel_delta_ = 0;
    std::int32_t pending_mouse_wheel_delta_ = 0;
};

} // namespace zeno::native

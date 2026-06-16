#pragma once

#include <cstdint>

#include <zeno/zeno_abi.h>
#include <zeno/math.hpp>
#include <zeno/zeno_native_backend.h>

namespace zeno {

enum class ResultCode : std::uint32_t {
    ok = ZEN_RESULT_OK,
    invalid_argument = ZEN_RESULT_INVALID_ARGUMENT,
    not_initialized = ZEN_RESULT_NOT_INITIALIZED,
    already_initialized = ZEN_RESULT_ALREADY_INITIALIZED,
    backend_error = ZEN_RESULT_BACKEND_ERROR,
    internal_error = ZEN_RESULT_INTERNAL_ERROR,
};

class Result final {
public:
    constexpr Result() = default;
    constexpr explicit Result(ZenResultCode code)
        : code_(code)
    {
    }

    constexpr bool ok() const { return code_ == ZEN_RESULT_OK; }
    constexpr ZenResultCode native_code() const { return code_; }
    const char* message() const;

private:
    ZenResultCode code_ = ZEN_RESULT_OK;
};

struct EngineConfig final {
    double target_fps = 60.0;
    std::uint64_t max_test_frames = ZEN_MAX_TEST_FRAMES_UNLIMITED;
};

class Engine final {
public:
    Engine() = default;
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&& other) noexcept;
    Engine& operator=(Engine&& other) noexcept;

    static Result create(const EngineConfig& config, Engine& out_engine);

    Result step();
    Result request_shutdown();
    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    explicit Engine(ZenEngineHandle handle);

    ZenEngineHandle handle_{};
};

struct WindowConfig final {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

struct Color final {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

enum class Key : std::uint32_t {
    unknown = ZEN_INPUT_KEY_UNKNOWN,
    escape = ZEN_INPUT_KEY_ESCAPE,
    space = ZEN_INPUT_KEY_SPACE,
    left = ZEN_INPUT_KEY_LEFT,
    right = ZEN_INPUT_KEY_RIGHT,
    up = ZEN_INPUT_KEY_UP,
    down = ZEN_INPUT_KEY_DOWN,
    a = ZEN_INPUT_KEY_A,
    d = ZEN_INPUT_KEY_D,
    w = ZEN_INPUT_KEY_W,
    s = ZEN_INPUT_KEY_S,
};

enum class MouseButton : std::uint32_t {
    left = ZEN_INPUT_MOUSE_BUTTON_LEFT,
    right = ZEN_INPUT_MOUSE_BUTTON_RIGHT,
    middle = ZEN_INPUT_MOUSE_BUTTON_MIDDLE,
};

struct InputSnapshot final {
    bool key_down[ZEN_INPUT_KEY_COUNT]{};
    bool key_pressed[ZEN_INPUT_KEY_COUNT]{};
    bool key_released[ZEN_INPUT_KEY_COUNT]{};
    bool mouse_down[ZEN_INPUT_MOUSE_BUTTON_COUNT]{};
    bool mouse_pressed[ZEN_INPUT_MOUSE_BUTTON_COUNT]{};
    bool mouse_released[ZEN_INPUT_MOUSE_BUTTON_COUNT]{};
    std::int32_t mouse_x = 0;
    std::int32_t mouse_y = 0;
    std::int32_t mouse_wheel_delta = 0;

    bool down(Key key) const;
    bool pressed(Key key) const;
    bool released(Key key) const;
    bool down(MouseButton button) const;
    bool pressed(MouseButton button) const;
    bool released(MouseButton button) const;
};

class RenderTriangle;

class NativeBackend final {
public:
    NativeBackend() = default;
    ~NativeBackend();

    NativeBackend(const NativeBackend&) = delete;
    NativeBackend& operator=(const NativeBackend&) = delete;

    NativeBackend(NativeBackend&& other) noexcept;
    NativeBackend& operator=(NativeBackend&& other) noexcept;

    static Result create(NativeBackend& out_backend);

    Result create_window(const WindowConfig& config);
    Result poll_events(bool& out_should_close);
    Result input_snapshot(InputSnapshot& out_snapshot);
    Result initialize_renderer();
    Result begin_frame();
    Result clear(const Color& color);
    Result create_triangle(RenderTriangle& out_triangle);
    Result set_camera_matrix(const Mat4& camera_matrix);
    Result draw_triangle(const RenderTriangle& triangle);
    Result draw_triangle(const RenderTriangle& triangle, const Mat4& model_matrix);
    Result draw_triangle(const RenderTriangle& triangle, const Transform& transform);
    Result present();
    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    explicit NativeBackend(ZenNativeBackendHandle handle);

    ZenNativeBackendHandle handle_{};
};

class RenderTriangle final {
public:
    RenderTriangle() = default;
    ~RenderTriangle();

    RenderTriangle(const RenderTriangle&) = delete;
    RenderTriangle& operator=(const RenderTriangle&) = delete;

    RenderTriangle(RenderTriangle&& other) noexcept;
    RenderTriangle& operator=(RenderTriangle&& other) noexcept;

    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;

    RenderTriangle(ZenNativeBackendHandle backend, ZenRenderTriangleHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenRenderTriangleHandle handle_{};
};

} // namespace zeno

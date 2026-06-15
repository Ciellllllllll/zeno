#pragma once

#include <cstdint>

#include <zeno/zeno_abi.h>
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
    Result initialize_renderer();
    Result begin_frame();
    Result clear(const Color& color);
    Result present();
    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    explicit NativeBackend(ZenNativeBackendHandle handle);

    ZenNativeBackendHandle handle_{};
};

} // namespace zeno

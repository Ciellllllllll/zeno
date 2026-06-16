#include <zeno/zeno.hpp>
#include <zeno/game_module.hpp>

#include <utility>

namespace zeno {

const char* Result::message() const
{
    return zen_result_to_string(static_cast<std::uint32_t>(code_));
}

namespace {

bool valid_key_index(std::uint32_t index)
{
    return index > ZEN_INPUT_KEY_UNKNOWN && index < ZEN_INPUT_KEY_COUNT;
}

bool valid_mouse_button_index(std::uint32_t index)
{
    return index < ZEN_INPUT_MOUSE_BUTTON_COUNT;
}

} // namespace

bool InputSnapshot::down(Key key) const
{
    const auto index = static_cast<std::uint32_t>(key);
    return valid_key_index(index) && key_down[index];
}

bool InputSnapshot::pressed(Key key) const
{
    const auto index = static_cast<std::uint32_t>(key);
    return valid_key_index(index) && key_pressed[index];
}

bool InputSnapshot::released(Key key) const
{
    const auto index = static_cast<std::uint32_t>(key);
    return valid_key_index(index) && key_released[index];
}

bool InputSnapshot::down(MouseButton button) const
{
    const auto index = static_cast<std::uint32_t>(button);
    return valid_mouse_button_index(index) && mouse_down[index];
}

bool InputSnapshot::pressed(MouseButton button) const
{
    const auto index = static_cast<std::uint32_t>(button);
    return valid_mouse_button_index(index) && mouse_pressed[index];
}

bool InputSnapshot::released(MouseButton button) const
{
    const auto index = static_cast<std::uint32_t>(button);
    return valid_mouse_button_index(index) && mouse_released[index];
}

Engine::Engine(ZenEngineHandle handle)
    : handle_(handle)
{
}

Engine::~Engine()
{
    reset();
}

Engine::Engine(Engine&& other) noexcept
    : handle_(std::exchange(other.handle_, ZenEngineHandle{}))
{
}

Engine& Engine::operator=(Engine&& other) noexcept
{
    if (this != &other) {
        reset();
        handle_ = std::exchange(other.handle_, ZenEngineHandle{});
    }

    return *this;
}

Result Engine::create(const EngineConfig& config, Engine& out_engine)
{
    ZenEngineConfig native_config{};
    native_config.size = sizeof(ZenEngineConfig);
    native_config.api_version = 1;
    native_config.target_fps = config.target_fps;
    native_config.max_test_frames = config.max_test_frames;

    ZenEngineHandle handle{};
    const ZenResultCode result = zen_engine_create(&native_config, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_engine = Engine(handle);
    return Result();
}

Result Engine::step()
{
    return Result(zen_engine_step(handle_));
}

Result Engine::request_shutdown()
{
    return Result(zen_engine_request_shutdown(handle_));
}

void Engine::reset()
{
    if (handle_.value == 0) {
        return;
    }

    zen_engine_destroy(handle_);
    handle_ = {};
}

NativeBackend::NativeBackend(ZenNativeBackendHandle handle)
    : handle_(handle)
{
}

NativeBackend::~NativeBackend()
{
    reset();
}

NativeBackend::NativeBackend(NativeBackend&& other) noexcept
    : handle_(std::exchange(other.handle_, ZenNativeBackendHandle{}))
{
}

NativeBackend& NativeBackend::operator=(NativeBackend&& other) noexcept
{
    if (this != &other) {
        reset();
        handle_ = std::exchange(other.handle_, ZenNativeBackendHandle{});
    }

    return *this;
}

Result NativeBackend::create(NativeBackend& out_backend)
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;
    config.flags = 0;
    config.reserved = 0;

    ZenNativeBackendHandle handle{};
    const ZenResultCode result = zen_native_backend_create(&config, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_backend = NativeBackend(handle);
    return Result();
}

Result NativeBackend::create_window(const WindowConfig& config)
{
    ZenNativeWindowConfig native_config{};
    native_config.size = ZEN_NATIVE_WINDOW_CONFIG_SIZE;
    native_config.api_version = ZEN_NATIVE_WINDOW_CONFIG_API_VERSION;
    native_config.width = config.width;
    native_config.height = config.height;

    return Result(zen_native_backend_create_window(handle_, &native_config));
}

Result NativeBackend::poll_events(bool& out_should_close)
{
    std::uint32_t should_close = 0;
    const ZenResultCode result = zen_native_backend_poll_events(handle_, &should_close);
    out_should_close = should_close != 0;
    return Result(result);
}

Result NativeBackend::input_snapshot(InputSnapshot& out_snapshot)
{
    ZenInputSnapshot snapshot{};
    snapshot.size = ZEN_INPUT_SNAPSHOT_SIZE;
    snapshot.api_version = ZEN_INPUT_SNAPSHOT_API_VERSION;

    const ZenResultCode result = zen_native_backend_get_input_snapshot(handle_, &snapshot);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    for (std::uint32_t i = 0; i < ZEN_INPUT_KEY_COUNT; ++i) {
        out_snapshot.key_down[i] = snapshot.key_down[i] != 0;
        out_snapshot.key_pressed[i] = snapshot.key_pressed[i] != 0;
        out_snapshot.key_released[i] = snapshot.key_released[i] != 0;
    }

    for (std::uint32_t i = 0; i < ZEN_INPUT_MOUSE_BUTTON_COUNT; ++i) {
        out_snapshot.mouse_down[i] = snapshot.mouse_down[i] != 0;
        out_snapshot.mouse_pressed[i] = snapshot.mouse_pressed[i] != 0;
        out_snapshot.mouse_released[i] = snapshot.mouse_released[i] != 0;
    }

    out_snapshot.mouse_x = snapshot.mouse_x;
    out_snapshot.mouse_y = snapshot.mouse_y;
    out_snapshot.mouse_wheel_delta = snapshot.mouse_wheel_delta;
    return Result();
}

Result NativeBackend::initialize_renderer()
{
    return Result(zen_native_backend_initialize_renderer(handle_));
}

Result NativeBackend::begin_frame()
{
    return Result(zen_native_backend_begin_frame(handle_));
}

Result NativeBackend::clear(const Color& color)
{
    return Result(zen_native_backend_clear(handle_, color.r, color.g, color.b, color.a));
}

Result NativeBackend::create_triangle(RenderTriangle& out_triangle)
{
    ZenRenderTriangleHandle handle{};
    const ZenResultCode result = zen_native_backend_create_triangle(handle_, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_triangle = RenderTriangle(handle_, handle);
    return Result();
}

Result NativeBackend::draw_triangle(const RenderTriangle& triangle)
{
    return Result(zen_native_backend_draw_triangle(handle_, triangle.handle_));
}

Result NativeBackend::present()
{
    return Result(zen_native_backend_present(handle_));
}

void NativeBackend::reset()
{
    if (handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy(handle_);
    handle_ = {};
}

RenderTriangle::RenderTriangle(ZenNativeBackendHandle backend, ZenRenderTriangleHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

RenderTriangle::~RenderTriangle()
{
    reset();
}

RenderTriangle::RenderTriangle(RenderTriangle&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenRenderTriangleHandle{}))
{
}

RenderTriangle& RenderTriangle::operator=(RenderTriangle&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenRenderTriangleHandle{});
    }

    return *this;
}

void RenderTriangle::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_triangle(backend_, handle_);
    backend_ = {};
    handle_ = {};
}

} // namespace zeno

namespace zeno {

Result initialize_game_module(GameModule& module, GameContext& context)
{
    if (module.on_init == nullptr) {
        return Result();
    }

    return module.on_init(context);
}

Result run_game_module_frame(GameModule& module, GameContext& context)
{
    if (module.on_update != nullptr) {
        Result result = module.on_update(context);
        if (!result.ok()) {
            return result;
        }
    }

    if (context.should_close) {
        return Result();
    }

    if (module.on_render != nullptr) {
        return module.on_render(context);
    }

    return Result();
}

Result shutdown_game_module(GameModule& module, GameContext& context)
{
    if (module.on_shutdown == nullptr) {
        return Result();
    }

    return module.on_shutdown(context);
}

} // namespace zeno

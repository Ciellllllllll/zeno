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

ZenMatrix4x4 to_native_matrix(const Mat4& matrix)
{
    ZenMatrix4x4 native_matrix{};
    for (std::uint32_t i = 0; i < 16; ++i) {
        native_matrix.elements[i] = matrix.m[i];
    }

    return native_matrix;
}

ZenShaderCompileLog make_native_compile_log()
{
    ZenShaderCompileLog log{};
    log.size = ZEN_SHADER_COMPILE_LOG_SIZE;
    log.api_version = ZEN_SHADER_COMPILE_LOG_API_VERSION;
    return log;
}

void copy_compile_log(const ZenShaderCompileLog& native_log, ShaderCompileLog& out_log)
{
    out_log.message.assign(native_log.message, native_log.message + native_log.message_length);
}

ZenVertexInputLayoutDesc make_triangle_input_layout()
{
    ZenVertexInputLayoutDesc desc{};
    desc.size = ZEN_VERTEX_INPUT_LAYOUT_DESC_SIZE;
    desc.api_version = ZEN_VERTEX_INPUT_LAYOUT_DESC_API_VERSION;
    desc.element_count = 2;
    desc.elements[0].semantic = ZEN_VERTEX_INPUT_SEMANTIC_POSITION;
    desc.elements[0].semantic_index = 0;
    desc.elements[0].format = ZEN_VERTEX_INPUT_FORMAT_FLOAT3;
    desc.elements[0].input_slot = 0;
    desc.elements[0].aligned_byte_offset = 0;
    desc.elements[1].semantic = ZEN_VERTEX_INPUT_SEMANTIC_COLOR;
    desc.elements[1].semantic_index = 0;
    desc.elements[1].format = ZEN_VERTEX_INPUT_FORMAT_FLOAT4;
    desc.elements[1].input_slot = 0;
    desc.elements[1].aligned_byte_offset = sizeof(float) * 3;
    return desc;
}

ZenSpriteDrawDesc make_native_sprite_draw_desc(const SpriteDrawDesc& desc)
{
    ZenSpriteDrawDesc native_desc{};
    native_desc.size = ZEN_SPRITE_DRAW_DESC_SIZE;
    native_desc.api_version = ZEN_SPRITE_DRAW_DESC_API_VERSION;
    native_desc.model_matrix = to_native_matrix(desc.transform.matrix());
    native_desc.color[0] = desc.color.r;
    native_desc.color[1] = desc.color.g;
    native_desc.color[2] = desc.color.b;
    native_desc.color[3] = desc.color.a;
    return native_desc;
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

Result NativeBackend::create_vertex_shader(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    std::string_view entry,
    VertexShader& out_shader,
    ShaderCompileLog& out_log)
{
    std::string source;
    Result result = assets.read_text(relative_path_utf8, source);
    if (!result.ok()) {
        out_log.message.clear();
        return result;
    }

    ZenShaderCompileLog native_log = make_native_compile_log();
    ZenVertexShaderHandle handle{};
    constexpr std::string_view profile = "vs_4_0";
    const ZenResultCode native_result = zen_native_backend_create_vertex_shader_from_source(
        handle_,
        source.data(),
        source.size(),
        entry.data(),
        entry.size(),
        profile.data(),
        profile.size(),
        &native_log,
        &handle);
    copy_compile_log(native_log, out_log);
    if (native_result != ZEN_RESULT_OK) {
        return Result(native_result);
    }

    out_shader = VertexShader(handle_, handle);
    return Result();
}

Result NativeBackend::create_pixel_shader(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    std::string_view entry,
    PixelShader& out_shader,
    ShaderCompileLog& out_log)
{
    std::string source;
    Result result = assets.read_text(relative_path_utf8, source);
    if (!result.ok()) {
        out_log.message.clear();
        return result;
    }

    ZenShaderCompileLog native_log = make_native_compile_log();
    ZenPixelShaderHandle handle{};
    constexpr std::string_view profile = "ps_4_0";
    const ZenResultCode native_result = zen_native_backend_create_pixel_shader_from_source(
        handle_,
        source.data(),
        source.size(),
        entry.data(),
        entry.size(),
        profile.data(),
        profile.size(),
        &native_log,
        &handle);
    copy_compile_log(native_log, out_log);
    if (native_result != ZEN_RESULT_OK) {
        return Result(native_result);
    }

    out_shader = PixelShader(handle_, handle);
    return Result();
}

Result NativeBackend::create_texture(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    Texture& out_texture)
{
    std::vector<std::uint8_t> bytes;
    Result result = assets.read_binary(relative_path_utf8, bytes);
    if (!result.ok()) {
        return result;
    }

    ZenTextureHandle handle{};
    const ZenResultCode native_result = zen_native_backend_create_texture_from_memory(
        handle_,
        bytes.data(),
        bytes.size(),
        &handle);
    if (native_result != ZEN_RESULT_OK) {
        return Result(native_result);
    }

    out_texture = Texture(handle_, handle);
    return Result();
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

Result NativeBackend::create_triangle(
    const VertexShader& vertex_shader,
    const PixelShader& pixel_shader,
    RenderTriangle& out_triangle)
{
    ZenRenderTriangleHandle handle{};
    ZenVertexInputLayoutDesc input_layout = make_triangle_input_layout();
    const ZenResultCode result = zen_native_backend_create_triangle_with_shaders(
        handle_,
        vertex_shader.handle_,
        pixel_shader.handle_,
        &input_layout,
        &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_triangle = RenderTriangle(handle_, handle);
    return Result();
}

Result NativeBackend::set_camera_matrix(const Mat4& camera_matrix)
{
    const ZenMatrix4x4 native_camera_matrix = to_native_matrix(camera_matrix);
    return Result(zen_native_backend_set_camera_matrix(handle_, &native_camera_matrix));
}

Result NativeBackend::draw_triangle(const RenderTriangle& triangle)
{
    return Result(zen_native_backend_draw_triangle(handle_, triangle.handle_));
}

Result NativeBackend::draw_triangle(const RenderTriangle& triangle, const Mat4& model_matrix)
{
    const ZenMatrix4x4 native_model_matrix = to_native_matrix(model_matrix);
    return Result(zen_native_backend_draw_triangle_transformed(handle_, triangle.handle_, &native_model_matrix));
}

Result NativeBackend::draw_triangle(const RenderTriangle& triangle, const Transform& transform)
{
    return draw_triangle(triangle, transform.matrix());
}

Result NativeBackend::draw_sprite(const Texture& texture, const SpriteDrawDesc& desc)
{
    const ZenSpriteDrawDesc native_desc = make_native_sprite_draw_desc(desc);
    return Result(zen_native_backend_draw_sprite(handle_, texture.handle_, &native_desc));
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

VertexShader::VertexShader(ZenNativeBackendHandle backend, ZenVertexShaderHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

VertexShader::~VertexShader()
{
    reset();
}

VertexShader::VertexShader(VertexShader&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenVertexShaderHandle{}))
{
}

VertexShader& VertexShader::operator=(VertexShader&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenVertexShaderHandle{});
    }

    return *this;
}

void VertexShader::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_vertex_shader(backend_, handle_);
    backend_ = {};
    handle_ = {};
}

PixelShader::PixelShader(ZenNativeBackendHandle backend, ZenPixelShaderHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

PixelShader::~PixelShader()
{
    reset();
}

PixelShader::PixelShader(PixelShader&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenPixelShaderHandle{}))
{
}

PixelShader& PixelShader::operator=(PixelShader&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenPixelShaderHandle{});
    }

    return *this;
}

void PixelShader::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_pixel_shader(backend_, handle_);
    backend_ = {};
    handle_ = {};
}

Texture::Texture(ZenNativeBackendHandle backend, ZenTextureHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

Texture::~Texture()
{
    reset();
}

Texture::Texture(Texture&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenTextureHandle{}))
{
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenTextureHandle{});
    }

    return *this;
}

void Texture::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_texture(backend_, handle_);
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

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

ZenDebugLineDesc make_native_debug_line_desc(const Vec3& start, const Vec3& end, const Color& color)
{
    ZenDebugLineDesc desc{};
    desc.size = ZEN_DEBUG_LINE_DESC_SIZE;
    desc.api_version = ZEN_DEBUG_LINE_DESC_API_VERSION;
    desc.start[0] = start.x;
    desc.start[1] = start.y;
    desc.start[2] = start.z;
    desc.end[0] = end.x;
    desc.end[1] = end.y;
    desc.end[2] = end.z;
    desc.color[0] = color.r;
    desc.color[1] = color.g;
    desc.color[2] = color.b;
    desc.color[3] = color.a;
    return desc;
}

ZenDebugRectDesc make_native_debug_rect_desc(const Aabb2& bounds, float z, const Color& color)
{
    ZenDebugRectDesc desc{};
    desc.size = ZEN_DEBUG_RECT_DESC_SIZE;
    desc.api_version = ZEN_DEBUG_RECT_DESC_API_VERSION;
    desc.center[0] = bounds.center.x;
    desc.center[1] = bounds.center.y;
    desc.half_extents[0] = bounds.half_extents.x;
    desc.half_extents[1] = bounds.half_extents.y;
    desc.z = z;
    desc.color[0] = color.r;
    desc.color[1] = color.g;
    desc.color[2] = color.b;
    desc.color[3] = color.a;
    return desc;
}

ZenMeshDesc make_native_mesh_desc(
    const MeshVertex* vertices,
    std::uint32_t vertex_count,
    const std::uint32_t* indices,
    std::uint32_t index_count)
{
    ZenMeshDesc desc{};
    desc.size = ZEN_MESH_DESC_SIZE;
    desc.api_version = ZEN_MESH_DESC_API_VERSION;
    desc.vertex_stride_bytes = sizeof(MeshVertex);
    desc.vertex_data = vertices;
    desc.vertex_count = vertex_count;
    desc.index_data = indices;
    desc.index_count = index_count;
    return desc;
}

ZenMaterialDesc make_native_material_desc(const MaterialDesc& desc, ZenTextureHandle texture = {})
{
    ZenMaterialDesc native_desc{};
    native_desc.size = ZEN_MATERIAL_DESC_SIZE;
    native_desc.api_version = ZEN_MATERIAL_DESC_API_VERSION;
    native_desc.kind = static_cast<std::uint32_t>(desc.kind);
    native_desc.blend_mode = static_cast<std::uint32_t>(desc.blend_mode);
    native_desc.depth_mode = static_cast<std::uint32_t>(desc.depth_mode);
    native_desc.cull_mode = static_cast<std::uint32_t>(desc.cull_mode);
    native_desc.texture = texture;
    return native_desc;
}

ZenAudioDesc make_native_audio_desc()
{
    ZenAudioDesc desc{};
    desc.size = ZEN_AUDIO_DESC_SIZE;
    desc.api_version = ZEN_AUDIO_DESC_API_VERSION;
    return desc;
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

Result Engine::begin_frame(EngineFrameInfo& out_frame_info)
{
    ZenEngineFrameInfo native_frame_info{};
    const Result result(zen_engine_begin_frame(handle_, &native_frame_info));
    if (!result.ok()) {
        return result;
    }

    out_frame_info.frame_index = native_frame_info.frame_index;
    out_frame_info.delta_time_seconds = native_frame_info.delta_time_seconds;
    return result;
}

Result Engine::end_frame()
{
    return Result(zen_engine_end_frame(handle_));
}

Result Engine::step_frame(EngineFrameInfo& out_frame_info)
{
    ZenEngineFrameInfo native_frame_info{};
    const Result result(zen_engine_step_frame(handle_, &native_frame_info));
    if (!result.ok()) {
        return result;
    }

    out_frame_info.frame_index = native_frame_info.frame_index;
    out_frame_info.delta_time_seconds = native_frame_info.delta_time_seconds;
    return result;
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

Result NativeBackend::create_mesh(
    const MeshVertex* vertices,
    std::uint32_t vertex_count,
    const std::uint32_t* indices,
    std::uint32_t index_count,
    Mesh& out_mesh)
{
    ZenMeshDesc desc = make_native_mesh_desc(vertices, vertex_count, indices, index_count);
    ZenMeshHandle handle{};
    const ZenResultCode result = zen_native_backend_create_mesh(handle_, &desc, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_mesh = Mesh(handle_, handle);
    return Result();
}

Result NativeBackend::create_material(const MaterialDesc& desc, Material& out_material)
{
    ZenMaterialDesc native_desc = make_native_material_desc(desc);
    ZenMaterialHandle handle{};
    const ZenResultCode result = zen_native_backend_create_material(handle_, &native_desc, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_material = Material(handle_, handle);
    return Result();
}

Result NativeBackend::create_sprite_material(
    const Texture& texture,
    const MaterialDesc& desc,
    Material& out_material)
{
    ZenMaterialDesc native_desc = make_native_material_desc(desc, texture.handle_);
    ZenMaterialHandle handle{};
    const ZenResultCode result = zen_native_backend_create_material(handle_, &native_desc, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_material = Material(handle_, handle);
    return Result();
}

Result NativeBackend::create_audio_engine(AudioEngine& out_audio)
{
    ZenAudioDesc desc = make_native_audio_desc();
    ZenAudioEngineHandle handle{};
    const ZenResultCode result = zen_native_backend_create_audio_engine(handle_, &desc, &handle);
    if (result != ZEN_RESULT_OK) {
        return Result(result);
    }

    out_audio = AudioEngine(handle_, handle);
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

Result NativeBackend::draw_sprite(const Material& material, const SpriteDrawDesc& desc)
{
    const ZenSpriteDrawDesc native_desc = make_native_sprite_draw_desc(desc);
    return Result(zen_native_backend_draw_sprite_with_material(handle_, material.handle_, &native_desc));
}

Result NativeBackend::draw_mesh(const Mesh& mesh, const Mat4& model_matrix)
{
    const ZenMatrix4x4 native_model_matrix = to_native_matrix(model_matrix);
    return Result(zen_native_backend_draw_mesh(handle_, mesh.handle_, &native_model_matrix));
}

Result NativeBackend::draw_mesh(const Mesh& mesh, const Transform& transform)
{
    return draw_mesh(mesh, transform.matrix());
}

Result NativeBackend::draw_mesh(const Mesh& mesh, const Material& material, const Mat4& model_matrix)
{
    const ZenMatrix4x4 native_model_matrix = to_native_matrix(model_matrix);
    return Result(zen_native_backend_draw_mesh_with_material(handle_, mesh.handle_, material.handle_, &native_model_matrix));
}

Result NativeBackend::draw_mesh(const Mesh& mesh, const Material& material, const Transform& transform)
{
    return draw_mesh(mesh, material, transform.matrix());
}

Result NativeBackend::draw_debug_line(const Vec3& start, const Vec3& end, const Color& color)
{
    const ZenDebugLineDesc desc = make_native_debug_line_desc(start, end, color);
    return Result(zen_native_backend_draw_debug_line(handle_, &desc));
}

Result NativeBackend::draw_debug_rect_2d(const Aabb2& bounds, float z, const Color& color)
{
    const ZenDebugRectDesc desc = make_native_debug_rect_desc(bounds, z, color);
    return Result(zen_native_backend_draw_debug_rect(handle_, &desc));
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

Mesh::Mesh(ZenNativeBackendHandle backend, ZenMeshHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

Mesh::~Mesh()
{
    reset();
}

Mesh::Mesh(Mesh&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenMeshHandle{}))
{
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenMeshHandle{});
    }

    return *this;
}

void Mesh::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_mesh(backend_, handle_);
    backend_ = {};
    handle_ = {};
}

Material::Material(ZenNativeBackendHandle backend, ZenMaterialHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

Material::~Material()
{
    reset();
}

Material::Material(Material&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenMaterialHandle{}))
{
}

Material& Material::operator=(Material&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenMaterialHandle{});
    }

    return *this;
}

void Material::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_material(backend_, handle_);
    backend_ = {};
    handle_ = {};
}

AudioEngine::AudioEngine(ZenNativeBackendHandle backend, ZenAudioEngineHandle handle)
    : backend_(backend)
    , handle_(handle)
{
}

AudioEngine::~AudioEngine()
{
    reset();
}

AudioEngine::AudioEngine(AudioEngine&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , handle_(std::exchange(other.handle_, ZenAudioEngineHandle{}))
{
}

AudioEngine& AudioEngine::operator=(AudioEngine&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        handle_ = std::exchange(other.handle_, ZenAudioEngineHandle{});
    }

    return *this;
}

Result AudioEngine::load_sound(const AssetRoot& assets, std::string_view relative_path_utf8, Sound& out_sound) const
{
    std::vector<std::uint8_t> bytes;
    Result result = assets.read_binary(relative_path_utf8, bytes);
    if (!result.ok()) {
        return result;
    }

    ZenSoundHandle handle{};
    const ZenResultCode native_result = zen_native_backend_create_sound_from_wav_memory(
        backend_,
        handle_,
        bytes.data(),
        bytes.size(),
        &handle);
    if (native_result != ZEN_RESULT_OK) {
        return Result(native_result);
    }

    out_sound = Sound(backend_, handle_, handle);
    return Result();
}

void AudioEngine::reset()
{
    if (backend_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_audio_engine(backend_, handle_);
    backend_ = {};
    handle_ = {};
}

Sound::Sound(ZenNativeBackendHandle backend, ZenAudioEngineHandle audio, ZenSoundHandle handle)
    : backend_(backend)
    , audio_(audio)
    , handle_(handle)
{
}

Sound::~Sound()
{
    reset();
}

Sound::Sound(Sound&& other) noexcept
    : backend_(std::exchange(other.backend_, ZenNativeBackendHandle{}))
    , audio_(std::exchange(other.audio_, ZenAudioEngineHandle{}))
    , handle_(std::exchange(other.handle_, ZenSoundHandle{}))
{
}

Sound& Sound::operator=(Sound&& other) noexcept
{
    if (this != &other) {
        reset();
        backend_ = std::exchange(other.backend_, ZenNativeBackendHandle{});
        audio_ = std::exchange(other.audio_, ZenAudioEngineHandle{});
        handle_ = std::exchange(other.handle_, ZenSoundHandle{});
    }

    return *this;
}

Result Sound::play() const
{
    return Result(zen_native_backend_play_sound(backend_, audio_, handle_));
}

Result Sound::stop() const
{
    return Result(zen_native_backend_stop_sound(backend_, audio_, handle_));
}

Result Sound::set_volume(float volume) const
{
    return Result(zen_native_backend_set_sound_volume(backend_, audio_, handle_, volume));
}

void Sound::reset()
{
    if (backend_.value == 0 || audio_.value == 0 || handle_.value == 0) {
        return;
    }

    zen_native_backend_destroy_sound(backend_, audio_, handle_);
    backend_ = {};
    audio_ = {};
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

GameApp::~GameApp()
{
    reset();
}

GameApp::GameApp(GameApp&& other) noexcept
    : engine_(std::move(other.engine_))
    , backend_(std::move(other.backend_))
    , assets_(std::move(other.assets_))
    , audio_(std::move(other.audio_))
    , runtime_scene_(std::move(other.runtime_scene_))
    , project_(std::move(other.project_))
    , scene_(std::move(other.scene_))
    , context_(other.context_)
    , running_(std::exchange(other.running_, false))
    , module_initialized_(std::exchange(other.module_initialized_, false))
{
    context_.engine = &engine_;
    context_.backend = &backend_;
    context_.assets = &assets_;
    context_.audio = &audio_;
    context_.runtime_scene = &runtime_scene_;
    context_.project = &project_;
    context_.scene = &scene_;
    other.context_ = {};
}

GameApp& GameApp::operator=(GameApp&& other) noexcept
{
    if (this != &other) {
        reset();
        engine_ = std::move(other.engine_);
        backend_ = std::move(other.backend_);
        assets_ = std::move(other.assets_);
        audio_ = std::move(other.audio_);
        runtime_scene_ = std::move(other.runtime_scene_);
        project_ = std::move(other.project_);
        scene_ = std::move(other.scene_);
        context_ = other.context_;
        running_ = std::exchange(other.running_, false);
        module_initialized_ = std::exchange(other.module_initialized_, false);
        context_.engine = &engine_;
        context_.backend = &backend_;
        context_.assets = &assets_;
        context_.audio = &audio_;
        context_.runtime_scene = &runtime_scene_;
        context_.project = &project_;
        context_.scene = &scene_;
        other.context_ = {};
    }

    return *this;
}

Result GameApp::initialize(const GameAppConfig& config)
{
    reset();

    Result result = Engine::create(config.engine, engine_);
    if (!result.ok()) {
        reset();
        return result;
    }

    result = NativeBackend::create(backend_);
    if (!result.ok()) {
        reset();
        return result;
    }

    result = AssetRoot::from_executable(assets_);
    if (!result.ok()) {
        reset();
        return result;
    }

    result = load_project_config(assets_, config.project_path, project_);
    if (!result.ok()) {
        reset();
        return result;
    }

    if (project_.asset_root != ".") {
        AssetPath project_asset_root;
        result = assets_.resolve(project_.asset_root, project_asset_root);
        if (!result.ok()) {
            reset();
            return result;
        }

        result = AssetRoot::from_path(project_asset_root.path(), assets_);
        if (!result.ok()) {
            reset();
            return result;
        }
    }

    result = load_scene_description(assets_, project_.initial_scene, scene_);
    if (!result.ok()) {
        reset();
        return result;
    }

    result = backend_.create_window(WindowConfig{ project_.window_width, project_.window_height });
    if (!result.ok()) {
        reset();
        return result;
    }

    result = backend_.initialize_renderer();
    if (!result.ok()) {
        reset();
        return result;
    }

    result = backend_.create_audio_engine(audio_);
    if (!result.ok()) {
        reset();
        return result;
    }

    context_ = {};
    context_.engine = &engine_;
    context_.backend = &backend_;
    context_.assets = &assets_;
    context_.audio = &audio_;
    context_.runtime_scene = &runtime_scene_;
    context_.project = &project_;
    context_.scene = &scene_;
    running_ = true;
    return Result();
}

Result GameApp::begin_frame()
{
    EngineFrameInfo frame_info{};
    Result result = engine_.begin_frame(frame_info);
    if (!result.ok()) {
        return result;
    }

    context_.frame_index = frame_info.frame_index;
    context_.delta_time_seconds = frame_info.delta_time_seconds;

    bool window_should_close = false;
    result = backend_.poll_events(window_should_close);
    if (!result.ok()) {
        return result;
    }

    context_.should_close = window_should_close;
    if (context_.should_close) {
        return Result();
    }

    return backend_.input_snapshot(context_.input);
}

Result GameApp::end_frame()
{
    return engine_.end_frame();
}

Result GameApp::shutdown(GameModule& module)
{
    Result result;
    if (module_initialized_) {
        result = shutdown_game_module(module, context_);
        module_initialized_ = false;
    }

    reset();
    return result;
}

Result GameApp::run(GameModule module, const GameAppConfig& config)
{
    Result result = initialize(config);
    if (!result.ok()) {
        return result;
    }

    result = initialize_game_module(module, context_);
    if (!result.ok()) {
        reset();
        return result;
    }
    module_initialized_ = true;

    while (!context_.should_close) {
        result = begin_frame();
        if (!result.ok()) {
            break;
        }

        if (!context_.should_close) {
            result = run_game_module_frame(module, context_);
        }
        const Result end_result = end_frame();
        if (!end_result.ok() && result.ok()) {
            result = end_result;
        }
        if (!result.ok()) {
            break;
        }
    }

    if (context_.should_close && engine_.valid()) {
        const Result shutdown_request = engine_.request_shutdown();
        if (!shutdown_request.ok() && result.ok()) {
            result = shutdown_request;
        }
    }

    const Result shutdown_result = shutdown(module);
    if (!shutdown_result.ok()) {
        return shutdown_result;
    }

    return result;
}

void GameApp::reset()
{
    module_initialized_ = false;
    running_ = false;
    context_ = {};
    runtime_scene_.clear();
    audio_.reset();
    backend_.reset();
    engine_.reset();
    assets_ = {};
    project_ = {};
    scene_ = {};
}

} // namespace zeno

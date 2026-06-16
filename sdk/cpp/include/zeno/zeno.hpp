#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

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

class AssetPath final {
public:
    AssetPath() = default;

    const std::filesystem::path& path() const { return path_; }
    bool valid() const { return !path_.empty(); }

private:
    friend class AssetRoot;

    explicit AssetPath(std::filesystem::path path);

    std::filesystem::path path_{};
};

class AssetRoot final {
public:
    AssetRoot() = default;

    static Result from_executable(AssetRoot& out_root);
    static Result from_path(const std::filesystem::path& root, AssetRoot& out_root);

    Result resolve(std::string_view relative_path_utf8, AssetPath& out_path) const;
    Result read_text(std::string_view relative_path_utf8, std::string& out_text) const;
    Result read_binary(std::string_view relative_path_utf8, std::vector<std::uint8_t>& out_bytes) const;

    const std::filesystem::path& path() const { return root_; }
    bool valid() const { return !root_.empty(); }

private:
    explicit AssetRoot(std::filesystem::path root);

    std::filesystem::path root_{};
};

struct EngineConfig final {
    double target_fps = 60.0;
    std::uint64_t max_test_frames = ZEN_MAX_TEST_FRAMES_UNLIMITED;
};

struct EngineFrameInfo final {
    std::uint64_t frame_index = 0;
    double delta_time_seconds = 0.0;
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
    Result begin_frame(EngineFrameInfo& out_frame_info);
    Result end_frame();
    Result step_frame(EngineFrameInfo& out_frame_info);
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

struct ShaderCompileLog final {
    std::string message{};
};

struct SpriteDrawDesc final {
    Transform transform{};
    Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct MeshVertex final {
    Vec3 position{};
    Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

enum class MaterialKind : std::uint32_t {
    sprite_texture = ZEN_MATERIAL_KIND_SPRITE_TEXTURE,
    mesh_color = ZEN_MATERIAL_KIND_MESH_COLOR,
};

enum class BlendMode : std::uint32_t {
    opaque = ZEN_BLEND_MODE_OPAQUE,
    alpha = ZEN_BLEND_MODE_ALPHA,
};

enum class DepthMode : std::uint32_t {
    disabled = ZEN_DEPTH_MODE_DISABLED,
    enabled = ZEN_DEPTH_MODE_ENABLED,
};

enum class CullMode : std::uint32_t {
    none = ZEN_CULL_MODE_NONE,
    back = ZEN_CULL_MODE_BACK,
};

struct MaterialDesc final {
    MaterialKind kind = MaterialKind::mesh_color;
    BlendMode blend_mode = BlendMode::opaque;
    DepthMode depth_mode = DepthMode::enabled;
    CullMode cull_mode = CullMode::back;
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
class VertexShader;
class PixelShader;
class Texture;
class Mesh;
class Material;
class AudioEngine;
class Sound;

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
    Result create_vertex_shader(
        const AssetRoot& assets,
        std::string_view relative_path_utf8,
        std::string_view entry,
        VertexShader& out_shader,
        ShaderCompileLog& out_log);
    Result create_pixel_shader(
        const AssetRoot& assets,
        std::string_view relative_path_utf8,
        std::string_view entry,
        PixelShader& out_shader,
        ShaderCompileLog& out_log);
    Result create_texture(
        const AssetRoot& assets,
        std::string_view relative_path_utf8,
        Texture& out_texture);
    Result create_mesh(
        const MeshVertex* vertices,
        std::uint32_t vertex_count,
        const std::uint32_t* indices,
        std::uint32_t index_count,
        Mesh& out_mesh);
    Result create_material(const MaterialDesc& desc, Material& out_material);
    Result create_sprite_material(
        const Texture& texture,
        const MaterialDesc& desc,
        Material& out_material);
    Result create_audio_engine(AudioEngine& out_audio);
    Result create_triangle(RenderTriangle& out_triangle);
    Result create_triangle(const VertexShader& vertex_shader, const PixelShader& pixel_shader, RenderTriangle& out_triangle);
    Result set_camera_matrix(const Mat4& camera_matrix);
    Result draw_triangle(const RenderTriangle& triangle);
    Result draw_triangle(const RenderTriangle& triangle, const Mat4& model_matrix);
    Result draw_triangle(const RenderTriangle& triangle, const Transform& transform);
    Result draw_sprite(const Texture& texture, const SpriteDrawDesc& desc);
    Result draw_sprite(const Material& material, const SpriteDrawDesc& desc);
    Result draw_mesh(const Mesh& mesh, const Mat4& model_matrix);
    Result draw_mesh(const Mesh& mesh, const Transform& transform);
    Result draw_mesh(const Mesh& mesh, const Material& material, const Mat4& model_matrix);
    Result draw_mesh(const Mesh& mesh, const Material& material, const Transform& transform);
    Result draw_debug_line(const Vec3& start, const Vec3& end, const Color& color);
    Result draw_debug_rect_2d(const Aabb2& bounds, float z, const Color& color);
    Result present();
    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    explicit NativeBackend(ZenNativeBackendHandle handle);

    ZenNativeBackendHandle handle_{};
};

struct ObjectId final {
    std::uint64_t value = 0;

    constexpr bool valid() const { return value != 0; }
};

constexpr bool operator==(ObjectId left, ObjectId right)
{
    return left.value == right.value;
}

constexpr bool operator!=(ObjectId left, ObjectId right)
{
    return !(left == right);
}

enum class RenderableKind : std::uint32_t {
    none = 0,
    sprite = 1,
    mesh = 2,
    triangle = 3,
};

struct SpriteRenderer final {
    const Material* material = nullptr;
    Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct MeshRenderer final {
    const Mesh* mesh = nullptr;
    const Material* material = nullptr;
};

struct TriangleRenderer final {
    const RenderTriangle* triangle = nullptr;
};

class Scene final {
public:
    Scene() = default;

    ObjectId create_object();
    Result destroy_object(ObjectId id);
    void clear();

    bool has_object(ObjectId id) const;
    Result set_transform(ObjectId id, const Transform& transform);
    Transform* transform(ObjectId id);
    const Transform* transform(ObjectId id) const;

    Result set_sprite_renderer(ObjectId id, const SpriteRenderer& renderer);
    Result set_mesh_renderer(ObjectId id, const MeshRenderer& renderer);
    Result set_triangle_renderer(ObjectId id, const TriangleRenderer& renderer);
    Result clear_renderer(ObjectId id);

    RenderableKind renderable_kind(ObjectId id) const;
    std::vector<ObjectId> objects_in_update_order() const;
    std::vector<ObjectId> objects_in_render_order() const;
    Result render(NativeBackend& backend) const;

private:
    struct ObjectState final {
        ObjectId id{};
        bool alive = false;
        Transform transform{};
        RenderableKind renderable_kind = RenderableKind::none;
        SpriteRenderer sprite_renderer{};
        MeshRenderer mesh_renderer{};
        TriangleRenderer triangle_renderer{};
    };

    ObjectState* find_object(ObjectId id);
    const ObjectState* find_object(ObjectId id) const;

    std::uint64_t next_object_id_ = 1;
    std::vector<ObjectState> objects_{};
};

struct ProjectConfig final {
    std::uint32_t version = 1;
    std::uint32_t window_width = 640;
    std::uint32_t window_height = 360;
    std::string asset_root = ".";
    std::string initial_scene = "scenes/sample_scene.zscene";
};

struct SceneObjectDesc final {
    std::string name{};
    RenderableKind renderable_kind = RenderableKind::none;
    Transform transform{};
    Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::string reference{};
};

struct SceneDescription final {
    std::uint32_t version = 1;
    std::vector<SceneObjectDesc> objects{};
};

Result load_project_config(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    ProjectConfig& out_config);
Result load_scene_description(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    SceneDescription& out_scene);
Result serialize_project_config(const ProjectConfig& config, std::string& out_text);
Result serialize_scene_description(const SceneDescription& scene, std::string& out_text);

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

class VertexShader final {
public:
    VertexShader() = default;
    ~VertexShader();

    VertexShader(const VertexShader&) = delete;
    VertexShader& operator=(const VertexShader&) = delete;

    VertexShader(VertexShader&& other) noexcept;
    VertexShader& operator=(VertexShader&& other) noexcept;

    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;

    VertexShader(ZenNativeBackendHandle backend, ZenVertexShaderHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenVertexShaderHandle handle_{};
};

class PixelShader final {
public:
    PixelShader() = default;
    ~PixelShader();

    PixelShader(const PixelShader&) = delete;
    PixelShader& operator=(const PixelShader&) = delete;

    PixelShader(PixelShader&& other) noexcept;
    PixelShader& operator=(PixelShader&& other) noexcept;

    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;

    PixelShader(ZenNativeBackendHandle backend, ZenPixelShaderHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenPixelShaderHandle handle_{};
};

class Texture final {
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;

    Texture(ZenNativeBackendHandle backend, ZenTextureHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenTextureHandle handle_{};
};

class Mesh final {
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;

    Mesh(ZenNativeBackendHandle backend, ZenMeshHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenMeshHandle handle_{};
};

class Material final {
public:
    Material() = default;
    ~Material();

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    Material(Material&& other) noexcept;
    Material& operator=(Material&& other) noexcept;

    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;

    Material(ZenNativeBackendHandle backend, ZenMaterialHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenMaterialHandle handle_{};
};

class AudioEngine final {
public:
    AudioEngine() = default;
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    AudioEngine(AudioEngine&& other) noexcept;
    AudioEngine& operator=(AudioEngine&& other) noexcept;

    Result load_sound(const AssetRoot& assets, std::string_view relative_path_utf8, Sound& out_sound) const;
    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class NativeBackend;
    friend class Sound;

    AudioEngine(ZenNativeBackendHandle backend, ZenAudioEngineHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenAudioEngineHandle handle_{};
};

class Sound final {
public:
    Sound() = default;
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    Sound(Sound&& other) noexcept;
    Sound& operator=(Sound&& other) noexcept;

    Result play() const;
    Result stop() const;
    Result set_volume(float volume) const;
    void reset();

    bool valid() const { return handle_.value != 0; }

private:
    friend class AudioEngine;

    Sound(ZenNativeBackendHandle backend, ZenAudioEngineHandle audio, ZenSoundHandle handle);

    ZenNativeBackendHandle backend_{};
    ZenAudioEngineHandle audio_{};
    ZenSoundHandle handle_{};
};

} // namespace zeno

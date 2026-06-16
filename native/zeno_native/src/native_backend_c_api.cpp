#include <zeno/zeno_native_backend.h>

#include "native_backend.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace {

constexpr std::uint32_t kConfigApiVersion = 1;
constexpr std::uint32_t kConfigSize = sizeof(ZenNativeBackendConfig);
constexpr std::uint32_t kWindowConfigApiVersion = 1;
constexpr std::uint32_t kWindowConfigSize = sizeof(ZenNativeWindowConfig);
constexpr std::uint32_t kInputSnapshotApiVersion = 1;
constexpr std::uint32_t kInputSnapshotSize = sizeof(ZenInputSnapshot);
constexpr std::uint32_t kShaderCompileLogApiVersion = 1;
constexpr std::uint32_t kShaderCompileLogSize = sizeof(ZenShaderCompileLog);
constexpr std::uint32_t kVertexInputLayoutDescApiVersion = 1;
constexpr std::uint32_t kVertexInputLayoutDescSize = sizeof(ZenVertexInputLayoutDesc);
constexpr std::uint32_t kSpriteDrawDescApiVersion = 1;
constexpr std::uint32_t kSpriteDrawDescSize = sizeof(ZenSpriteDrawDesc);
constexpr std::uint32_t kMeshDescApiVersion = 1;
constexpr std::uint32_t kMeshDescSize = sizeof(ZenMeshDesc);
constexpr std::uint32_t kMaterialDescApiVersion = 1;
constexpr std::uint32_t kMaterialDescSize = sizeof(ZenMaterialDesc);
constexpr std::uint32_t kAudioDescApiVersion = 1;
constexpr std::uint32_t kAudioDescSize = sizeof(ZenAudioDesc);
constexpr std::uint64_t kInvalidHandle = 0;

static_assert(sizeof(ZenMatrix4x4) == sizeof(zeno::native::Matrix4x4));

std::mutex g_backend_mutex;
std::unordered_map<std::uint64_t, std::shared_ptr<zeno::native::NativeBackend>> g_backends;
std::uint64_t g_next_backend_handle = 1;

bool is_valid_config(const ZenNativeBackendConfig& config)
{
    return config.size == kConfigSize
        && config.api_version == kConfigApiVersion
        && config.flags == 0
        && config.reserved == 0;
}

bool is_valid_window_config(const ZenNativeWindowConfig& config)
{
    return config.size == kWindowConfigSize
        && config.api_version == kWindowConfigApiVersion
        && config.width > 0
        && config.height > 0;
}

bool is_valid_input_snapshot_header(const ZenInputSnapshot& snapshot)
{
    return snapshot.size == kInputSnapshotSize
        && snapshot.api_version == kInputSnapshotApiVersion;
}

bool is_valid_shader_compile_log_header(const ZenShaderCompileLog& log)
{
    return log.size == kShaderCompileLogSize
        && log.api_version == kShaderCompileLogApiVersion;
}

bool is_valid_vertex_input_layout_desc(const ZenVertexInputLayoutDesc& desc)
{
    return desc.size == kVertexInputLayoutDescSize
        && desc.api_version == kVertexInputLayoutDescApiVersion
        && desc.element_count > 0
        && desc.element_count <= ZEN_VERTEX_INPUT_LAYOUT_MAX_ELEMENTS;
}

bool is_valid_sprite_draw_desc(const ZenSpriteDrawDesc& desc)
{
    return desc.size == kSpriteDrawDescSize
        && desc.api_version == kSpriteDrawDescApiVersion;
}

bool is_valid_mesh_desc(const ZenMeshDesc& desc)
{
    if (desc.size != kMeshDescSize
        || desc.api_version != kMeshDescApiVersion
        || desc.vertex_data == nullptr
        || desc.index_data == nullptr
        || desc.vertex_count == 0
        || desc.index_count == 0
        || desc.vertex_stride_bytes < sizeof(float) * 7
        || desc.vertex_stride_bytes % sizeof(float) != 0) {
        return false;
    }

    return desc.vertex_count <= UINT64_MAX / desc.vertex_stride_bytes
        && desc.index_count <= UINT64_MAX / sizeof(std::uint32_t);
}

bool is_valid_material_kind(std::uint32_t kind)
{
    return kind == ZEN_MATERIAL_KIND_SPRITE_TEXTURE
        || kind == ZEN_MATERIAL_KIND_MESH_COLOR;
}

bool is_valid_blend_mode(std::uint32_t blend_mode)
{
    return blend_mode == ZEN_BLEND_MODE_OPAQUE
        || blend_mode == ZEN_BLEND_MODE_ALPHA;
}

bool is_valid_depth_mode(std::uint32_t depth_mode)
{
    return depth_mode == ZEN_DEPTH_MODE_DISABLED
        || depth_mode == ZEN_DEPTH_MODE_ENABLED;
}

bool is_valid_cull_mode(std::uint32_t cull_mode)
{
    return cull_mode == ZEN_CULL_MODE_NONE
        || cull_mode == ZEN_CULL_MODE_BACK;
}

bool is_valid_material_desc(const ZenMaterialDesc& desc)
{
    if (desc.size != kMaterialDescSize
        || desc.api_version != kMaterialDescApiVersion
        || !is_valid_material_kind(desc.kind)
        || !is_valid_blend_mode(desc.blend_mode)
        || !is_valid_depth_mode(desc.depth_mode)
        || !is_valid_cull_mode(desc.cull_mode)
        || desc.reserved[0] != 0
        || desc.reserved[1] != 0) {
        return false;
    }

    if (desc.kind == ZEN_MATERIAL_KIND_SPRITE_TEXTURE) {
        return desc.texture.value != 0;
    }

    return desc.texture.value == 0;
}

bool is_valid_audio_desc(const ZenAudioDesc& desc)
{
    return desc.size == kAudioDescSize
        && desc.api_version == kAudioDescApiVersion
        && desc.reserved[0] == 0
        && desc.reserved[1] == 0;
}

bool allocate_handle(std::uint64_t& out_handle)
{
    if (g_next_backend_handle == UINT64_MAX) {
        return false;
    }

    out_handle = g_next_backend_handle++;
    return out_handle != kInvalidHandle;
}

ZenResultCode map_render_command_result(zeno::native::RenderCommandResult result)
{
    switch (result) {
    case zeno::native::RenderCommandResult::ok:
        return ZEN_RESULT_OK;
    case zeno::native::RenderCommandResult::wrong_state:
        return ZEN_RESULT_BACKEND_ERROR;
    case zeno::native::RenderCommandResult::missing_resource:
        return ZEN_RESULT_NOT_INITIALIZED;
    }

    return ZEN_RESULT_INTERNAL_ERROR;
}

ZenResultCode map_audio_command_result(zeno::native::AudioCommandResult result)
{
    switch (result) {
    case zeno::native::AudioCommandResult::ok:
        return ZEN_RESULT_OK;
    case zeno::native::AudioCommandResult::invalid_argument:
        return ZEN_RESULT_INVALID_ARGUMENT;
    case zeno::native::AudioCommandResult::wrong_state:
    case zeno::native::AudioCommandResult::backend_error:
        return ZEN_RESULT_BACKEND_ERROR;
    case zeno::native::AudioCommandResult::missing_resource:
        return ZEN_RESULT_NOT_INITIALIZED;
    }

    return ZEN_RESULT_INTERNAL_ERROR;
}

zeno::native::Matrix4x4 to_native_matrix(const ZenMatrix4x4& matrix)
{
    zeno::native::Matrix4x4 native_matrix{};
    for (std::uint32_t i = 0; i < 16; ++i) {
        native_matrix.elements[i] = matrix.elements[i];
    }

    return native_matrix;
}

zeno::native::VertexInputLayoutDesc to_native_input_layout(const ZenVertexInputLayoutDesc& desc)
{
    zeno::native::VertexInputLayoutDesc native_desc{};
    native_desc.element_count = desc.element_count;
    for (std::uint32_t i = 0; i < desc.element_count; ++i) {
        native_desc.elements[i].semantic = desc.elements[i].semantic;
        native_desc.elements[i].semantic_index = desc.elements[i].semantic_index;
        native_desc.elements[i].format = desc.elements[i].format;
        native_desc.elements[i].input_slot = desc.elements[i].input_slot;
        native_desc.elements[i].aligned_byte_offset = desc.elements[i].aligned_byte_offset;
    }

    return native_desc;
}

zeno::native::SpriteDrawDesc to_native_sprite_draw_desc(const ZenSpriteDrawDesc& desc)
{
    zeno::native::SpriteDrawDesc native_desc{};
    native_desc.model_matrix = to_native_matrix(desc.model_matrix);
    for (std::uint32_t i = 0; i < 4; ++i) {
        native_desc.color[i] = desc.color[i];
    }

    return native_desc;
}

zeno::native::MeshDesc to_native_mesh_desc(const ZenMeshDesc& desc)
{
    zeno::native::MeshDesc native_desc{};
    native_desc.vertex_data = desc.vertex_data;
    native_desc.vertex_count = desc.vertex_count;
    native_desc.vertex_stride_bytes = desc.vertex_stride_bytes;
    native_desc.index_data = desc.index_data;
    native_desc.index_count = desc.index_count;
    return native_desc;
}

zeno::native::MaterialDesc to_native_material_desc(const ZenMaterialDesc& desc)
{
    zeno::native::MaterialDesc native_desc{};
    native_desc.kind = desc.kind;
    native_desc.blend_mode = desc.blend_mode;
    native_desc.depth_mode = desc.depth_mode;
    native_desc.cull_mode = desc.cull_mode;
    native_desc.texture = desc.texture.value;
    return native_desc;
}

void copy_compile_log(const zeno::native::ShaderCompileLog& native_log, ZenShaderCompileLog& out_log)
{
    out_log.message_length = native_log.message_length;
    for (std::uint32_t i = 0; i < ZEN_SHADER_COMPILE_LOG_MESSAGE_CAPACITY; ++i) {
        out_log.message[i] = native_log.message[i];
    }
}

bool is_valid_borrowed_bytes(const char* data, std::uint64_t length)
{
    return data != nullptr && length > 0;
}

bool is_valid_borrowed_bytes(const std::uint8_t* data, std::uint64_t length)
{
    return data != nullptr && length > 0;
}

ZenResultCode with_backend(
    ZenNativeBackendHandle backend,
    const auto& action)
{
    if (backend.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    std::shared_ptr<zeno::native::NativeBackend> native_backend;
    {
        std::lock_guard<std::mutex> lock(g_backend_mutex);
        auto found = g_backends.find(backend.value);
        if (found == g_backends.end()) {
            return ZEN_RESULT_NOT_INITIALIZED;
        }

        native_backend = found->second;
    }

    return action(*native_backend);
}

} // namespace

extern "C" ZenResultCode zen_native_backend_create(
    const ZenNativeBackendConfig* config,
    ZenNativeBackendHandle* out_backend)
{
    try {
        if (config == nullptr || out_backend == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_config(*config)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        auto backend = std::make_shared<zeno::native::NativeBackend>();
        zeno::native::NativeBackendConfig native_config{};
        native_config.flags = config->flags;

        if (!backend->initialize(native_config)) {
            return ZEN_RESULT_INTERNAL_ERROR;
        }

        std::lock_guard<std::mutex> lock(g_backend_mutex);
        std::uint64_t handle = kInvalidHandle;
        if (!allocate_handle(handle)) {
            backend->shutdown();
            return ZEN_RESULT_INTERNAL_ERROR;
        }

        g_backends.emplace(handle, std::move(backend));
        out_backend->value = handle;
        return ZEN_RESULT_OK;
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy(ZenNativeBackendHandle backend)
{
    try {
        if (backend.value == kInvalidHandle) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        std::shared_ptr<zeno::native::NativeBackend> owned_backend;
        {
            std::lock_guard<std::mutex> lock(g_backend_mutex);
            auto found = g_backends.find(backend.value);
            if (found == g_backends.end()) {
                return ZEN_RESULT_NOT_INITIALIZED;
            }

            owned_backend = std::move(found->second);
            g_backends.erase(found);
        }

        owned_backend->shutdown();
        return ZEN_RESULT_OK;
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_window(
    ZenNativeBackendHandle backend,
    const ZenNativeWindowConfig* config)
{
    try {
        if (config == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_window_config(*config)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [config](zeno::native::NativeBackend& native_backend) {
            zeno::native::NativeWindowConfig native_config{};
            native_config.width = config->width;
            native_config.height = config->height;

            return native_backend.create_window(native_config)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_INTERNAL_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_poll_events(
    ZenNativeBackendHandle backend,
    std::uint32_t* out_should_close)
{
    try {
        if (out_should_close == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [out_should_close](zeno::native::NativeBackend& native_backend) {
            bool should_close = false;
            if (!native_backend.poll_events(should_close)) {
                return ZEN_RESULT_INTERNAL_ERROR;
            }

            *out_should_close = should_close ? 1u : 0u;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_get_input_snapshot(
    ZenNativeBackendHandle backend,
    ZenInputSnapshot* out_snapshot)
{
    try {
        if (out_snapshot == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_input_snapshot_header(*out_snapshot)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [out_snapshot](zeno::native::NativeBackend& native_backend) {
            zeno::native::InputSnapshot native_snapshot{};
            if (!native_backend.get_input_snapshot(native_snapshot)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            ZenInputSnapshot snapshot{};
            snapshot.size = ZEN_INPUT_SNAPSHOT_SIZE;
            snapshot.api_version = ZEN_INPUT_SNAPSHOT_API_VERSION;
            snapshot.mouse_x = native_snapshot.mouse_x;
            snapshot.mouse_y = native_snapshot.mouse_y;
            snapshot.mouse_wheel_delta = native_snapshot.mouse_wheel_delta;
            snapshot.reserved = 0;

            for (std::uint32_t i = 0; i < ZEN_INPUT_KEY_COUNT; ++i) {
                snapshot.key_down[i] = native_snapshot.key_down[i] ? 1u : 0u;
                snapshot.key_pressed[i] = native_snapshot.key_pressed[i] ? 1u : 0u;
                snapshot.key_released[i] = native_snapshot.key_released[i] ? 1u : 0u;
            }

            for (std::uint32_t i = 0; i < ZEN_INPUT_MOUSE_BUTTON_COUNT; ++i) {
                snapshot.mouse_down[i] = native_snapshot.mouse_down[i] ? 1u : 0u;
                snapshot.mouse_pressed[i] = native_snapshot.mouse_pressed[i] ? 1u : 0u;
                snapshot.mouse_released[i] = native_snapshot.mouse_released[i] ? 1u : 0u;
            }

            *out_snapshot = snapshot;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_initialize_renderer(ZenNativeBackendHandle backend)
{
    try {
        return with_backend(backend, [](zeno::native::NativeBackend& native_backend) {
            return native_backend.initialize_renderer()
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_begin_frame(ZenNativeBackendHandle backend)
{
    try {
        return with_backend(backend, [](zeno::native::NativeBackend& native_backend) {
            return native_backend.begin_frame()
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_clear(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a)
{
    try {
        return with_backend(backend, [r, g, b, a](zeno::native::NativeBackend& native_backend) {
            return native_backend.clear(r, g, b, a)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_clear_color(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a,
    ZenRenderClearColorHandle* out_clear_color)
{
    try {
        if (out_clear_color == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [r, g, b, a, out_clear_color](zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_clear_color(r, g, b, a, handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_clear_color->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_clear_color(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color)
{
    try {
        if (clear_color.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [clear_color](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_clear_color(clear_color.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_clear_with_resource(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color)
{
    try {
        if (clear_color.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [clear_color](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(native_backend.clear_with_resource(clear_color.value));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle* out_triangle)
{
    try {
        if (out_triangle == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [out_triangle](zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_triangle(handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_triangle->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_triangle_with_shaders(
    ZenNativeBackendHandle backend,
    ZenVertexShaderHandle vertex_shader,
    ZenPixelShaderHandle pixel_shader,
    const ZenVertexInputLayoutDesc* input_layout,
    ZenRenderTriangleHandle* out_triangle)
{
    try {
        if (vertex_shader.value == 0 || pixel_shader.value == 0 || input_layout == nullptr || out_triangle == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_vertex_input_layout_desc(*input_layout)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [vertex_shader, pixel_shader, input_layout, out_triangle](
            zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_triangle_with_shaders(
                    vertex_shader.value,
                    pixel_shader.value,
                    to_native_input_layout(*input_layout),
                    handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_triangle->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle)
{
    try {
        if (triangle.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [triangle](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_triangle(triangle.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_vertex_shader_from_source(
    ZenNativeBackendHandle backend,
    const char* source_utf8,
    std::uint64_t source_byte_count,
    const char* entry_utf8,
    std::uint64_t entry_byte_count,
    const char* profile_utf8,
    std::uint64_t profile_byte_count,
    ZenShaderCompileLog* compile_log,
    ZenVertexShaderHandle* out_shader)
{
    try {
        if (!is_valid_borrowed_bytes(source_utf8, source_byte_count)
            || !is_valid_borrowed_bytes(entry_utf8, entry_byte_count)
            || !is_valid_borrowed_bytes(profile_utf8, profile_byte_count)
            || compile_log == nullptr
            || out_shader == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_shader_compile_log_header(*compile_log)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        compile_log->message_length = 0;
        compile_log->message[0] = '\0';
        return with_backend(backend, [=](zeno::native::NativeBackend& native_backend) {
            zeno::native::ShaderCompileLog native_log{};
            std::uint64_t handle = 0;
            if (!native_backend.create_vertex_shader_from_source(
                    source_utf8,
                    source_byte_count,
                    entry_utf8,
                    entry_byte_count,
                    profile_utf8,
                    profile_byte_count,
                    native_log,
                    handle)) {
                copy_compile_log(native_log, *compile_log);
                return ZEN_RESULT_BACKEND_ERROR;
            }

            copy_compile_log(native_log, *compile_log);
            out_shader->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_pixel_shader_from_source(
    ZenNativeBackendHandle backend,
    const char* source_utf8,
    std::uint64_t source_byte_count,
    const char* entry_utf8,
    std::uint64_t entry_byte_count,
    const char* profile_utf8,
    std::uint64_t profile_byte_count,
    ZenShaderCompileLog* compile_log,
    ZenPixelShaderHandle* out_shader)
{
    try {
        if (!is_valid_borrowed_bytes(source_utf8, source_byte_count)
            || !is_valid_borrowed_bytes(entry_utf8, entry_byte_count)
            || !is_valid_borrowed_bytes(profile_utf8, profile_byte_count)
            || compile_log == nullptr
            || out_shader == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_shader_compile_log_header(*compile_log)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        compile_log->message_length = 0;
        compile_log->message[0] = '\0';
        return with_backend(backend, [=](zeno::native::NativeBackend& native_backend) {
            zeno::native::ShaderCompileLog native_log{};
            std::uint64_t handle = 0;
            if (!native_backend.create_pixel_shader_from_source(
                    source_utf8,
                    source_byte_count,
                    entry_utf8,
                    entry_byte_count,
                    profile_utf8,
                    profile_byte_count,
                    native_log,
                    handle)) {
                copy_compile_log(native_log, *compile_log);
                return ZEN_RESULT_BACKEND_ERROR;
            }

            copy_compile_log(native_log, *compile_log);
            out_shader->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_vertex_shader(
    ZenNativeBackendHandle backend,
    ZenVertexShaderHandle shader)
{
    try {
        if (shader.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [shader](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_vertex_shader(shader.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_pixel_shader(
    ZenNativeBackendHandle backend,
    ZenPixelShaderHandle shader)
{
    try {
        if (shader.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [shader](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_pixel_shader(shader.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_texture_from_memory(
    ZenNativeBackendHandle backend,
    const std::uint8_t* image_bytes,
    std::uint64_t image_byte_count,
    ZenTextureHandle* out_texture)
{
    try {
        if (!is_valid_borrowed_bytes(image_bytes, image_byte_count) || out_texture == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [image_bytes, image_byte_count, out_texture](
            zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_texture_from_memory(image_bytes, image_byte_count, handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_texture->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_texture(
    ZenNativeBackendHandle backend,
    ZenTextureHandle texture)
{
    try {
        if (texture.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [texture](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_texture(texture.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_sprite(
    ZenNativeBackendHandle backend,
    ZenTextureHandle texture,
    const ZenSpriteDrawDesc* desc)
{
    try {
        if (texture.value == 0 || desc == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_sprite_draw_desc(*desc)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [texture, desc](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(
                native_backend.draw_sprite(texture.value, to_native_sprite_draw_desc(*desc)));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_material(
    ZenNativeBackendHandle backend,
    const ZenMaterialDesc* desc,
    ZenMaterialHandle* out_material)
{
    try {
        if (desc == nullptr || out_material == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_material_desc(*desc)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [desc, out_material](zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            const ZenResultCode result = map_render_command_result(
                native_backend.create_material(to_native_material_desc(*desc), handle));
            if (result != ZEN_RESULT_OK) {
                return result;
            }

            out_material->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_material(
    ZenNativeBackendHandle backend,
    ZenMaterialHandle material)
{
    try {
        if (material.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [material](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_material(material.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_sprite_with_material(
    ZenNativeBackendHandle backend,
    ZenMaterialHandle material,
    const ZenSpriteDrawDesc* desc)
{
    try {
        if (material.value == 0 || desc == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_sprite_draw_desc(*desc)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [material, desc](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(
                native_backend.draw_sprite_with_material(material.value, to_native_sprite_draw_desc(*desc)));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_mesh(
    ZenNativeBackendHandle backend,
    const ZenMeshDesc* desc,
    ZenMeshHandle* out_mesh)
{
    try {
        if (desc == nullptr || out_mesh == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        if (!is_valid_mesh_desc(*desc)) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [desc, out_mesh](zeno::native::NativeBackend& native_backend) {
            std::uint64_t handle = 0;
            if (!native_backend.create_mesh(to_native_mesh_desc(*desc), handle)) {
                return ZEN_RESULT_BACKEND_ERROR;
            }

            out_mesh->value = handle;
            return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_mesh(
    ZenNativeBackendHandle backend,
    ZenMeshHandle mesh)
{
    try {
        if (mesh.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [mesh](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_mesh(mesh.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_mesh(
    ZenNativeBackendHandle backend,
    ZenMeshHandle mesh,
    const ZenMatrix4x4* model_matrix)
{
    try {
        if (mesh.value == 0 || model_matrix == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [mesh, model_matrix](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(
                native_backend.draw_mesh(mesh.value, to_native_matrix(*model_matrix)));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_mesh_with_material(
    ZenNativeBackendHandle backend,
    ZenMeshHandle mesh,
    ZenMaterialHandle material,
    const ZenMatrix4x4* model_matrix)
{
    try {
        if (mesh.value == 0 || material.value == 0 || model_matrix == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [mesh, material, model_matrix](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(native_backend.draw_mesh_with_material(
                mesh.value,
                material.value,
                to_native_matrix(*model_matrix)));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_audio_engine(
    ZenNativeBackendHandle backend,
    const ZenAudioDesc* desc,
    ZenAudioEngineHandle* out_audio)
{
    if (backend.value == kInvalidHandle || desc == nullptr || out_audio == nullptr || !is_valid_audio_desc(*desc)) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [out_audio](zeno::native::NativeBackend& native_backend) {
        std::uint64_t handle = 0;
        const ZenResultCode result = map_audio_command_result(native_backend.create_audio_engine(handle));
        if (result != ZEN_RESULT_OK) {
            return result;
        }

        out_audio->value = handle;
        return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_audio_engine(
    ZenNativeBackendHandle backend,
    ZenAudioEngineHandle audio)
{
    if (backend.value == kInvalidHandle || audio.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [audio](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_audio_engine(audio.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_create_sound_from_wav_memory(
    ZenNativeBackendHandle backend,
    ZenAudioEngineHandle audio,
    const uint8_t* wav_bytes,
    uint64_t wav_byte_count,
    ZenSoundHandle* out_sound)
{
    if (backend.value == kInvalidHandle
        || audio.value == kInvalidHandle
        || wav_bytes == nullptr
        || wav_byte_count == 0
        || out_sound == nullptr) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [audio, wav_bytes, wav_byte_count, out_sound](zeno::native::NativeBackend& native_backend) {
        std::uint64_t handle = 0;
        const ZenResultCode result = map_audio_command_result(
            native_backend.create_sound_from_wav_memory(audio.value, wav_bytes, wav_byte_count, handle));
        if (result != ZEN_RESULT_OK) {
            return result;
        }

        out_sound->value = handle;
        return ZEN_RESULT_OK;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_destroy_sound(
    ZenNativeBackendHandle backend,
    ZenAudioEngineHandle audio,
    ZenSoundHandle sound)
{
    if (backend.value == kInvalidHandle || audio.value == kInvalidHandle || sound.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [audio, sound](zeno::native::NativeBackend& native_backend) {
            return native_backend.destroy_sound(audio.value, sound.value)
                ? ZEN_RESULT_OK
                : ZEN_RESULT_NOT_INITIALIZED;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_play_sound(
    ZenNativeBackendHandle backend,
    ZenAudioEngineHandle audio,
    ZenSoundHandle sound)
{
    if (backend.value == kInvalidHandle || audio.value == kInvalidHandle || sound.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [audio, sound](zeno::native::NativeBackend& native_backend) {
            return map_audio_command_result(native_backend.play_sound(audio.value, sound.value));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_stop_sound(
    ZenNativeBackendHandle backend,
    ZenAudioEngineHandle audio,
    ZenSoundHandle sound)
{
    if (backend.value == kInvalidHandle || audio.value == kInvalidHandle || sound.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [audio, sound](zeno::native::NativeBackend& native_backend) {
            return map_audio_command_result(native_backend.stop_sound(audio.value, sound.value));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_set_sound_volume(
    ZenNativeBackendHandle backend,
    ZenAudioEngineHandle audio,
    ZenSoundHandle sound,
    float volume)
{
    if (backend.value == kInvalidHandle || audio.value == kInvalidHandle || sound.value == kInvalidHandle) {
        return ZEN_RESULT_INVALID_ARGUMENT;
    }

    try {
        return with_backend(backend, [audio, sound, volume](zeno::native::NativeBackend& native_backend) {
            return map_audio_command_result(native_backend.set_sound_volume(audio.value, sound.value, volume));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle)
{
    try {
        if (triangle.value == 0) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [triangle](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(native_backend.draw_triangle(triangle.value));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_set_camera_matrix(
    ZenNativeBackendHandle backend,
    const ZenMatrix4x4* camera_matrix)
{
    try {
        if (camera_matrix == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [camera_matrix](zeno::native::NativeBackend& native_backend) {
            return native_backend.set_camera_matrix(to_native_matrix(*camera_matrix))
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_draw_triangle_transformed(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle,
    const ZenMatrix4x4* model_matrix)
{
    try {
        if (triangle.value == 0 || model_matrix == nullptr) {
            return ZEN_RESULT_INVALID_ARGUMENT;
        }

        return with_backend(backend, [triangle, model_matrix](zeno::native::NativeBackend& native_backend) {
            return map_render_command_result(
                native_backend.draw_triangle_transformed(triangle.value, to_native_matrix(*model_matrix)));
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

extern "C" ZenResultCode zen_native_backend_present(ZenNativeBackendHandle backend)
{
    try {
        return with_backend(backend, [](zeno::native::NativeBackend& native_backend) {
            return native_backend.present()
                ? ZEN_RESULT_OK
                : ZEN_RESULT_BACKEND_ERROR;
        });
    } catch (...) {
        return ZEN_RESULT_INTERNAL_ERROR;
    }
}

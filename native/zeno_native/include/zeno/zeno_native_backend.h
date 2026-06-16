#pragma once

#include <stdint.h>

#include <zeno/zeno_abi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZEN_NATIVE_BACKEND_CONFIG_API_VERSION 1u
#define ZEN_NATIVE_BACKEND_CONFIG_SIZE ((uint32_t)sizeof(ZenNativeBackendConfig))
#define ZEN_NATIVE_WINDOW_CONFIG_API_VERSION 1u
#define ZEN_NATIVE_WINDOW_CONFIG_SIZE ((uint32_t)sizeof(ZenNativeWindowConfig))
#define ZEN_INPUT_SNAPSHOT_API_VERSION 1u
#define ZEN_INPUT_SNAPSHOT_SIZE ((uint32_t)sizeof(ZenInputSnapshot))
#define ZEN_SHADER_COMPILE_LOG_API_VERSION 1u
#define ZEN_SHADER_COMPILE_LOG_SIZE ((uint32_t)sizeof(ZenShaderCompileLog))
#define ZEN_SHADER_COMPILE_LOG_MESSAGE_CAPACITY 1024u
#define ZEN_VERTEX_INPUT_LAYOUT_DESC_API_VERSION 1u
#define ZEN_VERTEX_INPUT_LAYOUT_DESC_SIZE ((uint32_t)sizeof(ZenVertexInputLayoutDesc))
#define ZEN_VERTEX_INPUT_LAYOUT_MAX_ELEMENTS 8u
#define ZEN_SPRITE_DRAW_DESC_API_VERSION 1u
#define ZEN_SPRITE_DRAW_DESC_SIZE ((uint32_t)sizeof(ZenSpriteDrawDesc))
#define ZEN_MESH_DESC_API_VERSION 1u
#define ZEN_MESH_DESC_SIZE ((uint32_t)sizeof(ZenMeshDesc))

typedef enum ZenInputKey {
    ZEN_INPUT_KEY_UNKNOWN = 0,
    ZEN_INPUT_KEY_ESCAPE = 1,
    ZEN_INPUT_KEY_SPACE = 2,
    ZEN_INPUT_KEY_LEFT = 3,
    ZEN_INPUT_KEY_RIGHT = 4,
    ZEN_INPUT_KEY_UP = 5,
    ZEN_INPUT_KEY_DOWN = 6,
    ZEN_INPUT_KEY_A = 7,
    ZEN_INPUT_KEY_D = 8,
    ZEN_INPUT_KEY_W = 9,
    ZEN_INPUT_KEY_S = 10,
    ZEN_INPUT_KEY_COUNT = 11
} ZenInputKey;

typedef enum ZenInputMouseButton {
    ZEN_INPUT_MOUSE_BUTTON_LEFT = 0,
    ZEN_INPUT_MOUSE_BUTTON_RIGHT = 1,
    ZEN_INPUT_MOUSE_BUTTON_MIDDLE = 2,
    ZEN_INPUT_MOUSE_BUTTON_COUNT = 3
} ZenInputMouseButton;

typedef struct ZenNativeBackendHandle {
    /* Non-zero native backend registry handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenNativeBackendHandle;

typedef struct ZenRenderClearColorHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenRenderClearColorHandle;

typedef struct ZenRenderTriangleHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenRenderTriangleHandle;

typedef struct ZenVertexShaderHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenVertexShaderHandle;

typedef struct ZenPixelShaderHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenPixelShaderHandle;

typedef struct ZenTextureHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenTextureHandle;

typedef struct ZenMeshHandle {
    /* Non-zero renderer resource handle. Value 0 is always invalid/null. */
    uint64_t value;
} ZenMeshHandle;

typedef enum ZenVertexInputSemantic {
    ZEN_VERTEX_INPUT_SEMANTIC_POSITION = 1,
    ZEN_VERTEX_INPUT_SEMANTIC_COLOR = 2
} ZenVertexInputSemantic;

typedef enum ZenVertexInputFormat {
    ZEN_VERTEX_INPUT_FORMAT_FLOAT3 = 1,
    ZEN_VERTEX_INPUT_FORMAT_FLOAT4 = 2
} ZenVertexInputFormat;

typedef struct ZenNativeBackendConfig {
    /* Must be set to ZEN_NATIVE_BACKEND_CONFIG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_NATIVE_BACKEND_CONFIG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Reserved for future backend flags. Must be 0 for this phase. */
    uint32_t flags;
    uint32_t reserved;
} ZenNativeBackendConfig;

typedef struct ZenNativeWindowConfig {
    /* Must be set to ZEN_NATIVE_WINDOW_CONFIG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_NATIVE_WINDOW_CONFIG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Client area width in physical pixels. Must be greater than 0. */
    uint32_t width;
    /* Client area height in physical pixels. Must be greater than 0. */
    uint32_t height;
} ZenNativeWindowConfig;

typedef struct ZenInputSnapshot {
    /* Must be set to ZEN_INPUT_SNAPSHOT_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_INPUT_SNAPSHOT_API_VERSION by the caller. */
    uint32_t api_version;
    /* Mouse x in client-area pixels. */
    int32_t mouse_x;
    /* Mouse y in client-area pixels. */
    int32_t mouse_y;
    /* Mouse wheel delta accumulated during the most recent poll_events call. */
    int32_t mouse_wheel_delta;
    uint32_t reserved;
    /* Indexed by ZenInputKey values. */
    uint8_t key_down[ZEN_INPUT_KEY_COUNT];
    uint8_t key_pressed[ZEN_INPUT_KEY_COUNT];
    uint8_t key_released[ZEN_INPUT_KEY_COUNT];
    uint8_t mouse_down[ZEN_INPUT_MOUSE_BUTTON_COUNT];
    uint8_t mouse_pressed[ZEN_INPUT_MOUSE_BUTTON_COUNT];
    uint8_t mouse_released[ZEN_INPUT_MOUSE_BUTTON_COUNT];
} ZenInputSnapshot;

typedef struct ZenShaderCompileLog {
    /* Must be set to ZEN_SHADER_COMPILE_LOG_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_SHADER_COMPILE_LOG_API_VERSION by the caller. */
    uint32_t api_version;
    /* Number of bytes written to message, excluding the trailing null byte. */
    uint32_t message_length;
    uint32_t reserved;
    char message[ZEN_SHADER_COMPILE_LOG_MESSAGE_CAPACITY];
} ZenShaderCompileLog;

typedef struct ZenMatrix4x4 {
    /*
     * Row-major 4x4 matrix using row-vector convention.
     * Translation lives in elements[12], elements[13], elements[14].
     */
    float elements[16];
} ZenMatrix4x4;

typedef struct ZenVertexInputElement {
    /* One of ZenVertexInputSemantic. */
    uint32_t semantic;
    uint32_t semantic_index;
    /* One of ZenVertexInputFormat. */
    uint32_t format;
    uint32_t input_slot;
    uint32_t aligned_byte_offset;
    uint32_t reserved[3];
} ZenVertexInputElement;

typedef struct ZenVertexInputLayoutDesc {
    /* Must be set to ZEN_VERTEX_INPUT_LAYOUT_DESC_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_VERTEX_INPUT_LAYOUT_DESC_API_VERSION by the caller. */
    uint32_t api_version;
    /* Number of valid entries in elements. Must be 1..ZEN_VERTEX_INPUT_LAYOUT_MAX_ELEMENTS. */
    uint32_t element_count;
    uint32_t reserved;
    ZenVertexInputElement elements[ZEN_VERTEX_INPUT_LAYOUT_MAX_ELEMENTS];
} ZenVertexInputLayoutDesc;

typedef struct ZenSpriteDrawDesc {
    /* Must be set to ZEN_SPRITE_DRAW_DESC_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_SPRITE_DRAW_DESC_API_VERSION by the caller. */
    uint32_t api_version;
    uint32_t reserved[2];
    /* Row-major model matrix using the same convention as ZenMatrix4x4. */
    ZenMatrix4x4 model_matrix;
    /* Multiplicative RGBA tint. */
    float color[4];
} ZenSpriteDrawDesc;

typedef struct ZenMeshDesc {
    /* Must be set to ZEN_MESH_DESC_SIZE by the caller. */
    uint32_t size;
    /* Must be set to ZEN_MESH_DESC_API_VERSION by the caller. */
    uint32_t api_version;
    /* Size in bytes of one vertex. Must contain position float3 and color float4. */
    uint32_t vertex_stride_bytes;
    uint32_t reserved;
    /* Borrowed vertex byte range, only needed during the call. */
    const void* vertex_data;
    uint64_t vertex_count;
    /* Borrowed uint32 index range, only needed during the call. */
    const uint32_t* index_data;
    uint64_t index_count;
} ZenMeshDesc;

/*
 * Creates the native backend shell.
 *
 * config: borrowed, must not be null, only needed during the call.
 * out_backend: borrowed output pointer, must not be null. Receives an owned
 * non-zero handle on success.
 * The caller must eventually pass the returned handle to zen_native_backend_destroy.
 * Thread-safety: this function may be called concurrently with other create
 * or destroy calls.
 */
ZenResultCode zen_native_backend_create(
    const ZenNativeBackendConfig* config,
    ZenNativeBackendHandle* out_backend);

/*
 * Destroys an owned native backend handle.
 *
 * backend: must be a non-zero handle returned by zen_native_backend_create.
 * After ZEN_RESULT_OK, the handle is invalid and must not be used again.
 * Thread-safety: this function may be called concurrently for different
 * handles. The same handle must not be used by destroy, create_window, or
 * poll_events concurrently from multiple threads. Renderer functions also
 * must not run concurrently with destroy for the same handle.
 */
ZenResultCode zen_native_backend_destroy(ZenNativeBackendHandle backend);

/*
 * Creates the backend-owned Win32 window using a default title.
 *
 * backend: must be a live non-zero backend handle.
 * config: borrowed, must not be null, only needed during the call.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_create_window(
    ZenNativeBackendHandle backend,
    const ZenNativeWindowConfig* config);

/*
 * Polls pending Win32 messages without blocking.
 *
 * backend: must be a live non-zero backend handle.
 * out_should_close: borrowed output pointer, must not be null. Receives 1 when
 * the window has requested close, otherwise 0.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_poll_events(
    ZenNativeBackendHandle backend,
    uint32_t* out_should_close);

/*
 * Copies the latest per-frame input snapshot.
 *
 * backend: must be a live non-zero backend handle.
 * out_snapshot: borrowed output pointer, must not be null. The caller must set
 * size and api_version before calling. On success, the full struct is written.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy or poll_events.
 */
ZenResultCode zen_native_backend_get_input_snapshot(
    ZenNativeBackendHandle backend,
    ZenInputSnapshot* out_snapshot);

/*
 * Initializes DirectX 11 resources for the backend-owned window.
 *
 * backend: must be a live non-zero backend handle with a created window.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_initialize_renderer(ZenNativeBackendHandle backend);

/*
 * Begins one render frame by binding the current render target.
 *
 * backend: must have an initialized renderer and no active frame.
 * The frame remains active until zen_native_backend_present succeeds or fails.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_begin_frame(ZenNativeBackendHandle backend);

/*
 * Clears the current active render frame to the supplied color.
 *
 * backend: must have an initialized renderer and an active frame.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_clear(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a);

/*
 * Creates a backend-owned clear color resource.
 *
 * backend: must have an initialized renderer.
 * out_clear_color: borrowed output pointer, must not be null. Receives an owned
 * non-zero resource handle on success.
 * On failure, out_clear_color is left unchanged.
 * The caller must release a successful handle with
 * zen_native_backend_destroy_clear_color before or during backend shutdown.
 * Destroying the backend invalidates outstanding clear color handles.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_create_clear_color(
    ZenNativeBackendHandle backend,
    float r,
    float g,
    float b,
    float a,
    ZenRenderClearColorHandle* out_clear_color);

/*
 * Destroys a backend-owned clear color resource.
 *
 * backend: must have an initialized renderer. clear_color must be a non-zero
 * handle created by zen_native_backend_create_clear_color for the same backend.
 * Returns ZEN_RESULT_NOT_INITIALIZED when a non-zero handle is not present,
 * including double destroy and handles invalidated by backend destruction.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_destroy_clear_color(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color);

/*
 * Clears the current active render frame using a backend-owned clear color resource.
 *
 * backend: must have an initialized renderer and an active frame. clear_color
 * must be valid for that backend.
 * Returns ZEN_RESULT_NOT_INITIALIZED when a non-zero resource handle is not
 * present or the renderer is no longer initialized.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_clear_with_resource(
    ZenNativeBackendHandle backend,
    ZenRenderClearColorHandle clear_color);

/*
 * Creates a backend-owned minimal triangle render resource.
 *
 * backend: must have an initialized renderer.
 * out_triangle: borrowed output pointer, must not be null. Receives an owned
 * non-zero resource handle on success.
 * On failure, out_triangle is left unchanged.
 * The caller must release a successful handle with
 * zen_native_backend_destroy_triangle before or during backend shutdown.
 * Destroying the backend invalidates outstanding triangle handles.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_create_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle* out_triangle);

/*
 * Creates a backend-owned triangle resource using caller-provided shader
 * handles and a POD vertex input layout descriptor.
 *
 * backend: must have an initialized renderer.
 * vertex_shader and pixel_shader: must be live non-zero handles created for
 * the same backend.
 * input_layout: borrowed, must not be null, copied during the call.
 * out_triangle: borrowed output pointer, must not be null.
 */
ZenResultCode zen_native_backend_create_triangle_with_shaders(
    ZenNativeBackendHandle backend,
    ZenVertexShaderHandle vertex_shader,
    ZenPixelShaderHandle pixel_shader,
    const ZenVertexInputLayoutDesc* input_layout,
    ZenRenderTriangleHandle* out_triangle);

/*
 * Destroys a backend-owned triangle render resource.
 *
 * backend: must have an initialized renderer. triangle must be a non-zero
 * handle created by zen_native_backend_create_triangle for the same backend.
 * Returns ZEN_RESULT_NOT_INITIALIZED when a non-zero handle is not present,
 * including double destroy and handles invalidated by backend destruction.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_destroy_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle);

/*
 * Compiles and creates a backend-owned DirectX 11 vertex shader.
 *
 * Source, entry, and profile are borrowed UTF-8 byte ranges with explicit
 * lengths. They are only needed during the call and are not required to be
 * null-terminated.
 * compile_log: borrowed, must not be null. The caller must set size and
 * api_version before calling. The message buffer is always null-terminated on
 * return.
 * out_shader: borrowed output pointer, must not be null.
 */
ZenResultCode zen_native_backend_create_vertex_shader_from_source(
    ZenNativeBackendHandle backend,
    const char* source_utf8,
    uint64_t source_byte_count,
    const char* entry_utf8,
    uint64_t entry_byte_count,
    const char* profile_utf8,
    uint64_t profile_byte_count,
    ZenShaderCompileLog* compile_log,
    ZenVertexShaderHandle* out_shader);

/*
 * Compiles and creates a backend-owned DirectX 11 pixel shader.
 *
 * String and log ownership follows zen_native_backend_create_vertex_shader_from_source.
 */
ZenResultCode zen_native_backend_create_pixel_shader_from_source(
    ZenNativeBackendHandle backend,
    const char* source_utf8,
    uint64_t source_byte_count,
    const char* entry_utf8,
    uint64_t entry_byte_count,
    const char* profile_utf8,
    uint64_t profile_byte_count,
    ZenShaderCompileLog* compile_log,
    ZenPixelShaderHandle* out_shader);

/*
 * Destroys a backend-owned vertex shader handle.
 */
ZenResultCode zen_native_backend_destroy_vertex_shader(
    ZenNativeBackendHandle backend,
    ZenVertexShaderHandle shader);

/*
 * Destroys a backend-owned pixel shader handle.
 */
ZenResultCode zen_native_backend_destroy_pixel_shader(
    ZenNativeBackendHandle backend,
    ZenPixelShaderHandle shader);

/*
 * Decodes PNG/BMP image bytes with WIC and creates a backend-owned 2D texture.
 *
 * image_bytes: borrowed byte range, only needed during the call.
 * out_texture: borrowed output pointer, must not be null.
 * The caller must destroy a successful handle with
 * zen_native_backend_destroy_texture before or during backend shutdown.
 */
ZenResultCode zen_native_backend_create_texture_from_memory(
    ZenNativeBackendHandle backend,
    const uint8_t* image_bytes,
    uint64_t image_byte_count,
    ZenTextureHandle* out_texture);

/*
 * Destroys a backend-owned texture handle.
 */
ZenResultCode zen_native_backend_destroy_texture(
    ZenNativeBackendHandle backend,
    ZenTextureHandle texture);

/*
 * Draws a textured 2D sprite using the current camera matrix.
 *
 * backend: must have an initialized renderer and an active frame.
 * texture: must be a valid non-zero texture handle for this backend.
 * desc: borrowed POD draw descriptor, copied during the call.
 */
ZenResultCode zen_native_backend_draw_sprite(
    ZenNativeBackendHandle backend,
    ZenTextureHandle texture,
    const ZenSpriteDrawDesc* desc);

/*
 * Creates a backend-owned indexed mesh from caller-owned POD vertex/index data.
 *
 * Mesh vertices for this phase must use position float3 at byte offset 0 and
 * color float4 at byte offset sizeof(float) * 3. Indices are uint32.
 * desc: borrowed, copied into backend-owned GPU buffers during the call.
 * out_mesh: borrowed output pointer, must not be null.
 */
ZenResultCode zen_native_backend_create_mesh(
    ZenNativeBackendHandle backend,
    const ZenMeshDesc* desc,
    ZenMeshHandle* out_mesh);

/*
 * Destroys a backend-owned mesh handle.
 */
ZenResultCode zen_native_backend_destroy_mesh(
    ZenNativeBackendHandle backend,
    ZenMeshHandle mesh);

/*
 * Draws a backend-owned mesh with the supplied model matrix and current camera matrix.
 *
 * backend: must have an initialized renderer and an active frame.
 * mesh: must be a valid non-zero mesh handle for this backend.
 * model_matrix: borrowed pointer to a row-major left-handed matrix. The data
 * is copied during the call.
 */
ZenResultCode zen_native_backend_draw_mesh(
    ZenNativeBackendHandle backend,
    ZenMeshHandle mesh,
    const ZenMatrix4x4* model_matrix);

/*
 * Draws a backend-owned minimal triangle render resource.
 *
 * backend: must have an initialized renderer and an active frame.
 * triangle: must be a non-zero handle created by
 * zen_native_backend_create_triangle for the same backend.
 * Returns ZEN_RESULT_NOT_INITIALIZED when a non-zero handle is not present.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_draw_triangle(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle);

/*
 * Sets the camera/view-projection matrix used by transformed draw calls.
 *
 * backend: must have an initialized renderer.
 * camera_matrix: borrowed pointer to a row-major left-handed matrix. The data
 * is copied during the call and may be released by the caller afterwards.
 */
ZenResultCode zen_native_backend_set_camera_matrix(
    ZenNativeBackendHandle backend,
    const ZenMatrix4x4* camera_matrix);

/*
 * Draws a backend-owned triangle using the supplied model matrix and the
 * current camera matrix.
 *
 * backend: must have an initialized renderer and an active frame.
 * triangle: must be a valid non-zero triangle handle for this backend.
 * model_matrix: borrowed pointer to a row-major left-handed matrix. The data
 * is copied during the call.
 */
ZenResultCode zen_native_backend_draw_triangle_transformed(
    ZenNativeBackendHandle backend,
    ZenRenderTriangleHandle triangle,
    const ZenMatrix4x4* model_matrix);

/*
 * Presents the current swap chain frame and closes the active frame.
 *
 * backend: must have an initialized renderer and an active frame.
 * Thread-safety: this function must not be called concurrently for the same
 * backend handle, including concurrent destroy.
 */
ZenResultCode zen_native_backend_present(ZenNativeBackendHandle backend);

#ifdef __cplusplus
}
#endif

#include <zeno/zeno_native_backend.h>

#include <chrono>
#include <thread>

namespace {

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

constexpr const char kShaderSource[] = R"(
cbuffer TriangleTransformConstants : register(b0) {
    row_major float4x4 u_world;
    row_major float4x4 u_view_projection;
};

struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput vs_main(VSInput input) {
    PSInput output;
    output.position = mul(mul(float4(input.position, 1.0), u_world), u_view_projection);
    output.color = input.color;
    return output;
}

float4 ps_main(PSInput input) : SV_TARGET {
    return input.color;
}
)";

constexpr const char kInvalidShaderSource[] = "float4 broken_shader(";

constexpr uint8_t kBmp2x2[] = {
    0x42, 0x4D, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x13, 0x0B,
    0x00, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
};

ZenShaderCompileLog make_log()
{
    ZenShaderCompileLog log{};
    log.size = ZEN_SHADER_COMPILE_LOG_SIZE;
    log.api_version = ZEN_SHADER_COMPILE_LOG_API_VERSION;
    return log;
}

ZenVertexInputLayoutDesc make_triangle_input_layout()
{
    ZenVertexInputLayoutDesc layout{};
    layout.size = ZEN_VERTEX_INPUT_LAYOUT_DESC_SIZE;
    layout.api_version = ZEN_VERTEX_INPUT_LAYOUT_DESC_API_VERSION;
    layout.element_count = 2;
    layout.elements[0].semantic = ZEN_VERTEX_INPUT_SEMANTIC_POSITION;
    layout.elements[0].format = ZEN_VERTEX_INPUT_FORMAT_FLOAT3;
    layout.elements[0].aligned_byte_offset = 0;
    layout.elements[1].semantic = ZEN_VERTEX_INPUT_SEMANTIC_COLOR;
    layout.elements[1].format = ZEN_VERTEX_INPUT_FORMAT_FLOAT4;
    layout.elements[1].aligned_byte_offset = sizeof(float) * 3;
    return layout;
}

ZenMatrix4x4 identity_matrix()
{
    ZenMatrix4x4 matrix{};
    matrix.elements[0] = 1.0f;
    matrix.elements[5] = 1.0f;
    matrix.elements[10] = 1.0f;
    matrix.elements[15] = 1.0f;
    return matrix;
}

ZenSpriteDrawDesc make_sprite_draw_desc()
{
    ZenSpriteDrawDesc desc{};
    desc.size = ZEN_SPRITE_DRAW_DESC_SIZE;
    desc.api_version = ZEN_SPRITE_DRAW_DESC_API_VERSION;
    desc.model_matrix = identity_matrix();
    desc.color[0] = 1.0f;
    desc.color[1] = 1.0f;
    desc.color[2] = 1.0f;
    desc.color[3] = 0.85f;
    return desc;
}

} // namespace

int main()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;
    config.flags = 0;
    config.reserved = 0;

    if (!expect(zen_native_backend_create(nullptr, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 1;
    }

    ZenNativeBackendHandle scratch_backend{ 777 };
    if (!expect(zen_native_backend_create(nullptr, &scratch_backend), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_backend.value != 777) {
        return 2;
    }

    if (!expect(zen_native_backend_create(&config, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 3;
    }

    ZenNativeBackendConfig invalid_config = config;
    invalid_config.size = 0;
    scratch_backend.value = 777;
    if (!expect(zen_native_backend_create(&invalid_config, &scratch_backend), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_backend.value != 777) {
        return 4;
    }

    ZenNativeBackendHandle invalid_backend{};
    if (!expect(zen_native_backend_destroy(invalid_backend), ZEN_RESULT_INVALID_ARGUMENT)) {
        return 5;
    }

    invalid_backend.value = 123456;
    if (!expect(zen_native_backend_begin_frame(invalid_backend), ZEN_RESULT_NOT_INITIALIZED)) {
        return 6;
    }

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 7;
    }

    if (!expect(zen_native_backend_create_window(backend, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    ZenNativeWindowConfig invalid_window_config{};
    invalid_window_config.size = ZEN_NATIVE_WINDOW_CONFIG_SIZE;
    invalid_window_config.api_version = ZEN_NATIVE_WINDOW_CONFIG_API_VERSION;
    invalid_window_config.width = 0;
    invalid_window_config.height = 360;
    if (!expect(zen_native_backend_create_window(backend, &invalid_window_config), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    uint32_t should_close = 99;
    if (!expect(zen_native_backend_poll_events(backend, nullptr), ZEN_RESULT_INVALID_ARGUMENT)
        || should_close != 99) {
        zen_native_backend_destroy(backend);
        return 10;
    }

    if (!expect(zen_native_backend_initialize_renderer(backend), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    if (!expect(zen_native_backend_begin_frame(backend), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 12;
    }

    if (!expect(zen_native_backend_clear(backend, 0.0f, 0.0f, 0.0f, 1.0f), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 13;
    }

    ZenRenderClearColorHandle scratch_clear_color{ 777 };
    if (!expect(
            zen_native_backend_create_clear_color(backend, 0.0f, 0.0f, 0.0f, 1.0f, &scratch_clear_color),
            ZEN_RESULT_BACKEND_ERROR)
        || scratch_clear_color.value != 777) {
        zen_native_backend_destroy(backend);
        return 14;
    }

    ZenRenderTriangleHandle scratch_triangle{ 777 };
    if (!expect(zen_native_backend_create_triangle(backend, &scratch_triangle), ZEN_RESULT_BACKEND_ERROR)
        || scratch_triangle.value != 777) {
        zen_native_backend_destroy(backend);
        return 15;
    }

    if (!expect(zen_native_backend_draw_triangle(backend, scratch_triangle), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 16;
    }

    ZenMatrix4x4 identity = identity_matrix();
    if (!expect(zen_native_backend_set_camera_matrix(backend, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 17;
    }

    if (!expect(zen_native_backend_set_camera_matrix(backend, &identity), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 18;
    }

    if (!expect(
            zen_native_backend_draw_triangle_transformed(backend, scratch_triangle, nullptr),
            ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 19;
    }

    if (!expect(
            zen_native_backend_draw_triangle_transformed(backend, scratch_triangle, &identity),
            ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 20;
    }

    if (!expect(zen_native_backend_destroy_triangle(backend, scratch_triangle), ZEN_RESULT_NOT_INITIALIZED)) {
        zen_native_backend_destroy(backend);
        return 21;
    }

    if (!expect(zen_native_backend_present(backend), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 22;
    }

    ZenTextureHandle scratch_texture{ 777 };
    if (!expect(
            zen_native_backend_create_texture_from_memory(backend, kBmp2x2, sizeof(kBmp2x2), &scratch_texture),
            ZEN_RESULT_BACKEND_ERROR)
        || scratch_texture.value != 777) {
        zen_native_backend_destroy(backend);
        return 112;
    }

    ZenSpriteDrawDesc sprite_desc = make_sprite_draw_desc();
    scratch_texture.value = 777;
    if (!expect(zen_native_backend_draw_sprite(backend, scratch_texture, &sprite_desc), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 113;
    }

    ZenNativeWindowConfig window_config{};
    window_config.size = ZEN_NATIVE_WINDOW_CONFIG_SIZE;
    window_config.api_version = ZEN_NATIVE_WINDOW_CONFIG_API_VERSION;
    window_config.width = 640;
    window_config.height = 360;

    result = zen_native_backend_create_window(backend, &window_config);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 23;
    }

    result = zen_native_backend_initialize_renderer(backend);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 24;
    }

    result = zen_native_backend_set_camera_matrix(backend, &identity);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 25;
    }

    ZenTextureHandle texture{};
    result = zen_native_backend_create_texture_from_memory(backend, kBmp2x2, sizeof(kBmp2x2), &texture);
    if (result != ZEN_RESULT_OK || texture.value == 0) {
        zen_native_backend_destroy(backend);
        return 114;
    }

    ZenTextureHandle second_texture{};
    result = zen_native_backend_create_texture_from_memory(backend, kBmp2x2, sizeof(kBmp2x2), &second_texture);
    if (result != ZEN_RESULT_OK || second_texture.value == 0 || second_texture.value == texture.value) {
        zen_native_backend_destroy_texture(backend, texture);
        zen_native_backend_destroy(backend);
        return 115;
    }

    result = zen_native_backend_destroy_texture(backend, second_texture);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_texture(backend, texture);
        zen_native_backend_destroy(backend);
        return 116;
    }

    result = zen_native_backend_destroy_texture(backend, second_texture);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_texture(backend, texture);
        zen_native_backend_destroy(backend);
        return 117;
    }

    ZenShaderCompileLog shader_log = make_log();
    ZenVertexShaderHandle vertex_shader{};
    result = zen_native_backend_create_vertex_shader_from_source(
        backend,
        kShaderSource,
        sizeof(kShaderSource) - 1,
        "vs_main",
        7,
        "vs_4_0",
        6,
        &shader_log,
        &vertex_shader);
    if (result != ZEN_RESULT_OK || vertex_shader.value == 0 || shader_log.message_length != 0) {
        zen_native_backend_destroy(backend);
        return 101;
    }

    shader_log = make_log();
    ZenPixelShaderHandle pixel_shader{};
    result = zen_native_backend_create_pixel_shader_from_source(
        backend,
        kShaderSource,
        sizeof(kShaderSource) - 1,
        "ps_main",
        7,
        "ps_4_0",
        6,
        &shader_log,
        &pixel_shader);
    if (result != ZEN_RESULT_OK || pixel_shader.value == 0 || shader_log.message_length != 0) {
        zen_native_backend_destroy(backend);
        return 102;
    }

    shader_log = make_log();
    ZenVertexShaderHandle invalid_shader{};
    result = zen_native_backend_create_vertex_shader_from_source(
        backend,
        kInvalidShaderSource,
        sizeof(kInvalidShaderSource) - 1,
        "vs_main",
        7,
        "vs_4_0",
        6,
        &shader_log,
        &invalid_shader);
    if (result != ZEN_RESULT_BACKEND_ERROR || invalid_shader.value != 0 || shader_log.message_length == 0) {
        zen_native_backend_destroy(backend);
        return 103;
    }

    ZenRenderTriangleHandle shader_triangle{};
    ZenVertexInputLayoutDesc shader_layout = make_triangle_input_layout();
    result = zen_native_backend_create_triangle_with_shaders(
        backend,
        vertex_shader,
        pixel_shader,
        &shader_layout,
        &shader_triangle);
    if (result != ZEN_RESULT_OK || shader_triangle.value == 0) {
        zen_native_backend_destroy(backend);
        return 104;
    }

    ZenRenderClearColorHandle clear_color{};
    result = zen_native_backend_create_clear_color(backend, 0.05f, 0.20f, 0.35f, 1.0f, &clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 26;
    }

    ZenRenderClearColorHandle second_clear_color{};
    result = zen_native_backend_create_clear_color(backend, 0.35f, 0.10f, 0.10f, 1.0f, &second_clear_color);
    if (result != ZEN_RESULT_OK || second_clear_color.value == 0 || second_clear_color.value == clear_color.value) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 27;
    }

    result = zen_native_backend_destroy_clear_color(backend, second_clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 28;
    }

    ZenRenderClearColorHandle invalid_clear_color{};
    result = zen_native_backend_clear_with_resource(backend, invalid_clear_color);
    if (result != ZEN_RESULT_INVALID_ARGUMENT) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 24;
    }

    ZenRenderTriangleHandle invalid_triangle{};
    result = zen_native_backend_draw_triangle(backend, invalid_triangle);
    if (result != ZEN_RESULT_INVALID_ARGUMENT) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 25;
    }

    result = zen_native_backend_destroy_triangle(backend, invalid_triangle);
    if (result != ZEN_RESULT_INVALID_ARGUMENT) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 26;
    }

    ZenTextureHandle invalid_texture{};
    result = zen_native_backend_draw_sprite(backend, invalid_texture, &sprite_desc);
    if (result != ZEN_RESULT_INVALID_ARGUMENT) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 118;
    }

    invalid_texture.value = 999999;
    result = zen_native_backend_draw_sprite(backend, invalid_texture, &sprite_desc);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 119;
    }

    ZenRenderTriangleHandle fake_triangle{ 999999 };
    result = zen_native_backend_draw_triangle(backend, fake_triangle);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 27;
    }

    result = zen_native_backend_draw_triangle_transformed(backend, fake_triangle, &identity);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 28;
    }

    ZenRenderTriangleHandle triangle{};
    result = zen_native_backend_create_triangle(backend, &triangle);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 28;
    }

    ZenRenderTriangleHandle second_triangle{};
    result = zen_native_backend_create_triangle(backend, &second_triangle);
    if (result != ZEN_RESULT_OK || second_triangle.value == 0 || second_triangle.value == triangle.value) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 29;
    }

    result = zen_native_backend_destroy_triangle(backend, second_triangle);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 30;
    }

    result = zen_native_backend_clear_with_resource(backend, clear_color);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 31;
    }

    result = zen_native_backend_draw_triangle(backend, triangle);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 32;
    }

    result = zen_native_backend_draw_triangle_transformed(backend, triangle, &identity);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 33;
    }

    result = zen_native_backend_present(backend);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 33;
    }

    result = zen_native_backend_draw_sprite(backend, texture, &sprite_desc);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 120;
    }

    result = zen_native_backend_begin_frame(backend);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 34;
    }

    result = zen_native_backend_begin_frame(backend);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 35;
    }

    result = zen_native_backend_clear_with_resource(backend, clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 36;
    }

    result = zen_native_backend_draw_triangle(backend, triangle);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 37;
    }

    result = zen_native_backend_draw_triangle_transformed(backend, triangle, &identity);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_triangle(backend, shader_triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 38;
    }

    result = zen_native_backend_draw_triangle_transformed(backend, shader_triangle, &identity);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, shader_triangle);
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 105;
    }

    result = zen_native_backend_draw_sprite(backend, texture, &sprite_desc);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, shader_triangle);
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 121;
    }

    result = zen_native_backend_present(backend);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 38;
    }

    result = zen_native_backend_clear_with_resource(backend, clear_color);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 39;
    }

    result = zen_native_backend_draw_triangle(backend, triangle);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 40;
    }

    result = zen_native_backend_present(backend);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 41;
    }

    result = zen_native_backend_draw_sprite(backend, texture, &sprite_desc);
    if (result != ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_clear_color(backend, clear_color);
        zen_native_backend_destroy(backend);
        return 122;
    }

    for (int i = 0; i < 120; ++i) {
        uint32_t should_close = 0;
        result = zen_native_backend_poll_events(backend, &should_close);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_triangle(backend, triangle);
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 42;
        }

        if (should_close != 0) {
            break;
        }

        result = zen_native_backend_begin_frame(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_triangle(backend, triangle);
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 43;
        }

        result = zen_native_backend_clear_with_resource(backend, clear_color);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_triangle(backend, triangle);
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 44;
        }

        result = zen_native_backend_draw_triangle(backend, triangle);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_triangle(backend, triangle);
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 45;
        }

        result = zen_native_backend_draw_sprite(backend, texture, &sprite_desc);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_triangle(backend, triangle);
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 123;
        }

        result = zen_native_backend_present(backend);
        if (result != ZEN_RESULT_OK) {
            zen_native_backend_destroy_triangle(backend, triangle);
            zen_native_backend_destroy_clear_color(backend, clear_color);
            zen_native_backend_destroy(backend);
            return 46;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    result = zen_native_backend_destroy_clear_color(backend, clear_color);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy(backend);
        return 47;
    }

    result = zen_native_backend_clear_with_resource(backend, clear_color);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy(backend);
        return 48;
    }

    result = zen_native_backend_destroy_clear_color(backend, clear_color);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy_triangle(backend, shader_triangle);
        zen_native_backend_destroy(backend);
        return 49;
    }

    result = zen_native_backend_destroy_triangle(backend, shader_triangle);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy(backend);
        return 106;
    }

    result = zen_native_backend_destroy_triangle(backend, shader_triangle);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy_triangle(backend, triangle);
        zen_native_backend_destroy(backend);
        return 107;
    }

    result = zen_native_backend_destroy_triangle(backend, triangle);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 50;
    }

    result = zen_native_backend_destroy_texture(backend, texture);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 124;
    }

    result = zen_native_backend_draw_sprite(backend, texture, &sprite_desc);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 125;
    }

    result = zen_native_backend_destroy_texture(backend, texture);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 126;
    }

    result = zen_native_backend_draw_triangle(backend, triangle);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 51;
    }

    result = zen_native_backend_destroy_triangle(backend, triangle);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 52;
    }

    result = zen_native_backend_destroy_vertex_shader(backend, vertex_shader);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 108;
    }

    result = zen_native_backend_destroy_vertex_shader(backend, vertex_shader);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 109;
    }

    result = zen_native_backend_destroy_pixel_shader(backend, pixel_shader);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 110;
    }

    result = zen_native_backend_destroy_pixel_shader(backend, pixel_shader);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        zen_native_backend_destroy(backend);
        return 111;
    }

    ZenRenderTriangleHandle backend_owned_triangle{};
    result = zen_native_backend_create_triangle(backend, &backend_owned_triangle);
    if (result != ZEN_RESULT_OK) {
        zen_native_backend_destroy(backend);
        return 53;
    }

    result = zen_native_backend_destroy(backend);
    if (result != ZEN_RESULT_OK) {
        return 54;
    }

    result = zen_native_backend_draw_triangle(backend, backend_owned_triangle);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        return 55;
    }

    result = zen_native_backend_poll_events(backend, &should_close);
    if (result != ZEN_RESULT_NOT_INITIALIZED) {
        return 56;
    }

    result = zen_native_backend_destroy(backend);
    return result == ZEN_RESULT_NOT_INITIALIZED ? 0 : 57;
}

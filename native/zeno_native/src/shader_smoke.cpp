#include <zeno/zeno_native_backend.h>

namespace {

constexpr const char kValidShaderSource[] = R"(
float4 vs_main(float3 position : POSITION) : SV_POSITION {
    return float4(position, 1.0);
}
)";

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

ZenShaderCompileLog make_log()
{
    ZenShaderCompileLog log{};
    log.size = ZEN_SHADER_COMPILE_LOG_SIZE;
    log.api_version = ZEN_SHADER_COMPILE_LOG_API_VERSION;
    return log;
}

} // namespace

int main()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 1;
    }

    ZenShaderCompileLog log = make_log();
    ZenVertexShaderHandle vertex_shader{ 777 };
    result = zen_native_backend_create_vertex_shader_from_source(
        backend,
        nullptr,
        sizeof(kValidShaderSource) - 1,
        "vs_main",
        7,
        "vs_4_0",
        6,
        &log,
        &vertex_shader);
    if (!expect(result, ZEN_RESULT_INVALID_ARGUMENT) || vertex_shader.value != 777) {
        zen_native_backend_destroy(backend);
        return 2;
    }

    result = zen_native_backend_create_vertex_shader_from_source(
        backend,
        kValidShaderSource,
        0,
        "vs_main",
        7,
        "vs_4_0",
        6,
        &log,
        &vertex_shader);
    if (!expect(result, ZEN_RESULT_INVALID_ARGUMENT) || vertex_shader.value != 777) {
        zen_native_backend_destroy(backend);
        return 3;
    }

    ZenShaderCompileLog invalid_log = log;
    invalid_log.size = 0;
    result = zen_native_backend_create_vertex_shader_from_source(
        backend,
        kValidShaderSource,
        sizeof(kValidShaderSource) - 1,
        "vs_main",
        7,
        "vs_4_0",
        6,
        &invalid_log,
        &vertex_shader);
    if (!expect(result, ZEN_RESULT_INVALID_ARGUMENT) || vertex_shader.value != 777) {
        zen_native_backend_destroy(backend);
        return 4;
    }

    log = make_log();
    result = zen_native_backend_create_vertex_shader_from_source(
        backend,
        kValidShaderSource,
        sizeof(kValidShaderSource) - 1,
        "vs_main",
        7,
        "vs_4_0",
        6,
        &log,
        &vertex_shader);
    if (!expect(result, ZEN_RESULT_BACKEND_ERROR) || vertex_shader.value != 777 || log.message[0] != '\0') {
        zen_native_backend_destroy(backend);
        return 5;
    }

    ZenPixelShaderHandle pixel_shader{};
    result = zen_native_backend_destroy_vertex_shader(backend, vertex_shader);
    if (!expect(result, ZEN_RESULT_NOT_INITIALIZED)) {
        zen_native_backend_destroy(backend);
        return 6;
    }

    result = zen_native_backend_destroy_pixel_shader(backend, pixel_shader);
    if (!expect(result, ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 7;
    }

    ZenVertexInputLayoutDesc layout{};
    layout.size = ZEN_VERTEX_INPUT_LAYOUT_DESC_SIZE;
    layout.api_version = ZEN_VERTEX_INPUT_LAYOUT_DESC_API_VERSION;
    layout.element_count = 0;
    ZenRenderTriangleHandle triangle{ 999 };
    result = zen_native_backend_create_triangle_with_shaders(
        backend,
        ZenVertexShaderHandle{ 1 },
        ZenPixelShaderHandle{ 1 },
        &layout,
        &triangle);
    if (!expect(result, ZEN_RESULT_INVALID_ARGUMENT) || triangle.value != 999) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    layout.element_count = 1;
    layout.elements[0].semantic = ZEN_VERTEX_INPUT_SEMANTIC_POSITION;
    layout.elements[0].format = ZEN_VERTEX_INPUT_FORMAT_FLOAT3;
    result = zen_native_backend_create_triangle_with_shaders(
        backend,
        ZenVertexShaderHandle{},
        ZenPixelShaderHandle{ 1 },
        &layout,
        &triangle);
    if (!expect(result, ZEN_RESULT_INVALID_ARGUMENT) || triangle.value != 999) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    result = zen_native_backend_destroy(backend);
    return result == ZEN_RESULT_OK ? 0 : 10;
}

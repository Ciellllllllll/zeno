#include "native_backend.h"

#include <cstdint>

using namespace zeno::native;

namespace {

bool check(bool condition)
{
    return condition;
}

bool capabilities_equal(const RendererBackendCapabilities& left, const RendererBackendCapabilities& right)
{
    return left.kind == right.kind
        && left.support == right.support
        && left.implemented == right.implemented
        && left.default_backend == right.default_backend
        && left.supports_window_present == right.supports_window_present
        && left.supports_textures == right.supports_textures
        && left.supports_meshes == right.supports_meshes
        && left.supports_materials == right.supports_materials
        && left.supports_debug_draw == right.supports_debug_draw
        && left.supports_debug_text == right.supports_debug_text
        && left.supports_resize == right.supports_resize
        && left.max_vertex_input_elements == right.max_vertex_input_elements;
}

} // namespace

int main()
{
    auto renderer = create_renderer_backend(RendererBackendKind::directx11);
    if (!check(renderer != nullptr)) {
        return 1;
    }

    if (!check(renderer->kind() == RendererBackendKind::directx11)) {
        return 1;
    }

    const auto expected_capabilities = renderer_backend_capabilities(RendererBackendKind::directx11);
    if (!check(capabilities_equal(renderer->capabilities(), expected_capabilities))) {
        return 1;
    }

    if (!check(!renderer->begin_frame())) {
        return 1;
    }

    if (!check(!renderer->clear(0.1f, 0.2f, 0.3f, 1.0f))) {
        return 1;
    }

    std::uint64_t output_handle = 777;
    if (!check(!renderer->create_clear_color(0.1f, 0.2f, 0.3f, 1.0f, output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    if (!check(renderer->clear_with_resource(1) == RenderCommandResult::missing_resource)) {
        return 1;
    }

    output_handle = 777;
    if (!check(!renderer->create_triangle(output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    VertexInputLayoutDesc input_layout_desc{};
    output_handle = 777;
    if (!check(!renderer->create_triangle_with_shaders(1, 2, input_layout_desc, output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    const char shader_source[] = "void main() {}";
    const char shader_entry[] = "main";
    const char vertex_shader_profile[] = "vs_4_0";
    const char pixel_shader_profile[] = "ps_4_0";
    ShaderCompileLog compile_log{};

    output_handle = 777;
    if (!check(!renderer->create_vertex_shader_from_source(
            shader_source,
            sizeof(shader_source) - 1,
            shader_entry,
            sizeof(shader_entry) - 1,
            vertex_shader_profile,
            sizeof(vertex_shader_profile) - 1,
            compile_log,
            output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    output_handle = 777;
    if (!check(!renderer->create_pixel_shader_from_source(
            shader_source,
            sizeof(shader_source) - 1,
            shader_entry,
            sizeof(shader_entry) - 1,
            pixel_shader_profile,
            sizeof(pixel_shader_profile) - 1,
            compile_log,
            output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    const std::uint8_t image_bytes[] = { 0 };
    output_handle = 777;
    if (!check(!renderer->create_texture_from_memory(image_bytes, sizeof(image_bytes), output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    MeshDesc mesh_desc{};
    output_handle = 777;
    if (!check(!renderer->create_mesh(mesh_desc, output_handle))) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    MaterialDesc material_desc{};
    output_handle = 777;
    if (!check(renderer->create_material(material_desc, output_handle) == RenderCommandResult::wrong_state)) {
        return 1;
    }
    if (!check(output_handle == 777)) {
        return 1;
    }

    DebugLineDesc debug_line_desc{};
    debug_line_desc.end[0] = 1.0f;
    if (!check(renderer->draw_debug_line(debug_line_desc) == RenderCommandResult::wrong_state)) {
        return 1;
    }

    if (!check(!renderer->present())) {
        return 1;
    }

    auto directx12_backend = create_renderer_backend(RendererBackendKind::directx12);
    if (!check(directx12_backend == nullptr)) {
        return 1;
    }

    renderer->shutdown();
    return 0;
}

#include "native_backend.h"

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
    NativeBackend backend;
    const auto default_capabilities = renderer_backend_capabilities(kDefaultRendererBackendKind);

    if (!check(!backend.is_initialized())) {
        return 1;
    }
    if (!check(!backend.has_window())) {
        return 1;
    }
    if (!check(!backend.has_renderer())) {
        return 1;
    }
    if (!check(backend.renderer_backend_kind() == RendererBackendKind::directx11)) {
        return 1;
    }
    if (!check(capabilities_equal(backend.renderer_capabilities(), default_capabilities))) {
        return 1;
    }

    if (!check(!backend.begin_frame())) {
        return 1;
    }
    if (!check(!backend.clear(0.0f, 0.0f, 0.0f, 1.0f))) {
        return 1;
    }

    std::uint64_t handle = 123;
    if (!check(!backend.create_clear_color(1.0f, 0.0f, 0.0f, 1.0f, handle))) {
        return 1;
    }
    if (!check(handle == 123)) {
        return 1;
    }
    if (!check(backend.clear_with_resource(1) == RenderCommandResult::wrong_state)) {
        return 1;
    }
    if (!check(!backend.create_triangle(handle))) {
        return 1;
    }
    if (!check(handle == 123)) {
        return 1;
    }

    SpriteDrawDesc sprite_desc{};
    if (!check(backend.draw_sprite(1, sprite_desc) == RenderCommandResult::wrong_state)) {
        return 1;
    }

    DebugLineDesc line_desc{};
    if (!check(backend.draw_debug_line(line_desc) == RenderCommandResult::wrong_state)) {
        return 1;
    }

    if (!check(!backend.present())) {
        return 1;
    }

    if (!check(backend.initialize(NativeBackendConfig{}))) {
        return 1;
    }
    if (!check(backend.is_initialized())) {
        return 1;
    }
    if (!check(!backend.has_window())) {
        return 1;
    }
    if (!check(!backend.has_renderer())) {
        return 1;
    }

    const auto before_failed_initialize = backend.renderer_capabilities();
    if (!check(!backend.initialize_renderer())) {
        return 1;
    }
    if (!check(!backend.has_renderer())) {
        return 1;
    }
    if (!check(capabilities_equal(backend.renderer_capabilities(), before_failed_initialize))) {
        return 1;
    }
    if (!check(capabilities_equal(backend.renderer_capabilities(), default_capabilities))) {
        return 1;
    }

    backend.shutdown();
    if (!check(!backend.is_initialized())) {
        return 1;
    }
    if (!check(!backend.has_renderer())) {
        return 1;
    }
    if (!check(capabilities_equal(backend.renderer_capabilities(), default_capabilities))) {
        return 1;
    }

    return 0;
}

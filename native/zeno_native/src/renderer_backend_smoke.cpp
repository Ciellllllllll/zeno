#include "native_backend.h"

#include <cstring>

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
    static_assert(kDefaultRendererBackendKind == RendererBackendKind::directx11);
    static_assert(RendererBackendKind::directx11 != RendererBackendKind::directx12);

    if (!check(std::strcmp(renderer_backend_name(RendererBackendKind::directx11), "directx11") == 0)) {
        return 1;
    }

    if (!check(std::strcmp(renderer_backend_name(RendererBackendKind::directx12), "directx12") == 0)) {
        return 1;
    }

    if (!check(std::strcmp(renderer_backend_name(static_cast<RendererBackendKind>(999)), "unknown") == 0)) {
        return 1;
    }

    const auto directx11_capabilities = renderer_backend_capabilities(RendererBackendKind::directx11);
    if (!check(directx11_capabilities.kind == RendererBackendKind::directx11)) {
        return 1;
    }
    if (!check(directx11_capabilities.support == RendererBackendSupportStatus::supported)) {
        return 1;
    }
    if (!check(directx11_capabilities.implemented)) {
        return 1;
    }
    if (!check(directx11_capabilities.default_backend)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_window_present)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_textures)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_meshes)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_materials)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_debug_draw)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_debug_text)) {
        return 1;
    }
    if (!check(directx11_capabilities.supports_resize)) {
        return 1;
    }
    if (!check(directx11_capabilities.max_vertex_input_elements == 8)) {
        return 1;
    }

    const auto directx12_capabilities = renderer_backend_capabilities(RendererBackendKind::directx12);
    if (!check(directx12_capabilities.kind == RendererBackendKind::directx12)) {
        return 1;
    }
    if (!check(directx12_capabilities.support == RendererBackendSupportStatus::experimental)) {
        return 1;
    }
    if (!check(!directx12_capabilities.implemented)) {
        return 1;
    }
    if (!check(!directx12_capabilities.default_backend)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_window_present)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_textures)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_meshes)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_materials)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_debug_draw)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_debug_text)) {
        return 1;
    }
    if (!check(!directx12_capabilities.supports_resize)) {
        return 1;
    }
    if (!check(directx12_capabilities.max_vertex_input_elements == 0)) {
        return 1;
    }

    auto directx11_backend = create_renderer_backend(RendererBackendKind::directx11);
    if (!check(directx11_backend != nullptr)) {
        return 1;
    }
    if (!check(directx11_backend->kind() == RendererBackendKind::directx11)) {
        return 1;
    }
    if (!check(capabilities_equal(directx11_backend->capabilities(), directx11_capabilities))) {
        return 1;
    }

    auto directx12_backend = create_renderer_backend(RendererBackendKind::directx12);
    if (!check(directx12_backend == nullptr)) {
        return 1;
    }

    return 0;
}

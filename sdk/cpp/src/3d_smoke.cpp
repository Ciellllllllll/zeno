#include <zeno/zeno.hpp>

#include <array>
#include <cstdint>
#include <string>

namespace {

bool invalid_argument(const zeno::Result& result)
{
    return result.native_code() == ZEN_RESULT_INVALID_ARGUMENT;
}

bool contains(const std::string& text, const char* needle)
{
    return text.find(needle) != std::string::npos;
}

zeno::MaterialDesc mesh_material_desc()
{
    zeno::MaterialDesc desc{};
    desc.kind = zeno::MaterialKind::mesh_color;
    desc.blend_mode = zeno::BlendMode::opaque;
    desc.depth_mode = zeno::DepthMode::enabled;
    desc.cull_mode = zeno::CullMode::back;
    return desc;
}

} // namespace

int main()
{
    zeno::clear_last_diagnostic();

    zeno::NativeBackend invalid_backend;
    zeno::Mesh mesh;
    constexpr std::array<zeno::MeshVertex, 3> vertices{ {
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
    } };
    constexpr std::array<std::uint32_t, 3> indices{ 0, 1, 2 };

    zeno::Result result = invalid_backend.create_mesh(
        vertices.data(),
        static_cast<std::uint32_t>(vertices.size()),
        indices.data(),
        static_cast<std::uint32_t>(indices.size()),
        mesh);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_create_mesh failed")) {
        return 1;
    }

    zeno::Material material;
    result = invalid_backend.create_material(mesh_material_desc(), material);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_create_material failed")) {
        return 2;
    }

    zeno::Camera camera = zeno::Camera::perspective(1.0471976f, 640.0f / 360.0f, 0.1f, 10.0f);
    result = invalid_backend.set_camera_matrix(camera.view_projection());
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_set_camera_matrix failed")) {
        return 3;
    }

    zeno::Transform transform{};
    result = invalid_backend.draw_mesh(mesh, transform);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_draw_mesh failed")) {
        return 4;
    }

    result = invalid_backend.draw_mesh(mesh, material, transform);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_draw_mesh_with_material failed")) {
        return 5;
    }

    result = invalid_backend.draw_debug_line(
        zeno::Vec3{ -0.5f, 0.0f, 1.0f },
        zeno::Vec3{ 0.5f, 0.0f, 1.0f },
        zeno::Color{ 0.2f, 0.9f, 1.0f, 1.0f });
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_draw_debug_line failed")) {
        return 6;
    }

    zeno::ResourceManager resources;
    zeno::Scene scene;
    const zeno::ObjectId cube = scene.create_object();
    result = scene.set_mesh_renderer(cube, zeno::MeshRenderer{ zeno::MeshId{ 11 }, zeno::MaterialId{ 22 } });
    if (!result.ok() || scene.renderable_kind(cube) != zeno::RenderableKind::mesh) {
        return 7;
    }

    zeno::NativeBackend backend;
    result = zeno::NativeBackend::create(backend);
    if (!result.ok() || !backend.valid()) {
        return 8;
    }

    result = scene.render(backend, resources);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "scene: mesh or material ID is missing, stale, or invalid")) {
        return 9;
    }

    zeno::MeshId out_mesh{ 123 };
    result = resources.create_mesh(
        invalid_backend,
        vertices.data(),
        static_cast<std::uint32_t>(vertices.size()),
        indices.data(),
        static_cast<std::uint32_t>(indices.size()),
        out_mesh);
    if (!invalid_argument(result)
        || out_mesh.value != 123
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_create_mesh failed")) {
        return 10;
    }

    zeno::MaterialId out_material{ 456 };
    result = resources.create_material(invalid_backend, mesh_material_desc(), out_material);
    if (!invalid_argument(result)
        || out_material.value != 456
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_create_material failed")) {
        return 11;
    }

    return 0;
}

#include "sample_module.h"

#include <iostream>
#include <string>

namespace {

constexpr double kDemoDurationSeconds = 4.0;
constexpr double kColorCycleSeconds = 2.0;
constexpr float kMouseColorScale = 1.0f / 640.0f;

double g_elapsed_seconds = 0.0;
float g_keyboard_tint = 0.0f;
zeno::RenderTriangle g_triangle;
zeno::VertexShader g_vertex_shader;
zeno::PixelShader g_pixel_shader;
zeno::Texture g_sprite_texture;
zeno::Mesh g_cube_mesh;
zeno::Material g_sprite_material;
zeno::Material g_cube_material;
zeno::Scene g_scene;
zeno::ObjectId g_cube_object;
zeno::ObjectId g_triangle_object;
zeno::ObjectId g_sprite_object;
zeno::Camera g_camera;

void log_shader_failure(
    const char* stage,
    const char* asset,
    const char* entry,
    const zeno::Result& result,
    const zeno::ShaderCompileLog& log)
{
    std::cerr << "[ZENO][sample][shader] compile failed\n";
    std::cerr << "  stage: " << stage << "\n";
    std::cerr << "  asset: " << asset << "\n";
    std::cerr << "  entry: " << entry << "\n";
    std::cerr << "  result: " << result.message() << "\n";
    if (!log.message.empty()) {
        std::cerr << "  compiler:\n" << log.message << "\n";
    }
}

zeno::Result on_init(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    if (context.assets == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds = 0.0;
    g_keyboard_tint = 0.0f;
    g_scene.clear();
    g_cube_object = {};
    g_triangle_object = {};
    g_sprite_object = {};
    g_camera = zeno::Camera::perspective(1.0471976f, 640.0f / 360.0f, 0.1f, 10.0f);
    std::string manifest;
    zeno::Result result = context.assets->read_text("sample_manifest.txt", manifest);
    if (!result.ok()) {
        return result;
    }

    std::cerr << "[ZENO][sample] game init\n";
    std::cerr << "[ZENO][sample] asset manifest bytes: " << manifest.size() << "\n";
    result = context.backend->set_camera_matrix(g_camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    zeno::ShaderCompileLog vertex_log;
    constexpr const char* shader_asset = "shaders/sample_triangle.hlsl";
    result = context.backend->create_vertex_shader(*context.assets, shader_asset, "vs_main", g_vertex_shader, vertex_log);
    if (!result.ok()) {
        log_shader_failure("vertex", shader_asset, "vs_main", result, vertex_log);
        return result;
    }

    zeno::ShaderCompileLog pixel_log;
    result = context.backend->create_pixel_shader(*context.assets, shader_asset, "ps_main", g_pixel_shader, pixel_log);
    if (!result.ok()) {
        log_shader_failure("pixel", shader_asset, "ps_main", result, pixel_log);
        return result;
    }

    result = context.backend->create_texture(*context.assets, "textures/sample_sprite_2x2.bmp", g_sprite_texture);
    if (!result.ok()) {
        return result;
    }

    zeno::MaterialDesc sprite_material_desc{};
    sprite_material_desc.kind = zeno::MaterialKind::sprite_texture;
    sprite_material_desc.blend_mode = zeno::BlendMode::alpha;
    sprite_material_desc.depth_mode = zeno::DepthMode::disabled;
    sprite_material_desc.cull_mode = zeno::CullMode::none;
    result = context.backend->create_sprite_material(g_sprite_texture, sprite_material_desc, g_sprite_material);
    if (!result.ok()) {
        return result;
    }

    constexpr zeno::MeshVertex cube_vertices[] = {
        { { -0.5f, -0.5f, -0.5f }, { 0.95f, 0.20f, 0.20f, 1.0f } },
        { { -0.5f, 0.5f, -0.5f }, { 0.95f, 0.55f, 0.20f, 1.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 0.95f, 0.90f, 0.20f, 1.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 0.35f, 0.85f, 0.35f, 1.0f } },
        { { -0.5f, -0.5f, 0.5f }, { 0.20f, 0.80f, 0.95f, 1.0f } },
        { { -0.5f, 0.5f, 0.5f }, { 0.30f, 0.45f, 0.95f, 1.0f } },
        { { 0.5f, 0.5f, 0.5f }, { 0.70f, 0.35f, 0.95f, 1.0f } },
        { { 0.5f, -0.5f, 0.5f }, { 0.95f, 0.35f, 0.70f, 1.0f } },
    };
    constexpr std::uint32_t cube_indices[] = {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7,
    };

    result = context.backend->create_mesh(cube_vertices, 8, cube_indices, 36, g_cube_mesh);
    if (!result.ok()) {
        return result;
    }

    zeno::MaterialDesc cube_material_desc{};
    cube_material_desc.kind = zeno::MaterialKind::mesh_color;
    cube_material_desc.blend_mode = zeno::BlendMode::opaque;
    cube_material_desc.depth_mode = zeno::DepthMode::enabled;
    cube_material_desc.cull_mode = zeno::CullMode::back;
    result = context.backend->create_material(cube_material_desc, g_cube_material);
    if (!result.ok()) {
        return result;
    }

    result = context.backend->create_triangle(g_vertex_shader, g_pixel_shader, g_triangle);
    if (!result.ok()) {
        return result;
    }

    zeno::Transform cube_transform{};
    cube_transform.position = zeno::Vec3{ 0.0f, 0.0f, 3.0f };
    cube_transform.scale = zeno::Vec3{ 0.7f, 0.7f, 0.7f };
    g_cube_object = g_scene.create_object();
    result = g_scene.set_transform(g_cube_object, cube_transform);
    if (!result.ok()) {
        return result;
    }
    result = g_scene.set_mesh_renderer(g_cube_object, zeno::MeshRenderer{ &g_cube_mesh, &g_cube_material });
    if (!result.ok()) {
        return result;
    }

    zeno::Transform triangle_transform{};
    triangle_transform.position = zeno::Vec3{ 0.0f, 0.0f, 2.0f };
    triangle_transform.scale = zeno::Vec3{ 0.75f, 0.75f, 1.0f };
    g_triangle_object = g_scene.create_object();
    result = g_scene.set_transform(g_triangle_object, triangle_transform);
    if (!result.ok()) {
        return result;
    }
    result = g_scene.set_triangle_renderer(g_triangle_object, zeno::TriangleRenderer{ &g_triangle });
    if (!result.ok()) {
        return result;
    }

    zeno::Transform sprite_transform{};
    sprite_transform.position = zeno::Vec3{ -0.45f, -0.35f, 1.7f };
    sprite_transform.scale = zeno::Vec3{ 0.42f, 0.42f, 1.0f };
    g_sprite_object = g_scene.create_object();
    result = g_scene.set_transform(g_sprite_object, sprite_transform);
    if (!result.ok()) {
        return result;
    }
    result = g_scene.set_sprite_renderer(
        g_sprite_object,
        zeno::SpriteRenderer{ &g_sprite_material, zeno::Color{ 1.0f, 1.0f, 1.0f, 0.85f } });
    if (!result.ok()) {
        return result;
    }

    return zeno::Result();
}

zeno::Result on_update(zeno::GameContext& context)
{
    g_elapsed_seconds += context.delta_time_seconds;
    if (context.input.pressed(zeno::Key::escape)) {
        context.should_close = true;
        return zeno::Result();
    }

    if (context.input.down(zeno::Key::a) || context.input.down(zeno::Key::left)) {
        g_keyboard_tint -= static_cast<float>(context.delta_time_seconds);
    }

    if (context.input.down(zeno::Key::d) || context.input.down(zeno::Key::right)) {
        g_keyboard_tint += static_cast<float>(context.delta_time_seconds);
    }

    if (g_keyboard_tint < -0.25f) {
        g_keyboard_tint = -0.25f;
    }
    if (g_keyboard_tint > 0.25f) {
        g_keyboard_tint = 0.25f;
    }

    zeno::Transform* triangle_transform = g_scene.transform(g_triangle_object);
    zeno::Transform* sprite_transform = g_scene.transform(g_sprite_object);
    zeno::Transform* cube_transform = g_scene.transform(g_cube_object);
    if (triangle_transform == nullptr || sprite_transform == nullptr || cube_transform == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    triangle_transform->rotation_z_radians = static_cast<float>(g_elapsed_seconds);
    triangle_transform->position.x = 0.25f * g_keyboard_tint;
    sprite_transform->rotation_z_radians = -0.5f * static_cast<float>(g_elapsed_seconds);
    cube_transform->rotation_z_radians = 0.7f * static_cast<float>(g_elapsed_seconds);

    context.should_close = g_elapsed_seconds >= kDemoDurationSeconds;
    return zeno::Result();
}

zeno::Result on_render(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    const double cycle_position = g_elapsed_seconds / kColorCycleSeconds;
    const float t = static_cast<float>(cycle_position - static_cast<int>(cycle_position));
    const float mouse_t = static_cast<float>(context.input.mouse_x) * kMouseColorScale;
    zeno::Result result = context.backend->begin_frame();
    if (!result.ok()) {
        return result;
    }

    const zeno::Color color{
        0.05f + 0.25f * t + g_keyboard_tint,
        0.12f,
        0.20f + 0.20f * (1.0f - t) + 0.20f * mouse_t,
        1.0f,
    };

    result = context.backend->clear(color);
    if (!result.ok()) {
        return result;
    }

    result = context.backend->set_camera_matrix(g_camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    result = g_scene.render(*context.backend);
    if (!result.ok()) {
        return result;
    }

    return context.backend->present();
}

zeno::Result on_shutdown(zeno::GameContext&)
{
    g_scene.clear();
    g_cube_object = {};
    g_triangle_object = {};
    g_sprite_object = {};
    g_triangle.reset();
    g_cube_material.reset();
    g_sprite_material.reset();
    g_cube_mesh.reset();
    g_sprite_texture.reset();
    g_pixel_shader.reset();
    g_vertex_shader.reset();
    std::cerr << "[ZENO][sample] game shutdown\n";
    return zeno::Result();
}

} // namespace

zeno::GameModule create_sample_game_module()
{
    zeno::GameModule module{};
    module.on_init = on_init;
    module.on_update = on_update;
    module.on_render = on_render;
    module.on_shutdown = on_shutdown;
    return module;
}

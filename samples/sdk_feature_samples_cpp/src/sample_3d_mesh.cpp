#include <zeno/game_module.hpp>

#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <string_view>

namespace {

constexpr double kSampleDurationSeconds = 12.0;

double g_elapsed_seconds = 0.0;
zeno::MeshId g_mesh;
zeno::MaterialId g_material;
zeno::ObjectId g_cube;
zeno::Camera g_camera;

const zeno::SceneObjectDesc* find_scene_object(
    const zeno::SceneDescription& scene,
    std::string_view name,
    zeno::RenderableKind kind)
{
    for (const zeno::SceneObjectDesc& object : scene.objects) {
        if (object.name == name && object.renderable_kind == kind) {
            return &object;
        }
    }

    return nullptr;
}

zeno::Result reset_state(zeno::GameContext& context)
{
    if (context.runtime_scene == nullptr || context.resources == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    context.runtime_scene->clear();
    context.resources->clear();
    g_elapsed_seconds = 0.0;
    g_mesh = {};
    g_material = {};
    g_cube = {};
    g_camera = {};
    return zeno::Result();
}

zeno::Result on_init(zeno::GameContext& context)
{
    if (context.backend == nullptr || context.resources == nullptr
        || context.runtime_scene == nullptr || context.scene == nullptr || context.project == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Result result = reset_state(context);
    if (!result.ok()) {
        return result;
    }

    const zeno::SceneObjectDesc* cube = find_scene_object(*context.scene, "cube", zeno::RenderableKind::mesh);
    if (cube == nullptr || cube->reference != "builtin:cube") {
        zeno::log_message(
            zeno::LogLevel::error,
            "sample",
            "sample: 3D mesh sample requires scene object 'cube' with reference 'builtin:cube'");
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    const float aspect_ratio = context.project->window_height != 0
        ? static_cast<float>(context.project->window_width) / static_cast<float>(context.project->window_height)
        : 640.0f / 360.0f;
    g_camera = zeno::Camera::perspective(1.0471976f, aspect_ratio, 0.1f, 10.0f);

    constexpr std::array<zeno::MeshVertex, 8> vertices{ {
        { { -0.5f, -0.5f, -0.5f }, { 0.95f, 0.20f, 0.20f, 1.0f } },
        { { -0.5f, 0.5f, -0.5f }, { 0.95f, 0.55f, 0.20f, 1.0f } },
        { { 0.5f, 0.5f, -0.5f }, { 0.95f, 0.90f, 0.20f, 1.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 0.35f, 0.85f, 0.35f, 1.0f } },
        { { -0.5f, -0.5f, 0.5f }, { 0.20f, 0.80f, 0.95f, 1.0f } },
        { { -0.5f, 0.5f, 0.5f }, { 0.30f, 0.45f, 0.95f, 1.0f } },
        { { 0.5f, 0.5f, 0.5f }, { 0.70f, 0.35f, 0.95f, 1.0f } },
        { { 0.5f, -0.5f, 0.5f }, { 0.95f, 0.35f, 0.70f, 1.0f } },
    } };
    constexpr std::array<std::uint32_t, 36> indices{
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7,
    };

    result = context.resources->create_mesh(
        *context.backend,
        vertices.data(),
        static_cast<std::uint32_t>(vertices.size()),
        indices.data(),
        static_cast<std::uint32_t>(indices.size()),
        g_mesh);
    if (!result.ok()) {
        return result;
    }

    zeno::MaterialDesc material_desc{};
    material_desc.kind = zeno::MaterialKind::mesh_color;
    material_desc.blend_mode = zeno::BlendMode::opaque;
    material_desc.depth_mode = zeno::DepthMode::enabled;
    material_desc.cull_mode = zeno::CullMode::back;
    result = context.resources->create_material(*context.backend, material_desc, g_material);
    if (!result.ok()) {
        return result;
    }

    g_cube = context.runtime_scene->create_object();
    result = context.runtime_scene->set_transform(g_cube, cube->transform);
    if (!result.ok()) {
        return result;
    }
    result = context.runtime_scene->set_mesh_renderer(g_cube, zeno::MeshRenderer{ g_mesh, g_material });
    if (!result.ok()) {
        return result;
    }

    std::cerr << "[ZENO][3d-sample] Escape exits; mesh/material/camera are SDK-owned\n";
    return zeno::Result();
}

zeno::Result on_update(zeno::GameContext& context)
{
    if (context.runtime_scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds += context.delta_time_seconds;
    if (context.input.pressed(zeno::Key::escape)) {
        context.should_close = true;
        return zeno::Result();
    }

    zeno::Transform* transform = context.runtime_scene->transform(g_cube);
    if (transform == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    transform->rotation_z_radians = static_cast<float>(g_elapsed_seconds) * 0.9f;
    const float scale = 0.68f + 0.08f * static_cast<float>(std::sin(g_elapsed_seconds * 2.0));
    transform->scale = zeno::Vec3{ scale, scale, scale };
    context.should_close = g_elapsed_seconds >= kSampleDurationSeconds;
    return zeno::Result();
}

zeno::Result on_render(zeno::GameContext& context)
{
    if (context.backend == nullptr || context.runtime_scene == nullptr || context.resources == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Result result = context.backend->begin_frame();
    if (!result.ok()) {
        return result;
    }

    result = context.backend->clear(zeno::Color{ 0.025f, 0.035f, 0.055f, 1.0f });
    if (!result.ok()) {
        return result;
    }

    result = context.backend->set_camera_matrix(g_camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    result = context.runtime_scene->render(*context.backend, *context.resources);
    if (!result.ok()) {
        return result;
    }

    result = context.backend->draw_debug_line(
        zeno::Vec3{ -0.85f, -0.55f, 2.0f },
        zeno::Vec3{ 0.85f, 0.55f, 2.0f },
        zeno::Color{ 0.20f, 0.95f, 1.0f, 1.0f });
    if (!result.ok()) {
        return result;
    }

    const std::string overlay =
        "SDK 3D MESH"
        "\nFRAME: " + std::to_string(context.frame_index)
        + "\nMESH: RESOURCE MANAGER"
        + "\nCAMERA: PERSPECTIVE";
    result = context.backend->draw_debug_text(
        overlay,
        zeno::Vec3{ -0.96f, 0.92f, 0.0f },
        0.0065f,
        zeno::Color{ 0.85f, 0.95f, 1.0f, 1.0f });
    if (!result.ok()) {
        return result;
    }

    return context.backend->present();
}

zeno::Result on_shutdown(zeno::GameContext& context)
{
    return reset_state(context);
}

} // namespace

int main()
{
    zeno::GameModule module{};
    module.on_init = on_init;
    module.on_update = on_update;
    module.on_render = on_render;
    module.on_shutdown = on_shutdown;

    zeno::GameAppConfig config{};
    config.project_path = "projects/3d_mesh.zproj";

    zeno::GameApp app;
    zeno::Result result = app.run(module, config);
    if (!result.ok()) {
        std::cerr << "3D SDK sample failed: " << result.message() << "\n";
        return 1;
    }

    return 0;
}

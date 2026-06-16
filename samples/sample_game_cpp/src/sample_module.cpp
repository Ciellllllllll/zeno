#include "sample_module.h"

#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <string_view>

namespace {

constexpr double kDemoDurationSeconds = 18.0;
constexpr double kColorCycleSeconds = 2.0;
constexpr float kMouseColorScale = 1.0f / 640.0f;
constexpr float kPlayerMoveSpeed = 0.55f;
constexpr int kWinningScore = 3;
constexpr zeno::Vec3 kPlayerStart{ -0.75f, -0.35f, 1.7f };
constexpr std::array<zeno::Vec3, kWinningScore> kGoalPositions{
    zeno::Vec3{ 0.55f, 0.35f, 2.0f },
    zeno::Vec3{ -0.55f, 0.35f, 2.0f },
    zeno::Vec3{ 0.65f, -0.30f, 2.0f },
};

double g_elapsed_seconds = 0.0;
float g_keyboard_tint = 0.0f;
bool g_debug_draw_enabled = true;
bool g_player_touching_goal = false;
bool g_player_touching_hazard = false;
bool g_game_won = false;
int g_score = 0;
int g_goal_index = 0;
zeno::VertexShader g_vertex_shader;
zeno::PixelShader g_pixel_shader;
zeno::TextureId g_sprite_texture;
zeno::MeshId g_cube_mesh;
zeno::MaterialId g_sprite_material;
zeno::MaterialId g_cube_material;
zeno::SoundId g_event_sound;
zeno::TriangleId g_triangle;
zeno::ObjectId g_cube_object;
zeno::ObjectId g_triangle_object;
zeno::ObjectId g_sprite_object;
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

zeno::Aabb2 bounds_for_object(const zeno::Scene& scene, zeno::ObjectId object)
{
    const zeno::Transform* transform = scene.transform(object);
    if (transform == nullptr) {
        return {};
    }

    return zeno::aabb_from_transform_2d(*transform);
}

zeno::Result reset_play_state(zeno::Scene& scene)
{
    zeno::Transform* player_transform = scene.transform(g_sprite_object);
    zeno::Transform* goal_transform = scene.transform(g_triangle_object);
    zeno::Transform* hazard_transform = scene.transform(g_cube_object);
    if (player_transform == nullptr || goal_transform == nullptr || hazard_transform == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds = 0.0;
    g_keyboard_tint = 0.0f;
    g_debug_draw_enabled = true;
    g_player_touching_goal = false;
    g_player_touching_hazard = false;
    g_game_won = false;
    g_score = 0;
    g_goal_index = 0;

    player_transform->position = kPlayerStart;
    player_transform->rotation_z_radians = 0.0f;
    goal_transform->position = kGoalPositions[0];
    goal_transform->rotation_z_radians = 0.0f;
    goal_transform->scale = zeno::Vec3{ 0.35f, 0.35f, 1.0f };
    hazard_transform->scale = zeno::Vec3{ 0.48f, 0.48f, 0.48f };

    return scene.set_sprite_renderer(
        g_sprite_object,
        zeno::SpriteRenderer{ g_sprite_material, zeno::Color{ 1.0f, 1.0f, 1.0f, 0.85f } });
}

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
    if (context.audio == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    if (context.resources == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    if (context.runtime_scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    if (context.scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds = 0.0;
    g_keyboard_tint = 0.0f;
    g_debug_draw_enabled = true;
    g_player_touching_goal = false;
    g_player_touching_hazard = false;
    g_game_won = false;
    g_score = 0;
    g_goal_index = 0;
    context.runtime_scene->clear();
    g_cube_object = {};
    g_triangle_object = {};
    g_sprite_object = {};
    context.resources->clear();
    g_sprite_texture = {};
    g_cube_mesh = {};
    g_sprite_material = {};
    g_cube_material = {};
    g_event_sound = {};
    g_triangle = {};
    const zeno::SceneObjectDesc* cube_object = find_scene_object(*context.scene, "cube", zeno::RenderableKind::mesh);
    const zeno::SceneObjectDesc* triangle_object = find_scene_object(*context.scene, "triangle", zeno::RenderableKind::triangle);
    const zeno::SceneObjectDesc* sprite_object = find_scene_object(*context.scene, "sprite", zeno::RenderableKind::sprite);
    if (cube_object == nullptr || triangle_object == nullptr || sprite_object == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    if (cube_object->reference != "builtin:cube" || triangle_object->reference != "builtin:triangle") {
        return zeno::Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    const float aspect_ratio = context.project != nullptr && context.project->window_height != 0
        ? static_cast<float>(context.project->window_width) / static_cast<float>(context.project->window_height)
        : 640.0f / 360.0f;
    g_camera = zeno::Camera::perspective(1.0471976f, aspect_ratio, 0.1f, 10.0f);
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

    result = context.resources->create_texture(*context.backend, *context.assets, sprite_object->reference, g_sprite_texture);
    if (!result.ok()) {
        return result;
    }

    zeno::MaterialDesc sprite_material_desc{};
    sprite_material_desc.kind = zeno::MaterialKind::sprite_texture;
    sprite_material_desc.blend_mode = zeno::BlendMode::alpha;
    sprite_material_desc.depth_mode = zeno::DepthMode::disabled;
    sprite_material_desc.cull_mode = zeno::CullMode::none;
    result = context.resources->create_sprite_material(*context.backend, g_sprite_texture, sprite_material_desc, g_sprite_material);
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

    result = context.resources->create_mesh(*context.backend, cube_vertices, 8, cube_indices, 36, g_cube_mesh);
    if (!result.ok()) {
        return result;
    }

    zeno::MaterialDesc cube_material_desc{};
    cube_material_desc.kind = zeno::MaterialKind::mesh_color;
    cube_material_desc.blend_mode = zeno::BlendMode::opaque;
    cube_material_desc.depth_mode = zeno::DepthMode::enabled;
    cube_material_desc.cull_mode = zeno::CullMode::back;
    result = context.resources->create_material(*context.backend, cube_material_desc, g_cube_material);
    if (!result.ok()) {
        return result;
    }

    result = context.resources->load_sound(*context.audio, *context.assets, "audio/sample_click.wav", g_event_sound);
    if (!result.ok()) {
        return result;
    }
    if (zeno::Sound* sound = context.resources->sound(g_event_sound)) {
        result = sound->set_volume(0.35f);
        if (!result.ok()) {
            return result;
        }
    } else {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    result = context.resources->create_triangle(*context.backend, g_vertex_shader, g_pixel_shader, g_triangle);
    if (!result.ok()) {
        return result;
    }

    g_cube_object = context.runtime_scene->create_object();
    result = context.runtime_scene->set_transform(g_cube_object, cube_object->transform);
    if (!result.ok()) {
        return result;
    }
    result = context.runtime_scene->set_mesh_renderer(g_cube_object, zeno::MeshRenderer{ g_cube_mesh, g_cube_material });
    if (!result.ok()) {
        return result;
    }

    g_triangle_object = context.runtime_scene->create_object();
    result = context.runtime_scene->set_transform(g_triangle_object, triangle_object->transform);
    if (!result.ok()) {
        return result;
    }
    result = context.runtime_scene->set_triangle_renderer(g_triangle_object, zeno::TriangleRenderer{ g_triangle });
    if (!result.ok()) {
        return result;
    }

    g_sprite_object = context.runtime_scene->create_object();
    result = context.runtime_scene->set_transform(g_sprite_object, sprite_object->transform);
    if (!result.ok()) {
        return result;
    }
    result = context.runtime_scene->set_sprite_renderer(
        g_sprite_object,
        zeno::SpriteRenderer{ g_sprite_material, sprite_object->color });
    if (!result.ok()) {
        return result;
    }

    result = reset_play_state(*context.runtime_scene);
    if (!result.ok()) {
        return result;
    }

    std::cerr << "[ZENO][sample] controls: WASD/arrows move, Space restarts, Escape exits\n";
    std::cerr << "[ZENO][sample] goal score: 0/" << kWinningScore << "\n";
    return zeno::Result();
}

zeno::Result on_update(zeno::GameContext& context)
{
    g_elapsed_seconds += context.delta_time_seconds;
    if (context.input.pressed(zeno::Key::escape)) {
        context.should_close = true;
        return zeno::Result();
    }
    if (context.input.pressed(zeno::Key::space)) {
        if (context.runtime_scene == nullptr) {
            return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
        }

        zeno::Result result = reset_play_state(*context.runtime_scene);
        if (!result.ok()) {
            return result;
        }

        std::cerr << "[ZENO][sample] restart\n";
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

    if (context.runtime_scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Transform* triangle_transform = context.runtime_scene->transform(g_triangle_object);
    zeno::Transform* sprite_transform = context.runtime_scene->transform(g_sprite_object);
    zeno::Transform* cube_transform = context.runtime_scene->transform(g_cube_object);
    if (triangle_transform == nullptr || sprite_transform == nullptr || cube_transform == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Vec2 player_move{};
    if (context.input.down(zeno::Key::a) || context.input.down(zeno::Key::left)) {
        player_move.x -= 1.0f;
    }
    if (context.input.down(zeno::Key::d) || context.input.down(zeno::Key::right)) {
        player_move.x += 1.0f;
    }
    if (context.input.down(zeno::Key::w) || context.input.down(zeno::Key::up)) {
        player_move.y += 1.0f;
    }
    if (context.input.down(zeno::Key::s) || context.input.down(zeno::Key::down)) {
        player_move.y -= 1.0f;
    }

    sprite_transform->position.x += player_move.x * kPlayerMoveSpeed * static_cast<float>(context.delta_time_seconds);
    sprite_transform->position.y += player_move.y * kPlayerMoveSpeed * static_cast<float>(context.delta_time_seconds);
    if (sprite_transform->position.x < -0.9f) {
        sprite_transform->position.x = -0.9f;
    }
    if (sprite_transform->position.x > 0.9f) {
        sprite_transform->position.x = 0.9f;
    }
    if (sprite_transform->position.y < -0.55f) {
        sprite_transform->position.y = -0.55f;
    }
    if (sprite_transform->position.y > 0.55f) {
        sprite_transform->position.y = 0.55f;
    }

    triangle_transform->rotation_z_radians = static_cast<float>(g_elapsed_seconds) * 1.5f;
    sprite_transform->rotation_z_radians = -0.5f * static_cast<float>(g_elapsed_seconds);
    cube_transform->rotation_z_radians = 0.7f * static_cast<float>(g_elapsed_seconds);
    const float hazard_scale = 0.48f + 0.06f * static_cast<float>(g_score);
    cube_transform->scale = zeno::Vec3{ hazard_scale, hazard_scale, hazard_scale };

    const bool touching_goal = zeno::intersects(
        bounds_for_object(*context.runtime_scene, g_sprite_object),
        bounds_for_object(*context.runtime_scene, g_triangle_object));
    if (touching_goal && !g_player_touching_goal && !g_game_won) {
        ++g_score;
        std::cerr << "[ZENO][sample] goal score: " << g_score << "/" << kWinningScore << "\n";
        if (zeno::Sound* sound = context.resources != nullptr ? context.resources->sound(g_event_sound) : nullptr) {
            zeno::Result result = sound->play();
            if (!result.ok()) {
                return result;
            }
        }

        if (g_score >= kWinningScore) {
            g_game_won = true;
            std::cerr << "[ZENO][sample] goal complete, press Space to restart\n";
        } else {
            g_goal_index = g_score % kWinningScore;
            triangle_transform->position = kGoalPositions[static_cast<std::size_t>(g_goal_index)];
        }
    }
    g_player_touching_goal = touching_goal;

    const bool touching_hazard = zeno::intersects(
        bounds_for_object(*context.runtime_scene, g_sprite_object),
        bounds_for_object(*context.runtime_scene, g_cube_object));
    if (touching_hazard && !g_player_touching_hazard && !g_game_won) {
        sprite_transform->position = kPlayerStart;
        g_keyboard_tint = -0.20f;
        std::cerr << "[ZENO][sample] obstacle hit, player reset\n";
        if (zeno::Sound* sound = context.resources != nullptr ? context.resources->sound(g_event_sound) : nullptr) {
            zeno::Result result = sound->play();
            if (!result.ok()) {
                return result;
            }
        }
    }
    g_player_touching_hazard = touching_hazard;

    const zeno::Color player_color = g_game_won
        ? zeno::Color{ 0.35f, 1.0f, 0.45f, 0.95f }
        : touching_hazard
        ? zeno::Color{ 1.0f, 0.30f, 0.25f, 0.90f }
        : touching_goal
        ? zeno::Color{ 1.0f, 0.85f, 0.20f, 0.90f }
        : zeno::Color{ 1.0f, 1.0f, 1.0f, 0.85f };
    zeno::Result result = context.runtime_scene->set_sprite_renderer(
        g_sprite_object,
        zeno::SpriteRenderer{ g_sprite_material, player_color });
    if (!result.ok()) {
        return result;
    }

    if (g_game_won) {
        triangle_transform->scale = zeno::Vec3{ 0.55f, 0.55f, 1.0f };
    }

    context.should_close = g_elapsed_seconds >= kDemoDurationSeconds;
    return zeno::Result();
}

zeno::Result on_render(zeno::GameContext& context)
{
    if (context.backend == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    if (context.runtime_scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    const double cycle_position = g_elapsed_seconds / kColorCycleSeconds;
    const float t = static_cast<float>(cycle_position - static_cast<int>(cycle_position));
    const float mouse_t = static_cast<float>(context.input.mouse_x) * kMouseColorScale;
    zeno::Result result = context.backend->begin_frame();
    if (!result.ok()) {
        return result;
    }

    const float score_t = static_cast<float>(g_score) / static_cast<float>(kWinningScore);
    const zeno::Color color{
        0.05f + 0.20f * t + 0.12f * score_t + g_keyboard_tint,
        g_game_won ? 0.20f : 0.10f + 0.10f * score_t,
        0.18f + 0.18f * (1.0f - t) + 0.18f * mouse_t,
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

    if (context.resources == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }
    result = context.runtime_scene->render(*context.backend, *context.resources);
    if (!result.ok()) {
        return result;
    }

    if (g_debug_draw_enabled) {
        const zeno::Color player_bounds_color = g_game_won
            ? zeno::Color{ 0.35f, 1.0f, 0.45f, 1.0f }
            : g_player_touching_hazard
            ? zeno::Color{ 1.0f, 0.25f, 0.20f, 1.0f }
            : g_player_touching_goal
            ? zeno::Color{ 1.0f, 0.9f, 0.2f, 1.0f }
            : zeno::Color{ 0.2f, 0.95f, 1.0f, 1.0f };
        result = context.backend->draw_debug_rect_2d(
            bounds_for_object(*context.runtime_scene, g_sprite_object),
            1.6f,
            player_bounds_color);
        if (!result.ok()) {
            return result;
        }
        result = context.backend->draw_debug_rect_2d(
            bounds_for_object(*context.runtime_scene, g_triangle_object),
            1.9f,
            zeno::Color{ 1.0f, 0.35f, 0.35f, 1.0f });
        if (!result.ok()) {
            return result;
        }
        result = context.backend->draw_debug_rect_2d(
            bounds_for_object(*context.runtime_scene, g_cube_object),
            2.8f,
            zeno::Color{ 0.45f, 1.0f, 0.35f, 1.0f });
        if (!result.ok()) {
            return result;
        }
    }

    return context.backend->present();
}

zeno::Result on_shutdown(zeno::GameContext& context)
{
    if (context.runtime_scene != nullptr) {
        context.runtime_scene->clear();
    }
    if (context.resources != nullptr) {
        context.resources->clear();
    }
    g_cube_object = {};
    g_triangle_object = {};
    g_sprite_object = {};
    g_event_sound = {};
    g_triangle = {};
    g_cube_material = {};
    g_sprite_material = {};
    g_cube_mesh = {};
    g_sprite_texture = {};
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

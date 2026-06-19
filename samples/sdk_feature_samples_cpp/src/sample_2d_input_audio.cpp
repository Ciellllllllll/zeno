#include <zeno/game_module.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <string_view>

namespace {

constexpr double kSampleDurationSeconds = 12.0;
constexpr float kMoveSpeed = 0.85f;

double g_elapsed_seconds = 0.0;
bool g_play_requested = false;
zeno::TextureId g_texture;
zeno::MaterialId g_material;
zeno::SoundId g_sound;
zeno::ObjectId g_player;

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
    g_play_requested = false;
    g_texture = {};
    g_material = {};
    g_sound = {};
    g_player = {};
    return zeno::Result();
}

zeno::Result on_init(zeno::GameContext& context)
{
    if (context.backend == nullptr || context.assets == nullptr || context.audio == nullptr
        || context.resources == nullptr || context.runtime_scene == nullptr || context.scene == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Result result = reset_state(context);
    if (!result.ok()) {
        return result;
    }

    const zeno::SceneObjectDesc* player = find_scene_object(*context.scene, "player", zeno::RenderableKind::sprite);
    if (player == nullptr) {
        zeno::log_message(
            zeno::LogLevel::error,
            "2d-sample",
            "2d-sample: scene requires a sprite object named player");
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    std::string manifest;
    result = context.assets->read_text("sample_manifest.txt", manifest);
    if (!result.ok()) {
        return result;
    }

    result = context.resources->create_texture(*context.backend, *context.assets, player->reference, g_texture);
    if (!result.ok()) {
        return result;
    }

    zeno::MaterialDesc material_desc{};
    material_desc.kind = zeno::MaterialKind::sprite_texture;
    material_desc.blend_mode = zeno::BlendMode::alpha;
    material_desc.depth_mode = zeno::DepthMode::disabled;
    material_desc.cull_mode = zeno::CullMode::none;
    result = context.resources->create_sprite_material(*context.backend, g_texture, material_desc, g_material);
    if (!result.ok()) {
        return result;
    }

    result = context.resources->load_sound(*context.audio, *context.assets, "audio/sample_click.wav", g_sound);
    if (!result.ok()) {
        return result;
    }

    if (zeno::Sound* sound = context.resources->sound(g_sound)) {
        result = sound->set_volume(0.25f);
        if (!result.ok()) {
            return result;
        }
    }

    g_player = context.runtime_scene->create_object();
    result = context.runtime_scene->set_transform(g_player, player->transform);
    if (!result.ok()) {
        return result;
    }
    result = context.runtime_scene->set_sprite_renderer(g_player, zeno::SpriteRenderer{ g_material, player->color });
    if (!result.ok()) {
        return result;
    }

    std::cerr << "[ZENO][2d-sample] manifest bytes: " << manifest.size() << "\n";
    std::cerr << "[ZENO][2d-sample] WASD/arrows move, Space or mouse-left plays sound, Escape exits\n";
    return zeno::Result();
}

zeno::Result on_update(zeno::GameContext& context)
{
    if (context.runtime_scene == nullptr || context.resources == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    g_elapsed_seconds += context.delta_time_seconds;
    if (context.input.pressed(zeno::Key::escape)) {
        context.should_close = true;
        return zeno::Result();
    }

    zeno::Transform* transform = context.runtime_scene->transform(g_player);
    if (transform == nullptr) {
        return zeno::Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    zeno::Vec2 move{};
    if (context.input.down(zeno::Key::a) || context.input.down(zeno::Key::left)) {
        move.x -= 1.0f;
    }
    if (context.input.down(zeno::Key::d) || context.input.down(zeno::Key::right)) {
        move.x += 1.0f;
    }
    if (context.input.down(zeno::Key::w) || context.input.down(zeno::Key::up)) {
        move.y += 1.0f;
    }
    if (context.input.down(zeno::Key::s) || context.input.down(zeno::Key::down)) {
        move.y -= 1.0f;
    }

    transform->position.x += move.x * kMoveSpeed * static_cast<float>(context.delta_time_seconds);
    transform->position.y += move.y * kMoveSpeed * static_cast<float>(context.delta_time_seconds);
    transform->rotation_z_radians = std::sin(static_cast<float>(g_elapsed_seconds) * 2.0f) * 0.25f;

    if (transform->position.x < -0.85f) {
        transform->position.x = -0.85f;
    }
    if (transform->position.x > 0.85f) {
        transform->position.x = 0.85f;
    }
    if (transform->position.y < -0.55f) {
        transform->position.y = -0.55f;
    }
    if (transform->position.y > 0.55f) {
        transform->position.y = 0.55f;
    }

    const bool requested_sound = context.input.pressed(zeno::Key::space)
        || context.input.pressed(zeno::MouseButton::left);
    if (requested_sound) {
        if (zeno::Sound* sound = context.resources->sound(g_sound)) {
            zeno::Result result = sound->play();
            if (!result.ok()) {
                return result;
            }
        }
        g_play_requested = true;
    }

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

    const float pulse = static_cast<float>(std::sin(g_elapsed_seconds * 3.0) * 0.5 + 0.5);
    result = context.backend->clear(zeno::Color{ 0.04f, 0.07f + 0.08f * pulse, 0.10f, 1.0f });
    if (!result.ok()) {
        return result;
    }

    const zeno::Camera camera = zeno::Camera::orthographic(2.0f, 1.125f, 0.1f, 10.0f);
    result = context.backend->set_camera_matrix(camera.view_projection());
    if (!result.ok()) {
        return result;
    }

    result = context.runtime_scene->render(*context.backend, *context.resources);
    if (!result.ok()) {
        return result;
    }

    const zeno::Transform* transform = context.runtime_scene->transform(g_player);
    if (transform != nullptr) {
        result = context.backend->draw_debug_rect_2d(
            zeno::aabb_from_transform_2d(*transform),
            0.9f,
            zeno::Color{ 0.20f, 0.95f, 1.0f, 1.0f });
        if (!result.ok()) {
            return result;
        }
    }

    const std::string overlay =
        "SDK 2D INPUT AUDIO"
        "\nFRAME: " + std::to_string(context.frame_index)
        + "\nSOUND: " + (g_play_requested ? "PLAY" : "READY")
        + "\nMOUSE: " + std::to_string(context.input.mouse_x) + "," + std::to_string(context.input.mouse_y);
    result = context.backend->draw_debug_text(
        overlay,
        zeno::Vec3{ -0.96f, 0.92f, 0.0f },
        0.0065f,
        zeno::Color{ 0.95f, 1.0f, 0.75f, 1.0f });
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
    config.project_path = "projects/2d_input_audio.zproj";

    zeno::GameApp app;
    zeno::Result result = app.run(module, config);
    if (!result.ok()) {
        std::cerr << "2D SDK sample failed: " << result.message() << "\n";
        return 1;
    }

    return 0;
}

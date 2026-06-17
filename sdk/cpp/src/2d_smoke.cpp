#include <zeno/zeno.hpp>

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

zeno::MaterialDesc sprite_material_desc()
{
    zeno::MaterialDesc desc{};
    desc.kind = zeno::MaterialKind::sprite_texture;
    desc.blend_mode = zeno::BlendMode::alpha;
    desc.depth_mode = zeno::DepthMode::disabled;
    desc.cull_mode = zeno::CullMode::none;
    return desc;
}

} // namespace

int main()
{
    zeno::clear_last_diagnostic();

    zeno::NativeBackend backend;
    zeno::Result result = zeno::NativeBackend::create(backend);
    if (!result.ok() || !backend.valid()) {
        return 1;
    }

    zeno::ResourceManager resources;
    zeno::MaterialId sprite_material{ 123 };
    result = resources.create_sprite_material(
        backend,
        zeno::TextureId{ 77 },
        sprite_material_desc(),
        sprite_material);
    if (!invalid_argument(result)
        || sprite_material.value != 123
        || !contains(zeno::last_diagnostic(), "resource: sprite material texture ID is missing, stale, or invalid")) {
        return 2;
    }

    zeno::Scene scene;
    const zeno::ObjectId player = scene.create_object();
    zeno::Transform player_transform{};
    player_transform.position = zeno::Vec3{ -0.25f, 0.25f, 0.0f };
    player_transform.scale = zeno::Vec3{ 0.4f, 0.4f, 1.0f };
    if (!scene.set_transform(player, player_transform).ok()) {
        return 3;
    }

    result = scene.set_sprite_renderer(player, zeno::SpriteRenderer{ zeno::MaterialId{ 88 }, zeno::Color{ 1.0f, 1.0f, 1.0f, 0.8f } });
    if (!result.ok() || scene.renderable_kind(player) != zeno::RenderableKind::sprite) {
        return 4;
    }

    result = scene.render(backend, resources);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "scene: sprite material ID is missing, stale, or invalid")) {
        return 5;
    }

    result = scene.clear_renderer(zeno::ObjectId{ 999 });
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "scene: renderer clear target object is missing")) {
        return 6;
    }

    zeno::NativeBackend invalid_backend;
    const zeno::Camera camera = zeno::Camera::orthographic(2.0f, 1.125f, 0.1f, 10.0f);
    result = invalid_backend.set_camera_matrix(camera.view_projection());
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_set_camera_matrix failed")) {
        return 7;
    }

    result = invalid_backend.draw_debug_rect_2d(
        zeno::aabb_from_transform_2d(player_transform),
        0.9f,
        zeno::Color{ 0.2f, 0.9f, 1.0f, 1.0f });
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_draw_debug_rect failed")) {
        return 8;
    }

    result = invalid_backend.draw_debug_text(
        "2D smoke",
        zeno::Vec3{ -0.9f, 0.9f, 0.0f },
        0.0065f,
        zeno::Color{ 1.0f, 1.0f, 1.0f, 1.0f });
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_draw_debug_text failed")) {
        return 9;
    }

    zeno::InputSnapshot input{};
    result = invalid_backend.input_snapshot(input);
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_get_input_snapshot failed")) {
        return 10;
    }

    zeno::Sound sound;
    result = sound.play();
    if (!invalid_argument(result)
        || !contains(zeno::last_diagnostic(), "native: zen_native_backend_play_sound failed")) {
        return 11;
    }

    return 0;
}

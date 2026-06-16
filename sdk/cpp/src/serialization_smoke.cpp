#include <zeno/zeno.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>

namespace {

bool ok(const zeno::Result& result)
{
    return result.ok();
}

bool invalid_argument(const zeno::Result& result)
{
    return result.native_code() == ZEN_RESULT_INVALID_ARGUMENT;
}

bool not_initialized(const zeno::Result& result)
{
    return result.native_code() == ZEN_RESULT_NOT_INITIALIZED;
}

std::filesystem::path executable_directory()
{
    std::wstring buffer(260, L'\0');
    for (;;) {
        const DWORD written = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (written == 0) {
            return {};
        }

        if (written < buffer.size() - 1) {
            buffer.resize(written);
            return std::filesystem::path(buffer).parent_path();
        }

        buffer.resize(buffer.size() * 2);
    }
}

} // namespace

int main()
{
    zeno::AssetRoot assets;
    zeno::Result result = zeno::AssetRoot::from_path(executable_directory() / "scene_serialization", assets);
    if (!ok(result) || !assets.valid()) {
        return 1;
    }

    zeno::ProjectConfig project;
    result = zeno::load_project_config(assets, "valid_project.zproj", project);
    if (!ok(result)
        || project.version != 1
        || project.window_width != 640
        || project.window_height != 360
        || project.asset_root != "."
        || project.initial_scene != "valid_scene.zscene") {
        return 2;
    }

    zeno::SceneDescription scene;
    result = zeno::load_scene_description(assets, project.initial_scene, scene);
    if (!ok(result) || scene.version != 1 || scene.objects.size() != 2) {
        return 3;
    }

    const zeno::SceneObjectDesc& cube = scene.objects[0];
    if (cube.name != "cube"
        || cube.renderable_kind != zeno::RenderableKind::mesh
        || cube.reference != "builtin:cube"
        || cube.transform.position.z != 3.0f
        || cube.transform.scale.x != 0.7f) {
        return 4;
    }

    const zeno::SceneObjectDesc& sprite = scene.objects[1];
    if (sprite.name != "sprite"
        || sprite.renderable_kind != zeno::RenderableKind::sprite
        || sprite.reference != "textures/sample_sprite_2x2.bmp"
        || sprite.color.a != 0.85f) {
        return 5;
    }

    result = zeno::load_scene_description(assets, "invalid_scene.zscene", scene);
    if (!invalid_argument(result)) {
        return 6;
    }

    result = zeno::load_scene_description(assets, "../invalid_scene.zscene", scene);
    if (!invalid_argument(result)) {
        return 7;
    }

    result = zeno::load_scene_description(assets, "does_not_exist.zscene", scene);
    if (!not_initialized(result)) {
        return 8;
    }

    result = zeno::load_scene_description(assets, "missing_asset_scene.zscene", scene);
    if (!ok(result) || scene.objects.size() != 1) {
        return 9;
    }

    zeno::AssetPath missing_sprite;
    result = assets.resolve(scene.objects[0].reference, missing_sprite);
    if (!ok(result)) {
        return 10;
    }
    if (std::filesystem::exists(missing_sprite.path())) {
        return 11;
    }
    std::vector<std::uint8_t> bytes;
    if (!not_initialized(assets.read_binary(scene.objects[0].reference, bytes))) {
        return 12;
    }

    std::string project_text;
    result = zeno::serialize_project_config(project, project_text);
    if (!ok(result) || project_text.find("zeno_project") == std::string::npos) {
        return 13;
    }

    std::string scene_text;
    result = zeno::serialize_scene_description(scene, scene_text);
    if (!ok(result) || scene_text.find("missing_sprite") == std::string::npos) {
        return 14;
    }

    zeno::SceneDescription empty_scene;
    if (!invalid_argument(zeno::serialize_scene_description(empty_scene, scene_text))) {
        return 15;
    }

    return 0;
}

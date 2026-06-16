#include <zeno/zeno.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

struct SinkState final {
    int count = 0;
    zeno::LogLevel last_level = zeno::LogLevel::info;
    std::string last_category{};
    std::string last_message{};
};

void collect_log(const zeno::LogMessage& message, void* user_data)
{
    auto* state = static_cast<SinkState*>(user_data);
    if (state == nullptr) {
        return;
    }

    ++state->count;
    state->last_level = message.level;
    state->last_category = std::string(message.category);
    state->last_message = std::string(message.message);
}

bool contains(const std::string& text, const char* needle)
{
    return text.find(needle) != std::string::npos;
}

} // namespace

int main()
{
    zeno::clear_log_sink();
    zeno::clear_last_diagnostic();
    if (!zeno::last_diagnostic().empty()) {
        return 1;
    }

    SinkState sink{};
    zeno::set_log_sink(collect_log, &sink);
    zeno::log_message(zeno::LogLevel::warning, "diagnostics", "manual diagnostic");
    if (sink.count != 1
        || sink.last_level != zeno::LogLevel::warning
        || sink.last_category != "diagnostics"
        || sink.last_message != "manual diagnostic"
        || zeno::last_diagnostic() != "manual diagnostic") {
        return 2;
    }

    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / L"zeno_sdk_diagnostics_smoke_assets";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    std::filesystem::create_directories(root, error);
    if (error) {
        return 3;
    }

    zeno::AssetRoot assets;
    zeno::Result result = zeno::AssetRoot::from_path(root, assets);
    if (!result.ok()) {
        std::filesystem::remove_all(root, error);
        return 4;
    }

    std::string text;
    result = assets.read_text("missing.txt", text);
    if (result.ok()
        || !contains(zeno::last_diagnostic(), "asset: text file not found: missing.txt")
        || sink.last_category != "asset") {
        std::filesystem::remove_all(root, error);
        return 5;
    }

    {
        std::ofstream file(root / L"invalid_scene.zscene", std::ios::binary);
        file << "zeno_scene\n";
        file << "version=1\n";
    }

    zeno::SceneDescription scene;
    result = zeno::load_scene_description(assets, "invalid_scene.zscene", scene);
    if (result.ok()
        || !contains(zeno::last_diagnostic(), "scene: scene parse failed: invalid_scene.zscene")
        || sink.last_category != "scene") {
        std::filesystem::remove_all(root, error);
        return 6;
    }

    zeno::clear_log_sink();
    zeno::log_message(zeno::LogLevel::error, "diagnostics", "sink cleared");
    if (sink.count != 3 || zeno::last_diagnostic() != "sink cleared") {
        std::filesystem::remove_all(root, error);
        return 7;
    }

    std::filesystem::remove_all(root, error);
    return 0;
}

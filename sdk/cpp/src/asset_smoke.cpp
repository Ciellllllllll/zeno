#include <zeno/zeno.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

int main()
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / L"zeno_sdk_asset_smoke_assets";

    std::error_code error;
    std::filesystem::remove_all(root, error);
    std::filesystem::create_directories(root, error);
    if (error) {
        return 1;
    }

    {
        std::ofstream file(root / L"sample.txt", std::ios::binary);
        file << "zeno asset smoke\n";
    }

    zeno::AssetRoot missing_root;
    zeno::Result result = zeno::AssetRoot::from_path(root / L"missing", missing_root);
    if (result.ok() || missing_root.valid()) {
        std::filesystem::remove_all(root, error);
        return 2;
    }

    zeno::AssetRoot asset_root;
    result = zeno::AssetRoot::from_path(root, asset_root);
    if (!result.ok() || !asset_root.valid()) {
        std::filesystem::remove_all(root, error);
        return 3;
    }

    zeno::AssetPath resolved_path;
    result = asset_root.resolve("sample.txt", resolved_path);
    if (!result.ok() || !resolved_path.valid() || resolved_path.path() != (root / L"sample.txt").lexically_normal()) {
        std::filesystem::remove_all(root, error);
        return 4;
    }

    std::string text;
    result = asset_root.read_text("sample.txt", text);
    if (!result.ok() || text != "zeno asset smoke\n") {
        std::filesystem::remove_all(root, error);
        return 5;
    }

    text.clear();
    result = asset_root.read_text("missing.txt", text);
    if (result.ok() || !text.empty()) {
        std::filesystem::remove_all(root, error);
        return 6;
    }

    std::vector<std::uint8_t> bytes;
    result = asset_root.read_binary("sample.txt", bytes);
    if (!result.ok() || bytes.size() != 17) {
        std::filesystem::remove_all(root, error);
        return 10;
    }

    bytes.clear();
    result = asset_root.read_binary("missing.txt", bytes);
    if (result.ok() || !bytes.empty()) {
        std::filesystem::remove_all(root, error);
        return 11;
    }

    result = asset_root.resolve("../outside.txt", resolved_path);
    if (result.ok()) {
        std::filesystem::remove_all(root, error);
        return 7;
    }

    result = asset_root.resolve("", resolved_path);
    if (result.ok()) {
        std::filesystem::remove_all(root, error);
        return 8;
    }

    result = asset_root.resolve("folder\\child.txt", resolved_path);
    if (!result.ok()) {
        std::filesystem::remove_all(root, error);
        return 9;
    }

    std::filesystem::remove_all(root, error);
    return 0;
}

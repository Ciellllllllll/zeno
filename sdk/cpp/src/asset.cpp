#include <zeno/zeno.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <fstream>
#include <iterator>
#include <sstream>
#include <system_error>
#include <utility>

namespace zeno {
namespace {

Result path_from_utf8(std::string_view value, std::filesystem::path& out_path)
{
    if (value.empty()) {
        log_message(LogLevel::error, "asset", "asset: relative path is empty");
        return Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    const int required_size = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (required_size <= 0) {
        log_message(LogLevel::error, "asset", "asset: relative path is not valid UTF-8");
        return Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    std::wstring wide_path(static_cast<std::size_t>(required_size), L'\0');
    const int converted_size = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        wide_path.data(),
        required_size);
    if (converted_size != required_size) {
        log_message(LogLevel::error, "asset", "asset: failed to convert relative path from UTF-8");
        return Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    out_path = std::filesystem::path(wide_path);
    return Result();
}

std::string path_to_utf8(const std::filesystem::path& path)
{
    const std::u8string value = path.u8string();
    return std::string(reinterpret_cast<const char*>(value.data()), value.size());
}

bool is_safe_relative_path(const std::filesystem::path& path)
{
    if (path.empty() || path.is_absolute()) {
        return false;
    }

    for (const auto& part : path) {
        if (part.native() == L"." || part.native() == L"..") {
            return false;
        }
    }

    return true;
}

Result validate_asset_root(const std::filesystem::path& root)
{
    std::error_code error;
    if (!std::filesystem::is_directory(root, error) || error) {
        std::ostringstream message;
        message << "asset: root is not a directory: " << path_to_utf8(root);
        log_message(LogLevel::error, "asset", message.str());
        return Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    return Result();
}

Result executable_directory(std::filesystem::path& out_directory)
{
    std::wstring buffer(260, L'\0');

    for (;;) {
        const DWORD written = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size()));
        if (written == 0) {
            log_message(LogLevel::error, "asset", "asset: GetModuleFileNameW failed while finding executable directory");
            return Result(ZEN_RESULT_INTERNAL_ERROR);
        }

        if (written < buffer.size() - 1) {
            buffer.resize(written);
            out_directory = std::filesystem::path(buffer).parent_path();
            return Result();
        }

        buffer.resize(buffer.size() * 2);
    }
}

} // namespace

AssetPath::AssetPath(std::filesystem::path path)
    : path_(std::move(path))
{
}

AssetRoot::AssetRoot(std::filesystem::path root)
    : root_(std::move(root))
{
}

Result AssetRoot::from_executable(AssetRoot& out_root)
{
    std::filesystem::path directory;
    Result result = executable_directory(directory);
    if (!result.ok()) {
        return result;
    }

    return from_path(directory / L"assets", out_root);
}

Result AssetRoot::from_path(const std::filesystem::path& root, AssetRoot& out_root)
{
    const std::filesystem::path normalized_root = root.lexically_normal();
    Result result = validate_asset_root(normalized_root);
    if (!result.ok()) {
        return result;
    }

    out_root = AssetRoot(normalized_root);
    return Result();
}

Result AssetRoot::resolve(std::string_view relative_path_utf8, AssetPath& out_path) const
{
    if (!valid()) {
        log_message(LogLevel::error, "asset", "asset: AssetRoot is not initialized");
        return Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    std::filesystem::path relative_path;
    Result result = path_from_utf8(relative_path_utf8, relative_path);
    if (!result.ok()) {
        return result;
    }

    if (!is_safe_relative_path(relative_path)) {
        std::ostringstream message;
        message << "asset: unsafe relative path rejected: " << std::string(relative_path_utf8);
        log_message(LogLevel::error, "asset", message.str());
        return Result(ZEN_RESULT_INVALID_ARGUMENT);
    }

    out_path = AssetPath((root_ / relative_path).lexically_normal());
    return Result();
}

Result AssetRoot::read_text(std::string_view relative_path_utf8, std::string& out_text) const
{
    AssetPath path;
    Result result = resolve(relative_path_utf8, path);
    if (!result.ok()) {
        return result;
    }

    std::error_code error;
    if (!std::filesystem::is_regular_file(path.path(), error) || error) {
        std::ostringstream message;
        message << "asset: text file not found: " << std::string(relative_path_utf8)
                << " (root: " << path_to_utf8(root_)
                << ", resolved: " << path_to_utf8(path.path()) << ")";
        log_message(LogLevel::error, "asset", message.str());
        return Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    std::ifstream file(path.path(), std::ios::binary);
    if (!file) {
        std::ostringstream message;
        message << "asset: text file could not be opened: " << std::string(relative_path_utf8)
                << " (root: " << path_to_utf8(root_)
                << ", resolved: " << path_to_utf8(path.path()) << ")";
        log_message(LogLevel::error, "asset", message.str());
        return Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    out_text = contents.str();
    return Result();
}

Result AssetRoot::read_binary(std::string_view relative_path_utf8, std::vector<std::uint8_t>& out_bytes) const
{
    AssetPath path;
    Result result = resolve(relative_path_utf8, path);
    if (!result.ok()) {
        return result;
    }

    std::error_code error;
    if (!std::filesystem::is_regular_file(path.path(), error) || error) {
        std::ostringstream message;
        message << "asset: binary file not found: " << std::string(relative_path_utf8)
                << " (root: " << path_to_utf8(root_)
                << ", resolved: " << path_to_utf8(path.path()) << ")";
        log_message(LogLevel::error, "asset", message.str());
        return Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    std::ifstream file(path.path(), std::ios::binary);
    if (!file) {
        std::ostringstream message;
        message << "asset: binary file could not be opened: " << std::string(relative_path_utf8)
                << " (root: " << path_to_utf8(root_)
                << ", resolved: " << path_to_utf8(path.path()) << ")";
        log_message(LogLevel::error, "asset", message.str());
        return Result(ZEN_RESULT_NOT_INITIALIZED);
    }

    out_bytes.assign(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
    return Result();
}

} // namespace zeno

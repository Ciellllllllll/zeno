#include <zeno/zeno.hpp>

#include <charconv>
#include <sstream>

namespace zeno {
namespace {

Result invalid_argument()
{
    return Result(ZEN_RESULT_INVALID_ARGUMENT);
}

std::string trim(std::string_view value)
{
    std::size_t first = 0;
    while (first < value.size() && (value[first] == ' ' || value[first] == '\t' || value[first] == '\r')) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && (value[last - 1] == ' ' || value[last - 1] == '\t' || value[last - 1] == '\r')) {
        --last;
    }

    return std::string(value.substr(first, last - first));
}

bool split_key_value(const std::string& line, std::string& out_key, std::string& out_value)
{
    const std::size_t separator = line.find('=');
    if (separator == std::string::npos || separator == 0) {
        return false;
    }

    out_key = trim(std::string_view(line).substr(0, separator));
    out_value = trim(std::string_view(line).substr(separator + 1));
    return !out_key.empty() && !out_value.empty();
}

bool parse_u32(std::string_view text, std::uint32_t& out_value)
{
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, out_value);
    return result.ec == std::errc{} && result.ptr == last;
}

bool parse_float(std::string_view text, float& out_value)
{
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const auto result = std::from_chars(first, last, out_value);
    return result.ec == std::errc{} && result.ptr == last;
}

bool parse_vec3(const std::string& text, Vec3& out_value)
{
    std::istringstream stream(text);
    std::string x;
    std::string y;
    std::string z;
    if (!(stream >> x >> y >> z)) {
        return false;
    }

    std::string extra;
    if (stream >> extra) {
        return false;
    }

    return parse_float(x, out_value.x) && parse_float(y, out_value.y) && parse_float(z, out_value.z);
}

bool parse_color(const std::string& text, Color& out_value)
{
    std::istringstream stream(text);
    std::string r;
    std::string g;
    std::string b;
    std::string a;
    if (!(stream >> r >> g >> b >> a)) {
        return false;
    }

    std::string extra;
    if (stream >> extra) {
        return false;
    }

    return parse_float(r, out_value.r)
        && parse_float(g, out_value.g)
        && parse_float(b, out_value.b)
        && parse_float(a, out_value.a);
}

bool parse_renderable_kind(const std::string& text, RenderableKind& out_kind)
{
    if (text == "none") {
        out_kind = RenderableKind::none;
        return true;
    }
    if (text == "sprite") {
        out_kind = RenderableKind::sprite;
        return true;
    }
    if (text == "mesh") {
        out_kind = RenderableKind::mesh;
        return true;
    }
    if (text == "triangle") {
        out_kind = RenderableKind::triangle;
        return true;
    }

    return false;
}

bool is_safe_serialized_path(const std::string& text, bool allow_dot)
{
    if (text.empty()) {
        return false;
    }
    if (allow_dot && text == ".") {
        return true;
    }
    if (text == "." || text == ".." || text[0] == '/' || text[0] == '\\' || text.find(':') != std::string::npos) {
        return false;
    }

    std::size_t first = 0;
    while (first <= text.size()) {
        const std::size_t separator = text.find_first_of("/\\", first);
        const std::size_t last = separator == std::string::npos ? text.size() : separator;
        const std::string_view part(text.data() + first, last - first);
        if (part.empty() || part == "." || part == "..") {
            return false;
        }
        if (separator == std::string::npos) {
            break;
        }
        first = separator + 1;
    }

    return true;
}

bool is_safe_reference(const std::string& text)
{
    if (text == "builtin:cube" || text == "builtin:triangle") {
        return true;
    }

    return is_safe_serialized_path(text, false);
}

const char* renderable_kind_name(RenderableKind kind)
{
    switch (kind) {
    case RenderableKind::sprite:
        return "sprite";
    case RenderableKind::mesh:
        return "mesh";
    case RenderableKind::triangle:
        return "triangle";
    case RenderableKind::none:
        return "none";
    }

    return "none";
}

bool parse_project_text(const std::string& text, ProjectConfig& out_config)
{
    ProjectConfig config{};
    bool saw_header = false;
    bool saw_version = false;
    bool saw_initial_scene = false;

    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "zeno_project") {
            if (saw_header) {
                return false;
            }
            saw_header = true;
            continue;
        }

        std::string key;
        std::string value;
        if (!split_key_value(line, key, value)) {
            return false;
        }

        if (key == "version") {
            if (!parse_u32(value, config.version) || config.version != 1) {
                return false;
            }
            saw_version = true;
        } else if (key == "asset_root") {
            if (!is_safe_serialized_path(value, true)) {
                return false;
            }
            config.asset_root = value;
        } else if (key == "initial_scene") {
            if (!is_safe_serialized_path(value, false)) {
                return false;
            }
            config.initial_scene = value;
            saw_initial_scene = true;
        } else if (key == "window_width") {
            if (!parse_u32(value, config.window_width) || config.window_width == 0) {
                return false;
            }
        } else if (key == "window_height") {
            if (!parse_u32(value, config.window_height) || config.window_height == 0) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (!saw_header || !saw_version || !saw_initial_scene || config.asset_root.empty()) {
        return false;
    }

    out_config = std::move(config);
    return true;
}

bool parse_scene_text(const std::string& text, SceneDescription& out_scene)
{
    SceneDescription scene{};
    bool saw_header = false;
    bool saw_version = false;
    bool in_object = false;
    SceneObjectDesc object{};

    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "zeno_scene") {
            if (saw_header || in_object) {
                return false;
            }
            saw_header = true;
            continue;
        }

        if (line == "[object]") {
            if (!saw_header || in_object) {
                return false;
            }
            in_object = true;
            object = SceneObjectDesc{};
            continue;
        }

        if (line == "[/object]") {
            if (!in_object || object.renderable_kind == RenderableKind::none || object.reference.empty()) {
                return false;
            }
            scene.objects.push_back(object);
            in_object = false;
            continue;
        }

        std::string key;
        std::string value;
        if (!split_key_value(line, key, value)) {
            return false;
        }

        if (!in_object) {
            if (key != "version" || !parse_u32(value, scene.version) || scene.version != 1) {
                return false;
            }
            saw_version = true;
            continue;
        }

        if (key == "name") {
            object.name = value;
        } else if (key == "renderable") {
            if (!parse_renderable_kind(value, object.renderable_kind)) {
                return false;
            }
        } else if (key == "reference") {
            if (!is_safe_reference(value)) {
                return false;
            }
            object.reference = value;
        } else if (key == "position") {
            if (!parse_vec3(value, object.transform.position)) {
                return false;
            }
        } else if (key == "rotation_z") {
            if (!parse_float(value, object.transform.rotation_z_radians)) {
                return false;
            }
        } else if (key == "scale") {
            if (!parse_vec3(value, object.transform.scale)) {
                return false;
            }
        } else if (key == "color") {
            if (!parse_color(value, object.color)) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (!saw_header || !saw_version || in_object || scene.objects.empty()) {
        return false;
    }

    out_scene = std::move(scene);
    return true;
}

} // namespace

Result load_project_config(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    ProjectConfig& out_config)
{
    std::string text;
    Result result = assets.read_text(relative_path_utf8, text);
    if (!result.ok()) {
        return result;
    }

    ProjectConfig config{};
    if (!parse_project_text(text, config)) {
        return invalid_argument();
    }

    out_config = std::move(config);
    return Result();
}

Result load_scene_description(
    const AssetRoot& assets,
    std::string_view relative_path_utf8,
    SceneDescription& out_scene)
{
    std::string text;
    Result result = assets.read_text(relative_path_utf8, text);
    if (!result.ok()) {
        return result;
    }

    SceneDescription scene{};
    if (!parse_scene_text(text, scene)) {
        return invalid_argument();
    }

    out_scene = std::move(scene);
    return Result();
}

Result serialize_project_config(const ProjectConfig& config, std::string& out_text)
{
    if (config.version != 1 || config.window_width == 0 || config.window_height == 0 || config.asset_root.empty() || config.initial_scene.empty()) {
        return invalid_argument();
    }

    std::ostringstream output;
    output << "zeno_project\n";
    output << "version=" << config.version << "\n";
    output << "asset_root=" << config.asset_root << "\n";
    output << "initial_scene=" << config.initial_scene << "\n";
    output << "window_width=" << config.window_width << "\n";
    output << "window_height=" << config.window_height << "\n";
    out_text = output.str();
    return Result();
}

Result serialize_scene_description(const SceneDescription& scene, std::string& out_text)
{
    if (scene.version != 1 || scene.objects.empty()) {
        return invalid_argument();
    }

    std::ostringstream output;
    output << "zeno_scene\n";
    output << "version=" << scene.version << "\n";
    for (const SceneObjectDesc& object : scene.objects) {
        if (object.renderable_kind == RenderableKind::none || object.reference.empty()) {
            return invalid_argument();
        }

        output << "[object]\n";
        output << "name=" << object.name << "\n";
        output << "renderable=" << renderable_kind_name(object.renderable_kind) << "\n";
        output << "reference=" << object.reference << "\n";
        output << "position=" << object.transform.position.x << " " << object.transform.position.y << " " << object.transform.position.z << "\n";
        output << "rotation_z=" << object.transform.rotation_z_radians << "\n";
        output << "scale=" << object.transform.scale.x << " " << object.transform.scale.y << " " << object.transform.scale.z << "\n";
        output << "color=" << object.color.r << " " << object.color.g << " " << object.color.b << " " << object.color.a << "\n";
        output << "[/object]\n";
    }

    out_text = output.str();
    return Result();
}

} // namespace zeno

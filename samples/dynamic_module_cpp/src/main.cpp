#include <zeno/game_module.hpp>

#include <filesystem>
#include <iostream>

namespace {

std::filesystem::path default_module_path(const char* executable_path)
{
    std::filesystem::path path(executable_path);
    return path.parent_path() / "zeno_dynamic_sample_module.dll";
}

} // namespace

int main(int argc, char** argv)
{
    const std::filesystem::path module_path = argc > 1 ? std::filesystem::path(argv[1]) : default_module_path(argv[0]);
    const std::filesystem::path module_dir = module_path.parent_path();

    zeno::DynamicGameModule module;
    zeno::Result result = zeno::DynamicGameModule::load(module_dir / "does-not-exist.dll", module);
    if (result.ok() || module.valid()) {
        std::cerr << "missing dynamic module unexpectedly loaded\n";
        return 1;
    }

    result = zeno::DynamicGameModule::load(module_dir / "zeno_dynamic_missing_entry_module.dll", module);
    if (result.ok() || module.valid()) {
        std::cerr << "missing-entry dynamic module unexpectedly loaded\n";
        return 2;
    }

    result = zeno::DynamicGameModule::load(module_dir / "zeno_dynamic_bad_version_module.dll", module);
    if (result.ok() || module.valid()) {
        std::cerr << "bad-version dynamic module unexpectedly loaded\n";
        return 3;
    }

    result = zeno::DynamicGameModule::load(module_path, module);
    if (!result.ok()) {
        std::cerr << "dynamic module load failed: " << result.message() << "\n";
        return 4;
    }

    zeno::GameContext context{};
    result = zeno::initialize_game_module(module, context);
    if (!result.ok()) {
        std::cerr << "dynamic module init failed: " << result.message() << "\n";
        return 5;
    }

    result = zeno::run_game_module_frame(module, context);
    if (!result.ok()) {
        std::cerr << "dynamic module frame failed: " << result.message() << "\n";
        return 6;
    }

    result = zeno::shutdown_game_module(module, context);
    if (!result.ok()) {
        std::cerr << "dynamic module shutdown failed: " << result.message() << "\n";
        return 7;
    }

    module.reset();
    std::cout << "ZENO dynamic module loaded, ran, and unloaded\n";
    return 0;
}

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <zeno/game_module_abi.h>
#include <zeno/zeno.hpp>

namespace zeno {

struct GameContext final {
    Engine* engine = nullptr;
    NativeBackend* backend = nullptr;
    AssetRoot* assets = nullptr;
    AudioEngine* audio = nullptr;
    Scene* runtime_scene = nullptr;
    const ProjectConfig* project = nullptr;
    const SceneDescription* scene = nullptr;
    InputSnapshot input{};
    std::uint64_t frame_index = 0;
    double delta_time_seconds = 0.0;
    bool should_close = false;
};

using GameLifecycleFn = Result (*)(GameContext& context);

struct GameModule final {
    GameLifecycleFn on_init = nullptr;
    GameLifecycleFn on_update = nullptr;
    GameLifecycleFn on_render = nullptr;
    GameLifecycleFn on_shutdown = nullptr;
};

Result initialize_game_module(GameModule& module, GameContext& context);
Result run_game_module_frame(GameModule& module, GameContext& context);
Result shutdown_game_module(GameModule& module, GameContext& context);

class DynamicGameModule final {
public:
    DynamicGameModule() = default;
    ~DynamicGameModule();

    DynamicGameModule(const DynamicGameModule&) = delete;
    DynamicGameModule& operator=(const DynamicGameModule&) = delete;

    DynamicGameModule(DynamicGameModule&& other) noexcept;
    DynamicGameModule& operator=(DynamicGameModule&& other) noexcept;

    static Result load(const std::filesystem::path& module_path, DynamicGameModule& out_module);

    void reset();
    bool valid() const { return library_handle_ != nullptr; }
    const ZenGameModuleDescriptor& descriptor() const { return descriptor_; }

private:
    friend Result initialize_game_module(DynamicGameModule& module, GameContext& context);
    friend Result run_game_module_frame(DynamicGameModule& module, GameContext& context);
    friend Result shutdown_game_module(DynamicGameModule& module, GameContext& context);

    void* library_handle_ = nullptr;
    ZenGameModuleDescriptor descriptor_{};
};

Result initialize_game_module(DynamicGameModule& module, GameContext& context);
Result run_game_module_frame(DynamicGameModule& module, GameContext& context);
Result shutdown_game_module(DynamicGameModule& module, GameContext& context);

struct GameAppConfig final {
    EngineConfig engine{};
    std::string project_path = "project.zproj";
};

class GameApp final {
public:
    GameApp() = default;
    ~GameApp();

    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;

    GameApp(GameApp&& other) noexcept;
    GameApp& operator=(GameApp&& other) noexcept;

    Result run(GameModule module, const GameAppConfig& config = {});
    Result run(DynamicGameModule& module, const GameAppConfig& config = {});
    void reset();

    bool running() const { return running_; }
    GameContext& context() { return context_; }
    const GameContext& context() const { return context_; }

private:
    Result initialize(const GameAppConfig& config);
    Result begin_frame();
    Result end_frame();
    Result shutdown(GameModule& module);
    Result shutdown(DynamicGameModule& module);

    Engine engine_{};
    NativeBackend backend_{};
    AssetRoot assets_{};
    AudioEngine audio_{};
    Scene runtime_scene_{};
    ProjectConfig project_{};
    SceneDescription scene_{};
    GameContext context_{};
    bool running_ = false;
    bool module_initialized_ = false;
};

} // namespace zeno

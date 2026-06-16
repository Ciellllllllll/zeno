#pragma once

#include <cstdint>

#include <zeno/zeno.hpp>

namespace zeno {

struct GameContext final {
    NativeBackend* backend = nullptr;
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

} // namespace zeno

#include "sample_module.h"

#include <iostream>

int main()
{
    zeno::GameApp app;
    zeno::Result result = app.run(create_sample_game_module());
    if (!result.ok()) {
        std::cerr << "game app failed: " << result.message() << "\n";
        return 1;
    }

    return 0;
}

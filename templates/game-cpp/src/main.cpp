#include "template_module.h"

#include <iostream>

int main()
{
    zeno::GameApp app;
    zeno::Result result = app.run(create_template_game_module());
    if (!result.ok()) {
        std::cerr << "template game failed: " << result.message() << "\n";
        return 1;
    }

    return 0;
}

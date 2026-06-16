#include "native_backend.h"

int main()
{
    zeno::native::NativeBackend backend;
    if (!backend.initialize({})) {
        return 1;
    }

    if (!backend.create_window(zeno::native::NativeWindowConfig{ 320, 180 })) {
        return 2;
    }

    if (!backend.initialize_renderer()) {
        return 3;
    }

    bool should_close = false;
    if (!backend.poll_events(should_close) || should_close) {
        return 4;
    }

    backend.handle_window_resize(0, 0, true);
    if (!backend.begin_frame()) {
        return 5;
    }
    if (!backend.clear(0.05f, 0.10f, 0.15f, 1.0f)) {
        return 6;
    }
    if (!backend.present()) {
        return 7;
    }

    backend.handle_window_resize(400, 240, false);
    if (!backend.begin_frame()) {
        return 8;
    }
    if (!backend.clear(0.15f, 0.10f, 0.05f, 1.0f)) {
        return 9;
    }
    if (!backend.present()) {
        return 10;
    }

    backend.shutdown();
    return 0;
}

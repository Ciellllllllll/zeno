#include "native_backend.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>

namespace zeno::native {
namespace {

constexpr wchar_t kWindowClassName[] = L"ZenoNativeWindowClass";
constexpr wchar_t kDefaultWindowTitle[] = L"ZENO Engine";

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_CLOSE) {
        DestroyWindow(window);
        return 0;
    }

    if (message == WM_DESTROY) {
        auto* backend = reinterpret_cast<NativeBackend*>(GetWindowLongPtrW(window, GWLP_USERDATA));
        if (backend != nullptr) {
            backend->notify_window_destroyed(reinterpret_cast<HWND__*>(window));
        }

        return 0;
    }

    return DefWindowProcW(window, message, wparam, lparam);
}

bool register_window_class()
{
    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = GetModuleHandleW(nullptr);
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    window_class.lpszClassName = kWindowClassName;

    if (RegisterClassExW(&window_class) != 0) {
        return true;
    }

    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

} // namespace

bool NativeBackend::initialize(const NativeBackendConfig& config)
{
    if (initialized_) {
        return false;
    }

    config_ = config;
    initialized_ = true;
    std::cerr << "[ZENO][native] backend initialized\n";
    return true;
}

void NativeBackend::shutdown()
{
    if (!initialized_) {
        return;
    }

    destroy_window();
    initialized_ = false;
    std::cerr << "[ZENO][native] backend shutdown\n";
}

bool NativeBackend::create_window(const NativeWindowConfig& config)
{
    if (!initialized_ || window_handle_ != nullptr || config.width == 0 || config.height == 0) {
        return false;
    }

    if (!register_window_class()) {
        return false;
    }

    RECT window_rect{
        0,
        0,
        static_cast<LONG>(config.width),
        static_cast<LONG>(config.height),
    };
    constexpr DWORD window_style = WS_OVERLAPPEDWINDOW;
    if (AdjustWindowRect(&window_rect, window_style, FALSE) == FALSE) {
        return false;
    }

    HWND window = CreateWindowExW(
        0,
        kWindowClassName,
        kDefaultWindowTitle,
        window_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);

    if (window == nullptr) {
        return false;
    }

    SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    window_handle_ = window;
    should_close_ = false;
    std::cerr << "[ZENO][native] window created\n";
    return true;
}

bool NativeBackend::poll_events(bool& out_should_close)
{
    if (!initialized_) {
        return false;
    }

    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    out_should_close = should_close_;
    return true;
}

void NativeBackend::notify_window_destroyed(HWND__* window)
{
    if (window_handle_ == window) {
        window_handle_ = nullptr;
    }

    should_close_ = true;
}

bool NativeBackend::is_initialized() const
{
    return initialized_;
}

bool NativeBackend::has_window() const
{
    return window_handle_ != nullptr;
}

void NativeBackend::destroy_window()
{
    if (window_handle_ == nullptr) {
        return;
    }

    HWND window = window_handle_;
    window_handle_ = nullptr;
    DestroyWindow(window);
    should_close_ = true;
    std::cerr << "[ZENO][native] window destroyed\n";
}

} // namespace zeno::native

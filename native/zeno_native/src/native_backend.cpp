#include "native_backend.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <array>
#include <iostream>
#include <unordered_map>

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

class DirectX11Renderer final {
public:
    bool initialize(HWND window)
    {
        if (window == nullptr || swap_chain_ != nullptr) {
            return false;
        }

        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
        swap_chain_desc.BufferDesc.Width = 0;
        swap_chain_desc.BufferDesc.Height = 0;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
        swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.SampleDesc.Quality = 0;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.BufferCount = 2;
        swap_chain_desc.OutputWindow = window;
        swap_chain_desc.Windowed = TRUE;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        constexpr D3D_FEATURE_LEVEL requested_feature_levels[] = {
            D3D_FEATURE_LEVEL_11_0,
        };
        D3D_FEATURE_LEVEL created_feature_level{};

        HRESULT result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            requested_feature_levels,
            1,
            D3D11_SDK_VERSION,
            &swap_chain_desc,
            swap_chain_.GetAddressOf(),
            device_.GetAddressOf(),
            &created_feature_level,
            context_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
        result = swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
        if (FAILED(result)) {
            shutdown();
            return false;
        }

        result = device_->CreateRenderTargetView(back_buffer.Get(), nullptr, render_target_view_.GetAddressOf());
        if (FAILED(result)) {
            shutdown();
            return false;
        }

        std::cerr << "[ZENO][native] DirectX 11 renderer initialized\n";
        return true;
    }

    void shutdown()
    {
        if (context_ != nullptr) {
            context_->ClearState();
        }

        render_target_view_.Reset();
        swap_chain_.Reset();
        context_.Reset();
        device_.Reset();
    }

    bool begin_frame()
    {
        if (context_ == nullptr || render_target_view_ == nullptr) {
            return false;
        }

        ID3D11RenderTargetView* render_targets[] = { render_target_view_.Get() };
        context_->OMSetRenderTargets(1, render_targets, nullptr);
        return true;
    }

    bool clear(float r, float g, float b, float a)
    {
        if (context_ == nullptr || render_target_view_ == nullptr) {
            return false;
        }

        const float color[] = { r, g, b, a };
        context_->ClearRenderTargetView(render_target_view_.Get(), color);
        return true;
    }

    bool create_clear_color(float r, float g, float b, float a, std::uint64_t& out_handle)
    {
        if (context_ == nullptr || render_target_view_ == nullptr || next_clear_color_handle_ == UINT64_MAX) {
            return false;
        }

        const std::uint64_t handle = next_clear_color_handle_++;
        if (handle == 0) {
            return false;
        }

        clear_colors_.emplace(handle, std::array<float, 4>{ r, g, b, a });
        out_handle = handle;
        std::cerr << "[ZENO][native] clear color resource created\n";
        return true;
    }

    bool destroy_clear_color(std::uint64_t handle)
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = clear_colors_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] clear color resource destroyed\n";
        }

        return destroyed;
    }

    bool clear_with_resource(std::uint64_t handle)
    {
        const auto found = clear_colors_.find(handle);
        if (found == clear_colors_.end()) {
            return false;
        }

        return clear(found->second[0], found->second[1], found->second[2], found->second[3]);
    }

    bool present()
    {
        if (swap_chain_ == nullptr) {
            return false;
        }

        return SUCCEEDED(swap_chain_->Present(1, 0));
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;
    std::unordered_map<std::uint64_t, std::array<float, 4>> clear_colors_;
    std::uint64_t next_clear_color_handle_ = 1;
};

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

NativeBackend::NativeBackend() = default;

NativeBackend::~NativeBackend()
{
    shutdown();
}

void NativeBackend::shutdown()
{
    if (!initialized_) {
        return;
    }

    destroy_renderer();
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

    destroy_renderer();
    should_close_ = true;
}

bool NativeBackend::initialize_renderer()
{
    if (!initialized_ || window_handle_ == nullptr || renderer_ != nullptr) {
        return false;
    }

    auto renderer = std::make_unique<DirectX11Renderer>();
    if (!renderer->initialize(reinterpret_cast<HWND>(window_handle_))) {
        return false;
    }

    renderer_ = std::move(renderer);
    return true;
}

bool NativeBackend::begin_frame()
{
    return renderer_ != nullptr && renderer_->begin_frame();
}

bool NativeBackend::clear(float r, float g, float b, float a)
{
    return renderer_ != nullptr && renderer_->clear(r, g, b, a);
}

bool NativeBackend::create_clear_color(float r, float g, float b, float a, std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_clear_color(r, g, b, a, out_handle);
}

bool NativeBackend::destroy_clear_color(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_clear_color(handle);
}

bool NativeBackend::clear_with_resource(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->clear_with_resource(handle);
}

bool NativeBackend::present()
{
    return renderer_ != nullptr && renderer_->present();
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

void NativeBackend::destroy_renderer()
{
    if (renderer_ == nullptr) {
        return;
    }

    renderer_->shutdown();
    renderer_.reset();
    std::cerr << "[ZENO][native] DirectX 11 renderer shutdown\n";
}

} // namespace zeno::native

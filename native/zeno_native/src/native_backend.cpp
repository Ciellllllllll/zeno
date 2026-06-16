#include "native_backend.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <array>
#include <cstddef>
#include <iostream>
#include <unordered_map>

namespace zeno::native {
namespace {

constexpr wchar_t kWindowClassName[] = L"ZenoNativeWindowClass";
constexpr wchar_t kDefaultWindowTitle[] = L"ZENO Engine";

constexpr char kTriangleShaderSource[] = R"(
struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput vs_main(VSInput input) {
    PSInput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    return output;
}

float4 ps_main(PSInput input) : SV_TARGET {
    return input.color;
}
)";

struct TriangleVertex final {
    float position[3];
    float color[4];
};

struct TriangleResource final {
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
};

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

        D3D11_TEXTURE2D_DESC back_buffer_desc{};
        back_buffer->GetDesc(&back_buffer_desc);
        viewport_.TopLeftX = 0.0f;
        viewport_.TopLeftY = 0.0f;
        viewport_.Width = static_cast<float>(back_buffer_desc.Width);
        viewport_.Height = static_cast<float>(back_buffer_desc.Height);
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

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
        triangle_resources_.clear();
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
        context_->RSSetViewports(1, &viewport_);
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

    bool create_triangle(std::uint64_t& out_handle)
    {
        if (device_ == nullptr || context_ == nullptr || next_triangle_handle_ == UINT64_MAX) {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

        HRESULT result = D3DCompile(
            kTriangleShaderSource,
            sizeof(kTriangleShaderSource) - 1,
            nullptr,
            nullptr,
            nullptr,
            "vs_main",
            "vs_4_0",
            0,
            0,
            vertex_shader_blob.GetAddressOf(),
            error_blob.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        error_blob.Reset();
        result = D3DCompile(
            kTriangleShaderSource,
            sizeof(kTriangleShaderSource) - 1,
            nullptr,
            nullptr,
            nullptr,
            "ps_main",
            "ps_4_0",
            0,
            0,
            pixel_shader_blob.GetAddressOf(),
            error_blob.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        TriangleResource resource{};
        result = device_->CreateVertexShader(
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            nullptr,
            resource.vertex_shader.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = device_->CreatePixelShader(
            pixel_shader_blob->GetBufferPointer(),
            pixel_shader_blob->GetBufferSize(),
            nullptr,
            resource.pixel_shader.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        constexpr D3D11_INPUT_ELEMENT_DESC input_elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(TriangleVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(TriangleVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        result = device_->CreateInputLayout(
            input_elements,
            2,
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            resource.input_layout.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        constexpr TriangleVertex vertices[] = {
            { { 0.0f, 0.65f, 0.0f }, { 0.95f, 0.25f, 0.20f, 1.0f } },
            { { 0.65f, -0.55f, 0.0f }, { 0.20f, 0.85f, 0.45f, 1.0f } },
            { { -0.65f, -0.55f, 0.0f }, { 0.20f, 0.45f, 0.95f, 1.0f } },
        };

        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(vertices);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initial_data{};
        initial_data.pSysMem = vertices;
        result = device_->CreateBuffer(&buffer_desc, &initial_data, resource.vertex_buffer.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        const std::uint64_t handle = next_triangle_handle_++;
        if (handle == 0) {
            return false;
        }

        triangle_resources_.emplace(handle, std::move(resource));
        out_handle = handle;
        std::cerr << "[ZENO][native] triangle resource created\n";
        return true;
    }

    bool destroy_triangle(std::uint64_t handle)
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = triangle_resources_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] triangle resource destroyed\n";
        }

        return destroyed;
    }

    bool draw_triangle(std::uint64_t handle)
    {
        if (context_ == nullptr || render_target_view_ == nullptr) {
            return false;
        }

        const auto found = triangle_resources_.find(handle);
        if (found == triangle_resources_.end()) {
            return false;
        }

        const TriangleResource& resource = found->second;
        constexpr UINT stride = sizeof(TriangleVertex);
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { resource.vertex_buffer.Get() };
        context_->IASetInputLayout(resource.input_layout.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(resource.vertex_shader.Get(), nullptr, 0);
        context_->PSSetShader(resource.pixel_shader.Get(), nullptr, 0);
        context_->Draw(3, 0);
        return true;
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
    D3D11_VIEWPORT viewport_{};
    std::unordered_map<std::uint64_t, std::array<float, 4>> clear_colors_;
    std::unordered_map<std::uint64_t, TriangleResource> triangle_resources_;
    std::uint64_t next_clear_color_handle_ = 1;
    std::uint64_t next_triangle_handle_ = 1;
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

bool NativeBackend::create_triangle(std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_triangle(out_handle);
}

bool NativeBackend::destroy_triangle(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_triangle(handle);
}

bool NativeBackend::draw_triangle(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->draw_triangle(handle);
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

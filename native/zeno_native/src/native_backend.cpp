#include "native_backend.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <array>
#include <algorithm>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>

namespace zeno::native {
namespace {

constexpr wchar_t kWindowClassName[] = L"ZenoNativeWindowClass";
constexpr wchar_t kDefaultWindowTitle[] = L"ZENO Engine";

constexpr char kTriangleShaderSource[] = R"(
cbuffer TriangleTransformConstants : register(b0) {
    row_major float4x4 u_world;
    row_major float4x4 u_view_projection;
};

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
    output.position = mul(mul(float4(input.position, 1.0), u_world), u_view_projection);
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

struct VertexShaderResource final {
    Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
    Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
};

struct PixelShaderResource final {
    Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
};

struct TriangleTransformConstants final {
    float world[16];
    float view_projection[16];
};

static_assert(sizeof(TriangleTransformConstants) == 128);

Matrix4x4 identity_matrix()
{
    Matrix4x4 matrix{};
    matrix.elements[0] = 1.0f;
    matrix.elements[5] = 1.0f;
    matrix.elements[10] = 1.0f;
    matrix.elements[15] = 1.0f;
    return matrix;
}

void set_compile_log(ShaderCompileLog& compile_log, const char* message, std::size_t length)
{
    constexpr std::size_t capacity = sizeof(compile_log.message);
    const std::size_t copy_length = std::min(length, capacity - 1);
    if (copy_length > 0) {
        std::memcpy(compile_log.message, message, copy_length);
    }
    compile_log.message[copy_length] = '\0';
    compile_log.message_length = static_cast<std::uint32_t>(copy_length);
}

void set_compile_log(ShaderCompileLog& compile_log, const Microsoft::WRL::ComPtr<ID3DBlob>& blob)
{
    if (blob == nullptr || blob->GetBufferPointer() == nullptr) {
        set_compile_log(compile_log, "", 0);
        return;
    }

    set_compile_log(
        compile_log,
        static_cast<const char*>(blob->GetBufferPointer()),
        blob->GetBufferSize());
}

bool make_string(const char* data, std::uint64_t length, std::string& out_string)
{
    if (data == nullptr || length == 0 || length > static_cast<std::uint64_t>(SIZE_MAX)) {
        return false;
    }

    out_string.assign(data, static_cast<std::size_t>(length));
    return true;
}

DXGI_FORMAT map_vertex_format(std::uint32_t format)
{
    switch (format) {
    case 1:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case 2:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

const char* map_vertex_semantic(std::uint32_t semantic)
{
    switch (semantic) {
    case 1:
        return "POSITION";
    case 2:
        return "COLOR";
    default:
        return nullptr;
    }
}

bool make_input_elements(
    const VertexInputLayoutDesc& input_layout,
    std::array<D3D11_INPUT_ELEMENT_DESC, 8>& out_elements)
{
    if (input_layout.element_count == 0 || input_layout.element_count > out_elements.size()) {
        return false;
    }

    for (std::uint32_t i = 0; i < input_layout.element_count; ++i) {
        const VertexInputElement& input = input_layout.elements[i];
        const char* semantic = map_vertex_semantic(input.semantic);
        const DXGI_FORMAT format = map_vertex_format(input.format);
        if (semantic == nullptr || format == DXGI_FORMAT_UNKNOWN) {
            return false;
        }

        out_elements[i] = D3D11_INPUT_ELEMENT_DESC{
            semantic,
            input.semantic_index,
            format,
            input.input_slot,
            input.aligned_byte_offset,
            D3D11_INPUT_PER_VERTEX_DATA,
            0,
        };
    }

    return true;
}

std::uint32_t map_virtual_key_to_input_key(WPARAM key)
{
    switch (key) {
    case VK_ESCAPE:
        return 1;
    case VK_SPACE:
        return 2;
    case VK_LEFT:
        return 3;
    case VK_RIGHT:
        return 4;
    case VK_UP:
        return 5;
    case VK_DOWN:
        return 6;
    case 'A':
        return 7;
    case 'D':
        return 8;
    case 'W':
        return 9;
    case 'S':
        return 10;
    default:
        return 0;
    }
}

std::int32_t mouse_x_from_lparam(LPARAM lparam)
{
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(lparam & 0xffff));
}

std::int32_t mouse_y_from_lparam(LPARAM lparam)
{
    return static_cast<std::int16_t>(static_cast<std::uint16_t>((lparam >> 16) & 0xffff));
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    auto* backend = reinterpret_cast<NativeBackend*>(GetWindowLongPtrW(window, GWLP_USERDATA));

    if (message == WM_CLOSE) {
        DestroyWindow(window);
        return 0;
    }

    if (message == WM_DESTROY) {
        if (backend != nullptr) {
            backend->notify_window_destroyed(reinterpret_cast<HWND__*>(window));
        }

        return 0;
    }

    if (backend != nullptr) {
        switch (message) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            backend->handle_key_message(map_virtual_key_to_input_key(wparam), true);
            return 0;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            backend->handle_key_message(map_virtual_key_to_input_key(wparam), false);
            return 0;
        case WM_MOUSEMOVE:
            backend->handle_mouse_move(mouse_x_from_lparam(lparam), mouse_y_from_lparam(lparam));
            return 0;
        case WM_LBUTTONDOWN:
            backend->handle_mouse_button(0, true);
            return 0;
        case WM_LBUTTONUP:
            backend->handle_mouse_button(0, false);
            return 0;
        case WM_RBUTTONDOWN:
            backend->handle_mouse_button(1, true);
            return 0;
        case WM_RBUTTONUP:
            backend->handle_mouse_button(1, false);
            return 0;
        case WM_MBUTTONDOWN:
            backend->handle_mouse_button(2, true);
            return 0;
        case WM_MBUTTONUP:
            backend->handle_mouse_button(2, false);
            return 0;
        case WM_MOUSEWHEEL:
            backend->handle_mouse_wheel(static_cast<std::int16_t>((wparam >> 16) & 0xffff) / WHEEL_DELTA);
            return 0;
        case WM_KILLFOCUS:
        case WM_CAPTURECHANGED:
            backend->clear_input_state();
            return 0;
        default:
            break;
        }
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

        D3D11_BUFFER_DESC transform_buffer_desc{};
        transform_buffer_desc.ByteWidth = sizeof(TriangleTransformConstants);
        transform_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        transform_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        result = device_->CreateBuffer(&transform_buffer_desc, nullptr, triangle_transform_buffer_.GetAddressOf());
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

        frame_active_ = false;
        triangle_transform_buffer_.Reset();
        render_target_view_.Reset();
        triangle_resources_.clear();
        pixel_shader_resources_.clear();
        vertex_shader_resources_.clear();
        swap_chain_.Reset();
        context_.Reset();
        device_.Reset();
    }

    bool begin_frame()
    {
        if (context_ == nullptr || render_target_view_ == nullptr || frame_active_) {
            return false;
        }

        ID3D11RenderTargetView* render_targets[] = { render_target_view_.Get() };
        context_->OMSetRenderTargets(1, render_targets, nullptr);
        context_->RSSetViewports(1, &viewport_);
        frame_active_ = true;
        return true;
    }

    bool clear(float r, float g, float b, float a)
    {
        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
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

    RenderCommandResult clear_with_resource(std::uint64_t handle)
    {
        const auto found = clear_colors_.find(handle);
        if (found == clear_colors_.end()) {
            return RenderCommandResult::missing_resource;
        }

        return clear(found->second[0], found->second[1], found->second[2], found->second[3])
            ? RenderCommandResult::ok
            : RenderCommandResult::wrong_state;
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

    bool create_vertex_shader_from_source(
        const char* source,
        std::uint64_t source_length,
        const char* entry,
        std::uint64_t entry_length,
        const char* profile,
        std::uint64_t profile_length,
        ShaderCompileLog& compile_log,
        std::uint64_t& out_handle)
    {
        if (device_ == nullptr || context_ == nullptr || next_vertex_shader_handle_ == UINT64_MAX) {
            return false;
        }

        std::string entry_string;
        std::string profile_string;
        if (!make_string(entry, entry_length, entry_string) || !make_string(profile, profile_length, profile_string)) {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
        HRESULT result = D3DCompile(
            source,
            static_cast<SIZE_T>(source_length),
            nullptr,
            nullptr,
            nullptr,
            entry_string.c_str(),
            profile_string.c_str(),
            0,
            0,
            shader_blob.GetAddressOf(),
            error_blob.GetAddressOf());
        if (FAILED(result)) {
            set_compile_log(compile_log, error_blob);
            return false;
        }

        VertexShaderResource resource{};
        result = device_->CreateVertexShader(
            shader_blob->GetBufferPointer(),
            shader_blob->GetBufferSize(),
            nullptr,
            resource.shader.GetAddressOf());
        if (FAILED(result)) {
            set_compile_log(compile_log, "CreateVertexShader failed", 25);
            return false;
        }

        const std::uint64_t handle = next_vertex_shader_handle_++;
        if (handle == 0) {
            return false;
        }

        resource.bytecode = std::move(shader_blob);
        vertex_shader_resources_.emplace(handle, std::move(resource));
        out_handle = handle;
        set_compile_log(compile_log, "", 0);
        std::cerr << "[ZENO][native] vertex shader resource created\n";
        return true;
    }

    bool create_pixel_shader_from_source(
        const char* source,
        std::uint64_t source_length,
        const char* entry,
        std::uint64_t entry_length,
        const char* profile,
        std::uint64_t profile_length,
        ShaderCompileLog& compile_log,
        std::uint64_t& out_handle)
    {
        if (device_ == nullptr || context_ == nullptr || next_pixel_shader_handle_ == UINT64_MAX) {
            return false;
        }

        std::string entry_string;
        std::string profile_string;
        if (!make_string(entry, entry_length, entry_string) || !make_string(profile, profile_length, profile_string)) {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
        HRESULT result = D3DCompile(
            source,
            static_cast<SIZE_T>(source_length),
            nullptr,
            nullptr,
            nullptr,
            entry_string.c_str(),
            profile_string.c_str(),
            0,
            0,
            shader_blob.GetAddressOf(),
            error_blob.GetAddressOf());
        if (FAILED(result)) {
            set_compile_log(compile_log, error_blob);
            return false;
        }

        PixelShaderResource resource{};
        result = device_->CreatePixelShader(
            shader_blob->GetBufferPointer(),
            shader_blob->GetBufferSize(),
            nullptr,
            resource.shader.GetAddressOf());
        if (FAILED(result)) {
            set_compile_log(compile_log, "CreatePixelShader failed", 24);
            return false;
        }

        const std::uint64_t handle = next_pixel_shader_handle_++;
        if (handle == 0) {
            return false;
        }

        pixel_shader_resources_.emplace(handle, std::move(resource));
        out_handle = handle;
        set_compile_log(compile_log, "", 0);
        std::cerr << "[ZENO][native] pixel shader resource created\n";
        return true;
    }

    bool destroy_vertex_shader(std::uint64_t handle)
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = vertex_shader_resources_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] vertex shader resource destroyed\n";
        }

        return destroyed;
    }

    bool destroy_pixel_shader(std::uint64_t handle)
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = pixel_shader_resources_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] pixel shader resource destroyed\n";
        }

        return destroyed;
    }

    bool create_triangle_with_shaders(
        std::uint64_t vertex_shader,
        std::uint64_t pixel_shader,
        const VertexInputLayoutDesc& input_layout,
        std::uint64_t& out_handle)
    {
        if (device_ == nullptr || context_ == nullptr || next_triangle_handle_ == UINT64_MAX) {
            return false;
        }

        const auto found_vertex_shader = vertex_shader_resources_.find(vertex_shader);
        const auto found_pixel_shader = pixel_shader_resources_.find(pixel_shader);
        if (found_vertex_shader == vertex_shader_resources_.end() || found_pixel_shader == pixel_shader_resources_.end()) {
            return false;
        }

        std::array<D3D11_INPUT_ELEMENT_DESC, 8> input_elements{};
        if (!make_input_elements(input_layout, input_elements)) {
            return false;
        }

        TriangleResource resource{};
        resource.vertex_shader = found_vertex_shader->second.shader;
        resource.pixel_shader = found_pixel_shader->second.shader;
        HRESULT result = device_->CreateInputLayout(
            input_elements.data(),
            input_layout.element_count,
            found_vertex_shader->second.bytecode->GetBufferPointer(),
            found_vertex_shader->second.bytecode->GetBufferSize(),
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
        std::cerr << "[ZENO][native] shader-backed triangle resource created\n";
        return true;
    }

    RenderCommandResult draw_triangle(std::uint64_t handle)
    {
        return draw_triangle_transformed(handle, identity_matrix());
    }

    bool set_camera_matrix(const Matrix4x4& matrix)
    {
        if (context_ == nullptr || triangle_transform_buffer_ == nullptr) {
            return false;
        }

        view_projection_matrix_ = matrix;
        return true;
    }

    RenderCommandResult draw_triangle_transformed(std::uint64_t handle, const Matrix4x4& world_matrix)
    {
        const auto found = triangle_resources_.find(handle);
        if (found == triangle_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return RenderCommandResult::wrong_state;
        }

        const TriangleResource& resource = found->second;
        TriangleTransformConstants constants{};
        for (int i = 0; i < 16; ++i) {
            constants.world[i] = world_matrix.elements[i];
            constants.view_projection[i] = view_projection_matrix_.elements[i];
        }
        context_->UpdateSubresource(triangle_transform_buffer_.Get(), 0, nullptr, &constants, 0, 0);

        constexpr UINT stride = sizeof(TriangleVertex);
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { resource.vertex_buffer.Get() };
        ID3D11Buffer* constant_buffers[] = { triangle_transform_buffer_.Get() };
        context_->IASetInputLayout(resource.input_layout.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(resource.vertex_shader.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShader(resource.pixel_shader.Get(), nullptr, 0);
        context_->Draw(3, 0);
        return RenderCommandResult::ok;
    }

    bool present()
    {
        if (swap_chain_ == nullptr || !frame_active_) {
            return false;
        }

        const bool presented = SUCCEEDED(swap_chain_->Present(1, 0));
        frame_active_ = false;
        return presented;
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> triangle_transform_buffer_;
    D3D11_VIEWPORT viewport_{};
    bool frame_active_ = false;
    Matrix4x4 view_projection_matrix_ = identity_matrix();
    std::unordered_map<std::uint64_t, std::array<float, 4>> clear_colors_;
    std::unordered_map<std::uint64_t, TriangleResource> triangle_resources_;
    std::unordered_map<std::uint64_t, VertexShaderResource> vertex_shader_resources_;
    std::unordered_map<std::uint64_t, PixelShaderResource> pixel_shader_resources_;
    std::uint64_t next_clear_color_handle_ = 1;
    std::uint64_t next_triangle_handle_ = 1;
    std::uint64_t next_vertex_shader_handle_ = 1;
    std::uint64_t next_pixel_shader_handle_ = 1;
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

    std::copy(std::begin(current_keys_), std::end(current_keys_), std::begin(previous_keys_));
    std::copy(std::begin(current_mouse_buttons_), std::end(current_mouse_buttons_), std::begin(previous_mouse_buttons_));
    frame_mouse_wheel_delta_ = 0;

    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    frame_mouse_wheel_delta_ = pending_mouse_wheel_delta_;
    pending_mouse_wheel_delta_ = 0;
    out_should_close = should_close_;
    return true;
}

bool NativeBackend::get_input_snapshot(InputSnapshot& out_snapshot) const
{
    if (!initialized_) {
        return false;
    }

    for (std::uint32_t i = 0; i < kInputKeyCount; ++i) {
        out_snapshot.key_down[i] = current_keys_[i];
        out_snapshot.key_pressed[i] = current_keys_[i] && !previous_keys_[i];
        out_snapshot.key_released[i] = !current_keys_[i] && previous_keys_[i];
    }

    for (std::uint32_t i = 0; i < kInputMouseButtonCount; ++i) {
        out_snapshot.mouse_down[i] = current_mouse_buttons_[i];
        out_snapshot.mouse_pressed[i] = current_mouse_buttons_[i] && !previous_mouse_buttons_[i];
        out_snapshot.mouse_released[i] = !current_mouse_buttons_[i] && previous_mouse_buttons_[i];
    }

    out_snapshot.mouse_x = mouse_x_;
    out_snapshot.mouse_y = mouse_y_;
    out_snapshot.mouse_wheel_delta = frame_mouse_wheel_delta_;
    return true;
}

bool NativeBackend::debug_set_key_state(std::uint32_t key_code, bool is_down)
{
    if (key_code == 0 || key_code >= kInputKeyCount) {
        return false;
    }

    current_keys_[key_code] = is_down;
    return true;
}

bool NativeBackend::debug_set_mouse_state(
    std::int32_t x,
    std::int32_t y,
    std::uint32_t button,
    bool is_down,
    std::int32_t wheel_delta)
{
    if (button >= kInputMouseButtonCount) {
        return false;
    }

    mouse_x_ = x;
    mouse_y_ = y;
    current_mouse_buttons_[button] = is_down;
    frame_mouse_wheel_delta_ += wheel_delta;
    return true;
}

void NativeBackend::handle_key_message(std::uint32_t key_code, bool is_down)
{
    if (key_code == 0 || key_code >= kInputKeyCount) {
        return;
    }

    current_keys_[key_code] = is_down;
}

void NativeBackend::handle_mouse_move(std::int32_t x, std::int32_t y)
{
    mouse_x_ = x;
    mouse_y_ = y;
}

void NativeBackend::handle_mouse_button(std::uint32_t button, bool is_down)
{
    if (button >= kInputMouseButtonCount) {
        return;
    }

    current_mouse_buttons_[button] = is_down;
}

void NativeBackend::handle_mouse_wheel(std::int32_t wheel_delta)
{
    pending_mouse_wheel_delta_ += wheel_delta;
}

void NativeBackend::clear_input_state()
{
    std::fill(std::begin(current_keys_), std::end(current_keys_), false);
    std::fill(std::begin(current_mouse_buttons_), std::end(current_mouse_buttons_), false);
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

RenderCommandResult NativeBackend::clear_with_resource(std::uint64_t handle)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->clear_with_resource(handle);
}

bool NativeBackend::create_triangle(std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_triangle(out_handle);
}

bool NativeBackend::create_triangle_with_shaders(
    std::uint64_t vertex_shader,
    std::uint64_t pixel_shader,
    const VertexInputLayoutDesc& input_layout,
    std::uint64_t& out_handle)
{
    return renderer_ != nullptr
        && renderer_->create_triangle_with_shaders(vertex_shader, pixel_shader, input_layout, out_handle);
}

bool NativeBackend::destroy_triangle(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_triangle(handle);
}

bool NativeBackend::create_vertex_shader_from_source(
    const char* source,
    std::uint64_t source_length,
    const char* entry,
    std::uint64_t entry_length,
    const char* profile,
    std::uint64_t profile_length,
    ShaderCompileLog& compile_log,
    std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_vertex_shader_from_source(
        source,
        source_length,
        entry,
        entry_length,
        profile,
        profile_length,
        compile_log,
        out_handle);
}

bool NativeBackend::create_pixel_shader_from_source(
    const char* source,
    std::uint64_t source_length,
    const char* entry,
    std::uint64_t entry_length,
    const char* profile,
    std::uint64_t profile_length,
    ShaderCompileLog& compile_log,
    std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_pixel_shader_from_source(
        source,
        source_length,
        entry,
        entry_length,
        profile,
        profile_length,
        compile_log,
        out_handle);
}

bool NativeBackend::destroy_vertex_shader(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_vertex_shader(handle);
}

bool NativeBackend::destroy_pixel_shader(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_pixel_shader(handle);
}

RenderCommandResult NativeBackend::draw_triangle(std::uint64_t handle)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_triangle(handle);
}

bool NativeBackend::set_camera_matrix(const Matrix4x4& matrix)
{
    return renderer_ != nullptr && renderer_->set_camera_matrix(matrix);
}

RenderCommandResult NativeBackend::draw_triangle_transformed(std::uint64_t handle, const Matrix4x4& model_matrix)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_triangle_transformed(handle, model_matrix);
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

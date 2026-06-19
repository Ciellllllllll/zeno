#include "native_backend.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <objbase.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <xaudio2.h>

#include <array>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

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

constexpr char kSpriteShaderSource[] = R"(
cbuffer SpriteConstants : register(b0) {
    row_major float4x4 u_world;
    row_major float4x4 u_view_projection;
    float4 u_color;
};

Texture2D u_texture : register(t0);
SamplerState u_sampler : register(s0);

struct VSInput {
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput vs_main(VSInput input) {
    PSInput output;
    output.position = mul(mul(float4(input.position, 1.0), u_world), u_view_projection);
    output.uv = input.uv;
    return output;
}

float4 ps_main(PSInput input) : SV_TARGET {
    return u_texture.Sample(u_sampler, input.uv) * u_color;
}
)";

constexpr char kMeshShaderSource[] = R"(
cbuffer MeshConstants : register(b0) {
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

constexpr char kDebugLineShaderSource[] = R"(
cbuffer DebugLineConstants : register(b0) {
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

struct SpriteVertex final {
    float position[3];
    float uv[2];
};

struct DebugLineVertex final {
    float position[3];
    float color[4];
};

std::array<std::uint8_t, 7> debug_glyph_rows(char input)
{
    const char c = input >= 'a' && input <= 'z' ? static_cast<char>(input - 'a' + 'A') : input;
    switch (c) {
    case '0': return { 0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e };
    case '1': return { 0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e };
    case '2': return { 0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f };
    case '3': return { 0x1f, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0e };
    case '4': return { 0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02 };
    case '5': return { 0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e };
    case '6': return { 0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e };
    case '7': return { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 };
    case '8': return { 0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e };
    case '9': return { 0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c };
    case 'A': return { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };
    case 'B': return { 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e };
    case 'C': return { 0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e };
    case 'D': return { 0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e };
    case 'E': return { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f };
    case 'F': return { 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10 };
    case 'G': return { 0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0f };
    case 'H': return { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };
    case 'I': return { 0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e };
    case 'J': return { 0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0c };
    case 'K': return { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 };
    case 'L': return { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f };
    case 'M': return { 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11 };
    case 'N': return { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 };
    case 'O': return { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };
    case 'P': return { 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10 };
    case 'Q': return { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d };
    case 'R': return { 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11 };
    case 'S': return { 0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e };
    case 'T': return { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };
    case 'U': return { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };
    case 'V': return { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04 };
    case 'W': return { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11 };
    case 'X': return { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 };
    case 'Y': return { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 };
    case 'Z': return { 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f };
    case '-': return { 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00 };
    case ':': return { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00 };
    case '.': return { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x0c };
    case '/': return { 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10 };
    default: return { 0, 0, 0, 0, 0, 0, 0 };
    }
}

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

struct TextureResource final {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct MeshResource final {
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
    std::uint32_t vertex_stride_bytes = 0;
    std::uint32_t index_count = 0;
};

struct MaterialResource final {
    std::uint32_t kind = 0;
    std::uint32_t depth_mode = 0;
    std::uint64_t texture = 0;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
};

struct TriangleTransformConstants final {
    float world[16];
    float view_projection[16];
};

struct SpriteConstants final {
    float world[16];
    float view_projection[16];
    float color[4];
};

static_assert(sizeof(TriangleTransformConstants) == 128);
static_assert(sizeof(SpriteConstants) == 144);

Matrix4x4 identity_matrix()
{
    Matrix4x4 matrix{};
    matrix.elements[0] = 1.0f;
    matrix.elements[5] = 1.0f;
    matrix.elements[10] = 1.0f;
    matrix.elements[15] = 1.0f;
    return matrix;
}

bool all_finite(const float* values, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i) {
        if (!std::isfinite(values[i])) {
            return false;
        }
    }

    return true;
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
        case WM_SIZE:
            backend->handle_window_resize(
                static_cast<std::uint32_t>(LOWORD(lparam)),
                static_cast<std::uint32_t>(HIWORD(lparam)),
                wparam == SIZE_MINIMIZED);
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

std::uint16_t read_u16_le(const std::uint8_t* data)
{
    return static_cast<std::uint16_t>(data[0] | (data[1] << 8));
}

std::uint32_t read_u32_le(const std::uint8_t* data)
{
    return static_cast<std::uint32_t>(data[0]
        | (data[1] << 8)
        | (data[2] << 16)
        | (data[3] << 24));
}

struct ParsedWav final {
    WAVEFORMATEX format{};
    std::vector<std::uint8_t> data{};
};

bool parse_pcm_wav(const std::uint8_t* bytes, std::uint64_t byte_count, ParsedWav& out_wav)
{
    if (bytes == nullptr || byte_count < 44 || byte_count > static_cast<std::uint64_t>(SIZE_MAX)) {
        return false;
    }

    const auto size = static_cast<std::size_t>(byte_count);
    if (std::memcmp(bytes, "RIFF", 4) != 0 || std::memcmp(bytes + 8, "WAVE", 4) != 0) {
        return false;
    }

    bool saw_format = false;
    bool saw_data = false;
    WAVEFORMATEX format{};
    std::vector<std::uint8_t> sample_data;
    std::size_t offset = 12;
    while (offset + 8 <= size) {
        const std::uint8_t* chunk = bytes + offset;
        const std::uint32_t chunk_size = read_u32_le(chunk + 4);
        offset += 8;
        if (chunk_size > size - offset) {
            return false;
        }

        if (std::memcmp(chunk, "fmt ", 4) == 0) {
            if (chunk_size < 16) {
                return false;
            }

            const std::uint16_t format_tag = read_u16_le(bytes + offset);
            const std::uint16_t channels = read_u16_le(bytes + offset + 2);
            const std::uint32_t sample_rate = read_u32_le(bytes + offset + 4);
            const std::uint32_t avg_bytes_per_sec = read_u32_le(bytes + offset + 8);
            const std::uint16_t block_align = read_u16_le(bytes + offset + 12);
            const std::uint16_t bits_per_sample = read_u16_le(bytes + offset + 14);
            if (format_tag != WAVE_FORMAT_PCM
                || (channels != 1 && channels != 2)
                || sample_rate < 8000
                || sample_rate > 96000
                || (bits_per_sample != 8 && bits_per_sample != 16)) {
                return false;
            }

            const std::uint16_t expected_block_align = static_cast<std::uint16_t>((channels * bits_per_sample) / 8);
            if (block_align != expected_block_align || avg_bytes_per_sec != sample_rate * block_align) {
                return false;
            }

            format.wFormatTag = WAVE_FORMAT_PCM;
            format.nChannels = channels;
            format.nSamplesPerSec = sample_rate;
            format.nAvgBytesPerSec = avg_bytes_per_sec;
            format.nBlockAlign = block_align;
            format.wBitsPerSample = bits_per_sample;
            format.cbSize = 0;
            saw_format = true;
        } else if (std::memcmp(chunk, "data", 4) == 0) {
            if (chunk_size == 0) {
                return false;
            }
            sample_data.assign(bytes + offset, bytes + offset + chunk_size);
            saw_data = true;
        }

        offset += chunk_size + (chunk_size & 1u);
    }

    if (!saw_format || !saw_data || sample_data.size() % format.nBlockAlign != 0) {
        return false;
    }

    out_wav.format = format;
    out_wav.data = std::move(sample_data);
    return true;
}

} // namespace

struct AudioSystem final {
    struct SoundResource final {
        WAVEFORMATEX format{};
        std::vector<std::uint8_t> data{};
        IXAudio2SourceVoice* voice = nullptr;
        float volume = 1.0f;
    };

    struct EngineResource final {
        Microsoft::WRL::ComPtr<IXAudio2> engine;
        IXAudio2MasteringVoice* mastering_voice = nullptr;
        std::unordered_map<std::uint64_t, SoundResource> sounds;
        std::uint64_t next_sound_handle = 1;
    };

    ~AudioSystem()
    {
        shutdown();
    }

    void shutdown()
    {
        for (auto& entry : audio_engines) {
            destroy_engine_resource(entry.second);
        }
        audio_engines.clear();
    }

    AudioCommandResult create_engine(std::uint64_t& out_handle)
    {
        if (next_audio_handle == UINT64_MAX) {
            return AudioCommandResult::backend_error;
        }

        EngineResource resource{};
        HRESULT result = XAudio2Create(resource.engine.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(result)) {
            return AudioCommandResult::backend_error;
        }

        result = resource.engine->CreateMasteringVoice(&resource.mastering_voice);
        if (FAILED(result)) {
            return AudioCommandResult::backend_error;
        }

        const std::uint64_t handle = next_audio_handle++;
        audio_engines.emplace(handle, std::move(resource));
        out_handle = handle;
        return AudioCommandResult::ok;
    }

    bool destroy_engine(std::uint64_t handle)
    {
        const auto found = audio_engines.find(handle);
        if (found == audio_engines.end()) {
            return false;
        }

        destroy_engine_resource(found->second);
        audio_engines.erase(found);
        return true;
    }

    AudioCommandResult create_sound(
        std::uint64_t audio,
        const std::uint8_t* wav_bytes,
        std::uint64_t wav_byte_count,
        std::uint64_t& out_handle)
    {
        const auto found = audio_engines.find(audio);
        if (found == audio_engines.end()) {
            return AudioCommandResult::missing_resource;
        }

        if (found->second.next_sound_handle == UINT64_MAX) {
            return AudioCommandResult::backend_error;
        }

        ParsedWav wav{};
        if (!parse_pcm_wav(wav_bytes, wav_byte_count, wav)) {
            return AudioCommandResult::invalid_argument;
        }

        SoundResource sound{};
        sound.format = wav.format;
        sound.data = std::move(wav.data);
        const HRESULT result = found->second.engine->CreateSourceVoice(&sound.voice, &sound.format);
        if (FAILED(result) || sound.voice == nullptr) {
            return AudioCommandResult::backend_error;
        }

        const std::uint64_t handle = found->second.next_sound_handle++;
        found->second.sounds.emplace(handle, std::move(sound));
        out_handle = handle;
        return AudioCommandResult::ok;
    }

    bool destroy_sound(std::uint64_t audio, std::uint64_t sound)
    {
        const auto found_audio = audio_engines.find(audio);
        if (found_audio == audio_engines.end()) {
            return false;
        }

        const auto found_sound = found_audio->second.sounds.find(sound);
        if (found_sound == found_audio->second.sounds.end()) {
            return false;
        }

        destroy_sound_resource(found_sound->second);
        found_audio->second.sounds.erase(found_sound);
        return true;
    }

    AudioCommandResult play_sound(std::uint64_t audio, std::uint64_t sound)
    {
        SoundResource* resource = find_sound(audio, sound);
        if (resource == nullptr) {
            return AudioCommandResult::missing_resource;
        }

        resource->voice->Stop(0);
        resource->voice->FlushSourceBuffers();
        XAUDIO2_BUFFER buffer{};
        buffer.AudioBytes = static_cast<UINT32>(resource->data.size());
        buffer.pAudioData = resource->data.data();
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        HRESULT result = resource->voice->SubmitSourceBuffer(&buffer);
        if (FAILED(result)) {
            return AudioCommandResult::backend_error;
        }

        result = resource->voice->Start(0);
        return SUCCEEDED(result) ? AudioCommandResult::ok : AudioCommandResult::backend_error;
    }

    AudioCommandResult stop_sound(std::uint64_t audio, std::uint64_t sound)
    {
        SoundResource* resource = find_sound(audio, sound);
        if (resource == nullptr) {
            return AudioCommandResult::missing_resource;
        }

        resource->voice->Stop(0);
        resource->voice->FlushSourceBuffers();
        return AudioCommandResult::ok;
    }

    AudioCommandResult set_sound_volume(std::uint64_t audio, std::uint64_t sound, float volume)
    {
        if (!std::isfinite(volume) || volume < 0.0f || volume > 1.0f) {
            return AudioCommandResult::invalid_argument;
        }

        SoundResource* resource = find_sound(audio, sound);
        if (resource == nullptr) {
            return AudioCommandResult::missing_resource;
        }

        const HRESULT result = resource->voice->SetVolume(volume);
        if (FAILED(result)) {
            return AudioCommandResult::backend_error;
        }

        resource->volume = volume;
        return AudioCommandResult::ok;
    }

private:
    static void destroy_sound_resource(SoundResource& sound)
    {
        if (sound.voice != nullptr) {
            sound.voice->Stop(0);
            sound.voice->FlushSourceBuffers();
            sound.voice->DestroyVoice();
            sound.voice = nullptr;
        }
    }

    static void destroy_engine_resource(EngineResource& engine)
    {
        for (auto& entry : engine.sounds) {
            destroy_sound_resource(entry.second);
        }
        engine.sounds.clear();
        if (engine.mastering_voice != nullptr) {
            engine.mastering_voice->DestroyVoice();
            engine.mastering_voice = nullptr;
        }
        engine.engine.Reset();
    }

    SoundResource* find_sound(std::uint64_t audio, std::uint64_t sound)
    {
        const auto found_audio = audio_engines.find(audio);
        if (found_audio == audio_engines.end()) {
            return nullptr;
        }

        const auto found_sound = found_audio->second.sounds.find(sound);
        if (found_sound == found_audio->second.sounds.end()) {
            return nullptr;
        }

        return &found_sound->second;
    }

    std::unordered_map<std::uint64_t, EngineResource> audio_engines;
    std::uint64_t next_audio_handle = 1;
};

class DirectX11Renderer final : public RendererBackend {
public:
    RendererBackendKind kind() const override
    {
        return RendererBackendKind::directx11;
    }

    RendererBackendCapabilities capabilities() const override
    {
        return renderer_backend_capabilities(kind());
    }

    bool initialize(HWND window) override
    {
        if (window == nullptr || swap_chain_ != nullptr) {
            return false;
        }

        HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(result)) {
            com_initialized_ = true;
        } else if (result != RPC_E_CHANGED_MODE) {
            return false;
        }

        result = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(wic_factory_.GetAddressOf()));
        if (FAILED(result)) {
            shutdown();
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

        result = D3D11CreateDeviceAndSwapChain(
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

        if (!create_back_buffer_resources()) {
            shutdown();
            return false;
        }

        D3D11_DEPTH_STENCIL_DESC depth_state_desc{};
        depth_state_desc.DepthEnable = TRUE;
        depth_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth_state_desc.DepthFunc = D3D11_COMPARISON_LESS;
        result = device_->CreateDepthStencilState(&depth_state_desc, depth_stencil_state_.GetAddressOf());
        if (FAILED(result)) {
            shutdown();
            return false;
        }

        D3D11_DEPTH_STENCIL_DESC disabled_depth_state_desc{};
        disabled_depth_state_desc.DepthEnable = FALSE;
        disabled_depth_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        disabled_depth_state_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        result = device_->CreateDepthStencilState(
            &disabled_depth_state_desc,
            disabled_depth_stencil_state_.GetAddressOf());
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

    void shutdown() override
    {
        if (context_ != nullptr) {
            context_->ClearState();
        }

        frame_active_ = false;
        material_resources_.clear();
        mesh_resources_.clear();
        debug_line_blend_state_.Reset();
        debug_line_constant_buffer_.Reset();
        debug_line_input_layout_.Reset();
        debug_line_pixel_shader_.Reset();
        debug_line_vertex_shader_.Reset();
        mesh_input_layout_.Reset();
        mesh_pixel_shader_.Reset();
        mesh_vertex_shader_.Reset();
        texture_resources_.clear();
        sprite_blend_state_.Reset();
        sprite_sampler_state_.Reset();
        sprite_constant_buffer_.Reset();
        sprite_vertex_buffer_.Reset();
        sprite_input_layout_.Reset();
        sprite_pixel_shader_.Reset();
        sprite_vertex_shader_.Reset();
        triangle_transform_buffer_.Reset();
        disabled_depth_stencil_state_.Reset();
        depth_stencil_state_.Reset();
        depth_stencil_view_.Reset();
        depth_texture_.Reset();
        render_target_view_.Reset();
        triangle_resources_.clear();
        pixel_shader_resources_.clear();
        vertex_shader_resources_.clear();
        swap_chain_.Reset();
        context_.Reset();
        device_.Reset();
        wic_factory_.Reset();
        if (com_initialized_) {
            CoUninitialize();
            com_initialized_ = false;
        }
    }

    bool begin_frame() override
    {
        if (renderer_failed_ || !apply_pending_resize()) {
            return false;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || depth_stencil_view_ == nullptr || frame_active_) {
            return false;
        }

        ID3D11RenderTargetView* render_targets[] = { render_target_view_.Get() };
        context_->OMSetRenderTargets(1, render_targets, depth_stencil_view_.Get());
        context_->ClearDepthStencilView(depth_stencil_view_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        context_->RSSetViewports(1, &viewport_);
        frame_active_ = true;
        return true;
    }

    bool clear(float r, float g, float b, float a) override
    {
        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return false;
        }

        const float color[] = { r, g, b, a };
        context_->ClearRenderTargetView(render_target_view_.Get(), color);
        return true;
    }

    bool create_clear_color(float r, float g, float b, float a, std::uint64_t& out_handle) override
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

    bool destroy_clear_color(std::uint64_t handle) override
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

    RenderCommandResult clear_with_resource(std::uint64_t handle) override
    {
        const auto found = clear_colors_.find(handle);
        if (found == clear_colors_.end()) {
            return RenderCommandResult::missing_resource;
        }

        return clear(found->second[0], found->second[1], found->second[2], found->second[3])
            ? RenderCommandResult::ok
            : RenderCommandResult::wrong_state;
    }

    bool create_triangle(std::uint64_t& out_handle) override
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
        std::uint64_t& out_handle) override
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
        std::uint64_t& out_handle) override
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

    bool destroy_vertex_shader(std::uint64_t handle) override
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

    bool destroy_pixel_shader(std::uint64_t handle) override
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

    bool create_texture_from_memory(const std::uint8_t* image_bytes, std::uint64_t image_byte_count, std::uint64_t& out_handle) override
    {
        if (device_ == nullptr || context_ == nullptr || wic_factory_ == nullptr || next_texture_handle_ == UINT64_MAX) {
            return false;
        }

        if (image_bytes == nullptr || image_byte_count == 0 || image_byte_count > UINT_MAX) {
            return false;
        }

        Microsoft::WRL::ComPtr<IWICStream> stream;
        HRESULT result = wic_factory_->CreateStream(stream.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = stream->InitializeFromMemory(
            const_cast<BYTE*>(reinterpret_cast<const BYTE*>(image_bytes)),
            static_cast<DWORD>(image_byte_count));
        if (FAILED(result)) {
            return false;
        }

        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        result = wic_factory_->CreateDecoderFromStream(
            stream.Get(),
            nullptr,
            WICDecodeMetadataCacheOnLoad,
            decoder.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        result = decoder->GetFrame(0, frame.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        UINT width = 0;
        UINT height = 0;
        result = frame->GetSize(&width, &height);
        if (FAILED(result) || width == 0 || height == 0) {
            return false;
        }

        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        result = wic_factory_->CreateFormatConverter(converter.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(result)) {
            return false;
        }

        if (width > UINT_MAX / 4) {
            return false;
        }

        const UINT stride = width * 4;
        if (height > UINT_MAX / stride) {
            return false;
        }

        const UINT byte_count = stride * height;
        std::vector<std::uint8_t> pixels(byte_count);
        result = converter->CopyPixels(nullptr, stride, byte_count, pixels.data());
        if (FAILED(result)) {
            return false;
        }

        D3D11_TEXTURE2D_DESC texture_desc{};
        texture_desc.Width = width;
        texture_desc.Height = height;
        texture_desc.MipLevels = 1;
        texture_desc.ArraySize = 1;
        texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initial_data{};
        initial_data.pSysMem = pixels.data();
        initial_data.SysMemPitch = stride;

        TextureResource resource{};
        resource.width = width;
        resource.height = height;
        result = device_->CreateTexture2D(&texture_desc, &initial_data, resource.texture.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = device_->CreateShaderResourceView(resource.texture.Get(), nullptr, resource.shader_resource_view.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        const std::uint64_t handle = next_texture_handle_++;
        if (handle == 0) {
            return false;
        }

        texture_resources_.emplace(handle, std::move(resource));
        out_handle = handle;
        std::cerr << "[ZENO][native] texture resource created\n";
        return true;
    }

    bool destroy_texture(std::uint64_t handle) override
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = texture_resources_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] texture resource destroyed\n";
        }

        return destroyed;
    }

    RenderCommandResult create_material(const MaterialDesc& desc, std::uint64_t& out_handle) override
    {
        if (device_ == nullptr || context_ == nullptr || next_material_handle_ == UINT64_MAX) {
            return RenderCommandResult::wrong_state;
        }

        if (desc.kind == 1) {
            if (texture_resources_.find(desc.texture) == texture_resources_.end()) {
                return RenderCommandResult::missing_resource;
            }
        } else if (desc.kind == 2) {
            if (desc.texture != 0) {
                return RenderCommandResult::missing_resource;
            }
        } else {
            return RenderCommandResult::wrong_state;
        }

        MaterialResource resource{};
        resource.kind = desc.kind;
        resource.depth_mode = desc.depth_mode;
        resource.texture = desc.texture;

        D3D11_BLEND_DESC blend_desc{};
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (desc.blend_mode == 2) {
            blend_desc.RenderTarget[0].BlendEnable = TRUE;
            blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        }
        HRESULT result = device_->CreateBlendState(&blend_desc, resource.blend_state.GetAddressOf());
        if (FAILED(result)) {
            return RenderCommandResult::wrong_state;
        }

        D3D11_RASTERIZER_DESC rasterizer_desc{};
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = desc.cull_mode == 2 ? D3D11_CULL_BACK : D3D11_CULL_NONE;
        rasterizer_desc.DepthClipEnable = TRUE;
        result = device_->CreateRasterizerState(&rasterizer_desc, resource.rasterizer_state.GetAddressOf());
        if (FAILED(result)) {
            return RenderCommandResult::wrong_state;
        }

        const std::uint64_t handle = next_material_handle_++;
        if (handle == 0) {
            return RenderCommandResult::wrong_state;
        }

        material_resources_.emplace(handle, std::move(resource));
        out_handle = handle;
        std::cerr << "[ZENO][native] material resource created\n";
        return RenderCommandResult::ok;
    }

    bool destroy_material(std::uint64_t handle) override
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = material_resources_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] material resource destroyed\n";
        }

        return destroyed;
    }

    bool create_mesh(const MeshDesc& desc, std::uint64_t& out_handle) override
    {
        if (device_ == nullptr || context_ == nullptr || next_mesh_handle_ == UINT64_MAX) {
            return false;
        }

        if (desc.vertex_data == nullptr
            || desc.index_data == nullptr
            || desc.vertex_count == 0
            || desc.index_count == 0
            || desc.vertex_stride_bytes < sizeof(float) * 7
            || desc.vertex_stride_bytes % sizeof(float) != 0
            || desc.vertex_count > UINT32_MAX
            || desc.index_count > UINT32_MAX
            || desc.vertex_count > UINT_MAX / desc.vertex_stride_bytes
            || desc.index_count > UINT_MAX / sizeof(std::uint32_t)) {
            return false;
        }

        if (!ensure_mesh_pipeline()) {
            return false;
        }

        D3D11_BUFFER_DESC vertex_buffer_desc{};
        vertex_buffer_desc.ByteWidth = static_cast<UINT>(desc.vertex_count * desc.vertex_stride_bytes);
        vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vertex_initial_data{};
        vertex_initial_data.pSysMem = desc.vertex_data;

        MeshResource resource{};
        HRESULT result = device_->CreateBuffer(
            &vertex_buffer_desc,
            &vertex_initial_data,
            resource.vertex_buffer.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        D3D11_BUFFER_DESC index_buffer_desc{};
        index_buffer_desc.ByteWidth = static_cast<UINT>(desc.index_count * sizeof(std::uint32_t));
        index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA index_initial_data{};
        index_initial_data.pSysMem = desc.index_data;
        result = device_->CreateBuffer(
            &index_buffer_desc,
            &index_initial_data,
            resource.index_buffer.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        const std::uint64_t handle = next_mesh_handle_++;
        if (handle == 0) {
            return false;
        }

        resource.vertex_stride_bytes = desc.vertex_stride_bytes;
        resource.index_count = static_cast<std::uint32_t>(desc.index_count);
        mesh_resources_.emplace(handle, std::move(resource));
        out_handle = handle;
        std::cerr << "[ZENO][native] mesh resource created\n";
        return true;
    }

    bool destroy_mesh(std::uint64_t handle) override
    {
        if (handle == 0) {
            return false;
        }

        const bool destroyed = mesh_resources_.erase(handle) == 1;
        if (destroyed) {
            std::cerr << "[ZENO][native] mesh resource destroyed\n";
        }

        return destroyed;
    }

    bool create_triangle_with_shaders(
        std::uint64_t vertex_shader,
        std::uint64_t pixel_shader,
        const VertexInputLayoutDesc& input_layout,
        std::uint64_t& out_handle) override
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

    bool ensure_sprite_pipeline()
    {
        if (sprite_vertex_shader_ != nullptr) {
            return true;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
        HRESULT result = D3DCompile(
            kSpriteShaderSource,
            sizeof(kSpriteShaderSource) - 1,
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
            kSpriteShaderSource,
            sizeof(kSpriteShaderSource) - 1,
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

        result = device_->CreateVertexShader(
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            nullptr,
            sprite_vertex_shader_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = device_->CreatePixelShader(
            pixel_shader_blob->GetBufferPointer(),
            pixel_shader_blob->GetBufferSize(),
            nullptr,
            sprite_pixel_shader_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        constexpr D3D11_INPUT_ELEMENT_DESC input_elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(SpriteVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SpriteVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        result = device_->CreateInputLayout(
            input_elements,
            2,
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            sprite_input_layout_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        constexpr SpriteVertex vertices[] = {
            { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f } },
            { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f } },
            { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } },
        };
        D3D11_BUFFER_DESC vertex_buffer_desc{};
        vertex_buffer_desc.ByteWidth = sizeof(vertices);
        vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vertex_initial_data{};
        vertex_initial_data.pSysMem = vertices;
        result = device_->CreateBuffer(&vertex_buffer_desc, &vertex_initial_data, sprite_vertex_buffer_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        D3D11_BUFFER_DESC constant_buffer_desc{};
        constant_buffer_desc.ByteWidth = sizeof(SpriteConstants);
        constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        result = device_->CreateBuffer(&constant_buffer_desc, nullptr, sprite_constant_buffer_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        D3D11_SAMPLER_DESC sampler_desc{};
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        result = device_->CreateSamplerState(&sampler_desc, sprite_sampler_state_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        D3D11_BLEND_DESC blend_desc{};
        blend_desc.RenderTarget[0].BlendEnable = TRUE;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        result = device_->CreateBlendState(&blend_desc, sprite_blend_state_.GetAddressOf());
        return SUCCEEDED(result);
    }

    bool ensure_mesh_pipeline()
    {
        if (mesh_vertex_shader_ != nullptr) {
            return true;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

        HRESULT result = D3DCompile(
            kMeshShaderSource,
            sizeof(kMeshShaderSource) - 1,
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
            kMeshShaderSource,
            sizeof(kMeshShaderSource) - 1,
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

        result = device_->CreateVertexShader(
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            nullptr,
            mesh_vertex_shader_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = device_->CreatePixelShader(
            pixel_shader_blob->GetBufferPointer(),
            pixel_shader_blob->GetBufferSize(),
            nullptr,
            mesh_pixel_shader_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        constexpr D3D11_INPUT_ELEMENT_DESC input_elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        result = device_->CreateInputLayout(
            input_elements,
            2,
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            mesh_input_layout_.GetAddressOf());
        return SUCCEEDED(result);
    }

    RenderCommandResult draw_sprite(std::uint64_t texture, const SpriteDrawDesc& desc) override
    {
        const auto found = texture_resources_.find(texture);
        if (found == texture_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return RenderCommandResult::wrong_state;
        }

        if (!ensure_sprite_pipeline()) {
            return RenderCommandResult::wrong_state;
        }

        SpriteConstants constants{};
        for (int i = 0; i < 16; ++i) {
            constants.world[i] = desc.model_matrix.elements[i];
            constants.view_projection[i] = view_projection_matrix_.elements[i];
        }
        for (int i = 0; i < 4; ++i) {
            constants.color[i] = desc.color[i];
        }

        context_->UpdateSubresource(sprite_constant_buffer_.Get(), 0, nullptr, &constants, 0, 0);

        constexpr UINT stride = sizeof(SpriteVertex);
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { sprite_vertex_buffer_.Get() };
        ID3D11Buffer* constant_buffers[] = { sprite_constant_buffer_.Get() };
        ID3D11ShaderResourceView* shader_resource_views[] = { found->second.shader_resource_view.Get() };
        ID3D11SamplerState* samplers[] = { sprite_sampler_state_.Get() };
        float blend_factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

        context_->IASetInputLayout(sprite_input_layout_.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(sprite_vertex_shader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShader(sprite_pixel_shader_.Get(), nullptr, 0);
        context_->PSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShaderResources(0, 1, shader_resource_views);
        context_->PSSetSamplers(0, 1, samplers);
        context_->OMSetDepthStencilState(disabled_depth_stencil_state_.Get(), 0);
        context_->OMSetBlendState(sprite_blend_state_.Get(), blend_factor, 0xffffffff);
        context_->Draw(6, 0);

        ID3D11ShaderResourceView* null_srvs[] = { nullptr };
        context_->PSSetShaderResources(0, 1, null_srvs);
        context_->OMSetBlendState(nullptr, blend_factor, 0xffffffff);
        return RenderCommandResult::ok;
    }

    RenderCommandResult draw_sprite_with_material(std::uint64_t material, const SpriteDrawDesc& desc) override
    {
        const auto found_material = material_resources_.find(material);
        if (found_material == material_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        const MaterialResource& material_resource = found_material->second;
        if (material_resource.kind != 1) {
            return RenderCommandResult::wrong_state;
        }

        const auto found_texture = texture_resources_.find(material_resource.texture);
        if (found_texture == texture_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return RenderCommandResult::wrong_state;
        }

        if (!ensure_sprite_pipeline()) {
            return RenderCommandResult::wrong_state;
        }

        SpriteConstants constants{};
        for (int i = 0; i < 16; ++i) {
            constants.world[i] = desc.model_matrix.elements[i];
            constants.view_projection[i] = view_projection_matrix_.elements[i];
        }
        for (int i = 0; i < 4; ++i) {
            constants.color[i] = desc.color[i];
        }

        context_->UpdateSubresource(sprite_constant_buffer_.Get(), 0, nullptr, &constants, 0, 0);

        constexpr UINT stride = sizeof(SpriteVertex);
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { sprite_vertex_buffer_.Get() };
        ID3D11Buffer* constant_buffers[] = { sprite_constant_buffer_.Get() };
        ID3D11ShaderResourceView* shader_resource_views[] = { found_texture->second.shader_resource_view.Get() };
        ID3D11SamplerState* samplers[] = { sprite_sampler_state_.Get() };
        float blend_factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

        context_->IASetInputLayout(sprite_input_layout_.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(sprite_vertex_shader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShader(sprite_pixel_shader_.Get(), nullptr, 0);
        context_->PSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShaderResources(0, 1, shader_resource_views);
        context_->PSSetSamplers(0, 1, samplers);
        context_->OMSetDepthStencilState(
            material_resource.depth_mode == 2 ? depth_stencil_state_.Get() : disabled_depth_stencil_state_.Get(),
            0);
        context_->OMSetBlendState(material_resource.blend_state.Get(), blend_factor, 0xffffffff);
        context_->RSSetState(material_resource.rasterizer_state.Get());
        context_->Draw(6, 0);

        ID3D11ShaderResourceView* null_srvs[] = { nullptr };
        context_->PSSetShaderResources(0, 1, null_srvs);
        context_->OMSetBlendState(nullptr, blend_factor, 0xffffffff);
        context_->RSSetState(nullptr);
        return RenderCommandResult::ok;
    }

    RenderCommandResult draw_mesh(std::uint64_t mesh, const Matrix4x4& model_matrix) override
    {
        const auto found = mesh_resources_.find(mesh);
        if (found == mesh_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return RenderCommandResult::wrong_state;
        }

        if (!ensure_mesh_pipeline()) {
            return RenderCommandResult::wrong_state;
        }

        TriangleTransformConstants constants{};
        for (int i = 0; i < 16; ++i) {
            constants.world[i] = model_matrix.elements[i];
            constants.view_projection[i] = view_projection_matrix_.elements[i];
        }
        context_->UpdateSubresource(triangle_transform_buffer_.Get(), 0, nullptr, &constants, 0, 0);

        const MeshResource& resource = found->second;
        const UINT stride = resource.vertex_stride_bytes;
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { resource.vertex_buffer.Get() };
        ID3D11Buffer* constant_buffers[] = { triangle_transform_buffer_.Get() };
        context_->IASetInputLayout(mesh_input_layout_.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetIndexBuffer(resource.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(mesh_vertex_shader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShader(mesh_pixel_shader_.Get(), nullptr, 0);
        context_->OMSetDepthStencilState(depth_stencil_state_.Get(), 0);
        context_->OMSetBlendState(nullptr, nullptr, 0xffffffff);
        context_->DrawIndexed(resource.index_count, 0, 0);
        return RenderCommandResult::ok;
    }

    RenderCommandResult draw_mesh_with_material(std::uint64_t mesh, std::uint64_t material, const Matrix4x4& model_matrix) override
    {
        const auto found_mesh = mesh_resources_.find(mesh);
        if (found_mesh == mesh_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        const auto found_material = material_resources_.find(material);
        if (found_material == material_resources_.end()) {
            return RenderCommandResult::missing_resource;
        }

        const MaterialResource& material_resource = found_material->second;
        if (material_resource.kind != 2) {
            return RenderCommandResult::wrong_state;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return RenderCommandResult::wrong_state;
        }

        if (!ensure_mesh_pipeline()) {
            return RenderCommandResult::wrong_state;
        }

        TriangleTransformConstants constants{};
        for (int i = 0; i < 16; ++i) {
            constants.world[i] = model_matrix.elements[i];
            constants.view_projection[i] = view_projection_matrix_.elements[i];
        }
        context_->UpdateSubresource(triangle_transform_buffer_.Get(), 0, nullptr, &constants, 0, 0);

        const MeshResource& resource = found_mesh->second;
        const UINT stride = resource.vertex_stride_bytes;
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { resource.vertex_buffer.Get() };
        ID3D11Buffer* constant_buffers[] = { triangle_transform_buffer_.Get() };
        float blend_factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context_->IASetInputLayout(mesh_input_layout_.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetIndexBuffer(resource.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(mesh_vertex_shader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShader(mesh_pixel_shader_.Get(), nullptr, 0);
        context_->OMSetDepthStencilState(
            material_resource.depth_mode == 2 ? depth_stencil_state_.Get() : disabled_depth_stencil_state_.Get(),
            0);
        context_->OMSetBlendState(material_resource.blend_state.Get(), blend_factor, 0xffffffff);
        context_->RSSetState(material_resource.rasterizer_state.Get());
        context_->DrawIndexed(resource.index_count, 0, 0);
        context_->OMSetBlendState(nullptr, blend_factor, 0xffffffff);
        context_->RSSetState(nullptr);
        return RenderCommandResult::ok;
    }

    RenderCommandResult draw_debug_line(const DebugLineDesc& desc) override
    {
        if (!all_finite(desc.start, 3) || !all_finite(desc.end, 3) || !all_finite(desc.color, 4)) {
            return RenderCommandResult::wrong_state;
        }

        const DebugLineVertex vertices[] = {
            { { desc.start[0], desc.start[1], desc.start[2] }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { desc.end[0], desc.end[1], desc.end[2] }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
        };
        return draw_debug_vertices(vertices, 2);
    }

    RenderCommandResult draw_debug_rect(const DebugRectDesc& desc) override
    {
        const float values[] = {
            desc.center[0],
            desc.center[1],
            desc.half_extents[0],
            desc.half_extents[1],
            desc.z,
            desc.color[0],
            desc.color[1],
            desc.color[2],
            desc.color[3],
        };
        if (!all_finite(values, std::size(values)) || desc.half_extents[0] < 0.0f || desc.half_extents[1] < 0.0f) {
            return RenderCommandResult::wrong_state;
        }

        const float min_x = desc.center[0] - desc.half_extents[0];
        const float max_x = desc.center[0] + desc.half_extents[0];
        const float min_y = desc.center[1] - desc.half_extents[1];
        const float max_y = desc.center[1] + desc.half_extents[1];
        const DebugLineVertex vertices[] = {
            { { min_x, min_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { max_x, min_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { max_x, min_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { max_x, max_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { max_x, max_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { min_x, max_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { min_x, max_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
            { { min_x, min_y, desc.z }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } },
        };
        return draw_debug_vertices(vertices, 8);
    }

    RenderCommandResult draw_debug_text(const DebugTextDesc& desc) override
    {
        const float values[] = {
            desc.origin[0],
            desc.origin[1],
            desc.origin[2],
            desc.scale,
            desc.color[0],
            desc.color[1],
            desc.color[2],
            desc.color[3],
        };
        if (desc.text == nullptr || desc.text_length == 0 || !all_finite(values, std::size(values)) || desc.scale <= 0.0f) {
            return RenderCommandResult::wrong_state;
        }

        std::vector<DebugLineVertex> vertices;
        vertices.reserve(static_cast<std::size_t>(desc.text_length) * 5u * 7u * 8u);

        const auto add_line = [&vertices, &desc](float x0, float y0, float x1, float y1) {
            vertices.push_back({ { x0, y0, desc.origin[2] }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } });
            vertices.push_back({ { x1, y1, desc.origin[2] }, { desc.color[0], desc.color[1], desc.color[2], desc.color[3] } });
        };

        const auto add_cell = [&add_line, &desc](float x, float y) {
            const float x1 = x + desc.scale;
            const float y1 = y - desc.scale;
            add_line(x, y, x1, y);
            add_line(x1, y, x1, y1);
            add_line(x1, y1, x, y1);
            add_line(x, y1, x, y);
        };

        float cursor_x = desc.origin[0];
        float cursor_y = desc.origin[1];
        for (std::uint64_t index = 0; index < desc.text_length; ++index) {
            const char c = desc.text[index];
            if (c == '\n') {
                cursor_x = desc.origin[0];
                cursor_y -= desc.scale * 9.0f;
                continue;
            }

            const std::array<std::uint8_t, 7> rows = debug_glyph_rows(c);
            for (std::size_t row = 0; row < rows.size(); ++row) {
                for (std::size_t column = 0; column < 5; ++column) {
                    const std::uint8_t mask = static_cast<std::uint8_t>(1u << (4u - column));
                    if ((rows[row] & mask) != 0) {
                        add_cell(
                            cursor_x + static_cast<float>(column) * desc.scale,
                            cursor_y - static_cast<float>(row) * desc.scale);
                    }
                }
            }

            cursor_x += desc.scale * 6.0f;
        }

        if (vertices.empty()) {
            return RenderCommandResult::ok;
        }

        return draw_debug_vertices(vertices.data(), static_cast<std::uint32_t>(vertices.size()), identity_matrix());
    }

    RenderCommandResult draw_triangle(std::uint64_t handle) override
    {
        return draw_triangle_transformed(handle, identity_matrix());
    }

    bool set_camera_matrix(const Matrix4x4& matrix) override
    {
        if (context_ == nullptr || triangle_transform_buffer_ == nullptr) {
            return false;
        }

        view_projection_matrix_ = matrix;
        return true;
    }

    RenderCommandResult draw_triangle_transformed(std::uint64_t handle, const Matrix4x4& world_matrix) override
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
        context_->OMSetDepthStencilState(disabled_depth_stencil_state_.Get(), 0);
        context_->Draw(3, 0);
        return RenderCommandResult::ok;
    }

    bool present() override
    {
        if (swap_chain_ == nullptr || !frame_active_) {
            return false;
        }

        const HRESULT result = swap_chain_->Present(1, 0);
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
            renderer_failed_ = true;
        }

        const bool presented = SUCCEEDED(result);
        frame_active_ = false;
        return presented;
    }

    void request_resize(std::uint32_t width, std::uint32_t height, bool minimized) override
    {
        minimized_ = minimized || width == 0 || height == 0;
        if (minimized_) {
            return;
        }

        pending_width_ = width;
        pending_height_ = height;
        resize_pending_ = true;
    }

private:
    bool create_back_buffer_resources()
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
        HRESULT result = swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
        if (FAILED(result)) {
            note_device_removed(result);
            renderer_failed_ = true;
            return false;
        }

        D3D11_TEXTURE2D_DESC back_buffer_desc{};
        back_buffer->GetDesc(&back_buffer_desc);
        if (back_buffer_desc.Width == 0 || back_buffer_desc.Height == 0) {
            return false;
        }

        viewport_.TopLeftX = 0.0f;
        viewport_.TopLeftY = 0.0f;
        viewport_.Width = static_cast<float>(back_buffer_desc.Width);
        viewport_.Height = static_cast<float>(back_buffer_desc.Height);
        viewport_.MinDepth = 0.0f;
        viewport_.MaxDepth = 1.0f;

        result = device_->CreateRenderTargetView(back_buffer.Get(), nullptr, render_target_view_.GetAddressOf());
        if (FAILED(result)) {
            note_device_removed(result);
            renderer_failed_ = true;
            return false;
        }

        D3D11_TEXTURE2D_DESC depth_texture_desc{};
        depth_texture_desc.Width = back_buffer_desc.Width;
        depth_texture_desc.Height = back_buffer_desc.Height;
        depth_texture_desc.MipLevels = 1;
        depth_texture_desc.ArraySize = 1;
        depth_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth_texture_desc.SampleDesc.Count = 1;
        depth_texture_desc.Usage = D3D11_USAGE_DEFAULT;
        depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        result = device_->CreateTexture2D(&depth_texture_desc, nullptr, depth_texture_.GetAddressOf());
        if (FAILED(result)) {
            note_device_removed(result);
            renderer_failed_ = true;
            return false;
        }

        result = device_->CreateDepthStencilView(depth_texture_.Get(), nullptr, depth_stencil_view_.GetAddressOf());
        if (FAILED(result)) {
            note_device_removed(result);
            renderer_failed_ = true;
            return false;
        }

        return true;
    }

    bool apply_pending_resize()
    {
        if (minimized_ || !resize_pending_) {
            return true;
        }

        if (swap_chain_ == nullptr || context_ == nullptr || pending_width_ == 0 || pending_height_ == 0) {
            return false;
        }

        context_->OMSetRenderTargets(0, nullptr, nullptr);
        render_target_view_.Reset();
        depth_stencil_view_.Reset();
        depth_texture_.Reset();

        const HRESULT result = swap_chain_->ResizeBuffers(0, pending_width_, pending_height_, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(result)) {
            note_device_removed(result);
            renderer_failed_ = true;
            return false;
        }

        if (!create_back_buffer_resources()) {
            return false;
        }

        resize_pending_ = false;
        return true;
    }

    void note_device_removed(HRESULT result)
    {
        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
            renderer_failed_ = true;
        }
    }

    bool ensure_debug_line_pipeline()
    {
        if (debug_line_vertex_shader_ != nullptr) {
            return true;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> error_blob;

        HRESULT result = D3DCompile(
            kDebugLineShaderSource,
            sizeof(kDebugLineShaderSource) - 1,
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
            kDebugLineShaderSource,
            sizeof(kDebugLineShaderSource) - 1,
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

        result = device_->CreateVertexShader(
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            nullptr,
            debug_line_vertex_shader_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        result = device_->CreatePixelShader(
            pixel_shader_blob->GetBufferPointer(),
            pixel_shader_blob->GetBufferSize(),
            nullptr,
            debug_line_pixel_shader_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        constexpr D3D11_INPUT_ELEMENT_DESC input_elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(DebugLineVertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(DebugLineVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        result = device_->CreateInputLayout(
            input_elements,
            2,
            vertex_shader_blob->GetBufferPointer(),
            vertex_shader_blob->GetBufferSize(),
            debug_line_input_layout_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        D3D11_BUFFER_DESC constant_buffer_desc{};
        constant_buffer_desc.ByteWidth = sizeof(TriangleTransformConstants);
        constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        result = device_->CreateBuffer(&constant_buffer_desc, nullptr, debug_line_constant_buffer_.GetAddressOf());
        if (FAILED(result)) {
            return false;
        }

        D3D11_BLEND_DESC blend_desc{};
        blend_desc.RenderTarget[0].BlendEnable = TRUE;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        result = device_->CreateBlendState(&blend_desc, debug_line_blend_state_.GetAddressOf());
        return SUCCEEDED(result);
    }

    RenderCommandResult draw_debug_vertices(const DebugLineVertex* vertices, std::uint32_t vertex_count)
    {
        return draw_debug_vertices(vertices, vertex_count, view_projection_matrix_);
    }

    RenderCommandResult draw_debug_vertices(
        const DebugLineVertex* vertices,
        std::uint32_t vertex_count,
        const Matrix4x4& view_projection_matrix)
    {
        if (vertices == nullptr || vertex_count == 0) {
            return RenderCommandResult::wrong_state;
        }

        if (context_ == nullptr || render_target_view_ == nullptr || !frame_active_) {
            return RenderCommandResult::wrong_state;
        }

        if (!ensure_debug_line_pipeline()) {
            return RenderCommandResult::wrong_state;
        }

        D3D11_BUFFER_DESC vertex_buffer_desc{};
        vertex_buffer_desc.ByteWidth = sizeof(DebugLineVertex) * vertex_count;
        vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initial_data{};
        initial_data.pSysMem = vertices;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
        HRESULT result = device_->CreateBuffer(&vertex_buffer_desc, &initial_data, vertex_buffer.GetAddressOf());
        if (FAILED(result)) {
            return RenderCommandResult::wrong_state;
        }

        TriangleTransformConstants constants{};
        const Matrix4x4 identity = identity_matrix();
        for (int i = 0; i < 16; ++i) {
            constants.world[i] = identity.elements[i];
            constants.view_projection[i] = view_projection_matrix.elements[i];
        }
        context_->UpdateSubresource(debug_line_constant_buffer_.Get(), 0, nullptr, &constants, 0, 0);

        constexpr UINT stride = sizeof(DebugLineVertex);
        constexpr UINT offset = 0;
        ID3D11Buffer* vertex_buffers[] = { vertex_buffer.Get() };
        ID3D11Buffer* constant_buffers[] = { debug_line_constant_buffer_.Get() };
        float blend_factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context_->IASetInputLayout(debug_line_input_layout_.Get());
        context_->IASetVertexBuffers(0, 1, vertex_buffers, &stride, &offset);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context_->VSSetShader(debug_line_vertex_shader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constant_buffers);
        context_->PSSetShader(debug_line_pixel_shader_.Get(), nullptr, 0);
        context_->OMSetDepthStencilState(disabled_depth_stencil_state_.Get(), 0);
        context_->OMSetBlendState(debug_line_blend_state_.Get(), blend_factor, 0xffffffff);
        context_->Draw(vertex_count, 0);
        context_->OMSetBlendState(nullptr, blend_factor, 0xffffffff);
        return RenderCommandResult::ok;
    }

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_texture_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> disabled_depth_stencil_state_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> triangle_transform_buffer_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> debug_line_vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> debug_line_pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> debug_line_input_layout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> debug_line_constant_buffer_;
    Microsoft::WRL::ComPtr<ID3D11BlendState> debug_line_blend_state_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> mesh_vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> mesh_pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> mesh_input_layout_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> sprite_vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> sprite_pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> sprite_input_layout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> sprite_vertex_buffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> sprite_constant_buffer_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sprite_sampler_state_;
    Microsoft::WRL::ComPtr<ID3D11BlendState> sprite_blend_state_;
    Microsoft::WRL::ComPtr<IWICImagingFactory> wic_factory_;
    D3D11_VIEWPORT viewport_{};
    bool frame_active_ = false;
    bool com_initialized_ = false;
    bool resize_pending_ = false;
    bool minimized_ = false;
    bool renderer_failed_ = false;
    std::uint32_t pending_width_ = 0;
    std::uint32_t pending_height_ = 0;
    Matrix4x4 view_projection_matrix_ = identity_matrix();
    std::unordered_map<std::uint64_t, std::array<float, 4>> clear_colors_;
    std::unordered_map<std::uint64_t, TriangleResource> triangle_resources_;
    std::unordered_map<std::uint64_t, VertexShaderResource> vertex_shader_resources_;
    std::unordered_map<std::uint64_t, PixelShaderResource> pixel_shader_resources_;
    std::unordered_map<std::uint64_t, TextureResource> texture_resources_;
    std::unordered_map<std::uint64_t, MeshResource> mesh_resources_;
    std::unordered_map<std::uint64_t, MaterialResource> material_resources_;
    std::uint64_t next_clear_color_handle_ = 1;
    std::uint64_t next_triangle_handle_ = 1;
    std::uint64_t next_vertex_shader_handle_ = 1;
    std::uint64_t next_pixel_shader_handle_ = 1;
    std::uint64_t next_texture_handle_ = 1;
    std::uint64_t next_mesh_handle_ = 1;
    std::uint64_t next_material_handle_ = 1;
};

const char* renderer_backend_name(RendererBackendKind kind)
{
    switch (kind) {
    case RendererBackendKind::directx11:
        return "directx11";
    case RendererBackendKind::directx12:
        return "directx12";
    default:
        return "unknown";
    }
}

RendererBackendCapabilities renderer_backend_capabilities(RendererBackendKind kind)
{
    RendererBackendCapabilities capabilities{};
    capabilities.kind = kind;
    capabilities.default_backend = kind == kDefaultRendererBackendKind;

    switch (kind) {
    case RendererBackendKind::directx11:
        capabilities.support = RendererBackendSupportStatus::supported;
        capabilities.implemented = true;
        capabilities.supports_window_present = true;
        capabilities.supports_textures = true;
        capabilities.supports_meshes = true;
        capabilities.supports_materials = true;
        capabilities.supports_debug_draw = true;
        capabilities.supports_debug_text = true;
        capabilities.supports_resize = true;
        capabilities.max_vertex_input_elements = 8;
        break;
    case RendererBackendKind::directx12:
        capabilities.support = RendererBackendSupportStatus::experimental;
        capabilities.implemented = false;
        capabilities.max_vertex_input_elements = 0;
        break;
    default:
        break;
    }

    return capabilities;
}

std::unique_ptr<RendererBackend> create_renderer_backend(RendererBackendKind kind)
{
    if (kind == RendererBackendKind::directx11) {
        return std::make_unique<DirectX11Renderer>();
    }

    return nullptr;
}

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

    if (audio_system_ != nullptr) {
        audio_system_->shutdown();
        audio_system_.reset();
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

void NativeBackend::handle_window_resize(std::uint32_t width, std::uint32_t height, bool minimized)
{
    if (renderer_ != nullptr) {
        renderer_->request_resize(width, height, minimized);
    }
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

    auto renderer = create_renderer_backend(renderer_backend_kind_);
    if (renderer == nullptr) {
        return false;
    }

    renderer_capabilities_ = renderer->capabilities();
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

bool NativeBackend::create_texture_from_memory(
    const std::uint8_t* image_bytes,
    std::uint64_t image_byte_count,
    std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_texture_from_memory(image_bytes, image_byte_count, out_handle);
}

bool NativeBackend::destroy_texture(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_texture(handle);
}

RenderCommandResult NativeBackend::draw_sprite(std::uint64_t texture, const SpriteDrawDesc& desc)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_sprite(texture, desc);
}

bool NativeBackend::create_mesh(const MeshDesc& desc, std::uint64_t& out_handle)
{
    return renderer_ != nullptr && renderer_->create_mesh(desc, out_handle);
}

bool NativeBackend::destroy_mesh(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_mesh(handle);
}

RenderCommandResult NativeBackend::draw_mesh(std::uint64_t mesh, const Matrix4x4& model_matrix)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_mesh(mesh, model_matrix);
}

RenderCommandResult NativeBackend::create_material(const MaterialDesc& desc, std::uint64_t& out_handle)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->create_material(desc, out_handle);
}

bool NativeBackend::destroy_material(std::uint64_t handle)
{
    return renderer_ != nullptr && renderer_->destroy_material(handle);
}

RenderCommandResult NativeBackend::draw_sprite_with_material(std::uint64_t material, const SpriteDrawDesc& desc)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_sprite_with_material(material, desc);
}

RenderCommandResult NativeBackend::draw_mesh_with_material(
    std::uint64_t mesh,
    std::uint64_t material,
    const Matrix4x4& model_matrix)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_mesh_with_material(mesh, material, model_matrix);
}

RenderCommandResult NativeBackend::draw_debug_line(const DebugLineDesc& desc)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_debug_line(desc);
}

RenderCommandResult NativeBackend::draw_debug_rect(const DebugRectDesc& desc)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_debug_rect(desc);
}

RenderCommandResult NativeBackend::draw_debug_text(const DebugTextDesc& desc)
{
    if (renderer_ == nullptr) {
        return RenderCommandResult::wrong_state;
    }

    return renderer_->draw_debug_text(desc);
}

AudioCommandResult NativeBackend::create_audio_engine(std::uint64_t& out_handle)
{
    if (!initialized_) {
        return AudioCommandResult::wrong_state;
    }

    if (audio_system_ == nullptr) {
        audio_system_ = std::make_unique<AudioSystem>();
    }

    return audio_system_->create_engine(out_handle);
}

bool NativeBackend::destroy_audio_engine(std::uint64_t handle)
{
    return audio_system_ != nullptr && audio_system_->destroy_engine(handle);
}

AudioCommandResult NativeBackend::create_sound_from_wav_memory(
    std::uint64_t audio,
    const std::uint8_t* wav_bytes,
    std::uint64_t wav_byte_count,
    std::uint64_t& out_handle)
{
    if (audio_system_ == nullptr) {
        return AudioCommandResult::wrong_state;
    }

    return audio_system_->create_sound(audio, wav_bytes, wav_byte_count, out_handle);
}

bool NativeBackend::destroy_sound(std::uint64_t audio, std::uint64_t sound)
{
    return audio_system_ != nullptr && audio_system_->destroy_sound(audio, sound);
}

AudioCommandResult NativeBackend::play_sound(std::uint64_t audio, std::uint64_t sound)
{
    if (audio_system_ == nullptr) {
        return AudioCommandResult::wrong_state;
    }

    return audio_system_->play_sound(audio, sound);
}

AudioCommandResult NativeBackend::stop_sound(std::uint64_t audio, std::uint64_t sound)
{
    if (audio_system_ == nullptr) {
        return AudioCommandResult::wrong_state;
    }

    return audio_system_->stop_sound(audio, sound);
}

AudioCommandResult NativeBackend::set_sound_volume(std::uint64_t audio, std::uint64_t sound, float volume)
{
    if (audio_system_ == nullptr) {
        return AudioCommandResult::wrong_state;
    }

    return audio_system_->set_sound_volume(audio, sound, volume);
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
    std::cerr << "[ZENO][native] " << renderer_backend_name(renderer_backend_kind_) << " renderer shutdown\n";
}

} // namespace zeno::native

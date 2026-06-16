#include <zeno/zeno_native_backend.h>

#include <cstdint>
#include <vector>

namespace {

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

void push_u16(std::vector<std::uint8_t>& bytes, std::uint16_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value & 0xff));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
}

void push_u32(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value & 0xff));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8) & 0xff));
    bytes.push_back(static_cast<std::uint8_t>((value >> 16) & 0xff));
    bytes.push_back(static_cast<std::uint8_t>((value >> 24) & 0xff));
}

std::vector<std::uint8_t> make_wav()
{
    constexpr std::uint32_t sample_rate = 8000;
    constexpr std::uint16_t channels = 1;
    constexpr std::uint16_t bits_per_sample = 16;
    constexpr std::uint16_t block_align = channels * bits_per_sample / 8;
    constexpr std::uint32_t sample_count = 400;
    constexpr std::uint32_t data_size = sample_count * block_align;

    std::vector<std::uint8_t> bytes;
    bytes.insert(bytes.end(), { 'R', 'I', 'F', 'F' });
    push_u32(bytes, 36 + data_size);
    bytes.insert(bytes.end(), { 'W', 'A', 'V', 'E' });
    bytes.insert(bytes.end(), { 'f', 'm', 't', ' ' });
    push_u32(bytes, 16);
    push_u16(bytes, 1);
    push_u16(bytes, channels);
    push_u32(bytes, sample_rate);
    push_u32(bytes, sample_rate * block_align);
    push_u16(bytes, block_align);
    push_u16(bytes, bits_per_sample);
    bytes.insert(bytes.end(), { 'd', 'a', 't', 'a' });
    push_u32(bytes, data_size);
    for (std::uint32_t i = 0; i < sample_count; ++i) {
        const std::int16_t sample = (i % 20) < 10 ? 1200 : -1200;
        push_u16(bytes, static_cast<std::uint16_t>(sample));
    }

    return bytes;
}

ZenAudioDesc audio_desc()
{
    ZenAudioDesc desc{};
    desc.size = ZEN_AUDIO_DESC_SIZE;
    desc.api_version = ZEN_AUDIO_DESC_API_VERSION;
    return desc;
}

} // namespace

int main()
{
    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 1;
    }

    ZenAudioDesc desc = audio_desc();
    ZenAudioEngineHandle audio{ 777 };
    if (!expect(zen_native_backend_create_audio_engine({}, &desc, &audio), ZEN_RESULT_INVALID_ARGUMENT)
        || audio.value != 777) {
        zen_native_backend_destroy(backend);
        return 2;
    }
    if (!expect(zen_native_backend_create_audio_engine(backend, nullptr, &audio), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 3;
    }
    if (!expect(zen_native_backend_create_audio_engine(backend, &desc, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 4;
    }

    ZenAudioDesc invalid_desc = desc;
    invalid_desc.reserved[0] = 1;
    if (!expect(zen_native_backend_create_audio_engine(backend, &invalid_desc, &audio), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 5;
    }

    audio.value = 777;
    result = zen_native_backend_create_audio_engine(backend, &desc, &audio);
    if (result == ZEN_RESULT_BACKEND_ERROR) {
        zen_native_backend_destroy(backend);
        return 0;
    }
    if (result != ZEN_RESULT_OK || audio.value == 0) {
        zen_native_backend_destroy(backend);
        return 6;
    }

    ZenSoundHandle sound{ 777 };
    if (!expect(zen_native_backend_create_sound_from_wav_memory(backend, audio, nullptr, 1, &sound), ZEN_RESULT_INVALID_ARGUMENT)
        || sound.value != 777) {
        zen_native_backend_destroy(backend);
        return 7;
    }
    if (!expect(zen_native_backend_create_sound_from_wav_memory(backend, audio, reinterpret_cast<const std::uint8_t*>("bad"), 3, &sound), ZEN_RESULT_INVALID_ARGUMENT)
        || sound.value != 777) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    const std::vector<std::uint8_t> wav = make_wav();
    result = zen_native_backend_create_sound_from_wav_memory(backend, audio, wav.data(), wav.size(), &sound);
    if (result != ZEN_RESULT_OK || sound.value == 0 || sound.value == 777) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    if (!expect(zen_native_backend_set_sound_volume(backend, audio, sound, 0.25f), ZEN_RESULT_OK)
        || !expect(zen_native_backend_set_sound_volume(backend, audio, sound, -0.1f), ZEN_RESULT_INVALID_ARGUMENT)
        || !expect(zen_native_backend_play_sound(backend, audio, sound), ZEN_RESULT_OK)
        || !expect(zen_native_backend_stop_sound(backend, audio, sound), ZEN_RESULT_OK)) {
        zen_native_backend_destroy(backend);
        return 10;
    }

    if (!expect(zen_native_backend_destroy_sound(backend, audio, sound), ZEN_RESULT_OK)
        || !expect(zen_native_backend_destroy_sound(backend, audio, sound), ZEN_RESULT_NOT_INITIALIZED)
        || !expect(zen_native_backend_destroy_audio_engine(backend, audio), ZEN_RESULT_OK)
        || !expect(zen_native_backend_destroy_audio_engine(backend, audio), ZEN_RESULT_NOT_INITIALIZED)) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    return zen_native_backend_destroy(backend) == ZEN_RESULT_OK ? 0 : 12;
}

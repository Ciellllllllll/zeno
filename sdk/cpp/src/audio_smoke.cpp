#include <zeno/zeno.hpp>

#include <filesystem>
#include <fstream>
#include <vector>

namespace {

bool ok(const zeno::Result& result)
{
    return result.ok();
}

bool not_initialized(const zeno::Result& result)
{
    return result.native_code() == ZEN_RESULT_NOT_INITIALIZED;
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
    constexpr std::uint16_t block_align = 2;
    constexpr std::uint32_t sample_count = 400;
    constexpr std::uint32_t data_size = sample_count * block_align;

    std::vector<std::uint8_t> bytes;
    bytes.insert(bytes.end(), { 'R', 'I', 'F', 'F' });
    push_u32(bytes, 36 + data_size);
    bytes.insert(bytes.end(), { 'W', 'A', 'V', 'E' });
    bytes.insert(bytes.end(), { 'f', 'm', 't', ' ' });
    push_u32(bytes, 16);
    push_u16(bytes, 1);
    push_u16(bytes, 1);
    push_u32(bytes, sample_rate);
    push_u32(bytes, sample_rate * block_align);
    push_u16(bytes, block_align);
    push_u16(bytes, 16);
    bytes.insert(bytes.end(), { 'd', 'a', 't', 'a' });
    push_u32(bytes, data_size);
    for (std::uint32_t i = 0; i < sample_count; ++i) {
        const std::int16_t sample = (i % 20) < 10 ? 900 : -900;
        push_u16(bytes, static_cast<std::uint16_t>(sample));
    }

    return bytes;
}

bool write_binary(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
{
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(file);
}

} // namespace

int main()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / L"zeno_sdk_audio_smoke_assets";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / L"audio");
    if (!write_binary(root / L"audio" / L"click.wav", make_wav())) {
        return 1;
    }

    zeno::AssetRoot assets;
    zeno::Result result = zeno::AssetRoot::from_path(root, assets);
    if (!ok(result)) {
        return 2;
    }

    zeno::NativeBackend backend;
    result = zeno::NativeBackend::create(backend);
    if (!ok(result)) {
        return 3;
    }

    zeno::AudioEngine audio;
    result = backend.create_audio_engine(audio);
    if (result.native_code() == ZEN_RESULT_BACKEND_ERROR) {
        return 0;
    }
    if (!ok(result) || !audio.valid()) {
        return 4;
    }

    zeno::Sound missing_sound;
    result = audio.load_sound(assets, "audio/missing.wav", missing_sound);
    if (!not_initialized(result) || missing_sound.valid()) {
        return 5;
    }

    zeno::Sound sound;
    result = audio.load_sound(assets, "audio/click.wav", sound);
    if (!ok(result) || !sound.valid()) {
        return 6;
    }

    if (!ok(sound.set_volume(0.5f)) || sound.set_volume(-1.0f).ok() || !ok(sound.play()) || !ok(sound.stop())) {
        return 7;
    }

    zeno::Sound moved_sound(std::move(sound));
    if (sound.valid() || !moved_sound.valid()) {
        return 8;
    }

    zeno::Sound assigned_sound;
    assigned_sound = std::move(moved_sound);
    if (moved_sound.valid() || !assigned_sound.valid()) {
        return 9;
    }

    assigned_sound.reset();
    if (assigned_sound.valid()) {
        return 10;
    }

    audio.reset();
    if (audio.valid()) {
        return 11;
    }

    std::filesystem::remove_all(root);
    return 0;
}

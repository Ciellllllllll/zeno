#include <zeno/zeno_native_backend.h>

#include <cstdint>

namespace {

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

ZenSpriteDrawDesc make_sprite_desc()
{
    ZenSpriteDrawDesc desc{};
    desc.size = ZEN_SPRITE_DRAW_DESC_SIZE;
    desc.api_version = ZEN_SPRITE_DRAW_DESC_API_VERSION;
    desc.model_matrix.elements[0] = 1.0f;
    desc.model_matrix.elements[5] = 1.0f;
    desc.model_matrix.elements[10] = 1.0f;
    desc.model_matrix.elements[15] = 1.0f;
    desc.color[0] = 1.0f;
    desc.color[1] = 1.0f;
    desc.color[2] = 1.0f;
    desc.color[3] = 1.0f;
    return desc;
}

} // namespace

int main()
{
    constexpr std::uint8_t invalid_bytes[] = { 0x7a, 0x65, 0x6e, 0x6f };

    ZenTextureHandle scratch_texture{ 777 };
    if (!expect(
            zen_native_backend_create_texture_from_memory({}, invalid_bytes, sizeof(invalid_bytes), &scratch_texture),
            ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_texture.value != 777) {
        return 1;
    }

    ZenNativeBackendConfig config{};
    config.size = ZEN_NATIVE_BACKEND_CONFIG_SIZE;
    config.api_version = ZEN_NATIVE_BACKEND_CONFIG_API_VERSION;

    ZenNativeBackendHandle backend{};
    ZenResultCode result = zen_native_backend_create(&config, &backend);
    if (result != ZEN_RESULT_OK) {
        return 2;
    }

    scratch_texture.value = 777;
    if (!expect(
            zen_native_backend_create_texture_from_memory(backend, nullptr, sizeof(invalid_bytes), &scratch_texture),
            ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_texture.value != 777) {
        zen_native_backend_destroy(backend);
        return 3;
    }

    scratch_texture.value = 777;
    if (!expect(
            zen_native_backend_create_texture_from_memory(backend, invalid_bytes, 0, &scratch_texture),
            ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_texture.value != 777) {
        zen_native_backend_destroy(backend);
        return 4;
    }

    if (!expect(
            zen_native_backend_create_texture_from_memory(backend, invalid_bytes, sizeof(invalid_bytes), nullptr),
            ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 5;
    }

    scratch_texture.value = 777;
    if (!expect(
            zen_native_backend_create_texture_from_memory(backend, invalid_bytes, sizeof(invalid_bytes), &scratch_texture),
            ZEN_RESULT_BACKEND_ERROR)
        || scratch_texture.value != 777) {
        zen_native_backend_destroy(backend);
        return 6;
    }

    ZenTextureHandle invalid_texture{};
    if (!expect(zen_native_backend_destroy_texture(backend, invalid_texture), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 7;
    }

    invalid_texture.value = 1234;
    if (!expect(zen_native_backend_destroy_texture(backend, invalid_texture), ZEN_RESULT_NOT_INITIALIZED)) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    ZenSpriteDrawDesc sprite_desc = make_sprite_desc();
    if (!expect(zen_native_backend_draw_sprite(backend, {}, &sprite_desc), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    invalid_texture.value = 1234;
    if (!expect(zen_native_backend_draw_sprite(backend, invalid_texture, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 10;
    }

    ZenSpriteDrawDesc invalid_desc = sprite_desc;
    invalid_desc.size = 0;
    if (!expect(zen_native_backend_draw_sprite(backend, invalid_texture, &invalid_desc), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    if (!expect(zen_native_backend_draw_sprite(backend, invalid_texture, &sprite_desc), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 12;
    }

    result = zen_native_backend_destroy(backend);
    if (result != ZEN_RESULT_OK) {
        return 13;
    }

    if (!expect(zen_native_backend_destroy_texture(backend, invalid_texture), ZEN_RESULT_NOT_INITIALIZED)) {
        return 14;
    }

    return 0;
}

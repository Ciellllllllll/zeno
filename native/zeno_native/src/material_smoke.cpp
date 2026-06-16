#include <zeno/zeno_native_backend.h>

namespace {

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

ZenMaterialDesc make_mesh_material_desc()
{
    ZenMaterialDesc desc{};
    desc.size = ZEN_MATERIAL_DESC_SIZE;
    desc.api_version = ZEN_MATERIAL_DESC_API_VERSION;
    desc.kind = ZEN_MATERIAL_KIND_MESH_COLOR;
    desc.blend_mode = ZEN_BLEND_MODE_OPAQUE;
    desc.depth_mode = ZEN_DEPTH_MODE_ENABLED;
    desc.cull_mode = ZEN_CULL_MODE_BACK;
    return desc;
}

ZenMaterialDesc make_sprite_material_desc()
{
    ZenMaterialDesc desc{};
    desc.size = ZEN_MATERIAL_DESC_SIZE;
    desc.api_version = ZEN_MATERIAL_DESC_API_VERSION;
    desc.kind = ZEN_MATERIAL_KIND_SPRITE_TEXTURE;
    desc.blend_mode = ZEN_BLEND_MODE_ALPHA;
    desc.depth_mode = ZEN_DEPTH_MODE_DISABLED;
    desc.cull_mode = ZEN_CULL_MODE_NONE;
    desc.texture.value = 1234;
    return desc;
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

ZenMatrix4x4 identity_matrix()
{
    ZenMatrix4x4 matrix{};
    matrix.elements[0] = 1.0f;
    matrix.elements[5] = 1.0f;
    matrix.elements[10] = 1.0f;
    matrix.elements[15] = 1.0f;
    return matrix;
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

    ZenMaterialDesc mesh_desc = make_mesh_material_desc();
    ZenMaterialHandle scratch_material{ 777 };
    if (!expect(zen_native_backend_create_material({}, &mesh_desc, &scratch_material), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_material.value != 777) {
        zen_native_backend_destroy(backend);
        return 2;
    }

    if (!expect(zen_native_backend_create_material(backend, nullptr, &scratch_material), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 3;
    }

    if (!expect(zen_native_backend_create_material(backend, &mesh_desc, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 4;
    }

    ZenMaterialDesc invalid_desc = mesh_desc;
    invalid_desc.size = 0;
    scratch_material.value = 777;
    if (!expect(zen_native_backend_create_material(backend, &invalid_desc, &scratch_material), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_material.value != 777) {
        zen_native_backend_destroy(backend);
        return 5;
    }

    invalid_desc = mesh_desc;
    invalid_desc.blend_mode = 999;
    if (!expect(zen_native_backend_create_material(backend, &invalid_desc, &scratch_material), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 6;
    }

    invalid_desc = mesh_desc;
    invalid_desc.reserved[0] = 1;
    if (!expect(zen_native_backend_create_material(backend, &invalid_desc, &scratch_material), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 7;
    }

    ZenMaterialDesc sprite_desc = make_sprite_material_desc();
    scratch_material.value = 777;
    if (!expect(zen_native_backend_create_material(backend, &sprite_desc, &scratch_material), ZEN_RESULT_BACKEND_ERROR)
        || scratch_material.value != 777) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    scratch_material.value = 777;
    if (!expect(zen_native_backend_create_material(backend, &mesh_desc, &scratch_material), ZEN_RESULT_BACKEND_ERROR)
        || scratch_material.value != 777) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    ZenSpriteDrawDesc draw_sprite_desc = make_sprite_desc();
    if (!expect(zen_native_backend_draw_sprite_with_material(backend, {}, &draw_sprite_desc), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 10;
    }

    ZenMaterialHandle fake_material{ 1234 };
    if (!expect(zen_native_backend_draw_sprite_with_material(backend, fake_material, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    if (!expect(zen_native_backend_draw_sprite_with_material(backend, fake_material, &draw_sprite_desc), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 12;
    }

    ZenMatrix4x4 identity = identity_matrix();
    ZenMeshHandle fake_mesh{ 5678 };
    if (!expect(zen_native_backend_draw_mesh_with_material(backend, {}, fake_material, &identity), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 13;
    }

    if (!expect(zen_native_backend_draw_mesh_with_material(backend, fake_mesh, {}, &identity), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 14;
    }

    if (!expect(zen_native_backend_draw_mesh_with_material(backend, fake_mesh, fake_material, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 15;
    }

    if (!expect(zen_native_backend_draw_mesh_with_material(backend, fake_mesh, fake_material, &identity), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 16;
    }

    if (!expect(zen_native_backend_destroy_material(backend, {}), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 17;
    }

    if (!expect(zen_native_backend_destroy_material(backend, fake_material), ZEN_RESULT_NOT_INITIALIZED)) {
        zen_native_backend_destroy(backend);
        return 18;
    }

    result = zen_native_backend_destroy(backend);
    if (result != ZEN_RESULT_OK) {
        return 19;
    }

    if (!expect(zen_native_backend_destroy_material(backend, fake_material), ZEN_RESULT_NOT_INITIALIZED)) {
        return 20;
    }

    return 0;
}

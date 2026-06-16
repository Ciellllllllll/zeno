#include <zeno/zeno_native_backend.h>

#include <cstddef>
#include <cstdint>

namespace {

struct MeshVertex final {
    float position[3];
    float color[4];
};

constexpr MeshVertex kVertices[] = {
    { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
    { { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
};

constexpr std::uint32_t kIndices[] = { 0, 1, 2 };

bool expect(ZenResultCode actual, ZenResultCode expected)
{
    return actual == expected;
}

ZenMeshDesc make_mesh_desc()
{
    ZenMeshDesc desc{};
    desc.size = ZEN_MESH_DESC_SIZE;
    desc.api_version = ZEN_MESH_DESC_API_VERSION;
    desc.vertex_stride_bytes = sizeof(MeshVertex);
    desc.vertex_data = kVertices;
    desc.vertex_count = 3;
    desc.index_data = kIndices;
    desc.index_count = 3;
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
    ZenMeshDesc desc = make_mesh_desc();
    ZenMeshHandle scratch_mesh{ 777 };
    if (!expect(zen_native_backend_create_mesh({}, &desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_mesh.value != 777) {
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

    if (!expect(zen_native_backend_create_mesh(backend, nullptr, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 3;
    }

    if (!expect(zen_native_backend_create_mesh(backend, &desc, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 4;
    }

    ZenMeshDesc invalid_desc = desc;
    invalid_desc.size = 0;
    scratch_mesh.value = 777;
    if (!expect(zen_native_backend_create_mesh(backend, &invalid_desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)
        || scratch_mesh.value != 777) {
        zen_native_backend_destroy(backend);
        return 5;
    }

    invalid_desc = desc;
    invalid_desc.vertex_data = nullptr;
    if (!expect(zen_native_backend_create_mesh(backend, &invalid_desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 6;
    }

    invalid_desc = desc;
    invalid_desc.index_data = nullptr;
    if (!expect(zen_native_backend_create_mesh(backend, &invalid_desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 7;
    }

    invalid_desc = desc;
    invalid_desc.vertex_count = 0;
    if (!expect(zen_native_backend_create_mesh(backend, &invalid_desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 8;
    }

    invalid_desc = desc;
    invalid_desc.index_count = 0;
    if (!expect(zen_native_backend_create_mesh(backend, &invalid_desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 9;
    }

    invalid_desc = desc;
    invalid_desc.vertex_stride_bytes = sizeof(float) * 3;
    if (!expect(zen_native_backend_create_mesh(backend, &invalid_desc, &scratch_mesh), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 10;
    }

    scratch_mesh.value = 777;
    if (!expect(zen_native_backend_create_mesh(backend, &desc, &scratch_mesh), ZEN_RESULT_BACKEND_ERROR)
        || scratch_mesh.value != 777) {
        zen_native_backend_destroy(backend);
        return 11;
    }

    ZenMatrix4x4 identity = identity_matrix();
    if (!expect(zen_native_backend_draw_mesh(backend, {}, &identity), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 12;
    }

    ZenMeshHandle fake_mesh{ 1234 };
    if (!expect(zen_native_backend_draw_mesh(backend, fake_mesh, nullptr), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 13;
    }

    if (!expect(zen_native_backend_draw_mesh(backend, fake_mesh, &identity), ZEN_RESULT_BACKEND_ERROR)) {
        zen_native_backend_destroy(backend);
        return 14;
    }

    if (!expect(zen_native_backend_destroy_mesh(backend, {}), ZEN_RESULT_INVALID_ARGUMENT)) {
        zen_native_backend_destroy(backend);
        return 15;
    }

    if (!expect(zen_native_backend_destroy_mesh(backend, fake_mesh), ZEN_RESULT_NOT_INITIALIZED)) {
        zen_native_backend_destroy(backend);
        return 16;
    }

    result = zen_native_backend_destroy(backend);
    if (result != ZEN_RESULT_OK) {
        return 17;
    }

    if (!expect(zen_native_backend_destroy_mesh(backend, fake_mesh), ZEN_RESULT_NOT_INITIALIZED)) {
        return 18;
    }

    return 0;
}

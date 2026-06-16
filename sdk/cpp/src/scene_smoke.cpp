#include <zeno/zeno.hpp>

namespace {

bool ok(const zeno::Result& result)
{
    return result.ok();
}

bool invalid_argument(const zeno::Result& result)
{
    return result.native_code() == ZEN_RESULT_INVALID_ARGUMENT;
}

} // namespace

int main()
{
    zeno::Scene scene;
    if (scene.has_object(zeno::ObjectId{})) {
        return 1;
    }

    const zeno::ObjectId first = scene.create_object();
    const zeno::ObjectId second = scene.create_object();
    const zeno::ObjectId third = scene.create_object();
    if (!first.valid() || !second.valid() || !third.valid() || first == second || second == third) {
        return 2;
    }

    const auto update_order = scene.objects_in_update_order();
    if (update_order.size() != 3 || update_order[0] != first || update_order[1] != second || update_order[2] != third) {
        return 3;
    }

    zeno::Transform transform{};
    transform.position = zeno::Vec3{ 1.0f, 2.0f, 3.0f };
    if (!ok(scene.set_transform(second, transform))) {
        return 4;
    }

    zeno::Transform* stored_transform = scene.transform(second);
    if (stored_transform == nullptr || stored_transform->position.x != 1.0f || stored_transform->position.y != 2.0f || stored_transform->position.z != 3.0f) {
        return 5;
    }

    if (!invalid_argument(scene.set_transform(zeno::ObjectId{}, transform)) || scene.transform(zeno::ObjectId{}) != nullptr) {
        return 6;
    }

    if (!invalid_argument(scene.set_sprite_renderer(first, zeno::SpriteRenderer{}))) {
        return 7;
    }
    if (!invalid_argument(scene.set_mesh_renderer(first, zeno::MeshRenderer{}))) {
        return 8;
    }
    if (!invalid_argument(scene.set_triangle_renderer(first, zeno::TriangleRenderer{}))) {
        return 9;
    }

    const zeno::MaterialId material{ 100 };
    const zeno::MeshId mesh{ 200 };
    const zeno::TriangleId triangle{ 300 };
    if (!ok(scene.set_sprite_renderer(first, zeno::SpriteRenderer{ material, zeno::Color{} }))
        || scene.renderable_kind(first) != zeno::RenderableKind::sprite) {
        return 10;
    }
    if (!ok(scene.set_mesh_renderer(first, zeno::MeshRenderer{ mesh, material }))
        || scene.renderable_kind(first) != zeno::RenderableKind::mesh) {
        return 11;
    }
    if (!ok(scene.set_triangle_renderer(first, zeno::TriangleRenderer{ triangle }))
        || scene.renderable_kind(first) != zeno::RenderableKind::triangle) {
        return 12;
    }
    if (!ok(scene.clear_renderer(first)) || scene.renderable_kind(first) != zeno::RenderableKind::none) {
        return 13;
    }

    if (!ok(scene.destroy_object(second)) || scene.has_object(second) || scene.transform(second) != nullptr) {
        return 14;
    }
    if (!invalid_argument(scene.destroy_object(second))) {
        return 15;
    }

    const auto render_order = scene.objects_in_render_order();
    if (render_order.size() != 2 || render_order[0] != first || render_order[1] != third) {
        return 16;
    }

    scene.clear();
    if (scene.has_object(first) || scene.has_object(third) || !scene.objects_in_update_order().empty()) {
        return 17;
    }

    const zeno::ObjectId after_clear = scene.create_object();
    if (!after_clear.valid() || after_clear.value <= third.value) {
        return 18;
    }

    zeno::NativeBackend invalid_backend;
    if (!invalid_argument(scene.render(invalid_backend))) {
        return 19;
    }

    zeno::ResourceManager resources;
    if (resources.material(material) != nullptr || resources.mesh(mesh) != nullptr || resources.triangle(triangle) != nullptr) {
        return 20;
    }
    if (!invalid_argument(resources.destroy(material))
        || !invalid_argument(resources.destroy(mesh))
        || !invalid_argument(resources.destroy(triangle))
        || !invalid_argument(resources.destroy(zeno::TextureId{ 400 }))
        || !invalid_argument(resources.destroy(zeno::SoundId{ 500 }))) {
        return 21;
    }

    return 0;
}

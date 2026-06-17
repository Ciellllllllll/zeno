#include <zeno/zeno.hpp>

namespace zeno {

namespace {

Result invalid_argument()
{
    return Result(ZEN_RESULT_INVALID_ARGUMENT);
}

void log_scene_error(std::string_view message)
{
    log_message(LogLevel::error, "scene", message);
}

} // namespace

ObjectId Scene::create_object()
{
    ObjectState state{};
    state.id.value = next_object_id_++;
    state.alive = true;
    objects_.push_back(state);
    return state.id;
}

Result Scene::destroy_object(ObjectId id)
{
    ObjectState* state = find_object(id);
    if (state == nullptr) {
        log_scene_error("scene: destroy target object is missing");
        return invalid_argument();
    }

    state->alive = false;
    state->renderable_kind = RenderableKind::none;
    state->sprite_renderer = {};
    state->mesh_renderer = {};
    state->triangle_renderer = {};
    return Result();
}

void Scene::clear()
{
    objects_.clear();
}

bool Scene::has_object(ObjectId id) const
{
    return find_object(id) != nullptr;
}

Result Scene::set_transform(ObjectId id, const Transform& transform)
{
    ObjectState* state = find_object(id);
    if (state == nullptr) {
        log_scene_error("scene: transform target object is missing");
        return invalid_argument();
    }

    state->transform = transform;
    return Result();
}

Transform* Scene::transform(ObjectId id)
{
    ObjectState* state = find_object(id);
    if (state == nullptr) {
        return nullptr;
    }

    return &state->transform;
}

const Transform* Scene::transform(ObjectId id) const
{
    const ObjectState* state = find_object(id);
    if (state == nullptr) {
        return nullptr;
    }

    return &state->transform;
}

Result Scene::set_sprite_renderer(ObjectId id, const SpriteRenderer& renderer)
{
    if (!renderer.material.valid()) {
        log_scene_error("scene: sprite renderer requires a valid material ID");
        return invalid_argument();
    }

    ObjectState* state = find_object(id);
    if (state == nullptr) {
        log_scene_error("scene: sprite renderer target object is missing");
        return invalid_argument();
    }

    state->renderable_kind = RenderableKind::sprite;
    state->sprite_renderer = renderer;
    state->mesh_renderer = {};
    state->triangle_renderer = {};
    return Result();
}

Result Scene::set_mesh_renderer(ObjectId id, const MeshRenderer& renderer)
{
    if (!renderer.mesh.valid() || !renderer.material.valid()) {
        log_scene_error("scene: mesh renderer requires valid mesh and material IDs");
        return invalid_argument();
    }

    ObjectState* state = find_object(id);
    if (state == nullptr) {
        log_scene_error("scene: mesh renderer target object is missing");
        return invalid_argument();
    }

    state->renderable_kind = RenderableKind::mesh;
    state->sprite_renderer = {};
    state->mesh_renderer = renderer;
    state->triangle_renderer = {};
    return Result();
}

Result Scene::set_triangle_renderer(ObjectId id, const TriangleRenderer& renderer)
{
    if (!renderer.triangle.valid()) {
        log_scene_error("scene: triangle renderer requires a valid triangle ID");
        return invalid_argument();
    }

    ObjectState* state = find_object(id);
    if (state == nullptr) {
        log_scene_error("scene: triangle renderer target object is missing");
        return invalid_argument();
    }

    state->renderable_kind = RenderableKind::triangle;
    state->sprite_renderer = {};
    state->mesh_renderer = {};
    state->triangle_renderer = renderer;
    return Result();
}

Result Scene::clear_renderer(ObjectId id)
{
    ObjectState* state = find_object(id);
    if (state == nullptr) {
        log_scene_error("scene: renderer clear target object is missing");
        return invalid_argument();
    }

    state->renderable_kind = RenderableKind::none;
    state->sprite_renderer = {};
    state->mesh_renderer = {};
    state->triangle_renderer = {};
    return Result();
}

RenderableKind Scene::renderable_kind(ObjectId id) const
{
    const ObjectState* state = find_object(id);
    if (state == nullptr) {
        return RenderableKind::none;
    }

    return state->renderable_kind;
}

std::vector<ObjectId> Scene::objects_in_update_order() const
{
    std::vector<ObjectId> ids;
    ids.reserve(objects_.size());
    for (const ObjectState& state : objects_) {
        if (state.alive) {
            ids.push_back(state.id);
        }
    }

    return ids;
}

std::vector<ObjectId> Scene::objects_in_render_order() const
{
    return objects_in_update_order();
}

Result Scene::render(NativeBackend& backend) const
{
    ResourceManager resources;
    return render(backend, resources);
}

Result Scene::render(NativeBackend& backend, const ResourceManager& resources) const
{
    if (!backend.valid()) {
        log_scene_error("scene: render requires a valid native backend");
        return invalid_argument();
    }

    for (const ObjectState& state : objects_) {
        if (!state.alive) {
            continue;
        }

        Result result;
        switch (state.renderable_kind) {
        case RenderableKind::sprite: {
            const Material* material = resources.material(state.sprite_renderer.material);
            if (material == nullptr || !material->valid()) {
                log_scene_error("scene: sprite material ID is missing, stale, or invalid");
                return invalid_argument();
            }
            SpriteDrawDesc desc{};
            desc.transform = state.transform;
            desc.color = state.sprite_renderer.color;
            result = backend.draw_sprite(*material, desc);
            break;
        }
        case RenderableKind::mesh: {
            const Mesh* mesh = resources.mesh(state.mesh_renderer.mesh);
            const Material* material = resources.material(state.mesh_renderer.material);
            if (mesh == nullptr || material == nullptr || !mesh->valid() || !material->valid()) {
                log_scene_error("scene: mesh or material ID is missing, stale, or invalid");
                return invalid_argument();
            }
            result = backend.draw_mesh(*mesh, *material, state.transform);
            break;
        }
        case RenderableKind::triangle: {
            const RenderTriangle* triangle = resources.triangle(state.triangle_renderer.triangle);
            if (triangle == nullptr || !triangle->valid()) {
                log_scene_error("scene: triangle ID is missing, stale, or invalid");
                return invalid_argument();
            }
            result = backend.draw_triangle(*triangle, state.transform);
            break;
        }
        case RenderableKind::none:
            result = Result();
            break;
        }

        if (!result.ok()) {
            return result;
        }
    }

    return Result();
}

Scene::ObjectState* Scene::find_object(ObjectId id)
{
    if (!id.valid()) {
        return nullptr;
    }

    for (ObjectState& state : objects_) {
        if (state.alive && state.id == id) {
            return &state;
        }
    }

    return nullptr;
}

const Scene::ObjectState* Scene::find_object(ObjectId id) const
{
    if (!id.valid()) {
        return nullptr;
    }

    for (const ObjectState& state : objects_) {
        if (state.alive && state.id == id) {
            return &state;
        }
    }

    return nullptr;
}

} // namespace zeno

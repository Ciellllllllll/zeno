# 2D Rendering

The current 2D path is built from textures, sprite materials, transforms, scene renderers, debug rectangles, and debug text.

## Workflow

The focused 2D sample uses this public SDK sequence:

1. `GameApp` resolves executable-relative `assets/`, then loads the project and scene selected by `GameAppConfig::project_path`.
2. The module reads `GameContext::scene` and finds the sample sprite description named `player`.
3. `ResourceManager::create_texture` loads the texture named by `SceneObjectDesc::reference`.
4. `ResourceManager::create_sprite_material` creates a sprite material for that texture.
5. The module creates a runtime `ObjectId`, copies the scene transform, and attaches `SpriteRenderer`.
6. Each render callback calls `NativeBackend::begin_frame`, `clear`, sets an orthographic camera, renders the runtime scene, draws debug helpers, and calls `present`.

Project and scene loading provides descriptions only. Game code still decides which descriptions become runtime objects and which resources they use.

## Texture And Sprite Material

The focused 2D sample loads a texture referenced by scene data, then creates a sprite material:

```cpp
zeno::TextureId texture{};
context.resources->create_texture(*context.backend, *context.assets, player->reference, texture);

zeno::MaterialDesc material_desc{};
material_desc.kind = zeno::MaterialKind::sprite_texture;
material_desc.blend_mode = zeno::BlendMode::alpha;
material_desc.depth_mode = zeno::DepthMode::disabled;
material_desc.cull_mode = zeno::CullMode::none;

zeno::MaterialId material{};
context.resources->create_sprite_material(*context.backend, texture, material_desc, material);
```

For sprites, keep `MaterialKind::sprite_texture`. The focused 2D sample uses alpha blending, disables depth testing, and disables culling so the texture-backed quad behaves like a screen-space 2D object under the orthographic camera.

## Scene Object

Create an object, set its transform, and attach a sprite renderer:

```cpp
zeno::ObjectId player = context.runtime_scene->create_object();
context.runtime_scene->set_transform(player, player_desc.transform);
context.runtime_scene->set_sprite_renderer(player, zeno::SpriteRenderer{ material, player_desc.color });
```

The scene renders through:

```cpp
context.runtime_scene->render(*context.backend, *context.resources);
```

## Camera And Debug Drawing

The 2D sample uses an orthographic camera:

```cpp
const zeno::Camera camera = zeno::Camera::orthographic(2.0f, 1.125f, 0.1f, 10.0f);
context.backend->set_camera_matrix(camera.view_projection());
```

Set the camera before drawing the runtime scene. The sample's transform positions are authored for this orthographic view.

Debug helpers include:

- `NativeBackend::draw_debug_rect_2d`
- `NativeBackend::draw_debug_text`

The focused 2D sample draws a debug rectangle around the sprite and overlay text for frame index, sound state, and mouse coordinates. Debug text uses the built-in ASCII line font. It is useful for frame counters, state, and diagnostics, but it is not a full UI or font rendering system.

## Diagnostics

Common 2D setup failures report through `zeno::last_diagnostic()` and optional `zeno::set_log_sink`:

- Missing texture assets include the requested relative path, active asset root, and resolved path.
- Invalid or stale texture IDs fail sprite material creation with a ResourceManager diagnostic.
- Missing or stale sprite material IDs fail `Scene::render` with a scene diagnostic.
- Sprite draw, camera, debug rectangle, and debug text native failures log the failed operation name.

## Limits

The current 2D path is texture-backed sprites plus immediate debug rectangles and debug text. It does not provide sprite batching, retained UI widgets, general text layout, a font asset system, a physics engine, or an editor asset pipeline.

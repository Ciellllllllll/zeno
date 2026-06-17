# 2D Rendering

The current 2D path is built from textures, sprite materials, transforms, scene renderers, debug rectangles, and debug text.

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

Debug helpers include:

- `NativeBackend::draw_debug_rect_2d`
- `NativeBackend::draw_debug_text`

Debug text uses the built-in ASCII line font. It is useful for frame counters, state, and diagnostics, but it is not a full UI or font rendering system.

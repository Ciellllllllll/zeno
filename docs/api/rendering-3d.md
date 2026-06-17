# 3D Rendering

The current 3D path is a small SDK surface for indexed meshes, material state, transforms, scene rendering, perspective cameras, and debug lines.

## Mesh Data

Meshes are created from `zeno::MeshVertex` arrays and index arrays:

```cpp
context.resources->create_mesh(
    *context.backend,
    vertices.data(),
    static_cast<std::uint32_t>(vertices.size()),
    indices.data(),
    static_cast<std::uint32_t>(indices.size()),
    mesh);
```

`MeshVertex` contains a position and color. The focused 3D sample builds a cube from 8 vertices and 36 indices.

## Mesh Material

The focused 3D sample uses a color mesh material:

```cpp
zeno::MaterialDesc material_desc{};
material_desc.kind = zeno::MaterialKind::mesh_color;
material_desc.blend_mode = zeno::BlendMode::opaque;
material_desc.depth_mode = zeno::DepthMode::enabled;
material_desc.cull_mode = zeno::CullMode::back;
context.resources->create_material(*context.backend, material_desc, material);
```

## Scene Rendering

Attach a mesh renderer to a scene object:

```cpp
zeno::ObjectId cube = context.runtime_scene->create_object();
context.runtime_scene->set_transform(cube, cube_desc.transform);
context.runtime_scene->set_mesh_renderer(cube, zeno::MeshRenderer{ mesh, material });
```

Render it through the scene:

```cpp
context.runtime_scene->render(*context.backend, *context.resources);
```

## Camera And Debug Lines

The focused 3D sample uses a perspective camera:

```cpp
zeno::Camera camera = zeno::Camera::perspective(1.0471976f, aspect_ratio, 0.1f, 10.0f);
context.backend->set_camera_matrix(camera.view_projection());
```

`NativeBackend::draw_debug_line` is available for simple diagnostics. It is used by the 3D sample to draw a line over the scene.

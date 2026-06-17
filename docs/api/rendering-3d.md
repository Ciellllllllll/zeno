# 3D Rendering

The current 3D path is a small SDK surface for indexed position/color meshes, material state, transforms, scene rendering, perspective cameras, and debug lines. It is intentionally the public sample workflow, not a general renderer, mesh importer, animation system, or lighting redesign.

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

`MeshVertex` contains a position and color. The focused 3D sample builds a cube from 8 vertices and 36 indices. Mesh data is copied into backend-owned buffers during creation; the caller does not keep ownership of native GPU resources.

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

`MaterialKind::mesh_color` is the fixed color-mesh path. Use `BlendMode::opaque`, `DepthMode::enabled`, and `CullMode::back` for the focused sample unless a later phase explicitly broadens the renderer.

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

`Scene::render` resolves `MeshId` and `MaterialId` through `ResourceManager`. Missing, stale, or invalid mesh/material IDs return `ZEN_RESULT_INVALID_ARGUMENT` and write a scene diagnostic.

## Camera And Debug Lines

The focused 3D sample uses a perspective camera:

```cpp
zeno::Camera camera = zeno::Camera::perspective(1.0471976f, aspect_ratio, 0.1f, 10.0f);
context.backend->set_camera_matrix(camera.view_projection());
```

`NativeBackend::draw_debug_line` is available for simple diagnostics. It is used by the 3D sample to draw a line over the scene.

## Frame Order

The focused 3D sample uses this order inside `on_render`:

1. `NativeBackend::begin_frame`.
2. `NativeBackend::clear`.
3. `NativeBackend::set_camera_matrix`.
4. `Scene::render`.
5. `NativeBackend::draw_debug_line`.
6. `NativeBackend::draw_debug_text`.
7. `NativeBackend::present`.

Draw calls outside an active renderer/frame, invalid backend handles, invalid resource IDs, and missing scene resources return stable `ZenResultCode` values.

## Diagnostics

The SDK diagnostic channel records common 3D setup and draw failures:

- mesh creation failures through `NativeBackend::create_mesh` and `ResourceManager::create_mesh`
- mesh material creation failures through `NativeBackend::create_material` and `ResourceManager::create_material`
- camera setup failures through `NativeBackend::set_camera_matrix`
- mesh draw failures through `NativeBackend::draw_mesh`
- debug line/text failures through `NativeBackend::draw_debug_line` and `NativeBackend::draw_debug_text`
- missing scene mesh/material IDs during `Scene::render`

Use `zeno::last_diagnostic()` for the latest message or `zeno::set_log_sink` to collect messages.

## Current Limits

- No mesh file importer or mesh asset format.
- No animation, skeletal meshes, normals, UVs, lights, PBR materials, or editor tooling.
- No automatic scene-to-resource instantiation; game code creates the mesh and material resources explicitly.
- The focused sample is windowed. Headless regression coverage validates SDK failure paths and resource-ID behavior, but it does not replace manual visual inspection of the rotating cube.

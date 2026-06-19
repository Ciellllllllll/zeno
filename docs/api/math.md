# Math, Camera, And Collision

`zeno/math.hpp` provides the small math surface used by the current SDK samples.

## Vectors And Matrices

Public vector types:

- `zeno::Vec2`
- `zeno::Vec3`
- `zeno::Vec4`

`zeno::Mat4` stores 16 floats and provides helpers for identity, translation, scale, Z rotation, orthographic projection, and perspective projection.

Common helpers:

```cpp
zeno::Mat4 model = transform.matrix();
zeno::Mat4 view_projection = camera.view_projection();
zeno::Mat4 combined = zeno::multiply(model, view_projection);
```

## Transform

`zeno::Transform` contains:

- `position`
- `rotation_z_radians`
- `scale`

The current transform helper is intentionally small and supports the sample and focused SDK rendering paths.

## Camera

Use `zeno::Camera::orthographic` for the 2D sample path:

```cpp
zeno::Camera camera = zeno::Camera::orthographic(2.0f, 1.125f, 0.1f, 10.0f);
```

Use `zeno::Camera::perspective` for the 3D sample path:

```cpp
zeno::Camera camera = zeno::Camera::perspective(1.0471976f, aspect_ratio, 0.1f, 10.0f);
```

Pass `camera.view_projection()` to `NativeBackend::set_camera_matrix`.

## AABB Helpers

2D collision helpers include:

- `zeno::Aabb2::from_center_half_extents`
- `zeno::aabb_from_transform_2d`
- `zeno::intersects`
- `zeno::contains`

These helpers are enough for the current sample collision checks. They are not a physics engine or broadphase system.

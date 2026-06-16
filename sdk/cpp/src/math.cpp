#include <zeno/math.hpp>

namespace zeno {

Aabb2 Aabb2::from_center_half_extents(const Vec2& center, const Vec2& half_extents)
{
    Aabb2 result{};
    result.center = center;
    result.half_extents = Vec2{ std::fabs(half_extents.x), std::fabs(half_extents.y) };
    return result;
}

Vec2 Aabb2::min() const
{
    return Vec2{ center.x - half_extents.x, center.y - half_extents.y };
}

Vec2 Aabb2::max() const
{
    return Vec2{ center.x + half_extents.x, center.y + half_extents.y };
}

Mat4 Mat4::identity()
{
    Mat4 result{};
    result.at(0, 0) = 1.0f;
    result.at(1, 1) = 1.0f;
    result.at(2, 2) = 1.0f;
    result.at(3, 3) = 1.0f;
    return result;
}

Mat4 Mat4::translation(const Vec3& value)
{
    Mat4 result = identity();
    result.at(3, 0) = value.x;
    result.at(3, 1) = value.y;
    result.at(3, 2) = value.z;
    return result;
}

Mat4 Mat4::scale(const Vec3& value)
{
    Mat4 result{};
    result.at(0, 0) = value.x;
    result.at(1, 1) = value.y;
    result.at(2, 2) = value.z;
    result.at(3, 3) = 1.0f;
    return result;
}

Mat4 Mat4::rotation_z(float radians)
{
    Mat4 result = identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    result.at(0, 0) = c;
    result.at(0, 1) = s;
    result.at(1, 0) = -s;
    result.at(1, 1) = c;
    return result;
}

Mat4 Mat4::orthographic_lh(float width, float height, float near_z, float far_z)
{
    Mat4 result{};
    result.at(0, 0) = 2.0f / width;
    result.at(1, 1) = 2.0f / height;
    result.at(2, 2) = 1.0f / (far_z - near_z);
    result.at(3, 2) = -near_z / (far_z - near_z);
    result.at(3, 3) = 1.0f;
    return result;
}

Mat4 Mat4::perspective_lh(float vertical_fov_radians, float aspect_ratio, float near_z, float far_z)
{
    Mat4 result{};
    const float y_scale = 1.0f / std::tan(vertical_fov_radians * 0.5f);
    const float x_scale = y_scale / aspect_ratio;
    result.at(0, 0) = x_scale;
    result.at(1, 1) = y_scale;
    result.at(2, 2) = far_z / (far_z - near_z);
    result.at(2, 3) = 1.0f;
    result.at(3, 2) = -(near_z * far_z) / (far_z - near_z);
    return result;
}

Mat4 multiply(const Mat4& left, const Mat4& right)
{
    Mat4 result{};
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            float value = 0.0f;
            for (int i = 0; i < 4; ++i) {
                value += left.at(row, i) * right.at(i, column);
            }
            result.at(row, column) = value;
        }
    }
    return result;
}

Mat4 Transform::matrix() const
{
    const Mat4 scale_matrix = Mat4::scale(scale);
    const Mat4 rotation_matrix = Mat4::rotation_z(rotation_z_radians);
    const Mat4 translation_matrix = Mat4::translation(position);
    return multiply(multiply(scale_matrix, rotation_matrix), translation_matrix);
}

bool intersects(const Aabb2& left, const Aabb2& right)
{
    const Vec2 left_min = left.min();
    const Vec2 left_max = left.max();
    const Vec2 right_min = right.min();
    const Vec2 right_max = right.max();
    return left_min.x <= right_max.x
        && left_max.x >= right_min.x
        && left_min.y <= right_max.y
        && left_max.y >= right_min.y;
}

bool contains(const Aabb2& box, const Vec2& point)
{
    const Vec2 box_min = box.min();
    const Vec2 box_max = box.max();
    return point.x >= box_min.x
        && point.x <= box_max.x
        && point.y >= box_min.y
        && point.y <= box_max.y;
}

Aabb2 aabb_from_transform_2d(const Transform& transform)
{
    return Aabb2::from_center_half_extents(
        Vec2{ transform.position.x, transform.position.y },
        Vec2{ transform.scale.x * 0.5f, transform.scale.y * 0.5f });
}

Camera Camera::orthographic(float width, float height, float near_z, float far_z)
{
    Camera camera{};
    camera.projection = Mat4::orthographic_lh(width, height, near_z, far_z);
    return camera;
}

Camera Camera::perspective(float vertical_fov_radians, float aspect_ratio, float near_z, float far_z)
{
    Camera camera{};
    camera.projection = Mat4::perspective_lh(vertical_fov_radians, aspect_ratio, near_z, far_z);
    return camera;
}

Mat4 Camera::view_projection() const
{
    return multiply(view, projection);
}

} // namespace zeno

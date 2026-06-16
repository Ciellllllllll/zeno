#pragma once

#include <cmath>

namespace zeno {

struct Vec2 final {
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3 final {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Vec4 final {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

struct Mat4 final {
    float m[16]{};

    static Mat4 identity();
    static Mat4 translation(const Vec3& value);
    static Mat4 scale(const Vec3& value);
    static Mat4 rotation_z(float radians);
    static Mat4 orthographic_lh(float width, float height, float near_z, float far_z);
    static Mat4 perspective_lh(float vertical_fov_radians, float aspect_ratio, float near_z, float far_z);

    float at(int row, int column) const { return m[row * 4 + column]; }
    float& at(int row, int column) { return m[row * 4 + column]; }
};

Mat4 multiply(const Mat4& left, const Mat4& right);

struct Transform final {
    Vec3 position{};
    float rotation_z_radians = 0.0f;
    Vec3 scale{ 1.0f, 1.0f, 1.0f };

    Mat4 matrix() const;
};

struct Camera final {
    Mat4 view = Mat4::identity();
    Mat4 projection = Mat4::identity();

    static Camera orthographic(float width, float height, float near_z, float far_z);
    static Camera perspective(float vertical_fov_radians, float aspect_ratio, float near_z, float far_z);

    Mat4 view_projection() const;
};

} // namespace zeno

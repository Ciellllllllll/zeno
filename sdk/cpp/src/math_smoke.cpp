#include <zeno/math.hpp>

#include <cmath>

namespace {

bool near(float a, float b)
{
    return std::fabs(a - b) < 0.0001f;
}

} // namespace

int main()
{
    const zeno::Mat4 identity = zeno::Mat4::identity();
    if (!near(identity.at(0, 0), 1.0f) || !near(identity.at(3, 3), 1.0f)) {
        return 1;
    }

    const zeno::Mat4 translation = zeno::Mat4::translation(zeno::Vec3{ 2.0f, 3.0f, 4.0f });
    if (!near(translation.at(3, 0), 2.0f)
        || !near(translation.at(3, 1), 3.0f)
        || !near(translation.at(3, 2), 4.0f)) {
        return 2;
    }

    const zeno::Mat4 scale = zeno::Mat4::scale(zeno::Vec3{ 2.0f, 3.0f, 4.0f });
    if (!near(scale.at(0, 0), 2.0f)
        || !near(scale.at(1, 1), 3.0f)
        || !near(scale.at(2, 2), 4.0f)) {
        return 3;
    }

    const zeno::Mat4 rotation = zeno::Mat4::rotation_z(1.5707963f);
    if (!near(rotation.at(0, 0), 0.0f)
        || !near(rotation.at(0, 1), 1.0f)
        || !near(rotation.at(1, 0), -1.0f)
        || !near(rotation.at(1, 1), 0.0f)) {
        return 4;
    }

    zeno::Transform transform{};
    transform.position = zeno::Vec3{ 1.0f, 2.0f, 3.0f };
    transform.scale = zeno::Vec3{ 2.0f, 2.0f, 2.0f };
    const zeno::Mat4 model = transform.matrix();
    if (!near(model.at(0, 0), 2.0f)
        || !near(model.at(1, 1), 2.0f)
        || !near(model.at(3, 0), 1.0f)
        || !near(model.at(3, 1), 2.0f)
        || !near(model.at(3, 2), 3.0f)) {
        return 5;
    }

    const zeno::Camera ortho = zeno::Camera::orthographic(2.0f, 2.0f, 0.0f, 1.0f);
    const zeno::Mat4 view_projection = ortho.view_projection();
    if (!near(view_projection.at(0, 0), 1.0f)
        || !near(view_projection.at(1, 1), 1.0f)
        || !near(view_projection.at(2, 2), 1.0f)) {
        return 6;
    }

    const zeno::Camera perspective = zeno::Camera::perspective(1.5707963f, 1.0f, 0.1f, 100.0f);
    if (!near(perspective.projection.at(0, 0), 1.0f)
        || !near(perspective.projection.at(1, 1), 1.0f)
        || !near(perspective.projection.at(2, 3), 1.0f)) {
        return 7;
    }

    return 0;
}

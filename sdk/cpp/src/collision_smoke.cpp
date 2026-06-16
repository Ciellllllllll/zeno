#include <zeno/math.hpp>

#include <cmath>

namespace {

bool near(float left, float right)
{
    return std::fabs(left - right) < 0.0001f;
}

} // namespace

int main()
{
    const zeno::Aabb2 box = zeno::Aabb2::from_center_half_extents(
        zeno::Vec2{ 0.0f, 0.0f },
        zeno::Vec2{ 1.0f, 2.0f });
    const zeno::Vec2 min = box.min();
    const zeno::Vec2 max = box.max();
    if (!near(min.x, -1.0f) || !near(min.y, -2.0f) || !near(max.x, 1.0f) || !near(max.y, 2.0f)) {
        return 1;
    }

    const zeno::Aabb2 overlapping = zeno::Aabb2::from_center_half_extents(
        zeno::Vec2{ 1.5f, 0.0f },
        zeno::Vec2{ 1.0f, 1.0f });
    if (!zeno::intersects(box, overlapping)) {
        return 2;
    }

    const zeno::Aabb2 separated_x = zeno::Aabb2::from_center_half_extents(
        zeno::Vec2{ 3.1f, 0.0f },
        zeno::Vec2{ 1.0f, 1.0f });
    if (zeno::intersects(box, separated_x)) {
        return 3;
    }

    const zeno::Aabb2 separated_y = zeno::Aabb2::from_center_half_extents(
        zeno::Vec2{ 0.0f, 3.1f },
        zeno::Vec2{ 1.0f, 1.0f });
    if (zeno::intersects(box, separated_y)) {
        return 4;
    }

    const zeno::Aabb2 edge_touching = zeno::Aabb2::from_center_half_extents(
        zeno::Vec2{ 2.0f, 0.0f },
        zeno::Vec2{ 1.0f, 1.0f });
    if (!zeno::intersects(box, edge_touching)) {
        return 5;
    }

    if (!zeno::contains(box, zeno::Vec2{ 0.25f, -0.25f })) {
        return 6;
    }
    if (zeno::contains(box, zeno::Vec2{ 1.1f, 0.0f })) {
        return 7;
    }

    const zeno::Aabb2 normalized = zeno::Aabb2::from_center_half_extents(
        zeno::Vec2{ 2.0f, 3.0f },
        zeno::Vec2{ -4.0f, -5.0f });
    if (!near(normalized.half_extents.x, 4.0f) || !near(normalized.half_extents.y, 5.0f)) {
        return 8;
    }

    zeno::Transform transform{};
    transform.position = zeno::Vec3{ -0.5f, 0.75f, 1.0f };
    transform.scale = zeno::Vec3{ 0.4f, 0.8f, 1.0f };
    transform.rotation_z_radians = 1.57f;
    const zeno::Aabb2 from_transform = zeno::aabb_from_transform_2d(transform);
    if (!near(from_transform.center.x, -0.5f)
        || !near(from_transform.center.y, 0.75f)
        || !near(from_transform.half_extents.x, 0.2f)
        || !near(from_transform.half_extents.y, 0.4f)) {
        return 9;
    }

    return 0;
}

#include <zeno/game_module.hpp>
#include <zeno/math.hpp>
#include <zeno/zeno.hpp>

#include <gtest/gtest.h>

TEST(ZenoCppSmoke, PublicSdkHeadersExposeDefaultTypes)
{
    zeno::EngineConfig engine_config{};
    EXPECT_DOUBLE_EQ(engine_config.target_fps, 60.0);
    EXPECT_EQ(engine_config.max_test_frames, ZEN_MAX_TEST_FRAMES_UNLIMITED);

    zeno::WindowConfig window_config{};
    EXPECT_EQ(window_config.width, 1280u);
    EXPECT_EQ(window_config.height, 720u);

    zeno::GameAppConfig app_config{};
    EXPECT_EQ(app_config.project_path, "project.zproj");
}

TEST(ZenoCppSmoke, PublicMathTypesLinkThroughSdkTarget)
{
    const zeno::Mat4 identity = zeno::Mat4::identity();
    EXPECT_FLOAT_EQ(identity.at(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(identity.at(1, 1), 1.0f);
    EXPECT_FLOAT_EQ(identity.at(2, 2), 1.0f);
    EXPECT_FLOAT_EQ(identity.at(3, 3), 1.0f);

    const zeno::Mat4 translation = zeno::Mat4::translation(zeno::Vec3{ 2.0f, 3.0f, 4.0f });
    EXPECT_FLOAT_EQ(translation.at(3, 0), 2.0f);
    EXPECT_FLOAT_EQ(translation.at(3, 1), 3.0f);
    EXPECT_FLOAT_EQ(translation.at(3, 2), 4.0f);
}

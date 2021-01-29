/// \author Jian Wen
/// \date 2021/01/29

#include "../src/types.hpp"

#include <gtest/gtest.h>

TEST(Pos, initialize_instance)
{
    auto lon = 113.93593149478123;
    auto lat = 22.312648328005512;

    Pos pos{lon, lat};

    EXPECT_DOUBLE_EQ(pos.lon, lon);
    EXPECT_DOUBLE_EQ(pos.lat, lat);
}